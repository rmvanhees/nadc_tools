set (NADC_HDRS
    nadc_common.h
    nadc_error.h
    nadc_fresco.h
    nadc_gome.h
    nadc_imap.h
    nadc_imlm.h
    nadc_scia_cal.h
    nadc_scia.h
    nadc_scia_lv0.h
    nadc_scia_lv1.h
    nadc_scia_lv2.h
    nadc_sdmf.h
    nadc_togomi.h
    nadc_tosomi.h
)

## Installation instructions
install (
   FILES ${NADC_HDRS} DESTINATION include COMPONENT headers
)
