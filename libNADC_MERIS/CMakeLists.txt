## set version of library
file (READ VERSION _version_contents)
string (REGEX REPLACE ".*#define[ \t]+version_major[ \t]+([0-9]*).*$"
       "\\1" MERIS_VERS_MAJOR ${_version_contents})
string (REGEX REPLACE ".*#define[ \t]+version_minor[ \t]+([0-9]*).*$"
       "\\1" MERIS_VERS_MINOR ${_version_contents})
string (REGEX REPLACE ".*#define[ \t]+version_release[ \t]+([0-9]*).*$"
       "\\1" MERIS_VERS_RELEASE ${_version_contents})
set (LIB_VERS "${MERIS_VERS_MAJOR}.${MERIS_VERS_MINOR}.${MERIS_VERS_RELEASE}")

## define pre-compiler flags
TEST_BIG_ENDIAN(BIGENDIAN)
if (NOT ${BIGENDIAN})
   add_definitions (-D_SWAP_TO_LITTLE_ENDIAN)
endif (NOT ${BIGENDIAN})

## define source-files
set (MERIS_LV2_SRCS
    meris_rr2_rd_mds_13.c
    meris_rr2_rd_mds_14.c
    meris_rr2_rd_mds_15.c
    meris_rr2_rd_mds_16.c
    meris_rr2_rd_mds_17.c
    meris_rr2_rd_mds_18.c
    meris_rr2_rd_mds_19.c
    meris_rr2_rd_mds_20.c
    meris_rr2_rd_sfgi.c
    meris_rr2_rd_sqads.c
)

set (NADC_MERIS_SRCS
    meris_cre_h5_file.c
    meris_param.c
    meris_rd_sph.c
    meris_rd_tie.c
    meris_version.c
    meris_wr_ascii_sph.c
    meris_wr_ascii_tie.c
    meris_wr_h5_version.c
    ${MERIS_LV2_SRCS}
)

## Compiler instructions
add_library (${NADC_MERIS_LIB_TARGET} ${LIB_TYPE} ${NADC_MERIS_SRCS})

## Linker instructions
target_link_libraries (${NADC_MERIS_LIB_TARGET} ${NADC_LIB_TARGET})

IF (${LIB_TYPE} MATCHES "SHARED")
   SET_TARGET_PROPERTIES (${NADC_MERIS_LIB_TARGET} PROPERTIES 
   			 SOVERSION ${LIB_VERS})
ENDIF (${LIB_TYPE} MATCHES "SHARED")

## Installation instructions
install (
  TARGETS ${NADC_MERIS_LIB_TARGET}
  LIBRARY DESTINATION lib COMPONENT libraries
  ARCHIVE DESTINATION lib COMPONENT libraries
)
