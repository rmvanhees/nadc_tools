PRO ppg_scia__define
compile_opt idl2,hidden

struct = { ppg_scia ,$
           gain_fact      : fltarr(!nadc.sciencePixels) ,$
           etalon_fact    : fltarr(!nadc.sciencePixels) ,$
           etalon_resid   : fltarr(!nadc.sciencePixels) ,$
           wls_deg_fact   : fltarr(!nadc.sciencePixels) ,$
           bad_pixel      : bytarr(!nadc.sciencePixels)  $
         }
END
