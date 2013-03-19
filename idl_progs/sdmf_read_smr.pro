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
;	read rows of given orbit from SMR metaTable. In
;	addition, one can obtain SMR correction parameters.
;
; CATEGORY:
;	SDMF - SCIA calibration
;
; CALLING SEQUENCE:
;	SDMF_READ_SMR, absOrbit, mtbl, smr
;
; INPUTS:
;	absOrbit:	(absolute) Orbit number, a scalar, range [1...]
;
; OUTPUTS:
;	mtbl:           structure with rows of table metaTable
;       smr:            sun mean reference spectrum
;
; KEYWORD PARAMETERS:
;     use_neighbours:   flag to enable looking for neighbouring orbits
;
;     readouts:         240 readouts used for sun mean reference spectrum output
;     sunaz:            solar azimuth angle output
;     sunel:            solar elevation angle output
;     asm:              asm angle
;     esm:              esm angle
;
;     normalise:        normalise spectrum to BU/s
;     order_ch2:        flag to reverse channel 2 from measurement to science 
;                        order
;     pixelRange:       two element array with start and end pixel,
;                       range [0..8191]
;     SDMF_H5_DB:       path to the SDMF SMR database
;                         default: /SCIA/SDMF30/sdmf_smr.h5
;     SDMF_VERSION:     SDMF version <3.0>, default 3.0
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
;-
;---------------------------------------------------------------------------
FUNCTION __READ_SMRSET, dd, numIndx, metaIndx, pixelStart, pixelLength
 compile_opt idl2,logical_predicate,hidden

 space_id = H5D_GET_SPACE( dd )
 dims = H5S_GET_SIMPLE_EXTENT_DIMS( space_id )
 IF numIndx LT dims[0] THEN BEGIN
    databuffer = FLTARR( pixelLength, numIndx )
    mem_space_id = H5S_CREATE_SIMPLE( [1,pixelLength] )
    FOR ii = 0, numIndx-1 DO BEGIN
       H5S_SELECT_HYPERSLAB, space_id, [metaIndx[ii],pixelStart], $
                             [1,pixelLength], /reset
       tmp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
       databuffer[*,ii] = tmp
    ENDFOR
    H5S_CLOSE, mem_space_id
 ENDIF ELSE BEGIN
    mem_space_id = H5S_CREATE_SIMPLE( [numIndx,pixelLength] )
    H5S_SELECT_HYPERSLAB, space_id, [0,pixelStart], $
                          [numIndx,pixelLength], /reset
    databuffer = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
    H5S_CLOSE, mem_space_id
    databuffer = TRANSPOSE(databuffer)
 ENDELSE
 H5S_CLOSE, space_id 
 RETURN, databuffer
END

