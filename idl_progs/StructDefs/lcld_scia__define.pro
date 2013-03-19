PRO lcld_scia__define
compile_opt idl2,hidden

struct = { lcld_scia ,$
           mjd              : {mjd_scia} ,$
           dsrlen           : 0ul ,$
           quality          : 0b  ,$
           diag_cloud_algo                  : 0b  ,$
           flag_normal_water                : 0b  ,$
           flag_water_clouds                : 0b  ,$
           flag_ice_clouds                  : 0b  ,$
           hght_index_max_value_ice         : 0b  ,$
           flag_polar_strato_clouds         : 0b  ,$
           hght_index_max_value_strato      : 0b  ,$
           flag_noctilucent_clouds          : 0b  ,$
           hght_index_max_value_noctilucent : 0b  ,$
           intg_time                        : 0us ,$
           num_tangent_hghts                : 0us ,$
           num_cir                          : 0us ,$
           num_limb_para                    : 0us ,$
           max_value_cir                    : 0.0 ,$
           hght_max_value_cir               : 0.0 ,$
           max_value_cir_ice                : 0.0 ,$
           hght_max_value_cir_ice           : 0.0 ,$
           max_value_cir_strato             : 0.0 ,$
           hght_max_value_cir_strato        : 0.0 ,$
           hght_max_value_noctilucent       : 0.0 ,$
           tangent_hghts    : PTR_NEW() ,$ ; pointer to float
           cir              : PTR_NEW() ,$ ; pointer to float
           limb_para        : PTR_NEW()  $ ; pointer to float
         }
END
