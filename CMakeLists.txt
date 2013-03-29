## =============================================================================
##
##  CMake projects setttings
##
## =============================================================================

## Minimum required version of CMake to configure the project
cmake_minimum_required(VERSION 2.8)

## Name of the project
project (NADC_TOOLS)

file (READ VERSION _version_contents)
string (REGEX REPLACE ".*#define[ \t]+nadc_vers_major[ \t]+([0-9]*).*$"
       "\\1" NADC_VERS_MAJOR ${_version_contents})
string (REGEX REPLACE ".*#define[ \t]+nadc_vers_minor[ \t]+([0-9]*).*$"
       "\\1" NADC_VERS_MINOR ${_version_contents})
string (REGEX REPLACE ".*#define[ \t]+nadc_vers_release[ \t]+([0-9]*).*$"
       "\\1" NADC_VERS_RELEASE ${_version_contents})
string (REGEX REPLACE ".*#define[ \t]+nadc_vers_extra[ \t]+([a-z,0-9]*).*$"
       "\\1" NADC_VERS_EXTRA ${_version_contents})
set (PROJECT_VERSION 
    "${NADC_VERS_MAJOR}.${NADC_VERS_MINOR}.${NADC_VERS_RELEASE}")

## CMake standard modules
include (CheckIncludeFile)
include (CheckFunctionExists)
include (FindPackageHandleStandardArgs)
include (TestBigEndian)

## =============================================================================
##
##  Configuration options
##
## =============================================================================
option (ENABLE_TESTING    "Enable the CTest test driver?"            NO)
option (CONFIGURE_VERBOSE "Enhanced verbosity during configuration?" NO)
option (CONFIGURE_SUMMARY "Provide summary at the end of configure?" YES)

#-----------------------------------------------------------------------------
# Option path to SRON calibration parameters
#-----------------------------------------------------------------------------
set (NADC_TOOLS_DATADIR "/SCIA/share/nadc_tools")

#-----------------------------------------------------------------------------
# Option to Build Shared/Static libs, default is shared
#-----------------------------------------------------------------------------
option (BUILD_SHARED_LIBS "Build Shared Libraries" YES)
set (LIB_TYPE STATIC)
set (NADC_ENABLE_SHARED_LIB NO)
set (NADC_ENABLE_STATIC_LIB NO)
if (BUILD_SHARED_LIBS)
  set (LIB_TYPE SHARED)
  set (NADC_ENABLE_SHARED_LIB YES)
  set (CMAKE_INSTALL_SO_NO_EXE "0")
ELSE ()
  SET (NADC_ENABLE_STATIC_LIB YES)
ENDIF ()

##__________________________________________________________
## Installation prefix

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set (CMAKE_INSTALL_PREFIX
    "$ENV{HOME}/x86_64" CACHE PATH "Installation location prefix" FORCE
#    "${PROJECT_SOURCE_DIR}/release" CACHE PATH "Installation location prefix" FORCE
  )
endif (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

## =============================================================================
##
##  System inspection
##
## =============================================================================
## Extra location for custom CMake modules
list (INSERT CMAKE_MODULE_PATH 0 
     "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/"
)

foreach (_extraPath
    $ENV{NADC_EXTERN}
    $ENV{HDF5_DIR}
    $ENV{IDL_DIR}
    $ENV{PPF_DIR}
    )
  list (INSERT CMAKE_PREFIX_PATH  0 ${_extraPath}         )
  list (INSERT CMAKE_INCLUDE_PATH 0 ${_extraPath}/include )
  list (INSERT CMAKE_LIBRARY_PATH 0 ${_extraPath}/lib     )
  list (INSERT CMAKE_PROGRAM_PATH 0 ${_extraPath}/bin     )
endforeach (_extraPath)

## Check for required external packages
foreach (_externalPackage
  HDF5
  PGSQL
  NetCDF
  IDL
  PPF
  )
  ## Convert module name to upper-case
  string (TOUPPER ${_externalPackage} _externalPackageVar)
  ## Verbosity of the script to find the package
  if (CONFIGURE_VERBOSE)
    set (${_externalPackageVar}_FIND_QUIETLY NO)
  else (CONFIGURE_VERBOSE)
    set (${_externalPackageVar}_FIND_QUIETLY YES)
  endif (CONFIGURE_VERBOSE)
  ## Find the package
  find_package (${_externalPackage} REQUIRED)
endforeach (_externalPackage)

##__________________________________________________________
## Check for system header files

check_include_file ( errno.h    HAVE_ERRNO_H   )
check_include_file ( float.h    HAVE_FLOAT_H   )
check_include_file ( limits.h   HAVE_LIMITS_H  )
check_include_file ( math.h     HAVE_MATH_H    )
check_include_file ( stdbool.h  HAVE_STDBOOL_H )
check_include_file ( stdio.h    HAVE_STDIO_H   )
check_include_file ( stdlib.h   HAVE_STDLIB_H  )
check_include_file ( strings.h  HAVE_STRINGS_H )
check_include_file ( time.h     HAVE_TIME_H    )

#-----------------------------------------------------------------------------
# Set the target names of all the libraries
#-----------------------------------------------------------------------------
set (NADC_LIB_TARGET              "nadc")
set (NADC_GOME_LIB_TARGET         "nadc_gome")
set (NADC_MERIS_LIB_TARGET        "nadc_meris")
set (NADC_SCIA_LIB_TARGET         "nadc_scia")
set (NADC_SCIA_CAL_LIB_TARGET     "nadc_scia_cal")
set (NADC_IDL_LIB_TARGET          "nadc_idl")

#-----------------------------------------------------------------------------
# Set the true names of all the libraries if customized by external project
#-----------------------------------------------------------------------------
set (NADC_LIB_NAME                "lib${NADC_LIB_TARGET}")
set (NADC_GOME_LIB_NAME           "lib${NADC_GOME_LIB_TARGET}")
set (NADC_MERIS_LIB_NAME          "lib${NADC_MERIS_LIB_TARGET}")
set (NADC_SCIA_LIB_NAME           "lib${NADC_SCIA_LIB_TARGET}")
set (NADC_SCIA_CAL_LIB_NAME       "lib${NADC_SCIA_CAL_LIB_TARGET}")
set (NADC_IDL_LIB_NAME            "lib${NADC_IDL_LIB_TARGET}")

## =============================================================================
##
##  Global build parameters
##
## =============================================================================
## Compiler flags
set (CMAKE_C_FLAGS "-std=c99 -W -Wall -Wwrite-strings -Wcast-qual -Wc++-compat -Wdisabled-optimization -Wstrict-prototypes -Wdeclaration-after-statement")
set (CMAKE_C_FLAGS_DEBUG "-g -pg -pedantic -Wno-long-long -Wno-overlength-strings")
set (CMAKE_C_FLAGS_RELEASE "-march=native -O3")

## Include directories for header files
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${HDF5_INCLUDE_DIRS}
  ${PGSQL_INCLUDE_DIR}
  )

