;
; COPYRIGHT (c) 2011 - 2013 SRON (R.M.van.Hees@sron.nl)
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
;	SDMF_READ_EXTRACT_L1B
;
; PURPOSE:
;	obtain calibration state parameters from L1b key- database
;
; CATEGORY:
;	Sciamachy - SDMF
;
; CALLING SEQUENCE:
;	SDMF_READ_EXTRACT_L1B, procStage, orbit, mtbl
;
; INPUTS:
;       procStage:      character 'R', 'U' or 'W'
;	orbit:	        absolute orbit number, a scalar or vector[min,max], 
;                       or -1 to select all orbits
;
; OUTPUTS:
;       mtbl:           structure with rows of table metaTable
;
; KEYWORD PARAMETERS:
;       AO:
;       ERROR_AO:
;       LC:
;       ERROR_LC:
;       VLC:
;       ERROR_VLC:
;
;       BDPM:
;
;       GRID_BASE:
;       GRID_COEFFS:
;
;       SUN_DIFFUSER:
;
;       channel:        channel ID, named scalar, range [1..8]
;       pixelID:        pixel ID, named scalar, range [1...]
;
;       SDMF_H5_DB:     path to the SDMF calibration states database
;                         default: /SCIA/SDMF/3.1/sdmf_extract_l1b.h5
;       SDMF_VERSION:   SDMF version <3.1>, default 3.1
;       status :        returns named variable with error flag (0 = ok)
;
; PROCEDURE:
;	none
;
; EXAMPLE:
;       Please add!
;
; MODIFICATION HISTORY:
; 	Written by:	R.M. van Hees (SRON), 25-01-2011
;       Modified:  RvH, coding according new standard, 28 March 2012
;-
FUNCTION _RD_EXTRACT_L1B_ALL, ds_name, gid, numIndx, metaIndx
 compile_opt idl2,logical_predicate,hidden

 dd = H5D_OPEN( gid, ds_name )
 IF dd LT 0 THEN MESSAGE, 'Could not open dataset: ' + ds_name
 type_id = H5D_GET_TYPE( dd )
 dims = H5T_GET_ARRAY_DIMS( type_id )
 H5T_CLOSE, type_id
 space_id = H5D_GET_SPACE( dd )
 ext_dim = H5S_GET_SIMPLE_EXTENT_DIMS( space_id )
 
 IF numIndx EQ LONG(ext_dim) THEN $
    buff = H5D_READ( dd ) $
 ELSE IF numIndx EQ 1 THEN BEGIN
    mem_space_id = H5S_CREATE_SIMPLE( [1] )
    H5S_SELECT_HYPERSLAB, space_id, metaIndx[0], [1], /reset
    buff = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
 ENDIF ELSE BEGIN
    mem_space_id = H5S_CREATE_SIMPLE( [1] )
    H5S_SELECT_HYPERSLAB, space_id, metaIndx[0], [1], /reset
    temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
    IF SIZE( temp, /N_DIM ) EQ 1 THEN BEGIN
       buff = REPLICATE( temp[0], numIndx, dims )
       buff[0,*] = temp
       FOR ni = 1, numIndx-1 DO BEGIN
          H5S_SELECT_HYPERSLAB, space_id, metaIndx[ni], [1], /reset
          temp = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
          buff[ni,*] = temp
       ENDFOR
    ENDIF ELSE BEGIN
       buff = REPLICATE( temp[0], numIndx, dims[0], dims[1] )
       buff[0,*,*] = temp
       FOR ni = 1, numIndx-1 DO BEGIN
          H5S_SELECT_HYPERSLAB, space_id, metaIndx[ni], [1], /reset
          temp = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
          buff[ni,*,*] = temp
       ENDFOR
    ENDELSE
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
 ENDELSE
 H5D_CLOSE, dd

 RETURN, buff
END

