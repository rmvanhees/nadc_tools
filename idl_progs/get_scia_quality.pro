;
; COPYRIGHT (c) 2009 - 2013 SRON (R.M.van.Hees@sron.nl)
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
;	GET_SCIA_QUALITY
;
; PURPOSE:
;	obtain data quality of Sciamachy
;
; CATEGORY:
;	
;
; CALLING SEQUENCE:
;	Result = GET_SCIA_QUALITY( orbit, period=period )
;
; INPUTS:
;	orbit:	        absolute orbit number
;
; KEYWORD PARAMETERS:
;	period:	        first and last orbit affected by event
;
; OUTPUTS:
;	data quality: 
;                       0 = ok
;                       1 = decontamination
;                       2 = recovery ATC/TC affected
;                       3 = data unavailable
;
; PROCEDURE:
;	none
;
; EXAMPLE:
;	none
;
; MODIFICATION HISTORY:
; 	Written by:	R.M. van Hees (SRON), 25-09-2009
;-
FUNCTION GET_SCIA_QUALITY, orbit, period=period
  compile_opt idl2,logical_predicate,hidden

; obtain Sciamachy data quality
  orbit  = LONG( orbit )
  period_out = LONARR( 2 )
  result = call_external( lib_name('libnadc_idl'), '_GET_SCIA_QUALITY', $
                          orbit, period_out, /CDECL, /UI_VAL )

  IF N_ELEMENTS(period) GT 0 THEN period = period_out

  RETURN, result
END
