## =============================================================================
##
##  CMake projects setttings
##
## =============================================================================

## Minimum required version of CMake to configure the project
cmake_minimum_required(VERSION 2.8.9)

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
include (CheckCCompilerFlag)
include (CheckIncludeFile)
include (CheckFunctionExists)
include (FindPackageHandleStandardArgs)
include (TestBigEndian)

## =============================================================================
##
##  Configuration options
##
## =============================================================================
option (ENABLE_TESTING    "Enable the CTest test driver?"            NO )
option (BUILD_SHARED_LIBS "Enable building shared libraries"         YES)
option (CONFIGURE_VERBOSE "Enhanced verbosity during configuration?" NO )
option (CONFIGURE_SUMMARY "Provide summary at the end of configure?" YES)

##__________________________________________________________
## Installation prefix

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set (CMAKE_INSTALL_PREFIX
    "$ENV{HOME}/x86_64" CACHE PATH "Installation location prefix" FORCE
    )
endif (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

##__________________________________________________________
## Handle option: building shared libraries

set (LIB_TYPE STATIC)
set (NADC_ENABLE_SHARED_LIB NO)
set (NADC_ENABLE_STATIC_LIB NO)
if (BUILD_SHARED_LIBS)
  set (LIB_TYPE SHARED)
  set (NADC_ENABLE_SHARED_LIB YES)
  set (CMAKE_INSTALL_SO_NO_EXE "0")
else ()
  SET (NADC_ENABLE_STATIC_LIB YES)
endif ()

#-----------------------------------------------------------------------------
# Path to SRON calibration parameters
#-----------------------------------------------------------------------------
find_path (NADC_TOOLS_DATADIR
	  MEMcorr.h5
	  PATHS "/SCIA/share/nadc_tools"
	  	"$ENV{HOME}/SCIA/CKD"
		"/data/$ENV{USER}/SCIA/CKD"
	  NO_DEFAULT_PATH
)

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
check_include_file ( string.h   HAVE_STRING_H )
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

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

## =============================================================================
##
##  Global build parameters
##
## =============================================================================
## Compiler flags
set (CMAKE_C_FLAGS "-std=c99 -W -Wall -Wwrite-strings -Wcast-qual -Wc++-compat -Wstrict-prototypes -Wdeclaration-after-statement")

check_c_compiler_flag (-Wdisabled-optimization HAS_DISABLED_OPTIMIZATION)
if (HAS_DISABLED_OPTIMIZATION)
   set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wdisabled-optimization")
endif ()

check_c_compiler_flag (-Wsuggest-attribute=const HAS_SUGGEST_ATTRIBUTE)
if (HAS_SUGGEST_ATTRIBUTE)
   set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wsuggest-attribute=const")
endif ()

check_c_compiler_flag (-fcatch-undefined-behavior HAS_CATCH_UNDEF_BEHAVIOR)
if (HAS_CATCH_UNDEF_BEHAVIOR)
   set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fcatch-undefined-behavior")
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "")
   set (CMAKE_C_FLAGS "-Os ${CMAKE_C_FLAGS}")
endif ()

set (CMAKE_C_FLAGS_DEBUG "-g -pedantic -Wno-long-long -Wno-overlength-strings")

set (CMAKE_C_FLAGS_RELEASE "-march=native -O3")

## Include directories for header files
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${HDF5_INCLUDE_DIRS}
  ${PGSQL_INCLUDE_DIR}
)

## =============================================================================
##
##  Project sub-directories
##
## =============================================================================
add_subdirectory (libNADC)
add_subdirectory (libNADC_GOME)
add_subdirectory (libNADC_MERIS)
add_subdirectory (libNADC_SCIA)
add_subdirectory (FRESCO)
add_subdirectory (GOME)
add_subdirectory (IMAP)
add_subdirectory (IMLM)
add_subdirectory (MERIS)
add_subdirectory (SCIA)
add_subdirectory (TOGOMI)
add_subdirectory (TOSOMI)
add_subdirectory (include)
## check for IDL and shared libraries
if (IDL_FOUND AND BUILD_SHARED_LIBS)
   add_subdirectory (libNADC_IDL)
   add_subdirectory (idl_progs)
endif ()

## =============================================================================
##
##  Configuration summary
##
## =============================================================================
if (CONFIGURE_SUMMARY)
message ( " ========================================================"    )
message ( " [nadc_tools] Configuration summary."                         )
message ( " --------------------------------------------------------"    )
message ( " System configuration"                                        )
message ( " .. System name ................ = ${CMAKE_SYSTEM_NAME}"      )
message ( " .. System version ............. = ${CMAKE_SYSTEM_VERSION}"   )
message ( " .. Processor type ............. = ${CMAKE_SYSTEM_PROCESSOR}" )
message ( " .. CMake executable ........... = ${CMAKE_COMMAND}"          )
message ( " .. CMake version .............. = ${CMAKE_VERSION}"          )
message ( " .. CMake build type ........... = ${CMAKE_BUILD_TYPE}"       )
message ( " .. C compiler ................. = ${CMAKE_C_COMPILER}"       )
message ( " .. C compiler flags ........... = ${CMAKE_C_FLAGS}"          )
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
   message ( " .. C compiler flags Debug...... = ${CMAKE_C_FLAGS_DEBUG}"    )
endif()
if (CMAKE_BUILD_TYPE STREQUAL "Release")
   message ( " .. C compiler flags Release ... = ${CMAKE_C_FLAGS_RELEASE}"  )
endif()
message ( " --------------------------------------------------------"    )
message ( " Project configuration"                                       )
message ( " .. Project name ............... = ${PROJECT_NAME}"           )
message ( " .. Project version ............ = ${PROJECT_VERSION}"        )
message ( " .. Build shared libraries ..... = ${BUILD_SHARED_LIBS}"      )
message ( " .. Sciamachy CKD directory .... = ${NADC_TOOLS_DATADIR}"     )
message ( " .. Installation prefix ........ = ${CMAKE_INSTALL_PREFIX}"   )
message ( " --------------------------------------------------------"    )
message ( " Processing tools & libraries"                                )
message ( " .. HDF5 library ............... = ${HDF5_C_LIBRARIES}"       )
message ( " .. HDF5 includes .............. = ${HDF5_INCLUDE_DIRS}"      )
message ( " .. HDF5 library version ....... = ${HDF5_VERSION}"           )
message ( " .. NETCDF library ............. = ${NETCDF_LIBRARIES}"       )
message ( " .. NETCDF library version ..... = ${NETCDF_VERSION}"         )
if (PGSQL_FOUND)
   message ( " .. PSQL library ............... = ${PGSQL_LIBRARY}"          )
   message ( " .. PSQL includes .............. = ${PGSQL_INCLUDE_DIR}"      )
   message ( " .. PSQL library version ....... = ${PGSQL_VERSION}"          )
else ()
   message ( " .. PSQL library ............... = not installed"             )
endif ()
if (IDL_FOUND)
   message ( " .. IDL includes ............... = ${IDL_INCLUDE_DIR}"        )
else ()
   message ( " .. IDL full path .............. = not installed"             )
endif ()
if (PPF_FOUND)
   message ( " .. PPF libraries .............. = ${PPF_LIBRARIES}"          )
   message ( " .. PPF includes ............... = ${PPF_INCLUDE_DIR}"        )
else ()
   message ( " .. PPF libraries .............. = not installed"             )
endif ()
message ( " ========================================================"    )
endif (CONFIGURE_SUMMARY)
