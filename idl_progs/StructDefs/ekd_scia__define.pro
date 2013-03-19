PRO ekd_scia__define
compile_opt idl2,hidden

ss = { ekd_scia ,$
       mu2_nadir      : fltarr(!nadc.sciencePixels)  ,$
       mu3_nadir      : fltarr(!nadc.sciencePixels)  ,$
       mu2_limb       : fltarr(!nadc.sciencePixels)  ,$
       mu3_limb       : fltarr(!nadc.sciencePixels)  ,$
       radiance_vis   : fltarr(!nadc.sciencePixels)  ,$
       radiance_nadir : fltarr(!nadc.sciencePixels)  ,$
       radiance_limb  : fltarr(!nadc.sciencePixels)  ,$
       radiance_sun   : fltarr(!nadc.sciencePixels)  ,$
       bsdf           : fltarr(!nadc.sciencePixels)  $
     }
END
