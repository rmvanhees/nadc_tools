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
;       GET_LV0_MDS_TIME
;
; PURPOSE:
;       obtain decimal julian date for level 0 MDS records
;
; CATEGORY:
;       SCIAMACHY level 0
;
; CALLING SEQUENCE:
;       jday = GET_LV0_MDS_TIME( mds [, status=status] )
;
; INPUTS:
;       mds :    an element or array of type structure
;                MDS0_AUX/MDS0_DET/MDS0_PMD
;
; KEYWORD PARAMETERS:
;    status :    returns named variable with error status (0 = ok)
;
; OUTPUTS:
;       modified (decimal) julian date for the year 2000.
;       Detector MDS:  time when a packet was assembled
;           T = T_ICU + bcps / 16.d               
;       Auxiliary MDS: time when a BCP was assembled
;           T[16,5] = T_ICU + bcps[16,5] / 16.d   
;       PMD MDS:       time when a PMD data packet was assembled
;           T[200] = T_ICU + bcps[200] / 16.d + (2/10^3 * delta_T-12.5)/10^3
;
;        Note 1) that this routine does not return the time for each
;        detector channel reading, NOR for each PMD measurement
;        reading
;        Note 2) the returned time is not corrected for the difference
;        between the "ICU on-board time at start of measurement" and
;        the BCPS counter reset (refered to as "RI").
;
; EXAMPLE:
;
; REFERENCE:
;       Measurement data definition and format description for
;       SCIAMACHY, volume 14. PO-ID-DOR-SY-0032
;
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), 15 April 2002
;       Modified:  RvH, 15 May 2002
;                    removed bug calculating time for MDS_AUX
;       Modified:  RvH, 4 October 2002
;                    removed bug calculating time for MDS_DET
;                    (now the BCPS of the channel data is used)
;       Modified:  RvH, 4 December 2002
;                    can also handle MDS0_INFO records   
;-
FUNCTION GET_LV0_MDS_TIME, mds, status=status
  compile_opt idl2,logical_predicate,hidden

; set return values
  status = 0
  jday = -1

  sec2day = (24.d * 60 * 60)
  IF TAG_NAMES(mds, /struct) EQ 'MDS0_DET' THEN BEGIN
     dsec = mds.isp.secnd + mds.isp.musec * 1d-6 $
            + mds.data_src[0].hdr.bcps / 16.d
     jday = mds.isp.days + dsec / sec2day
  ENDIF ELSE IF TAG_NAMES(mds, /struct) EQ 'MDS0_INFO' THEN BEGIN
     dsec = mds.mjd.secnd + mds.mjd.musec * 1d-6 + mds.bcps / 16.d
     jday = mds.mjd.days + dsec / sec2day
  ENDIF ELSE IF TAG_NAMES(mds, /struct) EQ 'MDS0_PMD' THEN BEGIN
     n_mds = N_ELEMENTS(mds)
     jday = REPLICATE( !VALUES.D_NAN, !nadc.numLv0PMDpacket, n_mds )
     FOR nr = 0, n_mds-1 DO BEGIN
        indx = WHERE( mds[nr].data_src.packet.sync EQ 'EEEE'xus, count )
        IF count GT 0 THEN BEGIN
           dsec = mds[nr].isp.secnd + mds[nr].isp.musec * 1d-6
           delay = mds[nr].data_src.packet[indx].mdi / 16.d $
                   + (mds[nr].data_src.packet[indx].time * 2 / 1d3 - 12.5) / 1d3
           jday[indx, nr] = mds[nr].isp.days + (dsec + delay) / sec2day
        ENDIF
     ENDFOR
  ENDIF ELSE IF TAG_NAMES(mds, /struct) EQ 'MDS0_AUX' THEN BEGIN
     n_mds = N_ELEMENTS(mds)
     jday = REPLICATE( !VALUES.D_NAN, !nadc.numLv0AuxBCP, !nadc.numLv0AuxPMTC, $
                       n_mds )
     FOR nr = 0, n_mds-1 DO BEGIN
        dsec = mds[nr].isp.secnd + mds[nr].isp.musec * 1d-6
        FOR np = 0, !nadc.numLv0AuxPMTC-1 DO BEGIN
           indx = WHERE( mds[nr].data_src[np].bcp.sync EQ 'DDDD'xus, count )
           IF count GT 0 THEN BEGIN
              jday[indx, np, nr] = mds[nr].isp.days $
                                   + (dsec $
                                      + mds[nr].data_src[np].bcp[indx].bcps $
                                      / 16.d) $
                                   / sec2day
           ENDIF
        ENDFOR
     ENDFOR
  ENDIF ELSE status = -1

  RETURN, jday
END
