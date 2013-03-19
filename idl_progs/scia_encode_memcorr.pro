FUNCTION SCIA_ENCODE_MEMCORR, memcorr, channel, ATBD=ATBD
 compile_opt idl2,logical_predicate,hidden

 IF channel LT 1 OR channel GT 5 THEN $
    MESSAGE, 'Channel should be between 1 and 5'

 IF KEYWORD_SET( ATBD ) THEN BEGIN
    yvalue = BYTE(ROUND((memcorr[*,(channel-1)]/1.25) + 91) > 0 < 255 )
 ENDIF ELSE BEGIN
    CASE channel OF
       1: BEGIN
          yvalue = BYTE(ROUND((memcorr[*,0] / 1.25) + 91) > 0 < 255 )
       END
       2: BEGIN
          yvalue = BYTE(ROUND((memcorr[*,1] / 1.25) + 91) > 0 < 255 )
       END
       3: BEGIN
          yvalue = BYTE(ROUND((memcorr[*,2] / 1.25) + 91) > 0 < 255 )
       END
       4: BEGIN
          yvalue = BYTE(ROUND((memcorr[*,3] / 1.25) + 91) > 0 < 255 )
       END
       5: BEGIN
          ival = ROUND(2 * (memcorr[*,4]-2))
          
          yvalue = BYTE(ival > 0 < 50 )
          indx = WHERE( ival LT 0 OR ival GT 50 )
          yvalue[indx] = BYTE(ROUND( memcorr[indx,4] / 1.5 + 125 ) < 255)
       END
       ELSE: BEGIN
          yvalue = BYTARR( 65536 )
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
; yvalue = BYTARR(65536, /NOZERO)
; yvalue[xc[0]:xc[1]] = BYTE(ROUND(5 * (memcorr[xc[0]:xc[1],ichan] + 5)))
; yvalue[xc[1]:xc[2]] = BYTE(ROUND(2 * (memcorr[xc[1]:xc[2],ichan] + 115)))
; yvalue[xc[2]:-1]    = BYTE(ROUND(memcorr[xc[2]:-1,ichan]))

 RETURN, yvalue
END
