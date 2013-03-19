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
;       GET_LV1_MDS_DATA
;
; PURPOSE:
;       This function extracts detector pixel readouts from a 
;       level 1(c) MDS
;
; CATEGORY:
;       SCIAMACHY level 1
;
; CALLING SEQUENCE:
;       data = GET_LV1_MDS_DATA( mds, state_id )
;
; INPUTS:
;       mds :    an array of type Structure MDS1_SCIA
;  state_id :    a scalar of type integer specifying the ID of the
;                state to be selected from the input MDS. The value of
;                state_id can be set to -1 if all input MDS records
;                hold data from the same state.
;
; KEYWORD PARAMETERS:
;     posit :    a scalar of type integer specifying the index of the
;                MDS record to be extracted, starting at zero.
;
;   channel :    a scalar of type integer specifying number of the
;                channel to be extracted, valid range = [1:8]
;
;   cluster :    a scalar of type integer specifying the number of the
;                pixel data block (a cluster) to be extracted,
;                starting at 1
;                [this keyword is ignored in combination with "channel"]
;
;    pixels :    lower and/or upper boundary of the pixel numbers to
;                be extracted, valid range = [0:1023]
;                 pixels=[p1:p2]    extract pixels between p1 and p2
;                 pixels=-p1        extract pixels between 0 and p1
;                 pixels=p2         extract pixels between p2 and 1023
;
;     align :    if set to a non-zero value, the output array reflects
;                the detector array layout, only in combination with
;                keyword cluster or pixels
;
;     rebin :    if set to a non-zero value, the output array is
;                resized to number of observations for the longest
;                intergrations time (using nearest neighbor sampling)
;
;     coadd :    divide pixel counts by coadding factor
;
;      norm :    divide pixel counts by coadding factor times pixel
;                exposure time
;
;      jday :    named variable holding julian dates of each pixel
;                readout
;
;       NaN :    if set to a non-zero value, NaN is used as fill value
;
;    errors :    set this keyword to a named variable that will
;                contain the signal error values
;      wave :    set this keyword to a named variable that will
;                contain the wavelength of the data points.
;
;    status :    returns named variable with error status (0 = ok)
;
; SIDE EFFECTS:
;       1) wavelength calibration is not performed by this
;       function. One should call SCIA_LV1_RD_MDS, or SciaL1C with
;       the appropriate calibration options.
;       2) ...
;
; PROCEDURE:
;       Each MDS record holds science data from one cluster. The
;       science data is defined by its state ID, cluster ID and
;       channel ID. The purpose of this routine is to extract the
;       science data from the complicated MDS record and organize the
;       data per science channel.
;       The dimensions of the output array (maximum of 4):
;        1) number of pixels, maximum 1024
;        2) number of observations:
;                  state[].num_dsr * state[].Clcon[].n_read
;        3) number of channels, maximum 8
;        4) number of repeated MDS records of a requested state ID
;       - Note than on output all dimensions of size 1 are removed
;       - Note also that the number of observation (2) depends on the
;       integration time, which is large for (a.o.) channel 1.
;       Therefore, in case you not select a science channel, the gaps
;       between data of channel 1 are large and filled with zero's or NaN's.
;
; EXAMPLES:
;       - extract all data (from state_id = 6) with pixel numbers
;         between [102,154], only the second record:
;            data_out = GET_LV1_MDS_DATA( mds, 6, pixels=[102,154], posit=2 )
;
;         (the returned array is an Float array of size [53,8])
;
;       - extract all data of channel 4:
;            data_out = GET_LV1_MDS_DATA( mds, 6, channel=4 )
;
;         (the returned array is an Float array of size [1024,num_obs,num_mds])
;
;       - extract all data of cluster 2, only the third record:
;            data_out = GET_LV1_MDS_DATA( mds, 6, cluster=2, posit=3 )
;
;         (the returned array is an Float array of size [cluster_size])
;
;       - extract all data of cluster 2 as a 1024 array.
;            data_out = GET_LV1_MDS_DATA( mds, 6, cluster=2, posit=3, /align ) 
;         (the returned array is an Float array of size [1024])
;
;       - extract all data of channel 4 & pixel numbers between
;         [102,154], only the second record  
;            data_out = GET_LV1_MDS_DATA( mds, 7, posit=2, channel=4, $
;                                         pixels=[102,154] ) 
;         (the returned array is an Float array of size [53])
;
; REFERENCE:
;       SCIAMACHY Level 0 to 1b Processing
;       Input/Output Data Definition
;       Ref. ENV-TN-DLR-SCIA-0005
;            Isue 5, 21 July 2000
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), 28 June 2002
;       Modified:  RvH, 26 July 2002
;                    improved the given eamples examples
;       Modified:  RvH, 26 July 2002
;                    added keyword state_id, and wave
;       Modified:  RvH, 26 July 2002
;                    the cluster keywords now gives correct results
;       Modified:  RvH, 24 October 2002
;                    complete rewrite
;                    requires state_id as parameter (!!)
;                    updated documentation
;       Modified:  RvH, 28 October 2002
;                    more bug-fixes...
;       Modified:  RvH, 08 Januari 2003
;                    more bug-fixes (keyword cluster),
;                    and added the keyword rebin
;       Modified:  RvH, 16 Januari 2003
;                    added more documentation about NUM_OBS
;       Modified:  RvH, 21 July 2003
;                    made de align-keyword work correctly
;       Modified:  RvH, 07 November 2003
;                    made it work with the new IDL compile options
;       Modified:  RvH, 19 November 2003
;                    made the data selection also work on array "wave"
;       Modified:  RvH, 17 May 2004
;                    added keyword to obtain Signal error values
;       Modified:  RvH, 18 October 2004
;                    added keyword "norm" for normalisation of readouts
;       Modified:  RvH, 18 October 2004
;                    added keyword "coadd"
;       Modified:  RvH, 15 June 2005
;                    fixed several bugs for sampled data (using REBIN)
;       Modified:  RvH, 07 December 2005
;                    renamed pixel_val_err to pixel_err
;       Modified:  RvH, 07 Februari 2006
;                    bugfix removed optional keyword "state_id", which
;                    collide with the parameter "state_id"
;       Modified:  RvH, 06 July 2006
;                    bugfix usage of Norm and/or Coadd keywords
;                    patch came from Guenter Lightenberg
;       Modified:  RvH, 18 July 2006
;                    fixed bug when states with same ID have different
;                    number of dataset records
;-
FUNCTION GET_PARAM_MDS1_DATA, state_id, errors=errors, jday=jday, wave=wave, $
                              channel=channel, cluster=cluster, posit=posit, $
                              pixels=pixels, align=align, rebin=rebin, $
                              coadd=coadd, norm=norm, NaN=NaN, debug=debug
 compile_opt idl2,logical_predicate,hidden

