;
; COPYRIGHT (c) 2013 SRON (R.M.van.Hees@sron.nl)
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
;	SCIA_RD_MFACTOR
;
; PURPOSE:
;	obtain M-factor values (Ife Bremen)
;
; CALLING SEQUENCE:
;	SCIA_RD_MFACTOR, mftype, sensing_start, mfactor
;
; INPUTS:
;        mftype :  a string holding one of M_CAL, M_DL, M_DN
;
; sensing_start :  a string holding the sensing start taken from MPH
;
;
; OUTPUTS:
;       mfactor :  float-array holding mfactor, size [8192]
;
;
; SIDE EFFECTS:
;	Use environment variable SCIA_MFACTOR_DIR to use an alternative
;	directory with the M-factor files. 
;       Default: /SCIA/share/nadc_tools/m-factor_07.01
;
; EXAMPLE:
;	Please provide a simple example here. An example from the PICKFILE
;	documentation is shown below. Please try to include examples that
;       do not rely on variables or data files that are not defined in
;       the example code. Your example should execute properly if typed
;       in at the IDL command line with no other preparation.
;
;	Create a PICKFILE widget that lets users select only files with 
;	the extensions 'pro' and 'dat'.  Use the 'Select File to Read' title 
;	and store the name of the selected file in the variable F.  Enter:
;
;		F = PICKFILE(/READ, FILTER = ['pro', 'dat'])
;
; MODIFICATION HISTORY:
; 	Written by:	R.M. van Hees (SRON), July (2013)
;-
PRO SCIA_RD_MFACTOR, mftype, sensing_start, mfactor
 compile_opt idl2,logical_predicate,hidden

 IF mftype NE 'M_CAL' AND mftype NE 'M_DL'  AND mftype NE 'M_DN' THEN $
    MESSAGE, 'Unknown mftype requested'

 mfactor = FLTARR(8192)

 IF TYPENAME(sensing_start) EQ 'BYTE' THEN BEGIN
    res = call_external( lib_name('libnadc_idl'), '_SCIA_RD_MFACTOR', $
                         mftype, STRING(sensing_start), mfactor, /CDECL )
 ENDIF ELSE IF TYPENAME(sensing_start) EQ 'STRING' THEN BEGIN
    res = call_external( lib_name('libnadc_idl'), '_SCIA_RD_MFACTOR', $
                         mftype, sensing_start, mfactor, /CDECL )
 ENDIF ELSE BEGIN
    MESSAGE, 'FATAL: invalid sensing start given' 
 ENDELSE

 RETURN
END
