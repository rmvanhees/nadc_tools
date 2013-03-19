PRO sdmf_statedark__define
compile_opt idl2,hidden

struct = {sdmf_statedark                      $
          ,absOrbit          : long(0)        $
          ,StateId           : fix(0)         $
          ,StateCount        : fix(0)         $
          ,Tobm              : float(0)       $
          ,Tdet              : fltarr(!nadc.scienceChannels) $
          ,entryDate         : bytarr(20)     $
          ,saaFlag           : byte(0)        $
          ,Quality           : long(0)        $
}
END
