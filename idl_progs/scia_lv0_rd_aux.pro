;
; COPYRIGHT (c) 2002 - 2013 SRON (R.M.van.Hees@sron.nl)
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
;       SCIA_LV0_RD_AUX
;
; PURPOSE:
;       read selected SCIAMACHY level 0 Auxiliary MDS
;
; CATEGORY:
;       SCIAMACHY level 0
;
; CALLING SEQUENCE:
;       SCIA_LV0_RD_AUX, info, mds_aux, period=period,
;                        category=category, state_id=state_id, $
;                        status=status
;
; INPUTS:
;      info :    structure holding info about MDS records
;
; OUTPUTS:
;   mds_aux :    structure with Auxiliary MDS records
;
; KEYWORD PARAMETERS:
;  category :    read only selected category of measurements (scalar or array)
;                (default or -1: all categories)
;  state_id :    read only selected states (scalar or array)
;                (default or -1: all states)
;    period :    read only MDS within a time-window (scalar or array)
;                  date must be given in decimal julian 2000 day
;    status :    returns named variable with error status (0 = ok)
;
; EXAMPLES:
;       read mds_aux.pmtc_hdr.pmtc_1 bitfields:
;           phase :  ISHFT((mds_aux.pmtc_hdr.pmtc_1 AND ISHFT(15U,4)),-4)
;           ndfm  :  mds_aux.pmtc_hdr.pmtc_1 AND ISHFT(3U,0)
;           ncwm  :  ISHFT((mds_aux.pmtc_hdr.pmtc_1 AND ISHFT(3U,14)),-14)
;           apsm  :  ISHFT((mds_aux.pmtc_hdr.pmtc_1 AND ISHFT(3U,12)),-12)
;           wls   :  ISHFT((mds_aux.pmtc_hdr.pmtc_1 AND ISHFT(3U,10)),-10)
;           sls   :  ISHFT((mds_aux.pmtc_hdr.pmtc_1 AND ISHFT(3U,8)),-8)
;
; REFERENCE:
;       Measurement data definition and format description for
;       SCIAMACHY, volume 14. PO-ID-DOR-SY-0032
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), February 2002
;       Modified:  RvH, February 2002
;                    added keyword posit
;       Modified:  RvH, February 2002
;                    added keyword period
;       Modified:  RvH, 22 March 2002
;                    debugged data set position selection
;       Modified:  RvH, 26 March 2002
;                    added keyword state_id & category
;       Modified:  RvH, 24 March 2003
;                    check: 0 <= posit[0] <= posit[1] < array-size
;       Modified:  RvH, 22 March 2010
;                    added keywords: state_posit, indx_state, num_state
;                    use new function GET_LV0_MDS_STATE
;-
;---------------------------------------------------------------------------
PRO SCIA_LV0_RD_AUX, info_all, mds_aux, category=category, $
                     state_id=state_id, period=period, $
                     indx_state=indx_state, num_state=num_state, $
                     status=status
  compile_opt idl2,logical_predicate,hidden

; initialize the return values
  indx_state = -1
  num_state = 0
  status = 0

  IF N_PARAMS() NE 2 THEN BEGIN
     MESSAGE, ' Usage: scia_lv0_rd_aux, info, mds_aux' $
              + ', category=category, state_id=state_id' $
              + ', indx_state=indx_state, num_state=num_state' $
              + ', period=period, status=status', /INFO
     status = -1
     RETURN
  ENDIF
  NotSet = -1

; select Auxiliary source packets
  info_aux = GET_LV0_MDS_STATE( info_all, mds_type='AUX', $
                                category=category, state_id=state_id, $
                                period=period, $
                                indx_state=indx_state, num_state=num_state )
  IF num_state LE 0 THEN BEGIN
     MESSAGE, 'No state-records found for given selection criteria', /INFO
     status = -1
     RETURN
  ENDIF
  num_aux = N_ELEMENTS( info_aux )
  IF num_aux LE 0 THEN RETURN

; allocate memory for the Auxiliary MDS records
  mds_aux = replicate( {mds0_aux}, num_aux )

; read Auxiliary source packets
  num = call_external( lib_name('libnadc_idl'), '_SCIA_LV0_RD_AUX', $
                       info_aux, ULONG(num_aux), mds_aux, /CDECL, /UL_VALUE )

; check error status
  IF num NE num_aux THEN status = -1

  RETURN
END
