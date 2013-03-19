FUNCTION SCIA_DECODE_MEMCORR, corrval, channel, ATBD=ATBD
 compile_opt idl2,logical_predicate,hidden

 IF channel LT 1 OR channel GT 5 THEN $
    MESSAGE, 'Channel should be between 1 and 5'

 IF KEYWORD_SET( ATBD ) THEN BEGIN
    yvalue = 1.25 * (FLOAT(corrval) - 91)
 ENDIF ELSE BEGIN
    CASE channel OF
       1: BEGIN
          yvalue = 1.25 * (FLOAT(corrval) - 91)
       END
       2: BEGIN
          yvalue = 1.25 * (FLOAT(corrval) - 91)
       END
       3: BEGIN
          yvalue = 1.25 * (FLOAT(corrval) - 91)
       END
       4: BEGIN
          yvalue = 1.25 * (FLOAT(corrval) - 91)
       END
       5: BEGIN
          yvalue = BYTARR( 65536, /NOZERO )
          indx = WHERE( corrval GT 50 )
          yvalue = 1.5 * (FLOAT(corrval) - 125)
          indx = WHERE( corrval LE 50 )
          yvalue[indx] = FLOAT(corrval[indx]) / 2 + 2
       END
       ELSE: BEGIN
          yvalue = FLOAT(corrval)
       END
    ENDCASE
 ENDELSE
; IF channel EQ 1 THEN BEGIN
;    xc = [0,13824,32151]
; ENDIF ELSE IF channel EQ 2 THEN BEGIN
;    xc = [0,17139,32151]
; ENDIF ELSE IF channel EQ 3 THEN BEGIN
;    xc = [0,16154,32151]
; ENDIF ELSE IF channel EQ 4 THEN BEGIN
;    xc = [0,15859,32151]
; ENDIF ELSE IF channel EQ 5 THEN BEGIN
;    xc = [0,15945,32151]
; ENDIF ELSE $
;    MESSAGE, 'Channel should be between 1 and 5'
 
; ichan = FIX(channel - 1)
; yvalue = FLTARR(65536, /NOZERO)
; yvalue[xc[0]:xc[1]] = FLOAT(corrval[xc[0]:xc[1]]) / 5 - 5
; yvalue[xc[1]:xc[2]] = FLOAT(corrval[xc[1]:xc[2]]) / 2 - 115
; yvalue[xc[2]:-1] = FLOAT(corrval[xc[2]:-1])

 RETURN, yvalue
END
