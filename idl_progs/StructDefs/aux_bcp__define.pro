PRO aux_bcp__define
compile_opt idl2,hidden

struct = { aux_bcp,$
           sync            : 0us     ,$ ; unsigned short
           bcps            : 0us     ,$
           flags           : 0us     ,$
           azi_encode_cntr : 0ul     ,$ ; unsigned int
           ele_encode_cntr : 0ul     ,$
           azi_cntr_error  : 0us     ,$ ; unsigned short
           ele_cntr_error  : 0us     ,$
           azi_scan_error  : 0us     ,$
           ele_scan_error  : 0us      $
         }
END
