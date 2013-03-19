PRO sdmf_meta__define
compile_opt idl2,hidden

STR_SZ_DATE = 20

struct = { sdmf_meta ,$
           julianDay        : 0.d ,$
           duration         : 0l  ,$ 
           absOrbit         : 0l  ,$
           entryDate        : bytarr(STR_SZ_DATE) ,$
           softVersion      : bytarr(15) ,$
           procStage        : bytarr(2)  ,$
           saaFlag          : 0b  ,$
           rtsEnhFlag       : 0b  ,$
           vorporFlag       : 0b  ,$
           orbitPhase       : 0.  ,$
           sunSemiDiam      : 0.  ,$
           moonAreaSunlit   : 0.  ,$
           longitude        : 0.  ,$   
           latitude         : 0.  ,$
           asmAngle         : 0.  ,$
           esmAngle         : 0.  ,$
           obmTemp          : 0.  ,$
           detectorTemp     : fltarr( !nadc.scienceChannels ) $
         }
END
