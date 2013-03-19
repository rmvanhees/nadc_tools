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
;	SCIA_HL_LV1_MDS
;
; PURPOSE:
;	This function reads detector readouts for given stateID,
;	stateIndex and clusterID from a Sciamachy level 1 product
;
; CATEGORY:
;	NADC_TOOLS - SCIAMACHY - LEVEL 1
;
; CALLING SEQUENCE:
;	Result = SCIA_HL_LV1_MDS( flname, stateID, clusterID )
;
; INPUTS:
;      flname  :  Filename of the Sciamachy level 0 product
;
;      stateID :  state ID of data to be selected (scalar)
;
;    clusterID :  cluster ID of science data to be selected (scalar)
;
; KEYWORD PARAMETERS:
;  STATEINDEX :  index of state, use zero for the first occurrence of
;                this state (with ID stateID) in the product
;
;       ERROR :  add estimate the total relative accuracy on the
;                measured signal 
;
;     FRACPOL :  add fractional polarization data to output structure
;
;         PMD :  add PMD data to output structure
;
;  GEO_CORNER :  default is to write only the center coordinates from
;                the geolocation information. Use this keyword to get
;                the corner coordinates too.
;
; CALIBRATION :  string describing the calibration to be applied
;                (default or -1: no calibration of the MDS data)
;
;                0 : Memory Effect 
;                1 : Leakage Correction 
;                2 : PPG Correction 
;                3 : Etalon Correction 
;                4 : StrayLight Correction 
;                5 : WaveLength Calibration 
;                6 : Polarisation Sensitivity 
;                7 : Radiance Sensitivity 
;                8 : Division by Solar spectrum
;                9 : Bad/Dead pixel mask
;
;                This only works on level 1b products
;
;      status :  returns named variable with error status (0 = ok)
;
; OUTPUTS:
;	This function returns the detector readouts of states with
;	stateID equals stateID and clusterID equals clusterID. By
;	default the output structure contains the following elements:
;         * science: float array with (calibrated) cluster data
;         * wave:    float array with the cluster wavelength grid
;         * jday:    double scalar with modified (decimal) Julian date
;                    for the year 2000 
;         * lat:     float scalar with the central latitude coordinate
;         * lon:     float scalar with the central longitude coordinate
;         * sunz:    float scalar with the central Solar zenith angle
;         * pet:     float scalar with the pixel exposure time
;         * coaddf   byte scalar with the coadd-factor
;
;       keyword FRACPOL will add the following elements:
;         * Q:       float array with fractional polarization values Q
;         * U:       float array with fractional polarization values U
;         * wv_pol:  float array with represented wavelength for Q and U
;         * gdf:     float array with GDF parameters
;
;       keyword PMD will add the following elements:
;         * pmd:     float array with the integrated PMD values
;
; PROCEDURE:
;	<add text here>
;
; EXAMPLE:
;	<add examples here>
;
; MODIFICATION HISTORY:
; 	Written by:	Richard van Hees (SRON), 02 September 2006
;-
FUNCTION SCIA_HL_LV1_MDS, flname, stateID, clusterID, status=status, $
                          calib=calib, stateIndex=stateIndex, pmd=pmd, $
                          fracpol=fracpol, error=error
  compile_opt idl2,logical_predicate,hidden

  secperday = 24.d * 60 * 60

; check required parameters
  IF N_PARAMS() NE 3 THEN BEGIN
     MESSAGE, ' Usage: data = SCIA_HL_LV1_MDS( flname, stateID, clusterID' $
              + ', stateIndex=stateIndex, /FRACPOL, /PMD, /ERROR' $
              + ', calib=calib, status=status )', /INFO
     status = -1
     RETURN, -1
  ENDIF
  stateID = FIX(stateID[0])
  clusterID = FIX(clusterID[0])

; open file and read headers
  SCIA_HL_OPEN, flname, dsd, status=status
  IF status NE 0 THEN BEGIN
     NADC_ERR_TRACE, /No_Exit
     RETURN, -1
  ENDIF

; check if we are dealing with a level 1 product
  level = GET_SCIA_LEVEL()
  IF level NE '1B' AND level NE '1C' THEN BEGIN
     MESSAGE, ' can only process SCIAMACHY level 1 products', /INFO
     status = -1
     RETURN, -1
  ENDIF

