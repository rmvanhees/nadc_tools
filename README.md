nadc_tools
==========

The software package contains a number of tools to access the official GOME and SCIAMACHY
level 0, 1b and 2 NRT data products as distributed by DLR and ESRIN.
GOME and Sciamachy data extractors originally developed for the Netherlands Sciamachy Data Center.

## Organization of files and directories ##

After checking out a working copy, you will be left with the following directory
structure:

    nadc_tools
    |-- FRESCO
    |-- GOME
    |-- IMAP
    |-- IMLM
    |-- MERIS
    |-- PATCH
    |-- SCIA
    |-- TOGOMI
    |-- TOSOMI
    |-- cmake
    |   `-- modules
    |-- idl_progs
    |   `-- StructDefs
    |-- include
    |-- libNADC
    |-- libNADC_GOME
    |-- libNADC_IDL
    |-- libNADC_SCIA
    |   `-- CAL
    |   |   `-- CalibModules
    |   `-- SDMF

## Dependencies ##

### Development tools ###

| Name  | Version | Relevance | Notes                                  |
|-------|---------|-----------|----------------------------------------|
| Git   | > 1.7   | mandatory | Distributed revision control system.   |
| CMake | > 2.8.0 | mandatory | Cross-platform build system generator. |

### Documentation generation tools ###

| Name    | Version | Relevance | Notes                                    |
|---------|---------|-----------|------------------------------------------|

### External libraries ###

| Name      | Version | Relevance | Notes                                                  |
|-----------|---------|-----------|--------------------------------------------------------|
| netCDF-4  | 4.2.1   | mandatory |                                                        |
| HDF5      | 1.8.x   | mandatory |                                                        |
| PostgreSQL| 9.x     | optional  |                                                        |


## Configuration and build ##

The software uses the CMake (www.cmake.org) cross-platform makefile generator in
order to configure for building library, tools and test programs.

It is advised to perform an out-of-source build, in order to prevent polluting
the source directories:

    mkdir build
    cd build
    cmake [OPTIONS] ..

If the configuration completed succesfully, in order to build all available
components simply type (the optional `-j` option is for parallel builds):

    make [-j]

Configuration options typically used are:

 * `-DCMAKE_BUILD_TYPE=Debug` -- build type: `Debug` or `Release`
 * `-DCMAKE_INSTALL_PREFIX=<path>` -- user defined installation prefix.

If the compilation succeeds, you should run the unit tests:

    make test

In a similar manner you can trigger the installation of the various components
(library, header files, executables) via

    make install

In order to build an individual target/component, pick one of the items on the
list return with

    make help

_Note_: When configuring your build, you can choose between a debug and a release
build. The debug build contains the necessary symbols in the files for running a
debugger, and has been compiled with coverage flags.

The release build is built using the `-O2` optimization flag, and none of the
debugging flags. This does make some tools considerably faster, so once you're
satisfied that everything has built correctly, you should use the tools from a
release build for your analysis.


## Installation ##

Once the build has completed you can install the software: to do this run

    make install

such that header files, libraries, program executables and documentation will be
placed into the appropriate locations:

    <prefix>                      ...  Installation prefix, CMAKE_INSTALL_PREFIX
     |-- bin                      ...  Program executables of the tools
     |-- include
     |-- lib                      ...  (Shared) libraries
     `-- share
         `-- idl_nadc

