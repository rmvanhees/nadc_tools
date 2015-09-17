;
; COPYRIGHT (c) 2008 - 2013 SRON (R.M.van.Hees@sron.nl)
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
;	SDMF_READ_SMR
;
; PURPOSE:
;	obtain Sun Mean Reference spectra
;
; CATEGORY:
;	Sciamachy - SDMF
;
; CALLING SEQUENCE:
;	SDMF_READ_SMR, orbit, mtbl, smr
;
; INPUTS:
;	orbit:          absolute orbit number, a scalar or vector[min,max], 
;                       to select all orbits use -1
;
; OUTPUTS:
;	mtbl:           structure with rows of table metaTable
;       smr:            sun mean reference spectrum
;
; KEYWORD PARAMETERS:
;     ---- SDMF 3.0 ---
;     order_ch2:        flag to reverse channel 2 from measurement to science 
;                        order
;     ---- SDMF 3.1 ---
;     -----------------
;     normalise:        normalise spectrum to BU/s
;     channel:          channel ID, named scalar, range [1..8]
;     pixelID:          pixel ID, named scalar, range [1...8192]
;                       Note: pixelID ignored when channel is set
;
;     SDMF_H5_DB:       path to the SDMF SMR database
;                         default: /SCIA/SDMF/3.0/sdmf_smr.h5
;     SDMF_VERSION:     SDMF version <3.0|3.1>, default 3.0
;     status :          returns named variable with error flag (0 = ok)
;
; PROCEDURE:
;	ToDo...
;
; EXAMPLE:
;	ToDo...
;
; MODIFICATION HISTORY:
; 	Written by:	Pieter van der Meer (SRON), May 2009
;       Modified by:    RvH, 25 January 2012
;                         complete rewrite in using IDL HDF5 calls
;       Modified by:    RvH, 28 March 2012
;                       * coding according new standard
;       Modified by:    RvH, 28 March 2012
;                       * re-write and added SDMF v3.1
;-
;---------------------------------------------------------------------------
PRO __GET_30_SMR_DATA, fid, orbitRange, mtbl, smr, $
   channel=channel, pixelID=pixelID, normalise=normalise, order_ch2=order_ch2
 compile_opt idl2,logical_predicate,hidden
;
 nchan = -1
 IF N_ELEMENTS( channel ) GT 0 THEN nchan = FIX( channel[0] )
 ipix = -1
 IF N_ELEMENTS( pixelID ) GT 0 THEN ipix = FIX( pixelID[0] )
;
; obtain indices to entries of the requested orbit and state
;
 dd = H5D_OPEN( fid, 'orbitList' )
 orbitList = H5D_READ( dd )
 H5D_CLOSE, dd
 IF N_ELEMENTS( orbitRange ) EQ 1 THEN BEGIN
    IF orbitRange EQ -1 THEN BEGIN
       numIndx = N_ELEMENTS( orbitList )
    ENDIF ELSE BEGIN
       diff = MIN( ABS( orbitList - orbitRange ), indx )
       IF diff LE 50 THEN $
          metaIndx = WHERE( orbitList EQ orbitList[indx], numIndx ) $
       ELSE $
          numIndx = 0
    ENDELSE
 ENDIF ELSE BEGIN
    metaIndx = WHERE( orbitList GE orbitRange[0] $
                      AND orbitList LE orbitRange[1], numIndx )
 ENDELSE
 IF numIndx EQ 0 THEN BEGIN
    mtbl = !NULL
    MESSAGE, 'no entry found in database: sdmf_smr.h5', /INFO
    RETURN
 ENDIF
