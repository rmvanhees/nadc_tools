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
;       SCIA_LV0_MDS_TREND
;
; PURPOSE:
;       Extract level 0 MDS data, optionally correct Reticon memory
;       effect, Epitaxx non-linarity and/or normalize data to one
;       second integration time 
;
; CATEGORY:
;       SCIAMACHY level 0
;
; CALLING SEQUENCE:
;       data = SCIA_LV0_MDS_TREND( mds_det, channel, state_id=state_id,
;                                  memcorr=memcorr, nlcorr=nlcorr,
;                                  decoadd=decoadd, coadd=coadd, norm=norm, 
;                                  status=status )
; INPUTS:
;   mds_det :    Array of type Structure MDS0_DET
;   channel :    scalar specifying the channel to be extracted,
;                valid range = [1:8]
;
; KEYWORD PARAMETERS:
;  state_id :    read only data from a selected state
;                (default or -1: no selection state ID)
;   memcorr :    apply memory correction on Reticon data 
;                (channel 1-5), a named variable can be used to
;                speed-up a next call
;    nlcorr :    apply non-linearity correction on Epitaxx data
;                (channels 6-8), a named variable can be used to
;                speed-up a next call
;   decoadd :    divide data with the co-adding factor 
;                and return values for every PET
;     coadd :    divide data with the co-adding factor
;      norm :    normalize data to one second integration time 
;    status :    returns named variable with error status (0 = ok)
;
; OUTPUTS:
;       float array with reticon data corrected for memory effect
;
; SIDE EFFECTS:
;       Memory correction (Reference: SRON-SCIA-PhE-RP-011):
;       - the first readout is not corrected (no assuptions are made)
;       - it is assumed that the signal over integration time =>
;         signal/coadd is constant, and a good approximation for the
;         individual readouts in a coadding sequence
;       non-Linearity correction (Reference: SRON-SCIA-PhE-RP-013)
;
; PROCEDURE:
;
; EXAMPLE:
;       - extract all channel 2 data
;            data_out = SCIA_LV0_MDS_TREND( mds_det, 2 )
;  the return array is a float array of size [1024,N_ELEMENTS(mds_det)]
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), 08 April 2002
;       Modified:  RvH, 09 April 2002
;                    updated documentation
;       Modified:  RvH, 19 April 2002
;                    handle virtual channels correctly
;       Modified:  RvH, 15 May 2002
;                    handle array range under/overrun requests using
;                           keywords [posit] or [pixels] gracefully
;       Modified:  RvH, 06 May 2003
;                    more checks on mds_det and posit 
;                    do not modify return values of mds_det and posit
;       Modified:  RvH, 02 Dec 2003
;                    - applied all suggestions made by G. Lichtenberg
;                      ==> much improved memory correction
;                    - added non-linearity correction
;                    - memory correction no longer implies normalization!
;                    - changed keyword-parameters.
;       Modified:  RvH, 01 Mar 2004
;                    - add option to divide with the coadd-factor
;                    - handle empty selections gracefully   
;       Modified:  RvH, 03 March 2004
;                    fixed several bugs and did some code clean-ups
;       Modified:  RvH, 31 March 2004
;                    fixed bug: replaced REPLICATE with safer REPLICATE_INPLACE
;       Modified:  RvH, 28 April 2004
;                    One MDS with coaddf > 1 can be memory corrected
;       Modified:  RvH, 30 November 2004
;                    Speeding-up this code!
;       Modified:  RvH, 17 May 2007
;                    Replaced confusing error message
;       Modified:  RvH, 16 January 2009
;                    code clean-ups & bug-fixes
;                    usage of SCIA_APPLY_MEMCORR and SCIA_APPLY_NLINCORR
;       Modified:  RvH, 20 January 2009
;                    Adopted modified parameters for SCIA_APPLY_MEMCORR
;       Modified:  RvH, 15 November 2012
;                    Removed keywords: pixels, posit, align
;                    Add keyword: decoadd
;                    Replaced usage of GET_LV0_MSD_DATA by inline data-read
;-
FUNCTION SCIA_LV0_MDS_TREND, mds0_det, channel, state_id=state_id, $
                             memcorr=memcorr, nlcorr=nlcorr, norm=norm, $
                             decoadd=decoadd, coadd=coadd, status=status, $
                             orbitPhase=orbitPhase
  compile_opt idl2,logical_predicate,hidden

