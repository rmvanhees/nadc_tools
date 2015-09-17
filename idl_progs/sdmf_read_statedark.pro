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
;	SDMF_READ_STATEDARK
;
; PURPOSE:
;	obtain fitted/smoothed State Dark parameters. Only available
;	for SDMF v3.1
;
; CATEGORY:
;	Sciamachy - SDMF
;
; CALLING SEQUENCE:
;	SDMF_READ_STATEDARK, stateID, orbit, mtbl, signal, noise
;
; INPUTS:
;       stateID:        state ID [8, 26, 46, 63 or 67], a scalar 
;	orbit:	        absolute orbit number, a scalar or vector[min,max],
;                       to select all orbits use -1
;
; KEYWORD PARAMETERS:
;       channel:        channel ID, named scalar, range [1..8]
;       pixelID:        pixel ID, named scalar, range [1..8192]
;
;       pet:            returns average state dark data measured
;                       with PET=pet
;                       * Pixel Exposure Time must be multiple of
;                       0.03125 sec
;                       * requires channel of pixelID to be set
;                       * ignores parameter stateID
;
;       SDMF_H5_DB:     path to the SDMF Dark database
;                         default: /SCIA/SDMF/3.1/sdmf_dark.h5
;       SDMF_VERSION:   SDMF version <2.4|3.0|3.1>, default 3.1
;       status :        returns named variable with error flag (0 = ok)
;
; OUTPUTS:
;       mtbl:           metaTable (array of structures)
;       signal:         smooth dark signal (BU)
;	noise:          standard deviation of the smoothed dark signal
;
; PROCEDURE:
;	none
;
; EXAMPLE:
;	- obtain dark signal for a given orbit=7502, all channels: 
;       IDL> SDMF_READ_STATEDARK, 63, 7502, mtbl, signal, noise
;
;	- obtain dark signal for a given orbit=7502, channel=2: 
;       IDL> SDMF_READ_STATEDARK, 63, 7502, mtbl, signal, noise, channel=2
;
;	- obtain dark signal of channel 1 for a given orbit range: 
;       IDL> SDMF_READ_STATEDARK, 63, [11000,12000], mtbl, signal, noise, channel=1
;
;	- obtain dark signal for a given pixel=100 (all orbits): 
;       IDL> SDMF_READ_STATEDARK, 63, -1, mtbl, signal, noise, pixel=100
;
;       - obtain dark signal for a given pixel=100 for selected orbit range: 
;       IDL> SDMF_READ_STATEDARK, 63, [11000,12000], mtbl, signal, noise, pixel=100
;
; MODIFICATION HISTORY:
; 	Written by:	R.M. van Hees (SRON), 26-01-2010
;       Modified:  RvH, coding according new standard, 28 March 2012
;-
;---------------------------------------------------------------------------
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

PRO __GET_24_STATE_DARK, sdmf_path, fileList, stateID, nchan, ipix, $
   mtbl, signal, noise, pet=pet, status=status
 compile_opt idl2,logical_predicate,hidden
; 
 mtbl = !NULL
 signal = !NULL
 noise = !NULL
 status = 'SUCCESS'
