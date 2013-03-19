
pro sdmf_smr_meta__define
compile_opt idl2,hidden

struct = {sdmf_smr_meta   $
, julianDay  : double(0)  $
, absOrbit   : long(0)    $
, entryDate  : bytarr(20) $ 
, saaFlag    : byte(0)    $
, orbitPhase : float(0)   $ ;remove? What measurement should it refer to?
}
end

