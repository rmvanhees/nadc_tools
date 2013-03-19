; definition of the structure to hold a Offline Level 2 "Cloud/Aerosol record" 
PRO cld_scia_ol__define
compile_opt idl2,hidden

struct = { cld_scia_ol ,$
           mjd           : {mjd_scia} ,$
           quality       : 0b  ,$
           dummy         : 0b  ,$
           intg_time     : 0us ,$
           numpmdpix     : 0us ,$
           cloudtype     : 0us ,$
           cloudflag     : 0us ,$
           aaiflag       : 0us ,$
           numaeropars   : 0us ,$
           fullfree      : UINTARR(2) ,$
           dsrlen        : 0ul ,$
           surfpress     : 0.0 ,$
           cloudfrac     : 0.0 ,$
           errcldfrac    : 0.0 ,$
           toppress      : 0.0 ,$
           errtoppress   : 0.0 ,$
           cldoptdepth   : 0.0 ,$
           errcldoptdep  : 0.0 ,$
           cloudbrdf     : 0.0 ,$
           errcldbrdf    : 0.0 ,$
           effsurfrefl   : 0.0 ,$
           erreffsrefl   : 0.0 ,$
           aai           : 0.0 ,$
           aaidiag       : 0.0 ,$
           aeropars      : PTR_NEW()  $ ; float * (pointer)
         }
END