; set some constants
 ValNotSet = -1

 params = { NotSet     : ValNotSet, $
            Debug      : ValNotSet, $
            GiveErrors : ValNotSet, $
            GiveJulian : ValNotSet, $
            GiveWave   : ValNotSet, $
            Channel    : ValNotSet, $
            Cluster    : ValNotSet, $
            StateID    : ValNotSet, $
            Posit      : ValNotSet, $
            DoAlign    : ValNotSet, $
            DoRebin    : ValNotSet, $
            SetCoadd   : ValNotSet, $
            SetNorm    : ValNotSet, $
            SetNaN     : ValNotSet, $
            Pixels     : INTARR(2) $
          }

 params.StateID = FIX( state_id[0] )

; check (optional) keywords
 IF KEYWORD_SET( errors ) THEN params.GiveErrors = 1
 IF KEYWORD_SET( jday ) THEN params.GiveJulian = 1
 IF KEYWORD_SET( wave ) THEN params.GiveWave = 1

 IF KEYWORD_SET( debug ) THEN params.SetDebug = 1
 IF KEYWORD_SET( align ) THEN params.DoAlign = 1 
 IF KEYWORD_SET( coadd ) THEN params.SetCoadd = 1 
 IF KEYWORD_SET( norm ) THEN BEGIN
