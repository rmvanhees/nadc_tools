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
;       NADC_ERR_CLEAR
;
; PURPOSE:
;       clear current error stack of the underlaying C-routines
;
; CATEGORY:
;       error handling
;
; CALLING SEQUENCE:
;       NADC_ERR_CLEAR
;
; KEYWORD PARAMETERS:
;
; EXAMPLES:
;
; REFERENCE:
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), 28 June 2002
;
;-
PRO NADC_ERR_CLEAR
   COMPILE_OPT idl2, logical_predicate, hidden
   
   ; write error messages to this logfile
   void = call_external( lib_name('libnadc_idl'), 'Err_Clear', /CDECL )
   
   RETURN
END
