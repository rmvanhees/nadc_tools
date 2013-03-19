PRO POLV_SCIA__DEFINE
compile_opt idl2,hidden

struct = { POLV_SCIA, $
           Q         :  fltarr(12) ,$
           error_Q   :  fltarr(12) ,$
           U         :  fltarr(12) ,$
           error_U   :  fltarr(12) ,$
           rep_wv    :  fltarr(13) ,$
           gdf       :  {gdf_para} ,$
           intg_time :  0us        $
         }
END
