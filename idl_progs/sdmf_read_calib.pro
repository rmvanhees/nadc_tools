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
;	SDMF_READ_CALIB
;
; PURPOSE:
;	obtain calibration state parameters
;
; CATEGORY:
;	Sciamachy - SDMF
;
; CALLING SEQUENCE:
;	SDMF_READ_CALIB, stateID, orbitRange, mtbl
;
; INPUTS:
;       stateID:          calibration State ID, a scalar, valid ID's are: 
;                         8,16,26,39,46,48,52,59,61,62,63,65,67,69,70
;	orbitRange:       absolute orbit number, a scalar or vector[min,max]
;       
; KEYWORD PARAMETERS:
;       use_neighbours:    flag to enable looking for neighboring valid entries
;
;       coaddf:           
;       pet:              
;       count:            
;       avg:              
;       noise:            
;       histReadOut:      
;       channelNoiseMean: SDMF(v3.1)
;       channelNoiseAdev: SDMF(v3.1)
;       channelTempAdev:  SDMF(v3.1)
;       SDMF_H5_DB:       path to the SDMF calibration states database
;                           default: /SCIA/SDMF/3.0/sdmf_extract_calib.h5
;       SDMF_VERSION:     SDMF version <2.4.1|3.0|3.1>, default 3.0 
;       status :          returns named variable with error flag (0 = ok)
;
; OUTPUTS:
;       mtbl:             structure with rows of table metaTable
;
; PROCEDURE:
;	none
;
; EXAMPLE:
;       Please add!
;
; MODIFICATION HISTORY:
; 	Written by:	R.M. van Hees (SRON), 21-01-2011
;       Modified by:    Richard van Hees (SRON), November 2011
;                       * added check on SDMF_DB_VERSION
;       Modified by:    Richard van Hees (SRON), March 2012
;                       * storage of histReadOut in HDF5 file changed
;       Modified by:    RvH, 28 March 2012
;                       * coding according new standard
;-
;---------------------------------------------------------------------------
PRO __GET_30_CALIB_DATA, fid, stateID, orbitRange, mtbl, $
   coaddf=coaddf, pet=pet, count=count, avg=avg, noise=noise, $
   histReadOut=histReadOut, use_neighbours=use_neighbours
 compile_opt idl2,logical_predicate,hidden
;
 IF ~ keyword_set(use_neighbours) THEN use_neighbours = 0
;
 grpName = 'State_' + STRING(stateID, format='(I02)')
 gid = H5G_OPEN( fid, grpName )
 IF gid LT 0 THEN MESSAGE, 'Could not open group: ' + grpName
;
; obtain indices to entries of the requested orbit and state
;
 dd = H5D_OPEN( gid, 'orbitList' )
 orbitList = H5D_READ( dd )
 H5D_CLOSE, dd
 IF N_ELEMENTS( orbitRange ) EQ 1 THEN BEGIN
    metaIndx = WHERE( orbitList EQ orbitRange, numIndx )
    IF numIndx EQ 0 AND use_neighbours NE 0 THEN BEGIN
       diff = MIN( ABS( orbitList - orbitRange ), indx )
       metaIndx = WHERE( orbitList EQ orbitList[indx], numIndx )
    ENDIF
 ENDIF ELSE BEGIN
    metaIndx = WHERE( orbitList GE orbitRange[0] $
                      AND orbitList LE orbitRange[1], numIndx )
 ENDELSE
 IF numIndx EQ 0 THEN BEGIN
    MESSAGE, 'no entry found in database: ' + SDMF_H5_DB, /INFO
    RETURN
 ENDIF
;
; read metaTable entries
;
 dd = H5D_OPEN( gid, 'metaTable' )
 IF dd LT 0 THEN MESSAGE, 'Could not open dataset: metaTable'
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
 H5D_CLOSE, dd
;
 IF ARG_PRESENT( coaddf ) THEN BEGIN
    dd = H5D_OPEN( gid, 'readoutCoaddf' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: readoutCoaddf'
    space_id = H5D_GET_SPACE( dd )
    mem_space_id = H5S_CREATE_SIMPLE( [1,8192] )
    H5S_SELECT_HYPERSLAB, space_id, [metaIndx[0],0], [1,8192], /reset
    temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id  )
    coaddf = REPLICATE( temp[0], 8192, numIndx )
    coaddf[*,0] = temp
    FOR ni = 1, numIndx-1 DO BEGIN
       H5S_SELECT_HYPERSLAB, space_id, [metaIndx[ni],0], [1,8192], /reset
       temp = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
       coaddf[*,ni] = temp
    ENDFOR
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
    H5D_CLOSE, dd
 ENDIF
