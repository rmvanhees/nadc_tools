PRO vlcp_scia__define
compile_opt idl2,hidden

struct = { vlcp_scia ,$
           orbit_phase       : 0.0 ,$
           obm_pmd           : fltarr(!nadc.irChannels + !nadc.numberPMD)   ,$
           var_lc            : fltarr(!nadc.irChannels * !nadc.channelSize) ,$
           var_lc_error      : fltarr(!nadc.irChannels * !nadc.channelSize) ,$
           solar_stray       : fltarr(!nadc.sciencePixels) ,$
           solar_stray_error : fltarr(!nadc.sciencePixels) ,$
           pmd_stray         : fltarr(!nadc.numberPMD)     ,$
           pmd_stray_error   : fltarr(!nadc.numberPMD)     ,$
           pmd_dark          : fltarr(!nadc.numberIrPMD)   ,$
           pmd_dark_error    : fltarr(!nadc.numberIrPMD)   $
         }
END
