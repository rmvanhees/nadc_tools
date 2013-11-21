include (utilities)

set (IDL_COMMON_SRCS
    convert_to_runs.pro 
    defs_nadc.pro 
    get_scia_clusdef.pro 
    get_scia_level.pro 
    get_scia_orbitphase.pro 
    get_scia_quality.pro 
    get_scia_roe_eclipse.pro 
    get_scia_roe_info.pro
    lib_name.pro
    nadc_err_clear.pro
    nadc_err_trace.pro
    readbinarystructure.pro
    scia_apply_memcorr.pro
    scia_apply_nlincorr.pro
    scia_clusdef.pro
    scia_date2mjd.pro
    scia_decode_memcorr.pro
    scia_decode_nlcorr.pro
    scia_encode_memcorr.pro
    scia_encode_nlcorr.pro
    scia_fclose.pro
    scia_fopen.pro
    scia_get_orbit_params.pro
    scia_hl_open.pro
    scia_mjd2date.pro
    scia_mjd2time.pro
    scia_orbit_to_jd.pro
    scia_rd_dsd.pro
    scia_rd_h5_memcorr.pro
    scia_rd_h5_nlcorr.pro
    scia_rd_h5_straylight.pro
    scia_rd_lads.pro
    scia_rd_mfactor.pro
    scia_rd_mph.pro
    scia_wr_h5_memcorr.pro
    scia_wr_h5_nlcorr2.pro
    scia_wr_h5_nlcorr.pro
    scia_wr_h5_straylight.pro
)

set (IDL_LEVEL0_SRCS
    get_lv0_det_size.pro
    get_lv0_mds_collo.pro
    get_lv0_mds_data.pro
    get_lv0_mds_hk.pro
    get_lv0_mds_state.pro
    get_lv0_mds_time.pro
    get_lv0_state_temp.pro
    scia_hl_lv0_mds.pro
    scia_hl_lv0_pixels.pro
    scia_lv0_free_det.pro
    scia_lv0_mds_trend.pro
    scia_lv0_rd_aux.pro
    scia_lv0_rd_det.pro
    scia_lv0_rd_mds_info.pro
    scia_lv0_rd_pmd.pro
    scia_lv0_rd_sph.pro
)

set (IDL_LEVEL1_SRCS
    cmp_scia_cal.pro
    get_lv1_gome_spec.pro
    get_lv1_mds_data.pro
    get_lv1_mds_state.pro
    get_lv1_mds_time.pro
    get_scia_mds1_data.pro
    get_scia_mds1_wave.pro
    gome_lv1_free_fcd.pro
    gome_lv1_pcd_pmd.pro
    gome_lv1_rd_bdr.pro
    gome_lv1_rd_fcd.pro
    gome_lv1_rd_fsr.pro
    gome_lv1_rd_pcd.pro
    gome_lv1_rd_smbdr.pro
    gome_lv1_rd_smcd.pro
    gome_lv1_rd_sph.pro
    gome_lv1_smcd_pmd.pro
    scia_hl_lv1_mds.pro
    scia_hl_lv1_pixels.pro
    scia_lv1c_rd_calopt.pro
    scia_lv1c_rd_mds.pro
    scia_lv1_free_mds.pro
    scia_lv1_free_pmd.pro
    scia_lv1_free_polv.pro
    scia_lv1_rd_asfp.pro
    scia_lv1_rd_aux.pro
    scia_lv1_rd_base.pro
    scia_lv1_rd_clcp.pro
    scia_lv1_rd_dark.pro
    scia_lv1_rd_ekd.pro
    scia_lv1_rd_lcpn.pro
    scia_lv1_rd_mds.pro
    scia_lv1_rd_pmd.pro
    scia_lv1_rd_ppgn.pro
    scia_lv1_rd_ppg.pro
    scia_lv1_rd_pspl.pro
    scia_lv1_rd_pspn.pro
    scia_lv1_rd_pspo.pro
    scia_lv1_rd_rspl.pro
    scia_lv1_rd_rspn.pro
    scia_lv1_rd_rspo.pro
    scia_lv1_rd_scpn.pro
    scia_lv1_rd_scp.pro
    scia_lv1_rd_sfp.pro
    scia_lv1_rd_sip.pro
    scia_lv1_rd_sph.pro
    scia_lv1_rd_sqads.pro
    scia_lv1_rd_srsn.pro
    scia_lv1_rd_srs.pro
    scia_lv1_rd_state.pro
    scia_lv1_rd_vlcp.pro
)

