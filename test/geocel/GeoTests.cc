//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file geocel/GeoTests.cc
//---------------------------------------------------------------------------//
#include "GeoTests.hh"

#include <string_view>

#include "corecel/cont/Range.hh"
#include "corecel/math/ArrayOperators.hh"
#include "corecel/math/Turn.hh"
#include "corecel/sys/Version.hh"

#include "GenericGeoTestInterface.hh"
#include "TestMacros.hh"
#include "UnitUtils.hh"

namespace celeritas
{
namespace test
{
namespace
{
//---------------------------------------------------------------------------//
auto const vecgeom_version = celeritas::Version::from_string(
    CELERITAS_USE_VECGEOM ? cmake::vecgeom_version : "0.0.0");
auto const geant4_version = celeritas::Version::from_string(
    CELERITAS_USE_GEANT4 ? cmake::geant4_version : "0.0.0");

BoundingBox<> calc_expected_bbox(std::string_view geo_type, Real3 lo, Real3 hi)
{
    if (geo_type == "VecGeom")
    {
        // VecGeom bumps bounding boxes
        lo -= 0.001;
        hi += 0.001;
    }
    return BoundingBox<>{lo, hi};
}

//---------------------------------------------------------------------------//
}  // namespace

//---------------------------------------------------------------------------//
// CMS EE
//---------------------------------------------------------------------------//
//! Test geometry accessors
void CmsEeBackDeeGeoTest::test_accessors() const
{
    auto const& geo = *test_->geometry_interface();
    EXPECT_EQ(3, geo.max_depth());

    auto expected_bbox = calc_expected_bbox(
        test_->geometry_type(), {0., -177.5, 359.5}, {177.5, 177.5, 399.6});
    auto const& bbox = geo.bbox();
    EXPECT_VEC_NEAR(expected_bbox.lower(), to_cm(bbox.lower()), 1e-10);
    EXPECT_VEC_SOFT_EQ(expected_bbox.upper(), to_cm(bbox.upper()));

    static char const* const expected_vol_labels[] = {
        "EEBackPlate",
        "EESRing",
        "EEBackQuad",
        "EEBackDee",
        "EEBackQuad_refl",
        "EEBackPlate_refl",
        "EESRing_refl",
    };
    EXPECT_VEC_EQ(expected_vol_labels, test_->get_volume_labels());

    static char const* const expected_vol_inst_labels[] = {
        "EEBackPlate@0",
        "EESRing@0",
        "EEBackQuad@0",
        "EEBackPlate@1",
        "EESRing@1",
        "EEBackQuad@1",
        "EEBackDee_PV",
    };
    EXPECT_VEC_EQ(expected_vol_inst_labels,
                  test_->get_volume_instance_labels());

    if (test_->g4world())
    {
        EXPECT_VEC_EQ(expected_vol_inst_labels, test_->get_g4pv_labels());
    }
}

//---------------------------------------------------------------------------//
void CmsEeBackDeeGeoTest::test_trace() const
{
    // Surface VecGeom needs lower safety tolerance
    real_type const safety_tol = test_->safety_tol();

    {
        SCOPED_TRACE("+z top");
        auto result = test_->track({50, 0.1, 360.1}, {0, 0, 1});
        static char const* const expected_volumes[]
            = {"EEBackPlate", "EEBackQuad"};
        EXPECT_VEC_EQ(expected_volumes, result.volumes);
        static char const* const expected_volume_instances[]
            = {"EEBackPlate", "EEBackQuad"};
        EXPECT_VEC_EQ(expected_volume_instances, result.volume_instances);
        static real_type const expected_distances[] = {5.4, 34.1};
        EXPECT_VEC_SOFT_EQ(expected_distances, result.distances);
        static real_type const expected_hw_safety[] = {0.1, 0.1};
        EXPECT_VEC_NEAR(
            expected_hw_safety, result.halfway_safeties, safety_tol);
    }
    {
        SCOPED_TRACE("+z bottom");
        auto result = test_->track({50, -0.1, 360.1}, {0, 0, 1});
        static char const* const expected_volumes[]
            = {"EEBackPlate_refl", "EEBackQuad_refl"};
        EXPECT_VEC_EQ(expected_volumes, result.volumes);
        static char const* const expected_volume_instances[]
            = {"EEBackPlate", "EEBackQuad"};
        EXPECT_VEC_EQ(expected_volume_instances, result.volume_instances);
        static real_type const expected_distances[] = {5.4, 34.1};
        EXPECT_VEC_SOFT_EQ(expected_distances, result.distances);
        static real_type const expected_hw_safety[]
            = {0.099999999999956, 0.099999999999953};
        EXPECT_VEC_NEAR(
            expected_hw_safety, result.halfway_safeties, safety_tol);
    }
}

//---------------------------------------------------------------------------//
// CMSE
//---------------------------------------------------------------------------//

void CmseGeoTest::test_trace() const
{
    // Surface VecGeom needs lower safety tolerance
    real_type const safety_tol = test_->safety_tol();

    // clang-format off
    {
        SCOPED_TRACE("Center +z");
        auto result = test_->track({0, 0, -4000}, {0, 0, 1});
        static char const* const expected_volumes[] = {"CMStoZDC", "BEAM3",
            "BEAM2", "BEAM1", "BEAM", "BEAM", "BEAM1", "BEAM2", "BEAM3",
            "CMStoZDC", "CMSE", "ZDC", "CMSE", "ZDCtoFP420", "CMSE"};
        EXPECT_VEC_EQ(expected_volumes, result.volumes);
        static real_type const expected_distances[] = {1300, 1096.95, 549.15,
            403.9, 650, 650, 403.9, 549.15, 1096.95, 11200, 9.9999999999992,
            180, 910, 24000, 6000};
        EXPECT_VEC_SOFT_EQ(expected_distances, result.distances);
        if (test_->geometry_type() == "VecGeom" && CELERITAS_VECGEOM_SURFACE)
        {
            // Surface vecgeom underestimates some safety near internal
            // boundaries
            static real_type const expected_hw_safety[] = {100, 2.1499999999997,
                9.62498950958252, 13.023518051922, 6.95, 6.95, 13.023518051922,
                9.62498950958252, 2.15, 100, 5, 8, 100, 100, 100};
            EXPECT_VEC_NEAR(expected_hw_safety, result.halfway_safeties,
                            safety_tol);
        }
        else
        {
            static real_type const expected_hw_safety[] = {100, 2.1499999999997,
                10.3027302206744, 13.023518051922, 6.95, 6.95, 13.023518051922,
                10.3027302206745, 2.15, 100, 5, 8, 100, 100, 100};
            EXPECT_VEC_NEAR(expected_hw_safety, result.halfway_safeties,
                            safety_tol);
        }
    }
    {
        SCOPED_TRACE("Offset +z");
        auto result = test_->track({30, 30, -4000}, {0, 0, 1});
        static char const* const expected_volumes[] = {"CMStoZDC", "OQUA",
            "VCAL", "OQUA", "CMSE", "TotemT1", "CMSE", "MUON", "CALO",
            "Tracker", "CALO", "MUON", "CMSE", "TotemT1", "CMSE", "OQUA",
            "VCAL", "OQUA", "CMStoZDC", "CMSE", "ZDCtoFP420", "CMSE"};
        EXPECT_VEC_EQ(expected_volumes, result.volumes);
        static real_type const expected_distances[] = {1300, 1419.95, 165.1,
            28.95, 36, 300.1, 94.858988388759, 100.94101161124, 260.9, 586.4,
            260.9, 100.94101161124, 94.858988388759, 300.1, 36, 28.95, 165.1,
            1419.95, 11200, 1100, 24000, 6000};
        EXPECT_VEC_SOFT_EQ(expected_distances, result.distances);
        static real_type const expected_hw_safety[] = {57.573593128807,
            40.276406871193, 29.931406871193, 14.475, 18, 28.702447147997,
            29.363145173005, 32.665765921596, 34.260814069425, 39.926406871193,
            34.260814069425, 32.665765921596, 29.363145173005, 28.702447147997,
            18, 14.475, 29.931406871193, 40.276406871193, 57.573593128807,
            57.573593128807, 57.573593128807, 57.573593128807};
        EXPECT_VEC_NEAR(expected_hw_safety, result.halfway_safeties, safety_tol);
    }
    {
        SCOPED_TRACE("Across muon");
        auto result = test_->track({-1000, 0, -48.5}, {1, 0, 0});
        static char const* const expected_volumes[] = {"OCMS", "MUON", "CALO",
            "Tracker", "CMSE", "BEAM", "CMSE", "Tracker", "CALO", "MUON",
            "OCMS"};
        EXPECT_VEC_EQ(expected_volumes, result.volumes);
        static real_type const expected_distances[] = {170, 535, 171.7, 120.8,
            0.15673306650246, 4.6865338669951, 0.15673306650246, 120.8, 171.7,
            535, 920};
        EXPECT_VEC_SOFT_EQ(expected_distances, result.distances);
        static real_type const expected_hw_safety[] = {85, 267.5, 85.85,
            60.4, 0.078366388350241, 2.343262600759, 0.078366388350241,
            60.4, 85.85, 267.5, 460};
        EXPECT_VEC_NEAR(expected_hw_safety, result.halfway_safeties, safety_tol);
    }
    {
        SCOPED_TRACE("Differs between G4/VG");
        auto result = test_->track({0, 0, 1328.0}, {1, 0, 0});
        static char const* const expected_volumes[] = {"BEAM2", "OQUA", "CMSE",
            "OCMS"};
        EXPECT_VEC_EQ(expected_volumes, result.volumes);
        static real_type const expected_distances[] = {12.495, 287.505, 530,
            920};
        EXPECT_VEC_SOFT_EQ(expected_distances, result.distances);
        static real_type const expected_hw_safety[] = {6.2475, 47.95, 242, 460};
        EXPECT_VEC_NEAR(expected_hw_safety, result.halfway_safeties, safety_tol);
    }
    // clang-format on
}

//---------------------------------------------------------------------------//
//! Test geometry accessors
void FourLevelsGeoTest::test_accessors() const
{
    auto const& geo = *test_->geometry_interface();
    EXPECT_EQ(4, geo.max_depth());

    auto expected_bbox = calc_expected_bbox(
        test_->geometry_type(), {-24., -24., -24.}, {24., 24., 24.});
    auto const& bbox = geo.bbox();
    EXPECT_VEC_SOFT_EQ(expected_bbox.lower(), to_cm(bbox.lower()));
    EXPECT_VEC_SOFT_EQ(expected_bbox.upper(), to_cm(bbox.upper()));

    static char const* const expected_vol_labels[] = {
        "Shape2",
        "Shape1",
        "Envelope",
        "World",
    };
    EXPECT_VEC_EQ(expected_vol_labels, test_->get_volume_labels());

    static char const* const expected_vol_inst_labels[] = {
        "Shape2",
        "Shape1",
        "env1",
        "env2",
        "env3",
        "env4",
        "env5",
        "env6",
        "env7",
        "env8",
        "World_PV",
    };
    EXPECT_VEC_EQ(expected_vol_inst_labels,
                  test_->get_volume_instance_labels());

    if (test_->g4world())
    {
        EXPECT_VEC_EQ(expected_vol_inst_labels, test_->get_g4pv_labels());
    }
}

//---------------------------------------------------------------------------//
void FourLevelsGeoTest::test_trace() const
{
    // Surface VecGeom needs lower safety tolerance
    real_type const safety_tol = test_->safety_tol();

    {
        SCOPED_TRACE("Rightward");
        auto result = test_->track({-10, -10, -10}, {1, 0, 0});

        static char const* const expected_volumes[] = {
            "Shape2",
            "Shape1",
            "Envelope",
            "World",
            "Envelope",
            "Shape1",
            "Shape2",
            "Shape1",
            "Envelope",
            "World",
        };
        EXPECT_VEC_EQ(expected_volumes, result.volumes);
        static real_type const expected_distances[]
            = {5, 1, 1, 6, 1, 1, 10, 1, 1, 7};
        EXPECT_VEC_SOFT_EQ(expected_distances, result.distances);
        static real_type const expected_hw_safety[]
            = {2.5, 0.5, 0.5, 3, 0.5, 0.5, 5, 0.5, 0.5, 3.5};
        EXPECT_VEC_NEAR(
            expected_hw_safety, result.halfway_safeties, safety_tol);
    }
    {
        SCOPED_TRACE("From just inside outside edge");
        auto result = test_->track({-24 + 0.001, 10., 10.}, {1, 0, 0});

        static char const* const expected_volumes[] = {
            "World",
            "Envelope",
            "Shape1",
            "Shape2",
            "Shape1",
            "Envelope",
            "World",
            "Envelope",
            "Shape1",
            "Shape2",
            "Shape1",
            "Envelope",
            "World",
        };
        EXPECT_VEC_EQ(expected_volumes, result.volumes);
        static real_type const expected_distances[]
            = {7 - 0.001, 1, 1, 10, 1, 1, 6, 1, 1, 10, 1, 1, 7};
        EXPECT_VEC_SOFT_EQ(expected_distances, result.distances);
        static real_type const expected_hw_safety[]
            = {3.4995, 0.5, 0.5, 5, 0.5, 0.5, 3, 0.5, 0.5, 5, 0.5, 0.5, 3.5};
        EXPECT_VEC_NEAR(
            expected_hw_safety, result.halfway_safeties, safety_tol);
    }
    {
        SCOPED_TRACE("Leaving world");
        auto result = test_->track({-10, 10, 10}, {0, 1, 0});

        static char const* const expected_volumes[]
            = {"Shape2", "Shape1", "Envelope", "World"};
        EXPECT_VEC_EQ(expected_volumes, result.volumes);
        static real_type const expected_distances[] = {5, 1, 2, 6};
        EXPECT_VEC_SOFT_EQ(expected_distances, result.distances);
        static real_type const expected_hw_safety[] = {2.5, 0.5, 1, 3};
        EXPECT_VEC_NEAR(
            expected_hw_safety, result.halfway_safeties, safety_tol);
    }
    {
        SCOPED_TRACE("Upward");
        auto result = test_->track({-10, 10, 10}, {0, 0, 1});

        static char const* const expected_volumes[]
            = {"Shape2", "Shape1", "Envelope", "World"};
        EXPECT_VEC_EQ(expected_volumes, result.volumes);
        static real_type const expected_distances[] = {5, 1, 3, 5};
        EXPECT_VEC_SOFT_EQ(expected_distances, result.distances);
        static real_type const expected_hw_safety[] = {2.5, 0.5, 1.5, 2.5};
        EXPECT_VEC_NEAR(
            expected_hw_safety, result.halfway_safeties, safety_tol);
    }
}

//---------------------------------------------------------------------------//
//! Test geometry accessors
void MultiLevelGeoTest::test_accessors() const
{
    auto const& geo = *test_->geometry_interface();
    EXPECT_EQ(3, geo.max_depth());

    static char const* const expected_vol_labels[] = {
        "sph",
        "tri",
        "box",
        "world",
        "box_refl",
        "sph_refl",
        "tri_refl",
    };
    EXPECT_VEC_EQ(expected_vol_labels, test_->get_volume_labels());

    static char const* const expected_vol_inst_labels[] = {
        "boxsph1@0",
        "boxsph2@0",
        "boxtri@0",
        "topbox1",
        "topsph1",
        "topbox2",
        "topbox3",
        "boxsph1@1",
        "boxsph2@1",
        "boxtri@1",
        "topbox4",
        "world_PV",
    };
    EXPECT_VEC_EQ(expected_vol_inst_labels,
                  test_->get_volume_instance_labels());

    if (test_->g4world())
    {
        EXPECT_VEC_EQ(expected_vol_inst_labels, test_->get_g4pv_labels());
    }
}

//---------------------------------------------------------------------------//
void MultiLevelGeoTest::test_trace() const
{
    // Surface VecGeom needs lower safety tolerance
    real_type const safety_tol = test_->safety_tol();

    {
        SCOPED_TRACE("high");
        auto result = test_->track({-19.9, 7.5, 0}, {1, 0, 0});
        static char const* const expected_volumes[] = {
            "world",
            "box",
            "sph",
            "box",
            "tri",
            "box",
            "world",
            "box",
            "sph",
            "box",
            "tri",
            "box",
            "world",
        };
        EXPECT_VEC_EQ(expected_volumes, result.volumes);
        static char const* const expected_volume_instances[] = {
            "world_PV",
            "topbox2",
            "boxsph2",
            "topbox2",
            "boxtri",
            "topbox2",
            "world_PV",
            "topbox1",
            "boxsph2",
            "topbox1",
            "boxtri",
            "topbox1",
            "world_PV",
        };
        EXPECT_VEC_EQ(expected_volume_instances, result.volume_instances);
        static real_type const expected_distances[] = {
            2.4,
            3,
            4,
            1.8452994616207,
            2.3094010767585,
            3.8452994616207,
            5,
            3,
            4,
            1.8452994616207,
            2.3094010767585,
            3.8452994616207,
            6.5,
        };
        EXPECT_VEC_SOFT_EQ(expected_distances, result.distances);
        static real_type const expected_hw_safety[] = {
            1.2,
            1.5,
            2,
            0.79903810567666,
            1,
            1.6650635094611,
            2.5,
            1.5,
            2,
            0.79903810567666,
            1,
            1.6650635094611,
            3.25,
        };
        EXPECT_VEC_NEAR(
            expected_hw_safety, result.halfway_safeties, safety_tol);
    }
    {
        SCOPED_TRACE("low");
        auto result = test_->track({-19.9, -7.5, 0}, {1, 0, 0});
        static char const* const expected_volumes[] = {
            "world",
            "box",
            "sph",
            "box",
            "world",
            "box_refl",
            "sph_refl",
            "box_refl",
            "tri_refl",
            "box_refl",
            "world",
        };
        EXPECT_VEC_EQ(expected_volumes, result.volumes);
        static char const* const expected_volume_instances[] = {
            "world_PV",
            "topbox3",
            "boxsph2",
            "topbox3",
            "world_PV",
            "topbox4",
            "boxsph2",
            "topbox4",
            "boxtri",
            "topbox4",
            "world_PV",
        };
        EXPECT_VEC_EQ(expected_volume_instances, result.volume_instances);
        static real_type const expected_distances[] = {
            2.4,
            3,
            4,
            8,
            5,
            3,
            4,
            1.8452994616207,
            2.3094010767585,
            3.8452994616207,
            6.5,
        };
        EXPECT_VEC_SOFT_EQ(expected_distances, result.distances);
        static real_type const expected_hw_safety[] = {
            1.2,
            1.5,
            2,
            3.0990195135928,
            2.5,
            1.5,
            2,
            0.79903810567666,
            1,
            1.6650635094611,
            3.25,
        };
        EXPECT_VEC_NEAR(
            expected_hw_safety, result.halfway_safeties, safety_tol);
    }
}

//---------------------------------------------------------------------------//
// REPLICA
//---------------------------------------------------------------------------//

void ReplicaGeoTest::test_trace() const
{
    {
        SCOPED_TRACE("Center +z");
        auto result = test_->track({0, 0.5, -990}, {0, 0, 1});
        GenericGeoTrackingResult ref;
        ref.volumes = {
            "world",    "firstArm",   "hodoscope1", "firstArm",
            "chamber1", "wirePlane1", "chamber1",   "firstArm",
            "chamber1", "wirePlane1", "chamber1",   "firstArm",
            "chamber1", "wirePlane1", "chamber1",   "firstArm",
            "chamber1", "wirePlane1", "chamber1",   "firstArm",
            "chamber1", "wirePlane1", "chamber1",   "firstArm",
            "world",    "magnetic",   "world",      "secondArm",
            "chamber2", "wirePlane2", "chamber2",   "secondArm",
            "world",
        };
        ref.volume_instances = {
            "world_PV", "firstArm",   "hodoscope1", "firstArm",
            "chamber1", "wirePlane1", "chamber1",   "firstArm",
            "chamber1", "wirePlane1", "chamber1",   "firstArm",
            "chamber1", "wirePlane1", "chamber1",   "firstArm",
            "chamber1", "wirePlane1", "chamber1",   "firstArm",
            "chamber1", "wirePlane1", "chamber1",   "firstArm",
            "world_PV", "magnetic",   "world_PV",   "fSecondArmPhys",
            "chamber2", "wirePlane2", "chamber2",   "fSecondArmPhys",
            "world_PV",
        };
        ref.distances = {
            190,
            149.5,
            1,
            48.5,
            0.99,
            0.019999999999991,
            0.99000000000001,
            48,
            0.99,
            0.019999999999991,
            0.99000000000001,
            48,
            0.99,
            0.019999999999991,
            0.99000000000001,
            48,
            0.99,
            0.019999999999991,
            0.99000000000001,
            48,
            0.99,
            0.019999999999991,
            0.99000000000001,
            199,
            100,
            200,
            73.205080756887,
            114.31535329955,
            1.1431535329955,
            0.023094010767563,
            1.1431535329954,
            110.17016486681,
            600,
        };
        ref.halfway_safeties = {
            95,
            74.75,
            0.5,
            24.25,
            0.495,
            0.01,
            0.495,
            24,
            0.495,
            0.01,
            0.495,
            24,
            0.495,
            0.01,
            0.495,
            24,
            0.495,
            0.01,
            0.495,
            24,
            0.495,
            0.01,
            0.495,
            99.5,
            50,
            99.5,
            31.698729810778,
            49.5,
            0.49499999999996,
            0.0099999999999922,
            0.49499999999997,
            22.457458783298,
            150,
        };
        ref.bumps = {};
        auto tol = GenericGeoTrackingTolerance::from_test(*test_);
        tol.distance *= 10;  // 2e-12 diff between g4, vg
        EXPECT_RESULT_NEAR(ref, result, tol);
    }
    {
        SCOPED_TRACE("Second arm");
        Real3 dir{0, 0, 0};
        sincos(Turn{-30.0 / 360.}, &dir[0], &dir[2]);
        auto result = test_->track({0.125, 0.5, 0.0625}, dir);

        GenericGeoTrackingResult ref;
        ref.volumes = {
            "magnetic",     "world",       "secondArm",    "chamber2",
            "wirePlane2",   "chamber2",    "secondArm",    "chamber2",
            "wirePlane2",   "chamber2",    "secondArm",    "chamber2",
            "wirePlane2",   "chamber2",    "secondArm",    "chamber2",
            "wirePlane2",   "chamber2",    "secondArm",    "chamber2",
            "wirePlane2",   "chamber2",    "secondArm",    "hodoscope2",
            "secondArm",    "cell",        "secondArm",    "HadCalLayer",
            "HadCalScinti", "HadCalLayer", "HadCalScinti", "HadCalLayer",
            "HadCalScinti", "HadCalLayer", "HadCalScinti", "HadCalLayer",
            "HadCalScinti", "HadCalLayer", "HadCalScinti", "HadCalLayer",
            "HadCalScinti", "HadCalLayer", "HadCalScinti", "HadCalLayer",
            "HadCalScinti", "HadCalLayer", "HadCalScinti", "HadCalLayer",
            "HadCalScinti", "HadCalLayer", "HadCalScinti", "HadCalLayer",
            "HadCalScinti", "HadCalLayer", "HadCalScinti", "HadCalLayer",
            "HadCalScinti", "HadCalLayer", "HadCalScinti", "HadCalLayer",
            "HadCalScinti", "HadCalLayer", "HadCalScinti", "HadCalLayer",
            "HadCalScinti", "HadCalLayer", "HadCalScinti", "world",
        };
        ref.volume_instances = {
            "magnetic",          "world_PV",          "fSecondArmPhys",
            "chamber2",          "wirePlane2",        "chamber2",
            "fSecondArmPhys",    "chamber2",          "wirePlane2",
            "chamber2",          "fSecondArmPhys",    "chamber2",
            "wirePlane2",        "chamber2",          "fSecondArmPhys",
            "chamber2",          "wirePlane2",        "chamber2",
            "fSecondArmPhys",    "chamber2",          "wirePlane2",
            "chamber2",          "fSecondArmPhys",    "hodoscope2",
            "fSecondArmPhys",    "cell_param@42",     "fSecondArmPhys",
            "HadCalLayer_PV@0",  "HadCalScinti",      "HadCalLayer_PV@1",
            "HadCalScinti",      "HadCalLayer_PV@2",  "HadCalScinti",
            "HadCalLayer_PV@3",  "HadCalScinti",      "HadCalLayer_PV@4",
            "HadCalScinti",      "HadCalLayer_PV@5",  "HadCalScinti",
            "HadCalLayer_PV@6",  "HadCalScinti",      "HadCalLayer_PV@7",
            "HadCalScinti",      "HadCalLayer_PV@8",  "HadCalScinti",
            "HadCalLayer_PV@9",  "HadCalScinti",      "HadCalLayer_PV@10",
            "HadCalScinti",      "HadCalLayer_PV@11", "HadCalScinti",
            "HadCalLayer_PV@12", "HadCalScinti",      "HadCalLayer_PV@13",
            "HadCalScinti",      "HadCalLayer_PV@14", "HadCalScinti",
            "HadCalLayer_PV@15", "HadCalScinti",      "HadCalLayer_PV@16",
            "HadCalScinti",      "HadCalLayer_PV@17", "HadCalScinti",
            "HadCalLayer_PV@18", "HadCalScinti",      "HadCalLayer_PV@19",
            "HadCalScinti",      "world_PV",
        };
        ref.distances = {
            100.00827610654,
            50.000097305727,
            99,
            0.98999999999997,
            0.020000000000008,
            0.99,
            48,
            0.99000000000001,
            0.019999999999994,
            0.99000000000001,
            48,
            0.99000000000001,
            0.019999999999994,
            0.99000000000001,
            48,
            0.98999999999995,
            0.019999999999994,
            0.99000000000001,
            48,
            0.99000000000001,
            0.019999999999994,
            0.99000000000001,
            48.5,
            1,
            184.5,
            30,
            35,
            4,
            0.99999999999999,
            4,
            0.99999999999999,
            4.0000000000001,
            0.99999999999999,
            4.0000000000001,
            0.99999999999999,
            4.0000000000001,
            0.99999999999999,
            4.0000000000001,
            0.99999999999999,
            4.0000000000001,
            0.99999999999999,
            4,
            0.99999999999999,
            4.0000000000001,
            0.99999999999999,
            4.0000000000001,
            0.99999999999999,
            4.0000000000001,
            0.99999999999999,
            4.0000000000001,
            0.99999999999999,
            4.0000000000001,
            0.99999999999999,
            4,
            0.99999999999999,
            4.0000000000001,
            0.99999999999999,
            4.0000000000001,
            0.99999999999999,
            4.0000000000001,
            0.99999999999999,
            4.0000000000001,
            0.99999999999999,
            4.0000000000001,
            0.99999999999999,
            4,
            0.99999999999999,
            304.61999618334,
        };
        ref.halfway_safeties = {
            50.004040731528,
            25.000029191686,
            49.5,
            0.49499999999999,
            0.0099999999999814,
            0.49499999999999,
            24,
            0.49499999999997,
            0.0099999999999798,
            0.49499999999997,
            24,
            0.49499999999997,
            0.0099999999999798,
            0.49499999999997,
            24,
            0.49499999999998,
            0.0099999999999798,
            0.49499999999997,
            24,
            0.49499999999997,
            0.0099999999999798,
            0.49499999999997,
            24.25,
            0.49999999999999,
            92.25,
            0.13950317547316,
            17.5,
            0.13950317547311,
            0.13950317547314,
            0.13950317547311,
            0.13950317547314,
            0.13950317547311,
            0.13950317547314,
            0.13950317547311,
            0.13950317547314,
            0.13950317547311,
            0.13950317547314,
            0.13950317547311,
            0.13950317547314,
            0.13950317547311,
            0.13950317547314,
            0.13950317547311,
            0.13950317547314,
            0.13950317547311,
            0.13950317547314,
            0.13950317547311,
            0.13950317547314,
            0.13950317547311,
            0.13950317547314,
            0.13950317547311,
            0.13950317547314,
            0.13950317547311,
            0.13950317547314,
            0.13950317547311,
            0.13950317547314,
            0.13950317547311,
            0.13950317547314,
            0.13950317547311,
            0.13950317547314,
            0.13950317547311,
            0.13950317547314,
            0.13950317547311,
            0.13950317547314,
            0.13950317547311,
            0.13950317547314,
            0.13950317547311,
            0.13950317547314,
            131.90432759775,
        };
        ref.bumps = {};
        auto tol = GenericGeoTrackingTolerance::from_test(*test_);
        tol.distance *= 10;  // 2e-12 diff between g4, vg
        EXPECT_RESULT_NEAR(ref, result, tol);
    }
}

//---------------------------------------------------------------------------//
void ReplicaGeoTest::test_volume_stack() const
{
    {
        auto result = test_->volume_stack({-400, 0.1, 650});
        GenericGeoVolumeStackResult ref;
        ref.volume_instances = {
            "world_PV",
            "fSecondArmPhys",
            "HadCalorimeter",
            "HadCalColumn_PV",
            "HadCalCell_PV",
            "HadCalLayer_PV",
        };
        ref.replicas = {-1, -1, -1, 4, 1, 2};
        EXPECT_RESULT_EQ(ref, result);
    }
    {
        // Geant4 gets stuck here (it's close to a boundary)
        auto result = test_->volume_stack({-342.5, 0.1, 593.22740159234});
        GenericGeoVolumeStackResult ref;
        ref.volume_instances = {
            "world_PV",
            "fSecondArmPhys",
        };
        ref.replicas = {-1, -1};
        if (test_->geometry_type() == "Geant4"
            || (test_->geometry_type() == "VecGeom"
                && CELERITAS_VECGEOM_SURFACE))
        {
            ref.volume_instances.insert(ref.volume_instances.end(),
                                        {"EMcalorimeter", "cell_param"});
            ref.replicas.insert(ref.replicas.end(), {-1, 42});
        }
        EXPECT_RESULT_EQ(ref, result);
    }
    {
        // A bit further along from the stuck point
        auto result = test_->volume_stack({-343, 0.1, 596});
        GenericGeoVolumeStackResult ref;
        ref.volume_instances = {
            "world_PV",
            "fSecondArmPhys",
            "EMcalorimeter",
            "cell_param",
        };
        ref.replicas = {-1, -1, -1, 42};
        EXPECT_RESULT_EQ(ref, result);
    }
}

//---------------------------------------------------------------------------//
// SOLIDS
//---------------------------------------------------------------------------//
//! Test geometry accessors
void SolidsGeoTest::test_accessors() const
{
    auto const& geo = *test_->geometry_interface();
    EXPECT_EQ(2, geo.max_depth());

    auto expected_bbox = calc_expected_bbox(
        test_->geometry_type(), {-600., -300., -75.}, {600., 300., 75.});
    auto const& bbox = geo.bbox();
    EXPECT_VEC_SOFT_EQ(expected_bbox.lower(), to_cm(bbox.lower()));
    EXPECT_VEC_SOFT_EQ(expected_bbox.upper(), to_cm(bbox.upper()));

    static char const* const expected_vol_labels[] = {
        "box500",   "cone1",     "para1",      "sphere1",     "parabol1",
        "trap1",    "trd1",      "trd2",       "trd3_refl@1", "tube100",
        "boolean1", "polycone1", "genPocone1", "ellipsoid1",  "tetrah1",
        "orb1",     "polyhedr1", "hype1",      "elltube1",    "ellcone1",
        "arb8b",    "arb8a",     "xtru1",      "World",       "trd3_refl@0",
    };
    EXPECT_VEC_EQ(expected_vol_labels, test_->get_volume_labels());

    static char const* const expected_vol_inst_labels[] = {
        "box500_PV",   "cone1_PV",     "para1_PV",      "sphere1_PV",
        "parabol1_PV", "trap1_PV",     "trd1_PV",       "reflNormal",
        "reflected@0", "reflected@1",  "tube100_PV",    "boolean1_PV",
        "orb1_PV",     "polycone1_PV", "hype1_PV",      "polyhedr1_PV",
        "tetrah1_PV",  "arb8a_PV",     "arb8b_PV",      "ellipsoid1_PV",
        "elltube1_PV", "ellcone1_PV",  "genPocone1_PV", "xtru1_PV",
        "World_PV",
    };
    EXPECT_VEC_EQ(expected_vol_inst_labels,
                  test_->get_volume_instance_labels());

    if (test_->g4world())
    {
        EXPECT_VEC_EQ(expected_vol_inst_labels, test_->get_g4pv_labels());
    }
}

//---------------------------------------------------------------------------//
void SolidsGeoTest::test_trace() const
{
    // VecGeom adds bumps through boolean volumes
    real_type const bool_tol
        = (test_->geometry_type() == "VecGeom" ? 1e-7 : 1e-12);

    {
        SCOPED_TRACE("Upper +x");
        auto result = test_->track({-575, 125, 0.5}, {1, 0, 0});
        static char const* const expected_volumes[] = {
            "World",     "hype1",    "World",    "hype1",     "World",
            "para1",     "World",    "tube100",  "World",     "boolean1",
            "World",     "boolean1", "World",    "polyhedr1", "World",
            "polyhedr1", "World",    "ellcone1", "World",
        };
        EXPECT_VEC_EQ(expected_volumes, result.volumes);
        static char const* const expected_volume_instances[] = {
            "World_PV", "hype1_PV",     "World_PV", "hype1_PV",
            "World_PV", "para1_PV",     "World_PV", "tube100_PV",
            "World_PV", "boolean1_PV",  "World_PV", "boolean1_PV",
            "World_PV", "polyhedr1_PV", "World_PV", "polyhedr1_PV",
            "World_PV", "ellcone1_PV",  "World_PV",
        };
        EXPECT_VEC_EQ(expected_volume_instances, result.volume_instances);
        static real_type const expected_distances[] = {
            175.99886751197,
            4.0003045405969,
            40.001655894868,
            4.0003045405969,
            71.165534178636,
            60,
            74.833333333333,
            4,
            116,
            12.5,
            20,
            17.5,
            191.92750632007,
            26.020708495029,
            14.10357036981,
            26.020708495029,
            86.977506320066,
            9.8999999999999,
            220.05,
        };
        EXPECT_VEC_NEAR(expected_distances, result.distances, bool_tol);
        std::vector<real_type> expected_hw_safety = {
            74.5,
            1.9994549442736,
            20.000718268824,
            1.9994549442736,
            29.606651830022,
            24.961508830135,
            31.132548513141,
            2,
            42,
            6.25,
            9.5,
            8.75,
            74.5,
            0,
            6.5120702274482,
            11.947932358344,
            43.183743254945,
            4.9254340915394,
            74.5,
        };
        if (test_->geometry_type() == "VecGeom")
        {
            // v1.2.10: unknown differences
            expected_hw_safety[1] = 1.99361986757606;
            expected_hw_safety[3] = 1.99361986757606;
        }
        EXPECT_VEC_NEAR(expected_hw_safety, result.halfway_safeties, bool_tol);
    }
    {
        SCOPED_TRACE("Center -x");
        auto result = test_->track({575, 0, 0.50}, {-1, 0, 0});
        static char const* const expected_volumes[] = {
            "World", "ellipsoid1", "World", "polycone1", "World", "polycone1",
            "World", "sphere1",    "World", "box500",    "World", "cone1",
            "World", "trd1",       "World", "parabol1",  "World", "trd2",
            "World", "xtru1",      "World",
        };
        EXPECT_VEC_EQ(expected_volumes, result.volumes);
        static char const* const expected_volume_instances[] = {
            "World_PV", "ellipsoid1_PV", "World_PV", "polycone1_PV",
            "World_PV", "polycone1_PV",  "World_PV", "sphere1_PV",
            "World_PV", "box500_PV",     "World_PV", "cone1_PV",
            "World_PV", "trd1_PV",       "World_PV", "parabol1_PV",
            "World_PV", "reflNormal",    "World_PV", "xtru1_PV",
            "World_PV",
        };
        EXPECT_VEC_EQ(expected_volume_instances, result.volume_instances);
        static real_type const expected_distances[] = {
            180.00156256104,
            39.99687487792,
            94.90156256104,
            2,
            16.2,
            2,
            115.41481927853,
            39.482055599395,
            60.00312512208,
            50,
            73.06,
            53.88,
            83.01,
            30.1,
            88.604510136799,
            42.690979726401,
            88.61120889722,
            30.086602479158,
            1.4328892366113,
            15.880952380952,
            67.642857142857,
        };
        EXPECT_VEC_SOFT_EQ(expected_distances, result.distances);
        std::vector<real_type> expected_hw_safety = {
            74.5,
            0.5,
            45.689062136067,
            0,
            8.0156097709407,
            0.98058067569092,
            41.027453049596,
            13.753706517458,
            30.00022317033,
            24.5,
            36.269790909927,
            24.5,
            41.2093531814,
            14.97530971266,
            35.6477449316,
            14.272587510357,
            35.651094311811,
            14.968644196913,
            0.71288903993994,
            6.5489918373272,
            33.481506089183,
        };
        if (test_->geometry_type() == "VecGeom")
        {
            // v1.2.10: unknown differences
            expected_hw_safety[4] = 7.82052980478031;
            expected_hw_safety[14] = 42.8397753718277;
            expected_hw_safety[15] = 18.8833925371992;
            expected_hw_safety[16] = 42.8430141842906;
        }

        EXPECT_VEC_SOFT_EQ(expected_hw_safety, result.halfway_safeties);
    }
    {
        SCOPED_TRACE("Lower +x");
        auto result = test_->track({-575, -125, 0.5}, {1, 0, 0});
        static char const* const expected_volumes[] = {
            "World",   "trd3_refl",  "trd3_refl", "World",    "arb8b",
            "World",   "arb8a",      "World",     "trap1",    "World",
            "tetrah1", "World",      "orb1",      "World",    "genPocone1",
            "World",   "genPocone1", "World",     "elltube1", "World",
        };
        EXPECT_VEC_EQ(expected_volumes, result.volumes);
        std::vector<std::string> expected_volume_instances = {
            "World_PV",      "reflected", "reflected",     "World_PV",
            "arb8b_PV",      "World_PV",  "arb8a_PV",      "World_PV",
            "trap1_PV",      "World_PV",  "tetrah1_PV",    "World_PV",
            "orb1_PV",       "World_PV",  "genPocone1_PV", "World_PV",
            "genPocone1_PV", "World_PV",  "elltube1_PV",   "World_PV",
        };
        EXPECT_VEC_EQ(expected_volume_instances, result.volume_instances);
        static real_type const expected_distances[] = {
            34.956698760421,
            30.086602479158,
            24.913397520842,
            70.093301239579,
            79.9,
            45.1,
            79.9,
            68.323075218214,
            33.591007606176,
            57.452189546021,
            53.886393227913,
            81.800459523757,
            79.99374975584,
            39.95312512208,
            15,
            60.1,
            15,
            59.95,
            40,
            205,
        };
        EXPECT_VEC_SOFT_EQ(expected_distances, result.distances);
        std::vector<real_type> expected_hw_safety = {
            17.391607656793,
            14.968644196913,
            12.394878533861,
            34.872720758987,
            39.7517357488891,
            22.438088639235,
            33.0701970644251,
            32.739905171863,
            15.672519698479,
            26.80540527207,
            2.9387549751221,
            4.4610799311799,
            39.5,
            19.877422680791,
            7.2794797676807,
            29.515478338297,
            0,
            29.826239776544,
            20,
            74.5,
        };
        if (test_->geometry_type() == "Geant4"
            && geant4_version < Version{11, 3})
        {
            // Older versions of Geant4 have a bug in Arb8 that overestimates
            // safety distance to twisted surfaces
            expected_hw_safety[4] = 38.205672682313;
            expected_hw_safety[6] = 38.803595749271;
        }
        else if (test_->geometry_type() == "VecGeom")
        {
            expected_hw_safety = {
                17.391607656793,
                14.968644196913,
                12.394878533861,
                29.99665061979,
                27.765772866092,
                17.5,
                21.886464159888,
                29.111537609107,
                15.672519698479,
                26.80540527207,
                2.9387549751221,
                4.4610799311799,
                39.5,
                19.038294080807,
                0.5,
                29.515478338297,
                0,
                28.615060270982,
                20,
                74.5,
            };
        }

        EXPECT_VEC_SOFT_EQ(expected_hw_safety, result.halfway_safeties);
    }
    {
        SCOPED_TRACE("Middle +y");
        auto result = test_->track({0, -250, 0.5}, {0, 1, 0});
        static char const* const expected_volumes[] = {
            "World",
            "tetrah1",
            "World",
            "box500",
            "World",
            "boolean1",
            "World",
            "boolean1",
            "World",
        };
        EXPECT_VEC_EQ(expected_volumes, result.volumes);
        static char const* const expected_volume_instances[] = {
            "World_PV",
            "tetrah1_PV",
            "World_PV",
            "box500_PV",
            "World_PV",
            "boolean1_PV",
            "World_PV",
            "boolean1_PV",
            "World_PV",
        };
        EXPECT_VEC_EQ(expected_volume_instances, result.volume_instances);
        static real_type const expected_distances[] = {
            105.03085028998,
            20.463165522069,
            99.505984187954,
            50,
            75,
            15,
            20,
            15,
            150,
        };
        EXPECT_VEC_NEAR(expected_distances, result.distances, bool_tol);
        static real_type const expected_hw_safety[] = {
            48.759348159052,
            7.2348215525988,
            35.180678093972,
            24.5,
            37.5,
            7.5,
            7.5,
            7.5,
            74.5,
        };
        EXPECT_VEC_NEAR(expected_hw_safety, result.halfway_safeties, bool_tol);
    }
}

//---------------------------------------------------------------------------//
// SIMPLE CMS
//---------------------------------------------------------------------------//
void SimpleCmsGeoTest::test_trace() const
{
    bool const is_orange = test_->geometry_type() == "ORANGE";
    {
        auto result = test_->track({-75, 0, 0}, {1, 0, 0});
        GenericGeoTrackingResult ref;
        ref.volumes = {
            "si_tracker",
            "vacuum_tube",
            "si_tracker",
            "em_calorimeter",
            "had_calorimeter",
            "sc_solenoid",
            "fe_muon_chambers",
            "world",
        };
        ref.volume_instances = {
            "si_tracker_pv",
            "vacuum_tube_pv",
            "si_tracker_pv",
            "em_calorimeter_pv",
            "had_calorimeter_pv",
            "sc_solenoid_pv",
            "iron_muon_chambers_pv",
            "world_PV",
        };
        ref.distances = {45, 60, 95, 50, 100, 100, 325, 300};
        ref.halfway_safeties = {22.5, 30, 47.5, 25, 50, 50, 162.5, 150};
        ref.bumps = {};

        if (is_orange)
        {
            ref.volume_instances.clear();
            // TODO: at this exact point it ignores the cylindrical distance
            ref.halfway_safeties[1] = 700;
        }
        auto tol = GenericGeoTrackingTolerance::from_test(*test_);
        EXPECT_RESULT_NEAR(ref, result, tol);
    }
    {
        auto result = test_->track({25, 0, 701}, {0, 0, -1});
        GenericGeoTrackingResult ref;
        ref.volumes = {"world", "vacuum_tube", "world"};
        ref.volume_instances = {"world_PV", "vacuum_tube_pv", "world_PV"};
        ref.distances = {1, 1400, 1300};
        ref.halfway_safeties = {0.5, 5, 650};
        ref.bumps = {};

        if (is_orange)
        {
            ref.volume_instances.clear();
            ref.halfway_safeties[2] = 5;
        }

        auto tol = GenericGeoTrackingTolerance::from_test(*test_);
        EXPECT_RESULT_NEAR(ref, result, tol);
    }
}

//---------------------------------------------------------------------------//
// TESTEM3 NESTED
//---------------------------------------------------------------------------//
void TestEm3GeoTest::test_trace() const
{
    {
        auto result = test_->track({-20.1}, {1, 0, 0});

        static char const* const expected_volumes[] = {
            "world", "pb",  "lar",  "pb",  "lar", "pb",  "lar", "pb",  "lar",
            "pb",    "lar", "pb",   "lar", "pb",  "lar", "pb",  "lar", "pb",
            "lar",   "pb",  "lar",  "pb",  "lar", "pb",  "lar", "pb",  "lar",
            "pb",    "lar", "pb",   "lar", "pb",  "lar", "pb",  "lar", "pb",
            "lar",   "pb",  "lar",  "pb",  "lar", "pb",  "lar", "pb",  "lar",
            "pb",    "lar", "pb",   "lar", "pb",  "lar", "pb",  "lar", "pb",
            "lar",   "pb",  "lar",  "pb",  "lar", "pb",  "lar", "pb",  "lar",
            "pb",    "lar", "pb",   "lar", "pb",  "lar", "pb",  "lar", "pb",
            "lar",   "pb",  "lar",  "pb",  "lar", "pb",  "lar", "pb",  "lar",
            "pb",    "lar", "pb",   "lar", "pb",  "lar", "pb",  "lar", "pb",
            "lar",   "pb",  "lar",  "pb",  "lar", "pb",  "lar", "pb",  "lar",
            "pb",    "lar", "world"};
        EXPECT_VEC_EQ(expected_volumes, result.volumes);
        static real_type const expected_distances[] = {
            0.1,  0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57,
            0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23,
            0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57,
            0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23,
            0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57,
            0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23,
            0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57,
            0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23,
            0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57,
            0.23, 0.57, 4};
        EXPECT_VEC_SOFT_EQ(expected_distances, result.distances);
        static real_type const expected_hw_safety[]
            = {0.050, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285,
               0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115,
               0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285,
               0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115,
               0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285,
               0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115,
               0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285,
               0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115,
               0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285,
               0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115,
               0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285,
               0.115, 0.285, 2};
        EXPECT_VEC_SOFT_EQ(expected_hw_safety, result.halfway_safeties);
    }
}

//---------------------------------------------------------------------------//
// TESTEM3 FLAT
//---------------------------------------------------------------------------//
void TestEm3FlatGeoTest::test_trace() const
{
    {
        auto result = test_->track({-20.1}, {1, 0, 0});

        static char const* const expected_volumes[] = {
            "world",       "gap_0",  "absorber_0",  "gap_1",
            "absorber_1",  "gap_2",  "absorber_2",  "gap_3",
            "absorber_3",  "gap_4",  "absorber_4",  "gap_5",
            "absorber_5",  "gap_6",  "absorber_6",  "gap_7",
            "absorber_7",  "gap_8",  "absorber_8",  "gap_9",
            "absorber_9",  "gap_10", "absorber_10", "gap_11",
            "absorber_11", "gap_12", "absorber_12", "gap_13",
            "absorber_13", "gap_14", "absorber_14", "gap_15",
            "absorber_15", "gap_16", "absorber_16", "gap_17",
            "absorber_17", "gap_18", "absorber_18", "gap_19",
            "absorber_19", "gap_20", "absorber_20", "gap_21",
            "absorber_21", "gap_22", "absorber_22", "gap_23",
            "absorber_23", "gap_24", "absorber_24", "gap_25",
            "absorber_25", "gap_26", "absorber_26", "gap_27",
            "absorber_27", "gap_28", "absorber_28", "gap_29",
            "absorber_29", "gap_30", "absorber_30", "gap_31",
            "absorber_31", "gap_32", "absorber_32", "gap_33",
            "absorber_33", "gap_34", "absorber_34", "gap_35",
            "absorber_35", "gap_36", "absorber_36", "gap_37",
            "absorber_37", "gap_38", "absorber_38", "gap_39",
            "absorber_39", "gap_40", "absorber_40", "gap_41",
            "absorber_41", "gap_42", "absorber_42", "gap_43",
            "absorber_43", "gap_44", "absorber_44", "gap_45",
            "absorber_45", "gap_46", "absorber_46", "gap_47",
            "absorber_47", "gap_48", "absorber_48", "gap_49",
            "absorber_49", "world",
        };
        EXPECT_VEC_EQ(expected_volumes, result.volumes);
        static real_type const expected_distances[] = {
            0.1,  0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57,
            0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23,
            0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57,
            0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23,
            0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57,
            0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23,
            0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57,
            0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23,
            0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57, 0.23, 0.57,
            0.23, 0.57, 4,
        };
        EXPECT_VEC_SOFT_EQ(expected_distances, result.distances);
        static real_type const expected_hw_safety[] = {
            0.05,  0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285,
            0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115,
            0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285,
            0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115,
            0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285,
            0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115,
            0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285,
            0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115,
            0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285,
            0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115,
            0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285, 0.115, 0.285,
            0.115, 0.285, 2,
        };
        EXPECT_VEC_SOFT_EQ(expected_hw_safety, result.halfway_safeties);
    }
}

//---------------------------------------------------------------------------//
// TRANSFORMED BOX
//---------------------------------------------------------------------------//
//! Test geometry accessors
void TransformedBoxGeoTest::test_accessors() const
{
    auto const& geo = *test_->geometry_interface();
    if (test_->geometry_type() != "ORANGE")
    {
        EXPECT_EQ(3, geo.max_depth());
    }

    auto expected_bbox = calc_expected_bbox(
        test_->geometry_type(), {-50., -50., -50.}, {50., 50., 50.});
    auto const& bbox = geo.bbox();
    EXPECT_VEC_SOFT_EQ(expected_bbox.lower(), to_cm(bbox.lower()));
    EXPECT_VEC_SOFT_EQ(expected_bbox.upper(), to_cm(bbox.upper()));
}

//---------------------------------------------------------------------------//
void TransformedBoxGeoTest::test_trace() const
{
    // Surface VecGeom needs lower safety tolerance, and this test needs even
    // lower
    real_type const safety_tol = real_type{10} * test_->safety_tol();

    {
        auto result = test_->track({0, 0, -25}, {0, 0, 1});
        static char const* const expected_volumes[] = {
            "world",
            "simple",
            "world",
            "enclosing",
            "tiny",
            "enclosing",
            "world",
            "simple",
            "world",
        };
        EXPECT_VEC_EQ(expected_volumes, result.volumes);
        static real_type const expected_distances[] = {
            13,
            4,
            6,
            1.75,
            0.5,
            1.75,
            6,
            4,
            38,
        };
        EXPECT_VEC_SOFT_EQ(expected_distances, result.distances);
        if (test_->geometry_type() != "ORANGE")
        {
            static real_type const expected_hw_safety[] = {
                5.3612159321677,
                1,
                2.3301270189222,
                0.875,
                0.25,
                0.875,
                3,
                1,
                19,
            };
            EXPECT_VEC_NEAR(
                expected_hw_safety, result.halfway_safeties, safety_tol);
        }
    }
    {
        auto result = test_->track({0.25, 0, -25}, {0., 0, 1});
        static char const* const expected_volumes[] = {
            "world",
            "simple",
            "world",
            "enclosing",
            "tiny",
            "enclosing",
            "world",
            "simple",
            "world",
        };
        EXPECT_VEC_EQ(expected_volumes, result.volumes);
        static real_type const expected_distances[] = {
            12.834936490539,
            3.7320508075689,
            6.4330127018922,
            1.75,
            0.5,
            1.75,
            6,
            4,
            38,
        };
        EXPECT_VEC_SOFT_EQ(expected_distances, result.distances);
        if (test_->geometry_type() != "ORANGE")
        {
            static real_type const expected_hw_safety[] = {
                5.5576905283833,
                0.93301270189222,
                2.0176270189222,
                0.75,
                0.25,
                0.75,
                3,
                0.75,
                19,
            };
            EXPECT_VEC_NEAR(
                expected_hw_safety, result.halfway_safeties, safety_tol);
        }
    }
    {
        auto result = test_->track({0, 0.25, -25}, {0, 0., 1});
        static char const* const expected_volumes[] = {
            "world",
            "simple",
            "world",
            "enclosing",
            "tiny",
            "enclosing",
            "world",
            "simple",
            "world",
        };
        EXPECT_VEC_EQ(expected_volumes, result.volumes);
        static real_type const expected_distances[] = {
            13,
            4,
            6,
            1.75,
            0.5,
            1.75,
            6,
            4,
            38,
        };
        EXPECT_VEC_SOFT_EQ(expected_distances, result.distances);
        if (test_->geometry_type() != "ORANGE")
        {
            static real_type const expected_hw_safety[] = {
                5.3612159321677,
                1,
                2.3301270189222,
                0.875,
                0.12530113594871,
                0.875,
                3,
                1,
                19,
            };
            EXPECT_VEC_NEAR(
                expected_hw_safety, result.halfway_safeties, safety_tol);
        }
    }
    {
        auto result = test_->track({0.01, -20, 0.20}, {0, 1, 0});
        static char const* const expected_volumes[]
            = {"world", "enclosing", "tiny", "enclosing", "world"};
        EXPECT_VEC_EQ(expected_volumes, result.volumes);
        static real_type const expected_distances[]
            = {18.5, 1.1250390198213, 0.75090449735279, 1.1240564828259, 48.5};
        EXPECT_VEC_SOFT_EQ(expected_distances, result.distances);
        if (test_->geometry_type() != "ORANGE")
        {
            static real_type const expected_hw_safety[]
                = {9.25, 0.56184193052552, 0.05, 0.56135125378224, 24.25};
            EXPECT_VEC_NEAR(
                expected_hw_safety, result.halfway_safeties, safety_tol);
        }
    }
}

//---------------------------------------------------------------------------//
// TRANSFORMED BOX
//---------------------------------------------------------------------------//
//! Test geometry accessors
void TwoBoxesGeoTest::test_accessors() const
{
    auto const& geo = *test_->geometry_interface();
    if (test_->geometry_type() != "ORANGE")
    {
        EXPECT_EQ(2, geo.max_depth());
    }
    else
    {
        EXPECT_EQ(3, geo.volumes().size());
    }

    auto expected_bbox = calc_expected_bbox(
        test_->geometry_type(), {-500., -500., -500.}, {500., 500., 500.});
    auto const& bbox = geo.bbox();
    EXPECT_VEC_SOFT_EQ(expected_bbox.lower(), to_cm(bbox.lower()));
    EXPECT_VEC_SOFT_EQ(expected_bbox.upper(), to_cm(bbox.upper()));
}

//---------------------------------------------------------------------------//
void TwoBoxesGeoTest::test_trace() const
{
    {
        auto result = test_->track({0, 0.25, -25}, {0, 0., 1});
        result.print_expected();
    }
}

//---------------------------------------------------------------------------//
// ZNENV
//---------------------------------------------------------------------------//
void ZnenvGeoTest::test_trace() const
{
    // NOTE: This tests the capability of the G4PVDivision conversion based on
    // an ALICE component
    static char const* const expected_mid_volumes[] = {
        "World", "ZNENV", "ZNST", "ZNST",  "ZNST",  "ZNST", "ZNST",
        "ZNST",  "ZNST",  "ZNST", "ZNST",  "ZNST",  "ZNST", "ZNST",
        "ZNST",  "ZNST",  "ZNST", "ZNST",  "ZNST",  "ZNST", "ZNST",
        "ZNST",  "ZNST",  "ZNST", "ZNENV", "World",
    };
    static real_type const expected_mid_distances[] = {
        6.38, 0.1,  0.32, 0.32, 0.32, 0.32, 0.32, 0.32,  0.32,
        0.32, 0.32, 0.32, 0.32, 0.32, 0.32, 0.32, 0.32,  0.32,
        0.32, 0.32, 0.32, 0.32, 0.32, 0.32, 0.1,  46.38,
    };
    {
        auto result = test_->track({-10, 0.0001, 0}, {1, 0, 0});
        EXPECT_VEC_EQ(expected_mid_volumes, result.volumes);
        EXPECT_VEC_SOFT_EQ(expected_mid_distances, result.distances);
    }
    {
        auto result = test_->track({0.0001, -10, 0}, {0, 1, 0});
        EXPECT_VEC_EQ(expected_mid_volumes, result.volumes);
        EXPECT_VEC_SOFT_EQ(expected_mid_distances, result.distances);
    }
}

//---------------------------------------------------------------------------//
}  // namespace test
}  // namespace celeritas