;--------------------------------------------------
FUNCTION _RD_EXTRACT_L1B_CHAN, ds_name, gid, numIndx, metaIndx, channel
 compile_opt idl2,logical_predicate,hidden

 dd = H5D_OPEN( gid, ds_name )
 IF dd LT 0 THEN MESSAGE, 'Could not open dataset: ' + ds_name
 type_id = H5D_GET_TYPE( dd )
 dims = H5T_GET_ARRAY_DIMS( type_id )
 H5T_CLOSE, type_id
 space_id = H5D_GET_SPACE( dd )
 ext_dim = H5S_GET_SIMPLE_EXTENT_DIMS( space_id )

 IF dims[0] EQ 8 THEN $
    ich = channel-1 $
 ELSE IF dims[0] EQ 3072 THEN $
    ich = (channel-6) * 1024 + LINDGEN( 1024 ) $
 ELSE IF dims[0] EQ 8192 THEN $
    ich = (channel-1) * 1024 + LINDGEN( 1024 ) $
 ELSE $
    MESSAGE, 'Can not select channel data from data set: ' + ds_name
 
 IF numIndx EQ LONG(ext_dim) THEN BEGIN
    buff = (H5D_READ( dd ))[ich,*]
 ENDIF ELSE IF numIndx EQ 1 THEN BEGIN
    mem_space_id = H5S_CREATE_SIMPLE( [1] )
    H5S_SELECT_HYPERSLAB, space_id, metaIndx[0], [1], /reset
    buff = (H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id ))[ich]
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
 ENDIF ELSE BEGIN
    mem_space_id = H5S_CREATE_SIMPLE( [1] )
    H5S_SELECT_HYPERSLAB, space_id, metaIndx[0], [1], /reset
    temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
    IF N_ELEMENTS( ich ) EQ 1 THEN BEGIN
       buff = REPLICATE( temp[0], numIndx )
       buff[0] = temp[ich]
       FOR ni = 1, numIndx-1 DO BEGIN
          H5S_SELECT_HYPERSLAB, space_id, metaIndx[ni], [1], /reset
          temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
          buff[ni] = temp[ich]
       ENDFOR
    ENDIF ELSE BEGIN
       buff = REPLICATE( temp[0], numIndx, 1024 )
       buff[0,*] = temp[ich]
       FOR ni = 1, numIndx-1 DO BEGIN
          H5S_SELECT_HYPERSLAB, space_id, metaIndx[ni], [1], /reset
          temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
          buff[ni,*] = temp[ich]
       ENDFOR
    ENDELSE
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
 ENDELSE
 H5D_CLOSE, dd

 RETURN, buff
END

;--------------------------------------------------
FUNCTION _RD_EXTRACT_L1B_PIXEL, ds_name, gid, numIndx, metaIndx, pixelID
 compile_opt idl2,logical_predicate,hidden

 dd = H5D_OPEN( gid, ds_name )
 IF dd LT 0 THEN MESSAGE, 'Could not open dataset: ' + ds_name
 type_id = H5D_GET_TYPE( dd )
 dims = H5T_GET_ARRAY_DIMS( type_id )
 H5T_CLOSE, type_id
 space_id = H5D_GET_SPACE( dd )
 ext_dim = H5S_GET_SIMPLE_EXTENT_DIMS( space_id )

 IF dims[0] EQ 3072 THEN $
    ipx = pixelID - 5121 $
 ELSE IF dims[0] EQ 8192 THEN $
    ipx = pixelID - 1 $
 ELSE $
    MESSAGE, 'Can not select channel data from data set: ' + ds_name

 IF numIndx EQ LONG(ext_dim) THEN BEGIN
    buff = REFORMA((H5D_READ( dd ))[ipx,*])
 ENDIF ELSE IF numIndx EQ 1 THEN BEGIN
    mem_space_id = H5S_CREATE_SIMPLE( [1] )
    H5S_SELECT_HYPERSLAB, space_id, metaIndx[0], [1], /reset
    buff = (H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id ))[ipx]
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
 ENDIF ELSE BEGIN
    mem_space_id = H5S_CREATE_SIMPLE( [1] )
    H5S_SELECT_HYPERSLAB, space_id, metaIndx[0], [1], /reset
    temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
    buff = REPLICATE( temp[0], numIndx )
    buff[0] = temp[ipx]
    FOR ni = 1, numIndx-1 DO BEGIN
       H5S_SELECT_HYPERSLAB, space_id, metaIndx[ni], [1], /reset
       temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
       buff[ni] = temp[ipx]
    ENDFOR
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
 ENDELSE
 H5D_CLOSE, dd

 RETURN, buff
END

