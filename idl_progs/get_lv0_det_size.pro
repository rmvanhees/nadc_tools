FUNCTION GET_LV0_DET_SIZE, state_id, category, orbit, chan_mask
 compile_opt idl2,logical_predicate 

 ; obtain Sciamachy cluster definition
 SCIA_CLUSDEF, state_id, orbit

 duration = !scia.duration
 IF category EQ 2 OR category EQ 26 THEN BEGIN
    duration -= 2
    num_scan = FIX(duration / 27)
    duration -= (num_scan-1) * 3
 ENDIF
 
 sz_data = 0UL
 FOR ncl = 0, !scia.num_clus-1 DO BEGIN
    IF (chan_mask AND ISHFT(1B, ncl)) EQ 0B THEN CONTINUE

    intg = !scia.clusDef[ncl].intg
    IF !scia.clusDef[ncl].pet EQ 0.03125 $
       AND !scia.clusDef[ncl].coaddf > 1 THEN intg *= 2

    sz_data += !scia.clusDef[ncl].length * (duration / intg)
 ENDFOR

 RETURN, sz_data
END
