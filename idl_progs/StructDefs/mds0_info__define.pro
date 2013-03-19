PRO mds0_info__define
compile_opt idl2,hidden

MAX_CLUSTER      = 64         ; number of cluster configurations

struct = { info_clus, $
           chan_id    : 0b         ,$
           cluster_id : 0b         ,$
           co_adding  : 0b         ,$
           start      : 0us        ,$
           length     : 0us        $
         }
           
; definition of the structure to hold info records
struct = { mds0_info, $
           packet_id  : 0b         ,$
           category   : 0b         ,$
           state_id   : 0b         ,$
           num_clus   : 0b         ,$
           length     : 0us        ,$
           bcps       : 0us        ,$
           stateIndex : 0us        ,$
           offset     : 0ul        ,$
           mjd        : {mjd_scia} ,$
           cluster    : replicate({info_clus},MAX_CLUSTER) $
         }
END
