PRO sdmf_hist__define
compile_opt idl2,hidden

MAX_NUM_HIST = 80

struct = { sdmf_hist ,$
           offset     : 0ul  ,$ 
           binsize    : 0b  ,$
           coaddf     : 0b  ,$
           count      : uintarr(MAX_NUM_HIST) ,$
           location   : uintarr(MAX_NUM_HIST) ,$
           remainder  : uintarr(MAX_NUM_HIST) $
         }
END
