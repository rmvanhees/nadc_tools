PRO state2_scia__define
compile_opt idl2,hidden

struct = { state2_scia ,$
           mjd                : {mjd_scia} ,$
           flag_mds           : 0b   ,$
           state_id           : 0us  ,$
           duration           : 0us  ,$
           longest_intg_time  : 0us  ,$
           shortest_intg_time : 0us  ,$
           num_obs_state      : 0us  $
         }
END
