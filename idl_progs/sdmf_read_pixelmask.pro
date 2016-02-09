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
;	SDMF_READ_PIXELMASK
;
; PURPOSE:
;	obtain bad-dead pixel mask
;
; CATEGORY:
;	Sciamachy - SDMF
;
; CALLING SEQUENCE:
;	SDMF_READ_PIXELMASK, orbit, mtbl, bdpm
;
; INPUTS:
;	orbit:	        absolute orbit number, a scalar or vector[min,max], 
;                       to select all orbits use -1
;
; KEYWORD PARAMETERS:
;       use_neighbours:    flag to enable looking for neighboring valid entries
;       orbital:           obtain orbital instead of smooth mask 
;
;       mask_oa:           mask based on analogOffset
;       mask_oa_err:	   mask based on analogOffsetError
;       mask_chisq:	   mask based on chiSquare
;       mask_lc:	   mask based on darkCurrent
;       mask_lc_err:	   mask based on darkCurrentError
;       mask_invalid:	   mask based on invalid
;       mask_noise:	   mask based on noise
;       mask_ppg:	   mask based on pixelGain
;       mask_residu:	   mask based on residual
;       mask_sun:	   mask based on sunResponse
;       mask_wls:	   mask based on wlsResponse
;
;       channel:           channel ID, named scalar, range [1..8]
;       pixelID:           pixel ID, named scalar, range [1...8192]
;
;       SDMF_H5_DB:        path to the SDMF Dark database
;                            default: /SCIA/SDMF/3.0/sdmf_pixelmask.h5
;       SDMF_VERSION:      SDMF version <2.4|3.0>, default 3.0
;       status :           returns named variable with error flag (0 = ok)
;
; OUTPUTS:
;       mtbl:              metaTable (array of structures)
;       bdpm:              bad-dead pixel mask
;
; PROCEDURE:
;	none
;
; EXAMPLE:
;	- obtain bad-dead pixel mask for orbit=7502, all channels
;       IDL> SDMF_READ_PIXELMASK, 7502, mtbl, bdpm
;
;	- obtain bad-dead pixel mask for orbit=7502, channel=2:
;       IDL> SDMF_READ_PIXELMASK, 7502, mtbl, bdpm, channel=2
;
;	- obtain bad-dead pixel mask for pixel=100 (all orbits):
;       IDL> SDMF_READ_PIXELMASK, -1, mtbl, bdpm, pixel=100
;
;	- obtain (orbital) bad-dead pixel mask for orbit range:
;       IDL> SDMF_READ_PIXELMASK, [11000,12000], mtbl, bdpm, /orbital
;
; MODIFICATION HISTORY:
; 	Written by:	R.M. van Hees (SRON), 29 March 2012
;-
;---------------------------------------------------------------------------
FUNCTION __READ_MASKSET, dd, max_records, numIndx, metaIndx, nchan, ipix
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
PRO __GET_30_PIXELMASK, gid, orbitRange, mtbl, bdpm, $
   mask_oa=mask_oa, mask_oa_err=mask_oa_err, mask_chisq=mask_chisq, $
   mask_lc=mask_lc, mask_lc_err=mask_lc_err, mask_invalid=mask_invalid, $
   mask_noise=mask_noise, mask_ppg=mask_ppg, mask_residu=mask_residu, $
   mask_sun=mask_sun, mask_wls=mask_wls, channel=channel, $
   pixelID=pixelID, use_neighbours=use_neighbours
 compile_opt idl2,logical_predicate,hidden
;
 nchan = -1
 IF N_ELEMENTS( channel ) GT 0 THEN nchan = FIX( channel[0] )
 ipix = -1
 IF N_ELEMENTS( pixelID ) GT 0 THEN ipix = FIX( pixelID[0] )
 IF ~ keyword_set(use_neighbours) THEN use_neighbours = 0
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
 dd = H5D_OPEN( gid, 'combined' )
 IF dd LT 0 THEN MESSAGE, 'Could not open dataset: combined'
 bdpm = __READ_MASKSET( dd, dims[0], numIndx, metaIndx, nchan, ipix )
 IF numIndx EQ dims[0] THEN bdpm = bdpm[indx_sort,*]
 bdpm = REFORM( bdpm )
 H5D_CLOSE, dd
