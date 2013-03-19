;
; COPYRIGHT (c) 2003 - 2013 SRON (R.M.van.Hees@sron.nl)
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
;       GOME_LV1_RD_FSR
;
; PURPOSE:
;       read File Structure Record from a PDS GOME Level 1b product
;
; CATEGORY:
;       GOME level 1b
;
; CALLING SEQUENCE:
;       gome_lv1_rd_fsr, fsr, status=status
;
; OUTPUTS:
;       fsr :    structure with the File Structure Record
;
; KEYWORD PARAMETERS:
;   status  :    returns named variable with error status (0 = ok)
;
; EXAMPLES:
;       None
;
; REFERENCE:
;       Product Specification Document
;	of the GOME Data Processor
;	Ref. ER-PS-DLR-GO-0016
;	     Iss./Rev.3/C
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), March 2003
;-
PRO GOME_LV1_RD_FSR, fsr, status=status
  compile_opt idl2,logical_predicate,hidden

; initialize the return values
  status = 0

;read File Structure Record
  fsr = {fsr1_gome}
  num = call_external( lib_name('libIDL_NADC'), $
                       '_GOME_LV1_RD_FSR', fsr, /CDECL )

; check error status
  IF num NE 1 THEN status = -1

  RETURN
END
