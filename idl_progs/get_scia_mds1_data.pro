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
;       GET_SCIA_MDS1_DATA
;
; PURPOSE:
;       read/calibrate MDS science data of a channel from one or more states
;
; CATEGORY:
;       SCIA level 1b/1c data
;
; CALLING SEQUENCE:
;       data = GET_SCIA_MDS1_DATA( dsd, stateID, chanID, meta=meta, $
;                                  calib=calib, stateIndex=stateIndex, $
;                                  norm=norm, status=status )
;
; INPUTS:
;         dsd :    structure for Data Set Descriptors
;     stateID :    read selected state (scalar)
;      chanID :    channel number of science data to be returned (scalar)
;
; KEYWORD PARAMETERS:
;        meta :  named variable which contains on return the Julian
;                day and gelocation information of the data
;  stateIndex :  named variable which contains the indices to the
;                original state records as defined in the level 1b/1c
;                product
; calibration :  string describing the calibration to be applied
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
;      norm   : default is to normalize the data to the shortest
;               integration time in the state for the requested
;               channel. However, setting this keyword will return BU/s
;      status : named variable which contains the error status (0 = ok)
;
; SIDE EFFECTS:
;       None
;
; ENVIRONMENT VARIABLE "SCIA_CORR_LOS":
;       The values of the level 1b line-of-sight zenith angles are
;       always larger than zero, and the azimuth angle jumps with 180
;       degrees while scanning through nadir. 
;       Setting the environment variable "SCIA_CORR_LOS" to one will
;       modify these values as follows: removing the jump in the
;       azimuth angles and returns negative zenith angles, when the
;       original azimuth angle was larger than 180 degree.
;
; EXAMPLES:
;       None
;
; REFERENCE:
;       SCIAMACHY Level 0 to 1b Processing
;       Input/Output Data Definition
;       Ref. ENV-TN-DLR-SCIA-0005
;            Isue 5, 21 July 2000
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), February 2007
;       Modified:  RvH, 06 December 2007
;                    added timestamp and geolocation record keyword (meta)
;                    fixed several bugs
;       Modified:  RvH, 06 December 2007
;                    more checks build in for corrupted/in-complete products
;-
;---------------------------------------------------------------------------
FUNCTION GET_SCIA_MDS1_DATA, dsd, stateID, chanID, status=status, $
                             calib=calib, stateIndex=stateIndex, $
                             meta=meta, norm=norm, noPMD=noPMD
  compile_opt idl2,logical_predicate,hidden

; definition of SCIAMACHY related constants
  NotSet = -1

; initialisation of some returned variables
  data_out = -1
  status = 0

; check level of the Sciamachy product
  scia_level = GET_SCIA_LEVEL()
  IF scia_level NE '1B' AND scia_level NE '1C' THEN BEGIN
     MESSAGE, 'FATAL: Input file is not a valid level 1B/1C product', /INFO
     IF ARG_PRESENT( meta ) THEN meta = -1
     status = -1
     RETURN, -1
  ENDIF
  is_level_1c = (scia_level EQ '1C') ? 1 : 0

; check required parameters
  IF N_PARAMS() NE 3 THEN BEGIN
     MESSAGE, ' Usage: data = GET_SCIA_MDS1_DATA( dsd, stateID, chanID' $
              + ', meta=meta, calib=calib, stateIndex=stateIndex, norm=norm' $
              + ', noPMD=noPMD, status=status )', /INFO
     IF ARG_PRESENT( meta ) THEN meta = -1
     status = -1
     RETURN, -1
  ENDIF
  stateID = FIX(stateID[0])
  chanID = FIX(chanID[0])
  pmdScaling = (N_ELEMENTS( noPMD ) GT 0) ? 0 : 1

; read States of the Product
  SCIA_LV1_RD_STATE, dsd, state, status=status
  IF status NE 0 THEN BEGIN
     IF ARG_PRESENT( meta ) THEN meta = -1
     status = -1
     RETURN, -1
  ENDIF

; obtain info about requested state
  index = WHERE( state.state_id EQ stateID and state.flag_mds EQ 0, stateCount )
  IF stateCount EQ 0 THEN BEGIN
     MESSAGE, ' Error: No states with requested ID found in this product', /INFO
     IF ARG_PRESENT( meta ) THEN meta = -1
     status = -1
     RETURN, -1
  ENDIF
  IF N_ELEMENTS( stateIndex ) GT 0 THEN BEGIN
     IF stateIndex GE stateCount THEN BEGIN
        MESSAGE, ' Error: stateIndex is out-of-range', /INFO
        IF ARG_PRESENT( meta ) THEN meta = -1
        status = -1
        RETURN, -1
     ENDIF
     state = state[index[stateIndex]]
  ENDIF ELSE $
     state = state[index]

