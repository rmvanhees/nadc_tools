FUNCTION GET_SCIA_ROE_ECLIPSE, absOrbit, orbitList=orbitList
 compile_opt idl2,logical_predicate,hidden

 res = []

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
       RETURN, res
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
    indx1 = WHERE( orbitList EQ orbit-1, count )
    IF count NE 1 THEN GOTO, done
    indx2 = WHERE( orbitList EQ orbit, count )
    IF count NE 1 THEN GOTO, done

    ; read ROE table
    dd = H5D_OPEN( fid, 'roe_entry' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: roe_entry'
    space_id = H5D_GET_SPACE( dd )
    mem_space_id = H5S_CREATE_SIMPLE( [1] )
    H5S_SELECT_HYPERSLAB, space_id, indx1[0], [1], /reset
    roe1 = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
    H5S_SELECT_HYPERSLAB, space_id, indx2[0], [1], /reset
    roe2 = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
    H5D_CLOSE, dd

    res = [roe1.Julian + roe1.ECL_Entry / 86400.d, $
           roe2.Julian + roe2.ECL_Exit / 86400.d]
 ENDIF

done:
 H5F_CLOSE, fid
 RETURN, res
END
