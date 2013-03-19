PRO sip_scia__define
compile_opt idl2,hidden

MaxBoundariesSIP = 13

ss = { sip_scia ,$
       do_use_limb_dark :  bytarr(2)  ,$
       do_pixelwise     :  bytarr(!nadc.scienceChannels+1)  ,$
       do_ib_oc_etn     :  bytarr(!nadc.numberPMD+1)  ,$
       do_ib_sd_etn     :  bytarr(!nadc.numberPMD+1)  ,$
       do_fraunhofer    :  bytarr(5 * !nadc.scienceChannels+1)  ,$
       do_etalon        :  bytarr(3 * !nadc.scienceChannels+1)  ,$
       do_var_lc_cha    :  bytarr(4 * !nadc.irChannels+1)  ,$
       do_stray_lc_cha  :  bytarr(4 * !nadc.scienceChannels+1)  ,$
       do_var_lc_pmd    :  bytarr(4 * !nadc.numberIrPMD+1)  ,$
       do_stray_lc_pmd  :  bytarr(4 * !nadc.numberPMD+1)  ,$
       do_pol_point     :  bytarr(!nadc.scienceChannels + !nadc.numberPMD+1) ,$
       n_lc_min         :  0b  ,$
       ds_n_phases      :  0b  ,$
       sp_n_phases      :  0b  ,$
       ds_poly_order    :  0b  ,$
       lc_harm_order    :  0b  ,$
       level_2_smr      :  bytarr(!nadc.scienceChannels) ,$
       sat_level        :  uintarr(!nadc.scienceChannels) ,$
       pmd_sat_limit    :  0us ,$
       startpix_6       :  0s  ,$
       startpix_8       :  0s  ,$
       alpha0_asm       :  0.0 ,$
       alpha0_esm       :  0.0 ,$
       ppg_order        :  0.0 ,$
       stray_order      :  0.0 ,$
       h_toa            :  0.0 ,$
       lambda_end_gdf   :  0.0 ,$
       ds_phase_boundaries :  fltarr(MaxBoundariesSIP) ,$
       sp_phase_boundaries :  fltarr(MaxBoundariesSIP) ,$
       lc_stray_indx    :  fltarr(2) ,$
       electrons_bu     :  fltarr(!nadc.scienceChannels) $
     }
END
