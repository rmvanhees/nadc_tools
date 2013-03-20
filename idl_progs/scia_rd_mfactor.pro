;
; mftype        :  a string holding one of M_CAL, M_DL, M_DN
; sensing_start :  sensing start taken from MPH
;
; mfactor       :  array holding mfactor
;
PRO SCIA_RD_MFACTOR, mftype, sensing_start, mfactor
 compile_opt idl2,logical_predicate,hidden

 IF mftype NE 'M_CAL' AND mftype NE 'M_DL'  AND mftype NE 'M_DN' THEN $
    MESSAGE, 'Unknown mftype requested'

 mfactor = FLTARR(8192)

 result = call_external( lib_name('libnadc_idl'), '_SCIA_RD_MFACTOR', $
                         mftype, sensing_start, mfactor, /CDECL )
 print, result

 RETURN
END
