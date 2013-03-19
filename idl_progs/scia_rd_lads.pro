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
;       SCIA_RD_LADS
;
; PURPOSE:
;       read Geolocation of the state records
;
; CATEGORY:
;       SCIA level 1b and 2 data
;
; CALLING SEQUENCE:
;       SCIA_RD_LADS, dsd, lads, status=status
;
; INPUTS:
;       dsd :    structure with the DSD records
;
; OUTPUTS:
;      lads :    structure with the Geolocation of States
;
; KEYWORD PARAMETERS:
;   status  :    returns named variable with error status (0 = ok)
;
; EXAMPLES:
;       
;
; REFERENCE:
;       
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), February 2002
;
;-
PRO SCIA_RD_LADS, dsd, lads, status=status
  compile_opt idl2,logical_predicate,hidden

  status = -1

; get index to data set descriptor 
; Note same data, but different names in level 1 and 2 files:
;  - level 1b: GELOCATION
;  - level 2 : STATE_GEOLOCATION (contains also keyword GELOCATION!)
  indx_dsd = WHERE( STRCOMPRESS(STRING(dsd[*].name), /REMOVE) $
                    EQ 'STATE_GEOLOCATION', count )
  IF count EQ 0 THEN $
     indx_dsd = WHERE( STRCOMPRESS(STRING(dsd[*].name), /REMOVE) $
                       EQ 'GEOLOCATION', count )
  IF count EQ 0 THEN BEGIN
     lads = 0
     MESSAGE, ' FATAL, could not find keyword: ' + '(STATE_)GEOLOCATION'
  ENDIF

;read Data Set Descriptor records
  num_dsd = ULONG( SIZE( dsd, /N_ELEMENTS ))
  num_lads = dsd[indx_dsd].num_dsr
  lads = replicate( {lads_scia}, num_lads )
  num = call_external( lib_name('libIDL_NADC'), '_SCIA_RD_LADS', $
                       num_dsd, dsd, lads, /CDECL )
; check error status
  IF num EQ num_lads THEN status = 0

  RETURN
END