set (IDL_LEVEL2_SRCS
    scia_lv2_free_bias.pro
    scia_lv2_free_cld.pro
    scia_lv2_free_doas.pro
    scia_lv2_rd_bias.pro
    scia_lv2_rd_cld.pro
    scia_lv2_rd_doas.pro
    scia_lv2_rd_geo.pro
    scia_lv2_rd_sph.pro
    scia_lv2_rd_sqads.pro
    scia_lv2_rd_state.pro
    scia_ol2_free_cld.pro
    scia_ol2_free_lfit.pro
    scia_ol2_free_nfit.pro
    scia_ol2_rd_cld.pro
    scia_ol2_rd_lcld.pro
    scia_ol2_rd_lfit.pro
    scia_ol2_rd_lgeo.pro
    scia_ol2_rd_nfit.pro
    scia_ol2_rd_ngeo.pro
    scia_ol2_rd_sph.pro
    scia_ol2_rd_sqads.pro
)

set (IDL_SDMF_SRCS
    get_sdmf_path.pro
    get_sdmf_phasediff.pro
    sdmf_read_calib.pro
    sdmf_read_channelevolution.pro
    sdmf_read_extract_l1b.pro
    sdmf_read_fitteddark.pro
    sdmf_read_ll.pro
    sdmf_read_orbitdark.pro
    sdmf_read_pixelevolution.pro
    sdmf_read_pixelmask.pro
    sdmf_read_pmd_dark.pro
    sdmf_read_ppg.pro
    sdmf_read_simudark.pro
    sdmf_read_smr.pro
    sdmf_read_statedark.pro
    sdmf_read_sun.pro
    sdmf_read_transmission.pro
)

set (NADC_IDL_EXMPL
    test_gome_lv1.idl
    test_scia_lv0.idl
    test_scia_lv1c.idl
    test_scia_lv1.idl
    test_scia_ol2.idl
)

