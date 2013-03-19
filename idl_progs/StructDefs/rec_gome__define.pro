PRO REC_GOME__DEFINE
compile_opt idl2,hidden

CHANNEL_SIZE = 1024

struct = { REC_GOME ,$
           pixel_flags  : 0l  ,$
           indx_psp     : 0s  ,$
           indx_pcd     : 0s  ,$
           integration  : fltarr(2) ,$
           wave         : fltarr(CHANNEL_SIZE) ,$
           data         : fltarr(CHANNEL_SIZE) $
         }

END