; if we have already set the norm keyword, we do not want to divide by coadd
; again <gl - 06-07-06>
    params.SetCoadd = params.NotSet
    params.SetNorm = 1
 ENDIF
 IF KEYWORD_SET( rebin ) THEN params.DoRebin = 1 
 IF KEYWORD_SET( NaN ) THEN params.SetNaN = 1

 IF N_ELEMENTS( channel ) GT 0 THEN BEGIN
    IF channel[0] GT 0 THEN $
       params.Channel = FIX( channel[0] )
 ENDIF

 IF N_ELEMENTS( cluster ) GT 0 THEN BEGIN
    IF cluster[0] GT 0 THEN $
       params.Cluster = FIX( cluster[0] )
 ENDIF

 IF N_ELEMENTS( posit ) GT 0 THEN BEGIN
    IF posit[0] GT 0 THEN $
       params.Posit = FIX(posit[0])
 ENDIF

 IF N_ELEMENTS( pixels ) EQ 0 THEN BEGIN
    mn_pix = 0
    mx_pix = !nadc.channelSize-1
 ENDIF ELSE IF pixels[0] EQ -1 THEN BEGIN
    mn_pix = 0
    mx_pix = !nadc.channelSize-1
 ENDIF ELSE IF N_ELEMENTS( pixels ) EQ 1 THEN BEGIN
    IF pixels GE 0 THEN BEGIN
       mn_pix = pixels
       mx_pix = !nadc.channelSize-1
    ENDIF ELSE BEGIN
       mn_pix = 0
       mx_pix = ABS(pixels)
    ENDELSE
 ENDIF ELSE BEGIN
    mn_pix = pixels[0]
    mx_pix = pixels[1]
 ENDELSE
 params.Pixels = [mn_pix,mx_pix]

 RETURN, params
END

;--------------------------------------------------
FUNCTION GET_LV1_STATE_DATA, mds, params, status=status, $
                             errors=errors, jday=jday, wave=wave, _EXTRA=EXTRA
 compile_opt idl2,logical_predicate,hidden

 IF SIZE( mds, /TNAME ) EQ 'STRUCT' THEN BEGIN
    IF TAG_NAMES(mds, /struct) NE 'MDS1C_SCIA' THEN BEGIN
       MESSAGE, 'Exit: Not a level 1c MDS structure as input', /INFO
       status = -1
       RETURN, -1
    ENDIF
 ENDIF ELSE BEGIN
    status = -1
    RETURN, -1
 ENDELSE

; set some constants
 sec2day = (24.d * 60 * 60)

; initialisation of return values
 status = 0

; determine the dimensions of the output array
 NumPIXEL = !nadc.channelSize

 IF params.DoRebin NE params.NotSet THEN $
    NumOBS = MIN( mds.num_obs ) $
 ELSE $
    NumOBS = MAX( mds.num_obs )

 IF params.Channel NE params.NotSet OR params.Cluster NE params.NotSet THEN $
    NumCHAN = 1 $
 ELSE $
    NumCHAN = !nadc.scienceChannels

; define output arrays
 data_out = FLTARR( NumPIXEL, NumOBS, NumCHAN )
 IF params.GiveJulian NE params.NotSet THEN $
    jday = DBLARR( NumOBS, NumCHAN ) $
 ELSE $
    jday = params.NotSet
 IF params.GiveErrors NE params.NotSet THEN $
    errors = FLTARR( NumPIXEL, NumOBS, NumCHAN ) $
 ELSE $
    errors = params.NotSet
 IF params.GiveWave NE params.NotSet THEN $
    wave = FLTARR( NumPIXEL, NumCHAN ) $
 ELSE $
    wave = params.NotSet

; insert NaN vales, if requested
 IF params.SetNaN NE params.NotSet THEN BEGIN
    REPLICATE_INPLACE, data_out, !VALUES.F_NAN
    IF params.GiveJulian NE params.NotSet THEN $
       REPLICATE_INPLACE, jday, !VALUES.D_NAN
    IF params.GiveErrors NE params.NotSet THEN $
       REPLICATE_INPLACE, errors, !VALUES.F_NAN
    IF params.GiveWave NE params.NotSet THEN $
       REPLICATE_INPLACE, wave, !VALUES.F_NAN
 ENDIF

; extract data values from MDS-structure
 nd = -1
 mn_clus_id = MIN( mds.clus_id )
 FOR nm = 0, N_ELEMENTS( mds )-1 DO BEGIN
    IF mds[nm].chan_id GE 6 THEN BEGIN
;PET offset in EPITAXX detectors (TN-SCIA-0000DO/19,10.03.1999)
       PET_OFFSET = 1.18125e-3

