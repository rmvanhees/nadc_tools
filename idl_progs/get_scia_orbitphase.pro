;
; COPYRIGHT (c) 2008 - 2013 SRON (R.M.van.Hees@sron.nl)
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
;	GET_SCIA_ORBITPHASE
;
; PURPOSE:
;	obtain orbitphase for given
;
; CATEGORY:
;	
;
; CALLING SEQUENCE:
;	Result = GET_SCIA_ORBITPHASE( julianDay )
;
; INPUTS:
;	orbit:	        absolute orbit number
;	julianDay:	julian Day (# days since 2000-01-01)
;
; KEYWORD PARAMETERS:
;	eclipse_mode:   TRUE  - orbit phase used for SDMF (v2.4)
;                       FALSE - orbit phase as used by ESA
;       status:         error flag (0 = ok)
;
; OUTPUTS:
;	Describe any outputs here.  For example, "This function returns the
;	foobar superflimpt version of the input array."  This is where you
;	should also document the return value for functions.
;
; PROCEDURE:
;	You can describe the foobar superfloatation method being used here.
;	You might not need this section for your routine.
;
; EXAMPLE:
;	Please provide a simple example here. An example from the PICKFILE
;	documentation is shown below. Please try to include examples that
;       do not rely on variables or data files that are not defined in
;       the example code. Your example should execute properly if typed
;       in at the IDL command line with no other preparation.
;
; MODIFICATION HISTORY:
; 	Written by:	
;-
FUNCTION GET_SCIA_ORBITPHASE, julianDay, eclipse_mode=eclipse_mode, $
                              status=status
  compile_opt idl2,logical_predicate,hidden

  eclipse_mode = (n_elements(eclipse_mode) EQ 0) ? 0B : BYTE(eclipse_mode)

; initialize the return values
  status = 0

;read ROE-records and determine orbitPhase
  number = N_ELEMENTS( julianDay )
  orbitPhase = FLTARR( number )
  num = call_external( lib_name('libIDL_NADC'), '_GET_SCIA_ROE_ORBITPHASE', $
                       eclipse_mode, number, julianDay, orbitPhase, /CDECL )

; check error status
  IF num LT 0 THEN BEGIN 
     status = -1
     return, -1
  ENDIF

  RETURN, orbitPhase
END
