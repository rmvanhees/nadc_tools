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
;	SCIA_HL_LV0_MDS
;
; PURPOSE:
;	This function reads detector readouts for given stateID,
;	stateIndex and channelID from a Sciamachy level 0 product
;
; CATEGORY:
;	NADC_TOOLS - SCIAMACHY - LEVEL 0
;
; CALLING SEQUENCE:
;	Result = SCIA_HL_LV0_MDS( flname, stateID, clusID )
;
; INPUTS:
;      flname  :  Filename of the Sciamachy level 0 product
;
;      stateID :  state ID of data to be selected (scalar)
;
;       clusID :  cluster Index of science data to be selected (scalar)
;
; KEYWORD PARAMETERS:
;  STATEINDEX :  index of state, use zero for the first occurrence of
;                this state (with ID stateID) in the product
;
;         PMD :  add PMD data to output structure
;                - currently, the ouput only contains the collocated
;                  PMD readouts. This should be all PMD readouts
;                  within the integration time of the detector readout
;
;         AUX :  add Auxiliary data to the output structure
;
; CALIBRATION :  string describing the calibration to be applied
;                (default or -1: no calibration of the MDS data)
;
;                0 : Memory effect
;                    - coadded readouts are not corrected using PMD readouts
;                1 : Dark current correction
;                9 : Bad/Dead pixel mask
;
;      status :  returns named variable with error status (0 = ok)
;
; OUTPUTS:
;	This function returns the detector readouts of states with
;	state ID equals stateID and channel ID equals chanID. By
;	default the output structure contains the following elements:
;         * science: float array with (calibrated) channel data
;         * pixelID  unsigned int array with pixel IDs
;         * jday:    double scalar with modified (decimal) Julian date
;                    for the year 2000 
;         * pet:     float scalar with pixel exposure time (sec)
;         * clusID   byte scalar with cluster index
;         * coaddf   byte scalar with coadd-factor
;         * detTemp  float scalar with temperature of Detector block (K)
;
;       keyword PMD will add the following elements:
;         * pmd:     float array with the integrated PMD values
;         * pmdTemp: float scalar with temperature PMD block (K)
;
;       keyword AUX will add the following elements:
;         * obmTemp: float scalar with temperature of optical bench (K)
;         * asm:     float scalar with azimuth scan mirror position (degrees)
;         * esm:     float scalar with elevation scan mirror position (degrees)
;
; PROCEDURE:
;	<add text here>
;
; EXAMPLE:
;       +++++ read data of cluster 55 from all Nadir states in a file +++++
;	clus = scia_hl_lv0_mds( sciafl, 1, 55, calib='0,1,9' )
;       FOR ii = 2,7 DO BEGIN
;          clus_0 = scia_hl_lv0_mds( sciafl, ii, 55, calib='0,1,9' )
;          IF SIZE( clus_0, /TNAME ) EQ 'STRUCT' THEN clus = [clus,clus_0]
;       ENDFOR
;       clus = clus[SORT(clus.jday)]
;
; MODIFICATION HISTORY:
; 	Written by:	Richard van Hees (SRON), 10 October 2006
;       Modified:       implemented correction for the obital
;                       variation of darkcurrent [RvH (SRON), Oct 2006]
;       Modified:       bugfix in collocating asm/esm angles, 
;                       [RvH (SRON), June 2007]
;       Modified:       use SCIA_APPLY_MEMCORR and SCIA_APPLY_NLINCORR
;                       [RvH (SRON), Jan 2009]
;-
FUNCTION SCIA_HL_LV0_MDS, flname, stateID, clusID, status=status, $
                          calib=calib, memcorr=memcorr, nlcorr=nlcorr, $
                          stateIndex=stateIndex, pmd=pmd, aux=aux
  compile_opt idl2,logical_predicate,hidden

  secperday = 24.d * 60 * 60

