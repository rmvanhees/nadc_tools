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
;       GOME_LV1_RD_FCD
;
; PURPOSE:
;       read Fixed Calibration Data Record from a PDS GOME Level 1b product
;
; CATEGORY:
;       GOME level 1b
;
; CALLING SEQUENCE:
;       gome_lv1_rd_fcd, fcd, status=status
;
; INPUTS:
;       fcd :    structure with Fixed Calibration Data
;
; OUTPUTS:
;       fcd :    structure with the Fixed Calibration Data Record
;
; KEYWORD PARAMETERS:
;   status  :    returns named variable with error status (0 = ok)
;
; EXAMPLES:
;       None
;
; REFERENCE:
;       Product Specification Document
;	of the GOME Data Processor
;	Ref. ER-PS-DLR-GO-0016
;	     Iss./Rev.3/C
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), March 2003
;       Modified:  RvH, 18 August 2003
;                    added function GOME_LV1_FREE_FCD
;       Modified:  RvH, 15 December 2004
;                    (NEW!) read the whole FCD structure
;       Modified:  RvH, 30 Januari 2009
;                    moved function GOME_LV1_FREE_FCD to seperate module
;-
;---------------------------------------------------------------------------
PRO GOME_LV1_RD_FCD, fsr, fcd, status=status
  compile_opt idl2,logical_predicate,hidden

  MAX_ALLOC = 50

; release previouse allocated memory
  GOME_LV1_FREE_FCD, fcd

;read file structure record
  fcd   = {fcd_gome}
  leak  = REPLICATE({lv1_leak}, MAX_ALLOC )
  hot   = REPLICATE({lv1_hot}, MAX_ALLOC )
  spec  = REPLICATE({lv1_spec}, MAX_ALLOC )
  calib = REPLICATE({lv1_calib}, MAX_ALLOC )
  num = call_external( lib_name('libnadc_idl'), '_GOME_LV1_RD_FCD', $
                       fsr, fcd, leak, hot, spec, calib, /CDECL )
; check error status
  IF num NE 1 THEN BEGIN
     status = -1
     fcd = 0
     RETURN
  ENDIF
  IF fcd.nleak GT 0 THEN fcd.leak = PTR_NEW( leak[0:fcd.nleak-1] )
  IF fcd.nhot  GT 0 THEN fcd.hot = PTR_NEW( hot[0:fcd.nhot-1] )
  IF fcd.nspec GT 0 THEN fcd.spec = PTR_NEW( spec[0:fcd.nspec-1] )
  IF fcd.nang  GT 0 THEN fcd.calib = PTR_NEW( calib[0:fcd.nang-1] )
  status = 0
  RETURN
END
