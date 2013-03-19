PRO doas_scia__define
compile_opt idl2,hidden

struct = { doas_scia ,$
           mjd              : {mjd_scia} ,$
           quality          : 0b  ,$
           dummy            : 0b  ,$
           vcdflag          : 0us ,$
           escflag          : 0us ,$
           amfflag          : 0us ,$
           intg_time        : 0us ,$
           numfitp          : 0us ,$
           numiter          : 0us ,$
           dsrlen           : 0ul ,$
           vcd              : 0.0 ,$
           errvcd           : 0.0 ,$
           esc              : 0.0 ,$
           erresc           : 0.0 ,$
           rms              : 0.0 ,$
           chi2             : 0.0 ,$
           goodness         : 0.0 ,$
           amfgnd           : 0.0 ,$
           amfcld           : 0.0 ,$
           reflgnd          : 0.0 ,$
           reflcld          : 0.0 ,$
           refl             : 0.0 ,$
           corrpar          : PTR_NEW() $ ; float * (pointer)
         }
END