;
 Structure  = { MonitorRecord     , $ 
                Orbit             : 0L,       $
                MagicNumber       : 0L,       $ 
                MemoryFlag        : 0s,       $
                NonLinearityFlag  : 0s,       $
                StateId           : 0L,       $
                Time              : 0.d,      $
                Phase             : 0.0,      $
                Saa               : 0L,       $
                EsmAngle          : 0.0,      $
                AsmAngle          : 0.0,      $
                Tobm              : 0.0,      $ 
                Tdet              : fltarr(8),     $
                Pet               : fltarr(8192),  $
                Readout           : fltarr(8192),  $
                ReadoutError      : fltarr(8192),  $
                ReadoutNoise      : fltarr(8192)  $
              } 

 Data = !NULL
 FOREACH file, fileList DO BEGIN
    MonitorFile = sdmf_path + 'Data/' + file + '.monitor'
    MonitorData = ReadBinaryStructure(MonitorFile, {MonitorRecord})

    IF N_ELEMENTS( pet ) GT 0 AND nchan GT 0 THEN BEGIN
       FOREACH id, [8,26,46,63,67] DO BEGIN
          indx = WHERE( MonitorData.StateId EQ id, count )
          IF count EQ 0 THEN CONTINUE

          FOREACH tmpData, MonitorData[indx] DO BEGIN
             tmpData.Phase += 0.092
             IF (nchan LT 6 AND tmpData.Saa EQ 1) $
                OR (tmpData.Phase GT 0.4 AND tmpData.Phase LT 0.975) $
                OR tmpData.Pet[(nchan-1)*1024] NE pet THEN CONTINUE

             tmpMtbl = CREATE_STRUCT( 'julianDay', tmpData.Time, $
                                      'absOrbit', tmpData.Orbit, $
                                      'stateID', tmpData.StateId, $
                                      'saaFlag', tmpData.Saa, $
                                      'orbitPhase', tmpData.Phase, $
                                      'asmAngle', tmpData.AsmAngle, $
                                      'esmAngle', tmpData.EsmAngle, $
                                      'obmTemp', tmpData.Tobm, $
                                      'detectorTemp', tmpData.Tdet )

             IF Data EQ !NULL THEN BEGIN
                mtbl = tmpMtbl
                Data = tmpData
             ENDIF ELSE BEGIN
                ni = WHERE( ABS(Data.Phase - tmpData.Phase) LT 1e-3, num )
                IF num EQ 0 THEN BEGIN
                   mtbl = [mtbl, tmpMtbl]
                   Data = [Data, tmpData]
                ENDIF
             ENDELSE
          ENDFOREACH
       ENDFOREACH
    ENDIF ELSE BEGIN
       indx = WHERE( MonitorData.StateId EQ stateID, count )
       IF count EQ 0 THEN CONTINUE

       FOREACH tmpData, MonitorData[indx] DO BEGIN
          tmpData.Phase += 0.092
          IF (nchan LT 6 AND tmpData.Saa EQ 1) $
             OR (tmpData.Phase GT 0.4 AND tmpData.Phase LT 0.975) THEN CONTINUE

          tmpMtbl = CREATE_STRUCT( 'julianDay', tmpData.Time, $
                                   'absOrbit', tmpData.Orbit, $
                                   'stateID', tmpData.StateId, $
                                   'saaFlag', tmpData.Saa, $
                                   'orbitPhase', tmpData.Phase, $
                                   'asmAngle', tmpData.AsmAngle, $
                                   'esmAngle', tmpData.EsmAngle, $
                                   'obmTemp', tmpData.Tobm, $
                                   'detectorTemp', tmpData.Tdet )

          IF Data EQ !NULL THEN BEGIN
             mtbl = tmpMtbl
             Data = tmpData
          ENDIF ELSE BEGIN
             ni = WHERE( ABS(Data.Phase - tmpData.Phase) LT 1e-3, num )
             IF num EQ 0 THEN BEGIN
                mtbl = [mtbl, tmpMtbl]
                Data = [Data, tmpData]
             ENDIF
          ENDELSE
       ENDFOREACH
    ENDELSE
 ENDFOREACH
 IF Data NE !NULL THEN BEGIN
    IF ipix GT 0 THEN BEGIN
       buff = Data.Readout[ipix-1]
       signal = TOTAL( buff, /PRESERVE, /NaN ) $
                / (TOTAL( FINITE(buff), /INTEGER ) > 1)

       buff = (Data.ReadoutNoise[ipix-1])^2
       noise = SQRT( TOTAL( buff, /PRESERVE, /NaN ) $
                     / (TOTAL( FINITE(buff), /INTEGER ) > 1))
    ENDIF ELSE IF nchan GT 0 THEN BEGIN
       buff = Data.Readout[(nchan-1)*1024:nchan*1024-1]
       signal = TOTAL( buff, 2, /PRESERVE, /NaN ) $
                / (TOTAL( FINITE(buff), 2, /INTEGER ) > 1)

       buff = (Data.ReadoutNoise[(nchan-1)*1024:nchan*1024-1])^2
       noise = SQRT( TOTAL( buff, 2, /PRESERVE, /NaN ) $
                     / (TOTAL( FINITE(buff), 2, /INTEGER ) > 1))
    ENDIF ELSE BEGIN
       signal = TOTAL( Data.Readout, 2, /PRESERVE, /NaN ) $
                / (TOTAL( FINITE(Data.Readout), 2, /INTEGER ) > 1)

       noise = SQRT( TOTAL( Data.ReadoutNoise^2, 2, /PRESERVE, /NaN ) $
                     / (TOTAL( FINITE(Data.ReadoutNoise^2), 2, /INTEGER ) > 1))
    ENDELSE
 ENDIF

 RETURN
