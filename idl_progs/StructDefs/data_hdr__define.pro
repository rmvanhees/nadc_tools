PRO DATA_HDR__DEFINE
compile_opt idl2,hidden

struct = { DATA_HDR ,$
           category        : 0b      ,$ ; unsigned char
           state_id        : 0b      ,$ ; unsigned char
           length          : 0us     ,$ ; unsigned short
           rdv             : 0us     ,$ ; unsigned short two_bytes
           id              : 0us     ,$ ; unsigned short two_bytes
           on_board_time   : 0ul     $  ; unsigned int
         }

END
