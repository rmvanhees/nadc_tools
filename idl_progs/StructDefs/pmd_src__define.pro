PRO pmd_src__define
compile_opt idl2,hidden

struct = { pmd_src ,$
           temp   : 0us ,$
           packet : replicate({pmd_data}, !nadc.numLv0PMDpacket)  $
         }
END
