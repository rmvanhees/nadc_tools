PRO srsn_scia__define
compile_opt idl2,hidden

struct = { srsn_scia ,$
           mjd              : {mjd_scia} ,$
           flag_mds         : 0b         ,$
           flag_neu_dens    : 0b         ,$
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
           pmd_out          : fltarr(!nadc.numberPMD)     $
     }
END
