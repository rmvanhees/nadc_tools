set (NADC_SCIA_CAL_SRCS
    scia_lv1_cal.c
    scia_lv1c_cal.c
    scia_lv1c_scale.c
    scia_lv1_mfactor_srs.c
    scia_lv1_patch_mds.c
    scia_rd_h5_mem.c
    scia_rd_h5_nlin.c
    scia_rd_h5_psp.c
    scia_rd_h5_rsp.c
    scia_rd_h5_stray.c
    scia_rd_mfactor.c
)

add_subdirectory(CalibModules)
add_subdirectory(SDMF)

add_library(SCIA_CAL OBJECT ${NADC_SCIA_CAL_SRCS})
