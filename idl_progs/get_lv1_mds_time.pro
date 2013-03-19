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
;       GET_LV1_MDS_TIME
;
; PURPOSE:
;       obtain decimal julian date for level 1 MDS records
;
; CATEGORY:
;       SCIAMACHY level 1b/1c
;
; CALLING SEQUENCE:
;       jday = GET_LV1_MDS_TIME( [state,] mds [, status=status] )
;
; INPUTS:
;     [state :    structure for States of the product 
;               DEPRECIATED, DO NOT USE!]
;       mds :    a MDS1C_SCIA structure
;
; KEYWORD PARAMETERS:
;    status :    returns named variable with error status (0 = ok)
;
; OUTPUTS:
;       modified (decimal) julian date for the year 2000.
;
; EXAMPLE:
;
; REFERENCE:
;       SCIAMACHY Level 0 to 1b Processing
;       Input/Output Data Definition
;       Ref. ENV-TN-DLR-SCIA-0005
;            Isue 5, 21 July 2000
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), 07 November 2002
;       Modified:  RvH, 21 Jun 2005
;                    use of parameter "state" is depreciated
;       Modified:  RvH, 14 Apr 2004
;                    small bugfixes added more error messages
;-
FUNCTION GET_LV1_MDS_TIME, param1, param2, status=status
  compile_opt idl2,logical_predicate,hidden

; set some constants
  NotSet = -1
  sec2day = (24.d * 60 * 60)

; set return values
  status = 0
  jday = NotSet

  IF N_PARAMS() EQ 1 THEN BEGIN
     state = -1
     mds = param1[0]
  ENDIF ELSE IF N_PARAMS() EQ 2 THEN BEGIN
     state = param1
     mds = param2[0]
  ENDIF ELSE BEGIN
     MESSAGE, ' Usage: Result = GET_LV1_MDS_TIME( state, mds, status=status )',$
              /INFO
     status = -1
     RETURN, jday    
  ENDELSE

  IF TAG_NAMES(mds, /struct) NE 'MDS1C_SCIA' THEN BEGIN
     MESSAGE, 'Exit: Not a level 1c MDS structure as input', /INFO
     status = -1
     RETURN, jday
  ENDIF
    
; find corresponding state record
  IF  SIZE( state, /TNAME ) EQ 'STRUCT' THEN BEGIN
     indx = WHERE( state.mjd.days EQ mds.mjd.days, count )
     IF count GT 0 THEN BEGIN
        indx = WHERE( state.mjd.secnd EQ mds.mjd.secnd, count )
        IF count GT 0 THEN $
           indx = WHERE( state.mjd.musec EQ mds.mjd.musec, count )
     ENDIF
     IF count EQ 0 THEN BEGIN
        MESSAGE, 'Fatal: state and mds records are not of the same file', /INFO
        status = -1
        RETURN, jday
     ENDIF

     intg_time = state[indx].Clcon[mds.clus_id-1].int_time / 16.d
  ENDIF ELSE BEGIN
     intg_time = mds.pet * mds.coaddf
  ENDELSE
  dsec = mds.mjd.secnd + mds.mjd.musec * 1d-6 $
         + intg_time * FINDGEN(mds.num_obs)
  jday = mds.mjd.days + dsec / sec2day

  RETURN, jday
END