set (NADC_IDL_DEFS
    StructDefs/asfp_scia__define.pro
    StructDefs/aux_bcp__define.pro
    StructDefs/aux_scia__define.pro
    StructDefs/base_scia__define.pro
    StructDefs/bias_scia__define.pro
    StructDefs/calopt_scia__define.pro
    StructDefs/clcp_scia__define.pro
    StructDefs/cld_scia__define.pro
    StructDefs/cld_scia_ol__define.pro
    StructDefs/clusdef_scia__define.pro
    StructDefs/clus_scia__define.pro
    StructDefs/coord_scia__define.pro
    StructDefs/dark_scia__define.pro
    StructDefs/darksignal__define.pro
    StructDefs/data_hdr__define.pro
    StructDefs/doas_scia__define.pro
    StructDefs/dsd_scia__define.pro
    StructDefs/ekd_scia__define.pro
    StructDefs/esigc_scia__define.pro
    StructDefs/esig_scia__define.pro
    StructDefs/fcd_gome__define.pro
    StructDefs/fep_hdr__define.pro
    StructDefs/fsr1_gome__define.pro
    StructDefs/gdf_para__define.pro
    StructDefs/geoc_scia__define.pro
    StructDefs/geol_scia__define.pro
    StructDefs/geon_scia__define.pro
    StructDefs/geo_scia__define.pro
    StructDefs/glr1_gome__define.pro
    StructDefs/lads_scia__define.pro
    StructDefs/lcld_scia__define.pro
    StructDefs/lcpn_scia__define.pro
    StructDefs/lfit_scia__define.pro
    StructDefs/lgeo_scia__define.pro
    StructDefs/mds0_aux__define.pro
    StructDefs/mds0_det__define.pro
    StructDefs/mds0_info__define.pro
    StructDefs/mds0_pmd__define.pro
    StructDefs/mds1c_pmd__define.pro
    StructDefs/mds1c_polv__define.pro
    StructDefs/mds1c_scia__define.pro
    StructDefs/mds1_scia__define.pro
    StructDefs/mjd_scia__define.pro
    StructDefs/mph_scia__define.pro
    StructDefs/nfit_scia__define.pro
    StructDefs/ngeo_scia__define.pro
    StructDefs/packet_hdr__define.pro
    StructDefs/pcd_gome__define.pro
    StructDefs/pmd_data__define.pro
    StructDefs/pmd_gome__define.pro
    StructDefs/pmd_scia__define.pro
    StructDefs/pmd_src__define.pro
    StructDefs/pmtc_frame__define.pro
    StructDefs/pmtc_hdr__define.pro
    StructDefs/polv_scia__define.pro
    StructDefs/ppgn_scia__define.pro
    StructDefs/ppg_scia__define.pro
    StructDefs/psplo_scia__define.pro
    StructDefs/pspn_scia__define.pro
    StructDefs/rec_gome__define.pro
    StructDefs/rsigc_scia__define.pro
    StructDefs/rsig_scia__define.pro
    StructDefs/rsplo_scia__define.pro
    StructDefs/rspn_scia__define.pro
    StructDefs/scpn_scia__define.pro
    StructDefs/scp_scia__define.pro
    StructDefs/sdmf_dark__define.pro
    StructDefs/sdmf_hist__define.pro
    StructDefs/sdmf_mask__define.pro
    StructDefs/sdmf_meta__define.pro
    StructDefs/sdmf_ppg__define.pro
    StructDefs/sdmf_pt_geo__define.pro
    StructDefs/sdmf_pt_meta__define.pro
    StructDefs/sdmf_simudark__define.pro
    StructDefs/sdmf_smr_meta__define.pro
    StructDefs/sdmf_statedark__define.pro
    StructDefs/sdmf_trans__define.pro
    StructDefs/sfp_scia__define.pro
    StructDefs/sip_scia__define.pro
    StructDefs/smcd_gome__define.pro
    StructDefs/sph0_scia__define.pro
    StructDefs/sph1_gome__define.pro
    StructDefs/sph1_scia__define.pro
    StructDefs/sph2_scia__define.pro
    StructDefs/sph_scia_ol__define.pro
    StructDefs/sqads2_scia__define.pro
    StructDefs/sqads_scia__define.pro
    StructDefs/sqads_scia_ol__define.pro
    StructDefs/srsn_scia__define.pro
    StructDefs/srs_scia__define.pro
    StructDefs/state1_scia__define.pro
    StructDefs/state2_scia__define.pro
    StructDefs/vlcp_scia__define.pro
)

set (NADC_IDL_SRCS
    ${IDL_COMMON_SRCS}
    ${IDL_LEVEL0_SRCS}
    ${IDL_LEVEL1_SRCS}
    ${IDL_LEVEL2_SRCS}
    ${IDL_SDMF_SRCS}
)

set (NADC_SETUP_SH_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/setup_nadc.sh)
set (NADC_SETUP_IDL_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/setup_nadc.idl)
set (NADC_SETUP_ARGS_LIST
    ${NADC_SETUP_IDL_SCRIPT}
    ${CMAKE_INSTALL_PREFIX}/share/nadc_idl
    ${NADC_TOOLS_DATADIR}
    ${NADC_IDL_SRCS}
)
TO_LIST_SPACES (NADC_SETUP_ARGS_LIST NADC_SETUP_ARGS)

add_custom_command (
		   OUTPUT   ${NADC_SETUP_IDL_SCRIPT}
		   COMMAND  ${NADC_SETUP_SH_SCRIPT}
		   ARGS     ${NADC_SETUP_ARGS}
		   DEPENDS  ${NADC_IDL_SRCS}
		   COMMENT  "setup_nadc.sh" VERBATIM
)

add_custom_target (CreateIdlSetupScript ALL echo
		  DEPENDS ${NADC_SETUP_IDL_SCRIPT}
)

## Installation instructions
install (
   FILES ${NADC_SETUP_IDL_SCRIPT} ${NADC_IDL_SRCS} DESTINATION share/nadc_idl
)

install (
   FILES ${NADC_IDL_DEFS} DESTINATION share/nadc_idl/StructDefs
)

install (
   FILES ${NADC_IDL_EXMPL} DESTINATION share/nadc_idl/Examples
)
