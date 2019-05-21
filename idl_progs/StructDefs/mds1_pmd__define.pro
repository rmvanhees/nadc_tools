PRO mds1_pmd__define
compile_opt idl2,hidden

struct = { mds1_pmd ,$
           mjd        : {mjd_scia}        ,$
           flag_mds   : 1b                ,$
           padding1   : 1us               ,$
           packet_hdr : {packet_hdr}      ,$
           padding2   : 1us               ,$
           data_hdr   : {data_hdr}        ,$
           data_src   : {pmd_src}         $
         }
END
