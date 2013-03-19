PRO state1_scia__define
compile_opt idl2,hidden

struct = { Clcon_scia ,$
           id        : 0b   ,$
           channel   : 0b   ,$
           type      : 0b   ,$
           pixel_nr  : 0us  ,$
           length    : 0us  ,$
           int_time  : 0us  ,$
           coaddf    : 0us  ,$
           n_read    : 0us  ,$
           pet       : 0.0  $
         }

struct = { state1_scia ,$
           mjd               : {mjd_scia}  ,$
           Clcon             : replicate({Clcon_scia},!nadc.maxCluster) ,$
           flag_mds          : 0b          ,$
           flag_reason       : 0b          ,$
           type_mds          : 0b          ,$
           category          : 0us         ,$
           state_id          : 0us         ,$
           duration          : 0us         ,$
           longest_intg_time : 0us         ,$
           num_clusters      : 0us         ,$
           num_aux           : 0us         ,$
           num_pmd           : 0us         ,$
           num_intg          : 0us         ,$
           intg_times        : uintarr(!nadc.maxCluster) ,$
           num_polar         : uintarr(!nadc.maxCluster) ,$
           total_polar       : 0us         ,$
           num_dsr           : 0us         ,$
           indx              : 0ul         ,$
           length_dsr        : 0ul         ,$
           offset            : 0ul         ,$
           off_pmd           : 0ul         ,$
           off_polV          : 0ul         ,$
           orbit_phase       : 0.0         $
         }
END
