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
;       SCIA_OL2_RD_CLD
;
; PURPOSE:
;       read Cloud and Aerosol Data sets from a level 2 Off-line Product
;
; CATEGORY:
;       SCIA level 2 off-line product
;
; CALLING SEQUENCE:
;       SCIA_OL2_RD_CLD, dsd, cld, status=status
;
; INPUTS:
;       dsd :    structure for Data Set Descriptors
;
; OUTPUTS:
;       cld :    structure for Cloud and Aerosol Data sets
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
;       Modified:  RvH, 24 March 2003
;                    added checks on allocated memory and array sizes
;       Modified:  RvH, 30 Januari 2009
;                    moved function SCIA_OL2_FREE_CLD to seperate module
;-
;---------------------------------------------------------------------------
PRO SCIA_OL2_RD_CLD, dsd, cld, status=status
  compile_opt idl2,logical_predicate,hidden

; initialize the return values
  status = 0
  cld = -1

; upper limit of the number of aerosol parameters
  MAX_AEROPAR_READ = 32L

; get index to data set descriptor 
  indx_dsd = WHERE( STRCOMPRESS(STRING(dsd[*].name), /REMOVE) $
                  EQ 'CLOUDS_AEROSOL', count )
  IF count EQ 0 THEN $
     MESSAGE, ' FATAL, could not find keyword: ' + 'CLOUDS_AEROSOL'

;read Data Set Descriptor records
  num_dsd = ULONG( SIZE( dsd, /N_ELEMENTS ))
  num_cld = dsd[indx_dsd].num_dsr
  IF num_cld EQ 0 THEN BEGIN
     MESSAGE, ' no Clouds/Aerosol records found', /INFORM
     cld = 0
     RETURN
  ENDIF
  cld = REPLICATE( {cld_scia_ol}, num_cld )
  data = FLTARR( num_cld * MAX_AEROPAR_READ )
  num = call_external( lib_name('libnadc_idl'), '_SCIA_OL2_RD_CLD', $
                       num_dsd, dsd, cld, data, /CDECL )

; check error status
  IF num NE num_cld THEN BEGIN
     status = -1 
     RETURN
  ENDIF

; check array sizes
  IF (WHERE( cld.numaeropars GT MAX_AEROPAR_READ ))[0] NE (-1) THEN BEGIN
     MESSAGE, 'Fatal: MAX_AEROPAR_READ too small, please increase its value', $
              /INFORM
     status = 1
     RETURN
  ENDIF

; dynamically allocate memory to store variable size arrays in output structure
  offs = 0L
  FOR nr = 0, num_cld-1 DO BEGIN
     IF cld[nr].numaeropars GT 0 THEN BEGIN
        cld[nr].aeropars = PTR_NEW( data[offs:offs+cld[nr].numaeropars-1] )
        offs += cld[nr].numaeropars
     ENDIF
  ENDFOR

  RETURN
END
