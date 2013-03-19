;
; COPYRIGHT (c) 2004 - 2013 SRON (R.M.van.Hees@sron.nl)
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
;	GET_LV1_MDS_STATE
;
; PURPOSE:
;	obtain SCIA level 1 state definitions for given selection criteria
;
; CATEGORY:
;	SCIA level 1b and 1c data
;
; CALLING SEQUENCE:
;	Result = GET_LV1_MDS_STATE( dsd, keywrd=keywrd, ... )
;
; INPUTS:
;       dsd :    structure for Data Set Descriptors

; KEYWORD PARAMETERS:
;    type_mds :  scalar of type integer to select MDS of type: 
;                {!nadc.sciaNadir=1, !nadc.sciaLimb=2, !nadc.sciaOccult=3, !nadc.sciaMonitor=4}
;    category :  read only selected category of measurements (scalar or array)
;                (default or -1: all categories)
;    state_id :  read only selected states (scalar or array)
;                (default or -1: all states)
;  OrbitPhase :  selection on orbit phase, required are 2 values. The
;                orbit phase seletion is based on "States of the Product"
; geolocation :  vector of type float which defines the geografical
;                region as [lat_min,lat_max,lon_min,lon_max] (using LADS)
;       NoSAA :  set this keyword to inhibit to reject states with the
;                SAA set (using SQADS) 
;      period :  read only MDS within a time-window (scalar or array)
;                date must be given in decimal julian 2000 day
;                (default or -1: all data)
; state_posit :  relative index/indices to the state record(s), this
;                is last selection applied, only counting those MDS(s)
;                which contain data (default or -1: all data)
;  indx_state :  named variable which contains the indices to the
;                original state records as defined in the level 1b/1c product
;       count :  named variable which contains the number of selected states
;      status :  named variable which contains the error status (0 = ok)
;
; OUTPUTS:
;	SCIA level 1b state records, or -1 if the selection is empty
;
; EXAMPLE:
;	
;
; MODIFICATION HISTORY:
; 	Written by:	Richard van Hees (SRON), March 2004
;       Modified:  RvH, 08 Dec 2004
;                    added selection on orbit phase
;-
FUNCTION GET_LV1_MDS_STATE, dsd, type_mds=type_mds, category=category, $
                            state_id=state_id, period=period, $
                            geolocation=geolocation, NoSAA=NoSAA, $
                            state_posit=state_posit, indx_state=indx_state, $
                            count=count, OrbitPhase=OrbitPhase, status=status
  compile_opt idl2,logical_predicate,hidden

; check required parameters
  SWITCH N_PARAMS() OF
     1: BEGIN
        IF SIZE( dsd, /TNAME ) EQ 'STRUCT' THEN BEGIN
           IF TAG_NAMES( dsd, /STRUCT ) EQ 'DSD_SCIA' THEN BREAK
        ENDIF
     END
     ELSE: BEGIN
        MESSAGE, ' Usage: Result = GET_LV1_MDS_STATE( dsd, ... )', /INFO
        GOTO, Return_Failed
     END
  ENDSWITCH

; definition of SCIAMACHY related constants
  NotSet = -1

; initialize
  indx_state = NotSet
  count = 0

; read all state definitions
  SCIA_LV1_RD_STATE, dsd, state, status=status
  IF status NE 0 THEN GOTO, Return_Failed

; check optional parameters
  IF N_ELEMENTS( type_mds ) EQ 0 THEN $
     type_mds = NotSet $
  ELSE IF type_mds[0] EQ -1 THEN $
     type_mds = NotSet

  IF N_ELEMENTS( state_id ) EQ 0 THEN $
     state_id = NotSet $
  ELSE IF state_id[0] EQ -1 THEN $
     state_id = NotSet

  IF N_ELEMENTS( category ) EQ 0 THEN $
     category = NotSet $
  ELSE IF category[0] EQ -1 THEN $
     category = NotSet

  IF N_ELEMENTS( period ) EQ 0 THEN BEGIN
     period = NotSet
  ENDIF ELSE IF period[0] EQ -1 THEN BEGIN
     period = NotSet
  ENDIF ELSE IF N_ELEMENTS( period ) EQ 1 THEN BEGIN
     epsilon = 1.d-6 / (24.d * 3600.d)
     period=[period[0]-epsilon,period[0]+epsilon]
  ENDIF

; select only states which contain data, optionally select states on SAA flag
  IF KEYWORD_SET( NoSAA ) THEN BEGIN
     SCIA_LV1_RD_SQADS, dsd, sqads, status=status
     IF status NE 0 THEN GOTO, Return_Failed
     indx = WHERE( sqads.flag_saa_region NE 1 AND state.flag_mds EQ 0, count )
  ENDIF ELSE BEGIN
     indx = WHERE( state.flag_mds EQ 0, count )
  ENDELSE
  IF count EQ 0 THEN GOTO, Return_Failed
  indx_state = indx

