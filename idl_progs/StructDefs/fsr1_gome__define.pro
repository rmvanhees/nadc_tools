; definition of the structure to hold the File Structure Record
PRO FSR1_GOME__DEFINE
compile_opt idl2,hidden

NUM_SPEC_BANDS = 10

struct = { fsr1_gome ,$
           nr_sph      :   0s ,$
           sz_sph      :   0l ,$
           nr_fcd      :   0s ,$
           sz_fcd      :   0l ,$
           nr_pcd      :   0s ,$
           sz_pcd      :   0l ,$
           nr_scd      :   0s ,$
           sz_scd      :   0l ,$
           nr_mcd      :   0s ,$
           sz_mcd      :   0l ,$
           emply1      : bytarr(6) ,$
           nr_band     : intarr(NUM_SPEC_BANDS) ,$
           sz_band     : lonarr(NUM_SPEC_BANDS) $
         }

END
