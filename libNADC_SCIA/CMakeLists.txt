include (SetSoVersion)

## set version of library
file (READ VERSION _version_contents)
string (REGEX REPLACE ".*#define[ \t]+version_major[ \t]+([0-9]*).*$"
       "\\1" SCIA_VERS_MAJOR ${_version_contents})
string (REGEX REPLACE ".*#define[ \t]+version_minor[ \t]+([0-9]*).*$"
       "\\1" SCIA_VERS_MINOR ${_version_contents})
string (REGEX REPLACE ".*#define[ \t]+version_release[ \t]+([0-9]*).*$"
       "\\1" SCIA_VERS_RELEASE ${_version_contents})
set (LIB_VERS "${SCIA_VERS_MAJOR}.${SCIA_VERS_MINOR}.${SCIA_VERS_RELEASE}")

## define pre-compiler flags
add_definitions (-DDATA_DIR="${NADC_TOOLS_DATADIR}")

if (PGSQL_FOUND)
   add_definitions (-D_WITH_SQL)
endif

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
    scia_lv0_h5_info.c
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
    get_scia_lv1c_mean.c
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
    scia_lv2_rd_bias.c
    scia_lv2_rd_cld.c
    scia_lv2_rd_doas.c
    scia_lv2_rd_geo.c
    scia_lv2_rd_sph.c
    scia_lv2_rd_sqads.c
    scia_lv2_rd_state.c
    scia_lv2_wr_ascii_ads.c
    scia_lv2_wr_ascii_mds.c
    scia_lv2_wr_ascii_sph.c
    scia_lv2_wr_h5_bias.c
    scia_lv2_wr_h5_cld.c
    scia_lv2_wr_h5_doas.c
    scia_lv2_wr_h5_geo.c
    scia_lv2_wr_h5_sph.c
    scia_lv2_wr_h5_sqads.c
    scia_lv2_wr_h5_state.c
    scia_ol2_pds_sph.c
    scia_ol2_rd_cld.c
    scia_ol2_rd_geo.c
    scia_ol2_rd_lcld.c
    scia_ol2_rd_lfit.c
    scia_ol2_rd_nfit.c
    scia_ol2_rd_sqads.c
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
)

set (NADC_SCIA_CAL_SRCS
    CAL/scia_lv1_cal.c
    CAL/scia_lv1c_cal.c
    CAL/scia_lv1c_scale.c
    CAL/scia_lv1_mfactor_srs.c
    CAL/scia_lv1_patch_mds.c
    CAL/scia_rd_h5_mem.c
    CAL/scia_rd_h5_nlin.c
    CAL/scia_rd_h5_psp.c
    CAL/scia_rd_h5_rsp.c
    CAL/scia_rd_h5_stray.c
    CAL/scia_rd_mfactor.c
    CAL/CalibModules/calibAtbdDark.c
    CAL/CalibModules/calibAtbdMemLin.c
    CAL/CalibModules/calibAtbdPPG.c
    CAL/CalibModules/calibCalcWave.c
    CAL/CalibModules/calibCorrEtalon.c
    CAL/CalibModules/calibCorrPol.c
    CAL/CalibModules/calibCorrRad.c
    CAL/CalibModules/calibCorrRefl.c
    CAL/CalibModules/calibCorrStray.c
    CAL/CalibModules/calibFlagBDPM.c
    CAL/CalibModules/calibSronDark.c
    CAL/CalibModules/calibSronMemLin.c
    CAL/CalibModules/calibSronNoise.c
    CAL/CalibModules/calibSronPPG.c
    CAL/CalibModules/calibSronTrans.c
    CAL/CalibModules/calibStateDark.c
    CAL/SDMF/sdmf_array.c
    CAL/SDMF/sdmf_clusConf.c
    CAL/SDMF/sdmf_dark.c
    CAL/SDMF/sdmf_get_bdpm.c
    CAL/SDMF/sdmf_get_fileEntry.c
    CAL/SDMF/sdmf_get_fitteddark.c
    CAL/SDMF/sdmf_get_orbitaldark.c
    CAL/SDMF/sdmf_get_ppg.c
    CAL/SDMF/sdmf_get_statedark.c
    CAL/SDMF/sdmf_get_transmission.c
    CAL/SDMF/sdmf_hist.c
    CAL/SDMF/sdmf_meta.c
    CAL/SDMF/sdmf_orbitPhaseDiff.c
    CAL/SDMF/sdmf_path.c
    CAL/SDMF/sdmf_pt_db.c
    CAL/SDMF/sdmf_simudark.c
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

#-----------------------------------------------------------------------------
# Set-up for NADC_SCIA_LIB
#-----------------------------------------------------------------------------
## Compiler instructions
add_library (${NADC_SCIA_LIB_TARGET} ${LIB_TYPE} ${NADC_SCIA_SRCS})

## Linker instructions
target_link_libraries (${NADC_SCIA_LIB_TARGET} ${NADC_LIB_TARGET})

NADC_SET_LIB_OPTIONS (${NADC_SCIA_LIB_TARGET} ${NADC_SCIA_LIB_NAME} 
		     ${LIB_TYPE} ${LIB_VERS})
SET_TARGET_PROPERTIES (${NADC_SCIA_LIB_TARGET} PROPERTIES FOLDER libraries)

#-----------------------------------------------------------------------------
# Set-up for NADC_SCIA_CAL_LIB
#-----------------------------------------------------------------------------
## Compiler instructions
add_library (${NADC_SCIA_CAL_LIB_TARGET} ${LIB_TYPE} 
	    ${NADC_SCIA_SRCS} ${NADC_SCIA_CAL_SRCS})

## Linker instructions
target_link_libraries (${NADC_SCIA_CAL_LIB_TARGET} ${NADC_LIB_TARGET})

NADC_SET_LIB_OPTIONS (${NADC_SCIA_CAL_LIB_TARGET} ${NADC_SCIA_CAL_LIB_NAME} 
		     ${LIB_TYPE} ${LIB_VERS})
SET_TARGET_PROPERTIES (${NADC_SCIA_CAL_LIB_TARGET} PROPERTIES FOLDER libraries)

## Installation instructions
install (
  TARGETS ${NADC_SCIA_LIB_TARGET} ${NADC_SCIA_CAL_LIB_TARGET}
  LIBRARY DESTINATION lib COMPONENT libraries
  ARCHIVE DESTINATION lib COMPONENT libraries
  RUNTIME DESTINATION bin COMPONENT libraries
)
