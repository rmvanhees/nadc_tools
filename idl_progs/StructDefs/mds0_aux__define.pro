PRO mds0_aux__define
compile_opt idl2,hidden

struct = { mds0_aux, $
           isp             : {mjd_scia}   ,$
           fep_hdr         : {fep_hdr}    ,$
           packet_hdr      : {packet_hdr} ,$
           data_hdr        : {data_hdr}   ,$
           pmtc_hdr        : {pmtc_hdr}   ,$
           data_src        : replicate({pmtc_frame},!nadc.numLv0AuxPMTC) $
         }
END

