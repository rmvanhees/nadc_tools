; definition of the structure to hold Sun/Moon Calibration Records
PRO SMCD_GOME__DEFINE
compile_opt idl2,hidden

PMD_IN_GRID = 16
NUM_SPEC_BANDS = 10

ss = { smcd_gome ,$
       selected     : 0s            ,$
       indx_spec    : 0s            ,$
       indx_leak    : 0s            ,$
       indx_bands   : intarr(NUM_SPEC_BANDS) ,$
       utc_date     : 0ul           ,$
       utc_time     : 0ul           ,$
       north_sun_zen  : 0.          ,$
       north_sun_azim : 0.          ,$
       north_sm_zen   : 0.          ,$
       north_sm_azim  : 0.          ,$
       sun_or_moon    : 0.          ,$
       dark_current : 0.0           ,$
       noise_factor : 0.0           ,$
       mph0         : {mph0_gome}   ,$
       sph0         : {sph0_gome}   ,$
       ihr          : {ihr_gome}    ,$
       pmd          : replicate({pmd_gome}, PMD_IN_GRID) $
     }

END
