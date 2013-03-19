PRO nfit_scia__define
compile_opt idl2,hidden

struct = { nfit_scia ,$
           mjd           : {mjd_scia} ,$
           quality       : 0b    ,$
           dummy         : 0b    ,$
           intg_time     : 0us   ,$
           numvcd        : 0us   ,$
           vcdflag       : 0us   ,$
           num_fitp      : 0us   ,$
           num_nfitp     : 0us   ,$
           numiter       : 0us   ,$
           fitflag       : 0us   ,$
           amfflag       : 0us   ,$
           dsrlen        : 0ul   ,$
           esc           : 0.0   ,$
           erresc        : 0.0   ,$
           rms           : 0.0   ,$
           chi2          : 0.0   ,$
           goodness      : 0.0   ,$
           amfgrd        : 0.0   ,$
           erramfgrd     : 0.0   ,$
           amfcld        : 0.0   ,$
           erramfcld     : 0.0   ,$
           temperature   : 0.0   ,$
           vcd           : PTR_NEW() ,$ ; float * (pointer)
           errvcd        : PTR_NEW() ,$ ; float * (pointer)
           linpars       : PTR_NEW() ,$ ; float * (pointer)
           errlinpars    : PTR_NEW() ,$ ; float * (pointer)
           lincorrm      : PTR_NEW() ,$ ; float * (pointer)
           nlinpars      : PTR_NEW() ,$ ; float * (pointer)
           errnlinpars   : PTR_NEW() ,$ ; float * (pointer)
           nlincorrm     : PTR_NEW() $  ; float * (pointer)
         }
END
