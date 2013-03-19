PRO sqads2_scia__define
compile_opt idl2,hidden

NL2_SQADS_PQF_FLAGS = 152

struct = { sqads2_scia ,$
           mjd        : {mjd_scia} ,$
           flag_mds   : 0b  ,$
           flag_pqf   : bytarr(NL2_SQADS_PQF_FLAGS)  $ 
         }
END
