; definition of the structure to hold the calibration options
PRO CALOPT_SCIA__DEFINE
compile_opt idl2,hidden

struct = { calopt_scia ,$
           geo_filter       : 0b  ,$
           time_filter      : 0b  ,$
           category_filter  : 0b  ,$
           nadir_mds        : 0b  ,$
           limb_mds         : 0b  ,$
           occ_mds          : 0b  ,$
           moni_mds         : 0b  ,$
           pmd_mds          : 0b  ,$
           frac_pol_mds     : 0b  ,$
           slit_function    : 0b  ,$
           sun_mean_ref     : 0b  ,$
           leakage_current  : 0b  ,$
           spectral_cal     : 0b  ,$
           pol_sens         : 0b  ,$
           rad_sens         : 0b  ,$
           ppg_etalon       : 0b  ,$
           mem_effect_cal   : 0b  ,$
           leakage_cal      : 0b  ,$
           straylight_cal   : 0b  ,$
           ppg_cal          : 0b  ,$
           etalon_cal       : 0b  ,$
           wave_cal         : 0b  ,$
           polarisation_cal : 0b  ,$
           radiance_cal     : 0b  ,$
           num_nadir        : 0us ,$
           num_limb         : 0us ,$
           num_occ          : 0us ,$
           num_moni         : 0us ,$
           start_lat        : 0l  ,$
           start_lon        : 0l  ,$
           end_lat          : 0l  ,$
           end_lon          : 0l  ,$
           start_time       : {mjd_scia}   ,$
           stop_time        : {mjd_scia}   ,$
           category         : uintarr( 5 ) ,$
           nadir_cluster    : bytarr( !nadc.maxCluster ) ,$
           limb_cluster     : bytarr( !nadc.maxCluster ) ,$
           occ_cluster      : bytarr( !nadc.maxCluster ) ,$
           moni_cluster     : bytarr( !nadc.maxCluster ) ,$
           l1b_prod_name    : bytarr( 63 ) $
         }
END
