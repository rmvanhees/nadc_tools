; definition of the structure to hold a SCIA level 1 "Specific Product Header"
PRO sph1_scia__define
compile_opt idl2,hidden

struct = { sph1_scia ,$
           spec_cal     :  bytarr(5)  ,$
           saturate     :  bytarr(5)  ,$
           dark_check   :  bytarr(5)  ,$
           dead_pixel   :  bytarr(5)  ,$
           key_data     :  bytarr(6)  ,$
           m_factor     :  bytarr(6)  ,$
           descriptor   :  bytarr(29) ,$
           init_version :  bytarr(38) ,$
           start_time   :  bytarr(!nadc.sciaUTCstring) ,$
           stop_time    :  bytarr(!nadc.sciaUTCstring) ,$
           stripline    :  0s  ,$
           slice_pos    :  0s  ,$
           no_slice     :  0us ,$
           no_nadir     :  0us ,$
           no_limb      :  0us ,$
           no_occult    :  0us ,$
           no_monitor   :  0us ,$
           no_noproc    :  0us ,$
           comp_dark    :  0us ,$
           incomp_dark  :  0us ,$
           start_lat    :  0.d ,$
           start_lon    :  0.d ,$
           stop_lat     :  0.d ,$
           stop_lon     :  0.d  $
         }
END
