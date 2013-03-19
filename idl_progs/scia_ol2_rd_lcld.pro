;
; COPYRIGHT (c) 2011 - 2013 SRON (R.M.van.Hees@sron.nl)
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
;       SCIA_OL2_RD_LCLD
;
; PURPOSE:
;       read Limb Clouds Data set
;
; CATEGORY:
;       SCIA level 2 Off-line data products
;
; CALLING SEQUENCE:
;       scia_ol2_rd_lcld, dsd, lcld, status=status
;
; INPUTS:
;       dsd :    structure for Data Set Descriptors
;
; OUTPUTS:
;      lcld :    Limb Clouds Data sets
;
; KEYWORD PARAMETERS:
;    status :    returns named variable with error status (0 = ok)
;
; EXAMPLES:
;       None
;
; REFERENCE:
;       Volume 15: Envisat-1 Product Specifications
;	
;	Ref. IDEAS-SER-IPF-SPE-0398
;	     Isue 3/L, 21 Jan 2010
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), January 2011
;-
;---------------------------------------------------------------------------
PRO SCIA_OL2_RD_LCLD, dsd, lcld, status=status
  compile_opt idl2,logical_predicate,hidden

; initialize the return values
  status = 0
  lcld = -1

; upper limit of several variable size entries in lcld_scia
  MAX_TANH  = 10
  MAX_CIR   = 2
  MAX_LEVEL = MAX_TANH * MAX_CIR
  MAX_PARA = 10

; get index to data set descriptor 
  indx_dsd = WHERE( STRCOMPRESS(STRING(dsd[*].name), /REMOVE) $
                    EQ 'LIM_CLOUDS', count )
  IF count EQ 0 THEN $
     MESSAGE, ' FATAL, could not find keyword: ' + 'LIM_CLOUDS'

;read Data Set Descriptor records
  num_lcld = dsd[indx_dsd].num_dsr
  IF num_lcld EQ 0 THEN BEGIN
     MESSAGE, ' no LCLD records of type LIM_CLOUDS found', /INFORM
     lcld = 0
     RETURN
  ENDIF
  lcld = replicate( {lcld_scia}, num_lcld )
  tanh = FLTARR( num_lcld * MAX_TANH )
  cir  = FLTARR( num_lcld * MAX_LEVEL )
  para = FLTARR( num_lcld * MAX_PARA )

  num_dsd = ULONG( SIZE( dsd, /N_ELEMENTS ))
  num = call_external( lib_name('libIDL_NADC'), '_SCIA_OL2_RD_LCLD', $
                       num_dsd, dsd, lcld, tanh, cir, para, /CDECL )

; check error status
  IF num NE num_lcld THEN BEGIN
     status = -1 
     RETURN
  ENDIF

; check array sizes
  IF (WHERE( lcld.num_tangent_hghts GT MAX_TANH ))[0] NE (-1) THEN BEGIN
     MESSAGE, 'Fatal: MAX_TANH too small, please increase its value', /INFO
     status = 1
     RETURN
  ENDIF
  IF (WHERE( lcld.num_cir GT MAX_CIR ))[0] NE (-1) THEN BEGIN
     MESSAGE, 'Fatal: MAX_CIR too small, please increase its value', /INFO
     status = 1
     RETURN
  ENDIF
  IF (WHERE( lcld.num_limb_para GT MAX_PARA ))[0] NE (-1) THEN BEGIN
     MESSAGE, 'Fatal: MAX_PARA too small, please increase its value', /INFO
     status = 1
     RETURN
  ENDIF

; dynamically allocate memory to store variable size arrays in output structure
  off_tanh = 0L
  off_cir = 0L
  off_para = 0L
  FOR nr = 0, num_lcld-1 DO BEGIN
     IF lcld[nr].num_tangent_hghts GT 0 THEN BEGIN
        lcld[nr].tangent_hghts = $
           PTR_NEW( tanh[off_tanh:off_tanh+lcld[nr].num_tangent_hghts-1] )
        off_tanh += lcld[nr].num_tangent_hghts
     ENDIF
     IF lcld[nr].num_tangent_hghts GT 0 AND lcld[nr].num_cir GT 0 THEN BEGIN
        num_level = lcld[nr].num_tangent_hghts * lcld[nr].num_cir
        lcld[nr].cir = $
           PTR_NEW( cir[off_cir:off_cir+num_level-1] )
        off_cir += num_level
     ENDIF
     IF lcld[nr].num_limb_para GT 0 THEN BEGIN
        lcld[nr].limb_para = $
           PTR_NEW( para[off_para:off_para+lcld[nr].num_limb_para-1] )
        off_para += lcld[nr].num_limb_para
     ENDIF
  ENDFOR

  RETURN
END
