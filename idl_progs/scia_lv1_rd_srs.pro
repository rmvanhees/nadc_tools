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
;       SCIA_LV1_RD_SRS
;
; PURPOSE:
;       read Sun Reference Spectrum records
;
; CATEGORY:
;       SCIA level 1b data
;
; CALLING SEQUENCE:
;       scia_lv1_rd_srs, dsd, srs, status=status
;
; INPUTS:
;       dsd :    structure for Data Set Descriptors
;
; OUTPUTS:
;       srs :    structure for Sun reference spectrum
;
; KEYWORD PARAMETERS:
;    doppler:    correct Sun reference spectrum D0 for doppler shift
;    status :    returns named variable with error status (0 = ok)
;
; EXAMPLES:
;       None
;
; REFERENCE:
;       SCIAMACHY Level 0 to 1b Processing
;	Input/Output Data Definition
;	Ref. ENV-TN-DLR-SCIA-0005
;	     Isue 5, 21 July 2000
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), February 2002
;       Modified:  RvH, 20 Jan 2003
;                    added more failure checking, and documentation
;       Modified:  RvH, 02 Jul 2013
;                    added function DOPPLER_CORR_SRS
;-
FUNCTION DOPPLER_CORR_SRS, srs

 srs_d0 = srs[0]
 dopplerCorr = 1 + srs_d0.dopp_shift / 500.
 wvlen_sun = dopplerCorr * srs_d0.wvlen_ref_spec

 indx = LINDGEN(1023)
 spec = FINDGEN(8192)
 FOR ch = 0, 7 DO BEGIN
    spec[indx] = INTERPOL(srs_d0.mean_ref_spec[indx], wvlen_sun[indx], $
                          srs_d0.wvlen_ref_spec[indx], /NAN)
    indx += 1024L
 ENDFOR

 RETURN, spec
END


PRO SCIA_LV1_RD_SRS, dsd, srs, doppler=doppler, status=status
  compile_opt idl2,logical_predicate,hidden

; initialize the return values
  status = 0
  srs = -1

; get index to data set descriptor 
  indx_dsd = WHERE( STRCOMPRESS(STRING(dsd[*].name), /REMOVE) $
                    EQ 'SUN_REFERENCE', count )
  IF count EQ 0 THEN $
     MESSAGE, ' FATAL, could not find keyword: ' + 'SUN_REFERENCE'

;read Data Set Descriptor records
  num_dsd = ULONG( SIZE( dsd, /N_ELEMENTS ))
  num_srs = dsd[indx_dsd].num_dsr
  IF num_srs GT 0 THEN BEGIN
     srs = replicate( {srs_scia}, num_srs )
     num = call_external( lib_name('libnadc_idl'), '_SCIA_LV1_RD_SRS', $
                          num_dsd, dsd, srs, /CDECL )

; check error status
     IF num NE num_srs THEN status = -1
  ENDIF ELSE $
     MESSAGE, ' no SUN_REFERENCE DSD records found', /INFORM
  
  IF KEYWORD_SET( DOPPLER ) THEN BEGIN
     ii = 0
     REPEAT BEGIN
        IF STRING(srs[ii].spec_id) EQ 'D0' THEN BREAK
     ENDREP UNTIL ii EQ N_ELEMENTS(SRS)

     IF ii LT N_ELEMENTS(SRS) THEN BEGIN
        srs[ii].mean_ref_spec = DOPPLER_CORR_SRS( srs[0] )
     ENDIF
  ENDIF

  RETURN
END