add_subdirectory (libNADC)
add_subdirectory (libNADC_GOME)
add_subdirectory (libNADC_MERIS)
add_subdirectory (libNADC_SCIA)
if (IDL_FOUND AND BUILD_SHARED_LIBS)
   add_subdirectory (libNADC_IDL)  ## check for IDL and shared libraries
endif ()
add_subdirectory (FRESCO)
add_subdirectory (GOME)
add_subdirectory (IMAP)
add_subdirectory (IMLM)
add_subdirectory (MERIS)
add_subdirectory (SCIA)
add_subdirectory (TOGOMI)
add_subdirectory (TOSOMI)
add_subdirectory (include)
add_subdirectory (idl_progs)


## =============================================================================
##
##  Configuration summary
##
## =============================================================================
if (CONFIGURE_SUMMARY)
message ( " ========================================================"   )
message ( " [nadc_tools] Configuration summary."                        )
message ( " --------------------------------------------------------"   )
message ( " System configuration"                                       )
message ( " .. Processor type ......... = ${CMAKE_SYSTEM_PROCESSOR}"    )
message ( " .. CMake executable ....... = ${CMAKE_COMMAND}"             )
message ( " .. CMake version .......... = ${CMAKE_VERSION}"             )
message ( " .. System name ............ = ${CMAKE_SYSTEM_NAME}"         )
message ( " .. System version ......... = ${CMAKE_SYSTEM_VERSION}"      )
message ( " .. C compiler ............. = ${CMAKE_C_COMPILER}"          )
message ( " .. C compiler flags ....... = ${CMAKE_C_FLAGS}"             )
message ( " .. C++ compiler ........... = ${CMAKE_CXX_COMPILER}"        )
message ( " .. C++ compiler flags ..... = ${CMAKE_CXX_FLAGS}"           )
message ( " --------------------------------------------------------"   )
message ( " Project configuration"                                      )
message ( " .. Project name ............. = ${PROJECT_NAME}"            )
message ( " .. Project version .......... = ${PROJECT_VERSION}"         )
message ( " .. Build shared libraries ... = ${BUILD_SHARED_LIBS}"       )
message ( " .. Verbose configuration .... = ${CONFIGURE_VERBOSE}"       )
message ( " .. Installation prefix ...... = ${CMAKE_INSTALL_PREFIX}"    )
message ( " --------------------------------------------------------"   )
message ( " Processing tools & libraries"                               )
message ( " .. HDF5 library ................. = ${HDF5_C_LIBRARIES}"    )
message ( " .. HDF5 includes ................ = ${HDF5_INCLUDE_DIRS}"   )
message ( " .. HDF5 library version ......... = ${HDF5_VERSION}"        )
message ( " .. NETCDF library ............... = ${NETCDF_LIBRARIES}" )
message ( " .. NETCDF library version ....... = ${NETCDF_VERSION}" )
if (PGSQL_FOUND)
   message ( " .. PSQL library ................. = ${PGSQL_LIBRARY}"    )
   message ( " .. PSQL includes ................ = ${PGSQL_INCLUDE_DIR}"   )
   message ( " .. PSQL library version ......... = ${PGSQL_VERSION}"    )
else ()
   message ( " .. PSQL library ................. = not installed"    )
endif ()
if (IDL_FOUND)
   message ( " .. IDL includes ................. = ${IDL_INCLUDE_DIR}"   )
else ()
   message ( " .. IDL full path ................ = not installed"    )
endif ()
if (PPF_FOUND)
   message ( " .. PPF libraries ................ = ${PPF_LIBRARIES}"    )
   message ( " .. PPF includes ................. = ${PPF_INCLUDE_DIR}"   )
   message ( " .. PPF library version .......... = ${PPF_VERSION}"    )
else ()
   message ( " .. PPF libraries ................ = not installed"    )
endif ()
message ( " ========================================================"   )
endif (CONFIGURE_SUMMARY)
