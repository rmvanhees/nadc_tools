PRO MDS1_SCIA__DEFINE
compile_opt idl2,hidden

struct = { mds1_scia, $
           mjd          :  {mjd_scia},$
           quality_flag :  0b        ,$
           type_mds     :  0b        ,$
           state_id     :  0b        ,$
           state_index  :  0b        ,$
           n_clus       :  0us       ,$
           n_aux        :  0us       ,$ 
           n_pmd        :  0us       ,$ 
           n_pol        :  0us       ,$ 
           dsr_length   :  0ul       ,$
           scale_factor :  bytarr(!nadc.scienceChannels) ,$
           sat_flags    :  PTR_NEW() ,$ ; unsigned char * (pointer)
           red_grass    :  PTR_NEW() ,$ ; unsigned char * (pointer)
           lv0          :  PTR_NEW() ,$ ; struct lv0_hdr * (pointer)
           geoC         :  PTR_NEW() ,$ ; struct geoC_scia * (pointer)
           geoL         :  PTR_NEW() ,$ ; struct geoL_scia * (pointer)
           geoN         :  PTR_NEW() ,$ ; struct geoN_scia * (pointer)
           int_pmd      :  PTR_NEW() ,$ ; float * (pointer)
           polV         :  PTR_NEW() ,$ ; struct polV_scia * (pointer)
           clus         :  replicate({Clus_scia},!nadc.maxCluster)  $
         }
END
