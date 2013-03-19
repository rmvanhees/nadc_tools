PRO pmd_data__define
compile_opt idl2,hidden

struct = { pmd_data ,$
           sync    : 0us                        ,$
           data    : uintarr(2,!nadc.numberPMD) ,$
           mdi     : 0us                        ,$
           time    : 0us                        $
         }
END
