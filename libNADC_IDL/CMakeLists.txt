## set version of library
file (READ VERSION _version_contents)
string (REGEX REPLACE ".*#define[ \t]+version_major[ \t]+([0-9]*).*$"
       "\\1" GOME_VERS_MAJOR ${_version_contents})
string (REGEX REPLACE ".*#define[ \t]+version_minor[ \t]+([0-9]*).*$"
       "\\1" GOME_VERS_MINOR ${_version_contents})
string (REGEX REPLACE ".*#define[ \t]+version_release[ \t]+([0-9]*).*$"
       "\\1" GOME_VERS_RELEASE ${_version_contents})
set (LIB_VERS "${GOME_VERS_MAJOR}.${GOME_VERS_MINOR}.${GOME_VERS_RELEASE}")

## define pre-compiler flags
add_definitions (-DDATA_DIR="${NADC_TOOLS_DATADIR}")

## Include directories for header files
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${HDF5_INCLUDE_DIRS}
  ${IDL_INCLUDE_DIR}
)

## Linker instructions
set (CMAKE_LINKER_FLAGS "-Wl,-Bsymbolic")
set (CMAKE_EXE_LINKER_FLAGS "-Wl,-Bsymbolic")
set (CMAKE_MODULE_LINKER_FLAGS "-Wl,-Bsymbolic")

## define source-files
set (NADC_IDL_SRCS
    gome_lv1_idl.c
    scia_gen_idl.c
    scia_h5_idl.c
    scia_lv0_idl.c
    scia_lv1_idl.c
    scia_lv2_idl.c
    sdmf_h5_idl.c
)

if (PPF_FOUND)
   list (APPEND NADC_IDL_SRCS scia_cfi_idl.c)
endif (PPF_FOUND)

#-----------------------------------------------------------------------------
# Set-up for NADC_IDL
#-----------------------------------------------------------------------------
## Compiler instructions
add_library (${NADC_IDL_LIB_TARGET} ${LIB_TYPE} ${NADC_IDL_SRCS})

if (PPF_FOUND)
   target_link_libraries (${NADC_IDL_LIB_TARGET}
   		      	 ${NADC_SCIA_CAL_LIB_TARGET}
		      	 ${NADC_GOME_LIB_TARGET}
		      	 ${NADC_LIB_TARGET}
			 ${PPF_LIBRARIES}
		      	 )
else ()
   target_link_libraries (${NADC_IDL_LIB_TARGET}
   		      	 ${NADC_SCIA_CAL_LIB_TARGET}
		      	 ${NADC_GOME_LIB_TARGET}
		      	 ${NADC_LIB_TARGET}
		      	 )
endif()

IF (${LIB_TYPE} MATCHES "SHARED")
   SET_TARGET_PROPERTIES (${NADC_IDL_LIB_TARGET} PROPERTIES 
   			 SOVERSION ${LIB_VERS})
ENDIF (${LIB_TYPE} MATCHES "SHARED")

## Installation instructions
install (
  TARGETS ${NADC_IDL_LIB_TARGET}
  LIBRARY DESTINATION lib COMPONENT libraries
  ARCHIVE DESTINATION lib COMPONENT libraries
)
