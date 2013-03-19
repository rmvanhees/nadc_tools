PRO mds0_pmd__define
compile_opt idl2,hidden

struct = { mds0_pmd ,$
           isp        : {mjd_scia}   ,$
           fep_hdr    : {fep_hdr}    ,$
           packet_hdr : {packet_hdr} ,$
           data_hdr   : {data_hdr}   ,$
           data_src   : {pmd_src }   $
         }
END
