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
;	GET_LV0_MDS_STATE
;
; PURPOSE:
;	obtain SCIA level 0 info-records for given selection criteria
;
; CATEGORY:
;	SCIA level 0 
;
; CALLING SEQUENCE:
;	Result = GET_LV0_MDS_STATE( info, keywrd=keywrd, ... )
;
; INPUTS:
;       info :    structure holding info about MDS records

; KEYWORD PARAMETERS:
;    type_mds :  string to select MDS of type: 'DET', 'AUX', 'PMD'
;                defaults to 'DET'
;    category :  read only selected category of measurements (scalar or array)
;                (default or -1: all categories)
;    state_id :  read only selected states (scalar or array)
;                (default or -1: all states)
;      period :  read only MDS within a time-window (scalar or array)
;                date must be given in decimal julian 2000 day
;                (default or -1: all data)
; state_posit :  relative index/indices to the state record(s), this
;                is last selection applied, only counting those MDS(s)
;                which contain data (default or -1: all data)
;  indx_state :  named variable which contains the indices to the
;                original state records as defined in the level 1b/1c product
;   num_state :  named variable which contains the number of selected states
;      status :  named variable which contains the error status (0 = ok)
;
; OUTPUTS:
;	selected SCIA level 0 info-records
;
; EXAMPLE:
;	
;
; MODIFICATION HISTORY:
; 	Written by:	Richard van Hees (SRON), March 2010
;-
FUNCTION GET_LV0_MDS_STATE, info_all, mds_type=mds_type, $
                            category=category, state_id=state_id, $
                            state_posit=state_posit, period=period, $
                            indx_state=indx_state, num_state=num_state, $
                            channels=channels, posit=posit
  compile_opt idl2,logical_predicate,hidden

; definition of SCIAMACHY related constants
  NotSet = -1

; initialize
  indx_state = NotSet
  num_state = 0

; check required parameters
  SWITCH N_PARAMS() OF
     1: BEGIN
        IF SIZE( info_all, /TNAME ) EQ 'STRUCT' THEN BEGIN
           IF TAG_NAMES( info_all, /STRUCT ) EQ 'MDS0_INFO' THEN BREAK
        ENDIF
     END
     ELSE: BEGIN
        MESSAGE, ' Usage: Result = GET_LV0_MDS_STATE( info, ... )', /INFO
        RETURN, -1
     END
  ENDSWITCH

; check optional parameters
  IF N_ELEMENTS( mds_type ) GT 0 THEN BEGIN
     mds_type = STRUPCASE( mds_type )
  ENDIF ELSE mds_type = 'DET'

  IF N_ELEMENTS( state_id ) EQ 0 THEN $
     state_id = NotSet $
  ELSE IF state_id[0] EQ -1 THEN $
     state_id = NotSet
     
  IF N_ELEMENTS( state_posit ) EQ 0 THEN $
     state_posit = NotSet $
  ELSE IF state_id[0] EQ -1 THEN $
     state_posit = NotSet

  IF N_ELEMENTS( posit ) EQ 0 THEN $
     posit = NotSet $
  ELSE IF state_posit[0] NE NotSet THEN $
     posit = NotSet
     
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

  IF N_ELEMENTS( channels ) EQ 0 THEN BEGIN
     channels = NotSet
     chan_mask = (NOT 0B)       ; set to select data of all channels
  ENDIF ELSE IF channels[0] EQ -1 THEN BEGIN
     channels = NotSet
     chan_mask = (NOT 0B)       ; set to select data of all channels
  ENDIF ELSE BEGIN
     chan_mask = 0B
     IF channels[0] NE 0 THEN BEGIN
        FOR nb = 0, N_ELEMENTS( channels )-1 DO BEGIN
           chan_mask = chan_mask + ISHFT( 1B, channels[nb]-1 )
        ENDFOR
     ENDIF
  ENDELSE

; select Detector source packets
  IF STRCMP( mds_type, 'DET' ) EQ 1 THEN $
     indx = WHERE( info_all.packet_id EQ 1, num_info ) $
  ELSE IF STRCMP( mds_type, 'AUX' ) EQ 1 THEN $
     indx = WHERE( info_all.packet_id EQ 2, num_info ) $
  ELSE IF STRCMP( mds_type, 'PMD' ) EQ 1 THEN $
     indx = WHERE( info_all.packet_id EQ 3, num_info ) $
  ELSE BEGIN
     num_info = 0
     MESSAGE, 'Wrong value for mds_type provided...', /INFO
  ENDELSE
  IF num_info LE 0 THEN RETURN, -1
  info_mds = info_all[indx]

; select packages on state ID
  IF state_id[0] NE NotSet THEN BEGIN
     nr_state = N_ELEMENTS( state_id )
     num_info = 0
     FOR nr = 0, nr_state-1 DO BEGIN
        indx = WHERE( info_mds.state_id EQ state_id[nr], count )
        IF count GT 0 THEN BEGIN
           IF num_info GT 0 THEN BEGIN
              indx_all = [indx_all, indx]
              num_info = num_info + count
           ENDIF ELSE BEGIN
              indx_all = indx
              num_info = count
           ENDELSE
        ENDIF
     ENDFOR
     IF num_info GT 0 THEN BEGIN
        info_mds = info_mds[indx_all[UNIQ(indx_all, SORT(indx_all))]]
        num_info = N_ELEMENTS( info_mds )
     ENDIF ELSE RETURN, -1
  ENDIF
  
