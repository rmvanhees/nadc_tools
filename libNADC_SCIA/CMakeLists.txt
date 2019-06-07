## set version of library
file (READ VERSION _version_contents)
string (REGEX REPLACE ".*#define[ \t]+version_major[ \t]+([0-9]*).*$"
       "\\1" SCIA_VERS_MAJOR ${_version_contents})
string (REGEX REPLACE ".*#define[ \t]+version_minor[ \t]+([0-9]*).*$"
       "\\1" SCIA_VERS_MINOR ${_version_contents})
string (REGEX REPLACE ".*#define[ \t]+version_release[ \t]+([0-9]*).*$"
       "\\1" SCIA_VERS_RELEASE ${_version_contents})
set (LIB_VERS "${SCIA_VERS_MAJOR}.${SCIA_VERS_MINOR}.${SCIA_VERS_RELEASE}")

if (${LIB_TYPE} MATCHES "SHARED")
   set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif (${LIB_TYPE} MATCHES "SHARED")

## define pre-compiler flags
add_definitions (-DDATA_DIR="${NADC_TOOLS_DATADIR}")

if (PGSQL_FOUND)
   add_definitions (-D_WITH_SQL)
endif (PGSQL_FOUND)

TEST_BIG_ENDIAN(BIGENDIAN)
if (NOT ${BIGENDIAN})
   add_definitions (-D_SWAP_TO_LITTLE_ENDIAN)
endif (NOT ${BIGENDIAN})

## define source-files
set (SCIA_LV0_SRCS
    get_scia_lv0c_mds.c
    get_scia_lv0_mds.c
    get_scia_lv0_mds_hk.c
    get_scia_lv0_mds_info.c
    scia_lv0_mds_info.c
    scia_lv0_pds_sph.c
    scia_lv0_rd_mds.c
    scia_lv0_select.c
    scia_lv0_wr_ascii_info.c
    scia_lv0_wr_ascii_mds.c
    scia_lv0_wr_ascii_sph.c
    scia_lv0_wr_h5_mds.c
    scia_lv0_wr_h5_sph.c
    scia_lv0_wr_mds.c
)

set (SCIA_LV1_SRCS
    get_scia_lv1c_geo.c
    get_scia_lv1c_mds.c
    get_scia_lv1c_pmd.c
    get_scia_lv1c_polV.c
    is_scia_lv1c.c
    scia_lv1_chan2clus.c
    scia_lv1_corr_los.c
    scia_lv1c_pds_calopt.c
    scia_lv1c_wr_ascii_calopt.c
    scia_lv1_free_mds.c
    scia_lv1_h5_mds.c
    scia_lv1_lib_dsd.c
    scia_lv1_pds_asfp.c
    scia_lv1_pds_aux.c
    scia_lv1_pds_base.c
    scia_lv1_pds_clcp.c
    scia_lv1_pds_dark.c
    scia_lv1_pds_ekd.c
    scia_lv1_pds_lcpn.c
    scia_lv1_pds_pmd.c
    scia_lv1_pds_ppg.c
    scia_lv1_pds_ppgn.c
    scia_lv1_pds_psp.c
    scia_lv1_pds_rsp.c
    scia_lv1_pds_scp.c
    scia_lv1_pds_scpn.c
    scia_lv1_pds_sfp.c
    scia_lv1_pds_sip.c
    scia_lv1_pds_sph.c
    scia_lv1_pds_sqads.c
    scia_lv1_pds_srs.c
    scia_lv1_pds_srsn.c
    scia_lv1_pds_state.c
    scia_lv1_pds_vlcp.c
    scia_lv1_rd_mds.c
    scia_lv1_scale_mds.c
    scia_lv1_select.c
    scia_lv1_wr_ascii_ads.c
    scia_lv1_wr_ascii_gads.c
    scia_lv1_wr_ascii_mds.c
    scia_lv1_wr_ascii_sph.c
    scia_lv1_wr_h5_asfp.c
    scia_lv1_wr_h5_aux.c
    scia_lv1_wr_h5_base.c
    scia_lv1_wr_h5_clcp.c
    scia_lv1_wr_h5_dark.c
    scia_lv1_wr_h5_ekd.c
    scia_lv1_wr_h5_lcpn.c
    scia_lv1_wr_h5_pmd.c
    scia_lv1_wr_h5_ppg.c
    scia_lv1_wr_h5_ppgn.c
    scia_lv1_wr_h5_psp.c
    scia_lv1_wr_h5_rsp.c
    scia_lv1_wr_h5_scp.c
    scia_lv1_wr_h5_scpn.c
    scia_lv1_wr_h5_sfp.c
    scia_lv1_wr_h5_sip.c
    scia_lv1_wr_h5_sph.c
    scia_lv1_wr_h5_sqads.c
    scia_lv1_wr_h5_srs.c
    scia_lv1_wr_h5_srsn.c
    scia_lv1_wr_h5_state.c
    scia_lv1_wr_h5_vlcp.c
    scia_lv1_wr_mds.c
)

