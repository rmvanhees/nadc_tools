; definition of the structure to hold the Specific Product Header
PRO SPH1_GOME__DEFINE
compile_opt idl2,hidden

PMD_NUMBER = 3

ss = { sph1_gome ,$
       nr_inref      :  0s   ,$
       inref         :  bytarr(39,2) ,$
       soft_version  :  bytarr(6)    ,$
       calib_version :  bytarr(6)    ,$
       prod_version  :  0s   ,$
       time_orbit    :  0ul  ,$
       time_utc_day  :  0ul  ,$
       time_utc_ms   :  0ul  ,$
       time_counter  :  0ul  ,$
       time_period   :  0ul  ,$
       pmd_entry     :  0s   ,$
       subset_entry  :  0s   ,$
       status_entry  :  0s   ,$
       peltier_entry :  0s   ,$
       status2_entry :  0s   ,$
       pmd_conv      :  fltarr(PMD_NUMBER,2) ,$
       state_utc_day :  0ul  ,$
       state_utc_ms  :  0ul  ,$
       state_orbit   :  0ul  ,$
       state_x       :  0.0  ,$
       state_y       :  0.0  ,$
       state_z       :  0.0  ,$
       state_dx      :  0.0  ,$
       state_dy      :  0.0  ,$
       state_dz      :  0.0  ,$
       att_yaw       :  0.d  ,$
       att_pitch     :  0.d  ,$
       att_roll      :  0.d  ,$
       att_dyaw      :  0.d  ,$
       att_dpitch    :  0.d  ,$
       att_droll     :  0.d  ,$
       att_flag      :  0l   ,$
       att_stat      :  0l   ,$
       julian        :  0.d  ,$
       semi_major    :  0.d  ,$
       excen         :  0.d  ,$
       incl          :  0.d  ,$
       right_asc     :  0.d  ,$
       perigee       :  0.d  ,$
       mn_anom       :  0.d  $
     }

END