;
 dd = H5D_OPEN( fid, 'metaTable' )
 IF dd LT 0 THEN MESSAGE, 'Could not open dataset: metaTable'
 space_id = H5D_GET_SPACE( dd )
 dims = H5S_GET_SIMPLE_EXTENT_DIMS( space_id )
 IF numIndx LT dims[0] THEN BEGIN
    mem_space_id = H5S_CREATE_SIMPLE( [1] )
    H5S_SELECT_HYPERSLAB, space_id, [metaIndx[0]], [1], /RESET
    temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
    mtbl = REPLICATE( temp, numIndx )
    mtbl[0] = temp
    FOR ii = 1, numIndx-1 DO BEGIN
       H5S_SELECT_HYPERSLAB, space_id, [metaIndx[ii]], [1], /RESET
       mtbl[ii] = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
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
 dd = H5D_OPEN( fid, 'SMR' )
 IF dd LT 0 THEN MESSAGE, 'Could not open dataset: SMR'
 space_id = H5D_GET_SPACE( dd )
 IF nchan GT 0 THEN BEGIN
    IF numIndx LT dims[0] THEN BEGIN
       smr = FLTARR( 1024, numIndx, /NOZERO )
       mem_space_id = H5S_CREATE_SIMPLE( [1, 1024] )
       FOR ii = 0, numIndx-1 DO BEGIN
          H5S_SELECT_HYPERSLAB, space_id, [metaIndx[ii], (nchan-1)*1024], $
                                [1, 1024], /RESET
          temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
          smr[*,ii] = temp
       ENDFOR
    ENDIF ELSE BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [numIndx, 1024] )
       H5S_SELECT_HYPERSLAB, space_id, [0, (nchan-1)*1024], $
                             [numIndx,1024], /RESET
       temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
       smr = TRANSPOSE( temp )
    ENDELSE
 ENDIF ELSE IF ipix GT 0 THEN BEGIN
    IF numIndx LT dims[0] THEN BEGIN
       smr = FLTARR( numIndx, /NOZERO )
       mem_space_id = H5S_CREATE_SIMPLE( [1, 1] )
       FOR ii = 0, numIndx-1 DO BEGIN
          H5S_SELECT_HYPERSLAB, space_id, [metaIndx[ii], ipix-1], [1,1], /RESET
          temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
          smr[ii] = temp
       ENDFOR
    ENDIF ELSE BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [numIndx, 1] )
       H5S_SELECT_HYPERSLAB, space_id, [0, ipix-1], [numIndx, 1], /RESET
       smr = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
    ENDELSE
 ENDIF ELSE BEGIN
    IF numIndx LT dims[0] THEN BEGIN
       smr = FLTARR( 8192, numIndx, /NOZERO )
       mem_space_id = H5S_CREATE_SIMPLE( [1, 8192] )
       FOR ii = 0, numIndx-1 DO BEGIN
          H5S_SELECT_HYPERSLAB, space_id, [metaIndx[ii], 0], [1, 8192], /RESET
          temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
          smr[*,ii] = temp
       ENDFOR
    ENDIF ELSE BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [numIndx, 8192] )
       H5S_SELECT_HYPERSLAB, space_id, [0, 0], [numIndx, 8192], /RESET
       temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
       smr = TRANSPOSE( temp )
    ENDELSE
 ENDELSE
 H5S_CLOSE, mem_space_id
 H5S_CLOSE, space_id 
 H5D_CLOSE, dd
;
 smr = REFORM(smr)
 IF keyword_set(normalise) THEN BEGIN
    pets = REBIN( [.0625,.0625,.03125,.03125,.0625,.03125,.0625,.125], $
                  8192, /sample )
    pets[5 * 1024:-1] -= 1.18125e-3
    IF nchan GT 0 THEN BEGIN
       pets = pets[(nchan-1)*1024:nchan*1024-1]
    ENDIF ELSE IF ipix GT 0 THEN BEGIN
       pets = pets[ipix]
    ENDIF
    smr /= pets
 ENDIF
 IF keyword_set(order_ch2) THEN BEGIN
    IF nchan EQ -1 THEN $
       smr[1024:2*1024-1,*] = REVERSE( smr[1024:2*1024-1,*], 1 ) $
    ELSE IF nchan EQ 2 THEN $
       smr[0:1024-1,*] = REVERSE( smr[0:1024-1,*], 1 )
 ENDIF

 RETURN
END

;---------------------------------------------------------------------------
PRO __GET_31_SMR_DATA, fid, orbitRange, mtbl, smr, $
   channel=channel, pixelID=pixelID, normalise=normalise, order_ch2=order_ch2
 compile_opt idl2,logical_predicate,hidden