set (SCIA_LV2_SRCS
    scia_ol2_pds_sph.c
    scia_ol2_rd_cld.c
    scia_ol2_rd_geo.c
    scia_ol2_rd_lcld.c
    scia_ol2_rd_lfit.c
    scia_ol2_rd_nfit.c
    scia_ol2_rd_sqads.c
    scia_ol2_rd_state.c
    scia_ol2_wr_ascii_ads.c
    scia_ol2_wr_ascii_mds.c
    scia_ol2_wr_ascii_sph.c
    scia_ol2_wr_h5_cld.c
    scia_ol2_wr_h5_geo.c
    scia_ol2_wr_h5_lcld.c
    scia_ol2_wr_h5_lfit.c
    scia_ol2_wr_h5_nfit.c
    scia_ol2_wr_h5_sph.c
    scia_ol2_wr_h5_sqads.c
    scia_ol2_wr_state.c
)

set (NADC_SCIA_SRCS
    get_scia_clusdef.c
    get_scia_magic_id.c
    get_scia_mds_type.c
    get_scia_quality.c
    get_scia_roe_info.c
    scia_codecalib.c
    scia_codepatch.c
    scia_cre_h5_file.c
    scia_clusDef.c
    scia_h5_structs.c
    scia_param.c
    scia_pds_lads.c
    scia_version.c
    scia_wr_ascii_lads.c
    scia_wr_h5_lads.c
    scia_wr_h5_mph.c
    scia_wr_h5_version.c
    ${SCIA_LV0_SRCS}
    ${SCIA_LV1_SRCS}
    ${SCIA_LV2_SRCS}
)

add_library (SCIA OBJECT ${NADC_SCIA_SRCS})

#-----------------------------------------------------------------------------
# Set-up for NADC_SCIA_LIB
#-----------------------------------------------------------------------------
## Compiler instructions
if (APPLE)
   set (CMAKE_SHARED_LINKER_FLAGS "-undefined dynamic_lookup")
endif (APPLE)

add_library (${NADC_SCIA_LIB_TARGET} ${LIB_TYPE} $<TARGET_OBJECTS:SCIA>)

## Linker instructions
target_link_libraries (${NADC_SCIA_LIB_TARGET} ${NADC_LIB_TARGET})

if (${LIB_TYPE} MATCHES "SHARED")
   SET_TARGET_PROPERTIES (${NADC_SCIA_LIB_TARGET} PROPERTIES 
   			 SOVERSION ${LIB_VERS})
endif (${LIB_TYPE} MATCHES "SHARED")

#-----------------------------------------------------------------------------
# Set-up for NADC_SCIA_CAL_LIB
#-----------------------------------------------------------------------------
## Compiler instructions
add_subdirectory(CAL)
add_library (${NADC_SCIA_CAL_LIB_TARGET} ${LIB_TYPE} 
	    $<TARGET_OBJECTS:SCIA>
	    $<TARGET_OBJECTS:SCIA_CAL>
	    $<TARGET_OBJECTS:MODS>
	    $<TARGET_OBJECTS:SDMF>
)

## Linker instructions
target_link_libraries (${NADC_SCIA_CAL_LIB_TARGET} ${NADC_LIB_TARGET})

if (${LIB_TYPE} MATCHES "SHARED")
   SET_TARGET_PROPERTIES (${NADC_SCIA_CAL_LIB_TARGET} PROPERTIES 
   			 SOVERSION ${LIB_VERS})
endif (${LIB_TYPE} MATCHES "SHARED")

## Installation instructions
install (
  TARGETS ${NADC_SCIA_LIB_TARGET} ${NADC_SCIA_CAL_LIB_TARGET}
  LIBRARY DESTINATION lib COMPONENT libraries
  ARCHIVE DESTINATION lib COMPONENT libraries
)
