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
;       SCIA_RD_MPH
;
; PURPOSE:
;       read Main Product Header of the PDS SCIAMACHY product
;
; CATEGORY:
;       SCIAMACHY PDS headers
;
; CALLING SEQUENCE:
;       SCIA_RD_MPH, mph, status=status
;
; OUTPUTS:
;       mph :    structure with the Main Product Header
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
PRO SCIA_RD_MPH, mph, status=status
  compile_opt idl2,logical_predicate,hidden

; read Main Product Header
  mph = {mph_scia}
  num = call_external( lib_name('libnadc_idl'), '_ENVI_RD_MPH', $
                       mph, /CDECL )

; check error status
  IF num NE 1 THEN $
     status = -1 $
  ELSE $
     status = 0

  RETURN
END
