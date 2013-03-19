PRO MDS1C_PMD__DEFINE
compile_opt idl2,hidden

struct = { MDS1C_PMD, $
           mjd            :  {mjd_scia} ,$
           quality_flag   :  0b         ,$
           type_mds       :  0b         ,$
           category       :  0b         ,$ 
           state_id       :  0b         ,$ 
           state_index    :  0b         ,$
           dur_scan       :  0us        ,$ 
           num_pmd        :  0us        ,$ 
           num_geo        :  0us        ,$ 
           dsr_length     :  0ul        ,$
           orbit_phase    :  0.0        ,$
           int_pmd        :  PTR_NEW()  ,$ ; float * (pointer)
           geoL           :  PTR_NEW()  ,$ ; struct geoL_scia * (pointer)
           geoN           :  PTR_NEW()  $  ; struct geoN_scia * (pointer)
         }
END
