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
;       SCIA_LV2_RD_CLD
;
; PURPOSE:
;       read Cloud and Aerosol Data sets
;
; CATEGORY:
;       SCIA level 2 NRT data products
;
; CALLING SEQUENCE:
;       SCIA_LV2_RD_CLD, dsd, cld, status=status
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
;       SCIAMACHY Level 1b to 2 NRT Processing
;	Input/Output Data Definition
;	Ref. ENV-TN-DLR-SCIA-0010
;	     Isue 3/B, 29 May 2000
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), February 2002
;       Modified:  RvH, 22 May 2002
;                    gave Offline and NRT parameters same names
;       Modified:  RvH, 20 Jan 2003
;                    added more failure checking, and documentation
;       Modified:  RvH, 30 Januari 2009
;                    moved function SCIA_LV2_FREE_CLD to seperate module
;-
;---------------------------------------------------------------------------
PRO SCIA_LV2_RD_CLD, dsd, cld, status=status
  compile_opt idl2,logical_predicate,hidden

; initialize the return values
  status = 0
  cld = -1

; upper limit of the number of PMD cloud flactions
  MAX_PMD_READ = 32L

; get index to data set descriptor 
  indx_dsd = WHERE( STRCOMPRESS(STRING(dsd[*].name), /REMOVE) $
                    EQ 'CLOUDS_AEROSOL', count )
  IF count EQ 0 THEN $
     MESSAGE, ' FATAL, could not find keyword: ' + 'CLOUDS_AEROSOL'

;read Data Set Descriptor records
  num_dsd = ULONG( SIZE( dsd, /N_ELEMENTS ))
  num_cld = dsd[indx_dsd].num_dsr
  IF num_cld GT 0 THEN BEGIN
     cld = REPLICATE( {cld_scia}, num_cld )
     data = FLTARR( num_cld * MAX_PMD_READ )
     num = call_external( lib_name('libIDL_NADC'), '_SCIA_LV2_RD_CLD', $
                          num_dsd, dsd, cld, data, /CDECL )

; check error status
     IF num NE num_cld THEN BEGIN
        status = -1 
        RETURN
     ENDIF

     offs = 0L
     FOR nr = 0, num_cld-1 DO BEGIN
        IF cld[nr].numpmd GT MAX_PMD_READ THEN BEGIN
           status = -1
           MESSAGE, ' MAX_PMD_READ too small, enlarge it', /INFORM
           RETURN
        ENDIF
        cld[nr].pmdcloudfrac = PTR_NEW( data[offs:offs+cld[nr].numpmd-1] )
        offs += cld[nr].numpmd
     ENDFOR
  ENDIF ELSE $
     MESSAGE, ' no Clouds/Aerosol records found', /INFORM

  RETURN
END
