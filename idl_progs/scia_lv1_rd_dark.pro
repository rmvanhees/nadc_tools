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
;       SCIA_LV1_RD_DARK
;
; PURPOSE:
;       read average of the Dark Measurements per State
;
; CATEGORY:
;       SCIA level 1b data
;
; CALLING SEQUENCE:
;       SCIA_LV1_RD_DARK, dsd, dark, status=status
;
; INPUTS:
;       dsd :    structure for Data Set Descriptors
;   
; OUTPUTS:
;      dark :    structure for average of the dark measurements
;
; KEYWORD PARAMETERS:
;    status :    returns named variable with error status (0 = ok)
;
; EXAMPLES:
;       None
;
; REFERENCE:
;       SCIAMACHY Level 0 to 1b Processing
;	Input/Output Data Definition
;	Ref. ENV-TN-DLR-SCIA-0005
;            Isue 5, 21 July 2000
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), February 2002
;       Modified:    check number of datasets RvH (SRON), March 2002
;
;-
PRO SCIA_LV1_RD_DARK, dsd, dark, status=status
  compile_opt idl2,logical_predicate,hidden

; initialize the return values
  status = 0
  dark = -1

; get index to data set descriptor 
  indx_dsd = WHERE( STRCOMPRESS(STRING(dsd[*].name), /REMOVE) $
                    EQ 'DARK_AVERAGE', count )
  IF count EQ 0 THEN $
     MESSAGE, ' FATAL, could not find keyword: ' + 'DARK_AVERAGE'

;read Data Set Descriptor records
  num_dsd = ULONG( SIZE( dsd, /N_ELEMENTS ))
  num_dark = dsd[indx_dsd].num_dsr
  IF num_dark GT 0 THEN BEGIN
     dark = replicate( {dark_scia}, num_dark )
     num = call_external( lib_name('libIDL_NADC'), '_SCIA_LV1_RD_DARK', $
                          num_dsd, dsd, dark, /CDECL )
; check error status
     IF num NE num_dark THEN status = -1 
  ENDIF ELSE $
     MESSAGE, ' no Average Dark measurments found', /INFORM

  RETURN
END