; check number of parameters
  IF N_PARAMS() NE 2 THEN BEGIN
     MESSAGE, ' Usage: SCIA_LV0_MDS_TREND, mds_det, channel' $
              + ' [, state_id=state_id]' $
              + ' [, memcorr=memcorr] [, nlcorr=nlcorr]' $
              + ' [, /decoadd] [, /coadd] [, /norm] [, status=status]', /INFO
     status = -1
     return, -1
  ENDIF

; check input structures
  IF TAG_NAMES( mds0_det, /struct) NE 'MDS0_DET' THEN BEGIN
     MESSAGE, 'input has to be a level 0 Detector MDS', /INFO
     status = -1
     RETURN, -1
  ENDIF
;
; interpret optional parameters
;
  IF N_ELEMENTS( state_id ) EQ 0 THEN state_id = -1
  IF state_id[0] EQ -1 THEN BEGIN
     indx = UNIQ( mds0_det.data_hdr.state_id, SORT(mds0_det.data_hdr.state_id) )
     IF N_ELEMENTS( indx ) GT 1 THEN BEGIN
        MESSAGE, 'Exit: MDS records do not have an unique state ID', /INFO
        status = -1
        RETURN, -1
     ENDIF
     state_id = mds0_det[indx].data_hdr.state_id
     mds_det = mds0_det
  ENDIF ELSE BEGIN
     state_id = BYTE( state_id[0] )
     indx = WHERE( mds0_det.data_hdr.state_id EQ state_id, count )
     IF count EQ 0 THEN BEGIN
        MESSAGE, 'no MDS records with state ID: ' $
                 + STRING( FORMAT='(i3)', state_id ), /INFO
        status = -1
        RETURN, -1
     ENDIF
     mds_det = mds0_det[indx]
  ENDELSE

  Do_COADD    = KEYWORD_SET( coadd )   ? !TRUE : !FALSE
  Do_DECOADD  = KEYWORD_SET( decoadd ) ? !TRUE : !FALSE
  Do_NORM     = KEYWORD_SET( norm )    ? !TRUE : !FALSE
  IF Do_DECOADD OR Do_NORM THEN Do_COADD = !TRUE

  DO_MemCorr  = (KEYWORD_SET(memcorr) OR ARG_PRESENT(memcorr)) ? !TRUE : !FALSE
  Do_nLinCorr = (KEYWORD_SET(nlcorr) OR ARG_PRESENT(nlcorr))   ? !TRUE : !FALSE
;
; initialisation of return values
;
  status = 0
  num_det = N_ELEMENTS( mds_det )
  data_out = FLTARR( !nadc.channelSize, num_det )
;
; obtain PET
;
  IF Do_MemCorr AND channel LT 6 THEN BEGIN
     FOR nd = 0, num_det-1 DO BEGIN
        pet = GET_LV0_MDS_HK( mds_det[nd], channel=channel, /PET, VIRTUAL=ipix )
        IF FINITE( pet[0] ) THEN break
     ENDFOR
  ENDIF
