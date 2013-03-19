PRO rsplo_scia__define
compile_opt idl2,hidden

struct = { rsplo_scia, $
           elevation    : 0.0  ,$
           azimuth      : 0.0  ,$
           sensitivity  : dblarr(!nadc.sciencePixels)  $
         }
END
