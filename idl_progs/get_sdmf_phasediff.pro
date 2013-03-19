FUNCTION GET_SDMF_PHASEDIFF, absOrbit, orbitList=orbitList
 compile_opt idl2,logical_predicate,hidden

 ; default phaseDiff difference between SDMF v2.4 and ATBD definition
 phaseDiff = 0.0895

 ; open ROE database
 roe_fl = 'ROE_EXC_all.h5'
 IF FILE_TEST( './' + roe_fl, /READ, /NOEXPAND ) THEN $
    ROE_DB = './' + roe_fl $
 ELSE BEGIN
    DEFSYSV, '!nadc', exists=i
    IF i NE 1 THEN DEFS_NADC, "/SCIA/share/nadc_tools"
    ROE_DB = !nadc.dataDir + '/' + roe_fl

    IF (~ FILE_TEST( ROE_DB, /READ, /NOEXPAND )) THEN BEGIN
       MESSAGE, ' Can not find database: ' + ROE_DB, /INFO
       RETURN, phaseDiff
    ENDIF
 ENDELSE
 fid = H5F_OPEN( ROE_DB )
 IF fid LT 0 THEN MESSAGE, 'Could not open database: ' + ROE_DB

 orbit = LONG( absOrbit[0] )
 IF orbit GT 0 THEN BEGIN
    dd = H5D_OPEN( fid, 'orbitList' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: orbitList'
    orbitList = H5D_READ( dd )
    H5D_CLOSE, dd
    indx = WHERE( orbitList EQ orbit, count )
    IF count NE 1 THEN GOTO, done

    ; read ROE table
    dd = H5D_OPEN( fid, 'roe_entry' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: roe_entry'
    space_id = H5D_GET_SPACE( dd )
    mem_space_id = H5S_CREATE_SIMPLE( [1] )
    H5S_SELECT_HYPERSLAB, space_id, indx[0], [1], /reset
    roe = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
    H5D_CLOSE, dd

    ; compute orbitPhase correction
    orbitList = roe.Orbit
    phaseDiff = ((roe.ECL_Entry - roe.ECL_Exit) / roe.Period - 0.5)
    phaseDiff /= 2.
 ENDIF ELSE BEGIN
    ; read ROE table
    dd = H5D_OPEN( fid, 'roe_entry' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: roe_entry'
    roeList = H5D_READ( dd )
    H5D_CLOSE, dd

    ; compute orbitPhase correction
    orbitList = roeList.Orbit
    phaseDiff = ((roeList.ECL_Entry - roeList.ECL_Exit) / roeList.Period - 0.5)
    phaseDiff /= 2.
 ENDELSE

done:
 H5F_CLOSE, fid
 RETURN, phaseDiff
END
