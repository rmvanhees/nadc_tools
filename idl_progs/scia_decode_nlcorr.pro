FUNCTION SCIA_DECODE_NLCORR, corrval, curveIndex, ATBD=ATBD
 compile_opt idl2,logical_predicate,hidden

 IF curveIndex LT 1 OR curveIndex GE 15 THEN $
    MESSAGE, 'CurveIndex should be between 1 and 15'

 IF KEYWORD_SET( ATBD ) THEN BEGIN
    IF curveIndex LE 6 THEN BEGIN
       yvalue = 1.25 * (FLOAT(corrval) - 26)
    ENDIF ELSE IF curveIndex LE 10 THEN BEGIN
       yvalue = 1.5 * (FLOAT(corrval) - 254)
    ENDIF ELSE IF curveIndex LE 14 THEN BEGIN
       yvalue = 1.25 * (FLOAT(corrval) - 254)
    ENDIF

    RETURN, yvalue
 ENDIF

 yvalue = FLTARR(65536, /NOZERO)
 CASE curveIndex OF
    1: BEGIN
       indx = WHERE( corrval LE 127 )
       yvalue[indx] = FLOAT(corrval[indx]) / 12.5 - 3
       indx = WHERE( corrval GT 127 )
       yvalue[indx] = FLOAT(corrval[indx] - 127) / 0.45
    END
    2:BEGIN
       indx = WHERE( corrval LE 127 )
       yvalue[indx] = FLOAT(corrval[indx]) / 12.5 - 4.5
       indx = WHERE( corrval GT 127 )
       yvalue[indx] = FLOAT(corrval[indx] - 127) / 0.45
    END
    3:BEGIN
       indx = WHERE( corrval LE 127 )
       yvalue[indx] = FLOAT(corrval[indx]) / 12.5 - 1
       indx = WHERE( corrval GT 127 )
       yvalue[indx] = FLOAT(corrval[indx] - 127) / 0.45
    END
    4:BEGIN
       indx = WHERE( corrval LE 127 )
       yvalue[indx] = FLOAT(corrval[indx]) / 12.5 - 3
       indx = WHERE( corrval GT 127 )
       yvalue[indx] = FLOAT(corrval[indx] - 127) / 0.45
    END
    5:BEGIN
       indx = WHERE( corrval LE 127 )
       yvalue[indx] = FLOAT(corrval[indx]) / 12.5 - 3
       indx = WHERE( corrval GT 127 )
       yvalue[indx] = FLOAT(corrval[indx] - 127) / 0.45
    END
    6:BEGIN
       indx = WHERE( corrval LE 127 )
       yvalue[indx] = FLOAT(corrval[indx]) / 12.5 - 3
       indx = WHERE( corrval GT 127 )
       yvalue[indx] = FLOAT(corrval[indx] - 127) / 0.45
    END
    7:BEGIN
       indx = WHERE( corrval LE 127 )
       yvalue[indx] = FLOAT(corrval[indx]) / 12.5 - 9
       indx = WHERE( corrval GT 127 )
       yvalue[indx] = FLOAT(1.25 * (corrval[indx] - 255))
    END
    8:BEGIN
       indx = WHERE( corrval LE 127 )
       yvalue[indx] = FLOAT(corrval[indx]) / 12.5 - 9
       indx = WHERE( corrval GT 127 )
       yvalue[indx] = FLOAT(1.25 * (corrval[indx] - 255))
    END
    9:BEGIN
       indx = WHERE( corrval LE 127 )
       yvalue[indx] = FLOAT(corrval[indx]) / 12.5 - 9
       indx = WHERE( corrval GT 127 )
       yvalue[indx] = FLOAT(1.25 * (corrval[indx] - 255))
    END
    10:BEGIN
       indx = WHERE( corrval LE 127 )
       yvalue[indx] = FLOAT(corrval[indx]) / 12.5 - 9
       indx = WHERE( corrval GT 127 )
       yvalue[indx] = FLOAT(1.25 * (corrval[indx] - 255))
    END
    ELSE: BEGIN
       yvalue = 1.25 * (FLOAT(corrval) - 254)
    END
 ENDCASE
; IF curveIndex LE 6 THEN BEGIN
;    xc = [0,31500,50000]

;    yvalue[xc[0]:xc[1]] = FLOAT(corrval[xc[0]:xc[1]]) / 6.3 - 5
;    yvalue[xc[1]:xc[2]] = FLOAT(corrval[xc[1]:xc[2]]) + 30
;    yvalue[xc[2]:-1]    = 2 * FLOAT(corrval[xc[2]:-1])
;     indx = WHERE( corrval LE 127 )
;     yvalue[indx] = FLOAT(corrval[indx]) / 12.5 - 1
;     indx = WHERE( corrval GT 127 )
;     yvalue[indx] = FLOAT(corrval[indx] - 127) / 0.45
; ENDIF ELSE IF curveIndex LE 10 THEN BEGIN
;    xc = [0,15000]
;
;    yvalue[xc[0]:xc[1]] = FLOAT(corrval[xc[0]:xc[1]]) / 10 - 20
;    yvalue[xc[1]:-1]    = FLOAT(corrval[xc[1]:-1]) - 255
; ENDIF ELSE IF curveIndex LE 14 THEN BEGIN
;    xc = [0,12000]
;
;    yvalue[xc[0]:xc[1]] = FLOAT(corrval[xc[0]:xc[1]]) / 10 - 20
;    yvalue[xc[1]:-1]    = FLOAT(corrval[xc[1]:-1]) - 255
; ENDIF
 

 RETURN, yvalue
END