END

;---------------------------------------------------------------------------
PRO __GET_30_STATE_DARK, gid, orbitRange, nchan, ipix, mtbl, signal, noise, $
   pet=pet, status=status
 compile_opt idl2,logical_predicate,hidden
; 
 mtbl = !NULL
 signal = !NULL
 noise = !NULL
 status = 'SUCCESS'
;
; obtain indices to entries of the requested orbit
;
 dd = H5D_OPEN( gid, 'orbitList' )
 orbitList = H5D_READ( dd )
 H5D_CLOSE, dd
 IF N_ELEMENTS( orbitRange ) EQ 1 THEN BEGIN
    orbitRange = [orbitRange, orbitRange]
 ENDIF
 metaIndx = WHERE( orbitList GE orbitRange[0] $
                   AND orbitList LE orbitRange[1], numIndx )
 IF numIndx EQ 0 THEN BEGIN
    status = 'NO_ENTRY'
    MESSAGE, 'no entries found in database', /INFO
    RETURN
 ENDIF
; 
; if PET then read 'readoutPet' and check if pet == readoutPet, else return
;
 IF N_ELEMENTS( pet ) GT 0 AND nchan GT 0 THEN BEGIN
    dd = H5D_OPEN( gid, 'readoutPet' )
    IF dd LT 0 THEN BEGIN
       status = 'FAILED'
       MESSAGE, 'Could not open dataset: readoutPet', /INFO
    ENDIF
    space_id = H5D_GET_SPACE( dd )
    mem_space_id = H5S_CREATE_SIMPLE( [1,1] )
    readoutPET = FLTARR( numIndx )
    FOR ni = 0, numIndx-1 DO BEGIN
       H5S_SELECT_HYPERSLAB, space_id, [metaIndx[ni],(nchan-1)*1024+511], $
                             [1,1], /reset
       temp = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
       readoutPET[ni] = temp
    ENDFOR
    H5S_CLOSE, mem_space_id
    H5S_CLOSE, space_id
    H5D_CLOSE, dd

    numIndx2 = 0L & metaIndx2 = !NULL
    FOR ni = 0, numIndx-1 DO BEGIN
       IF readoutPET[ni] EQ pet THEN BEGIN
          metaIndx2 = [metaIndx2, metaIndx[ni]]
          numIndx2 += 1L
       ENDIF
    ENDFOR

    IF numIndx2 EQ 0L THEN BEGIN
       status = 'PET_WRONG'
       RETURN
    ENDIF

    IF numIndx2 EQ numIndx THEN $
       status = 'SUCCESS' $
    ELSE BEGIN
       status = 'PET_CHANGED'
       numIndx = numIndx2
       metaIndx = metaIndx2
    ENDELSE
 ENDIF
