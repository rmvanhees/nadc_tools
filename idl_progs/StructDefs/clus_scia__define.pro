PRO CLUS_SCIA__DEFINE
compile_opt idl2,hidden

struct = { CLUS_SCIA, $
           n_sig  : 0us       ,$
           n_sigc : 0us       ,$
           sig    : PTR_NEW() ,$ ; struct Sig_scia * (pointer)
           sigc   : PTR_NEW() $ ; struct Sigc_scia * (pointer)
         }
END