;apply EPITAXX offset for ch. 6-8 and PET >= 31.25ms
       indx = WHERE( mds[nm].pet ge 0.03125, count )
       IF count GT 0 THEN mds[nm].pet[indx] -= PET_OFFSET
    ENDIF
    intg_time = mds[nm].coaddf * mds[nm].pet

    IF mds[nm].clus_id EQ mn_clus_id THEN nd = nd + 1

    IF params.Channel EQ params.NotSet $
       AND params.Cluster EQ params.NotSet THEN $
          n_ch = mds[nm].chan_id - 1 $
    ELSE $
       n_ch = 0

    first = (*mds[nm].pixel_ids)[0] MOD !nadc.channelSize
    last = first + mds[nm].num_pixels - 1
    IF mds[nm].num_obs EQ NumOBS THEN BEGIN
       data_out[first:last,*,n_ch,nd] = *mds[nm].pixel_val
       IF params.GiveErrors NE params.NotSet THEN BEGIN
          errors[first:last,*,n_ch,nd] = *mds[nm].pixel_err
       ENDIF
       IF params.SetCoadd NE params.NotSet THEN $
          data_out[first:last,*,n_ch,nd] /= mds[nm].coaddf
       IF params.SetNorm NE params.NotSet THEN $
          data_out[first:last,*,n_ch,nd] /= intg_time
    ENDIF ELSE IF mds[nm].num_obs GT NumOBS THEN BEGIN
       data_out[first:last,*,n_ch,nd] = $
          REBIN( *mds[nm].pixel_val, mds[nm].num_pixels, NumOBS )
       IF params.GiveErrors NE params.NotSet THEN BEGIN
          errors[first:last,*,n_ch,nd] = $
             REBIN( *mds[nm].pixel_err, mds[nm].num_pixels,NumOBS )
       ENDIF
       IF params.SetCoadd NE params.NotSet THEN $
          data_out[first:last,*,n_ch,nd] /= mds[nm].coaddf
       IF params.SetNorm NE params.NotSet THEN $
          data_out[first:last,*,n_ch,nd] /= intg_time
    ENDIF ELSE BEGIN
       data_out[first:last,*,n_ch,nd] = $
          REBIN( *mds[nm].pixel_val, mds[nm].num_pixels, NumOBS, /SAMPLE )
       IF params.GiveErrors NE params.NotSet THEN BEGIN
          errors[first:last,*,n_ch,nd] = $
             REBIN( *mds[nm].pixel_err,mds[nm].num_pixels,NumOBS,/SAMPLE )
       ENDIF
;        IF nm LT num_mds-1 THEN BEGIN
;            nr_sample = NumOBS / mds[nm].num_obs
;            mn_indx = mds[nm+1].num_pixels - 11 > 0
;            mx_indx = mds[nm-1].num_pixels - 1
;            wght = FLTARR( nr_sample )
;            FOR nr = 0, mds[nm].num_obs-1 DO BEGIN
;                avg = MEAN((*mds[nm+1].pixel_val)[mn_indx:mx_indx,:])
;                FOR ii = 0, nr_sample-1 DO $
;                  wght[ii] = MEAN(*mds[nm].pixel_val[mn_indx:mx_indx,ii])/avg
;            ENDFOR
;        ENDFOR
       IF params.SetCoadd NE params.NotSet THEN $
          data_out[first:last,*,n_ch,nd] /= mds[nm].coaddf
       IF params.SetNorm NE params.NotSet THEN $
          data_out[first:last,*,n_ch,nd] /= intg_time
    ENDELSE 

    IF params.GiveJulian NE params.NotSet THEN BEGIN
       intg_time = mds[nm].pet * mds[nm].coaddf
       dsec = mds[nm].mjd.secnd + mds[nm].mjd.musec * 1d-6 $
              + intg_time * DINDGEN(mds[nm].num_obs)
       IF mds[nm].num_obs LT NumOBS THEN $
          dsec = REBIN( dsec, NumOBS, /SAMPLE ) $
       ELSE IF mds[nm].num_obs GT NumOBS THEN $
          dsec = REBIN( dsec, NumOBS )
       jday[*,n_ch,nd] = mds[nm].mjd.days + dsec / sec2day
    ENDIF
    IF params.GiveWave NE params.NotSet THEN $
       wave[first:last,n_ch,nd] = *mds[nm].pixel_wv
 ENDFOR

; select only the relevant pixel numbers if align is not set
 IF params.Cluster NE params.NotSet $
    AND params.DoAlign EQ params.NotSet THEN BEGIN
    clusterMinID = (*mds[0].pixel_ids)[0] MOD !nadc.channelSize
    clusterMaxID = (*mds[0].pixel_ids)[mds[0].num_pixels-1] MOD !nadc.channelSize
    IF params.Pixels[0] LT clusterMinID THEN params.Pixels[0] = clusterMinID
    IF params.Pixels[1] GT clusterMaxID THEN params.Pixels[1] = clusterMaxID
 ENDIF
 IF params.Pixels[0] NE params.NotSet THEN BEGIN
    data_out = data_out[params.Pixels[0]:params.Pixels[1],*,*,*]
    IF params.GiveErrors NE params.NotSet THEN $
       errors = errors[params.Pixels[0]:params.Pixels[1],*,*,*]
    IF params.GiveWave NE params.NotSet THEN $
       wave = wave[params.Pixels[0]:params.Pixels[1],*,*]
 ENDIF

