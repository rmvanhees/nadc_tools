PRO scpn_scia__define
compile_opt idl2,hidden

struct = { scpn_scia ,$
           mjd               : {mjd_scia}                   ,$
           flag_mds          : 0b                           ,$
           orbit_phase       : 0.0                          ,$
           srs_param         : bytarr(!nadc.scienceChannels)     ,$
           num_lines         : uintarr(!nadc.scienceChannels)    ,$
           wv_error_calib    : fltarr(!nadc.scienceChannels)     ,$
           sol_spec          : fltarr(!nadc.sciencePixels)       ,$
           line_pos          : fltarr(3 * !nadc.scienceChannels) ,$
           coeffs            : dblarr(5 * !nadc.scienceChannels) $
         }
END