;
 IF ARG_PRESENT( pet ) THEN BEGIN
    dd = H5D_OPEN( gid, 'readoutPet' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: readoutPet'
    space_id = H5D_GET_SPACE( dd )
    mem_space_id = H5S_CREATE_SIMPLE( [1,8192] )
    H5S_SELECT_HYPERSLAB, space_id, [metaIndx[0],0], [1,8192], /reset
    temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id  )
    pet = REPLICATE( temp[0], 8192, numIndx )
    pet[*,0] = temp
    FOR ni = 1, numIndx-1 DO BEGIN
       H5S_SELECT_HYPERSLAB, space_id, [metaIndx[ni],0], [1,8192], /reset
       temp = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
       pet[*,ni] = temp
    ENDFOR
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
    H5D_CLOSE, dd
 ENDIF
;
 IF ARG_PRESENT( count ) THEN BEGIN
    dd = H5D_OPEN( gid, 'readoutCount' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: readoutCount'
    space_id = H5D_GET_SPACE( dd )
    mem_space_id = H5S_CREATE_SIMPLE( [1,8192] )
    H5S_SELECT_HYPERSLAB, space_id, [metaIndx[0],0], [1,8192], /reset
    temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id  )
    count = REPLICATE( temp[0], 8192, numIndx )
    count[*,0] = temp
    FOR ni = 1, numIndx-1 DO BEGIN
       H5S_SELECT_HYPERSLAB, space_id, [metaIndx[ni],0], [1,8192], /reset
       temp = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
       count[*,ni] = temp
    ENDFOR
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
    H5D_CLOSE, dd
 ENDIF
;
 IF ARG_PRESENT( avg ) THEN BEGIN
    dd = H5D_OPEN( gid, 'readoutMean' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: readoutMean'
    space_id = H5D_GET_SPACE( dd )
    IF ipix GT 0 THEN BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [1,1] )
       H5S_SELECT_HYPERSLAB, space_id, [metaIndx[0],ipix-1], [1,1], /reset
       temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id  )
       avg = REPLICATE( temp[0], numIndx )
       avg[0] = temp
       FOR ni = 1, numIndx-1 DO BEGIN
          H5S_SELECT_HYPERSLAB, space_id, [metaIndx[ni],ipix-1], [1,1], /reset
          temp = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
          avg[ni] = temp
       ENDFOR
    ENDIF ELSE IF nchan GT 0 THEN BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [1,1024] )
       H5S_SELECT_HYPERSLAB, space_id, [metaIndx[0],(nchan-1)*1024], $
                             [1,1024], /reset
       temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id  )
       avg = REPLICATE( temp[0], 1024, numIndx )
       avg[*,0] = temp
       FOR ni = 1, numIndx-1 DO BEGIN
          H5S_SELECT_HYPERSLAB, space_id, [metaIndx[ni],(nchan-1)*1024], $
                                [1,1024], /reset
          temp = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
          avg[*,ni] = temp
       ENDFOR
    ENDIF ELSE BEGIN
       mem_space_id = H5S_CREATE_SIMPLE( [1,8192] )
       H5S_SELECT_HYPERSLAB, space_id, [metaIndx[0],0], [1,8192], /reset
       temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id  )
       avg = REPLICATE( temp[0], 8192, numIndx )
       avg[*,0] = temp
       FOR ni = 1, numIndx-1 DO BEGIN
          H5S_SELECT_HYPERSLAB, space_id, [metaIndx[ni],0], [1,8192], /reset
          temp = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
          avg[*,ni] = temp
       ENDFOR
    ENDELSE
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
    H5D_CLOSE, dd
 ENDIF
;
 IF ARG_PRESENT( noise ) THEN BEGIN
    dd = H5D_OPEN( gid, 'readoutNoise' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: readoutNoise'
    space_id = H5D_GET_SPACE( dd )
    mem_space_id = H5S_CREATE_SIMPLE( [1,8192] )
    H5S_SELECT_HYPERSLAB, space_id, [metaIndx[0],0], [1,8192], /reset
    temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id  )
    noise = REPLICATE( temp[0], 8192, numIndx )
    noise[*,0] = temp
    FOR ni = 1, numIndx-1 DO BEGIN
       H5S_SELECT_HYPERSLAB, space_id, [metaIndx[ni],0], [1,8192], /reset
       temp = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
       noise[*,ni] = temp
    ENDFOR
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
    H5D_CLOSE, dd
 ENDIF
