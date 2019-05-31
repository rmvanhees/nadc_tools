## set version of library
file (READ VERSION _version_contents)
string (REGEX REPLACE ".*#define[ \t]+version_major[ \t]+([0-9]*).*$"
       "\\1" NADC_VERS_MAJOR ${_version_contents})
string (REGEX REPLACE ".*#define[ \t]+version_minor[ \t]+([0-9]*).*$"
       "\\1" NADC_VERS_MINOR ${_version_contents})
string (REGEX REPLACE ".*#define[ \t]+version_release[ \t]+([0-9]*).*$"
       "\\1" NADC_VERS_RELEASE ${_version_contents})
set (LIB_VERS "${NADC_VERS_MAJOR}.${NADC_VERS_MINOR}.${NADC_VERS_RELEASE}")

## define source-files
set (NADC_ADAGUC_SRCS
    adaguc_init_param.c 
    adaguc_version.c
)

set (NADC_ENVI_SRCS
    envi_get_dsd_index.c 
    envi_pds_dsd.c 
    envi_pds_info.c 
    envi_pds_mph.c
    envi_wr_ascii_dsd.c 
    envi_wr_ascii_mph.c
)
set (NADC_COMMON_SRCS
    nadc_akima.c 
    nadc_alloc.c
    nadc_binsearch.c
    nadc_bits.c
    nadc_biweight.c
    nadc_check_for_saa.c 
    nadc_copyright.c
    nadc_date.c 
    nadc_error.c 
    nadc_ext_h5_array.c
    nadc_file_equal.c 
    nadc_file_exists.c 
    nadc_file_size.c 
    nadc_fit.c
    nadc_flip.c
    nadc_gettime.c
    nadc_hdf5_api.c 
    nadc_info_screen.c
    nadc_interpol.c 
    nadc_medfit.c
    nadc_params.c
    nadc_pytable_api.c
    nadc_receivedate.c 
    nadc_select.c 
    nadc_sigmaclipped.c
    nadc_string.c
    nadc_usrindx.c 
    nadc_usrinp.c
    nadc_version.c
    nadc_wr_ascii.c 
    nadc_xml_metadb.c
)

set (NADC_SRCS
    ${NADC_ADAGUC_SRCS}
    ${NADC_ENVI_SRCS}
    ${NADC_COMMON_SRCS}
)

#-----------------------------------------------------------------------------
# Set-up for NADC_LIB
#-----------------------------------------------------------------------------
## Compiler instructions
add_library (${NADC_LIB_TARGET} ${LIB_TYPE} ${NADC_SRCS})

## Linker instructions
target_link_libraries(${NADC_LIB_TARGET} ${HDF5_C_LIBRARIES} m)

IF (${LIB_TYPE} MATCHES "SHARED")
   SET_TARGET_PROPERTIES (${NADC_LIB_TARGET} PROPERTIES SOVERSION ${LIB_VERS})
ENDIF (${LIB_TYPE} MATCHES "SHARED")

## Installation instructions
install (
  TARGETS ${NADC_LIB_TARGET}
  LIBRARY DESTINATION lib COMPONENT libraries
  ARCHIVE DESTINATION lib COMPONENT libraries
)