; check required parameters
  IF N_PARAMS() NE 3 THEN BEGIN
     MESSAGE, ' Usage: data = SCIA_HL_LV0_MDS( flname, stateID, clusID' $
              + ', stateIndex=stateIndex, /PMD, /AUX, calib=calib ' $
              + ', status=status )', /INFO
     status = -1
     RETURN, -1
  ENDIF
  stateID = FIX(stateID[0])
  clusID = FIX(clusID[0])

; open file and read headers
  SCIA_HL_OPEN, flname, dsd, mph=mph, status=status
  IF status NE 0 THEN BEGIN
     NADC_ERR_TRACE, /No_Exit
     RETURN, -1
  ENDIF

; check if we are dealing with a level 0 product
  level = GET_SCIA_LEVEL()
  IF level NE '0' THEN BEGIN
     MESSAGE, ' can only process SCIAMACHY level 0 products', /INFO
     status = -1
     RETURN, -1
  ENDIF

; get cluster definition
  clusDef = GET_SCIA_CLUSDEF( stateID  )
  chanID = clusDef[clusID-1].chanID
  id = clusDef[clusID-1].clusID

; collect info about SCIAMACHY source packets
  sciafl = STRMID(flname, STRPOS( flname, '/', /REVERSE_SEARCH )+1)
  SCIA_LV0_RD_MDS_INFO, sciafl, dsd, info_all, status=status
  IF status NE 0 THEN NADC_ERR_TRACE, /No_Exit

; select (Detector readouts) info records on stateID
  indxInfo = WHERE( info_all.packet_id EQ 1 AND info_all.state_id EQ stateID, $
                    infoCount )
  IF infoCount EQ 0 THEN BEGIN
     MESSAGE, ' Error: No states with requested ID found in this product', /INFO
     status = -1
     RETURN, -1
  ENDIF

; select (Detector readouts) info records (further) on stateIndex 
  indxStateLast = UNIQ( info_all[indxInfo].mjd.days $
                        + info_all[indxInfo].mjd.secnd / secperday )
  indxStateFirst = SHIFT( indxStateLast, 1 ) + 1 &  indxStateFirst[0] = 0
  IF N_ELEMENTS( stateIndex ) GT 0 THEN BEGIN
     IF stateIndex GE N_ELEMENTS( indxStateLast ) THEN BEGIN
        MESSAGE, ' Error: stateIndex is out-of-range', /INFO
        status = -1
        RETURN, -1
     ENDIF
     info = info_all[indxInfo[indxStateFirst:indxStateLast]]
  ENDIF ELSE $
     info = info_all[indxInfo]

; select (Detector readouts) info records (further) on channel ID
  indx = WHERE( info.cluster.chan_id EQ chanID ) / 64   ; = MAX_CLUSTER
  info = info[indx[UNIQ(indx)]]

; read MDS data from file
  SCIA_LV0_RD_DET, info, mds_det, status=status
  IF status NE 0 THEN BEGIN
     NADC_ERR_TRACE, /No_Exit
     RETURN, -1
  ENDIF

; allocate memory for output data structure
  struct = CREATE_STRUCT( 'science', fltarr(clusDef[clusID-1].length), $
                          'pixel_ids', uintarr(clusDef[clusID-1].length), $
                          'jday'   , 0.d ,$
                          'pet'    , 0.  ,$
                          'clusID' , 0b  ,$
                          'coaddf' , 0b  ,$
                          'detTemp', 0.  )

; add integrated PMD values when keyword 'PMD' is set
  IF KEYWORD_SET( PMD ) THEN $
     struct = CREATE_STRUCT( struct   , $
                             'pmd'    , uintarr(2,7),$
                             'pmdTemp', 0. )

; add integrated Auxiliary data packets when keyword 'AUX' is set
  IF KEYWORD_SET( AUX ) THEN $
     struct = CREATE_STRUCT( struct   , $
                             'obmTemp', 0. ,$
                             'asm'    , 0. ,$
                             'esm'    , 0. )
  data = REPLICATE( struct, N_ELEMENTS( mds_det ) )
  struct = 0