;--------------------------------------------------
PRO SDMF_READ_EXTRACT_L1B, procStage, orbit, mtbl, AO=AO, ERROR_AO=ERROR_AO, $
                           LC=LC, ERROR_LC=ERROR_LC, VLC=VLC, $
                           ERROR_VLC=ERROR_VLC, BDPM=BDPM, $
                           GRID_BASE=GRID_BASE, GRID_COEFFS=GRID_COEFFS, $
                           SUN_DIFFUSER=SUN_DIFFUSER, SDMF_H5_DB=SDMF_H5_DB, $
                           SDMF_VERSION=SDMF_VERSION, pixelID=pixelID, $
                           channel=channel, status=status
 compile_opt idl2,logical_predicate,hidden

 mtbl = -1
 status = -1
 IF N_PARAMS() NE 3 THEN BEGIN
    MESSAGE, ' Usage: SDMF_READ_EXTRACT_L1B, procStage, orbit, mtbl' $
             + ', AO=AO, ERROR_AO=ERROR_AO, LC=LC, ERROR_LC=ERROR_LC' $
             + ', VLC=VLC, ERROR_VLC=ERROR_VLC, BDPM=BDPM' $
             + ', GRID_BASE=GRID_BASE, GRID_COEFFS=GRID_COEFFS' $
             + ', SUN_DIFFUSER=SUN_DIFFUSER, SDMF_H5_DB=SDMF_H5_DB' $
             + ', SDMF_VERSION=SDMF_VERSION, pixelID=pixelID'$
             + ', channel=channel, status=status', /INFO
    RETURN
 ENDIF
 orbit = LONG( orbit )
 ichan = -1
 IF N_ELEMENTS( channel ) GT 0 THEN ichan = FIX( channel[0] )
 ipix = -1
 IF N_ELEMENTS( pixelID ) GT 0 THEN ipix = FIX( pixelID[0] )

 IF N_ELEMENTS( SDMF_VERSION ) GT 0 THEN BEGIN
    IF SDMF_VERSION NE '3.1' THEN $
       MESSAGE, 'FATAL: only available for SDMF v3.1 and higher'
 ENDIF

 IF N_ELEMENTS( SDMF_H5_DB ) EQ 0 THEN $
    SDMF_H5_DB = GET_SDMF_PATH(SDMF_VERSION='3.1') + './sdmf_extract_l1b.h5'
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
 gid = H5G_OPEN( fid, procStage )
 IF gid LT 0 THEN MESSAGE, 'Could not open group: ' + procStage
;
; obtain indices to entries of the requested orbit and state
;
 dd = H5D_OPEN( gid, 'orbitList' )
 orbitList = H5D_READ( dd )
 IF N_ELEMENTS( orbit ) EQ 1 THEN BEGIN
    IF orbit GT 0 THEN $
       metaIndx = WHERE( orbitList EQ orbit, numIndx ) $
    ELSE BEGIN
       numIndx = N_ELEMENTS( orbitList )
       metaIndx = LINDGEN( numIndx )
    ENDELSE
 ENDIF ELSE BEGIN
    metaIndx = WHERE( orbitList GE orbit[0] AND orbitList LE orbit[1], numIndx )
 ENDELSE
 IF numIndx EQ 0 THEN BEGIN
    MESSAGE, 'no entry found in database: ' + SDMF_H5_DB, /INFO
    RETURN
 ENDIF
 H5D_CLOSE, dd
;
; read metaTable entries
;
 dd = H5D_OPEN( gid, 'metaTable' )
 IF dd LT 0 THEN MESSAGE, 'Could not open dataset: metaTable'
 IF orbit[0] EQ -1 THEN $
    mtbl = H5D_READ( dd ) $
 ELSE BEGIN
    space_id = H5D_GET_SPACE( dd )
    mem_space_id = H5S_CREATE_SIMPLE( [1] )
    H5S_SELECT_HYPERSLAB, space_id, metaIndx[0], [1], /reset
    temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id  )
    mtbl = REPLICATE( temp, numIndx )
    mtbl[0] = temp
    FOR ni = 1, numIndx-1 DO BEGIN
       H5S_SELECT_HYPERSLAB, space_id, metaIndx[ni], [1], /reset
       mtbl[ni] = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
    ENDFOR
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
 ENDELSE
 H5D_CLOSE, dd
;
 IF ARG_PRESENT( AO ) THEN BEGIN
    IF ipix GT 0 THEN $
       ao = _RD_EXTRACT_L1B_PIXEL( 'ao', gid, numIndx, metaIndx, ipix ) $
    ELSE IF ichan GT 0 THEN $
       ao = _RD_EXTRACT_L1B_CHAN( 'ao', gid, numIndx, metaIndx, ichan ) $
    ELSE $
       ao = _RD_EXTRACT_L1B_ALL( 'ao', gid, numIndx, metaIndx )
 ENDIF
;
 IF ARG_PRESENT( ERROR_AO ) THEN BEGIN
    IF ipix GT 0 THEN BEGIN
       error_ao = $
          _RD_EXTRACT_L1B_PIXEL( 'ao_error', gid, numIndx, metaIndx, ipix )
    ENDIF ELSE IF ichan GT 0 THEN BEGIN
       error_ao = $
          _RD_EXTRACT_L1B_CHAN( 'ao_error', gid, numIndx, metaIndx, ichan )
    ENDIF ELSE BEGIN
       error_ao = $
          _RD_EXTRACT_L1B_ALL( 'ao_error', gid, numIndx, metaIndx )
    ENDELSE
 ENDIF
