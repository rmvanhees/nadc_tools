; definition of the structure to hold a "Data Set Descriptor" record
PRO DSD_SCIA__DEFINE
compile_opt idl2,hidden

struct= { DSD_SCIA, $
          name            : bytarr(29) ,$ ; char
          type            : bytarr(2)  ,$
          flname          : bytarr(63) ,$
          offset          : 0ul        ,$ ; unsigned int
          size            : 0ul        ,$
          num_dsr         : 0ul        ,$
          dsr_size        : 0l         $  ; int
        }
END
