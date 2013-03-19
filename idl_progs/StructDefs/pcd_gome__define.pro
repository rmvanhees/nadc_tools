; definition of the structure to hold the Pixel Specific Calibration Records
PRO PCD_GOME__DEFINE
compile_opt idl2,hidden

NUM_POLAR_COEFFS = 8
NUM_SPEC_BANDS = 10

PMD_NUMBER = 3
PMD_IN_GRID = 16

SCIENCE_CHANNELS = 4

ss = { polar_gome ,$
       wv    : fltarr(NUM_POLAR_COEFFS) ,$
       coeff : fltarr(NUM_POLAR_COEFFS) ,$
       error : fltarr(NUM_POLAR_COEFFS) ,$
       chi   : 0.0 $
     }

ss = { mph0_gome ,$
       ProductConfidenceData    : bytarr(3)  ,$
       UTC_MPH_Generation       : bytarr(25) ,$
       ProcessorSoftwareVersion : bytarr(9)  $
     }

ss = { sph0_gome ,$
       sph_5  : bytarr(2)  ,$
       sph_6  : bytarr(21) $
     }

ss = { ihr_gome ,$
       subsetcounter : 0us ,$
       prism_temp    : 0us ,$
       averagemode   : 0us ,$
       intgstat      : 0us ,$
       pmd           : uintarr(PMD_IN_GRID,PMD_NUMBER) ,$
       peltier       : intarr(SCIENCE_CHANNELS) $
     }

ss = { pcd_gome ,$
       selected     : 0s           ,$
       indx_spec    : 0s           ,$
       indx_leak    : 0s           ,$
       indx_bands   : intarr(NUM_SPEC_BANDS) ,$
       dark_current : 0.0          ,$
       noise_factor : 0.0          ,$
       polar        : {polar_gome} ,$
       glr          : {glr1_gome}  ,$
       mph0         : {mph0_gome}  ,$
       sph0         : {sph0_gome}  ,$
       ihr          : {ihr_gome}   ,$
       pmd          : replicate({pmd_gome}, PMD_IN_GRID) $
     }

END
