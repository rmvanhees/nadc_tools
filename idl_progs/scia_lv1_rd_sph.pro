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
;       SCIA_LV1_RD_SPH
;
; PURPOSE:
;       read Specific Product Header from a PDS SCIAMACHY Level 1b product
;
; CATEGORY:
;       SCIAMACHY level 1b
;
; CALLING SEQUENCE:
;       scia_lv1_rd_sph, mph, sph, status=status
;
; OUTPUTS:
;       sph :    structure with the SPH
;
; KEYWORD PARAMETERS:
;   status  :    returns named variable with error status (0 = ok)
;
; EXAMPLES:
;       None
;
; REFERENCE:
;       SCIAMACHY Level 0 to 1b Processing
;	Input/Output Data Definition
;	Ref. ENV-TN-DLR-SCIA-0005
;	     Isue 5, 21 July 2000
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), February 2002
;       Modified:  RvH, 20 Jan 2003
;                    added more documentation
;-
PRO SCIA_LV1_RD_SPH, mph, sph, status=status
  compile_opt idl2,logical_predicate,hidden

; initialize the return values
  status = 0

;read Specific Product Header
  sph = {sph1_scia}
  num = call_external( lib_name('libIDL_NADC'), '_SCIA_LV1_RD_SPH', $
                       mph, sph, /CDECL )
; check error status
  IF num NE 1 THEN status = -1

  RETURN
END
