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
;       SCIA_LV1C_RD_CALOPT
;
; PURPOSE:
;       read the calibration options GADS to SciaL1C
;
; CATEGORY:
;       SCIA level 1c data
;
; CALLING SEQUENCE:
;       SCIA_LV1C_RD_CALOPT, dsd, calopt, status=status
;
; INPUTS:
;       dsd :    structure for Data Set Descriptors
;
; OUTPUTS:
;      calopt :  structure for the calibration options
;
; KEYWORD PARAMETERS:
;    status :    returns named variable with error status (0 = ok)
;
; EXAMPLES:
;       None
;
; REFERENCE:
;       
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), July 2002
;       Modified:  RvH, 11 December 2002
;                    rearranged elements of structure "cal_options"
;-
PRO SCIA_LV1C_RD_CALOPT, dsd, calopt, status=status
  compile_opt idl2,logical_predicate,hidden

; initialize the return values
  status = 0
  calopt = -1

; get index to data set descriptor 
  indx_dsd = WHERE( STRCOMPRESS(STRING(dsd[*].name), /REMOVE) $
                    EQ 'CAL_OPTIONS', count )
  IF count EQ 0 THEN $
     MESSAGE, ' FATAL, could not find keyword: ' + 'CAL_OPTIONS', /INFORM

;read Data Set Descriptor records
  num_dsd = ULONG( SIZE( dsd, /N_ELEMENTS ))
  num_calopt = dsd[indx_dsd].num_dsr
  calopt = replicate( {calopt_scia}, num_calopt )
  IF num_calopt GT 0 THEN BEGIN
     num = call_external( lib_name('libIDL_NADC'), '_SCIA_LV1C_RD_CALOPT', $
                          num_dsd, dsd, calopt, /CDECL )
; check error status
     IF num NE num_calopt THEN status = -1
  ENDIF ELSE $
     MESSAGE, ' no CAL_OPTIONS DSD records found', /INFORM

  RETURN
END