; select one dataset if posit is set
 IF params.Posit NE params.NotSet THEN BEGIN
    xpos = params.Posit MOD NumOBS
    ypos = params.Posit / NumOBS
    data_out = data_out[*,xpos,*,ypos]
    IF params.GiveJulian NE params.NotSet THEN jday = jday[xpos,*,ypos]
    IF params.GiveErrors NE params.NotSet THEN errors = errors[*,xpos,*,ypos]
    IF params.GiveWave NE params.NotSet THEN wave = wave[*,*,ypos]    
 ENDIF

 RETURN, data_out
END

;--------------------------------------------------
FUNCTION GET_LV1_MDS_DATA, mds_in, state_id, status=status, $
                           errors=errors, jday=jday, wave=wave, _EXTRA=EXTRA
 compile_opt idl2,logical_predicate,hidden

 IF N_PARAMS() NE 2 THEN BEGIN
    MESSAGE, 'Usage: Result = GET_LV1_MDS_DATA( mds, state_id' $
             + ', errors=errors, jday=jday, wave=wave', $
             + ', channel=channel, cluster=cluster,', $
             + ', posit=posit, pixels=pixels, status=status', $
             + ', /align, /rebin, /coadd, /norm, /NaN )', /INFO
    status = -1
    RETURN, -1
 ENDIF

; check/read (optional) keywords
 IF ARG_PRESENT( errors ) THEN errors = 1
 IF ARG_PRESENT( jday ) THEN jday = 1
 IF ARG_PRESENT( wave ) THEN wave = 1
 params = GET_PARAM_MDS1_DATA( state_id, errors=errors, jday=jday, wave=wave, $
                               _EXTRA=EXTRA )

; initialisation of return values
 status = 0
 data_out = params.NotSet

; select MDS records with state ID equals state_id
 IF params.StateID EQ params.NotSet THEN BEGIN
    IF N_ELEMENTS(UNIQ(mds_in.state_id, SORT(mds_in.state_id))) GT 1 THEN BEGIN
       MESSAGE, 'Exit: MDS records do not have an unique state ID', /INFO
       status = -1
       RETURN, data_out
    ENDIF
    num_mds = N_ELEMENTS( mds_in )
    mds = mds_in
 ENDIF ELSE BEGIN
    indx = WHERE( mds_in.state_id EQ params.StateID, num_mds )
    IF num_mds EQ 0 THEN BEGIN
       MESSAGE, 'no MDS records with requested state_id', /INFO
       status = -1
       RETURN, data_out
    ENDIF
    mds = mds_in[indx]
 ENDELSE

; check (optional) keywords
 IF params.Channel NE params.NotSet THEN BEGIN
    indx = WHERE( mds.chan_id EQ params.Channel, num_mds )
    IF num_mds EQ 0 THEN BEGIN
       status = -1
       RETURN, data_out
    ENDIF
    mds = mds[indx]
 ENDIF

 IF params.Cluster NE params.NotSet THEN BEGIN
    indx = WHERE( mds.clus_id EQ params.Cluster, num_mds )
    IF num_mds EQ 0 THEN BEGIN
       status = -1
       RETURN, data_out
    ENDIF
    mds = mds[indx]
 ENDIF

