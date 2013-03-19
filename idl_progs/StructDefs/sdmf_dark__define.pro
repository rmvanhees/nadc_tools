PRO sdmf_dark__define
compile_opt idl2,hidden

STR_SZ_DATE = 20

struct = { sdmf_dark ,$
           julianDay        : 0.d  ,$
           absOrbit         : 0l   ,$
           entryDate        : bytarr(STR_SZ_DATE) ,$
           saaFlag          : 0b   ,$
           quality          : 0s   ,$
           orbitPhase       : 0.   ,$
           obmTemp          : 0.   ,$
           detectorTemp     : fltarr( !nadc.scienceChannels ) $
         }
END