;
 nchan = -1
 IF N_ELEMENTS( channel ) GT 0 THEN nchan = FIX( channel[0] )
 ipix = -1
 IF N_ELEMENTS( pixelID ) GT 0 THEN ipix = FIX( pixelID[0] )
;
; obtain indices to entries of the requested orbit and state
;
 dd = H5D_OPEN( fid, 'orbitList' )
 orbitList = H5D_READ( dd )
 H5D_CLOSE, dd
 IF N_ELEMENTS( orbitRange ) EQ 1 THEN BEGIN
    IF orbitRange EQ -1 THEN BEGIN
       numIndx = N_ELEMENTS( orbitList )
    ENDIF ELSE BEGIN
       diff = MIN( ABS( orbitList - orbitRange ), indx )
       IF diff LE 50 THEN $
          metaIndx = WHERE( orbitList EQ orbitList[indx], numIndx ) $
       ELSE $
          numIndx = 0
    ENDELSE
 ENDIF ELSE BEGIN
    metaIndx = WHERE( orbitList GE orbitRange[0] $
                      AND orbitList LE orbitRange[1], numIndx )
 ENDELSE
 IF numIndx EQ 0 THEN BEGIN
    mtbl = !NULL
    MESSAGE, 'no entry found in database: sdmf_smr.h5', /INFO
    RETURN
 ENDIF
;
 dd = H5D_OPEN( fid, 'metaTable' )
 IF dd LT 0 THEN MESSAGE, 'Could not open dataset: metaTable'
 space_id = H5D_GET_SPACE( dd )
 dims = H5S_GET_SIMPLE_EXTENT_DIMS( space_id )
 IF numIndx LT dims[0] THEN BEGIN
    mem_space_id = H5S_CREATE_SIMPLE( [1] )
    H5S_SELECT_HYPERSLAB, space_id, [metaIndx[0]], [1], /RESET
    temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
    mtbl = REPLICATE( temp, numIndx )
    mtbl[0] = temp
    FOR ii = 1, numIndx-1 DO BEGIN
       H5S_SELECT_HYPERSLAB, space_id, [metaIndx[ii]], [1], /RESET
       mtbl[ii] = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
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
 dd = H5D_OPEN( fid, 'smr' )
 IF dd LT 0 THEN MESSAGE, 'Could not open dataset: smr'
 space_id = H5D_GET_SPACE( dd )
 IF nchan GT 0 THEN BEGIN
    IF numIndx LT dims[0] THEN BEGIN
       smr = FLTARR( 1024, numIndx, /NOZERO )
       mem_space_id = H5S_CREATE_SIMPLE( [1024, 1] )
       FOR ii = 0, numIndx-1 DO BEGIN
          H5S_SELECT_HYPERSLAB, space_id, [(nchan-1)*1024, metaIndx[ii]], $
                                [1024, 1], /RESET
          temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
          smr[*,ii] = temp
       ENDFOR
    ENDIF ELSE BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [1024, numIndx] )
       H5S_SELECT_HYPERSLAB, space_id, [(nchan-1)*1024, 0], $
                             [1024, numIndx], /RESET
       smr = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
    ENDELSE
 ENDIF ELSE IF ipix GT 0 THEN BEGIN
    IF numIndx LT dims[0] THEN BEGIN
       smr = FLTARR( numIndx, /NOZERO )
       mem_space_id = H5S_CREATE_SIMPLE( [1, 1] )
       FOR ii = 0, numIndx-1 DO BEGIN
          H5S_SELECT_HYPERSLAB, space_id, [ipix-1, metaIndx[ii]], [1, 1], /RESET
          temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
          smr[ii] = temp
       ENDFOR
    ENDIF ELSE BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [1, numIndx] )
       H5S_SELECT_HYPERSLAB, space_id, [ipix-1, 0], [1, numIndx], /RESET
       smr = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
    ENDELSE
 ENDIF ELSE BEGIN
    IF numIndx LT dims[0] THEN BEGIN
       smr = FLTARR( 8192, numIndx, /NOZERO )
       mem_space_id = H5S_CREATE_SIMPLE( [8192, 1] )
       FOR ii = 0, numIndx-1 DO BEGIN
          H5S_SELECT_HYPERSLAB, space_id, [0, metaIndx[ii]], [8192, 1], /RESET
          temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
          smr[*,ii] = temp
       ENDFOR
    ENDIF ELSE BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [8192, numIndx] )
       H5S_SELECT_HYPERSLAB, space_id, [0, 0], [8192, numIndx], /RESET
       smr = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
    ENDELSE
 ENDELSE
 H5S_CLOSE, mem_space_id
 H5S_CLOSE, space_id 
 H5D_CLOSE, dd