;
; Fill output array with detector counts of selected detector pixels
; The co-adding factor are stored in the variable "coaddf"
;
  clusID = 0U
  WHILE !TRUE DO BEGIN
     num_det_clus = 0U
     FOR nd = 0, num_det-1 DO BEGIN
        chanList = ISHFT( mds_det[nd].data_src.hdr.channel, -4 ) $
                   AND (NOT ISHFT(NOT 0us, 4))
        n_ch = WHERE( chanList EQ channel, num )
        IF num NE 1 THEN continue

        nc = ISHFT( mds_det[nd].data_src[n_ch].hdr.channel, -8 )
        n_cl = WHERE( mds_det[nd].data_src[n_ch].cluster[0:nc-1].id EQ clusID,$
                      num )
        IF num NE 1 THEN continue

        IF num_det_clus EQ 0 THEN BEGIN
           start = mds_det[nd].data_src[n_ch].cluster[n_cl].start
           length = mds_det[nd].data_src[n_ch].cluster[n_cl].length
        ENDIF
        num_det_clus += 1U
     ENDFOR
     IF num_det_clus EQ 0 THEN break

     indx_det = UINTARR( num_det_clus, /NOZERO )
     clus = FLTARR( length, num_det_clus, /NOZERO )
     coaddf = BYTARR( length, num_det_clus, /NOZERO )

     nr = 0U
     FOR nd = 0, num_det-1 DO BEGIN
        chanList = ISHFT( mds_det[nd].data_src.hdr.channel, -4 ) $
                   AND (NOT ISHFT(NOT 0us, 4))
        n_ch = WHERE( chanList EQ channel, num )
        IF num NE 1 THEN continue

        nc = ISHFT( mds_det[nd].data_src[n_ch].hdr.channel, -8 )
        n_cl = WHERE( mds_det[nd].data_src[n_ch].cluster[0:nc-1].id EQ clusID,$
                      num )
        IF num NE 1 THEN continue

        IF (~ PTR_VALID(mds_det[nd].data_src[n_ch].cluster[n_cl].data)) THEN $
           MESSAGE, "invalid pointer to data ... EXIT"
        
        indx_det[nr] = nd
        clus[*,nr] = FLOAT(*mds_det[nd].data_src[n_ch].cluster[n_cl].data)
        coaddf[*,nr] = $
           REPLICATE(mds_det[nd].data_src[n_ch].cluster[n_cl].coaddf, length)
        nr += 1U
     ENDFOR

     ; perform memory of non-linearity correction
     pixel_ids = start + UINDGEN(length)
     IF Do_MemCorr AND channel LT 6 THEN BEGIN
        IF ipix EQ 0 OR (start MOD !nadc.channelSize) LT ipix THEN $
           ipet = pet[0] $
        ELSE $
           ipet = pet[1]
        SCIA_APPLY_MEMCORR, state_id, coaddf, ipet, pixel_ids, clus, $
                            MEMCORR=memcorr
     ENDIF
     IF Do_nLinCorr AND channel GE 6 THEN BEGIN
        SCIA_APPLY_NLINCORR, coaddf, pixel_ids, clus, NLCORR=nlcorr
     ENDIF

     ; apply coadding correction factor
     IF Do_COADD THEN clus /= coaddf

      ; fill array data_out
     ids_mn = start MOD !nadc.channelSize
     ids_mx = ids_mn + length - 1U
     IF Do_DECOADD THEN $
        data_out[ids_mn:ids_mx,*] = REBIN( clus, length, num_det, /SAMPLE ) $
     ELSE $
        data_out[ids_mn:ids_mx,indx_det] = clus 

     clusID += 1U
  ENDWHILE

  IF n_elements(orbitPhase) GT 0 THEN BEGIN
     orbitPhase = DBLARR( num_det )
     FOR nd = 0, num_det-1 DO BEGIN
        julianDay = GET_LV0_MDS_TIME( mds_det[nd] )
        orbitPhase[nd] = GET_SCIA_ORBITPHASE( julianDay, eclipse_mode=1 )
     ENDFOR
  ENDIF
;
; apply normalisation to 1 second integration time
; Get Pixel exposure time: channel <= 2: pet = FLTARR( 2, num_det )
;                          channel > 2 : pet = FLTARR( num_det )
;
  IF Do_NORM THEN BEGIN
     IF channel LE 2 THEN BEGIN
        pet = GET_LV0_MDS_HK( mds_det, channel=channel, /pet, virtual=virtual )

        indx = WHERE( virtual NE 0, count )
        IF count EQ 0 THEN BEGIN
           FOR ipix = mn_pix, mx_pix DO BEGIN
              indx = WHERE( FINITE(data_out[ipix,*]), count )
              IF count GT 0 THEN BEGIN
                 data_out[ipix,indx] /= pet[indx]
              ENDIF
           ENDFOR
        ENDIF ELSE BEGIN
           FOR nd = 0, num_det_chan-1 DO BEGIN
              indx = WHERE( FINITE(data_out[*,nd]), count )
              IF count GT 0 THEN BEGIN
                 IF virtual[nd] EQ 0 THEN BEGIN
                    data_out[indx,nd] /= pet[nd]
                 ENDIF ELSE BEGIN
                    buff = indx < (virtual[nd] - 1)
                    indx_0 = buff[UNIQ(buff, sort(buff))]
                    data_out[indx_0,nd] /= pet[0,nd]
                    buff = indx > virtual[nd]
                    indx_1 = buff[UNIQ(buff, sort(buff))]
                    data_out[indx_1,nd] /= pet[1,nd]
                 ENDELSE
              ENDIF
           ENDFOR
        ENDELSE
     ENDIF ELSE BEGIN
        pet = GET_LV0_MDS_HK( mds_det, channel=channel, /PET )

        FOR ipix = mn_pix, mx_pix DO BEGIN
           indx = WHERE( FINITE(data_out[ipix,*]), count )
           IF count GT 0 THEN BEGIN
              data_out[ipix,indx] /= pet[indx]
           ENDIF
        ENDFOR
     ENDELSE
  ENDIF

  RETURN, REFORM( data_out )
END
