PRO mds1c_polv__define
compile_opt idl2,hidden

struct = { mds1c_polv, $
           mjd            :  {mjd_scia} ,$
           quality_flag   :  0b         ,$
           type_mds       :  0b         ,$
           category       :  0b         ,$ 
           state_id       :  0b         ,$ 
           state_index    :  0b         ,$
           dur_scan       :  0us        ,$ 
           total_polV     :  0us        ,$ 
           num_diff_intg  :  0us        ,$ 
           num_geo        :  0us        ,$ 
           dsr_length     :  0ul        ,$
           orbit_phase    :  0.0        ,$
           intg_times     :  UINTARR( !nadc.maxCluster ) ,$
           num_polar      :  UINTARR( !nadc.maxCluster ) ,$
           polV           :  PTR_NEW()  ,$ ; struct polV_scia * (pointer)
           geoL           :  PTR_NEW()  .$ ; struct geoL_scia * (pointer)
           geoN           :  PTR_NEW()  $  ; struct geoN_scia * (pointer)
         }

END
