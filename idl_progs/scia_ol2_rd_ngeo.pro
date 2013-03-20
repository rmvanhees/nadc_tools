;
; COPYRIGHT (c) 2003 - 2013 SRON (R.M.van.Hees@sron.nl)
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
;       SCIA_OL2_RD_NGEO
;
; PURPOSE:
;       read Geolocation Data Sets (Nadir)
;
; CATEGORY:
;       SCIA level 2 Off-line data product
;
; CALLING SEQUENCE:
;       SCIA_OL2_RD_NGEO, dsd, ngeo, status=status
;
; INPUTS:
;       dsd :    structure for Data Set Descriptors
;
; OUTPUTS:
;       ngeo :    structure for NADIR Geolocation Data sets
;
; KEYWORD PARAMETERS:
;    status :    returns named variable with error status (0 = ok)
;
; EXAMPLES:
;       None
;
; REFERENCE:
;       SCIAMACHY Level 1b to 2 Off-line Processing
;	Input/Output Data Definition
;	Ref. ENV-ID-DLR-SCI-2200-4
;	     Isue 4/A, 09 Aug 2002
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), January 2003
;-
PRO SCIA_OL2_RD_NGEO, dsd, ngeo, status=status
  compile_opt idl2,logical_predicate,hidden

; initialize the return values
  status = 0
  ngeo = -1

; get index to data set descriptor 
  indx_dsd = WHERE( STRCOMPRESS(STRING(dsd[*].name), /REMOVE) $
                  EQ 'GEOLOCATION_NADIR', count )
  IF count EQ 0 THEN $
     MESSAGE, ' FATAL, could not find keyword: ' + 'GEOLOCATION_NADIR'

;read Data Set Descriptor records
  num_dsd = ULONG( SIZE( dsd, /N_ELEMENTS ))
  num_geo = dsd[indx_dsd].num_dsr
  IF num_geo GT 0 THEN BEGIN
     ngeo = replicate( {ngeo_scia}, num_geo )
     num = call_external( lib_name('libnadc_idl'), '_SCIA_OL2_RD_NGEO', $
                          num_dsd, dsd, ngeo, /CDECL )

; check error status
     IF num NE num_geo THEN status = -1
  ENDIF ELSE $
     MESSAGE, ' no NADIR Geolocation records found', /INFORM

  RETURN
END
