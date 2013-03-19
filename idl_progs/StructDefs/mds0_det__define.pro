PRO mds0_det__define
compile_opt idl2,hidden

struct = { chan_hdr ,$
           sync        : 0us       ,$ ; unsigned short
           bcps        : 0us       ,$ ; unsigned short
           bias        : 0us       ,$ ; unsigned short
           temp        : 0us       ,$ ; unsigned short
           channel     : 0us       ,$ ; unsigned short two_byte
           ratio_hdr   : 0us       ,$ ; unsigned short two_byte
           command_vis : 0ul       ,$ ; unsigned int four_byte
           command_ir  : 0ul       $  ; unsigned int four_byte
         }

struct = { chan_src ,$
           id         :  0b            ,$ ; unsigned char
           coaddf     :  0b            ,$ ; unsigned char
           sync       :  0us           ,$ ; unsigned short
           block_nr   :  0us           ,$ ; unsigned short
           start      :  0us           ,$ ; unsigned short
           length     :  0us           ,$ ; unsigned short
           data       :  PTR_NEW()     $  ; unsigned char * (pointer)
         }

struct = { det_src, $
           hdr        :  {chan_hdr}    ,$ 
           cluster    :  replicate({chan_src}, 16) $ ; number of cluster blocks
         }


struct = { mds0_det ,$
           bcps         : 0us          ,$
           num_chan     : 0us          ,$
           orbit_vector : lonarr(8)    ,$
           isp          : {mjd_scia}   ,$
           fep_hdr      : {fep_hdr}    ,$
           packet_hdr   : {packet_hdr} ,$
           data_hdr     : {data_hdr}   ,$
           pmtc_hdr     : {pmtc_hdr}   ,$
           data_src     : replicate({det_src},!nadc.scienceChannels) $
         }
END