;
 dd = H5D_OPEN( gid, 'metaTable' )
 IF dd LT 0 THEN BEGIN
    status = 'FAILED'
    MESSAGE, 'Could not open dataset: metaTable', /INFO
 ENDIF
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
; reject SAA measurements of chan 1-5 and non-eclipse darks
;
 IF nchan LT 6 THEN $
    indx = WHERE( mtbl.saaFlag EQ 0 AND $
                  (mtbl.orbitPhase LE 0.4 OR mtbl.orbitPhase GE 0.975), num ) $
 ELSE $
    indx = WHERE( mtbl.orbitPhase LE 0.4 OR mtbl.orbitPhase GE 0.975, num )
 IF num EQ 0 THEN BEGIN
    status = 'NO_ENTRY'
    MESSAGE, 'no entries found in database', /INFO
    RETURN
 ENDIF
 numIndx = num
 metaIndx = metaIndx[indx]
 mtbl = mtbl[indx]
;
 dd = H5D_OPEN( gid, 'readoutMean' )
 IF dd LT 0 THEN BEGIN
    status = 'FAILED'
    MESSAGE, 'Could not open dataset: readoutMean', /INFO
 ENDIF
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
;
 dd = H5D_OPEN( gid, 'readoutNoise' )
 IF dd LT 0 THEN BEGIN
    status = 'FAILED'
    MESSAGE, 'Could not open dataset: readoutNoise', /INFO
 ENDIF
 space_id = H5D_GET_SPACE( dd )
 IF ipix GT 0 THEN BEGIN
    mem_space_id = H5S_CREATE_SIMPLE( [1,1] )
    H5S_SELECT_HYPERSLAB, space_id, [metaIndx[0],ipix-1], [1,1], /reset
    temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id  )
    sdev = REPLICATE( temp[0], numIndx )
    sdev[0] = temp
    FOR ni = 1, numIndx-1 DO BEGIN
       H5S_SELECT_HYPERSLAB, space_id, [metaIndx[ni],ipix-1], [1,1], /reset
       temp = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
       sdev[ni] = temp
    ENDFOR
 ENDIF ELSE IF nchan GT 0 THEN BEGIN
    mem_space_id = H5S_CREATE_SIMPLE( [1,1024] )
    H5S_SELECT_HYPERSLAB, space_id, [metaIndx[0],(nchan-1)*1024], $
                          [1,1024], /reset
    temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id  )
    sdev = REPLICATE( temp[0], 1024, numIndx )
    sdev[*,0] = temp
    FOR ni = 1, numIndx-1 DO BEGIN
       H5S_SELECT_HYPERSLAB, space_id, [metaIndx[ni],(nchan-1)*1024], $
                             [1,1024], /reset
       temp = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
       sdev[*,ni] = temp
    ENDFOR
 ENDIF ELSE BEGIN
    mem_space_id = H5S_CREATE_SIMPLE( [1,8192] )
    H5S_SELECT_HYPERSLAB, space_id, [metaIndx[0],0], [1,8192], /reset
    temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id  )
    sdev = REPLICATE( temp[0], 8192, numIndx )
    sdev[*,0] = temp
    FOR ni = 1, numIndx-1 DO BEGIN
       H5S_SELECT_HYPERSLAB, space_id, [metaIndx[ni],0], [1,8192], /reset
       temp = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
       sdev[*,ni] = temp
    ENDFOR
 ENDELSE
 H5S_CLOSE, mem_space_id
 H5S_CLOSE, space_id
 H5D_CLOSE, dd
