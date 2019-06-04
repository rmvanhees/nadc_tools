;
; COPYRIGHT (c) 2010 - 2015 SRON (R.M.van.Hees@sron.nl)
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
;       Modified:  RvH, March 2015
;                Works again for nadc_tools v2.x; removed obsolete keywords
;-
FUNCTION GET_LV0_MDS_STATE, info_all, mds_type=mds_type, $
                            category=category, state_id=state_id, $
                            period=period, num_state=num_state, $
                            indx_state=indx_state
  compile_opt idl2,logical_predicate,hidden

; definition of SCIAMACHY related constants
  NotSet = -1

; initialize
  num_state = 0
  indx_state = LINDGEN(N_ELEMENTS(UNIQ(info_all.on_board_time)))

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

; select on source-packet type
  IF STRCMP( mds_type, 'DET' ) EQ 1 THEN $
     indx = WHERE( info_all.packet_type EQ 1, num_info ) $
  ELSE IF STRCMP( mds_type, 'AUX' ) EQ 1 THEN $
     indx = WHERE( info_all.packet_type EQ 2, num_info ) $
  ELSE IF STRCMP( mds_type, 'PMD' ) EQ 1 THEN $
     indx = WHERE( info_all.packet_type EQ 3, num_info ) $
  ELSE BEGIN
     num_info = 0
     MESSAGE, 'Wrong value for mds_type provided...', /INFO
  ENDELSE
  IF num_info LE 0 THEN RETURN, -1
  info_mds = info_all[indx]
  indx_state = indx_state[indx]

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
        indx_state = indx_state[indx_all[UNIQ(indx_all, SORT(indx_all))]]
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
        indx_state = indx_state[indx_all[UNIQ(indx_all, SORT(indx_all))]]
        num_info = N_ELEMENTS( info_mds )
     ENDIF ELSE RETURN, -1
  ENDIF

; select packages within a time window
  IF period[0] NE NotSet THEN BEGIN
     SecInDay = 60d * 60d * 24d
     mjd_time = info_mds.mjd.days $
                + (info_mds.mjd.secnd + info_mds.mjd.musec / 1d6) / SecInDay 
     indx = WHERE( mjd_time GE period[0] AND mjd_time LE period[1], num_info )
     IF num_info GT 0 THEN BEGIN
        info_mds = info_mds[indx]
        indx_state = indx_state[indx]
     ENDIF ELSE $
        RETURN, -1
  ENDIF

  num_state = N_ELEMENTS(UNIQ(info_mds.on_board_time))

  RETURN, info_mds
END
