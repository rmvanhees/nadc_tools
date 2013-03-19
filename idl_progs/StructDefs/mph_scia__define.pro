PRO MPH_SCIA__DEFINE
compile_opt idl2,hidden

struct = { MPH_SCIA, $
           product         : bytarr(63) ,$
           proc_stage      : bytarr(2)  ,$
           ref_doc         : bytarr(24) ,$
           acquis          : bytarr(21) ,$
           proc_center     : bytarr(7)  ,$
           proc_time       : bytarr(!nadc.sciaUTCstring),$
           soft_version    : bytarr(15) ,$
           sensing_start   : bytarr(!nadc.sciaUTCstring),$
           sensing_stop    : bytarr(!nadc.sciaUTCstring),$
           phase           : bytarr(2)  ,$
           cycle           : 0s    ,$
           rel_orbit       : 0l    ,$
           abs_orbit       : 0l    ,$
           state_vector    : bytarr(!nadc.sciaUTCstring),$
           delta_ut        : 0.d   ,$
           x_position      : 0.d   ,$
           y_position      : 0.d   ,$
           z_position      : 0.d   ,$
           x_velocity      : 0.d   ,$
           y_velocity      : 0.d   ,$
           z_velocity      : 0.d   ,$
           vector_source   : bytarr(3)  ,$
           utc_sbt_time    : bytarr(!nadc.sciaUTCstring),$
           sat_binary_time : 0ul   ,$
           clock_step      : 0ul   ,$
           leap_utc        : bytarr(!nadc.sciaUTCstring),$
           leap_sign       : 0s    ,$
           leap_err        : bytarr(2)  ,$
           product_err     : bytarr(2)  ,$
           tot_size        : 0ul   ,$
           sph_size        : 0ul   ,$
           num_dsd         : 0ul   ,$
           dsd_size        : 0ul   ,$
           num_data_sets   : 0ul   $
         }
END
