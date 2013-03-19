; definition of the structure to hold a NRT level 2 "Specific Product Header"
PRO sph2_scia__define
compile_opt idl2,hidden

MAX_BIAS_MICRO_WIN   = 3
MAX_BIAS_FITTING_WIN = 4
MAX_DOAS_FITTING_WIN = 7
MAX_DOAS_SPECIES     = 22
MAX_BIAS_SPECIES     = 10

struct = { bias_record ,$
           wv_min    : 0us,$
           wv_max    : 0us,$
           nr_micro  : 0us,$
           micro_min : uintarr(MAX_BIAS_MICRO_WIN) ,$
           micro_max : uintarr(MAX_BIAS_MICRO_WIN)  $
         }

struct = { doas_record ,$
           wv_min    : 0us,$
           wv_max    : 0us $
         }

struct = { sph2_scia ,$
           fit_error   :  bytarr(5)  ,$
           descriptor  :  bytarr(29) ,$
           start_time  :  bytarr(!nadc.sciaUTCstring) ,$
           stop_time   :  bytarr(!nadc.sciaUTCstring) ,$
           bias_mol    :  bytarr(9,MAX_BIAS_SPECIES) ,$
           doas_mol    :  bytarr(9,MAX_DOAS_SPECIES) ,$
           stripline   :  0s  ,$
           slice_pos   :  0s  ,$
           no_slice    :  0us ,$
           no_bias_win :  0us ,$
           no_bias_mol :  0us ,$
           no_doas_win :  0us ,$
           no_doas_mol :  0us ,$
           start_lat   :  0.d ,$
           start_lon   :  0.d ,$
           stop_lat    :  0.d ,$
           stop_lon    :  0.d ,$
           bias_win    :  replicate({bias_record}, MAX_BIAS_FITTING_WIN) ,$
           doas_win    :  replicate({doas_record}, MAX_DOAS_FITTING_WIN) $
         }
END
