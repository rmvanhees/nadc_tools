PRO LADS_SCIA__DEFINE
compile_opt idl2,hidden

struct = { lads_scia ,$
           mjd       : {mjd_scia} ,$
           flag_mds  : 0b                       ,$
           coord     : replicate({coord_scia}, 4) $
         }
END