; copy requested data from the MDS records
  FOR ii = 0, N_ELEMENTS( mds_det )-1 DO BEGIN
     idList = ISHFT(mds_det[ii].data_src.hdr.channel, -4) $
              AND (NOT ISHFT(NOT 0us, 4))
     
     chanIndx = WHERE( idList EQ chanID )
     numClus = ISHFT( mds_det[ii].data_src[chanIndx].hdr.channel, -8)
     clusIds = mds_det[ii].data_src[chanIndx].cluster[0:numClus-1].id
     clusIndx = WHERE( clusIds EQ id, count )
     IF count GT 0 THEN BEGIN
        data[ii].science = $
           FLOAT( *mds_det[ii].data_src[chanIndx].cluster[clusIndx].data )
        data[ii].pixel_ids = clusDef[clusID-1].start $
                             + UINDGEN(clusDef[clusID-1].length)
        data[ii].coaddf = $
           mds_det[ii].data_src[chanIndx].cluster[clusIndx].coaddf
        data[ii].clusID = BYTE( clusID )
     ENDIF
  ENDFOR
  data.jday    = GET_LV0_MDS_TIME( mds_det )
  data.detTemp = GET_LV0_MDS_HK( mds_det, channel=chanID, /temp )
  IF chanID LE 5 THEN BEGIN
     pet = GET_LV0_MDS_HK( mds_det, channel=chanID, /pet, virtual=virtual )
     IF virtual[0] EQ 0 THEN $
        data.pet = pet $
     ELSE BEGIN
        IF virtual[0] GT clusDef[clusID-1].start THEN $
           data.pet = REFORM(pet[0,*]) $
        ELSE $
           data.pet = REFORM(pet[1,*])
        data = data[WHERE( data.clusID NE 0 )]
     ENDELSE
  ENDIF ELSE $
     data.pet = GET_LV0_MDS_HK( mds_det, channel=chanID, /ir_pet )

; free allocated memory
  SCIA_LV0_FREE_DET, mds_det

; add PMD readouts
  IF KEYWORD_SET( PMD ) THEN BEGIN
     indxInfo = WHERE( info_all.packet_id EQ 3 $
                       AND info_all.state_id EQ stateID, $
                       infoCount )
     IF infoCount EQ 0 THEN BEGIN
        MESSAGE, ' Error: No states with requested ID found in this product', $
                 /INFO
        status = -1
        RETURN, -1
     ENDIF
     indxStateLast = UNIQ( info_all[indxInfo].mjd.days $
                           + info_all[indxInfo].mjd.secnd / secperday )
     indxStateFirst = SHIFT( indxStateLast, 1 ) + 1 &  indxStateFirst[0] = 0
     IF N_ELEMENTS( stateIndex ) GT 0 THEN BEGIN
        IF stateIndex GE N_ELEMENTS( indxStateLast ) THEN BEGIN
           MESSAGE, ' Error: stateIndex is out-of-range', /INFO
           status = -1
           RETURN, -1
        ENDIF
        info = info_all[indxInfo[indxStateFirst:indxStateLast]]
     ENDIF ELSE $
        info = info_all[indxInfo]

     SCIA_LV0_RD_PMD, info, mds_pmd, status=status
     IF status NE 0 THEN BEGIN
        NADC_ERR_TRACE, /No_Exit
        RETURN, -1
     ENDIF
     ss = GET_LV0_MDS_COLLO( data.jday, mds_pmd )
     data.pmd     = (mds_pmd.data_src.packet)[ss.i].data
     data.pmdTemp = (GET_LV0_MDS_HK( mds_pmd, /temp ))[ss.y]
  ENDIF

