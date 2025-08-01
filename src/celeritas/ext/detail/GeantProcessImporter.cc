//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/ext/detail/GeantProcessImporter.cc
//---------------------------------------------------------------------------//
#include "GeantProcessImporter.hh"

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <CLHEP/Units/SystemOfUnits.h>
#include <G4ParticleDefinition.hh>
#include <G4ParticleTable.hh>
#include <G4Physics2DVector.hh>
#include <G4PhysicsTable.hh>
#include <G4PhysicsVector.hh>
#include <G4PhysicsVectorType.hh>
#include <G4ProcessManager.hh>
#include <G4ProcessType.hh>
#include <G4ProcessVector.hh>
#include <G4ProductionCutsTable.hh>
#include <G4String.hh>
#include <G4VEmProcess.hh>
#include <G4VEnergyLossProcess.hh>
#include <G4VMultipleScattering.hh>
#include <G4VProcess.hh>
#include <G4Version.hh>

#include "corecel/Assert.hh"
#include "corecel/cont/Range.hh"
#include "corecel/data/HyperslabIndexer.hh"
#include "corecel/io/Logger.hh"
#include "corecel/math/Algorithms.hh"
#include "corecel/math/SoftEqual.hh"
#include "celeritas/UnitTypes.hh"
#include "celeritas/io/ImportUnits.hh"
#include "celeritas/phys/PDGNumber.hh"

#include "GeantModelImporter.hh"

