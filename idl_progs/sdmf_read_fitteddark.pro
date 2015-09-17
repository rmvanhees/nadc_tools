;
; COPYRIGHT (c) 2010 - 2013 SRON (R.M.van.Hees@sron.nl)
;
;   This is free software; you can redistribute it and/or modify it
;   under the terms of the GNU General Public License, version 2, as
;   published by the Free Software Foundation.
;
;   The software is distributed in the hope that it will be useful, but
;   WITHOUT ANY WARRANTY; without even the implied warranty of
;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;   General Public License for more details.
;
;   You should have received a copy of the GNU General Public License
;   along with this program; if not, write to the Free Software
;   Foundation, Inc., 59 Temple Place - Suite 330, 
;   Boston, MA  02111-1307, USA.
;
;+
; NAME:
;	SDMF_READ_FITTEDDARK
;
; PURPOSE:
;	obtain Dark parameters obtained by a linear fit through the State Darks
;
; CATEGORY:
;	Sciamachy - SDMF
;
; CALLING SEQUENCE:
;	SDMF_READ_FITTEDDARK, orbit, mtbl, analogOffset, darkCurrent
;
; INPUTS:
;	orbit:	        absolute orbit number, a scalar or vector[min,max], 
;                       to select all orbits use -1
;
; KEYWORD PARAMETERS:
;       use_neighbours:    flag to enable looking for neighboring valid entries
;
;	analogOffsError:   uncertainty in analogOffset
;       darkCurrentError:  uncertainty in darkCurrent
;       meanNoise:         average standard deviation (SDMF 3.1)
;       chiSquareFit:      chi-square of fit
;       probabilityFit:    probability of fit (SDMF 3.1)
;       stateCount:        states used for fit: 8=1, 26=2, 46=4, 63=8, 67=16
;
;       channel:           channel ID, named scalar, range [1..8]
;       pixelID:           pixel ID, named scalar, range [1...8192]
;
;       SDMF_H5_DB:        path to the SDMF Dark database
;                            default: /SCIA/SDMF/3.0/sdmf_dark.h5
;       SDMF_VERSION:      SDMF version <3.0|3.1>, default 3.0
;       status :           returns named variable with error flag (0 = ok)
;
; OUTPUTS:
;       mtbl:           metaTable (array of structures)
;       analogOffset:   analog offset (BU) or fixed pattern noise
;       darkCurrent:    dark current (BU/s) or leakage current
;
; PROCEDURE:
;	none
;
; EXAMPLE:
;	- obtain for orbit=7502, all channels: analogOffset and darkCurrent
;       IDL> SDMF_READ_FITTEDDARK, 7502, mtbl, analogOffset, darkCurrent
;
;	- obtain for orbit=7502, channel=2: analogOffset and darkCurrent
;       IDL> SDMF_READ_FITTEDDARK, 7502, mtbl, analogOffset, darkCurrent, channel=2
;
;	- obtain for pixel=100 (all orbits): analogOffset and darkCurrent
;       IDL> SDMF_READ_FITTEDDARK, -1, mtbl, analogOffset, darkCurrent, pixel=100
;
;	- obtain for orbit range: analogOffset and darkCurrent
;       IDL> SDMF_READ_FITTEDDARK, [11000,12000], mtbl, analogOffset, darkCurrent
;
; MODIFICATION HISTORY:
; 	Written by:	R.M. van Hees (SRON), 26-01-2010
;       Modified:  RvH, added keyword use_neighbours, October 2010
;       Modified:  RvH, added keyword stateCount, January 2011
;       Modified:  RvH, apply REFORM on output arrays, January 2011
;       Modified:  RvH, coding according new standard, 28 March 2012
;-
;---------------------------------------------------------------------------
FUNCTION __READ_DARKSET, dd, max_records, numIndx, metaIndx, nchan, ipix
 compile_opt idl2,logical_predicate,hidden

 space_id = H5D_GET_SPACE( dd )
 IF nchan GT 0 THEN BEGIN
    IF numIndx LT max_records THEN BEGIN
       databuffer = FLTARR( numIndx, 1024 )
       mem_space_id = H5S_CREATE_SIMPLE( [1,1024] )
       FOR ii = 0, numIndx-1 DO BEGIN
          H5S_SELECT_HYPERSLAB, space_id, [metaIndx[ii],(nchan-1)*1024], $
                                [1,1024], /reset
          tmp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
          databuffer[ii,*] = tmp
       ENDFOR
       H5S_CLOSE, mem_space_id
    ENDIF ELSE BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [numIndx,1024] )
       H5S_SELECT_HYPERSLAB, space_id, [0,(nchan-1)*1024], $
                             [numIndx,1024], /reset
       databuffer = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
       H5S_CLOSE, mem_space_id
    ENDELSE
 ENDIF ELSE IF ipix GT 0 THEN BEGIN
    IF numIndx LT max_records THEN BEGIN
       databuffer = FLTARR( numIndx )
       mem_space_id = H5S_CREATE_SIMPLE( [1,1] )
       FOR ii = 0, numIndx-1 DO BEGIN
          H5S_SELECT_HYPERSLAB, space_id, [metaIndx[ii],ipix-1], [1,1], /reset
          tmp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
          databuffer[ii] = tmp
       ENDFOR
       H5S_CLOSE, mem_space_id
    ENDIF ELSE BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [numIndx,1] )
       H5S_SELECT_HYPERSLAB, space_id, [0,ipix-1], [numIndx,1], /reset
       databuffer = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
       H5S_CLOSE, mem_space_id
    ENDELSE
 ENDIF ELSE BEGIN
    IF numIndx LT max_records THEN BEGIN
       databuffer = FLTARR( numIndx, 8192 )
       mem_space_id = H5S_CREATE_SIMPLE( [1,8192] )
       FOR ii = 0, numIndx-1 DO BEGIN
          H5S_SELECT_HYPERSLAB, space_id, [metaIndx[ii],0], [1,8192], /reset
          tmp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
          databuffer[ii,*] = tmp
       ENDFOR
       H5S_CLOSE, mem_space_id
    ENDIF ELSE BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [numIndx,8192] )
       H5S_SELECT_HYPERSLAB, space_id, [0,0], [numIndx,8192], /reset
       databuffer = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
       H5S_CLOSE, mem_space_id
    ENDELSE
 ENDELSE
 H5S_CLOSE, space_id
 
 RETURN, databuffer
