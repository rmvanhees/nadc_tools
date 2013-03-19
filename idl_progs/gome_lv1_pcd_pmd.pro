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
;       GOME_LV1_PCD_PMD
;
; PURPOSE:
;       grep/calibrate Earth PMD data from a GOME Level 1b product
;
; CATEGORY:
;       GOME level 1b
;
; CALLING SEQUENCE:
;       gome_lv1_pcd_pmd, fsr, pcd, calibration=calibration, status=status
;
; INPUTS:
;       fsr :    structure with File Structure Record
;       pcd :    structure with the Pixel Specific Calibration Records
;
; KEYWORD PARAMETERS:
;   status    :  returns named variable with error status (0 = ok)
;  calibration:  string describing the calibration to be applied
;                  (default or -1: no calibration of the PMD data)

;               L : correct for fixed-pattern noise and leakage current
;               P : correct signals for polarisation sensitivity
;               I : absolute radiance calibration (Watt/(s cm^2 sr nm)
;               U : unit conversion to photons/(s cm^2 sr nm)
;             all : apply default calibration (='LPI')
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
;       Written by:  Richard van Hees (SRON), April 2003
;-
FUNCTION Set_GOME_PMD_Calib, calib_str
  compile_opt idl2,logical_predicate,hidden

  GOME_CAL_LEAK   = '01'xus
  GOME_CAL_POLAR  = '040'xus
  GOME_CAL_INTENS = '080'xus
  GOME_CAL_UNIT   = '0400'xus

  GOME_CAL_EARTH  = (GOME_CAL_LEAK OR GOME_CAL_POLAR OR GOME_CAL_INTENS)

; initialize return value
  nr_char = STRLEN( calib_str )
  calib_mask = '0'xus
  calib_str = STRUPCASE( calib_str )

; handle special cases
  IF STRCMP( calib_str, 'NONE' ) THEN $
     return, '0'xus

  IF STRCMP( calib_str, 'ALL' ) THEN $
     return, GOME_CAL_EARTH

  ipos = STRPOS( calib_str, 'L' )
  IF ipos NE -1 THEN calib_mask = calib_mask OR GOME_CAL_LEAK
  ipos = STRPOS( calib_str, 'P' )
  IF ipos NE -1 THEN calib_mask = calib_mask OR GOME_CAL_POLAR
  ipos = STRPOS( calib_str, 'I' )
  IF ipos NE -1 THEN calib_mask = calib_mask OR GOME_CAL_INTENS
  ipos = STRPOS( calib_str, 'U' )
  IF ipos NE -1 THEN calib_mask = calib_mask OR GOME_CAL_UNIT

  RETURN, calib_mask
END

;---------------------------------------------------------------------------
PRO GOME_LV1_PCD_PMD, fsr, pcd, status=status, $
                      calibration=calibration, posit=posit
  compile_opt idl2,logical_predicate,hidden

; check required parameters
  IF N_PARAMS() NE 2 THEN BEGIN
     MESSAGE, ' Usage: GOME_LV1_PCD_PMD, fsr, pcd, status=status, ',  $
              'calibration=calibration'
     status = -1
     RETURN
  ENDIF

; obtain calibration mask
  calib_mask = '0'xus
  IF N_ELEMENTS( calibration ) EQ 0 THEN BEGIN
     calibration = -1
  ENDIF ELSE BEGIN
     calib_mask = Set_GOME_PMD_Calib( calibration )
  ENDELSE

; selection on relative index
  nr_pcd = FIX(N_ELEMENTS( pcd ))
  indx_pcd = INDGEN( nr_pcd )

; initialize the return values
  status = 0

; collect PMD records
  num = call_external( lib_name('libIDL_NADC'), '_GOME_LV1_PCD_PMD', $
                       calib_mask, fsr, nr_pcd, indx_pcd, pcd, /CDECL )

; check error status
  IF num NE nr_pcd THEN status = -1

  RETURN
END
