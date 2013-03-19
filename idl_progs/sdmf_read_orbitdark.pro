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
;	SDMF_READ_ORBITDARK
;
; PURPOSE:
;	obtain Orbital Dark parameters (channel 8, only)
;
; CATEGORY:
;	Sciamachy - SDMF
;
; CALLING SEQUENCE:
;	SDMF_READ_ORBITDARK, orbit, orbitPhase, mtbl, analogOffset, darkCurrent
;
; INPUTS:
;	orbit:	        absolute orbit number, a scalar or vector[min,max],
;                       to select all orbits use -1
;       orbitPhase:     orbit phase(s), ESA definition
;                       a scalar or vector[min,max]
;
; KEYWORD PARAMETERS:
;       analogOffsError:   uncertainty in analogOffset
;       darkCurrentError:  uncertainty in darkCurrent
;
;       pixelID:        pixel ID, named scalar, range [1..8192]
;
;       SDMF_H5_DB:     path to the SDMF Dark database
;                         default: /SCIA/SDMF31/sdmf_dark.h5
;       SDMF_VERSION:   SDMF version <2.4|3.0|3.1>, default 3.0
;       status :        returns named variable with error flag (0 = ok)
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
;	- obtain dark signal for a given orbit=7502: 
;       IDL> SDMF_READ_ORBITDARK, 7502, 0.5, mtbl, ao, lc, ao_err=ao_err
;
;	- obtain dark signal for a given orbit=7502, several orbit phases: 
;       IDL> SDMF_READ_ORBITDARK, 7502, [0,0.25, 0.5, 0.75], mtbl, oa, lc
;
;	- obtain dark signal for a given orbit range: 
;       IDL> SDMF_READ_ORBITDARK, [11000,12000], 0.33, mtbl, ao, lc
;
;	- obtain dark signal for a given pixel=100 (all orbits): 
;       IDL> SDMF_READ_ORBITDARK, 63, -1, mtbl, signal, noise, pixel=100
;
; MODIFICATION HISTORY:
; 	Written by:	R.M. van Hees (SRON), 18-06-2012
;-
;------------------------ SDMF Version 2.4 ------------------------
;
; internal function to select files from SDMF v2.4 fileList
;
FUNCTION __GET_24_FILELIST, sdmf_path
 compile_opt idl2,logical_predicate,hidden

 Structure = { DatabaseRecord    , $
               FileName          : bytarr(70), $
               Orbit             : 0L,      $
               MagicNumber       : 0L,      $
               StateCount        : lonarr(16), $
               QualityNumber     : 0L,      $
               QualitySmoothMask : 0L,      $
               Consolidated      : 0L,      $
               Transmission      : 0L,      $
               WLSTransmission   : 0L,      $
               PixelGain         : 0L,      $
               Orbital           : 0L,      $
               OrbitalData       : 0L,      $
               OrbitalFit        : 0L,      $
               SMR               : 0L      $
             }
 MonitorFile = sdmf_path + 'MonitorList.dat'
 RETURN, ReadBinaryStructure(MonitorFile, {DatabaseRecord})
END

;------------------------------
PRO __GET_24_ORBITAL_DARK, sdmfPath, darkFile, orbit, orbitPhase_in, $
   ipix, orbitDark, status=status

 status = 'failed'
 numOrbitDark = 72
 MonitorFile = sdmfPath + 'OrbitalVariation/Transmission/' $
               + darkFile + '.orbital'
 IF (~ FILE_TEST( MonitorFile, /READ, /NOEXPAND )) THEN BEGIN
    MESSAGE, ' Can not read from: ' + MonitorFile, /INFO
    RETURN
 ENDIF
 on_ioerror, GOTO_FAILED
 OPENR, Unit, MonitorFile, /GET_LUN
 Data = REPLICATE( !VALUES.F_NAN, 1024, numOrbitDark )
 READU, Unit, Data
 status = 'success'
 GOTO_FAILED:
 FREE_LUN, UNIT 
 IF status EQ 'failed' THEN BEGIN
    PRINT, !ERROR_STATE.MSG
    RETURN
 ENDIF

 ; correct orbit phase for SDMF v2.4 definition
 orbitPhase = orbitPhase_in - GET_SDMF_PHASEDIFF( orbit )
 indx = WHERE( orbitPhase LT 0., count )
 FOR ii = 0, count-1 DO orbitPhase[indx[ii]] += 1.

 ; extend array for linear interpolation for orbitPhase close to 1.0
 XX = FINDGEN(numOrbitDark+1) / numOrbitDark
 Data = [[Data],[Data[*,0]]]
 IF ipix NE -1 THEN BEGIN
    orbitDark = INTERPOL( Data[ipix,*], XX, orbitPhase, /NAN )
 ENDIF ELSE BEGIN
    orbitDark = FLTARR( N_ELEMENTS(orbitPhase), 1024 )
    FOR np = 0, 1023 DO BEGIN
       orbitDark[*,np] = INTERPOL( Data[np,*], XX, orbitPhase, /NAN )
    ENDFOR
 ENDELSE

 RETURN
