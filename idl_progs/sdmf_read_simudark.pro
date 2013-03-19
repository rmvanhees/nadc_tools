;
; COPYRIGHT (c) 2008 - 2013 SRON (P.van.der.Meer@sron.nl)
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
;       SDMF_READ_SIMUDARK
;
; PURPOSE:
;       read rows of given orbit from simultaneous fit darksignal metaTable. In
;       addition, one can obtain dark correction parameters.
;
; CATEGORY:
;       SDMF - SCIA calibration
;
; CALLING SEQUENCE:
;       SDMF_READ_SIMUDARK, nchan, orbit, mtbl, analogOffset, darkCurrent, 
;                           sig_ao=sig_ao, sig_lc=sig_lc, amp1=amp1,
;                           sig_amp1=sig_amp1, chisquare=chisquare, 
;                           use_neighbours=use_neighbours,
;                           SDMF_H5_DB=SDMF_H5_DB, SDMF_VERSION=SDMF_VERSION,
;                           pixelID=pixelID, status=status
;
; INPUTS:
;       nchan:          channel ID [6, 7 or 8]
;	orbit:	        absolute orbit number, a scalar or vector[min,max], 
;                       to select all orbits use -1
; OUTPUTS:
;       mtbl:           structure with rows of table metaTable
;
; KEYWORD PARAMETERS:
;       use_neighbours:    flag to enable looking for neighboring valid entries
;
;       sig_ao:            sigma of analogOffset
;       sig_lc:            sigma of leakage current
;       amp1:              amplitude first harmonic
;       sig_amp1:          sigma of amplitude first harmonic
;       chiSquareFit:      chi-square of fit
;
;       pixelID:           pixel ID, named scalar, range [1...8192]
;
;       SDMF_H5_DB:        path to the SDMF Dark database
;                            default: /SCIA/SDMF30/sdmf_simudark.h5
;       SDMF_VERSION:      SDMF version <3.0|3.1>, default 3.0
;       status :           returns named variable with error flag (0 = ok)
;
; OUTPUTS:
;       mtbl:           metaTable (array of structures)
;       analogOffset:   analog offset (BU) or fixed pattern noise
;       darkCurrent:    dark current (BU/s) or leakage current
;
; PROCEDURE:
;       TODO...
;
; EXAMPLE:
;       TODO...
;
; MODIFICATION HISTORY:
;       Written by:  Pieter van der Meer (SRON), July 2008
;       Modified: PvdM, added 1st harmonic, August 2008
;       Modified:  RvH, coding according new standard, 28 March 2012
;-
FUNCTION __READ_SIMUSET, dd, max_records, numIndx, metaIndx, ipix
 compile_opt idl2,logical_predicate,hidden

 space_id = H5D_GET_SPACE( dd )
 IF ipix GT 0 THEN BEGIN
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
       databuffer = FLTARR( numIndx, 1024 )
       mem_space_id = H5S_CREATE_SIMPLE( [1,1024] )
       FOR ii = 0, numIndx-1 DO BEGIN
          H5S_SELECT_HYPERSLAB, space_id, [metaIndx[ii],0], [1,1024], /reset
          tmp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
          databuffer[ii,*] = tmp
       ENDFOR
       H5S_CLOSE, mem_space_id
    ENDIF ELSE BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [numIndx,1024] )
       H5S_SELECT_HYPERSLAB, space_id, [0,0], [numIndx,1024], /reset
       databuffer = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
       H5S_CLOSE, mem_space_id
    ENDELSE
 ENDELSE
 H5S_CLOSE, space_id
 
 RETURN, databuffer
END

