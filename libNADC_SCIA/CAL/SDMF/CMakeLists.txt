set (NADC_SCIA_SDMF_SRCS
    sdmf_array.c
    sdmf_clusConf.c
    sdmf_dark.c
    sdmf_get_bdpm.c
    sdmf_get_fileEntry.c
    sdmf_get_fitteddark.c
    sdmf_get_orbitaldark.c
    sdmf_get_ppg.c
    sdmf_get_statedark.c
    sdmf_get_transmission.c
    sdmf_hist.c
    sdmf_meta.c
    sdmf_orbitPhaseDiff.c
    sdmf_path.c
    sdmf_pt_db.c
    sdmf_simudark.c
)

add_library (SDMF OBJECT ${NADC_SCIA_SDMF_SRCS})