;
 smr = REFORM(smr)
 IF keyword_set(normalise) THEN BEGIN
    pets = REBIN( [.0625,.0625,.03125,.03125,.0625,.03125,.0625,.125], $
                  8192, /sample )
    pets[5 * 1024:-1] -= 1.18125e-3
    IF nchan GT 0 THEN BEGIN
       pets = pets[(nchan-1)*1024:nchan*1024-1]
    ENDIF ELSE IF ipix GT 0 THEN BEGIN
       pets = pets[ipix]
    ENDIF
    smr /= pets
 ENDIF
 IF keyword_set(order_ch2) THEN BEGIN
    IF nchan EQ -1 THEN $
       smr[1024:2*1024-1,*] = REVERSE( smr[1024:2*1024-1,*], 1 ) $
    ELSE IF nchan EQ 2 THEN $
       smr[0:1024-1,*] = REVERSE( smr[0:1024-1,*], 1 )
 ENDIF

 RETURN
END

; ---------------------------------------------------------------------------
PRO SDMF_READ_SMR, orbitRange_in, mtbl, smr, $
                   SDMF_H5_DB=SDMF_H5_DB, SDMF_VERSION=SDMF_VERSION, $
                   _REF_EXTRA=EXTRA, status=status 
 compile_opt idl2,logical_predicate,hidden

 mtbl = -1
 status = -1
 IF N_PARAMS() NE 3 THEN BEGIN
    MESSAGE, ' Usage: SDMF_READ_SMR, orbit, mtbl, smr' $
             + ', channel=channel, pixelID=pixelID' $
             + ', sunaz=sunaz, sunel=sunel, asm=asm, esm=esm' $
             + ', readouts=readouts, normalise=normalise, order_ch2=order_ch2' $
             + ', SDMF_H5_DB=SDMF_H5_DB, SDMF_VERSION=SDMF_VERSION' $
             + ', status=status', /INFO
    RETURN
 ENDIF
 orbitRange = LONG( orbitRange_in )

 IF N_ELEMENTS( SDMF_VERSION ) GT 0 THEN BEGIN
    IF SDMF_VERSION EQ '2.4' THEN $
       MESSAGE, 'FATAL: not implemented for SDMF v2.4.x'
 ENDIF
;
 IF N_ELEMENTS( SDMF_H5_DB ) EQ 0 THEN $
    SDMF_H5_DB = GET_SDMF_PATH( SDMF_VERSION=SDMF_VERSION ) + './sdmf_smr.h5'
 IF (~ FILE_TEST( SDMF_H5_DB, /READ, /NOEXPAND )) THEN BEGIN
    MESSAGE, ' Can not read from database: ' + SDMF_H5_DB, /INFO
    RETURN
 ENDIF
;
 fid = H5F_OPEN( SDMF_H5_DB )
 IF fid LT 0 THEN MESSAGE, 'Could not open database: ' + SDMF_H5_DB
 IF N_ELEMENTS( SDMF_VERSION ) EQ 0 THEN BEGIN
    aid = H5A_OPEN_NAME( fid, 'SDMF_VERSION' )
    SDMF_VERSION = H5A_READ( aid )
    H5A_CLOSE, aid
 ENDIF
;
 CASE SDMF_VERSION OF
    '3.0': BEGIN
       __GET_30_SMR_DATA, fid, orbitRange, mtbl, smr, _STRICT_EXTRA=EXTRA
    END
    '3.1': BEGIN
       __GET_31_SMR_DATA, fid, orbitRange, mtbl, smr, _STRICT_EXTRA=EXTRA
    END
    ELSE: BEGIN
       MESSAGE, 'Not available for SDMF version ' + SDMF_VERSION
    END
 ENDCASE
;
; close HDF5 resources
;
 H5F_CLOSE, fid
 status = 0
 RETURN
END
