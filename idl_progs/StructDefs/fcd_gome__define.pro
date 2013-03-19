; definition of the structure to hold the Fixed Calibration Data Record
PRO FCD_GOME__DEFINE
compile_opt idl2,hidden

NUM_STRAY_GHOSTS = 2
NUM_FPA_SCALE = 5
NUM_SPEC_COEFFS = 5
NUM_SPEC_BANDS = 10
NUM_FPA_COEFFS = 100

SCIENCE_CHANNELS = 4
CHANNEL_SIZE = 1024
SCIENCE_PIXELS = (SCIENCE_CHANNELS * CHANNEL_SIZE)

PMD_NUMBER = 3

struct = { lv1_ghost ,$
           symmetry   : intarr(NUM_STRAY_GHOSTS,SCIENCE_CHANNELS) ,$
           center     : intarr(NUM_STRAY_GHOSTS,SCIENCE_CHANNELS) ,$
           defocus    : fltarr(NUM_STRAY_GHOSTS,SCIENCE_CHANNELS) ,$
           energy     : fltarr(NUM_STRAY_GHOSTS,SCIENCE_CHANNELS) $
         }

struct = { lv1_kde ,$
           bsdf_1     : fltarr(SCIENCE_CHANNELS) ,$
           bsdf_2     : fltarr(SCIENCE_CHANNELS) ,$
           resp_1     : fltarr(SCIENCE_CHANNELS) ,$
           resp_2     : fltarr(SCIENCE_CHANNELS) ,$
           f2_1       : fltarr(SCIENCE_CHANNELS) ,$
           f2_2       : fltarr(SCIENCE_CHANNELS) ,$
           smdep_1    : fltarr(SCIENCE_CHANNELS) ,$
           smdep_2    : fltarr(SCIENCE_CHANNELS) ,$
           chi_1      : fltarr(SCIENCE_CHANNELS) ,$
           chi_2      : fltarr(SCIENCE_CHANNELS) ,$
           eta_1      : fltarr(SCIENCE_CHANNELS) ,$
           eta_2      : fltarr(SCIENCE_CHANNELS) ,$
           ksi_1      : fltarr(SCIENCE_CHANNELS) ,$
           ksi_2      : fltarr(SCIENCE_CHANNELS) ,$
           rfs        : fltarr(SCIENCE_PIXELS)   $
         }

struct = { lv1_bcr ,$
           array_id    :   0s  ,$
           start_pixel :   0s  ,$
           end_pixel   :   0s  $
         }

struct = { lv1_leak ,$
           noise      :   0.0                    ,$
           pmd_offs   :   fltarr(PMD_NUMBER)     ,$
           pmd_noise  :   0.0                    ,$
           dark       :   fltarr(SCIENCE_PIXELS) $
         }

struct = { lv1_hot ,$
           record     :   0s ,$
           array      :   0s ,$
           pixel      :   0s $
         }

struct = { lv1_spec ,$
           coeffs     :   dblarr(NUM_SPEC_COEFFS,SCIENCE_CHANNELS) ,$
           error      :   dblarr(SCIENCE_CHANNELS) $
         }

struct = { lv1_calib ,$
           eta_omega  :   fltarr(CHANNEL_SIZE) ,$
           response   :   fltarr(CHANNEL_SIZE) $
         }

struct = { fcd_gome ,$
           flags      :   0l  ,$
           npeltier   :   0s  ,$
           nleak      :   0s  ,$
           nhot       :   0s  ,$
           nspec      :   0s  ,$
           nang       :   0s  ,$
           width_conv :   0s  ,$
           indx_spec  :   0s  ,$
           sun_date   :   0ul ,$
           sun_time   :   0ul ,$
           bsdf_0     :   0.0 ,$
           elevation  :   0.0 ,$
           azimuth    :   0.0 ,$
           sun_pmd    : fltarr(PMD_NUMBER) ,$
           sun_pmd_wv : fltarr(PMD_NUMBER) ,$
           stray_level    : fltarr(4)       ,$
           scale_peltier  : fltarr(NUM_FPA_SCALE) ,$
           coeffs     : fltarr(8)          ,$
           filter_peltier : fltarr(NUM_FPA_COEFFS),$
           pixel_gain : fltarr(SCIENCE_PIXELS) ,$
           intensity  : fltarr(SCIENCE_PIXELS) ,$
           sun_ref    : fltarr(SCIENCE_PIXELS) ,$
           sun_precision  : fltarr(SCIENCE_PIXELS),$
           ghost      : {lv1_ghost} ,$
           kde        : {lv1_kde}   ,$
           bcr        : replicate({lv1_bcr}, NUM_SPEC_BANDS) ,$
           leak       : PTR_NEW() ,$ ; struct lv1_leak * (pointer)
           hot        : PTR_NEW() ,$ ; struct lv1_hot * (pointer)
           spec       : PTR_NEW() ,$ ; struct lv1_spec * (pointer)
           calib      : PTR_NEW() $  ; struct lv1_calib * (pointer)
         }

END