;
; calculate the average per orbit
;
 sz = SIZE( avg )
 IF sz[0] EQ 1 THEN BEGIN
    signal = FLTARR( (orbitRange[1]-orbitRange[0]+1) )
    noise = FLTARR( (orbitRange[1]-orbitRange[0]+1) )
    nj = 0L
    FOR ni = orbitRange[0], orbitRange[1] DO BEGIN
       indx = WHERE( mtbl.absOrbit EQ ni, count )
       IF count EQ 0 THEN CONTINUE
       IF count GT 1 THEN BEGIN
          signal[nj] = TOTAL( avg[indx], /PRESERVE, /NaN ) $
                       / (TOTAL( FINITE(avg[indx]), /INTEGER ) > 1)
          noise[nj] = SQRT( TOTAL( (sdev[indx])^2, /PRESERVE, /NaN) $
                          / (TOTAL( FINITE(avg[indx]), /INTEGER ) > 1))
       ENDIF ELSE BEGIN
          signal[nj] = avg[indx]
          noise[nj] = sdev[indx]
       ENDELSE
       nj += 1L
    ENDFOR
    signal = signal[0:nj-1]
    noise = noise[0:nj-1]
 ENDIF ELSE BEGIN
    signal = FLTARR( sz[1], (orbitRange[1]-orbitRange[0]+1) )
    noise = FLTARR( sz[1], (orbitRange[1]-orbitRange[0]+1) )
    nj = 0L
    FOR ni = orbitRange[0], orbitRange[1] DO BEGIN
       indx = WHERE( mtbl.absOrbit EQ ni, count )
       IF count EQ 0 THEN CONTINUE
       IF count GT 1 THEN BEGIN
          signal[*,nj] = TOTAL( avg[*,indx], 2, /PRESERVE, /NaN ) $
                         / (TOTAL( FINITE(avg[*, indx]), 2, /INTEGER ) > 1)
          buff = (sdev[*,indx])^2
          noise[*,nj] = SQRT(TOTAL( buff, 2, /PRESERVE, /NaN ) $
                             / (TOTAL( FINITE(buff), 2, /INTEGER ) > 1))
       ENDIF ELSE BEGIN
          signal[*,nj] = avg[*,indx]
          noise[*,nj] = sdev[*,indx]
       ENDELSE
       nj += 1L
    ENDFOR
    signal = signal[*,0:nj-1]
    noise = noise[*,0:nj-1]
 ENDELSE

 SCIA_RD_H5_MEMCORR, memcorr
 SCIA_RD_H5_NLCORR, nlcorr, curveIndex
 signorm = ROUND(signal) > 0 < (2L^16 - 1)
 IF ipix GT 0 THEN BEGIN
    ichan = ipix / 1024
    IF ichan LT 5 THEN $
       signal -= memcorr[signorm, ichan] $
    ELSE $
       signal -= nlcorr[signorm, curveIndex[ipix MOD 1024, ichan]]
 ENDIF ELSE IF nchan GT 0 THEN BEGIN
    IF nchan LT 6 THEN $
       signal -= memcorr[signorm, nchan-1] $
    ELSE $
       signal -= nlcorr[signorm, curveIndex[*, nchan-1]]
 ENDIF ELSE BEGIN
    signal[0:1023,*]    -= memcorr[signorm[0:1023,*]   , 0]
    signal[1024:2047,*] -= memcorr[signorm[1024:2047,*], 1]
    signal[2048:3071,*] -= memcorr[signorm[2048:3071,*], 2]
    signal[3072:4095,*] -= memcorr[signorm[3072:4095,*], 3]
    signal[4096:5119,*] -= memcorr[signorm[4096:5119,*], 4]
    signal[5120:6143,*] -= nlcorr[signorm[5120:6143,*], curveIndex[*, 5]]
    signal[6144:7167,*] -= nlcorr[signorm[6144:7167,*], curveIndex[*, 6]]
    signal[7168:8191,*] -= nlcorr[signorm[7168:8191,*], curveIndex[*, 7]]
 ENDELSE

 RETURN
END

;---------------------------------------------------------------------------
PRO __GET_31_STATE_DARK, gid, orbitRange, nchan, ipix, mtbl, signal, noise
 compile_opt idl2,logical_predicate,hidden
