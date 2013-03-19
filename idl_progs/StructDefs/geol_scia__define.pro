PRO GEOL_SCIA__DEFINE
compile_opt idl2,hidden

struct = { GEOL_SCIA, $
           pixel_type        : 0b        ,$
           glint_flag        : 0b        ,$
           pos_esm           : 0.0       ,$
           pos_asm           : 0.0       ,$
           sat_h             : 0.0       ,$
           earth_rad         : 0.0       ,$
           dopp_shift        : 0.0       ,$
           sun_zen_ang       : fltarr(3) ,$
           sun_azi_ang       : fltarr(3) ,$
           los_zen_ang       : fltarr(3) ,$
           los_azi_ang       : fltarr(3) ,$
           tan_h             : fltarr(3) ,$
           sub_sat_point     : {coord_scia} ,$
           tang_ground_point : replicate({coord_scia},3) $
         }
END
