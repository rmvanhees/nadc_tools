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
;	GET_LV1_GOME_SPEC
;
; PURPOSE:
;	combine GOME spectral band data into a spectrum
;
; CATEGORY:
;       GOME level 1b
;
; CALLING SEQUENCE:
;	Result = GET_LV1_GOME_SPEC( posit, fsr, fcd, pcd, status=status,
;                                   calibration=calibration, wave=wave )
;
; INPUTS:
;     posit :    scalar of type integer with the index of an PCD 
;                record specifying the Spectral Band Record to be
;                extracted, starting at zero
;     fsr   :    structure with File Structure Record
;     fcd   :    structure with Fixed Calibration Records
;     pcd   :    structure with the Pixel Specific Calibration Records
;
; KEYWORD PARAMETERS:
;     status :   returns named variable with error status (0 = ok)
; calibration:   string describing the calibration to be applied
;     wave   :   set this keyword to a named variable that will
;                contain the wavelength of the data points.
;
; OUTPUTS:
;	This function returns the combined spectral band data of band
;	1a, 1b, 2a, 2b, 3, and 4
;
; EXAMPLE:
;
; MODIFICATION HISTORY:
; 	Written by:	Richard van Hees, March 2003
;-
FUNCTION GET_LV1_GOME_SPEC, posit, fsr, fcd, pcd, $ 
                            calibration=calibration, wave=wave, status=status
  compile_opt idl2,logical_predicate,hidden

; initialize output arrays
  nr_pix = 0
  FOR nb = 0, 5 DO $
     nr_pix = nr_pix + (fcd.bcr[nb].end_pixel - fcd.bcr[nb].start_pixel + 1)
  print, nr_pix
  IF N_ELEMENTS( wave ) GT 0 THEN wave = FLTARR( nr_pix )
  data = FLTARR( nr_pix )
  REPLICATE_INPLACE, data, !VALUES.F_NAN

; read Band Data Records: 1a, 1b, 2a, 2b, 3, 4
  n_mn = 0
  FOR nb = 0, 5 DO BEGIN
     n_mx = n_mn + fcd.bcr[nb].end_pixel - fcd.bcr[nb].start_pixel
     print, n_mn, n_mx
     GOME_LV1_RD_BDR, nb, fsr, pcd, rec, $
                      calibration=calibration, posit=posit, status=status
     IF status NE 0 THEN $
        NADC_ERR_TRACE, /No_Exit $
     ELSE BEGIN
        help, rec[0].wave
        help, rec[0].data
        IF N_ELEMENTS( wave ) GT 0 THEN BEGIN
           wave[n_mn:n_mx] = $
              rec[0].wave[fcd.bcr[nb].start_pixel:fcd.bcr[nb].end_pixel]
        ENDIF
        data[n_mn:n_mx] = $
           rec[0].data[fcd.bcr[nb].start_pixel:fcd.bcr[nb].end_pixel]
     ENDELSE
     n_mn = n_mx + 1
  ENDFOR

  RETURN, data
END
