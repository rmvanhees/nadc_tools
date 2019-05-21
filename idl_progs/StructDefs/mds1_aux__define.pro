PRO mds1_aux__define
compile_opt idl2,hidden

struct = { mds1_aux, $
           mjd        : {mjd_scia}        ,$
           flag_mds   : 1b                ,$
           padding1   : 1us               ,$
           packet_hdr : {packet_hdr}      ,$
           padding2   : 1us               ,$
           data_hdr   : {data_hdr}        ,$
           pmtc_hdr   : {pmtc_hdr}        ,$
           data_src   : replicate({pmtc_frame},!nadc.numLv0AuxPMTC) $
         }
END
