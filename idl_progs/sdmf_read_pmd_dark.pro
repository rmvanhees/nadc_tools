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
;	SDMF_READ_PMD_DARK
;
; PURPOSE:
;	Obtain PMD dark read-outs during a Dark state (only SDMF v3.1)
;
; CATEGORY:
;	
;
; CALLING SEQUENCE:
;	SDMF_READ_PMD_DARK, absOrbit, stateID, mtbl, pmdHist;
; INPUTS:
;	absOrbit:	(absolute) Orbit number, a scalar
;       stateID:        Calibration State ID, a scalar, valid ID's are: 
;                       8,16,26,39,46,48,52,59,61,62,63,65,67,69,70
;
; OUTPUTS:
;       mtbl:           structure with rows of table metaTable
;       pmdHist         histogram structures of the raw PMD read-outs
;
; KEYWORD PARAMETERS:
;	pmdMean:	average PMD darks (7 PMD's, high/low gain)
;       pmdAdev:        adev of PMD darks (7 PMD's, high/low gain)
;
;       SDMF_H5_DB:     full path to sdmf_extract_dark database (>= v3.1)
;                         default: /SCIA/SDMF31/sdmf_extract_dark.h5
;
; PROCEDURE:
;	ToDo
;
; EXAMPLE:
;	Please add!
;
; MODIFICATION HISTORY:
; 	Written by:     Richard van Hees (SRON), Januari 2011
;-
PRO SDMF_READ_PMD_DARK, orbit_in, stateID_in, mtbl, pmdHist, $
                        pmdMean=pmdMean, pmdAdev=pmdAdev, $
                        SDMF_H5_DB=SDMF_H5_DB
 compile_opt idl2,logical_predicate,hidden

 mtbl = -1
 pmdHist = -1
 status = -1
 IF N_PARAMS() NE 4 THEN BEGIN
    MESSAGE, ' Usage: SDMF_READ_PMD_DARK, absOrbit, stateID, mtbl, pmdHist' $
             + ', SDMF_H5_DB=SDMF_H5_DB', /INFO
    RETURN
 ENDIF
 absOrbit = FIX( orbit_in )
 stateID = FIX( stateID_in )

 IF N_ELEMENTS( SDMF_H5_DB ) EQ 0 THEN $
    SDMF_H5_DB = GET_SDMF_PATH( SDMF_VERSION='3.1' ) + './sdmf_extract_dark.h5'
 IF (~ FILE_TEST( SDMF_H5_DB, /READ, /NOEXPAND )) THEN BEGIN
    MESSAGE, ' Can not read from database: ' + SDMF_H5_DB, /INFO
    RETURN
 ENDIF

 fid = H5F_OPEN( SDMF_H5_DB )
 IF fid LT 0 THEN MESSAGE, 'Could not open database: ' + SDMF_H5_DB
 grpName = '/State_' + STRING(stateID, format='(I02)')
 gid = H5G_OPEN( fid, grpName )
 IF gid LT 0 THEN MESSAGE, 'Could not open group: ' + grpName
;
; obtain indices to entries of the requested orbit and state
;
 dd = H5D_OPEN( gid, 'orbitList' )
 orbitList = H5D_READ( dd )
 H5D_CLOSE, dd
 metaIndx = WHERE( orbitList EQ absOrbit, numIndx )
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
; read PMD histogram data (7 PMD's low-gain & high-gain)
;
 dd = H5D_OPEN( gid, 'histPMD' )
 IF dd LT 0 THEN MESSAGE, 'Could not open dataset: histPMD'
 space_id = H5D_GET_SPACE( dd )
 mem_space_id = H5S_CREATE_SIMPLE( [1] )
 H5S_SELECT_HYPERSLAB, space_id, metaIndx[0], [1], /reset
 temp = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id  )
 adim = N_ELEMENTS( temp )
 pmdHist = REPLICATE( temp[0], adim, numIndx )
 pmdHist[*,0] = temp
 FOR ni = 1, numIndx-1 DO BEGIN
    H5S_SELECT_HYPERSLAB, space_id, metaIndx[ni], [1], /reset
    temp = H5D_READ(dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id)
    pmdHist[*,ni] = temp
 ENDFOR
 H5S_CLOSE, mem_space_id
 H5S_CLOSE, space_id
 H5D_CLOSE, dd

 pmdMean = DBLARR( adim * numIndx )
 FOR ii = 0, (adim * numIndx-1) DO BEGIN
    pmdMean[ii] = TOTAL(pmdHist[ii].location * double(pmdHist[ii].count))
    IF pmdHist[ii].binsize GT 1 THEN $
       pmdMean[ii] += TOTAL( pmdHist[ii].remainder, /DOUBLE )
    pmdMean[ii] /= TOTAL( pmdHist[ii].count, /DOUBLE )
 ENDFOR

 IF ARG_PRESENT(pmdAdev) THEN BEGIN
    pmdAdev = DBLARR( adim * numIndx )
    FOR ii = 0, (adim * numIndx-1) DO BEGIN
       tmp = DOUBLE(pmdHist[ii].location + pmdHist[ii].remainder - pmdMean[ii])
       pmdAdev[ii] = TOTAL( ABS( tmp ) * double(pmdHist[ii].count) ) $
                     / TOTAL( pmdHist[ii].count, /DOUBLE ) 
    ENDFOR
 ENDIF
;
; close HDF5 resources
;
 H5G_CLOSE, gid
 H5F_CLOSE, fid
 status = 0
 RETURN
END
