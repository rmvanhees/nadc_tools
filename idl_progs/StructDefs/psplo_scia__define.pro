PRO psplo_scia__define
compile_opt idl2,hidden

struct = { psplo_scia ,$
           elevation : 0.0  ,$
           azimuth   : 0.0  ,$
           mu2       : dblarr(!nadc.sciencePixels)  ,$
           mu3       : dblarr(!nadc.sciencePixels)  $
         }
END