; obtain calibration mask
  calib_mask = '0'xu
  IF N_ELEMENTS( calib ) GT 0 THEN BEGIN
     calib_mask = call_external( lib_name('libnadc_idl'), '_SCIA_SET_CALIB', $
                                 calib, /UL_VALUE, /CDECL )
  ENDIF

; set up structs for geolocation data
  CASE FIX( state[0].type_mds ) OF
     !nadc.sciaNadir  : BEGIN
        struct = { meta_rec, jday : 0.d, glr : {geoN_scia} }
     END
     !nadc.sciaLimb   : BEGIN
        struct = { meta_rec, jday : 0.d, glr : {geoL_scia} }
     END
     !nadc.sciaOccult : BEGIN
        struct = { meta_rec, jday : 0.d, glr : {geoL_scia} }
     END
     !nadc.sciaMonitor: BEGIN
        struct = { meta_rec, jday : 0.d, glr : {geoC_scia} }
     END
  ENDCASE

; read data of the selected states
  dims_out = -1
  dim_X = ULONG( !nadc.channelSize )
  FOR ns = 0, N_ELEMENTS( state )-1 DO BEGIN
     clus_mask = 0ull
     iindx = WHERE( state[ns].Clcon[*].channel EQ chanID, count )
     IF count EQ 0 THEN continue

     FOR nc = 0, count-1 DO BEGIN
        clus_mask = clus_mask OR ISHFT( 1ull, state[ns].Clcon[iindx[nc]].id-1 )
     ENDFOR
     dim_Y = MAX( state[ns].Clcon[iindx].n_read )
     dim_Z = ULONG( state[ns].num_dsr )
     IF MIN([dim_Y, dim_Z]) EQ 0 THEN continue

     data =  FLTARR( dim_X * dim_Y * dim_Z )
     data_glr = REPLICATE( {meta_rec}, (dim_Y * dim_Z) )
     num = call_external( lib_name('libnadc_idl'), '_GET_SCIA_MDS1_DATA', $
                          is_level_1c, pmdScaling, state[ns], clus_mask, $
                          calib_mask, data, data_glr, /CDECL )
     IF num NE (dim_Y * dim_Z) THEN BEGIN
        IF ARG_PRESENT( meta ) THEN meta = -1
        status = -1
        RETURN, -1
     ENDIF

     IF SIZE( data_out, /DIM ) NE 0 THEN BEGIN
        data_out = [data_out,data]
        IF ARG_PRESENT( meta ) THEN glr_tmp = [glr_tmp, data_glr]
        dims_out[2] += dim_Z
     ENDIF ELSE BEGIN
        data_out = data 
        IF ARG_PRESENT( meta ) THEN glr_tmp = data_glr
        dims_out = [dim_X, dim_Y, dim_Z]
     ENDELSE
  ENDFOR
  IF SIZE( dims_out, /DIM ) EQ 0 THEN BEGIN
     MESSAGE, ' Error: no data found of state found in product', /INFO
     IF ARG_PRESENT( meta ) THEN meta = -1
     status = -1
     RETURN, -1
  ENDIF
  data_out = REFORM(data_out, dims_out, /OVERWRITE)
  IF ARG_PRESENT( meta ) THEN meta = REFORM( glr_tmp, dims_out[1:2] )

; normalize the data to one second integration time
  IF N_ELEMENTS( norm ) GT 0 THEN BEGIN
     iindx = WHERE( state[0].Clcon[*].channel EQ chanID, count )
     int_time = MIN( state[0].Clcon[iindx].int_time / 16.)
     FOR nc = 0, count-1 DO BEGIN
        ni = state[0].Clcon[iindx[nc]].pixel_nr
        nj = ni + state[0].Clcon[iindx[nc]].length - 1
        data_out[ni:nj,*] /= int_time
     ENDFOR
  ENDIF ELSE BEGIN
     iindx = WHERE( state[0].Clcon[*].channel EQ chanID, count )
     mn_coaddf = MIN( state[0].Clcon[iindx].coaddf )
     FOR nc = 0, count-1 DO BEGIN
        IF state[0].Clcon[iindx[nc]].coaddf NE mn_coaddf THEN BEGIN
           ni = state[0].Clcon[iindx[nc]].pixel_nr
           nj = ni + state[0].Clcon[iindx[nc]].length - 1
           data_out[ni:nj,*] *= (state[0].Clcon[iindx[nc]].coaddf / mn_coaddf)
        ENDIF
     ENDFOR
  ENDELSE

  RETURN, data_out
END
