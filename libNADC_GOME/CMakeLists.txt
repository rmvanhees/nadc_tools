include (SetSoVersion)

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
if (PGSQL_FOUND)
   add_definitions (-D_WITH_SQL)
endif (PGSQL_FOUND)

TEST_BIG_ENDIAN(BIGENDIAN)
if (NOT ${BIGENDIAN})
   add_definitions (-D_SWAP_TO_LITTLE_ENDIAN)
endif (NOT ${BIGENDIAN})

## define source-files
set (GOME_LV1_SRCS
    gome_lv1_bdr.c
    gome_lv1_cal_bdr.c
    gome_lv1_cal_pmd.c
    gome_lv1_pmd_geo.c
    gome_lv1_rd_bdr.c
    gome_lv1_rd_fcd.c
    gome_lv1_rd_fsr.c
    gome_lv1_rd_pcd.c
    gome_lv1_rd_smcd.c
    gome_lv1_rd_sph.c
    gome_lv1_wr_ascii.c
)

set (GOME_LV2_SRCS
    gome_lv2_rd_ddr.c
    gome_lv2_rd_fsr.c
    gome_lv2_rd_sph.c
    gome_lv2_wr_ascii.c
)

set (GOME_HDF5_SRCS
    gome_cre_h5_file.c
    gome_h5_structs.c
    gome_wr_h5_pir.c
    gome_wr_h5_version.c
    gome_lv1_wr_h5_fcd.c
    gome_lv1_wr_h5_pcd.c
    gome_lv1_wr_h5_rec.c
    gome_lv1_wr_h5_smcd.c
    gome_lv1_wr_h5_sph.c
    gome_lv2_wr_h5_ddr.c
    gome_lv2_wr_h5_irr.c
    gome_lv2_wr_h5_sph.c
)

set (NADC_GOME_SRCS
    gome_chk_size.c
    gome_codecalib.c
    gome_param.c
    gome_rd_pir.c
    gome_select.c
    gome_version.c
    ${GOME_LV1_SRCS}
    ${GOME_LV2_SRCS}
    ${GOME_HDF5_SRCS}
)

## Compiler instructions
add_library (${NADC_GOME_LIB_TARGET} ${LIB_TYPE} ${NADC_GOME_SRCS})

## Linker instructions
target_link_libraries (${NADC_GOME_LIB_TARGET} ${NADC_LIB_TARGET})

NADC_SET_LIB_OPTIONS (${NADC_GOME_LIB_TARGET} ${NADC_GOME_LIB_NAME} 
		     ${LIB_TYPE} ${LIB_VERS})
SET_TARGET_PROPERTIES (${NADC_GOME_LIB_TARGET} PROPERTIES FOLDER libraries)

## Installation instructions
install (
  TARGETS ${NADC_GOME_LIB_TARGET}
  LIBRARY DESTINATION lib COMPONENT libraries
  ARCHIVE DESTINATION lib COMPONENT libraries
)
