PRO PMTC_HDR__DEFINE
compile_opt idl2,hidden

struct = { PMTC_HDR, $
           pmtc_1        : 0us       ,$ ; unsigned short two_byte
           scanner_mode  : 0us       ,$ ; unsigned short
           az_param      : 0ul       ,$ ; unsigned int four_byte
           elv_param     : 0ul       ,$ ; unsigned int four_byte
           factor        : bytarr(6) $  ; unsigned char factor[6]
         }

END
