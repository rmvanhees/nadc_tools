; definition of the structure to hold a Cloud/Aerosol data set
PRO cld_scia__define
compile_opt idl2,hidden

struct = { cld_scia ,$
           mjd           : {mjd_scia} ,$
           quality       : 0b  ,$
           quality_cld   : 0b  ,$
           outputflag    : 0us ,$
           intg_time     : 0us ,$
           numpmd        : 0us ,$
           dsrlen        : 0ul ,$
           cloudfrac     : 0.0 ,$
           toppress      : 0.0 ,$
           aai           : 0.0 ,$
           albedo        : 0.0 ,$
           pmdcloudfrac  : PTR_NEW()  $ ; float * (pointer)
         }
END
