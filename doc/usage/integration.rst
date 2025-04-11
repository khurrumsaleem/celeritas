.. Copyright Celeritas contributors: see top-level COPYRIGHT file for details
.. SPDX-License-Identifier: CC-BY-4.0

.. highlight:: cmake

.. _library:

Software library
================

The main current use case for Celeritas as a library is to integrate with user
Geant4 applications. The interface for Geant4 integration is quite stable, and
additional lower level code, described in :ref:`api`, is also stable.

Usage overview
--------------

This describes the main points of integrating using the tracking interface,
recommended for all applications that support Geant4 11.0 or higher.

1. Find and link in your CMake project:

   - Find the Celeritas package.
   - Link ``Celeritas::G4`` .
   - Use ``celeritas_target_link_libraries`` instead of
     ``target_link_libraries`` when both VecGeom and CUDA are enabled.

2. Set up physics offloading in your "main" function.

   - Include ``<CeleritasG4.hh>``
   - Register the ``TrackingManagerConstructor``, which tells Geant4 to send EM
     tracks to Celeritas rather than the main tracking loop.
   - Tweak the ``SetupOptions`` based on problem requirements, or use the
     :ref:`g4_ui_macros`.

3. Add hooks to safely set up and tear down Celeritas and its GPU code.

   - Call ``BeginOfRunAction`` at the beginning of the run to initialize
     problem data (master or serial) and local state (worker or serial).
   - Call ``EndOfRunAction`` at the end of the run to safely deallocate
     everything.

The changes for a simple application look like:

.. literalinclude:: ../../example/geant4/add-celer.diff
   :language: diff

CMake integration
-----------------

The Celeritas library is most easily used when your downstream app is built with
CMake, demonstrated by the :ref:`example_minimal` and :ref:`example_cmake` examples. It should require a single line to initialize::

   find_package(Celeritas REQUIRED)

and a single line to link::

   celeritas_target_link_libraries(mycode PUBLIC Celeritas::celeritas)

This special CMake command and its sisters wrap the CMake commands
``add_library``, ``set_target_properties``
``target_link_libraries``, ``target_include_directories``,
``target_compile_options``, and ``install``. They are needed in case Celeritas
uses both CUDA and VecGeom, forwarding to the ``CudaRdcUtils`` commands if so
and forwarding to native CMake commands if not. (Note that ``add_executable``
does not need wrapping, so the regular CMake command can be used.)

CudaRdcUtils
^^^^^^^^^^^^

.. note:: Prior to Celeritas v0.6, obtaining the CudaRdc commands required the
   user to include :file:`CudaRdcUtils.cmake` manually. Now it is included
   automatically, but the use of the ``celeritas_`` macros is preferred for
   readability (provenance) in the downstream application.

The special ``cuda_rdc_`` commands are needed when Celeritas is built with both
CUDA and the current version of VecGeom, which uses a special but messy feature
called CUDA Relocatable Device Code (RDC).
As the ``cuda_rdc_...`` functions decay to the wrapped CMake commands if CUDA
and VecGeom are disabled, you can directly use them to safely build and link
nearly all targets
consuming Celeritas in your project, but the ``celeritas_`` versions will
result in a slightly faster (and possibly safer) configuration when VecGeom or
CUDA are disabled.

The CudaRdc commands track and propagate the appropriate sequence of linking
for the final application::

  cuda_rdc_add_library(myconsumer SHARED ...)
  cuda_rdc_target_link_libraries(myconsumer PUBLIC Celeritas::celeritas)

  cuda_rdc_add_executable(myapplication ...)
  cuda_rdc_target_link_libraries(myapplication PRIVATE myconsumer)

If your project builds shared libraries that are intended to be loaded at
application runtime (e.g. via ``dlopen``), you should prefer use the CMake
``MODULE`` target type::

  cuda_rdc_add_library(myplugin MODULE ...)
  cuda_rdc_target_link_libraries(myplugin PRIVATE Celeritas::celeritas)

This is recommended as ``cuda_rdc_target_link_libraries`` understands these as
a final target for which all device symbols require resolving. If you are
forced to use the ``SHARED`` target type for plugin libraries (e.g. via your
project's own wrapper functions), then these should be declared with the CMake
or project-specific commands with linking to both the primary Celeritas target
and its device code counterpart::

  add_library(mybadplugin SHARED ...)
  # ... or myproject_add_library(mybadplugin ...)
  target_link_libraries(mybadplugin
    PRIVATE Celeritas::celeritas $<TARGET_NAME_IF_EXISTS:Celeritas::celeritas_final>
  )
  # ... or otherwise declare the plugin as requiring linking to the two targets

Celeritas device code counterpart target names are always the name of the
primary target appended with ``_final``. They are only present if Celeritas was
built with CUDA support so it is recommended to use the CMake generator
expression above to support CUDA or CPU-only builds transparently.

Targets
^^^^^^^

CMake targets exported by Celeritas live in the ``Celeritas::`` namespace.
These targets are:

- An interface lib ``BuildFlags`` that provides the include path to Celeritas,
  language requirements, and warning overrides;
- Wrapper interface libraries that provide links to optional dependencies if
  enabled: ``ExtMPI``, ``ExtOpenMP``, ``ExtPerfetto``, ``ExtGeant4Geo``
  (G4Geometry if available), and ``ExtDeviceApi`` (CUDA::toolkit if available,
  ROCM/HIP libraries and includes, and roctx64 if available);
- Code libraries described in :ref:`api`: corecel, geocel, orange,
  celeritas, and accel;
- Executables such as ``celer-sim`` and ``celer-geo``.


App integration
---------------

Integrating with Geant4 user applications and experiment frameworks requires
setting up:

- Initialization options that depend on the problem being run and the system
  architecture (see :cpp:class:`celeritas::SetupOptions` until version 1.0, or
  :ref:`input` after)
- "Global" data structures for shared problem data such as geometry (see
  :cpp:class:`celeritas::SharedParams`)
- Per-worker (i.e., ``G4ThreadLocal`` using Geant4 manager/worker semantics)
  data structures that hold the track states (see
  :cpp:class:`celeritas::LocalTransporter`).

See :ref:`example_geant` for examples of integrating Geant4, and
:ref:`api_g4_interface` for detailed documentation of the Geant4 integration
interface.
