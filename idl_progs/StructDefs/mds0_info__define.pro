; definition of the structure to hold info records
;
PRO mds0_info__define
compile_opt idl2,hidden

struct = { mds0_info, $
           mjd           : {mjd_scia} ,$
           packet_type   : 0b         ,$
           category      : 0b         ,$
           state_id      : 0b         ,$
           quality       : 0b         ,$
           crc_errors    : 0us        ,$
           rs_errors     : 0us        ,$
           bcps          : 0us        ,$
           packet_length : 0us        ,$
           on_board_time : 0ul        ,$
           offset        : 0ul        $
         }
END