namespace celeritas
{
namespace detail
{
namespace
{
//---------------------------------------------------------------------------//
/*!
 * Convert process type from Geant4 to Celeritas IO.
 */
ImportProcessType to_import_process_type(G4ProcessType g4_process_type)
{
    switch (g4_process_type)
    {
        case G4ProcessType::fNotDefined:
            return ImportProcessType::other;
        case G4ProcessType::fTransportation:
            return ImportProcessType::transportation;
        case G4ProcessType::fElectromagnetic:
            return ImportProcessType::electromagnetic;
        case G4ProcessType::fOptical:
            return ImportProcessType::optical;
        case G4ProcessType::fHadronic:
            return ImportProcessType::hadronic;
        case G4ProcessType::fPhotolepton_hadron:
            return ImportProcessType::photolepton_hadron;
        case G4ProcessType::fDecay:
            return ImportProcessType::decay;
        case G4ProcessType::fGeneral:
            return ImportProcessType::general;
        case G4ProcessType::fParameterisation:
            return ImportProcessType::parameterisation;
        case G4ProcessType::fUserDefined:
            return ImportProcessType::user_defined;
        case G4ProcessType::fParallel:
            return ImportProcessType::parallel;
        case G4ProcessType::fPhonon:
            return ImportProcessType::phonon;
        case G4ProcessType::fUCN:
            return ImportProcessType::ucn;
    }
    CELER_ASSERT_UNREACHABLE();
}

//---------------------------------------------------------------------------//
/*!
 * Safely retrieve the correct process enum from a given string.
 */
ImportProcessClass to_import_process_class(G4VProcess const& process)
{
    auto&& name = process.GetProcessName();
    ImportProcessClass result;
    try
    {
        result = geant_name_to_import_process_class(name);
    }
    catch (celeritas::RuntimeError const&)
    {
        CELER_LOG(warning) << "Encountered unknown process '" << name << "'";
        result = ImportProcessClass::other;
    }
    return result;
}

//---------------------------------------------------------------------------//
/*!
 * Initialize a process result.
 */
ImportProcess
init_process(G4ParticleDefinition const& particle, G4VProcess const& process)
{
    CELER_LOG(debug) << "Saving process '" << process.GetProcessName()
                     << "' for particle " << particle.GetParticleName() << " ("
                     << particle.GetPDGEncoding() << ')';

    ImportProcess result;
    result = {};
    result.process_type = to_import_process_type(process.GetProcessType());
    result.process_class = to_import_process_class(process);
    result.particle_pdg = particle.GetPDGEncoding();

    auto* rest_processes
        = particle.GetProcessManager()->GetAtRestProcessVector();
    CELER_ASSERT(rest_processes);
    result.applies_at_rest
        = rest_processes->contains(const_cast<G4VProcess*>(&process));

    return result;
}

//---------------------------------------------------------------------------//
/*!
 * Get the PDG of a process.
 */
template<class T>
int get_secondary_pdg(T const& process)
{
    static_assert(std::is_base_of<G4VProcess, T>::value,
                  "process must be a G4VProcess");

    // Save secondaries
    if (auto const* secondary = process.SecondaryParticle())
    {
        return secondary->GetPDGEncoding();
    }
    return 0;
}

//---------------------------------------------------------------------------//
/*!
 * Import data from a Geant4 physics table if available.
 */
void assign_table(G4PhysicsTable const* g4table,
                  Array<ImportUnits, 2> units,
                  [[maybe_unused]] ImportProcessClass process_class,
                  ImportPhysicsTable* table,
                  inp::Interpolation interpolation)
{
    if (!g4table)
    {
        // Table isn't present
        return;
    }

    table->x_units = units[0];
    table->y_units = units[1];

    // Save physics vectors, using spline interpolation if enabled and valid
    for (auto const* g4vector : *g4table)
    {
        table->grids.emplace_back(import_physics_log_vector(*g4vector, units));
#if G4VERSION_NUMBER < 1100
        // Hardcode whether the lambda table uses spline for older Geant4
        // versions. Always use spline for lambda, energy loss, range, and msc
        //! \todo Coulomb scattering disables spline when \c isCombined = false
        static std::unordered_set<ImportProcessClass> disable_spline{
            ImportProcessClass::rayleigh};

        if (!disable_spline.count(process_class))
#else
        if (g4vector->GetSpline())
#endif
        {
            table->grids.back().interpolation = interpolation;
        }
    }
    CELER_ENSURE(
        table->grids.size()
        == G4ProductionCutsTable::GetProductionCutsTable()->GetTableSize());
}

template<class T>
bool all_are_assigned(std::vector<T> const& arr)
{
    return std::all_of(arr.begin(), arr.end(), Identity{});
}

//---------------------------------------------------------------------------//
}  // namespace

//---------------------------------------------------------------------------//
/*!
 * Construct with a selected list of tables.
 */
GeantProcessImporter::GeantProcessImporter(
    std::vector<ImportPhysMaterial> const& materials,
    std::vector<ImportElement> const& elements,
    inp::Interpolation interpolation)
    : materials_(materials), elements_(elements), interpolation_(interpolation)
{
    CELER_ENSURE(!materials_.empty());
    CELER_ENSURE(!elements_.empty());
}

//---------------------------------------------------------------------------//
/*!
 * Store EM cross section tables for the current process.
 *
 * Cross sections are calculated in G4EmModelManager::FillLambdaVector by
 * calling G4VEmModel::CrossSection .
 */
ImportProcess
GeantProcessImporter::operator()(G4ParticleDefinition const& particle,
                                 G4VEmProcess const& process)
{
    auto result = init_process(particle, process);
    result.secondary_pdg = get_secondary_pdg(process);

    GeantModelImporter convert_model(materials_,
                                     PDGNumber{result.particle_pdg},
                                     PDGNumber{result.secondary_pdg});
#if G4VERSION_NUMBER < 1100
    for (auto i : celeritas::range(process.GetNumberOfModels()))
#else
    for (auto i : celeritas::range(process.NumberOfModels()))
#endif
    {
        result.models.push_back(convert_model(*process.GetModelByIndex(i)));
        CELER_ASSERT(result.models.back());
    }

    // Save cross section tables if available
    assign_table(process.LambdaTable(),
                 {ImportUnits::mev, ImportUnits::len_inv},
                 result.process_class,
                 &result.lambda,
                 interpolation_);
    assign_table(process.LambdaTablePrim(),
                 {ImportUnits::mev, ImportUnits::len_mev_inv},
                 result.process_class,
                 &result.lambda_prim,
                 interpolation_);
    CELER_ENSURE(result && all_are_assigned(result.models));
    return result;
}

//---------------------------------------------------------------------------//
/*!
 * Store energy loss XS tables to this->result.
 *
 * The following XS tables do not exist in Geant4 v11:
 * - DEDXTableForSubsec()
 * - IonisationTableForSubsec()
 * - SubLambdaTable()
 */
ImportProcess
GeantProcessImporter::operator()(G4ParticleDefinition const& particle,
                                 G4VEnergyLossProcess const& process)
{
    auto result = init_process(particle, process);
    result.secondary_pdg = get_secondary_pdg(process);

    // Note: NumberOfModels/GetModelByIndex is a *not* a virtual method on
    // G4VProcess.

    GeantModelImporter convert_model(materials_,
                                     PDGNumber{result.particle_pdg},
                                     PDGNumber{result.secondary_pdg});
    for (auto i : celeritas::range(process.NumberOfModels()))
    {
        result.models.push_back(convert_model(*process.GetModelByIndex(i)));
    }

    if (process.IsIonisationProcess())
    {
        // The de/dx and range tables created by summing the contribution from
        // each energy loss process are stored in the "ionization process"
        // (which might be ionization or might be another arbitrary energy loss
        // process if there is no ionization in the problem).
        assign_table(process.DEDXTable(),
                     {ImportUnits::mev, ImportUnits::mev_per_len},
                     result.process_class,
                     &result.dedx,
                     interpolation_);
    }

    assign_table(process.LambdaTable(),
                 {ImportUnits::mev, ImportUnits::len_inv},
                 result.process_class,
                 &result.lambda,
                 interpolation_);

    CELER_ENSURE(result && all_are_assigned(result.models));
    return result;
}

//---------------------------------------------------------------------------//
/*!
 * Store multiple scattering XS tables to the process data.
 *
 * Whereas other EM processes combine the model tables into a single process
 * table, MSC keeps them independent.
 *
 * Starting on Geant4 v11, G4MultipleScattering provides \c NumberOfModels() .
 *
 * The cross sections are stored with an extra factor of E^2 multiplied in.
 * They're calculated in G4LossTableBuilder::BuildTableForModel which calls
 * G4VEmModel::Value.
 */
std::vector<ImportMscModel>
GeantProcessImporter::operator()(G4ParticleDefinition const& particle,
                                 G4VMultipleScattering const& process)
{
    std::vector<ImportMscModel> result;
    int primary_pdg = particle.GetPDGEncoding();

    GeantModelImporter convert_model(
        materials_, PDGNumber{primary_pdg}, PDGNumber{});

#if G4VERSION_NUMBER < 1100
    for (auto i : celeritas::range(4))
#else
    for (auto i : celeritas::range(process.NumberOfModels()))
#endif
    {
        if (G4VEmModel* model = process.GetModelByIndex(i))
        {
            CELER_LOG(debug) << "Saving MSC model '" << model->GetName()
                             << "' for particle " << particle.GetParticleName()
                             << " (" << particle.GetPDGEncoding() << ")";

            ImportMscModel imm;
            imm.particle_pdg = primary_pdg;
            try
            {
                imm.model_class
                    = geant_name_to_import_model_class(model->GetName());
            }
            catch (celeritas::RuntimeError const&)
            {
                CELER_LOG(warning) << "Encountered unknown MSC model '"
                                   << model->GetName() << "'";
                imm.model_class = ImportModelClass::other;
            }
            assign_table(model->GetCrossSectionTable(),
                         {ImportUnits::mev, ImportUnits::mev_sq_per_len},
                         ImportProcessClass::size_,
                         &imm.xs_table,
                         interpolation_);
            result.push_back(std::move(imm));
        }
    }

    CELER_ENSURE(all_are_assigned(result));
    return result;
}

//---------------------------------------------------------------------------//
/*!
 * Import a uniform physics vector with the given x, y units.
 *
 * The x-grid is uniform in log(x);
 */
inp::UniformGrid import_physics_log_vector(G4PhysicsVector const& pv,
                                           Array<ImportUnits, 2> units)
{
    // Convert units
    double const x_scaling = native_value_from_clhep(units[0]);
    double const y_scaling = native_value_from_clhep(units[1]);
    auto size = pv.GetVectorLength();

    inp::UniformGrid grid;
    grid.x = {std::log(pv.Energy(0) * x_scaling),
              std::log(pv.Energy(size - 1) * x_scaling)};
    grid.y.resize(size);

    double delta
        = fastpow(pv.Energy(size - 1) / pv.Energy(0), 1.0 / (size - 1));
    for (auto i : range(size))
    {
        // Check that the grid has log spacing
        CELER_ASSERT(i == 0
                     || soft_equal(delta, pv.Energy(i) / pv.Energy(i - 1)));
        grid.y[i] = pv[i] * y_scaling;
    }
    CELER_ENSURE(grid);
    return grid;
}

//---------------------------------------------------------------------------//
/*!
 * Import a generic physics vector with the given x, y units.
 */
inp::Grid
import_physics_vector(G4PhysicsVector const& pv, Array<ImportUnits, 2> units)
{
    // Convert units
    double const x_scaling = native_value_from_clhep(units[0]);
    double const y_scaling = native_value_from_clhep(units[1]);

    inp::Grid grid;
    grid.x.resize(pv.GetVectorLength());
    grid.y.resize(grid.x.size());

    for (auto i : range(pv.GetVectorLength()))
    {
        grid.x[i] = pv.Energy(i) * x_scaling;
        grid.y[i] = pv[i] * y_scaling;
    }
    CELER_ENSURE(grid);
    return grid;
}

//---------------------------------------------------------------------------//
/*!
 * Import a 2D physics vector.
 *
 * \note In Geant4 the values are stored as a vector of vectors indexed as
 * [y][x]. Because the Celeritas \c TwodGridCalculator and \c
 * TwodSubgridCalculator expect the y grid values to be on the inner dimension,
 * the table is inverted during import so that the x and y grids are swapped.
 */
inp::TwodGrid import_physics_2dvector(G4Physics2DVector const& pv,
                                      Array<ImportUnits, 3> units)
{
    // Convert units
    double const x_scaling = native_value_from_clhep(units[0]);
    double const y_scaling = native_value_from_clhep(units[1]);
    double const v_scaling = native_value_from_clhep(units[2]);

    Array<size_type, 2> dims{static_cast<size_type>(pv.GetLengthY()),
                             static_cast<size_type>(pv.GetLengthX())};
    HyperslabIndexer<2> index(dims);

    inp::TwodGrid grid;
    grid.x.resize(dims[0]);
    grid.y.resize(dims[1]);
    grid.value.resize(dims[0] * dims[1]);

    for (auto i : range(dims[0]))
    {
        grid.x[i] = pv.GetY(i) * y_scaling;
        for (auto j : range(dims[1]))
        {
            grid.y[j] = pv.GetX(j) * x_scaling;
            grid.value[index(i, j)] = pv.GetValue(j, i) * v_scaling;
        }
    }
    CELER_ENSURE(grid);
    return grid;
}

//---------------------------------------------------------------------------//
}  // namespace detail
}  // namespace celeritas
