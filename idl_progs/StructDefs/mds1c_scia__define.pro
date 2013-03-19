PRO MDS1C_SCIA__DEFINE
compile_opt idl2,hidden

struct = { mds1c_scia, $
           mjd            :  {mjd_scia},$
           rad_units_flag :  0b        ,$
           quality_flag   :  0b        ,$
           type_mds       :  0b        ,$
           coaddf         :  0b        ,$
           category       :  0b        ,$ 
           state_id       :  0b        ,$ 
           state_index    :  0b        ,$
           chan_id        :  0b        ,$ 
           clus_id        :  0b        ,$ 
           dur_scan       :  0us       ,$ 
           num_obs        :  0us       ,$ 
           num_pixels     :  0us       ,$ 
           dsr_length     :  0ul       ,$
           orbit_phase    :  0.0       ,$
           pet            :  0.0       ,$
           pixel_ids      :  PTR_NEW() ,$ ; unsigned short * (pointer)
           pixel_wv       :  PTR_NEW() ,$ ; float * (pointer)
           pixel_wv_err   :  PTR_NEW() ,$ ; float * (pointer)
           pixel_val      :  PTR_NEW() ,$ ; float * (pointer)
           pixel_err      :  PTR_NEW() ,$ ; float * (pointer)
           geoC           :  PTR_NEW() ,$ ; struct geoC_scia * (pointer)
           geoL           :  PTR_NEW() ,$ ; struct geoL_scia * (pointer)
           geoN           :  PTR_NEW() $  ; struct geoN_scia * (pointer)
         }

END