; 
 mtbl = !NULL
 signal = !NULL
 noise = !NULL
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
 H5S_SELECT_HYPERSLAB, space_id, [iorbit-1], [numOrbits], /reset
 mtbl = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id  )
 H5S_CLOSE, mem_space_id
 H5S_CLOSE, space_id
 H5D_CLOSE, dd

 dd = H5D_OPEN( gid, 'darkSignal' )
 IF dd LT 0 THEN MESSAGE, 'Could not open dataset: darkSignal'
 space_id = H5D_GET_SPACE( dd )
 IF ipix GT 0 THEN BEGIN
    mem_space_id = H5S_CREATE_SIMPLE( [1, numOrbits] )
    H5S_SELECT_HYPERSLAB, space_id, [ipix-1, iorbit-1], $
                          [1, numOrbits], /reset
 ENDIF ELSE IF nchan GT 0 THEN BEGIN
    mem_space_id = H5S_CREATE_SIMPLE( [1024, numOrbits] )
    H5S_SELECT_HYPERSLAB, space_id, [(nchan-1)*1024, iorbit-1], $
                          [1024, numOrbits], /reset
 ENDIF ELSE BEGIN
    mem_space_id = H5S_CREATE_SIMPLE( [8192, numOrbits] )
    H5S_SELECT_HYPERSLAB, space_id, [0, iorbit-1], $
                          [8192, numOrbits], /reset
 ENDELSE
 signal = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id  )
 IF ipix GT 0 THEN signal = REFORM( signal )
 H5S_CLOSE, mem_space_id
 H5S_CLOSE, space_id
 H5D_CLOSE, dd

 dd = H5D_OPEN( gid, 'darkNoise' )
 IF dd LT 0 THEN MESSAGE, 'Could not open dataset: darkNoise'
 space_id = H5D_GET_SPACE( dd )
 IF ipix GT 0 THEN BEGIN
    mem_space_id = H5S_CREATE_SIMPLE( [1, numOrbits] )
    H5S_SELECT_HYPERSLAB, space_id, [ipix-1, iorbit-1], [1, numOrbits], /reset
 ENDIF ELSE IF nchan GT 0 THEN BEGIN
    mem_space_id = H5S_CREATE_SIMPLE( [1024, numOrbits] )
    H5S_SELECT_HYPERSLAB, space_id, [(nchan-1)*1024, iorbit-1], $
                          [1024, numOrbits], /reset
 ENDIF ELSE BEGIN
    mem_space_id = H5S_CREATE_SIMPLE( [8192, numOrbits] )
    H5S_SELECT_HYPERSLAB, space_id, [0, iorbit-1], [8192, numOrbits], /reset
 ENDELSE
 noise = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id  )
 IF ipix GT 0 THEN noise = REFORM( noise )
 H5S_CLOSE, mem_space_id
 H5S_CLOSE, space_id
 H5D_CLOSE, dd

 RETURN
END

