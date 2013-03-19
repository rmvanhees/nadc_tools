PRO sqads_scia_ol__define
compile_opt idl2,hidden

OL2_SQADS_PQF_FLAGS = 180

struct = { sqads_scia_ol ,$
           mjd        : {mjd_scia} ,$
           flag_mds   : 0b  ,$
           flag_pqf   : bytarr(OL2_SQADS_PQF_FLAGS)  $ 
         }
END