; read States of the Product
  SCIA_LV1_RD_STATE, dsd, state, status=status
  IF status NE 0 THEN NADC_ERR_TRACE

; obtain info about requested state
  index = WHERE( state.state_id EQ stateID, stateCount )
  IF stateCount EQ 0 THEN BEGIN
     MESSAGE, ' Error: No states with requested ID found in this product', /INFO
     status = -1
     RETURN, -1
  ENDIF
  IF N_ELEMENTS( stateIndex ) GT 0 THEN BEGIN
     IF stateIndex GE stateCount THEN BEGIN
        MESSAGE, ' Error: stateIndex is out-of-range', /INFO
        status = -1
        RETURN, -1
     ENDIF
     state = state[index[stateIndex]]
  ENDIF ELSE $
     state = state[index]

; initialize
  numberState = N_ELEMENTS( state )
  clusterIndex = (WHERE( state[0].Clcon.id EQ clusterID, count ))[0]
  IF count EQ 0 THEN BEGIN
     MESSAGE, ' Error: requested clusterID not found requested state', /INFO
     status = -1
     RETURN, -1
  ENDIF
  clusterLength = state[0].Clcon[clusterIndex].length
  numberDsr = state[0].num_dsr * state[0].Clcon[clusterIndex].n_read
  intgTime = state[0].Clcon[clusterIndex].int_time
  index = (WHERE( state[0].intg_times EQ intgTime ))[0]
  polvIndex = 0
  FOR ii = 0, index-1 DO polvIndex += (state[0].num_polar)[ii]
  pmdPerIntg = 2 * intgTime

; make sure we calculate the wavelength grid and add option to
; calculate error estimate
  IF level EQ '1B' THEN BEGIN
     calib = (SIZE(calib, /TYPE) EQ 0 ) ? '5' : calib+',5'
     IF KEYWORD_SET( ERROR ) THEN calib += ',E'
  ENDIF

; read MDS data from file
  IF KEYWORD_SET( PMD ) AND KEYWORD_SET( FRACPOL ) THEN BEGIN
     IF level EQ '1B' THEN BEGIN
        SCIA_LV1_RD_MDS, dsd, mds, status=status, polv=polv, pmd=pmd, $
                         state_id=stateID, clusters=clusterID, calib=calib, $
                         state_posit=stateIndex
     ENDIF ELSE BEGIN
        SCIA_LV1C_RD_MDS, dsd, mds, status=status, polv=polv, pmd=pmd, $
                          state_id=stateID, clusters=clusterID, $
                          state_posit=stateIndex
     ENDELSE
  ENDIF ELSE IF KEYWORD_SET( PMD ) THEN BEGIN
     IF level EQ '1B' THEN BEGIN
        SCIA_LV1_RD_MDS, dsd, mds, status=status, pmd=pmd, $
                         state_id=stateID, clusters=clusterID, calib=calib, $
                         state_posit=stateIndex
     ENDIF ELSE BEGIN
        SCIA_LV1C_RD_MDS, dsd, mds, status=status, pmd=pmd, $
                          state_id=stateID, clusters=clusterID, $
                          state_posit=stateIndex
     ENDELSE
  ENDIF ELSE IF KEYWORD_SET( FRACPOL ) THEN BEGIN
     IF level EQ '1B' THEN BEGIN
        SCIA_LV1_RD_MDS, dsd, mds, status=status, polv=polv, $
                         state_id=stateID, clusters=clusterID, calib=calib, $
                         state_posit=stateIndex
     ENDIF ELSE BEGIN
        SCIA_LV1C_RD_MDS, dsd, mds, status=status, polv=polv, $
                          state_id=stateID, clusters=clusterID, $
                          state_posit=stateIndex
     ENDELSE
  ENDIF ELSE BEGIN
     IF level EQ '1B' THEN BEGIN
        SCIA_LV1_RD_MDS, dsd, mds, status=status, $
                         state_id=stateID, clusters=clusterID, calib=calib, $
                         state_posit=stateIndex
     ENDIF ELSE BEGIN
        SCIA_LV1C_RD_MDS, dsd, mds, status=status, $
                          state_id=stateID, clusters=clusterID, $
                          state_posit=stateIndex
     ENDELSE
  ENDELSE
  IF status NE 0 THEN BEGIN
     NADC_ERR_TRACE, /No_Exit
     RETURN, -1
  ENDIF

