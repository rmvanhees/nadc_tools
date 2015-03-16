; definition of the structure to hold info records
;
PRO mds0_states__define
compile_opt idl2,hidden

struct = { mds0_states, $
           mjd           : {mjd_scia} ,$
           q_aux         : 0b         ,$
           q_det         : 0b         ,$
           q_pmd         : 0b         ,$
           category      : 0b         ,$
           state_id      : 0b         ,$
           num_aux       : 0ul        ,$
           num_det       : 0ul        ,$
           num_pmd       : 0ul        ,$
           on_board_time : 0ul        ,$
           offset        : 0ul        ,$
           info_aux      : PTR_NEW()  ,$ ;struct mds0_info * (pointer)
           info_det      : PTR_NEW()  ,$ ;struct mds0_info * (pointer)
           info_pmd      : PTR_NEW()  $  ;struct mds0_info * (pointer)
         }
END
