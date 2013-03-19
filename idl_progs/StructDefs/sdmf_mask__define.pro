PRO sdmf_mask__define
compile_opt idl2,hidden

STR_SZ_DATE = 20

struct = { sdmf_mask ,$
           absOrbit         : 0l   ,$
           entryDate        : bytarr(STR_SZ_DATE) ,$
           saaFlag          : 0b   ,$
           obmTemp          : 0.   ,$
           detectorTemp     : fltarr( !nadc.scienceChannels ) ,$
           smoothLength     : 0us  ,$
           availableOrbits  : 0us  ,$
           selectedOrbits   : 0us  ,$
           maskCount        : uintarr(9) ,$
           maskCountFit     : uintarr(9) ,$
           maskQuality      : uintarr(9)  $
         }
END