; ---------------------------------------------------------------------------
PRO SDMF_READ_SIMUDARK, nchan, orbitRange_in, mtbl, analogOffset, darkCurrent,$
                        sig_ao=sig_ao, sig_lc=sig_lc, $
                        amp1=amp1, sig_amp1=sig_amp1, $
                        chisquare=chisquare, use_neighbours=use_neighbours, $
                        SDMF_H5_DB=SDMF_H5_DB, SDMF_VERSION=SDMF_VERSION, $
                        pixelID=pixelID, status=status
  compile_opt idl2,logical_predicate,hidden
 mtbl = -1
 status = -1
 IF N_PARAMS() NE 5 THEN BEGIN
    MESSAGE, ' Usage: SDMF_READ_SIMUDARK, nchan, orbitRange_in, mtbl' $
             + ', analogOffset, darkCurrent, sig_ao=sig_ao, sig_lc=sig_lc' $
             + ', amp1=amp1, sig_amp1=sig_amp1' $
             + ', chisquare=chisquare, use_neighbours=use_neighbours' $
             + ', SDMF_H5_DB=SDMF_H5_DB, SDMF_VERSION=SDMF_VERSION' $
             + ', pixelID=pixelID, status=status', /INFO
    RETURN
 ENDIF
 orbitRange = LONG( orbitRange_in )
 ipix = -1
 IF N_ELEMENTS( pixelID ) GT 0 THEN ipix = FIX( pixelID[0] )
 IF ~ keyword_set(use_neighbours) THEN use_neighbours = 0

 IF N_ELEMENTS( SDMF_VERSION ) GT 0 THEN BEGIN
    IF SDMF_VERSION EQ '2.4' THEN $
       MESSAGE, 'FATAL: not implemented for SDMF v2.4.x'
 ENDIF

 IF N_ELEMENTS( SDMF_H5_DB ) EQ 0 THEN $
    SDMF_H5_DB = GET_SDMF_PATH( SDMF_VERSION=SDMF_VERSION ) $
                 + 'sdmf_simudark.h5'
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
 grpName = 'ch' + STRING(nchan, format='(I1)')
 gid = H5G_OPEN( fid, grpName )
 IF gid LT 0 THEN MESSAGE, 'Could not open group: ' + grpName
;
 dd = H5D_OPEN( gid, 'orbitList' )
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
;
 dd = H5D_OPEN( gid, 'metaTable' )
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
 dd = H5D_OPEN( gid, 'ao' )
 IF dd LT 0 THEN MESSAGE, 'Could not open dataset: ao'
 analogOffset = __READ_SIMUSET( dd, dims[0], numIndx, metaIndx, ipix )
 IF numIndx EQ dims[0] THEN BEGIN
    help, indx_sort
    analogOffset = analogOffset[indx_sort,*]
 ENDIF 
 analogOffset = REFORM( analogOffset )
 H5D_CLOSE, dd

 dd = H5D_OPEN( gid, 'lc' )
 IF dd LT 0 THEN MESSAGE, 'Could not open dataset: lc'
 darkCurrent = __READ_SIMUSET( dd, dims[0], numIndx, metaIndx, ipix )
 IF numIndx EQ dims[0] THEN darkCurrent = darkCurrent[indx_sort,*]
 darkCurrent = REFORM( darkCurrent )
 H5D_CLOSE, dd
;
 IF ARG_PRESENT( amp1 ) THEN BEGIN
    dd = H5D_OPEN( gid, 'amp1' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: amp1'
    amp1 = __READ_SIMUSET( dd, dims[0], numIndx, metaIndx, ipix )
    IF numIndx EQ dims[0] THEN amp1 = amp1[indx_sort,*]
    amp1 = REFORM( amp1 )
    H5D_CLOSE, dd
 ENDIF
;
 IF ARG_PRESENT( sig_amp1 ) THEN BEGIN
    dd = H5D_OPEN( gid, 'sig_amp1' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: sig_amp1'
    sig_amp1 = __READ_SIMUSET( dd, dims[0], numIndx, metaIndx, ipix )
    IF numIndx EQ dims[0] THEN sig_amp1 = sig_amp1[indx_sort,*]
    sig_amp1 = REFORM( sig_amp1 )
    H5D_CLOSE, dd
 ENDIF
;
 IF ARG_PRESENT( sig_ao ) THEN BEGIN
    dd = H5D_OPEN( gid, 'sig_ao' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: sig_ao'
    sig_ao = __READ_SIMUSET( dd, dims[0], numIndx, metaIndx, ipix )
    IF numIndx EQ dims[0] THEN sig_ao = sig_ao[indx_sort,*]
    sig_ao = REFORM( sig_ao )
    H5D_CLOSE, dd
 ENDIF
;
 IF ARG_PRESENT( sig_lc ) THEN BEGIN
    dd = H5D_OPEN( gid, 'sig_lc' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: sig_lc'
    sig_lc = __READ_SIMUSET( dd, dims[0], numIndx, metaIndx, ipix )
    IF numIndx EQ dims[0] THEN sig_lc = sig_lc[indx_sort,*]
    sig_lc = REFORM( sig_lc )
    H5D_CLOSE, dd
 ENDIF
;
 IF ARG_PRESENT( chisquare ) THEN BEGIN
    dd = H5D_OPEN( gid, 'chisq' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: chisq'
    chisquare = __READ_SIMUSET( dd, dims[0], numIndx, metaIndx, ipix )
    IF numIndx EQ dims[0] THEN chisquare = chisquare[indx_sort,*]
    chisquare = REFORM( chisquare )
    H5D_CLOSE, dd
 ENDIF

 H5G_CLOSE, gid
 H5F_CLOSE, fid
 status = 0
  RETURN
END
