;
; COPYRIGHT (c) 2006 - 2013 SRON (R.M.van.Hees@sron.nl)
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
;       GOME_LV1_RD_SMBDR
;
; PURPOSE:
;       read/calibrate Sun Calibration Records from a GOME Level 1b product
;
; CATEGORY:
;       GOME level 1b
;
; CALLING SEQUENCE:
;       gome_lv1_rd_smbdr, nband, fsr, pcd, rec, 
;                        calibration=calibration, posit=posit, status=status
;
; INPUTS:
;       nband :  integer with number of spectral band [1a=0,1b,2a..]
;       fsr :    structure with File Structure Record
;       pcd :    structure with the Pixel Specific Calibration Records
;
; OUTPUTS:
;       rec :    structure with Spectral Band Records
;
; KEYWORD PARAMETERS:
;   status  :    returns named variable with error status (0 = ok)
;  calibration: string describing the calibration to be applied
;                (default or -1: no calibration of the MDS data)

;               L : correct for fixed-pattern noise and leakage current
;               A : correct band 1a for cross-talk of the Peltier coolers
;               F : correct signals for pixel-to-pixel variations
;               S : correct signals for stray-light
;               N : normalise the signals to 1 second integration time
;               P : correct signals for polarisation sensitivity
;               I : absolute radiance calibration (Watt/(s cm^2 sr nm)
;               U : unit conversion to photons/(s cm^2 sr nm)
;             all : apply default calibration (='LAFSNPIU')
;     posit :    scalar of type integer with the index of an PCD 
;                record specifying the Spectral Band Record to be
;                extracted, starting at zero
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
;-
FUNCTION Set_GOME_Calib, calib_str
  compile_opt idl2,logical_predicate,hidden

  GOME_CAL_LEAK   = '01'xus
  GOME_CAL_FPA    = '02'xus
  GOME_CAL_FIXED  = '04'xus
  GOME_CAL_STRAY  = '08'xus
  GOME_CAL_NORM   = '010'xus
  GOME_CAL_ALBEDO = '020'xus
  GOME_CAL_POLAR  = '040'xus
  GOME_CAL_INTENS = '080'xus
  GOME_CAL_JUMPS  = '0100'xus
  GOME_CAL_AGING  = '0200'xus
  GOME_CAL_UNIT   = '0400'xus

  GOME_CAL_EARTH  = (GOME_CAL_LEAK OR GOME_CAL_FPA OR GOME_CAL_FIXED $
                     OR GOME_CAL_STRAY OR GOME_CAL_NORM OR GOME_CAL_POLAR $
                     OR GOME_CAL_INTENS)

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
  ipos = STRPOS( calib_str, 'A' )
  IF ipos NE -1 THEN calib_mask = calib_mask OR GOME_CAL_FPA
  ipos = STRPOS( calib_str, 'F' )
  IF ipos NE -1 THEN calib_mask = calib_mask OR GOME_CAL_FIXED
  ipos = STRPOS( calib_str, 'S' )
  IF ipos NE -1 THEN calib_mask = calib_mask OR GOME_CAL_STRAY
  ipos = STRPOS( calib_str, 'N' )
  IF ipos NE -1 THEN calib_mask = calib_mask OR GOME_CAL_NORM
  ipos = STRPOS( calib_str, 'P' )
  IF ipos NE -1 THEN calib_mask = calib_mask OR GOME_CAL_POLAR
  ipos = STRPOS( calib_str, 'I' )
  IF ipos NE -1 THEN calib_mask = calib_mask OR GOME_CAL_INTENS
  ipos = STRPOS( calib_str, 'U' )
  IF ipos NE -1 THEN calib_mask = calib_mask OR GOME_CAL_UNIT

  RETURN, calib_mask
END

;---------------------------------------------------------------------------
PRO GOME_LV1_RD_SMBDR, nband, fsr, smcd, rec, status=status, $
                       calibration=calibration, posit=posit
  compile_opt idl2,logical_predicate,hidden

; check required parameters
  IF N_PARAMS() NE 4 THEN BEGIN
     MESSAGE, ' Usage: nband, fsr, smcd, rec, status=status, ',  $
              'calibration=calibration, posit=posit'
     status = -1
     RETURN
  ENDIF

; obtain calibration mask
  calib_mask = '0'xus
  IF N_ELEMENTS( calibration ) EQ 0 THEN BEGIN
     calibration = -1
  ENDIF ELSE BEGIN
     calib_mask = Set_GOME_Calib( calibration )
  ENDELSE

; selection on relative index
  indx_smcd = WHERE( smcd.indx_bands[nband] NE (-1), nr_smcd )
  IF N_ELEMENTS( posit ) GT 0 THEN BEGIN
     IF posit[0] LT 0 OR posit[0] GE nr_smcd THEN BEGIN
        MESSAGE, 'Attempt to use subscript POSIT out of range.', /INFO
        status = -1
        RETURN
     ENDIF
     indx_smcd = smcd[posit[0]].indx_bands[nband]
     IF indx_smcd EQ (-1) THEN BEGIN
        MESSAGE, 'Integration time of this band was not completed', /INFO
        status = 1
        RETURN
     ENDIF
     nr_smcd = 1
     indx_smcd = posit[0]
  ENDIF

; initialize the return values
  status = 0

;read Spectral Band Records
  nr_rec = nr_smcd
  rec = replicate( {rec_gome}, nr_rec )
  print, nband, nr_rec, nr_smcd
  num = call_external( lib_name('libnadc_idl'), '_GOME_LV1_RD_SMBDR', $
                       nband, calib_mask, fsr, nr_smcd, indx_smcd, $
                       smcd, rec, /CDECL )

; check error status
  IF num NE nr_rec THEN status = -1

  RETURN
END