;
 IF ARG_PRESENT( LC ) THEN BEGIN
    IF ipix GT 0 THEN $
       lc = _RD_EXTRACT_L1B_PIXEL( 'lc', gid, numIndx, metaIndx, ipix ) $
    ELSE IF ichan GT 0 THEN $
       lc = _RD_EXTRACT_L1B_CHAN( 'lc', gid, numIndx, metaIndx, ichan ) $
    ELSE $
       lc = _RD_EXTRACT_L1B_ALL( 'lc', gid, numIndx, metaIndx )
 ENDIF
;
 IF ARG_PRESENT( ERROR_LC ) THEN BEGIN
    IF ipix GT 0 THEN BEGIN
       error_lc = $
          _RD_EXTRACT_L1B_PIXEL( 'lc_error', gid, numIndx, metaIndx, ipix )
    ENDIF ELSE IF ichan GT 0 THEN BEGIN
       error_lc = $
          _RD_EXTRACT_L1B_CHAN( 'lc_error', gid, numIndx, metaIndx, ichan )
    ENDIF ELSE BEGIN
       error_lc = $
          _RD_EXTRACT_L1B_ALL( 'lc_error', gid, numIndx, metaIndx )
    ENDELSE
 ENDIF
;
 IF ARG_PRESENT( VLC ) THEN BEGIN
    IF ipix GT 0 THEN $
       vlc = _RD_EXTRACT_L1B_PIXEL( 'vlc', gid, numIndx, metaIndx, ipix ) $
    ELSE IF ichan GT 0 THEN $
       vlc = _RD_EXTRACT_L1B_CHAN( 'vlc', gid, numIndx, metaIndx, ichan ) $
    ELSE $
       vlc = _RD_EXTRACT_L1B_ALL( 'vlc', gid, numIndx, metaIndx )
 ENDIF
;
 IF ARG_PRESENT( ERROR_VLC ) THEN BEGIN
    IF ipix GT 0 THEN BEGIN
       error_vlc = $
          _RD_EXTRACT_L1B_PIXEL( 'vlc_error', gid, numIndx, metaIndx, ipix )
    ENDIF ELSE IF ichan GT 0 THEN BEGIN
       error_vlc = $
          _RD_EXTRACT_L1B_CHAN( 'vlc_error', gid, numIndx, metaIndx, ichan )
    ENDIF ELSE BEGIN
       error_vlc = $
          _RD_EXTRACT_L1B_ALL( 'vlc_error', gid, numIndx, metaIndx )
    ENDELSE
 ENDIF
;
 IF ARG_PRESENT( BDPM ) THEN BEGIN
    IF ipix GT 0 THEN $
       bdpm = _RD_EXTRACT_L1B_PIXEL( 'bdpm', gid, numIndx, metaIndx, ipix ) $
    ELSE IF ichan GT 0 THEN $
       bdpm = _RD_EXTRACT_L1B_CHAN( 'bdpm', gid, numIndx, metaIndx, ichan ) $
    ELSE $
       bdpm = _RD_EXTRACT_L1B_ALL( 'bdpm', gid, numIndx, metaIndx )
 ENDIF
;
 IF ARG_PRESENT( GRID_BASE ) THEN BEGIN
    IF ipix GT 0 THEN BEGIN
       grid_base = $
          _RD_EXTRACT_L1B_PIXEL( 'grid_base', gid, numIndx, metaIndx, ipix )
    ENDIF ELSE IF ichan GT 0 THEN BEGIN
       grid_base = $
          _RD_EXTRACT_L1B_CHAN( 'grid_base', gid, numIndx, metaIndx, ichan )
    ENDIF ELSE BEGIN
       grid_base = $
          _RD_EXTRACT_L1B_ALL( 'grid_base', gid, numIndx, metaIndx )
    ENDELSE
 ENDIF
;
 IF ARG_PRESENT( GRID_COEFFS ) THEN BEGIN
    grid_coeffs = _RD_EXTRACT_L1B_ALL( 'grid_coeffs', gid, numIndx, metaIndx )
 ENDIF
;
 IF ARG_PRESENT( SUN_DIFFUSER ) THEN BEGIN
    IF ipix GT 0 THEN BEGIN
       sun_diffuser = $
          _RD_EXTRACT_L1B_PIXEL( 'sun_diffuser', gid, numIndx, metaIndx, ipix )
    ENDIF ELSE IF ichan GT 0 THEN BEGIN
       sun_diffuser = $
          _RD_EXTRACT_L1B_CHAN( 'sun_diffuser', gid, numIndx, metaIndx, ichan )
    ENDIF ELSE BEGIN
       sun_diffuser = $
          _RD_EXTRACT_L1B_ALL( 'sun_diffuser', gid, numIndx, metaIndx )
    ENDELSE
 ENDIF
;
; close HDF5 resources
;
 H5G_CLOSE, gid
 H5F_CLOSE, fid
 status = 0
 RETURN
END