END

;------------------------ SDMF Version 3.0 ------------------------
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

PRO __GET_30_ORBITAL_DARK, fid, orbitRange, orbitPhase, ipix, orbitDark, $
   dark_param=dark_param, use_neighbours=use_neighbours, status=status

 compile_opt idl2,logical_predicate,hidden

 IF ~ keyword_set(use_neighbours) THEN use_neighbours = 0

 gid = H5G_OPEN( fid, 'ch8' )
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
       buff = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
       mtbl[ii] = buff
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
 ao = __READ_SIMUSET( dd, dims[0], numIndx, metaIndx, ipix )
 IF numIndx EQ dims[0] THEN ao = ao[indx_sort,*]
 ao = REFORM( ao )
 H5D_CLOSE, dd

 dd = H5D_OPEN( gid, 'lc' )
 IF dd LT 0 THEN MESSAGE, 'Could not open dataset: lc'
 lc = __READ_SIMUSET( dd, dims[0], numIndx, metaIndx, ipix )
 IF numIndx EQ dims[0] THEN lc = lc[indx_sort,*]
 lc = REFORM( lc )
 H5D_CLOSE, dd
;
 dd = H5D_OPEN( gid, 'amp1' )
 IF dd LT 0 THEN MESSAGE, 'Could not open dataset: amp1'
 amp1 = __READ_SIMUSET( dd, dims[0], numIndx, metaIndx, ipix )
 IF numIndx EQ dims[0] THEN amp1 = amp1[indx_sort,*]
 amp1 = REFORM( amp1 )
 H5D_CLOSE, dd
;
 dd = H5D_OPEN( gid, 'sig_amp1' )
 IF dd LT 0 THEN MESSAGE, 'Could not open dataset: sig_amp1'
 sig_amp1 = __READ_SIMUSET( dd, dims[0], numIndx, metaIndx, ipix )
 IF numIndx EQ dims[0] THEN sig_amp1 = sig_amp1[indx_sort,*]
 sig_amp1 = REFORM( sig_amp1 )
 H5D_CLOSE, dd
;
 dd = H5D_OPEN( gid, 'sig_ao' )
 IF dd LT 0 THEN MESSAGE, 'Could not open dataset: sig_ao'
 sig_ao = __READ_SIMUSET( dd, dims[0], numIndx, metaIndx, ipix )
 IF numIndx EQ dims[0] THEN sig_ao = sig_ao[indx_sort,*]
 sig_ao = REFORM( sig_ao )
 H5D_CLOSE, dd
;
 dd = H5D_OPEN( gid, 'sig_lc' )
 IF dd LT 0 THEN MESSAGE, 'Could not open dataset: sig_lc'
 sig_lc = __READ_SIMUSET( dd, dims[0], numIndx, metaIndx, ipix )
 IF numIndx EQ dims[0] THEN sig_lc = sig_lc[indx_sort,*]
 sig_lc = REFORM( sig_lc )
 H5D_CLOSE, dd
;
 H5G_CLOSE, gid

; calculate orbital dark correction for orbitPhase
 IF ipix GT 0 THEN $
    orbitDark = FLTARR( N_ELEMENTS(orbitPhase), numIndx ) $
 ELSE $
    orbitDark = FLTARR( N_ELEMENTS(orbitPhase), numIndx, 1024 )
 FOR ii = 0, numIndx-1 DO BEGIN
    orbvar = cos(2 * !PI * (mtbl[ii].phase1 + orbitPhase)) $
             + mtbl[ii].amp2 * cos(4 * !PI * (mtbl[ii].phase2 + orbitPhase))
    IF ipix GT 0 THEN BEGIN
       orbitDark[*,ii] = orbvar # amp1[ii]
    ENDIF ELSE BEGIN
       FOR ip = 0, 1023 DO $
          orbitDark[*,ii,ip] = orbvar # amp1[ii,ip]
    ENDELSE
 ENDFOR
 orbitDark = REFORM( orbitDark )

