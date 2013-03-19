PRO pmd_scia__define
compile_opt idl2,hidden

struct = { pmd_scia ,$
           mjd        : {mjd_scia}        ,$
           flag_mds   : 1b                ,$
           isp        : {mjd_scia}        ,$
           fep_hdr    : {fep_hdr}         ,$
           packet_hdr : {packet_hdr}      ,$
           data_hdr   : {data_hdr}        ,$
           data_src   : {pmd_src }        $
         }
END
