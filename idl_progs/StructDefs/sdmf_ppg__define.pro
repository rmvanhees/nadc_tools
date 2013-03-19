PRO sdmf_ppg__define
compile_opt idl2,hidden

STR_SZ_DATE = 20

struct = { sdmf_ppg ,$
           absOrbit         : 0l   ,$
           entryDate        : bytarr(STR_SZ_DATE) ,$
           saaFlag          : 0b   ,$
           obmTemp          : 0.   ,$
           detectorTemp     : fltarr( !nadc.scienceChannels ) $
         }
END