; allocate memory for output data structure
  struct = CREATE_STRUCT( 'science', fltarr(clusterLength), $
                          'wave', fltarr(clusterLength), $
                          'jday', 0.d, $
                          'lat', 0., $
                          'lon', 0., $
                          'sunz', 0., $
                          'pet', 0., $
                          'coaddf', 0b $
                        )

; add error estimate values when keyword 'ERROR' is set
  IF KEYWORD_SET( ERROR ) THEN $
     struct = CREATE_STRUCT( struct, 'error', fltarr(clusterLength) )

; add fractional polarisation values when keyword 'FRACPOL' is set
  IF KEYWORD_SET( FRACPOL ) THEN $
     struct = CREATE_STRUCT( struct, $
                             'Q', fltarr(12), $
                             'U', fltarr(12), $
                             'wv_pol', fltarr(13), $
                             'gdf', {GDF_PARA} )

; add integrated PMD values when keyword 'PMD' is set
  IF KEYWORD_SET( PMD ) THEN $
     struct = CREATE_STRUCT( struct, 'pmd', fltarr(7, pmdPerIntg) )

  data = REPLICATE( struct, numberState * numberDsr )

; destroy structure declaration
  struct = 0

; copy requested data from the MDS records
  nr = 0
  FOR ns = 0, numberState-1 DO BEGIN
     intg_day = (mds[ns].pet * mds[ns].coaddf) / secperday
     jday = mds[ns].mjd.days $
            + (mds[ns].mjd.secnd + mds[ns].mjd.musec * 1d-6) / secperday
     FOR nd = 0, numberDsr - 1 DO BEGIN
        data[nr].science = (*mds[ns].pixel_val)[*,nd]
        IF KEYWORD_SET( ERROR ) THEN $
           data[nr].error = (*mds[ns].pixel_err)[*,nd]
        data[nr].wave = (*mds[ns].pixel_wv)
        data[nr].jday = jday + nd * intg_day
        IF PTR_VALID( mds[ns].geoN ) THEN BEGIN
           data[nr].lat = (*mds[ns].geoN)[nd].center_coord.lat / 1e6
           data[nr].lon = (*mds[ns].geoN)[nd].center_coord.lon / 1e6
           data[nr].sunz = (*mds[ns].geoN)[nd].sun_zen_ang[1]
        ENDIF ELSE IF PTR_VALID( mds[ns].geoL ) THEN BEGIN
           data[nr].lat = (*mds[ns].geoL)[nd].tang_ground_point[1].lat / 1e6
           data[nr].lon = (*mds[ns].geoL)[nd].tang_ground_point[1].lon / 1e6
           data[nr].sunz = (*mds[ns].geoL)[nd].sun_zen_ang[1]
        ENDIF
        data[nr].coaddf = mds[ns].coaddf
        data[nr].pet = mds[ns].pet
        IF KEYWORD_SET( PMD ) THEN BEGIN
           data[nr].pmd = $
              (*pmd[ns].int_pmd)[*,(pmdPerIntg * nd):(pmdPerIntg * (nd+1) - 1)]
        ENDIF
        IF KEYWORD_SET( FRACPOL ) THEN BEGIN
           data[nr].Q = (*polv[ns].polv)[polvIndex + nd].Q
           data[nr].U = (*polv[ns].polv)[polvIndex + nd].U
           data[nr].wv_pol = (*polv[ns].polv)[polvIndex + nd].rep_wv
           data[nr].gdf = (*polv[ns].polv)[polvIndex + nd].gdf
        ENDIF
        nr++
     ENDFOR
  ENDFOR
  SCIA_LV1_FREE_MDS, mds
  IF KEYWORD_SET( PMD ) THEN SCIA_LV1_FREE_PMD, pmd 
  IF KEYWORD_SET( FRACPOL ) THEN SCIA_LV1_FREE_POLV, polV

; close file
  status = SCIA_FCLOSE()

  RETURN, data
END