;---------------------------------------------------------------------------
PRO SDMF_READ_STATEDARK, stateID_in, orbitRange_in, mtbl, signal, noise, $
                         channel=channel, pixelID=pixelID, pet=pet, $
                         SDMF_H5_DB=SDMF_H5_DB, SDMF_VERSION=SDMF_VERSION, $
                         status=status
 compile_opt idl2,logical_predicate,hidden

 mtbl = -1
 status = -1
 IF N_PARAMS() NE 5 THEN BEGIN
    MESSAGE, ' Usage: SDMF_READ_STATEDARK, stateID, orbit, mtbl' $
             + ', signal, noise, channel=channel, pixelID=pixelID' $
             + ', pet=pet, SDMF_H5_DB=SDMF_H5_DB, SDMF_VERSION=SDMF_VERSION' $
             + ', status=status', /INFO
    RETURN
 ENDIF
 orbitRange = LONG( orbitRange_in )
 IF N_ELEMENTS( orbitRange ) GT 1 THEN BEGIN
    IF orbitRange[0] GT orbitRange[1] THEN BEGIN
       MESSAGE, ' invalid orbitRange -- orbitRange[0] > orbitRange[1]', /INFO
       RETURN
    ENDIF
 ENDIF ELSE orbitRange = [orbitRange, orbitRange]
 stateID = FIX( stateID_in )
 nchan = -1
 IF N_ELEMENTS( channel ) GT 0 THEN nchan = FIX( channel[0] )
 ipix = -1
 IF N_ELEMENTS( pixelID ) GT 0 THEN BEGIN
    ipix = FIX( pixelID[0] )
    nchan = 1 + (ipix / 1024)
 ENDIF

 mtbl = !NULL
 signal = !NULL
 noise = !NULL
 sdmfPath = GET_SDMF_PATH( SDMF_VERSION=SDMF_VERSION )
 CASE SDMF_VERSION OF
    '2.4': BEGIN
       monitorList = __GET_24_FILELIST( sdmfPath )

       FOR norb = orbitRange[0], orbitRange[1] DO BEGIN
          indx = WHERE( monitorList.Orbit EQ norb $
                        AND monitorList.QualityNumber GE 50, count )
          IF count EQ 0 THEN CONTINUE

          fileList = STRING( MonitorList[indx].FileName )
          fileList = fileList[REVERSE(SORT( fileList ))]

          __GET_24_STATE_DARK, sdmfPath, fileList, stateID, nchan, ipix, $
             mtbl_buff, sign_buff, noise_buff, pet=pet, status=status
          mtbl = [mtbl, mtbl_buff]
          signal = [[signal], [sign_buff]]
          noise = [[noise], [noise_buff]]
       ENDFOR
       IF ipix GT 0 THEN BEGIN
          signal = REFORM( signal )
          noise = REFORM( noise )
       ENDIF
    END
    '3.0': BEGIN
       IF N_ELEMENTS( SDMF_H5_DB ) EQ 0 THEN $
          SDMF_H5_DB = sdmfPath + 'sdmf_extract_calib.h5'
       IF (~ FILE_TEST( SDMF_H5_DB, /READ, /NOEXPAND )) THEN BEGIN
          MESSAGE, ' Can not read from database: ' + SDMF_H5_DB, /INFO
          RETURN
       ENDIF

       fid = H5F_OPEN( SDMF_H5_DB )
       IF fid LT 0 THEN MESSAGE, 'Could not open database: ' + SDMF_H5_DB
       IF N_ELEMENTS( pet ) GT 0 THEN BEGIN
          IF nchan EQ -1 THEN $
             MESSAGE, '*** Fatal, pet requires selection on channel or pixel'

          FOREACH id, [8,26,46,63,67] DO BEGIN
             grpName = 'State_' + STRING(id, format='(I02)')
             gid = H5G_OPEN( fid, grpName )
             IF gid LT 0 THEN MESSAGE, 'Could not open group: ' + grpName

             __GET_30_STATE_DARK, gid, orbitRange, nchan, ipix, tmp1, $
                tmp2, tmp3, pet=pet, status=statPET
             print, statPET
             IF statPET EQ 'PET_CORRECT' OR statPET EQ 'PET_CHANGED' THEN $
                noise = [[noise], [tmp3]]
             H5G_CLOSE, gid
             IF statPET EQ 'NO_ENTRY' OR statPET EQ 'FAILED' THEN break
             IF statPET EQ 'SUCCESS' OR statPET EQ 'PET_CHANGED' THEN BEGIN
                mtbl = [mtbl, tmp1]
                signal = [[signal], [tmp2]]
                IF statPET EQ 'SUCCESS' THEN break
             ENDIF
          ENDFOREACH
       ENDIF ELSE BEGIN
          grpName = 'State_' + STRING(stateID, format='(I02)')
          gid = H5G_OPEN( fid, grpName )
          IF gid LT 0 THEN MESSAGE, 'Could not open group: ' + grpName
          __GET_30_STATE_DARK, gid, orbitRange, nchan, ipix, mtbl, signal, noise
          H5G_CLOSE, gid
       ENDELSE
       H5F_CLOSE, fid
    END
    '3.1': BEGIN
       IF N_ELEMENTS( SDMF_H5_DB ) EQ 0 THEN $
          SDMF_H5_DB = sdmfPath + 'sdmf_dark.h5'
       IF (~ FILE_TEST( SDMF_H5_DB, /READ, /NOEXPAND )) THEN BEGIN
          MESSAGE, ' Can not read from database: ' + SDMF_H5_DB, /INFO
          RETURN
       ENDIF

       fid = H5F_OPEN( SDMF_H5_DB )
       IF fid LT 0 THEN MESSAGE, 'Could not open database: ' + SDMF_H5_DB
       IF N_ELEMENTS( pet ) GT 0 THEN BEGIN
          IF nchan EQ -1 THEN $
             MESSAGE, '*** Fatal, pet requires selection on channel or pixel'
          pet = FLOAT( pet )
          clusDef = GET_SCIA_CLUSDEF( 8 )
          clusList = WHERE( clusDef.chanID EQ nchan )

          FOREACH id, [8,26,46,63,67] DO BEGIN
             grpName = 'State_' + STRING(id, format='(I02)')
             gid = H5G_OPEN( fid, grpName )
             IF gid LT 0 THEN MESSAGE, 'Could not open group: ' + grpName
             dd = H5D_OPEN( gid, 'clusConf' )
             clusConf = H5D_READ( dd )
             H5D_CLOSE, dd
             dimConf = N_ELEMENTS(clusConf)
             indx1 = WHERE( orbitRange[0] GE clusConf.orbit, count1 )
             nij = indx1[count1-1] & njj = nij
             IF N_ELEMENTS( orbitRange ) GT 1 THEN BEGIN
                indx2 = WHERE( orbitRange[1] GE clusConf.orbit, count2 )
                njj = indx2[count2-1]
             ENDIF 

             FOR nj = nij, njj DO BEGIN
                IF clusConf[nj].pet[clusList[0]] EQ pet THEN BEGIN
                   orbitMin = (nj EQ 0) ? 0 :  clusConf[nj].orbit
                   orbitMax = (nj LT dimConf-1) ? clusConf[nj+1].orbit-1 : 99999
                   orbitList = [ orbitRange[0] > orbitMin, $
                                 orbitRange[1] < orbitMax]
                   grpName = 'State_' + STRING(id, format='(I02)')
                   gid = H5G_OPEN( fid, grpName )
                   IF gid LT 0 THEN MESSAGE, 'Could not open group: ' + grpName
                   __GET_31_STATE_DARK, gid, orbitList, nchan, ipix, $
                      tmp1, tmp2, tmp3
                   mtbl = [mtbl, tmp1]
                   signal = [[signal], [tmp2]]
                   noise = [[noise], [tmp3]]
                   H5G_CLOSE, gid
                ENDIF
             ENDFOR
          ENDFOREACH
       ENDIF ELSE BEGIN
          grpName = 'State_' + STRING(stateID, format='(I02)')
          gid = H5G_OPEN( fid, grpName )
          IF gid LT 0 THEN MESSAGE, 'Could not open group: ' + grpName
          __GET_31_STATE_DARK, gid, orbitRange, nchan, ipix, mtbl, signal, noise
          H5G_CLOSE, gid
       ENDELSE
       H5F_CLOSE, fid
    END
    ELSE: BEGIN
       MESSAGE, 'Not available for SDMF version ' + SDMF_VERSION
    END
 ENDCASE

 status = 0
 RETURN
END
