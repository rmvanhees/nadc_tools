PRO clcp_scia__define
compile_opt idl2,hidden

ss = { clcp_scia ,$
       fpn            : fltarr(!nadc.sciencePixels) ,$
       fpn_error      : fltarr(!nadc.sciencePixels) ,$
       lc             : fltarr(!nadc.sciencePixels) ,$
       lc_error       : fltarr(!nadc.sciencePixels) ,$
       pmd_dark       : fltarr(2 * !nadc.numberPMD) ,$
       pmd_dark_error : fltarr(2 * !nadc.numberPMD) ,$
       mean_noise     : fltarr(!nadc.sciencePixels) $
     }
END