; ---------------------------------------------------------------------------
PRO SDMF_READ_SMR, orbitRange_in, mtbl, smr, readouts=readouts, $
                   sunaz=sunaz, sunel=sunel, asm=asm, esm=esm, $
		   pixelRange=pixelRange, use_neighbours=use_neighbours, $
                   normalise=normalise, order_ch2=order_ch2, $
                   SDMF_H5_DB=SDMF_H5_DB, SDMF_VERSION=SDMF_VERSION, $
                   status=status
 compile_opt idl2,logical_predicate,hidden

 mtbl = -1
 status = -1
 IF N_PARAMS() NE 3 THEN BEGIN
    MESSAGE, ' Usage: SDMF_READ_SMR, orbit, mtbl, smr, readouts=readouts' $
             + ', sunaz=sunaz, sunel=sunel, asm=asm, esm=esm' $
             + ', pixelRange=pixelRange, use_neighbours=use_neighbours' $
             + ', normalise=normalise, order_ch2=order_ch2' $
             + ', SDMF_H5_DB=SDMF_H5_DB, SDMF_VERSION=SDMF_VERSION' $
             + ', status=status', /INFO
    RETURN
 ENDIF
 orbitRange = LONG( orbitRange_in )
 IF ~ keyword_set(use_neighbours) THEN use_neighbours = 0

 IF N_ELEMENTS( SDMF_VERSION ) GT 0 THEN BEGIN
    IF SDMF_VERSION EQ '2.4' THEN $
       MESSAGE, 'FATAL: not implemented for SDMF v2.4.x'
 ENDIF

 CASE N_ELEMENTS(pixelRange) OF
    0: BEGIN
       pixelStart = 0L
       pixelLength = 8192L
       pixelRange = [0L, 8192 - 1L]
    END
    1: BEGIN
       pixelStart = pixelRange
       pixelLength = 1L
       pixelRange = [pixelRange, pixelRange]
    END
    2: BEGIN
       pixelRange[0] >= 0L
       pixelRange[1] <= 8192 - 1L
       
       pixelStart = pixelRange[0]
       pixelLength = (pixelRange[1] - pixelRange[0] + 1L)
    END
 ENDCASE
 pets = REBIN( [.0625,.0625,.03125,.03125,.0625,.03125,.0625,.125], $
               8192, /sample )
 pets = pets[pixelrange[0]:pixelrange[1]]

 IF N_ELEMENTS( SDMF_H5_DB ) EQ 0 THEN $
    SDMF_H5_DB = GET_SDMF_PATH( SDMF_VERSION=SDMF_VERSION ) + './sdmf_smr.h5'
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
 dd = H5D_OPEN( fid, 'orbitList' )
 orbitList = H5D_READ( dd )
 H5D_CLOSE, dd
 IF N_ELEMENTS( orbitRange ) EQ 1 THEN BEGIN
    IF orbitRange LT 0 THEN BEGIN
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

 dd = H5D_OPEN( fid, 'SMR' )
 IF dd LT 0 THEN MESSAGE, 'Could not open dataset: SMR'
 smr = __READ_SMRSET( dd, numIndx, metaIndx, pixelStart, pixelLength )
 H5D_CLOSE, dd
 smr = REFORM(smr)
 IF keyword_set(normalise) THEN smr /= pets
 IF keyword_set(order_ch2) THEN $
    smr[1024:2*1024-1] = REVERSE( smr[1024:2*1024-1] )

 IF ARG_PRESENT( readouts) THEN BEGIN
    dd = H5D_OPEN( fid, 'readouts' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: readouts'
    space_id = H5D_GET_SPACE( dd )
    dims = H5S_GET_SIMPLE_EXTENT_DIMS( space_id )
    print, numIndx, dims
    IF numIndx LT dims[0] THEN BEGIN
       readouts = FLTARR( pixelLength, 240, numIndx )
       mem_space_id = H5S_CREATE_SIMPLE( [1,pixelLength,240] )
       FOR ii = 0, numIndx-1 DO BEGIN
          H5S_SELECT_HYPERSLAB, space_id, [metaIndx[ii],pixelStart,0], $
                                [1,pixelLength,240], /reset
          tmp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
          readouts[*,*,ii] = REFORM(tmp)
       ENDFOR
       H5S_CLOSE, mem_space_id
    ENDIF ELSE BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [numIndx,pixelLength,240] )
       H5S_SELECT_HYPERSLAB, space_id, [0,pixelStart,0], $
                             [numIndx,pixelLength,240], /reset
       readouts = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
       H5S_CLOSE, mem_space_id
       readouts = readouts
    ENDELSE
    H5S_CLOSE, space_id 
 ENDIF

 IF ARG_PRESENT( asm ) THEN BEGIN
    dd = H5D_OPEN( fid, 'asm' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: asm'
    space_id = H5D_GET_SPACE( dd )
    dims = H5S_GET_SIMPLE_EXTENT_DIMS( space_id )
    mem_space_id = H5S_CREATE_SIMPLE( [1, 240] )
    H5S_SELECT_HYPERSLAB, space_id, [metaIndx, 0], [1, 240], /reset
    asm = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
    H5D_CLOSE, dd
    asm = REFORM(asm)
 ENDIF

 IF ARG_PRESENT( esm ) THEN BEGIN
    dd = H5D_OPEN( fid, 'esm' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: esm'
    space_id = H5D_GET_SPACE( dd )
    dims = H5S_GET_SIMPLE_EXTENT_DIMS( space_id )
    mem_space_id = H5S_CREATE_SIMPLE( [1, 240] )
    H5S_SELECT_HYPERSLAB, space_id, [metaIndx, 0], [1, 240], /reset
    esm = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
    H5D_CLOSE, dd
    esm = REFORM(esm)
 ENDIF

 IF ARG_PRESENT( sunaz) THEN BEGIN
    dd = H5D_OPEN( fid, 'sunaz' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: sunaz'
    space_id = H5D_GET_SPACE( dd )
    dims = H5S_GET_SIMPLE_EXTENT_DIMS( space_id )
    mem_space_id = H5S_CREATE_SIMPLE( [1, 240] )
    H5S_SELECT_HYPERSLAB, space_id, [metaIndx, 0], [1, 240], /reset
    sunaz = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
    H5D_CLOSE, dd
    sunaz = REFORM(sunaz)
 ENDIF

 IF ARG_PRESENT( sunel ) THEN BEGIN
    dd = H5D_OPEN( fid, 'sunel' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: sunel'
    space_id = H5D_GET_SPACE( dd )
    dims = H5S_GET_SIMPLE_EXTENT_DIMS( space_id )
    mem_space_id = H5S_CREATE_SIMPLE( [1, 240] )
    H5S_SELECT_HYPERSLAB, space_id, [metaIndx, 0], [1, 240], /reset
    sunel = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
    H5D_CLOSE, dd
    sunel = REFORM(sunel)
 ENDIF

 H5F_CLOSE, fid
 status = 0
 RETURN
END
