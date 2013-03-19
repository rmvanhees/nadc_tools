PRO lcpn_scia__define
compile_opt idl2,hidden

struct = { lcpn_scia ,$
           mjd               : {mjd_scia} ,$
           mjd_last          : {mjd_scia} ,$
           flag_mds          : 0b                      ,$
           orbit_phase       : 0.0                     ,$
           obm_pmd           : fltarr(!nadc.irChannels + !nadc.numberPMD) ,$
           fpn               : fltarr(!nadc.sciencePixels) ,$
           fpn_error         : fltarr(!nadc.sciencePixels) ,$
           lc                : fltarr(!nadc.sciencePixels) ,$
           lc_error          : fltarr(!nadc.sciencePixels) ,$
           mean_noise        : fltarr(!nadc.sciencePixels) ,$
           pmd_off           : fltarr(2 * !nadc.numberPMD) ,$
           pmd_off_error     : fltarr(2 * !nadc.numberPMD) $
         }
END
