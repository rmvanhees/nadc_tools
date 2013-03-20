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
;       SCIA_LV0_RD_SPH
;
; PURPOSE:
;       read Specific Product Header from a PDS SCIAMACHY Level 0 product
;
; CATEGORY:
;       SCIAMACHY level 0
;
; CALLING SEQUENCE:
;       scia_lv0_rd_sph, mph, sph, status=status
;
; OUTPUTS:
;       sph :    structure with the SPH
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
PRO SCIA_LV0_RD_SPH, mph, sph, status=status
  compile_opt idl2,logical_predicate,hidden

; initialize the return values
  status = 0

;read Specific Product Header
  sph = {sph0_scia}
  num = call_external( lib_name('libnadc_idl'), '_SCIA_LV0_RD_SPH', $
                       mph, sph, /CDECL )

; check error status
  IF num NE 1 THEN status = -1

  RETURN
END
