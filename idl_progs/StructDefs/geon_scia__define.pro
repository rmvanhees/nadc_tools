PRO GEON_SCIA__DEFINE
compile_opt idl2,hidden

struct = { GEON_SCIA, $
           pixel_type        : 0b        ,$
           glint_flag        : 0b        ,$
           pos_esm           : 0.0       ,$
           sat_h             : 0.0       ,$
           earth_rad         : 0.0       ,$
           sun_zen_ang       : fltarr(3) ,$
           sun_azi_ang       : fltarr(3) ,$
           los_zen_ang       : fltarr(3) ,$
           los_azi_ang       : fltarr(3) ,$
           sub_sat_point     : {coord_scia} ,$
           corner_coord      : replicate({coord_scia},4) ,$
           center_coord      : {coord_scia} $
         }

END
