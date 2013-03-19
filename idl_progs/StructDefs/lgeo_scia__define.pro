PRO lgeo_scia__define
compile_opt idl2,hidden

struct = { lgeo_scia ,$
           mjd         : {mjd_scia} ,$
           flag_mds    : 0b         ,$
           dummy       : 0b         ,$
           intg_time   : 0us        ,$
           sun_zen_ang : fltarr(3)  ,$
           los_zen_ang : fltarr(3)  ,$
           rel_azi_ang : fltarr(3)  ,$
           sat_h       : 0.0        ,$
           earth_rad   : 0.0        ,$
           tan_h       : fltarr(3)  ,$
           subsat      : {coord_scia} ,$
           tang        : replicate({coord_scia},3) $
         }
END