; select states on Orbit Phase (needs index from previous selection!!!)
  IF N_ELEMENTS( OrbitPhase ) EQ 0 THEN BEGIN
     OrbitPhase = NotSet
  ENDIF ELSE IF OrbitPhase[0] EQ -1 THEN BEGIN
     OrbitPhase = NotSet
  ENDIF ELSE IF N_ELEMENTS( OrbitPhase ) EQ 1 $
     OR N_ELEMENTS( OrbitPhase ) GT 2 THEN BEGIN
     MESSAGE, 'Orbit Phase should be given as [min,max]', /Info
     GOTO, Return_Failed
  ENDIF ELSE IF N_ELEMENTS( OrbitPhase ) EQ 2 THEN BEGIN
     phase = state[indx_state].orbit_phase
     indx = WHERE( phase GE OrbitPhase[0] AND phase LE OrbitPhase[1], count )
     IF count EQ 0 THEN GOTO, Return_Failed
     indx_state = indx_state[indx]
  ENDIF

; select states on Geolocation
  IF N_ELEMENTS( geolocation ) EQ 4 THEN BEGIN
     SCIA_RD_LADS, dsd, lads, status=status
     IF status NE 0 THEN GOTO, Return_Failed
     lads = lads[indx_state]

     geo = LONG( 1e6 * geolocation )
     num = 0
     FOR nr = 0, count-1 DO BEGIN
        nc = 0
        in_region = 0
        REPEAT BEGIN
           IF (lads[nr].coord[nc].lat GE geo[0] $
               AND lads[nr].coord[nc].lat LE geo[1]) $
              AND (lads[nr].coord[nc].lon GE geo[2] $
                   AND lads[nr].coord[nc].lon LE geo[3]) THEN BEGIN
              IF num EQ 0 THEN $
                 indx = indx_state[nr] $
              ELSE $
                 indx = [indx,indx_state[nr]]
              num++
              in_region = 1
           ENDIF
        ENDREP UNTIL ( ++nc GE 4 OR in_region EQ 1 )
     ENDFOR
     IF num EQ 0 THEN GOTO, Return_Failed

     count = num
     indx_state = indx[0:num-1]
  ENDIF

; select packages on type_mds
  IF type_mds[0] NE NotSet THEN BEGIN
     nr_type = N_ELEMENTS( type_mds )
     num = 0
     FOR nr = 0, nr_type-1 DO BEGIN 
        indx = WHERE( state[indx_state].type_mds EQ type_mds[nr], count )
        IF count GT 0 THEN BEGIN
           IF num EQ 0 THEN $
              indx_all = indx_state[indx] $
           ELSE $
              indx_all = [indx_all, indx_state[indx]]
           num += count
        ENDIF
     ENDFOR
     IF num EQ 0 THEN GOTO, Return_Failed

     indx_state = indx_all[UNIQ(indx_all, SORT(indx_all))]
     count = N_ELEMENTS( indx_state )
  ENDIF

; select packages on State ID
  IF state_id[0] NE NotSet THEN BEGIN
     nr_state = N_ELEMENTS( state_id )
     num = 0
     FOR nr = 0, nr_state-1 DO BEGIN 
        indx = WHERE( state[indx_state].state_id EQ state_id[nr], count )
        IF count GT 0 THEN BEGIN
           IF num EQ 0 THEN $
              indx_all = indx_state[indx] $
           ELSE $
              indx_all = [indx_all, indx_state[indx]]
           num += count
        ENDIF
     ENDFOR
     IF num EQ 0 THEN GOTO, Return_Failed

     indx_state = indx_all[UNIQ(indx_all, SORT(indx_all))]
     count = N_ELEMENTS( indx_state )
  ENDIF

; select packages on measurement category
  IF category[0] NE NotSet THEN BEGIN
     nr_cat = N_ELEMENTS( category )
     num = 0
     FOR nr = 0, nr_cat-1 DO BEGIN 
        indx = WHERE( state[indx_state].category EQ category[nr], count )
        IF count GT 0 THEN BEGIN
           IF num EQ 0 THEN $
              indx_all = indx_state[indx] $
           ELSE $
              indx_all = [indx_all, indx_state[indx]]
           num += count
        ENDIF
     ENDFOR
     IF num EQ 0 THEN GOTO, Return_Failed

     indx_state = indx_all[UNIQ(indx_all, SORT(indx_all))]
     count = N_ELEMENTS( indx_state )
  ENDIF

; select packages within a time window
  IF period[0] NE NotSet THEN BEGIN
     SecInDay = 60d * 60d * 24d
     mjd_time = state[indx_state].mjd.days $
                + (state[indx_state].mjd.secnd $
                   + state[indx_state].mjd.musec / 1d6) $
                / SecInDay 
     indx = WHERE( mjd_time GE period[0] AND mjd_time LE period[1], count )
     IF count EQ 0 THEN GOTO, Return_Failed

     indx_state = indx_state[indx]
  ENDIF

; selection on relative index
  IF N_ELEMENTS( state_posit ) GT 0 THEN BEGIN
     IF state_posit[0] GE count THEN BEGIN
        MESSAGE, 'indx_state too large...', /INFO
        GOTO, Return_Failed
     ENDIF
     IF N_ELEMENTS( state_posit ) EQ 1 THEN BEGIN
        IF state_posit GE 0 THEN BEGIN
           count = 1
           indx_state = indx_state[state_posit]
        ENDIF
     ENDIF ELSE BEGIN
        indx_state = indx_state[state_posit[0]:state_posit[1] < (count-1)]
        count = N_ELEMENTS( indx_state )
     ENDELSE
  ENDIF
  RETURN, state[indx_state]

Return_Failed:
  if status NE 0 THEN $
     NADC_ERR_TRACE $
  ELSE $
     status = -1
  count = 0

  RETURN, (-1)
END
