nadc_tools
==========

The software package "nadc_tools" contains a number of tools to access the official GOME and Sciamachy level 0, 1b and 2 data products as distributed by DLR and ESRIN and tools to access GOME and Sciamachy level 2 products developed by SRON and KNMI. 

### Purpose and Limitations ###
The probably most important part of "nadc_tools" are the C-libraries, which contain functions to read Sciamachy data products into memory, dump the data as ASCII output or write the data in PDS or HDF5 format. The software is written in ANSI-C (and even assembler), and coded in such a way that you (an experienced (C-) programmer) can easily understand the code. The data extractors, included in this software package, should give a good example of how to use the libraries.

The software distributed in this package contains libraries, written in ANSI-C (mostly POSIX compliant except for some ISO C99 extensions) and an IDL wrapper library (using _CALL_EXTERNAL_) with IDL functions to read GOME and Sciamachy data.
The IDL interface requires sharable object libraries. Building shared libraries is easy on the supported platforms, however, do not forget to set the environment variable _LD_LIBRARY_PATH_.

This software package is written and maintained by Richard van Hees (SRON), and distributed under the GNU General Public License (version 2).

### Acknowledgement ###
In case (part of) this dataset is used for a publication and essential to the work and the results, an offer of co-authorship to Richard van Hees (SRON) and /or Ralph Snel (SRON) is highly appreciated. Please do not hesitate to contact us in an early stage for necessary support.

### DISCLAIMER ###
We at SRON have developed nadc_tools as part of our commitment towards the verification and further improvement of the calibration of Sciamachy data. This software is developed for in-house usage, and generously shared with you WITHOUT ANY WARRANTY. We will try to help you with any problems, but only on a ``best effort'' basis. In order to continuously improve this facility, feedback from users is highly appreciated. 

Note that this software package is also not supported in any way by ESA or DLR, although the nadc_tools extractors and libraries mimic some of the functionality of the gdp and EnviView toolbox.

The software package is in no way intended to replace the official data processor and should not be treated as such. Especially it is not meant to (and cannot) produce official data products. These have to be derived with ESA approved tools such as EnviView that can be downloaded for free from ESA.

It is your own responsibility to verify the results you produce with this package against official data products. If you find discrepancies, please inform SRON (not ESA or DLR) at once. Any information you give will help us to improve our software. We will relay to ESA all information to improve the official data processor.

## Checking out a working copy ##
As of 2013-03-20 the source code is hosted as a public repository on Github.

In order to retrieve a working copy or clone into a local bare Git repository:

    git clone https://github.com/rmvanhees/nadc_tools.git [<local>]

It is adviced though that in order to push back changes, your are working off a fork from the original repository and then issue a pull request once you think your code is ready to be merged back into the main repositorys' master branch.

## Organization of files and directories ##
After checking out a working copy, you will be left with the following directory structure:

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
    `-- libNADC_SCIA
        |-- CAL
        |   `-- CalibModules
        `-- SDMF

## Dependencies ##

### Development tools ###

| Name  | Version | Relevance | Notes                                  |
|-------|---------|-----------|----------------------------------------|
| Git   | > 1.7   | mandatory | Distributed revision control system    |
| CMake | > 2.8.0 | mandatory | Cross-platform build system generator  |

### Documentation generation tools ###

| Name    | Version | Relevance | Notes                                    |
|---------|---------|-----------|------------------------------------------|

### External libraries ###

| Name      | Version | Relevance | Notes                                                      |
|-----------|---------|-----------|------------------------------------------------------------|
| HDF5      | 1.8.x   | mandatory | Hierarchical Data Format, version 5 (www.hdfgroup.org)     |
| netCDF-4  | 4.2.x   | optional  | Network Common Data Form, version 4 (www.unidata.ucar.edu) |                                                      |
| PostgreSQL| 9.x     | optional  | www.postgresql.org                                         |


## Configuration and build ##

The software uses the CMake (www.cmake.org) cross-platform makefile generator in order to configure for building library, tools and test programs.

It is advised to perform an out-of-source build, in order to prevent polluting the source directories:

    mkdir build
    cd build
    cmake [OPTIONS] ..

If the configuration completed succesfully, in order to build all available components simply type (the optional `-j` option is for parallel builds):

    make [-j<n>]

Configuration options typically used are:

 * `-DCMAKE_BUILD_TYPE=Debug` -- build type: `Debug` or `Release`
 * `-DCMAKE_INSTALL_PREFIX=<path>` -- user defined installation prefix
 * `-DNADC_TOOLS_DATADIR=<path>` -- path to various Sciamachy calibration parameters

Non-standard Environment variable used by cmake:
 * `NADC_EXTERN` -- directory which contains the include/ and lib/ subdirectories of required packages
 * `HDF5_DIR` -- directory which contains the include/ and lib/ subdirectories of the HDF5 installation
 * `IDL_DIR` -- directory which contains the include/ and lib/ subdirectories of the IDL installation

In a similar manner you can trigger the installation of the various components (library, header files, executables) via

    make install

In order to build an individual target/component, pick one of the items on the list return with

    make help

_Note_: When configuring your build, you can choose between a debug and a release build. The debug build contains the necessary symbols in the files for running a debugger, and has been compiled with coverage flags.

The release build is built using the `-march-native -O3` optimization flag, and none of the debugging flags. This does make some tools considerably faster, so once you're satisfied that everything has built correctly, you should use the tools from a release build for your analysis.

## Installation ##

Once the build has completed you can install the software: to do this run

    make install

such that header files, libraries, program executables and documentation will be placed into the appropriate locations:

    <prefix>                      ...  Installation prefix, CMAKE_INSTALL_PREFIX
     |-- bin                      ...  Program executables of the tools
     |-- include                  ...  Header files
     |-- lib                      ...  (Shared) libraries
     `-- share
         `-- idl_nadc             ...  IDL programs
             |-- Examples
             `-- StructDefs
