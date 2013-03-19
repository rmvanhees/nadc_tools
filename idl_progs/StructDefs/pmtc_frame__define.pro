PRO pmtc_frame__define
compile_opt idl2,hidden

struct = { pmtc_frame, $
           bcp        : replicate({aux_bcp}, !nadc.numLv0AuxBCP) ,$
           bench_rad  : 0us      ,$
           bench_elv  : 0us      ,$
           bench_az   : 0us      $
         }
END
