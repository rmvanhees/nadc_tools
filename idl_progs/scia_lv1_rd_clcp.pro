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
;       SCIA_LV1_RD_CLCP
;
; PURPOSE:
;       read Leakage Current Parameters (constant fraction) records
;
; CATEGORY:
;       SCIA level 1b data
;
; CALLING SEQUENCE:
;       SCIA_LV1_RD_CLCP, dsd, clcp, status=status
;
; INPUTS:
;       dsd :    structure for Data Set Descriptors
;
; OUTPUTS:
;      clcp :    structure for leakage current parameters (constant)
;
; KEYWORD PARAMETERS:
;    status :    returns named variable with error status (0 = ok)
;
; EXAMPLES:
;       None
;
; REFERENCE:
;       SCIAMACHY Level 0 to 1b Processing
;	Input/Output Data Definition
;	Ref. ENV-TN-DLR-SCIA-0005
;            Isue 5, 21 July 2000
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), February 2002
;       Modified:  RvH, 20 Jan 2003
;                    added more failure checking, and documentation
;-
PRO SCIA_LV1_RD_CLCP, dsd, clcp, status=status
  compile_opt idl2,logical_predicate,hidden

; initialize the return values
  status = 0
  clcp = -1

; get index to data set descriptor 
  indx_dsd = WHERE( STRCOMPRESS(STRING(dsd[*].name), /REMOVE) $
                    EQ 'LEAKAGE_CONSTANT', count )
  IF count EQ 0 THEN $
     MESSAGE, ' FATAL, could not find keyword: ' + 'LEAKAGE_CONSTANT'

;read Data Set Descriptor records
  num_dsd = ULONG( SIZE( dsd, /N_ELEMENTS ))
  num_clcp = dsd[indx_dsd].num_dsr
  IF num_clcp GT 0 THEN BEGIN
     clcp = replicate( {clcp_scia}, num_clcp )
     num = call_external( lib_name('libnadc_idl'), '_SCIA_LV1_RD_CLCP', $
                          num_dsd, dsd, clcp, /CDECL )

; check error status
     IF num NE num_clcp THEN status = -1 
  ENDIF ELSE $
     MESSAGE, ' no LEAKAGE_CONSTANT DSD records found', /INFORM

  RETURN
END