END

; ---------------------------------------------------------------------------
PRO __GET_30_FITTED_DARK, fid, orbitRange, mtbl, analogOffset, darkCurrent, $
   analogOffsError=analogOffsError, darkCurrentError=darkCurrentError, $
   chiSquareFit=chiSquareFit, channel=channel, pixelID=pixelID, $
   use_neighbours=use_neighbours
 compile_opt idl2,logical_predicate,hidden
;
 nchan = -1
 IF N_ELEMENTS( channel ) GT 0 THEN nchan = FIX( channel[0] )
 ipix = -1
 IF N_ELEMENTS( pixelID ) GT 0 THEN ipix = FIX( pixelID[0] )
 IF ~ keyword_set(use_neighbours) THEN use_neighbours = 0
;
 dd = H5D_OPEN( fid, 'orbitList' )
 orbitList = H5D_READ( dd )
 H5D_CLOSE, dd
 IF N_ELEMENTS( orbitRange ) EQ 1 THEN BEGIN
    IF orbitRange EQ -1 THEN BEGIN
       numIndx = N_ELEMENTS( orbitList )
       metaIndx = LINDGEN( numIndx )
    ENDIF ELSE BEGIN
       metaIndx = WHERE( orbitList EQ orbitRange, numIndx )
       IF numIndx EQ 0 AND use_neighbours NE 0 THEN BEGIN
          diff = MIN( ABS( orbitList - orbitRange ), indx )
          metaIndx = WHERE( orbitList EQ orbitList[indx], numIndx )
       ENDIF
    ENDELSE
 ENDIF ELSE BEGIN
    metaIndx = WHERE( orbitList GE orbitRange[0] $
                      AND orbitList LE orbitRange[1], numIndx )
 ENDELSE
 IF numIndx EQ 0 THEN BEGIN
    MESSAGE, 'no entry found in database: ' + SDMF_H5_DB, /INFO
    RETURN
 ENDIF

 dd = H5D_OPEN( fid, 'metaTable' )
 IF dd LT 0 THEN MESSAGE, 'Could not open dataset: metaTable'
 space_id = H5D_GET_SPACE( dd )
 dims = H5S_GET_SIMPLE_EXTENT_DIMS( space_id )
 IF numIndx LT dims[0] THEN BEGIN
    mem_space_id = H5S_CREATE_SIMPLE( [1] )
    H5S_SELECT_HYPERSLAB, space_id, [metaIndx[0]], [1], /reset
    mtbl_1 = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
    mtbl = REPLICATE( mtbl_1, numIndx )
    FOR ii = 1, numIndx-1 DO BEGIN
       H5S_SELECT_HYPERSLAB, space_id, [metaIndx[ii]], [1], /reset
       mtbl_1 = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
       mtbl[ii] = mtbl_1
    ENDFOR
    H5S_CLOSE, mem_space_id
 ENDIF ELSE IF numIndx EQ dims[0] THEN BEGIN
    mtbl = H5D_READ( dd )
    ii = WHERE( mtbl.absOrbit LE 0, count )
    IF count GT 0 THEN $
       indx_sort = (SORT( mtbl.absOrbit ))[1:-1] $
    ELSE $
       indx_sort = SORT( mtbl.absOrbit )
    mtbl = mtbl[indx_sort]
 ENDIF
 H5S_CLOSE, space_id
 H5D_CLOSE, dd
