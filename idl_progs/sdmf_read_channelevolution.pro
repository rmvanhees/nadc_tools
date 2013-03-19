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
;	SDMF_READ_CHANNELEVOLUTION
;
; PURPOSE:
;	read rows of given orbit from CHANNELEVOLUTION metaTable. In
;	addition, one can obtain CHANNELEVOLUTION correction parameters.
;
; CATEGORY:
;	SDMF - SCIA calibration
;
; CALLING SEQUENCE:
;	SDMF_READ_CHANNELEVOLUTION, chanID, orbit, chanevo
;
; INPUTS:
;       chanID:         string ['1','2','3','4','5','6','6+','7','8']
;	orbit:	        absolute orbit number, a scalar or vector[min,max], 
;                       to select all orbits use -1
;
; OUTPUTS:
;       chanevo:        channel evolution record(s)
;
; KEYWORD PARAMETERS:
;       use_neighbours: flag to enable looking for neighboring valid entries
;       SDMF_H5_DB:     path to the SDMF Dark database
;                          default: /SCIA/SDMF30/sdmf_ppg.h5
;       SDMF_VERSION:   SDMF version <3.0>, default 3.0
;       status :        returns named variable with error flag (0 = ok)
;
; PROCEDURE:
;	ToDo...
;
; EXAMPLE:
;	ToDo...
;
; MODIFICATION HISTORY:
; 	Written by:	R.M. van Hees (SRON), 29 March 2012
;-
; ---------------------------------------------------------------------------
PRO SDMF_READ_CHANNELEVOLUTION, chanID, orbitRange_in, chanevo, $
                                use_neighbours=use_neighbours, $
                                SDMF_H5_DB=SDMF_H5_DB, $
                                SDMF_VERSION=SDMF_VERSION, $
                                status=status
 compile_opt idl2,logical_predicate,hidden

 mtbl = -1
 status = -1
 IF N_PARAMS() NE 3 THEN BEGIN
    MESSAGE, ' Usage: SDMF_READ_CHANNELEVOLUTION, chanID, orbit, chanevo' $
             + ', SDMF_H5_DB=SDMF_H5_DB, SDMF_VERSION' $
             + ', use_neighbours=use_neighbours, status=status', /INFO
    RETURN
 ENDIF
 orbitRange = LONG( orbitRange_in )
 IF ~ keyword_set(use_neighbours) THEN use_neighbours = 0

 IF N_ELEMENTS( SDMF_VERSION ) GT 0 THEN BEGIN
    IF SDMF_VERSION EQ '2.4' THEN $
       MESSAGE, 'FATAL: not implemented for SDMF v2.4.x'
 ENDIF

 IF N_ELEMENTS( SDMF_H5_DB ) EQ 0 THEN $
    SDMF_H5_DB = GET_SDMF_PATH( SDMF_VERSION=SDMF_VERSION ) $
                 + './sdmf_channelevolution.h5'
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
    MESSAGE, 'no entry found in database: ' + SDMF_H5_DB, /INFO
    RETURN
 ENDIF

 dd = H5D_OPEN( fid, 'Channel' + chanID )
 IF dd LT 0 THEN MESSAGE, 'Could not open dataset: ' + 'Channel' + chanID
 space_id = H5D_GET_SPACE( dd )
 dims = H5S_GET_SIMPLE_EXTENT_DIMS( space_id )
 IF numIndx LT dims[0] THEN BEGIN
    mem_space_id = H5S_CREATE_SIMPLE( [1] )
    H5S_SELECT_HYPERSLAB, space_id, [metaIndx[0]], [1], /reset
    buff = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
    chanevo = REPLICATE( buff, numIndx )
    FOR ii = 1, numIndx-1 DO BEGIN
       H5S_SELECT_HYPERSLAB, space_id, [metaIndx[ii]], [1], /reset
       buff = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id )
       chanevo[ii] = buff
    ENDFOR
    chanevo = REFORM(chanevo)
    H5S_CLOSE, mem_space_id
 ENDIF ELSE IF numIndx EQ dims[0] THEN BEGIN
    chanevo = H5D_READ( dd )
    ii = WHERE( orbitList LE 0, count )
    IF count GT 0 THEN $
       indx_sort = (SORT( orbitList ))[1:-1] $
    ELSE $
       indx_sort = SORT( orbitList )
    chanevo = chanevo[indx_sort]
 ENDIF
 H5S_CLOSE, space_id
 H5D_CLOSE, dd

 H5F_CLOSE, fid
 status = 0
 RETURN
END
