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
;       SCIA_LV2_RD_SQADS
;
; PURPOSE:
;       read Summary of Quality Flags per State
;
; CATEGORY:
;       SCIA level 2 NRT data product
;
; CALLING SEQUENCE:
;       SCIA_LV2_RD_SQADS, dsd, sqads, status=status
;
; INPUTS:
;       dsd :    structure for Data Set Descriptors
;
; OUTPUTS:
;     sqads :    structure for Summary of Quality flags per state
;
; KEYWORD PARAMETERS:
;    status :    returns named variable with error status (0 = ok)
;
; EXAMPLES:
;       None
;
; REFERENCE:
;       SCIAMACHY Level 1b to 2 NRT Processing
;	Input/Output Data Definition
;	Ref. ENV-TN-DLR-SCIA-0010
;	     Isue 3/B, 29 May 2000
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), February 2002
;       Modified by: RvH adjusted to Simulated Products v.4, March 2002
;
;-
PRO SCIA_LV2_RD_SQADS, dsd, sqads, status=status
  compile_opt idl2,logical_predicate,hidden

; initialize the return values
  status = 0
  sqads = -1

; get index to data set descriptor 
  indx_dsd = WHERE( STRCOMPRESS(STRING(dsd[*].name), /REMOVE) $
                    EQ 'SUMMARY_QUALITY', count )
  IF count EQ 0 THEN $
     MESSAGE, ' FATAL, could not find keyword: ' + 'SUMMARY_QUALITY'

;read Data Set Descriptor records
  num_dsd = ULONG( SIZE( dsd, /N_ELEMENTS ))
  num_sqads = dsd[indx_dsd].num_dsr
  sqads = replicate( {sqads2_scia}, num_sqads )
  num = call_external( lib_name('libIDL_NADC'), '_SCIA_LV2_RD_SQADS', $
                       num_dsd, dsd, sqads, /CDECL )
; check error status
  IF num NE num_sqads THEN status = -1

  RETURN
END
