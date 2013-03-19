PRO dark_scia__define
compile_opt idl2,hidden

struct = { dark_scia ,$
           mjd               : {mjd_scia} ,$
           flag_mds          : 0b                      ,$
           dark_spec         : fltarr(!nadc.sciencePixels)  ,$
           sdev_dark_spec    : fltarr(!nadc.sciencePixels)  ,$
           pmd_off           : fltarr(2 * !nadc.numberPMD)  ,$
           pmd_off_error     : fltarr(2 * !nadc.numberPMD)  ,$
           sol_stray         : fltarr(!nadc.sciencePixels)  ,$
           sol_stray_error   : fltarr(!nadc.sciencePixels)  ,$
           pmd_stray         : fltarr( !nadc.numberPMD)     ,$
           pmd_stray_error   : fltarr( !nadc.numberPMD)     $
         }
END
