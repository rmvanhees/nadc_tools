PRO sdmf_pt_meta__define
compile_opt idl2,hidden

STR_SZ_DATE = 20

struct = { sdmf_pt_meta ,$
           julianDate       : 0.d ,$
           duration         : 0l  ,$ 
           absOrbit         : 0l  ,$
           entryDate        : bytarr(STR_SZ_DATE) ,$
           procStage        : 0b  ,$
           saaFlag          : 0b  ,$
           porFlag          : 0b  ,$
           orbitPhase       : 0.  ,$
           asmAngle         : 0.  ,$
           esmAngle         : 0.  ,$
           sunAz            : 0.  ,$
           sunEl            : 0.  ,$
           longitude        : 0.  ,$   
           latitude         : 0.  ,$
           obmTemp          : 0.  ,$
           detectorTemp     : fltarr( !nadc.scienceChannels ) $
         }
END
