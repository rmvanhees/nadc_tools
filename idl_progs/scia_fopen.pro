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
;       SCIA_FOPEN
;
; PURPOSE:
;       general interface to open and close file, using a "C" FILE pointer
;
; CATEGORY:
;       file handling
;
; CALLING SEQUENCE:
;       stat = SCIA_FOPEN( flname )
;
; INPUTS:
;    flname :    string with the name of the file
;
; RETURNS:
;      stat :    error status (0 = ok)
;
; EXAMPLES:
;       stat = SCIA_FOPEN( flname_1 ) // opens the file with name flname_1
;       .
;       .   // read data sets from flname_1
;       .
;       stat = SCIA_FOPEN( flname_2 ) // closes flname_1, opens flname_2
;       .
;       .   // read data sets from flname_2
;       .
;       stat = SCIA_FCLOSE ()
;
; REFERENCE:
;       
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), February 2002
;       Modified:  RvH, 31 May 2002
;                    renamed module to scia_fopen
;       Modified:  RvH, 27 June 2002
;                    moved scia_fclose to a separate module
;-
FUNCTION SCIA_FOPEN, flname
  compile_opt idl2,hidden

  RETURN, call_external( lib_name('libnadc_idl'), 'OpenFile', flname, /CDECL )
END
