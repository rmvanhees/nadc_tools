PRO PACKET_HDR__DEFINE
compile_opt idl2,hidden

struct = { packet_hdr ,$
           id        : 0us  ,$ ; unsigned short two_byte
           seq_cntrl : 0us  ,$
           length    : 0us  $
         }

END
