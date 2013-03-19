PRO srs_scia__define
compile_opt idl2,hidden

struct = { srs_scia ,$
           spec_id          : bytarr(3)  ,$
           avg_azim         : 0.0        ,$
           avg_elev         : 0.0        ,$
           avg_ele_ang      : 0.0        ,$
           dopp_shift       : 0.0        ,$
           wvlen_ref_spec   : fltarr(!nadc.sciencePixels) ,$
           mean_ref_spec    : fltarr(!nadc.sciencePixels) ,$
           rad_pre_ref_spec : fltarr(!nadc.sciencePixels) ,$
           rad_acc_ref_spec : fltarr(!nadc.sciencePixels) ,$
           etalon           : fltarr(!nadc.sciencePixels) ,$
           pmd_mean         : fltarr(!nadc.numberPMD)     ,$
           pmd_out_nd_out   : fltarr(!nadc.numberPMD)     ,$
           pmd_out_nd_in    : fltarr(!nadc.numberPMD)     $
         }
END