;
 IF ARG_PRESENT( histReadOut ) THEN BEGIN
    dd = H5D_OPEN( gid, 'histReadOut' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: histReadOut'
    space_id = H5D_GET_SPACE( dd )
    mem_space_id = H5S_CREATE_SIMPLE( [8192] )
    H5S_SELECT_HYPERSLAB, space_id, [metaIndx[0] * 8192], [8192], /reset
    temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id  )
    histReadOut = REPLICATE( temp[0], 8192, numIndx )
    histReadOut[*,0] = temp
    FOR ni = 1, numIndx-1 DO BEGIN
       H5S_SELECT_HYPERSLAB, space_id, [metaIndx[ni] * 8192], [8192], /reset
       temp = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
       histReadOut[*,ni] = temp
    ENDFOR
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
    H5D_CLOSE, dd
 ENDIF
 H5G_CLOSE, gid
 RETURN
END

;---------------------------------------------------------------------------
PRO __GET_31_CALIB_DATA, fid, stateID, orbitRange, mtbl, $
   coaddf=coaddf, pet=pet, count=count, avg=avg, noise=noise, $
   histReadOut=histReadOut, channelNoiseMean=channelNoiseMean, $
   channelNoiseAdev=channelNoiseAdev, channelTempAdev=channelTempAdev, $
   use_neighbours=use_neighbours
 compile_opt idl2,logical_predicate,hidden
;
 IF ~ keyword_set(use_neighbours) THEN use_neighbours = 0
;
 IF ARG_PRESENT( coaddf ) OR ARG_PRESENT( pet ) THEN $
    clusDef = GET_SCIA_CLUSDEF( stateID )
;
 grpName = 'State_' + STRING(stateID, format='(I02)')
 gid = H5G_OPEN( fid, grpName )
 IF gid LT 0 THEN MESSAGE, 'Could not open group: ' + grpName
;
; obtain indices to entries of the requested orbit and state
;
 dd = H5D_OPEN( gid, 'orbitList' )
 orbitList = H5D_READ( dd )
 H5D_CLOSE, dd
 IF N_ELEMENTS( orbitRange ) EQ 1 THEN BEGIN
    metaIndx = WHERE( orbitList EQ orbitRange, numIndx )
    IF numIndx EQ 0 AND use_neighbours NE 0 THEN BEGIN
       diff = MIN( ABS( orbitList - orbitRange ), indx )
       metaIndx = WHERE( orbitList EQ orbitList[indx], numIndx )
    ENDIF
 ENDIF ELSE BEGIN
    metaIndx = WHERE( orbitList GE orbitRange[0] $
                      AND orbitList LE orbitRange[1], numIndx )
 ENDELSE
 IF numIndx EQ 0 THEN BEGIN
    mtbl = !NULL
    MESSAGE, 'no entry found in database ', /INFO
    RETURN
 ENDIF

 dd = H5D_OPEN( gid, 'metaTable' )
 IF dd LT 0 THEN MESSAGE, 'Could not open dataset: metaTable'
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
 H5D_CLOSE, dd
;
 IF ARG_PRESENT( coaddf ) THEN BEGIN
    dd = H5D_OPEN( gid, 'clusConf' )
    clusConf = H5D_READ( dd )
    H5D_CLOSE, dd
    coaddf = REPLICATE( 0us, 8192, numIndx )
    FOR ni = 0, numIndx-1 DO BEGIN
       indx = WHERE( mtbl[ni].absOrbit GE clusConf.orbit, count )
       nj = indx[count-1]
       FOR nc = 0, 39 DO BEGIN
          xx = clusDef[nc].start
          yy = clusDef[nc].start + clusDef[nc].length - 1
          coaddf[xx:yy, ni] = clusConf[nj].coaddf[nc]
       ENDFOR
    ENDFOR
 ENDIF
