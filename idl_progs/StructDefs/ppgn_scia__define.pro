PRO ppgn_scia__define
compile_opt idl2,hidden

struct = { ppgn_scia ,$
           mjd               : {mjd_scia} ,$
           flag_mds          : 0b                      ,$
           gain_fact         : fltarr(!nadc.sciencePixels)  ,$
           etalon_fact       : fltarr(!nadc.sciencePixels)  ,$
           etalon_resid      : fltarr(!nadc.sciencePixels)  ,$
           avg_wls_spec      : fltarr(!nadc.sciencePixels)  ,$
           sd_wls_spec       : fltarr(!nadc.sciencePixels)  ,$
           bad_pixel         : bytarr(!nadc.sciencePixels)  $
         }
END
