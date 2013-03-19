PRO lfit_scia__define
compile_opt idl2,hidden

struct = { layer_rec ,$
           tangvmr    : 0.0 ,$
           errtangvmr : 0.0 ,$
           vertcol    : 0.0 ,$
           errvertcol : 0.0 $
         }

struct = { meas_grid ,$
           mjd        : {mjd_scia} ,$
           tangh      : 0.0 ,$
           tangp      : 0.0 ,$
           tangt      : 0.0 ,$
           num_win    : 0b  ,$
           win_limits : fltarr(2) $
         }

struct = { state_vec ,$
           value      : 0.0 ,$
           error      : 0.0 ,$
           type       : bytarr(4) $
         }

struct = { lfit_scia ,$
           mjd              : {mjd_scia} ,$
           quality          : 0b  ,$
           criteria         : 0b  ,$
           method           : 0b  ,$
           refpsrc          : 0b  ,$
           num_rlevel       : 0b  ,$
           num_mlevel       : 0b  ,$
           num_species      : 0b  ,$
           num_closure      : 0b  ,$
           num_other        : 0b  ,$
           num_scale        : 0b  ,$
           intg_time        : 0us ,$
           stvec_size       : 0us ,$
           cmatrixsize      : 0us ,$
           numiter          : 0us ,$
           ressize          : 0us ,$
           num_adddiag      : 0us ,$
           summary          : uintarr(2) ,$
           dsrlen           : 0ul ,$
           refh             : 0.0 ,$
           refp             : 0.0 ,$
           rms              : 0.0 ,$
           chi2             : 0.0 ,$
           goodness         : 0.0 ,$
           tangh            : PTR_NEW() ,$ ; pointer to float
           tangp            : PTR_NEW() ,$ ; pointer to float
           tangt            : PTR_NEW() ,$ ; pointer to float
           corrmatrix       : PTR_NEW() ,$ ; pointer to float
           residuals        : PTR_NEW() ,$ ; pointer to float
           addiag           : PTR_NEW() ,$ ; pointer to float
           mainrec          : PTR_NEW() ,$ ; pointer to struct layer_rec
           scaledrec        : PTR_NEW() ,$ ; pointer to struct layer_rec
           mgrid            : PTR_NEW() ,$ ; pointer to struct meas_grid
           statevec         : PTR_NEW() $  ; pointer to struct state_vec
         }
END