;
 IF ARG_PRESENT( mask_oa ) THEN BEGIN
    dd = H5D_OPEN( gid, 'analogOffset' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: analogOffset'
    mask_oa = __READ_MASKSET( dd, dims[0], numIndx, metaIndx, nchan, ipix )
    IF numIndx EQ dims[0] THEN mask_oa = mask_oa[indx_sort,*]
    mask_oa = REFORM( mask_oa )
    H5D_CLOSE, dd
 ENDIF
;
 IF ARG_PRESENT( mask_oa_err ) THEN BEGIN
    dd = H5D_OPEN( gid, 'analogOffsetError' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: analogOffsetError'
    mask_oa_err = __READ_MASKSET( dd, dims[0], numIndx, metaIndx, nchan, ipix )
    IF numIndx EQ dims[0] THEN mask_oa_err = mask_oa_err[indx_sort,*]
    mask_oa_err = REFORM( mask_oa_err )
    H5D_CLOSE, dd
 ENDIF
;
 IF ARG_PRESENT( mask_chisq ) THEN BEGIN
    dd = H5D_OPEN( gid, 'chiSquare' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: chiSquare'
    mask_chisq = __READ_MASKSET( dd, dims[0], numIndx, metaIndx, nchan, ipix )
    IF numIndx EQ dims[0] THEN mask_chisq = mask_chisq[indx_sort,*]
    mask_chisq = REFORM( mask_chisq )
    H5D_CLOSE, dd
 ENDIF
;
 IF ARG_PRESENT( mask_lc ) THEN BEGIN
    dd = H5D_OPEN( gid, 'darkCurrent' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: darkCurrent'
    mask_lc = __READ_MASKSET( dd, dims[0], numIndx, metaIndx, nchan, ipix )
    IF numIndx EQ dims[0] THEN mask_lc = mask_lc[indx_sort,*]
    mask_lc = REFORM( mask_lc )
    H5D_CLOSE, dd
 ENDIF
;
 IF ARG_PRESENT( mask_lc_err ) THEN BEGIN
    dd = H5D_OPEN( gid, 'darkCurrentError' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: darkCurrentError'
    mask_lc_err = __READ_MASKSET( dd, dims[0], numIndx, metaIndx, nchan, ipix )
    IF numIndx EQ dims[0] THEN mask_lc_err = mask_lc_err[indx_sort,*]
    mask_lc_err = REFORM( mask_lc_err )
    H5D_CLOSE, dd
 ENDIF
;
 IF ARG_PRESENT( mask_invalid ) THEN BEGIN
    dd = H5D_OPEN( gid, 'invalid' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: invalid'
    mask_invalid = __READ_MASKSET( dd, dims[0], numIndx, metaIndx, nchan, ipix )
    IF numIndx EQ dims[0] THEN mask_invalid = mask_invalid[indx_sort,*]
    mask_invalid = REFORM( mask_invalid )
    H5D_CLOSE, dd
 ENDIF
;
 IF ARG_PRESENT( mask_noise ) THEN BEGIN
    dd = H5D_OPEN( gid, 'noise' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: noise'
    mask_noise = __READ_MASKSET( dd, dims[0], numIndx, metaIndx, nchan, ipix )
    IF numIndx EQ dims[0] THEN mask_noise = mask_noise[indx_sort,*]
    mask_noise = REFORM( mask_noise )
    H5D_CLOSE, dd
 ENDIF
;
 IF ARG_PRESENT( mask_ppg ) THEN BEGIN
    dd = H5D_OPEN( gid, 'pixelGain' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: pixelGain'
    mask_ppg = __READ_MASKSET( dd, dims[0], numIndx, metaIndx, nchan, ipix )
    IF numIndx EQ dims[0] THEN mask_ppg = mask_ppg[indx_sort,*]
    mask_ppg = REFORM( mask_ppg )
    H5D_CLOSE, dd
 ENDIF
;
 IF ARG_PRESENT( mask_residu ) THEN BEGIN
    dd = H5D_OPEN( gid, 'residual' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: residual'
    mask_residu = __READ_MASKSET( dd, dims[0], numIndx, metaIndx, nchan, ipix )
    IF numIndx EQ dims[0] THEN mask_residu = mask_residu[indx_sort,*]
    mask_residu = REFORM( mask_residu )
    H5D_CLOSE, dd
 ENDIF
;
 IF ARG_PRESENT( mask_sun ) THEN BEGIN
    dd = H5D_OPEN( gid, 'sunResponse' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: sunResponse'
    mask_sun = __READ_MASKSET( dd, dims[0], numIndx, metaIndx, nchan, ipix )
    IF numIndx EQ dims[0] THEN mask_sun = mask_sun[indx_sort,*]
    mask_sun = REFORM( mask_sun )
    H5D_CLOSE, dd
 ENDIF
;
 IF ARG_PRESENT( mask_wls ) THEN BEGIN
    dd = H5D_OPEN( gid, 'wlsResponse' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: wlsResponse'
    mask_wls = __READ_MASKSET( dd, dims[0], numIndx, metaIndx, nchan, ipix )
    IF numIndx EQ dims[0] THEN mask_wls = mask_wls[indx_sort,*]
    mask_wls = REFORM( mask_wls )
    H5D_CLOSE, dd
 ENDIF
 RETURN
END

;---------------------------------------------------------------------------
PRO SDMF_READ_PIXELMASK, orbitRange_in, mtbl, bdpm, orbital=orbital, $
                          SDMF_H5_DB=SDMF_H5_DB, SDMF_VERSION=SDMF_VERSION, $
                          _REF_EXTRA=EXTRA, status=status
 compile_opt idl2,logical_predicate,hidden

 mtbl = -1
 status = -1
 IF N_PARAMS() NE 3 THEN BEGIN
    MESSAGE, ' Usage: SDMF_READ_PIXELMASK, orbit, mtbl, bdpm' $
             + ', mask_oa=mask_oa, mask_oa_err=mask_oa_err' $
             + ', mask_chisq=mask_chisq, mask_lc=mask_lc' $
             + ', mask_lc_err=mask_lc_err, mask_invalid=mask_invalid' $
             + ', mask_noise=mask_noise, mask_ppg=mask_ppg' $
             + ', mask_residu=mask_residu, mask_sun=mask_sun' $
             + ', mask_wls=mask_wls, channel=channel, pixelID=pixelID' $
             + ', SDMF_H5_DB=SDMF_H5_DB, SDMF_VERSION=SDMF_VERSION' $
             + ', /orbital, /use_neighbours, status=status', /INFO
    RETURN
 ENDIF
 orbitRange = LONG( orbitRange_in )
 IF ~ keyword_set(orbital) THEN orbital = 0

 IF N_ELEMENTS( SDMF_VERSION ) GT 0 THEN BEGIN
    IF SDMF_VERSION EQ '2.4' THEN $
       MESSAGE, 'FATAL: not implemented for SDMF v2.4.x'
 ENDIF

 IF N_ELEMENTS( SDMF_H5_DB ) EQ 0 THEN $
    SDMF_H5_DB = GET_SDMF_PATH( SDMF_VERSION=SDMF_VERSION ) $
                 + 'sdmf_pixelmask.h5'
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
 IF orbital EQ 0 THEN BEGIN
    gid = H5G_OPEN( fid, 'smoothMask' )
    IF gid LT 0 THEN MESSAGE, 'Could not open group: smoothMask'
 ENDIF ELSE BEGIN
    gid = H5G_OPEN( fid, 'orbitalMask' )
    IF gid LT 0 THEN MESSAGE, 'Could not open group: orbitalMask'
 ENDELSE

 CASE SDMF_VERSION OF
    '3.0': BEGIN
       __GET_30_PIXELMASK, gid, orbitRange, mtbl, bdpm, $
          _STRICT_EXTRA=EXTRA
    END
    ELSE: BEGIN
       MESSAGE, 'Not available for SDMF version ' + SDMF_VERSION
    END
 ENDCASE

 H5G_CLOSE, gid
 H5F_CLOSE, fid
 status = 0
 RETURN
END
