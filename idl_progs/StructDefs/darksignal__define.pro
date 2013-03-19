PRO DarkSignal__define
compile_opt idl2,hidden

struct = { DARKSIGNAL                         ,$
           Orbit             : long(0)        ,$ 
           MagicNumber       : long(0)        ,$ 
           Saa               : long(0)        ,$
           Time              : double(0)      ,$
           Tobm              : float(0)       ,$ 
           Tdet              : fltarr(8)      ,$
           QualityNumber     : long(0)        ,$ 
           AnalogOffset      : Fltarr( !Nadc.SciencePixels ) ,$
           DarkCurrent       : Fltarr( !Nadc.SciencePixels ) ,$
           AnalogOffsetError : Fltarr( !Nadc.SciencePixels ) ,$
           DarkCurrentError  : Fltarr( !Nadc.SciencePixels ) ,$
           ChiSquare         : Fltarr( !Nadc.SciencePixels ) $
         }
END