;
 dd = H5D_OPEN( fid, 'analogOffset' )
 IF dd LT 0 THEN MESSAGE, 'Could not open dataset: analogOffset'
 analogOffset = __READ_DARKSET( dd, dims[0], numIndx, metaIndx, nchan, ipix )
 IF numIndx EQ dims[0] THEN analogOffset = analogOffset[indx_sort,*]
 analogOffset = REFORM( analogOffset )
 H5D_CLOSE, dd

 dd = H5D_OPEN( fid, 'darkCurrent' )
 IF dd LT 0 THEN MESSAGE, 'Could not open dataset: darkCurrent'
 darkCurrent = __READ_DARKSET( dd, dims[0], numIndx, metaIndx, nchan, ipix )
 IF numIndx EQ dims[0] THEN darkCurrent = darkCurrent[indx_sort,*]
 darkCurrent = REFORM( darkCurrent )
 H5D_CLOSE, dd
;
 IF ARG_PRESENT( analogOffsError ) THEN BEGIN
    dd = H5D_OPEN( fid, 'analogOffsetError' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: analogOffsetError'
    analogOffsError = $
       __READ_DARKSET( dd, dims[0], numIndx, metaIndx, nchan, ipix )
    IF numIndx EQ dims[0] THEN analogOffsError = analogOffsError[indx_sort,*]
    analogOffsError = REFORM( analogOffsError )
    H5D_CLOSE, dd
 ENDIF
;
 IF ARG_PRESENT( darkCurrentError ) THEN BEGIN
    dd = H5D_OPEN( fid, 'darkCurrentError' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: darkCurrentError'
    darkCurrentError = $
       __READ_DARKSET( dd, dims[0], numIndx, metaIndx, nchan, ipix )
    IF numIndx EQ dims[0] THEN darkCurrentError = darkCurrentError[indx_sort,*]
    darkCurrentError = REFORM( darkCurrentError )
    H5D_CLOSE, dd
 ENDIF
;
 IF ARG_PRESENT( chiSquareFit ) THEN BEGIN
    dd = H5D_OPEN( fid, 'chiSquareFit' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: chiSquareFit'
    chiSquareFit = __READ_DARKSET( dd, dims[0], numIndx, metaIndx, nchan, ipix )
    IF numIndx EQ dims[0] THEN chiSquareFit = chiSquareFit[indx_sort,*]
    chiSquareFit = REFORM( chiSquareFit )
    H5D_CLOSE, dd
 ENDIF
 RETURN
END

;---------------------------------------------------------------------------
PRO __GET_31_FITTED_DARK, gid, orbitRange, mtbl, analogOffset, darkCurrent, $
   analogOffsError=analogOffsError, darkCurrentError=darkCurrentError, $
   meanNoise=meanNoise, chiSquareFit=chiSquareFit, stateCount=stateCount, $
   probabilityFit=probabilityFit, channel=channel, pixelID=pixelID, $
   use_neighbours=use_neighbours
 compile_opt idl2,logical_predicate,hidden
;
 nchan = -1
 IF N_ELEMENTS( channel ) GT 0 THEN nchan = FIX( channel[0] )
 ipix = -1
 IF N_ELEMENTS( pixelID ) GT 0 THEN ipix = FIX( pixelID[0] )
;
 dd = H5D_OPEN( gid, 'metaTable' )
 IF dd LT 0 THEN MESSAGE, 'Could not open dataset: metaTable'
 space_id = H5D_GET_SPACE( dd )
 dims = H5S_GET_SIMPLE_EXTENT_DIMS( space_id )
 IF N_ELEMENTS( orbitRange ) EQ 1 THEN BEGIN
    IF orbitRange GT 0 THEN BEGIN
       numOrbits = 1
       iorbit = MIN( orbitRange < dims[0] )
    ENDIF ELSE BEGIN
       numOrbits = dims[0]
       iorbit = 1
    ENDELSE
 ENDIF ELSE BEGIN
    orbitRange[0] = (orbitRange[0] < dims[0])
    orbitRange[1] = (orbitRange[1] < dims[0])

    numOrbits = ABS(orbitRange[1] - orbitRange[0]) + 1
    iorbit = MIN( orbitRange )
 ENDELSE
 mem_space_id = H5S_CREATE_SIMPLE( [numOrbits] )
 IF keyword_set(use_neighbours) AND numOrbits EQ 1 THEN BEGIN
    offs = 0
    REPEAT BEGIN
       no = (iorbit + offs - 1) > 0 < (dims[0]-1)
       H5S_SELECT_HYPERSLAB, space_id, [no], [numOrbits], /reset
       mtbl = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
       IF mtbl.stateCount EQ 0 THEN BEGIN
          offs = (offs GT 0) ? (-offs) : (1 - offs)
       ENDIF
    ENDREP UNTIL mtbl.stateCount GT 0 OR ABS(offs) GT 16384

    IF mtbl.stateCount EQ 0 THEN $
       MESSAGE, 'Fatal: no solution found for orbit: ' + STRING(iorbit) $
    ELSE BEGIN
       IF ABS(offs) GT 28 THEN $
          MESSAGE, 'Warning: no solution found within 28 orbit ', /INFO
       iorbit += offs
    ENDELSE
 ENDIF ELSE BEGIN
    H5S_SELECT_HYPERSLAB, space_id, [iorbit-1], [numOrbits], /reset
    mtbl = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
 ENDELSE
 H5S_CLOSE, mem_space_id
 H5S_CLOSE, space_id
 H5D_CLOSE, dd

 dd = H5D_OPEN( gid, 'analogOffset' )
 IF dd LT 0 THEN MESSAGE, 'Could not open dataset: analogOffset'
 space_id = H5D_GET_SPACE( dd )
 IF nchan GT 0 THEN BEGIN
    mem_space_id = H5S_CREATE_SIMPLE( [1024, numOrbits] )
    H5S_SELECT_HYPERSLAB, space_id, [(nchan-1)*1024, iorbit-1], $
                          [1024, numOrbits], /reset
 ENDIF ELSE IF ipix GT 0 THEN BEGIN
    mem_space_id = H5S_CREATE_SIMPLE( [1, numOrbits] )
    H5S_SELECT_HYPERSLAB, space_id, [ipix-1, iorbit-1], $
                          [1, numOrbits], /reset
 ENDIF ELSE BEGIN
    mem_space_id = H5S_CREATE_SIMPLE( [8192, numOrbits] )
    H5S_SELECT_HYPERSLAB, space_id, [0, iorbit-1], $
                          [8192, numOrbits], /reset
 ENDELSE
 analogOffset = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
 H5S_CLOSE, mem_space_id
 H5S_CLOSE, space_id
 H5D_CLOSE, dd
 IF ipix GT 0 THEN analogOffset = REFORM( analogOffset )

 dd = H5D_OPEN( gid, 'darkCurrent' )
 IF dd LT 0 THEN MESSAGE, 'Could not open dataset: darkCurrent'
 space_id = H5D_GET_SPACE( dd )
 IF nchan GT 0 THEN BEGIN
    mem_space_id = H5S_CREATE_SIMPLE( [1024, numOrbits] )
    H5S_SELECT_HYPERSLAB, space_id, [(nchan-1)*1024, iorbit-1], $
                          [1024, numOrbits], /reset
 ENDIF ELSE IF ipix GT 0 THEN BEGIN
    mem_space_id = H5S_CREATE_SIMPLE( [1, numOrbits] )
    H5S_SELECT_HYPERSLAB, space_id, [ipix-1, iorbit-1], $
                          [1, numOrbits], /reset
 ENDIF ELSE BEGIN
    mem_space_id = H5S_CREATE_SIMPLE( [8192, numOrbits] )
    H5S_SELECT_HYPERSLAB, space_id, [0, iorbit-1], $
                          [8192, numOrbits], /reset
 ENDELSE
 darkCurrent = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
 H5S_CLOSE, mem_space_id
 H5S_CLOSE, space_id
 H5D_CLOSE, dd
 IF ipix GT 0 THEN darkCurrent = REFORM( darkCurrent )

 IF ARG_PRESENT( analogOffsError ) THEN BEGIN
    dd = H5D_OPEN( gid, 'analogOffsetError' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: analogOffsetError'
    space_id = H5D_GET_SPACE( dd )
    IF nchan GT 0 THEN BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [1024, numOrbits] )
       H5S_SELECT_HYPERSLAB, space_id, [(nchan-1)*1024, iorbit-1], $
                             [1024, numOrbits], /reset
    ENDIF ELSE IF ipix GT 0 THEN BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [1, numOrbits] )
       H5S_SELECT_HYPERSLAB, space_id, [ipix-1, iorbit-1], $
                             [1, numOrbits], /reset
    ENDIF ELSE BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [8192, numOrbits] )
       H5S_SELECT_HYPERSLAB, space_id, [0, iorbit-1], $
                             [8192, numOrbits], /reset
    ENDELSE
    analogOffsError = $
       H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id  )
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
    H5D_CLOSE, dd
    IF ipix GT 0 THEN analogOffsError = REFORM( analogOffsError )
 ENDIF

 IF ARG_PRESENT( darkCurrentError ) THEN BEGIN
    dd = H5D_OPEN( gid, 'darkCurrentError' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: darkCurrentError'
    space_id = H5D_GET_SPACE( dd )
    IF nchan GT 0 THEN BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [1024, numOrbits] )
       H5S_SELECT_HYPERSLAB, space_id, [(nchan-1)*1024, iorbit-1], $
                             [1024, numOrbits], /reset
    ENDIF ELSE IF ipix GT 0 THEN BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [1, numOrbits] )
       H5S_SELECT_HYPERSLAB, space_id, [ipix-1, iorbit-1], $
                             [1, numOrbits], /reset
    ENDIF ELSE BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [8192, numOrbits] )
       H5S_SELECT_HYPERSLAB, space_id, [0, iorbit-1], $
                             [8192, numOrbits], /reset
    ENDELSE
    darkCurrentError = $
       H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
    H5D_CLOSE, dd
    IF ipix GT 0 THEN darkCurrentError = REFORM( darkCurrentError )
 ENDIF

 IF ARG_PRESENT( stateCount ) THEN BEGIN
    dd = H5D_OPEN( gid, 'stateCount' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: stateCount'
    space_id = H5D_GET_SPACE( dd )
    IF nchan GT 0 THEN BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [1, numOrbits] )
       H5S_SELECT_HYPERSLAB, space_id, [nchan-1, iorbit-1], $
                             [1, numOrbits], /reset
    ENDIF ELSE IF ipix GT 0 THEN BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [1, numOrbits] )
       H5S_SELECT_HYPERSLAB, space_id, [(ipix-1)/1024, iorbit-1], $
                             [1, numOrbits], /reset
    ENDIF ELSE BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [8, numOrbits] )
       H5S_SELECT_HYPERSLAB, space_id, [0, iorbit-1], $
                             [8, numOrbits], /reset
    ENDELSE
    stateCount = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
    H5D_CLOSE, dd
    IF ipix GT 0 THEN stateCount = REFORM( stateCount )
 ENDIF

 IF ARG_PRESENT( meanNoise ) THEN BEGIN
    dd = H5D_OPEN( gid, 'meanNoise' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: meanNoise'
    space_id = H5D_GET_SPACE( dd )
    IF nchan GT 0 THEN BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [1024, numOrbits] )
       H5S_SELECT_HYPERSLAB, space_id, [(nchan-1)*1024, iorbit-1], $
                             [1024, numOrbits], /reset
    ENDIF ELSE IF ipix GT 0 THEN BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [1, numOrbits] )
       H5S_SELECT_HYPERSLAB, space_id, [ipix-1, iorbit-1], $
                             [1, numOrbits], /reset
    ENDIF ELSE BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [8192, numOrbits] )
       H5S_SELECT_HYPERSLAB, space_id, [0, iorbit-1], $
                             [8192, numOrbits], /reset
    ENDELSE
    meanNoise = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
    H5D_CLOSE, dd
    IF ipix GT 0 THEN meanNoise = REFORM( meanNoise )
 ENDIF

 IF ARG_PRESENT( chiSquareFit ) THEN BEGIN
    dd = H5D_OPEN( gid, 'chiSquareFit' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: chiSquareFit'
    space_id = H5D_GET_SPACE( dd )
    IF nchan GT 0 THEN BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [1024, numOrbits] )
       H5S_SELECT_HYPERSLAB, space_id, [(nchan-1)*1024, iorbit-1], $
                             [1024, numOrbits], /reset
    ENDIF ELSE IF ipix GT 0 THEN BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [1, numOrbits] )
       H5S_SELECT_HYPERSLAB, space_id, [ipix-1, iorbit-1], $
                             [1, numOrbits], /reset
    ENDIF ELSE BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [8192, numOrbits] )
       H5S_SELECT_HYPERSLAB, space_id, [0, iorbit-1], $
                             [8192, numOrbits], /reset
    ENDELSE
    chiSquareFit = $
       H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
    H5D_CLOSE, dd
    IF ipix GT 0 THEN chiSquareFit = REFORM( chiSquareFit )
 ENDIF
 
 IF ARG_PRESENT( probabilityFit ) THEN BEGIN
    dd = H5D_OPEN( gid, 'probabilityFit' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: probabilityFit'
    space_id = H5D_GET_SPACE( dd )
    IF nchan GT 0 THEN BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [1024, numOrbits] )
       H5S_SELECT_HYPERSLAB, space_id, [(nchan-1)*1024, iorbit-1], $
                             [1024, numOrbits], /reset
    ENDIF ELSE IF ipix GT 0 THEN BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [1, numOrbits] )
       H5S_SELECT_HYPERSLAB, space_id, [ipix-1, iorbit-1], $
                             [1, numOrbits], /reset
    ENDIF ELSE BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [8192, numOrbits] )
       H5S_SELECT_HYPERSLAB, space_id, [0, iorbit-1], $
                             [8192, numOrbits], /reset
    ENDELSE
    probabilityFit = $
       H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
    H5D_CLOSE, dd
    IF ipix GT 0 THEN probabilityFit = REFORM( probabilityFit )
 ENDIF
 RETURN
