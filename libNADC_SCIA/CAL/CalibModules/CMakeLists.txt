set (NADC_SCIA_MODS_SRCS
    calibAtbdDark.c
    calibAtbdMemLin.c
    calibAtbdPPG.c
    calibCalcWave.c
    calibCorrEtalon.c
    calibCorrPol.c
    calibCorrRad.c
    calibCorrRefl.c
    calibCorrStray.c
    calibFlagBDPM.c
    calibSronDark.c
    calibSronMemLin.c
    calibSronNoise.c
    calibSronPPG.c
    calibSronTrans.c
    calibStateDark.c
)

add_library (MODS OBJECT ${NADC_SCIA_MODS_SRCS})
