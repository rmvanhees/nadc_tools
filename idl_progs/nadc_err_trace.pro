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
;       NADC_ERR_TRACE
;
; PURPOSE:
;       Display error messages produced by underlaying C-routines
;
; CATEGORY:
;       error handling
;
; CALLING SEQUENCE:
;       NADC_ERR_TRACE, /No_Exit
;
; KEYWORD PARAMETERS:
;   No_Exit :    do not return control to the main program level
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
PRO NADC_ERR_TRACE, No_Exit=No_Exit
   COMPILE_OPT idl2, logical_predicate, hidden
   
   ; set template for temporary logfile
   log_name = '/tmp/nadc_log.XXXXXX'
   
   ; write error messages to this logfile
   void = call_external( LIB_NAME('libIDL_NADC'), 'Err_Trace', $
      log_name, /CDECL )
      
   ; read and display error massages using IDL interface
   line = ''
   OPENR, unit, log_name, /GET_LUN
   log_size = (FSTAT(unit)).SIZE
   IF log_size GT 0 THEN BEGIN
      WHILE (~ EOF(unit)) DO BEGIN
         READF, unit, line
         MESSAGE, line, /INFO
      END
   ENDIF
   FREE_LUN, unit
   
   ; remove temporary logfile
   FILE_DELETE, log_name
   
   IF KEYWORD_SET( No_Exit ) OR log_size EQ 0 THEN $
      RETURN $
   ELSE $
      RETALL
END
