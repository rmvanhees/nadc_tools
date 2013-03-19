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
;       SCIA_LV2_RD_GEO
;
; PURPOSE:
;       read Geolocation Data Sets
;
; CATEGORY:
;       SCIA level 2 NRT data product
;
; CALLING SEQUENCE:
;       SCIA_LV2_RD_GEO, dsd, geo, status=status
;
; INPUTS:
;       dsd :    structure for Data Set Descriptors
;
; OUTPUTS:
;       geo :    structure for Geolocation Data sets
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
;       Modified:  RvH, 20 Jan 2003
;                    added more failure checking, and documentation
;-
PRO SCIA_LV2_RD_GEO, dsd, geo, status=status
  compile_opt idl2,logical_predicate,hidden

; initialize the return values
  status = 0
  geo = -1

; get index to data set descriptor 
  indx_dsd = WHERE( STRCOMPRESS(STRING(dsd[*].name), /REMOVE) $
                    EQ 'GEOLOCATION', count )
  IF count EQ 0 THEN $
     MESSAGE, ' FATAL, could not find keyword: ' + 'GEOLOCATION'

;read Data Set Descriptor records
  num_dsd = ULONG( SIZE( dsd, /N_ELEMENTS ))
  num_geo = dsd[indx_dsd].num_dsr
  IF num_geo GT 0 THEN BEGIN
     geo = replicate( {geo_scia}, num_geo )
     num = call_external( lib_name('libIDL_NADC'), '_SCIA_LV2_RD_GEO', $
                          num_dsd, dsd, geo, /CDECL )
; check error status
     IF num NE num_geo THEN status = -1
  ENDIF ELSE $
     MESSAGE, ' no Geolocation records found', /INFORM

  RETURN
END
