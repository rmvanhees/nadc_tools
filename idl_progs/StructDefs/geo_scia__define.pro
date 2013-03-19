PRO geo_scia__define
compile_opt idl2,hidden

struct = { geo_scia ,$
           mjd         : {mjd_scia} ,$
           flag_mds    : 0b         ,$
           intg_time   : 0us        ,$
           sun_zen_ang : fltarr(3)  ,$
           los_zen_ang : fltarr(3)  ,$
           rel_azi_ang : fltarr(3)  ,$
           sat_h       : 0.0        ,$
           earth_rad   : 0.0        ,$
           corner      : replicate({coord_scia}, 4) ,$
           center      : {coord_scia} $
         }
END
