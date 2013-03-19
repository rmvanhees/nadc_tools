FUNCTION SCIA_ENCODE_NLCORR, nlcorr, curveIndex, ATBD=ATBD
 compile_opt idl2,logical_predicate,hidden

 IF curveIndex LT 1 OR curveIndex GE 15 THEN $
    MESSAGE, 'CurveIndex should be between 1 and 15'

 IF KEYWORD_SET( ATBD ) THEN BEGIN
    IF curveIndex LE 6 THEN BEGIN
       yvalue = BYTE(ROUND((nlcorr[*,curveIndex]/1.25) + 26))
    ENDIF ELSE IF curveIndex LE 10 THEN BEGIN
       yvalue = BYTE(ROUND((nlcorr[*,curveIndex]/1.5) + 254))
    ENDIF ELSE IF curveIndex LE 14 THEN BEGIN
       yvalue = BYTE(ROUND((nlcorr[*,curveIndex]/1.25) + 254))
    ENDIF
    
    RETURN, yvalue
 ENDIF
 
 yvalue = BYTARR(65536, /NOZERO)
 CASE curveIndex OF
    1: BEGIN
       yvalue[*] = BYTE(ROUND( 12.5 * (nlcorr[*,curveIndex] + 3)) > 0 < 127)
       indx = WHERE( ROUND( 12.5 * (nlcorr[*,curveIndex] + 3)) GT 127 )
       yvalue[indx] = BYTE(ROUND( 0.45 * nlcorr[indx,curveIndex] + 127) < 255)
    END
    2:BEGIN
       yvalue[*] = BYTE(ROUND( 12.5 * (nlcorr[*,curveIndex] + 4.5)) > 0 < 127)
       indx = WHERE( ROUND( 12.5 * (nlcorr[*,curveIndex] + 4.5)) GT 127 )
       yvalue[indx] = BYTE(ROUND( 0.45 * nlcorr[indx,curveIndex] + 127) < 255)
    END
    3:BEGIN
       yvalue[*] = BYTE(ROUND( 12.5 * (nlcorr[*,curveIndex] + 1)) > 0 < 127)
       indx = WHERE( ROUND( 12.5 * (nlcorr[*,curveIndex] + 1)) GT 127 )
       yvalue[indx] = BYTE(ROUND( 0.45 * nlcorr[indx,curveIndex] + 127) < 255)
    END
    4:BEGIN
       yvalue[*] = BYTE(ROUND( 12.5 * (nlcorr[*,curveIndex] + 3)) > 0 < 127)
       indx = WHERE( ROUND( 12.5 * (nlcorr[*,curveIndex] + 3)) GT 127 )
       yvalue[indx] = BYTE(ROUND( 0.45 * nlcorr[indx,curveIndex] + 127) < 255)
    END
    5:BEGIN
       yvalue[*] = BYTE(ROUND( 12.5 * (nlcorr[*,curveIndex] + 3)) > 0 < 127)
       indx = WHERE( ROUND( 12.5 * (nlcorr[*,curveIndex] + 3)) GT 127 )
       yvalue[indx] = BYTE(ROUND( 0.45 * nlcorr[indx,curveIndex] + 127) < 255)
    END
    6:BEGIN
       yvalue[*] = BYTE(ROUND( 12.5 * (nlcorr[*,curveIndex] + 3)) > 0 < 127)
       indx = WHERE( ROUND( 12.5 * (nlcorr[*,curveIndex] + 3)) GT 127 )
       yvalue[indx] = BYTE(ROUND( 0.45 * nlcorr[indx,curveIndex] + 127) < 255)
    END
    7:BEGIN
       yvalue[*] = BYTE(ROUND( 12.5 * (nlcorr[*,curveIndex] + 9)) > 0 < 127)
       indx = WHERE( ROUND( 12.5 * (nlcorr[*,curveIndex] + 9) ) LE 0 )
       yvalue[indx] = BYTE(ROUND( nlcorr[indx,curveIndex] / 1.25 + 255) < 255)
    END
    8:BEGIN
       yvalue[*] = BYTE(ROUND( 12.5 * (nlcorr[*,curveIndex] + 9)) > 0 < 127)
       indx = WHERE( ROUND( 12.5 * (nlcorr[*,curveIndex] + 9) ) LE 0 )
       yvalue[indx] = BYTE(ROUND( nlcorr[indx,curveIndex] / 1.25 + 255) < 255)
    END
    9:BEGIN
       yvalue[*] = BYTE(ROUND( 12.5 * (nlcorr[*,curveIndex] + 9)) > 0 < 127)
       indx = WHERE( ROUND( 12.5 * (nlcorr[*,curveIndex] + 9) ) LE 0 )
       yvalue[indx] = BYTE(ROUND( nlcorr[indx,curveIndex] / 1.25 + 255) < 255)
    END
    10:BEGIN
       yvalue[*] = BYTE(ROUND( 12.5 * (nlcorr[*,curveIndex] + 9)) > 0 < 127)
       indx = WHERE( ROUND( 12.5 * (nlcorr[*,curveIndex] + 9) ) LE 0 )
       yvalue[indx] = BYTE(ROUND( nlcorr[indx,curveIndex] / 1.25 + 255) < 255)
    END
    ELSE: BEGIN
       yvalue = BYTE(ROUND((nlcorr[*,curveIndex]/1.25) + 254))
    END
 ENDCASE
; IF curveIndex LE 6 THEN BEGIN
;    xc = [0,31500,50000]

;    yvalue[xc[0]:xc[1]] = BYTE(ROUND(6.3 * (nlcorr[xc[0]:xc[1],curveIndex]+5)))
;    yvalue[xc[1]:xc[2]] = BYTE(ROUND(nlcorr[xc[1]:xc[2],curveIndex]-30))
;    yvalue[xc[2]:-1]    = BYTE(ROUND(nlcorr[xc[2]:-1,curveIndex] / 2))
; ENDIF ELSE IF curveIndex LE 10 THEN BEGIN
;    xc = [0,15000]
;
;    yvalue[xc[0]:xc[1]] = BYTE(ROUND(10 * (nlcorr[xc[0]:xc[1],curveIndex]+20)))
;    yvalue[xc[1]:-1]    = BYTE(ROUND(nlcorr[xc[1]:-1,curveIndex]+255))
; ENDIF ELSE IF curveIndex LE 14 THEN BEGIN
;    xc = [0,12000]
;
;    yvalue[xc[0]:xc[1]] = BYTE(ROUND(10 * (nlcorr[xc[0]:xc[1],curveIndex]+20)))
;    yvalue[xc[1]:-1]    = BYTE(ROUND(nlcorr[xc[1]:-1,curveIndex]+255))
; ENDIF

 RETURN, yvalue
END
