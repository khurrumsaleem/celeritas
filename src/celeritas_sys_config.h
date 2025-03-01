/*------------------------------- -*- C -*- ---------------------------------*
 * Copyright Celeritas contributors: see top-level COPYRIGHT file for details
 * SPDX-License-Identifier: (Apache-2.0 OR MIT)
 *---------------------------------------------------------------------------*/
/*!
 * \file celeritas_sys_config.h
 * \brief System-specific configuration options for Celeritas.
 * \deprecated This file should be replaced by "corecel/Config.hh".
 */
// DEPRECATED: remove in Celeritas v1.0
//---------------------------------------------------------------------------//
#ifndef celeritas_sys_config_h
#define celeritas_sys_config_h

#if __cplusplus < 201703L
#    error "Celeritas requires C++17 or greater and is not C compatible"
#endif

#if __GNUC__ > 8 || __clang__
#    pragma GCC warning \
        "celeritas_sys_config.h is deprecated and should be replaced by \"corecel/Config.hh\""
#endif

#include "corecel/Config.hh"

#endif /* celeritas_sys_config_h */
