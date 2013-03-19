;
; COPYRIGHT (c) 2012 - 2013 SRON (R.M.van.Hees@sron.nl)
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
;	SDMF_READ_PPG
;
; PURPOSE:
;	obtain pixel-to-pixel gain factors
;
; CATEGORY:
;	Sciamachy - SDMF
;
; CALLING SEQUENCE:
;	SDMF_READ_PPG, orbit, mtbl, pixelGain
;
; INPUTS:
;	orbit:	        absolute orbit number, a scalar or vector[min,max], 
;                       to select all orbits use -1
;
; KEYWORD PARAMETERS:
;       use_neighbours:    flag to enable looking for neighboring valid entries
;
;       channel:           channel ID, named scalar, range [1..8]
;       pixelID:           pixel ID, named scalar, range [1...8192]
;
;       SDMF_H5_DB:        path to the SDMF Dark database
;                            default: /SCIA/SDMF30/sdmf_ppg.h5
;       SDMF_VERSION:      SDMF version <3.0>, default 3.0
;       status :           returns named variable with error flag (0 = ok)
;
; OUTPUTS:
;       mtbl:           metaTable (array of structures)
;       pixelGain:      pixel-to-pixel gain factor
;
; PROCEDURE:
;	none
;
; EXAMPLE:
;	- obtain pixelGain for orbit=7502, all channels
;       IDL> SDMF_READ_PPG, 7502, mtbl, ppg
;
;	- obtain pixelGain for orbit=7502, channel=2
;       IDL> SDMF_READ_PPG, 7502, mtbl, ppg, channel=2
;
;	- obtain pixelGain for pixel=100 (all orbits)
;       IDL> SDMF_READ_PPG, -1, mtbl, ppg, pixel=100
;
;	- obtain pixelGain for orbit range
;       IDL> SDMF_READ_PPG, [11000,12000], mtbl, ppg
;
; MODIFICATION HISTORY:
;       Written by:     Richard van Hees (SRON), 29 March 2012
;-
; ---------------------------------------------------------------------------
PRO __GET_30_PIXELGAIN, fid, orbitRange, mtbl, pixelGain, $
   channel=channel, pixelID=pixelID, use_neighbours=use_neighbours
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
    MESSAGE, 'no entry found in database: sdmf_ppg.h5', /INFO
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
 dd = H5D_OPEN( fid, 'pixelGain' )
 IF dd LT 0 THEN MESSAGE, 'Could not open dataset: pixelGain'
 space_id = H5D_GET_SPACE( dd )
 IF nchan GT 0 THEN BEGIN
    IF numIndx LT dims[0] THEN BEGIN
       pixelGain = FLTARR( numIndx, 1024 )
       mem_space_id = H5S_CREATE_SIMPLE( [1,1024] )
       FOR ii = 0, numIndx-1 DO BEGIN
          H5S_SELECT_HYPERSLAB, space_id, [metaIndx[ii],(nchan-1)*1024], $
                                [1,1024], /reset
          tmp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
          pixelGain[ii,*] = tmp
       ENDFOR
       H5S_CLOSE, mem_space_id
    ENDIF ELSE BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [numIndx,1024] )
       H5S_SELECT_HYPERSLAB, space_id, [0,(nchan-1)*1024], $
                             [numIndx,1024], /reset
       pixelGain = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
       H5S_CLOSE, mem_space_id
    ENDELSE
 ENDIF ELSE IF ipix GT 0 THEN BEGIN
    IF numIndx LT dims[0] THEN BEGIN
       pixelGain = FLTARR( numIndx )
       mem_space_id = H5S_CREATE_SIMPLE( [1,1] )
       FOR ii = 0, numIndx-1 DO BEGIN
          H5S_SELECT_HYPERSLAB, space_id, [metaIndx[ii],ipix-1], [1,1], /reset
          tmp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
          pixelGain[ii] = tmp
       ENDFOR
       H5S_CLOSE, mem_space_id
    ENDIF ELSE BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [numIndx,1] )
       H5S_SELECT_HYPERSLAB, space_id, [0,ipix-1], [numIndx,1], /reset
       pixelGain = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
       H5S_CLOSE, mem_space_id
    ENDELSE
 ENDIF ELSE BEGIN
    IF numIndx LT dims[0] THEN BEGIN
       pixelGain = FLTARR( numIndx, 8192 )
       mem_space_id = H5S_CREATE_SIMPLE( [1,8192] )
       FOR ii = 0, numIndx-1 DO BEGIN
          H5S_SELECT_HYPERSLAB, space_id, [metaIndx[ii],0], [1,8192], /reset
          tmp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
          pixelGain[ii,*] = tmp
       ENDFOR
       H5S_CLOSE, mem_space_id
    ENDIF ELSE BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [numIndx,8192] )
       H5S_SELECT_HYPERSLAB, space_id, [0,0], [numIndx,8192], /reset
       pixelGain = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
       H5S_CLOSE, mem_space_id
    ENDELSE
 ENDELSE
 H5S_CLOSE, space_id
 H5D_CLOSE, dd
 IF numIndx EQ dims[0] THEN pixelGain = pixelGain[indx_sort,*]
 pixelGain = REFORM( pixelGain )

 RETURN
END

;---------------------------------------------------------------------------
PRO SDMF_READ_PPG, orbitRange_in, mtbl, pixelGain, $
                   SDMF_H5_DB=SDMF_H5_DB, SDMF_VERSION=SDMF_VERSION, $
                   _REF_EXTRA=EXTRA, status=status
 compile_opt idl2,logical_predicate,hidden

 mtbl = -1
 status = -1
 IF N_PARAMS() NE 3 THEN BEGIN
    MESSAGE, ' Usage: SDMF_READ_PPG, orbit, mtbl' $
             + ', pixelGain, channel=channel, pixelID=pixelID' $
             + ', SDMF_H5_DB=SDMF_H5_DB, SDMF_VERSION=SDMF_VERSION' $
             + ', /use_neighbours, status=status', /INFO
    RETURN
 ENDIF
 orbitRange = LONG( orbitRange_in )

 IF N_ELEMENTS( SDMF_VERSION ) GT 0 THEN BEGIN
    IF SDMF_VERSION EQ '2.4' THEN $
       MESSAGE, 'FATAL: not implemented for SDMF v2.4.x'
 ENDIF

 IF N_ELEMENTS( SDMF_H5_DB ) EQ 0 THEN $
    SDMF_H5_DB = GET_SDMF_PATH( SDMF_VERSION=SDMF_VERSION ) + 'sdmf_ppg.h5'
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

 CASE SDMF_VERSION OF
    '3.0': BEGIN
       __GET_30_PIXELGAIN, fid, orbitRange, mtbl, pixelGain, $
          _STRICT_EXTRA=EXTRA
    END
    ELSE: BEGIN
       MESSAGE, 'Not available for SDMF version ' + SDMF_VERSION
    END
 ENDCASE

 H5F_CLOSE, fid
 status = 0
 RETURN
END