; select packages on measurement category
  IF category[0] NE NotSet THEN BEGIN
     nr_cat = N_ELEMENTS( category )
     num_info = 0
     FOR nr = 0, nr_cat-1 DO BEGIN 
        indx = WHERE( info_mds.category EQ category[nr], count )
        IF count GT 0 THEN BEGIN
           IF num_info GT 0 THEN BEGIN
              indx_all = [indx_all, indx]
              num_info = num_info + count
           ENDIF ELSE BEGIN
              indx_all = indx
              num_info = count
           ENDELSE
        ENDIF
     ENDFOR
     IF num_info GT 0 THEN BEGIN
        info_mds = info_mds[indx_all[UNIQ(indx_all, SORT(indx_all))]]
        num_info = N_ELEMENTS( info_mds )
     ENDIF ELSE RETURN, -1
  ENDIF

; select packages within a time window
  IF period[0] NE NotSet THEN BEGIN
     SecInDay = 60d * 60d * 24d
     mjd_time = info_mds.mjd.days $
                + (info_mds.mjd.secnd + info_mds.mjd.musec / 1d6) / SecInDay 
     indx = WHERE( mjd_time GE period[0] AND mjd_time LE period[1], num_info )
     IF num_info GT 0 THEN $
        info_mds = info_mds[indx] $
     ELSE $
        RETURN, -1
  ENDIF

; select packages on channel ID
  IF channels[0] NE NotSet THEN BEGIN
     nr_chan = N_ELEMENTS( channels )
     nr_info = N_ELEMENTS( info_mds )

     num_info = 0
     FOR ni = 0, nr_info-1 DO BEGIN
        chan_ids = info_mds[ni].cluster.chan_id
        nc = 0
        REPEAT BEGIN
           indx = WHERE( chan_ids EQ channels[nc], count )
        ENDREP UNTIL ( count GT 0 OR ++nc GE nr_chan )

        IF count GT 0 THEN BEGIN
           IF num_info GT 0 THEN BEGIN
              indx_all = [indx_all, ni]
              num_info += count
           ENDIF ELSE BEGIN
              indx_all = ni
              num_info = count
           ENDELSE
        ENDIF
     ENDFOR
     IF num_info GT 0 THEN BEGIN
        info_mds = info_mds[indx_all[UNIQ(indx_all, SORT(indx_all))]]
        num_info = N_ELEMENTS( info_mds )
     ENDIF
  ENDIF

; selection on relative index
  IF posit[0] NE NotSet THEN BEGIN
     IF N_ELEMENTS( posit ) EQ 1 THEN BEGIN
        IF posit[0] LT 0 OR posit[0] GE num_info THEN BEGIN
           MESSAGE, 'Attempt to use subscript POSIT out of range.', /INFO
           status = 1
           RETURN, -1
        ENDIF
        num_info = 1
        info_mds = info_mds[posit[0]]
     ENDIF ELSE BEGIN
        IF posit[0] LT 0 OR posit[0] GE num_info THEN BEGIN
           MESSAGE, 'Attempt to use subscript POSIT[0] out of range.', /INFO
           status = 1
           RETURN, -1
        ENDIF
        IF posit[1] LT 0 OR posit[1] GE num_info THEN BEGIN
           MESSAGE, 'Attempt to use subscript POSIT[1] out of range.', /INFO
           status = 1
           RETURN, -1
        ENDIF
        IF posit[0] GT posit[1] THEN BEGIN
           MESSAGE, 'Subscript range values of the form low:high must be' $
                    + ' low <= high', /INFO
           status = 1
           RETURN, -1
        ENDIF
        info_mds = info_mds[posit[0]:posit[1]]
        num_info = N_ELEMENTS( info_mds )
     ENDELSE
  ENDIF

; selection on state index
  num_state = N_ELEMENTS(UNIQ(info_mds.stateIndex))
  indx_state = info_mds[UNIQ(info_mds.stateIndex)].stateIndex
  IF state_posit[0] NE NotSet THEN BEGIN
     nr_posit = N_ELEMENTS( state_posit )

     num_info = 0
     FOR nr = 0, nr_posit-1 DO BEGIN
        IF state_posit[nr] LT num_state THEN BEGIN
           indx = WHERE( info_mds.stateIndex EQ indx_state[state_posit[nr]],$
                         count )
           IF num_info EQ 0 THEN $
              indx_all = indx $
           ELSE $
              indx_all = [indx_all, indx]
           num_info += count
        ENDIF
     ENDFOR
     IF num_info GT 0 THEN $
        info_mds = info_mds[indx_all] $
     ELSE BEGIN
        num_state = 0
        indx_state = -1
        RETURN, -1
     ENDELSE
     num_state = N_ELEMENTS(UNIQ(info_mds.stateIndex))
     indx_state = info_mds[UNIQ(info_mds.stateIndex)].stateIndex
  ENDIF

  RETURN, info_mds
END
