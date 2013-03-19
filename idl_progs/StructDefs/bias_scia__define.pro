PRO bias_scia__define
compile_opt idl2,hidden

struct = { bias_scia ,$
           mjd              : {mjd_scia} ,$
           quality          : 0b  ,$
           dummy            : 0b  ,$
           hghtflag         : 0us ,$
           vcdflag          : 0us ,$
           intg_time        : 0us ,$
           numfitp          : 0us ,$
           numsegm          : 0us ,$
           numiter          : 0us ,$
           dsrlen           : 0ul ,$
           hght             : 0.0 ,$
           errhght          : 0.0 ,$
           vcd              : 0.0 ,$
           errvcd           : 0.0 ,$
           closure          : 0.0 ,$
           errclosure       : 0.0 ,$
           rms              : 0.0 ,$
           chi2             : 0.0 ,$
           goodness         : 0.0 ,$
           cutoff           : 0.0 ,$
           corrpar          : PTR_NEW()  $ ; float * (pointer)
         }
END
