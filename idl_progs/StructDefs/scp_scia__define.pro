PRO scp_scia__define
compile_opt idl2,hidden

struct = { scp_scia ,$
           orbit_phase    : 0.0  ,$
           coeffs         : dblarr(5 * !nadc.scienceChannels) ,$
           num_lines      : uintarr(!nadc.scienceChannels)    ,$
           wv_error_calib : fltarr(!nadc.scienceChannels)      $
     }
END
