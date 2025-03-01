//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/ext/RootFileManager.cc
//---------------------------------------------------------------------------//
#include "RootFileManager.hh"

#include <TBranch.h>
#include <TFile.h>
#include <TTree.h>

#include "corecel/Assert.hh"
#include "corecel/io/Logger.hh"
#include "corecel/io/ScopedTimeLog.hh"
#include "corecel/sys/Environment.hh"
#include "corecel/sys/ScopedMem.hh"

// This "public API" function is defined in CeleritasRootInterface.cxx to
// initialize ROOT. It's not necessary for shared libraries (due to static
// initialization shenanigans) but is needed for static libs. The name is a
// function of the name passed to the MODULE argument of
// the cmake root_generate_dictionary command.
void TriggerDictionaryInitialization_libceleritas();

namespace celeritas
{
//---------------------------------------------------------------------------//
/*!
 * Whether ROOT interfacing is enabled.
 *
 * This is true unless the \c CELER_DISABLE_ROOT environment variable is
 * set to a non-empty value.
 */
bool RootFileManager::use_root()
{
    static bool const result = [] {
        if (!celeritas::getenv("CELER_DISABLE_ROOT").empty())
        {
            CELER_LOG(info) << "Disabling ROOT support since the "
                               "'CELER_DISABLE_ROOT' "
                               "environment variable is present and non-empty";
            return false;
        }

        CELER_LOG(debug) << "Initializing ROOT dictionaries";
        TriggerDictionaryInitialization_libceleritas();
        return true;
    }();
    return result;
}

//---------------------------------------------------------------------------//
/*!
 * Construct with ROOT filename.
 */
RootFileManager::RootFileManager(char const* filename)
{
    CELER_EXPECT(filename);

    CELER_LOG(info) << "Opening ROOT file at " << filename;
    ScopedMem record_mem("RootImporter.open");
    ScopedTimeLog scoped_time;

    tfile_.reset(TFile::Open(filename, "recreate"));
    CELER_VALIDATE(tfile_->IsOpen(),
                   << "failed to open ROOT file at '" << filename << "'");
}

//---------------------------------------------------------------------------//
/*!
 * Get the filename of the associated ROOT file.
 */
char const* RootFileManager::filename() const
{
    return tfile_->GetName();
}

//---------------------------------------------------------------------------//
/*!
 * Create tree by providing its name and title.
 *
 * It is still possible to simply create a `TTree("name", "title")` in any
 * scope that a RootFileManager exists, but this function explicitly shows the
 * relationship between the newly created tree and `this->tfile_`.
 *
 * To expand this class to write multiple root files (one per thread), add a
 * `tid` input parameter and call `tfile_[tid].get()`.
 */
UPRootTreeWritable
RootFileManager::make_tree(char const* name, char const* title)
{
    CELER_EXPECT(tfile_->IsOpen());

    int const split_level = 99;
    UPRootTreeWritable uptree;
    uptree.reset(new TTree(name, title, split_level, tfile_.get()));
    return uptree;
}

//---------------------------------------------------------------------------//
/*!
 * Manually write TFile.
 */
void RootFileManager::write()
{
    CELER_EXPECT(tfile_->IsOpen());
    int write_status = tfile_->Write();
    CELER_ENSURE(write_status);
}

//---------------------------------------------------------------------------//
}  // namespace celeritas
