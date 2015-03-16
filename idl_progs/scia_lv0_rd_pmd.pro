;
; COPYRIGHT (c) 2002 - 2015 SRON (R.M.van.Hees@sron.nl)
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
;       SCIA_LV0_RD_PMD
;
; PURPOSE:
;       read selected SCIAMACHY level 0 PMD MDS
;
; CATEGORY:
;       SCIAMACHY level 0
;
; CALLING SEQUENCE:
;       SCIA_LV0_RD_PMD, info, mds_pmd, period=period,
;                        category=category, state_id=state_id, $
;                        status=status 
;
; INPUTS:
;      info :    structure holding info about MDS records
;
; OUTPUTS:
;   mds_pmd :    structure with PMD MDS records
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
;       
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
;       Modified:  RvH, March 2015
;                Works again for nadc_tools v2.x; removed obsolete keywords
;-
;---------------------------------------------------------------------------
PRO SCIA_LV0_RD_PMD, info_all, mds_pmd, category=category, $
                     state_id=state_id, period=period, $
                     indx_state=indx_state, num_state=num_state, $
                     status=status
  compile_opt idl2,logical_predicate,hidden

; initialize the return values
  indx_state = -1
  num_state = 0
  status = 0

  IF N_PARAMS() NE 2 THEN BEGIN
     MESSAGE, ' Usage: scia_lv0_rd_pmd, info, mds_pmd' $
              + ', category=category, state_id=state_id' $
              + ', period=period, indx_state=indx_state' $
              + ', num_state=num_state, status=status', /INFO
     status = -1
     RETURN
  ENDIF
  NotSet = -1

; select PMD source packets
  info_pmd = GET_LV0_MDS_STATE( info_all, mds_type='PMD', $
                                category=category, state_id=state_id, $
                                period=period, $
                                indx_state=indx_state, num_state=num_state )
  IF num_state LE 0 THEN BEGIN
     MESSAGE, 'No state-records found for given selection criteria', /INFO
     status = -1
     RETURN
  ENDIF
  num_pmd = N_ELEMENTS( info_pmd )
  IF num_pmd LE 0 THEN RETURN

; allocate memory for the PMD MDS records
  mds_pmd = replicate( {mds0_pmd}, num_pmd )

; process PMD source packets
  num = call_external( lib_name('libnadc_idl'), '_SCIA_LV0_RD_PMD', $
                       info_pmd, ULONG(num_pmd), mds_pmd, /CDECL, /UL_VALUE )

; check error status
  IF num NE num_pmd THEN status = -1

  RETURN
END