;
 IF ARG_PRESENT( pet ) THEN BEGIN
    dd = H5D_OPEN( gid, 'clusConf' )
    clusConf = H5D_READ( dd )
    H5D_CLOSE, dd
    pet = REPLICATE( 1., 8192, numIndx )
    FOR ni = 0, numIndx-1 DO BEGIN
       indx = WHERE( mtbl[ni].absOrbit GE clusConf.orbit, count )
       nj = indx[count-1]
       FOR nc = 0, 39 DO BEGIN
          xx = clusDef[nc].start
          yy = clusDef[nc].start + clusDef[nc].length - 1
          pet[xx:yy, ni] = clusConf[nj].pet[nc]
       ENDFOR
    ENDFOR
 ENDIF
;
 IF ARG_PRESENT( count ) THEN BEGIN
    dd = H5D_OPEN( gid, 'readoutCount' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: readoutCount'
    space_id = H5D_GET_SPACE( dd )
    mem_space_id = H5S_CREATE_SIMPLE( [8192,1] )
    H5S_SELECT_HYPERSLAB, space_id, [0,metaIndx[0]], [8192,1], /reset
    temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id  )
    count = REPLICATE( temp[0], 8192, numIndx )
    count[*,0] = temp
    FOR ni = 1, numIndx-1 DO BEGIN
       H5S_SELECT_HYPERSLAB, space_id, [0,metaIndx[ni]], [8192,1], /reset
       temp = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
       count[*,ni] = temp
    ENDFOR
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
    H5D_CLOSE, dd
 ENDIF
;
 IF ARG_PRESENT( avg ) THEN BEGIN
    dd = H5D_OPEN( gid, 'readoutMean' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: readoutMean'
    space_id = H5D_GET_SPACE( dd )
    mem_space_id = H5S_CREATE_SIMPLE( [8192,1] )
    H5S_SELECT_HYPERSLAB, space_id, [0,metaIndx[0]], [8192,1], /reset
    temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id  )
    avg = REPLICATE( temp[0], 8192, numIndx )
    avg[*,0] = temp
    FOR ni = 1, numIndx-1 DO BEGIN
       H5S_SELECT_HYPERSLAB, space_id, [0,metaIndx[ni]], [8192,1], /reset
       temp = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
       avg[*,ni] = temp
    ENDFOR
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
    H5D_CLOSE, dd
 ENDIF
;
 IF ARG_PRESENT( noise ) THEN BEGIN
    dd = H5D_OPEN( gid, 'readoutNoise' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: readoutNoise'
    space_id = H5D_GET_SPACE( dd )
    mem_space_id = H5S_CREATE_SIMPLE( [8192,1] )
    H5S_SELECT_HYPERSLAB, space_id, [0,metaIndx[0]], [8192,1], /reset
    temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id  )
    noise = REPLICATE( temp[0], 8192, numIndx )
    noise[*,0] = temp
    FOR ni = 1, numIndx-1 DO BEGIN
       H5S_SELECT_HYPERSLAB, space_id, [0,metaIndx[ni]], [8192,1], /reset
       temp = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
       noise[*,ni] = temp
    ENDFOR
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
    H5D_CLOSE, dd
 ENDIF
;
 IF ARG_PRESENT( histReadOut ) THEN BEGIN
    dd = H5D_OPEN( gid, 'histReadOut' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: histReadOut'
    space_id = H5D_GET_SPACE( dd )
    mem_space_id = H5S_CREATE_SIMPLE( [8192,1] )
    H5S_SELECT_HYPERSLAB, space_id, [0,metaIndx[0]], [8192,1], /reset
    temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id  )
    histReadOut = REPLICATE( temp[0], 8192, numIndx )
    histReadOut[*,0] = temp
    FOR ni = 1, numIndx-1 DO BEGIN
       H5S_SELECT_HYPERSLAB, space_id, [0,metaIndx[ni]], [8192,1], /reset
       temp = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
       histReadOut[*,ni] = temp
    ENDFOR
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
    H5D_CLOSE, dd
 ENDIF
;
 IF ARG_PRESENT( channelNoiseMean ) THEN BEGIN
    dd = H5D_OPEN( gid, 'channelNoise' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: channelNoise'
    space_id = H5D_GET_SPACE( dd )
    mem_space_id = H5S_CREATE_SIMPLE( [8,1] )
    H5S_SELECT_HYPERSLAB, space_id, [0,metaIndx[0]], [8,1], /reset
    temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id  )
    channelNoiseMean = REPLICATE( temp[0], 8, numIndx )
    channelNoiseMean[*,0] = temp
    FOR ni = 1, numIndx-1 DO BEGIN
       H5S_SELECT_HYPERSLAB, space_id, [0,metaIndx[ni]], [8,1], /reset
       temp = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
       channelNoiseMean[*,ni] = temp
    ENDFOR
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
    H5D_CLOSE, dd
 ENDIF