; add Auxiliary information
  IF KEYWORD_SET( AUX ) THEN BEGIN
     indxInfo = WHERE( info_all.packet_id EQ 2 $
                       AND info_all.state_id EQ stateID, $
                       infoCount )
     IF infoCount EQ 0 THEN BEGIN
        MESSAGE, ' Error: No states with requested ID found in this product', $
                 /INFO
        status = -1
        RETURN, -1
     ENDIF
     indxStateLast = UNIQ( info_all[indxInfo].mjd.days $
                           + info_all[indxInfo].mjd.secnd / secperday )
     indxStateFirst = SHIFT( indxStateLast, 1 ) + 1 &  indxStateFirst[0] = 0
     IF N_ELEMENTS( stateIndex ) GT 0 THEN BEGIN
        IF stateIndex GE N_ELEMENTS( indxStateLast ) THEN BEGIN
           MESSAGE, ' Error: stateIndex is out-of-range', /INFO
           status = -1
           RETURN, -1
        ENDIF
        info = info_all[indxInfo[indxStateFirst:indxStateLast]]
     ENDIF ELSE $
        info = info_all[indxInfo]

     SCIA_LV0_RD_AUX, info, mds_aux, status=status
     IF status NE 0 THEN BEGIN
        NADC_ERR_TRACE, /No_Exit
        RETURN, -1
     ENDIF
     ss = GET_LV0_MDS_COLLO( data.jday, mds_aux )
     data.obmTemp = (GET_LV0_MDS_HK( mds_aux, /obm ))[ss.y,ss.z]
     data.asm = (GET_LV0_MDS_HK( mds_aux, /asm ))[ss.x,ss.y,ss.z]
     data.esm = (GET_LV0_MDS_HK( mds_aux, /esm ))[ss.x,ss.y,ss.z]
  ENDIF

; return to calling program when calibration is not required
  IF N_ELEMENTS( calib ) EQ 0 THEN RETURN, data

; basic calibration of the level 0 detector readouts
  IF STRPOS( calib, '0' ) GE 0 THEN BEGIN
     pixel_ids = data[0].pixel_ids
     pixel_val = data.science
     IF chanID LT 6 THEN BEGIN
        SCIA_APPLY_MEMCORR, stateID, data[0].coaddf, data[0].pet, pixel_ids, $
                            pixel_val, MEMCORR=memcorr, /RESET
     ENDIF ELSE BEGIN
        SCIA_APPLY_NLINCORR, data[0].coaddf, pixel_ids, pixel_val, $
                             NLCORR=nlcorr
     ENDELSE
     data.science = pixel_val & pixel_val = 0
  ENDIF
  IF STRPOS( calib, '1' ) GE 0 THEN BEGIN
     PRINT, 'Perform dark correction'
     IF chanID EQ 8 THEN BEGIN
        orbphase = FLTARR( N_ELEMENTS( data ) )
        FOR ii = 0, N_ELEMENTS( data )-1 DO BEGIN
           orbphase[ii] = GET_SCIA_ORBITPHASE(data[ii].jday )
        ENDFOR
        SDMF_READ_DARK, mph.abs_orbit, mtbl, ao_out=ao_out, lc_out=lc_out, $
                        status=status, orbphase=orbphase
     ENDIF ELSE BEGIN
        SDMF_READ_DARK, mph.abs_orbit, mtbl, ao_out=ao_out, lc_out=lc_out, $
                        status=status
     ENDELSE
     indx_clus = data[0].pixel_ids
     FOR ii = 0, N_ELEMENTS( data )-1 DO BEGIN
        data[ii].science -= $
           data[ii].coaddf * (ao_out[indx_clus] + lc_out[indx_clus] * data[ii].pet)
     ENDFOR
  ENDIF
  IF STRPOS( calib, '9' ) GE 0 THEN BEGIN
     dbpmMask = GET_SDMF_DBPM( mph.abs_orbit )
     IF dbpmMask[0] NE -1 THEN BEGIN
        indx_clus = data[0].pixel_ids
        iindx = WHERE( dbpmMask[indx_clus] EQ 1b, count )
        IF count GT 0 THEN BEGIN
           FOR ii = 0, N_ELEMENTS( data )-1 DO BEGIN
              data[ii].science[iindx] = !Values.F_NAN
           ENDFOR
        ENDIF
     ENDIF
  ENDIF

  RETURN, data
END
