; definition of the structure to hold a SCIA level 0 "Specific Product Header"
PRO SPH0_SCIA__DEFINE
compile_opt idl2,hidden

struct = { SPH0_SCIA ,$
           descriptor         : bytarr(29),$
           start_lat          : 0.d       ,$
           start_long         : 0.d       ,$
           stop_lat           : 0.d       ,$
           stop_long          : 0.d       ,$
           sat_track          : 0.d       ,$

           isp_errors         : 0us       ,$
           missing_isps       : 0us       ,$
           isp_discard        : 0us       ,$
           rs_sign            : 0us       ,$

           num_error_isps     : 0l        ,$
           error_isps_thres   : 0.d       ,$
           num_miss_isps      : 0l        ,$
           miss_isps_thres    : 0.d       ,$
           num_discard_isps   : 0l        ,$
           discard_isps_thres : 0.d       ,$
           num_rs_isps        : 0l        ,$
           rs_thres           : 0.d       ,$

           tx_rx_polar        : bytarr(6) ,$
           swath              : bytarr(4) $
         }
END