END

;---------------------------------------------------------------------------
PRO SDMF_READ_FITTEDDARK, orbitRange_in, mtbl, analogOffset, darkCurrent, $
                          SDMF_H5_DB=SDMF_H5_DB, SDMF_VERSION=SDMF_VERSION, $
                          _REF_EXTRA=EXTRA, status=status

 compile_opt idl2,logical_predicate,hidden

 mtbl = -1
 status = -1
 IF N_PARAMS() NE 4 THEN BEGIN
    MESSAGE, ' Usage: SDMF_READ_FITTEDDARK, orbit, mtbl' $
             + ', analogOffset, darkCurrent' $
             + ', analogOffsError=analogOffsError' $
             + ', darkCurrentError=darkCurrentError' $
             + ', chiSquareFit=chiSquareFit, meanNoise=meanNoise' $
             + ', probabilityFit=probabilityFit, stateCount=stateCount' $
             + ', channel=channel, pixelID=pixelID, /use_neighbours' $
             + ', SDMF_H5_DB=SDMF_H5_DB, SDMF_VERSION=SDMF_VERSION' $
             + ', status=status', /INFO
    RETURN
 ENDIF
 orbitRange = LONG( orbitRange_in )

 IF N_ELEMENTS( SDMF_VERSION ) GT 0 THEN BEGIN
    IF SDMF_VERSION EQ '2.4' THEN $
       MESSAGE, 'FATAL: not implemented for SDMF v2.4.x'
 ENDIF

 IF N_ELEMENTS( SDMF_H5_DB ) EQ 0 THEN $
    SDMF_H5_DB = GET_SDMF_PATH( SDMF_VERSION=SDMF_VERSION ) + 'sdmf_dark.h5'
 IF (~ FILE_TEST( SDMF_H5_DB, /READ, /NOEXPAND )) THEN BEGIN
    MESSAGE, ' Can not read from database: ' + SDMF_H5_DB, /INFO
    RETURN
 ENDIF

 fid = H5F_OPEN( SDMF_H5_DB )
 IF fid LT 0 THEN MESSAGE, 'Could not open database: ' + SDMF_H5_DB
 IF N_ELEMENTS( SDMF_VERSION ) EQ 0 THEN BEGIN
    aid = H5A_OPEN_NAME( fid, 'SDMF_VERSION' )
    SDMF_VERSION = H5A_READ( aid )
    H5A_CLOSE, aid
 ENDIF
 IF SDMF_VERSION EQ '3.1' THEN BEGIN
    gid = H5G_OPEN( fid, 'DarkFit' )
    IF gid LT 0 THEN MESSAGE, 'Could not open group: DarkFit'
 ENDIF

 CASE SDMF_VERSION OF
    '3.0': BEGIN
       __GET_30_FITTED_DARK, fid, orbitRange, mtbl, $
          analogOffset, darkCurrent, _STRICT_EXTRA=EXTRA
    END
    '3.1': BEGIN
       __GET_31_FITTED_DARK, gid, orbitRange, mtbl, $
          analogOffset, darkCurrent, _STRICT_EXTRA=EXTRA
    END
    ELSE: BEGIN
       MESSAGE, 'Not available for SDMF version ' + SDMF_VERSION
    END
 ENDCASE

 IF SDMF_VERSION EQ '3.1' THEN H5G_CLOSE, gid
 H5F_CLOSE, fid
 status = 0
 RETURN
END