;
 IF ARG_PRESENT( channelNoiseAdev ) THEN BEGIN
    dd = H5D_OPEN( gid, 'channelNoiseAdev' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: channelNoiseAdev'
    space_id = H5D_GET_SPACE( dd )
    mem_space_id = H5S_CREATE_SIMPLE( [8,1] )
    H5S_SELECT_HYPERSLAB, space_id, [0,metaIndx[0]], [8,1], /reset
    temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id  )
    channelNoiseAdev = REPLICATE( temp[0], 8, numIndx )
    channelNoiseAdev[*,0] = temp
    FOR ni = 1, numIndx-1 DO BEGIN
       H5S_SELECT_HYPERSLAB, space_id, [0,metaIndx[ni]], [8,1], /reset
       temp = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
       channelNoiseAdev[*,ni] = temp
    ENDFOR
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
    H5D_CLOSE, dd
 ENDIF
;
 IF ARG_PRESENT( channelTempAdev ) THEN BEGIN
    dd = H5D_OPEN( gid, 'channelTempAdev' )
    IF dd LT 0 THEN MESSAGE, 'Could not open dataset: channelTempAdev'
    space_id = H5D_GET_SPACE( dd )
    mem_space_id = H5S_CREATE_SIMPLE( [8,1] )
    H5S_SELECT_HYPERSLAB, space_id, [0,metaIndx[0]], [8,1], /reset
    temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id  )
    channelTempAdev = REPLICATE( temp[0], 8, numIndx )
    channelTempAdev[*,0] = temp
    FOR ni = 1, numIndx-1 DO BEGIN
       H5S_SELECT_HYPERSLAB, space_id, [0,metaIndx[ni]], [8,1], /reset
       temp = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
       channelTempAdev[*,ni] = temp
    ENDFOR
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
    H5D_CLOSE, dd
 ENDIF
 H5G_CLOSE, gid
 RETURN
END

;---------------------------------------------------------------------------
PRO SDMF_READ_CALIB, stateID_in, orbitRange_in, mtbl, $
                     _REF_EXTRA=EXTRA, $
                     SDMF_H5_DB=SDMF_H5_DB, SDMF_VERSION=SDMF_VERSION, $
                     status=status
 compile_opt idl2,logical_predicate,hidden
 
 mtbl = -1
 status = -1
 IF N_PARAMS() NE 3 THEN BEGIN
    MESSAGE, ' Usage: SDMF_READ_CALIB, stateID, orbitRange, mtbl' $
             + ', coaddf=coaddf, pet=pet, count=count, avg=avg' $
             + ', noise=noise, histReadOut=histReadOut' $
             + ', use_neighbours=use_neighbours' $
             + ', channelNoiseMean=channelNoiseMean' $
             + ', channelNoiseAdev=channelNoiseAdev' $
             + ', channelTempAdev=channelTempAdev' $
             + ', SDMF_H5_DB=SDMF_H5_DB, SDMF_VERSION=SDMF_VERSION', /INFO
    RETURN
 ENDIF
 orbitRange = LONG( orbitRange_in )
 stateID = FIX( stateID_in )
 
IF N_ELEMENTS( SDMF_VERSION ) GT 0 THEN BEGIN
    IF SDMF_VERSION EQ '2.4' THEN $
       MESSAGE, 'FATAL: not implemented for SDMF v2.4.x'
 ENDIF

 IF N_ELEMENTS( SDMF_H5_DB ) EQ 0 THEN $
    SDMF_H5_DB = GET_SDMF_PATH( SDMF_VERSION=SDMF_VERSION ) $
                 + 'sdmf_extract_calib.h5'
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
       __GET_30_CALIB_DATA, fid, stateID, orbitRange, mtbl, _STRICT_EXTRA=EXTRA
    END
    '3.1': BEGIN
       __GET_31_CALIB_DATA, fid, stateID, orbitRange, mtbl, _STRICT_EXTRA=EXTRA
    END
 ENDCASE
;
; close HDF5 resources
;
 H5F_CLOSE, fid
 status = 0
 RETURN
END
