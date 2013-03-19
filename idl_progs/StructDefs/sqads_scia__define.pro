PRO sqads_scia__define
compile_opt idl2,hidden

struct = { sqads_scia ,$
           mjd             : {mjd_scia} ,$
           flag_mds        : 0b         ,$
           flag_glint      : 0b         ,$
           flag_rainbow    : 0b         ,$
           flag_saa_region : 0b         ,$
           flag_missing_readouts : 0us  ,$
           hotpixel        : uintarr(!nadc.scienceChannels + !nadc.numberPMD),$
           mean_wv_diff    : fltarr(!nadc.scienceChannels) ,$
           sdev_wv_diff    : fltarr(!nadc.scienceChannels) ,$
           mean_diff_leak  : fltarr(!nadc.scienceChannels + !nadc.numberPMD) $
         }
END