; process all MDS records per state
 secperday = (60 * 60 * 24.d)
 mjd_all = mds.mjd.days + (mds.mjd.secnd + mds.mjd.musec / 1.d6) / secperday
 mjd_uniq = mjd_all[UNIQ( mjd_all )]
 NumDSR = N_ELEMENTS( mjd_uniq )
 FOR ii = 0, NumDSR-1 DO BEGIN
    indx = WHERE( mjd_all EQ mjd_uniq[ii] )
    data_new = GET_LV1_STATE_DATA( mds[indx], params, status=status, $
                                   errors=errors, jday=jday, wave=wave,$
                                   _EXTRA=EXTRA )
    sz = SIZE( data_new )
    IF ii EQ 0 THEN BEGIN
       data_out = MAKE_ARRAY( DIM=[sz[1:sz[0]],NumDSR], /NOZERO, /FLOAT )
       sz_out = SIZE( data_out )
       IF params.GiveErrors NE params.NotSet THEN $
          errors_out = MAKE_ARRAY( DIM=sz_out[1:sz_out[0]], /NOZERO, /FLOAT )
       IF params.GiveJulian NE params.NotSet THEN $
          jday_out = MAKE_ARRAY( DIM=sz_out[2:sz_out[0]], /NOZERO, /DOUBLE )
       IF params.GiveWave NE params.NotSet THEN BEGIN
          sz_wv = SIZE( wave )
          wave_out = MAKE_ARRAY( DIM=[sz_wv[1:sz_wv[0]],NumDSR], $
                                 /NOZERO, /FLOAT )
       ENDIF
    ENDIF ELSE IF sz_out[0] GT 1 THEN BEGIN
       IF sz_out[2] GT sz[2] THEN BEGIN
          buff = data_new
          data_new = MAKE_ARRAY( DIM=sz_out[1:sz[0]], $
                                 VALUE=!VALUES.F_NAN, /FLOAT )
          data_new[*,0:sz[2]-1,*] = buff
          IF params.GiveErrors NE params.NotSet THEN BEGIN
             buff = errors
             errors = MAKE_ARRAY( DIM=sz_out[1:sz[0]], $
                                  VALUE=!VALUES.F_NAN, /FLOAT )
             errors[*,0:sz[2]-1,*] = buff
          ENDIF
          IF params.GiveJulian NE params.NotSet THEN BEGIN
             buff = jday
             jday = MAKE_ARRAY( DIM=sz_out[2:sz[0]], $
                                VALUE=!VALUES.D_NAN, /DOUBLE )
             jday[0:sz[2]-1,*] = buff
          ENDIF
          buff = 0
       ENDIF ELSE IF sz_out[2] LT sz[2] THEN BEGIN
          buff = data_new
          data_new = MAKE_ARRAY( DIM=sz[1:sz[0]], VALUE=!VALUES.F_NAN, /FLOAT )
          data_new = buff[*,0:sz_out[2]-1,*]
          IF params.GiveErrors NE params.NotSet THEN BEGIN
             buff = errors
             errors = MAKE_ARRAY( DIM=sz[1:sz[0]], $
                                  VALUE=!VALUES.F_NAN, /FLOAT )
             errors = buff[*,0:sz_out[2]-1,*]
          ENDIF
          IF params.GiveJulian NE params.NotSet THEN BEGIN
             buff = jday
             jday = MAKE_ARRAY( DIM=sz[2:sz[0]], $
                                VALUE=!VALUES.D_NAN, /DOUBLE )
             jday = buff[0:sz_out[2]-1,*]
          ENDIF
;         sz_out = sz
          buff = 0
       ENDIF
    ENDIF

    IF sz[0] EQ 3 THEN BEGIN
       data_out[*,*,*,ii] = data_new
       IF params.GiveErrors NE params.NotSet THEN errors_out[*,*,*,ii] = errors
       IF params.GiveJulian NE params.NotSet THEN jday_out[*,*,ii] = jday
       IF params.GiveWave NE params.NotSet THEN wave_out[*,*,ii] = wave
    ENDIF ELSE IF sz[0] EQ 1 THEN BEGIN
       data_out[*,ii] = data_new
       IF params.GiveErrors NE params.NotSet THEN errors_out[*,ii] = errors
       IF params.GiveJulian NE params.NotSet THEN jday_out[ii] = jday
       IF params.GiveWave NE params.NotSet THEN wave_out[ii] = wave
    ENDIF ELSE BEGIN
       data_out[*,*,ii] = data_new
       IF params.GiveErrors NE params.NotSet THEN errors_out[*,*,ii] = errors
       IF params.GiveJulian NE params.NotSet THEN jday_out[*,ii] = jday
       IF params.GiveWave NE params.NotSet THEN wave_out[*,ii] = wave
    ENDELSE
 ENDFOR

; get rid of dimensions with size equals 1
 IF params.GiveJulian NE params.NotSet THEN jday = REFORM( jday_out )
 IF params.GiveErrors NE params.NotSet THEN errors = REFORM( errors_out )
 IF params.GiveWave NE params.NotSet THEN wave = REFORM( wave_out )

 RETURN, REFORM(data_out)
END