; combine dark parameters in structure
 dark_param = CREATE_STRUCT( 'mtbl', mtbl, 'ao', ao, 'lc', lc, $
                             'sig_ao', sig_ao, 'sig_lc', sig_lc, $
                             'amp1', amp1, 'sig_amp1', sig_amp1 )
 RETURN
END

;------------------------ SDMF Version 3.1 ------------------------

;------------------------ MAIN PROGRAM ------------------------
PRO SDMF_READ_ORBITDARK, orbitRange_in, orbitPhase, orbitDark, $
                         dark_param=dark_param, orbitList=orbitList, $
                         pixelID=pixelID, use_neighbours=use_neighbours, $
                         SDMF_H5_DB=SDMF_H5_DB, SDMF_VERSION=SDMF_VERSION, $
                         status=status

 compile_opt idl2,logical_predicate,hidden

 mtbl = -1
 status = -1
 IF N_PARAMS() NE 3 THEN BEGIN
    MESSAGE, ' Usage: SDMF_READ_ORBITDARK, orbit, orbitPhase, orbitDark' $
             + ', dark_param=dark_param, orbitList=orbitList' $
             + ', pixelID=pixelID, use_neighbours=use_neighbours' $
             + ', SDMF_H5_DB=SDMF_H5_DB, SDMF_VERSION=SDMF_VERSION', /INFO
    RETURN
 ENDIF
 orbitRange = LONG( orbitRange_in )
 IF N_ELEMENTS( orbitRange ) GT 1 THEN BEGIN
    IF orbitRange[0] GT orbitRange[1] THEN BEGIN
       MESSAGE, ' invalid orbitRange -- orbitRange[0] > orbitRange[1]', /INFO
       RETURN
    ENDIF
 ENDIF ELSE orbitRange = [orbitRange, orbitRange]
 ipix = -1
 IF N_ELEMENTS( pixelID ) GT 0 THEN ipix = FIX( pixelID[0] )
;
 sdmfPath = GET_SDMF_PATH( SDMF_VERSION=SDMF_VERSION )
;
 orbitList = !NULL
 orbitDark = !NULL
 CASE SDMF_VERSION OF
    '2.4': BEGIN
       monitorList = __GET_24_FILELIST( sdmfPath )
       indx = WHERE( monitorList.Orbital GE 1 $
                     AND (monitorList.Orbit GE (orbitRange[0]-7) $
                          AND monitorList.Orbit LE (orbitRange[1]+7)), count )
       IF count EQ 0 THEN GOTO, Jump_Done
       monitorList = monitorList[indx]
       FOR norb = orbitRange[0], orbitRange[1] DO BEGIN
          indx = WHERE( monitorList.Orbit EQ norb $
                        AND monitorList.Consolidated EQ 1, count )
          IF count EQ 0 THEN BEGIN
             diff = MIN( ABS( monitorList.Orbit - norb ), indx )
             IF diff GT 14 THEN CONTINUE
          ENDIF
          fileList = STRING( MonitorList[indx].FileName )
          fileList = fileList[(REVERSE(SORT( fileList )))[0]]

          __GET_24_ORBITAL_DARK, sdmfPath, fileList, norb, orbitPhase, $
             ipix, orbitDark_buff, status=stat_str
          IF stat_str EQ 'success' THEN BEGIN
             orbitList = [orbitList, MonitorList[indx[0]].Orbit]
             orbitDark = [[orbitDark], [orbitDark_buff]]
          ENDIF
       ENDFOR
    END
    '3.0': BEGIN
       IF N_ELEMENTS( SDMF_H5_DB ) EQ 0 THEN $
          SDMF_H5_DB = sdmfPath + 'sdmf_simudark.h5'
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
       __GET_30_ORBITAL_DARK, fid, orbitRange, orbitPhase, ipix, orbitDark, $
          dark_param=dark_param, use_neighbours=use_neighbours
       H5F_CLOSE, fid
    END
    '3.1': BEGIN
       PRINT, 'Not yet implemented'
    END
    ELSE: BEGIN
       MESSAGE, 'Not available for SDMF version ' + SDMF_VERSION
    END
 ENDCASE

Jump_Done:
 status = 0
 RETURN
END
