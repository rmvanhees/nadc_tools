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
;       GET_LV0_MDS_DATA
;
; PURPOSE:
;       This function extracts detector pixel readouts from a 
;       level 0 Detector MDS
;
; CATEGORY:
;       SCIAMACHY level 0
;
; CALLING SEQUENCE:
;       data = GET_LV0_MDS_DATA( mds_det, ... )
;
; INPUTS:
;   mds_det :    an element or array of type Structure MDS0_DET
;
; KEYWORD PARAMETERS:
;  state_id :    a scalar of type integer specifying the ID of the
;                state to be selected from the input MDS. The value of
;                state_id can be set to -1 if all input MDS records
;                hold data from the same state.
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
;  do_coadd :    divide pixel counts by co-adding factor
;
;      norm :    divide pixel counts by co-adding factor times pixel
;                exposure time
;
;    coaddf :    named variable holding co-adding factors of each pixel
;
;      jday :    named variable holding julian dates of each pixel
;                readout
;
;       NaN :    if set to a non-zero value, NaN is used as fill value
;                [implies that the return array is of type float]
;
;    status :    returns named variable with error status (0 = ok)
;
; PROCEDURE:
;       Each Detector MDS record holds science data of only those
;       clusters which have finished their integration time. The
;       purpose of this routine is to collect data from one or more
;       MDS records and organize the data per science channel. The
;       dimensions of the output array are (maximum 3):
;        1) number of pixels, maximum 1024
;        2) number of channels, zero or eight
;        3) number of (selected) MDS records, maximum number of input
;        MDS
;
; EXAMPLES:
;       - extract all data from the second MDS.
;            data_out = GET_LV0_MDS_DATA( mds_det[1] )
;
;         (the returned array is an ULONG array of size [1024, num_chan])
;       - extract all data with pixel numbers between [102,154]from
;         the second MDS. 
;            data_out = GET_LV0_MDS_DATA( mds_det[1], pixels=[102,154] )
;
;         (the returned array is an ULONG array of size [53, num_chan])
;       - extract all data of channel 4 from the second MDS.
;            data_out = GET_LV0_MDS_DATA( mds_det[1], channel=4 )
;
;         (the returned array is an ULONG array of size [1024])
;       - extract all data of channel 4 & cluster 2 from the second MDS.
;            data_out = GET_LV0_MDS_DATA( mds_det[1], channel=4,
;                                         cluster=2 )
;         (the returned array is an ULONG array of size [cluster_size])
;       - extract all data of channel 4 & cluster 2 from the second MDS.
;            data_out = GET_LV0_MDS_DATA( mds_d[2], channel=4,
;                                         cluster=2, /align ) 
;         (the returned array is an ULONG array of size [1024])
;       - extract all data of channel 4 & pixel numbers between
;         [102,154] from the second MDS.  
;            data_out = GET_LV0_MDS_DATA( mds_det[1], channel=4,
;                                         pixels=[102,154] ) 
;         (the returned array is an ULONG array of size [53])
;
; REFERENCE:
;       Measurement data definition and format description for
;       SCIAMACHY, volume 14. PO-ID-DOR-SY-0032
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), Januari 2002
;       Modified:  RvH, February 2002
;                    Uses the new function GET_KEY_CHAN_HDR
;       Modified:  RvH, March 2002
;                    removed use of obsoleted function GET_KEY_CHAN_HDR
;       Modified:  RvH, March 2002
;                    added normalisation of data values both PET and coadding
;       Modified:  RvH, 21 March 2002
;                    set keywords to -1 results in no selection
;       Modified:  RvH, 09 April 2002
;                    updated documentation
;       Modified:  RvH, 19 April 2002
;                    now align also works in case of pixel selection
;       Modified:  RvH, 21 April 2002
;                    PET also works with virtual channels
;       Modified:  RvH, 06 November 2002
;                    more checks on data
;                    added selection on state ID
;       Modified:  RvH, 07 November 2002
;                    bugfix for state_id checking
;                    check on MDS input record type
;                    added "PROCEDURE" to the documentation
;       Modified:  RvH, 03 March 2004
;                    fixed several bugs and did some code clean-ups
;       Modified:  RvH, 30 November 2004
;                    function can now return the coadding factors
;       Modified:  RvH, 30 November 2004
;                    Speeding-up this code!
;       Modified:  RvH, 30 August 2005
;                    Fixed several "brown paperbag" bugs
;                    Added optional parameter: jday
;       Modified:  RvH, 17 May 2007
;                    Reduce output array to actual number of readouts
;       Modified:  RvH, 15 January 2009
;                    Fixed bugs (a.o. keyword CLUSTER) and code clean-up
;       Modified:  RvH, 19 January 2009
;                    added keyword DO_COADD to make a distinction
;                    between applying co-adding factor and to return
;                    the co-adding factors
;       Modified:  RvH, 13 April 2010
;                    adopted to modified mds_det_data_src.hdr structure
;-
FUNCTION GET_PARAM_MDS0_DATA, debug=debug, coaddf=coaddf, jday=jday, NaN=NaN, $
                              align=align, channel=channel, cluster=cluster, $
                              pixels=pixels, norm=norm, state_id=state_id, $
                              do_coadd=do_coadd
  compile_opt idl2,hidden,logical_predicate,hidden

  params = { Debug      : !FALSE, $
             ApplyCoadd : !FALSE, $
             GiveCoadd  : !FALSE, $
             GiveJulian : !FALSE, $
             DoAlign    : !FALSE, $
             SetNorm    : !FALSE, $
             SetNaN     : !FALSE, $
             Channel    : -1, $
             Cluster    : -1, $
             StateID    : -1, $
             Pixels     : [-1,-1] $
           }

; check (optional) keywords
  IF KEYWORD_SET( debug ) THEN params.Debug = !TRUE

  IF KEYWORD_SET( do_coadd ) THEN params.ApplyCoadd = !TRUE
  IF KEYWORD_SET( coaddf ) THEN params.GiveCoadd = !TRUE

  IF KEYWORD_SET( norm ) THEN BEGIN
     IF ~ params.ApplyCoadd THEN params.ApplyCoadd = !TRUE
     params.SetNorm = !TRUE
  ENDIF

  IF KEYWORD_SET( jday ) THEN params.GiveJulian = !TRUE

  IF KEYWORD_SET( align ) THEN params.DoAlign = !TRUE

  IF KEYWORD_SET( NaN ) THEN params.SetNaN = !TRUE

  IF N_ELEMENTS( channel ) GT 0 THEN BEGIN
     IF channel[0] GT 0 THEN $
        params.Channel = FIX( channel[0] )
  ENDIF

  IF N_ELEMENTS( cluster ) GT 0 THEN BEGIN
     IF cluster[0] GT 0 THEN $
        params.Cluster = FIX( cluster[0] )
  ENDIF

  IF N_ELEMENTS( state_id ) GT 0 THEN BEGIN
     IF state_id[0] GT 0 THEN $
        params.StateID = FIX( state_id[0] )
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
FUNCTION GET_LV0_ONE_MDS_DATA, mds_det, params, coaddf=coaddf, jday=jday, $
                               status=status
  compile_opt idl2,logical_predicate,hidden

; initialisation of indx_data and return value
  status = 0
  indx_data = 0L
  data_out = -1

  IF N_ELEMENTS( mds_det ) NE 1 THEN BEGIN
     MESSAGE, 'can only handle one detector MDS at a time', /INFO
     status = -1
     RETURN, data_out
  ENDIF
;   
; 3 different output arrays
; a) no selection on channel ID and num_chan GT 1  (implicit "align")
; b) selection on clusters                         (no implicit "align")
; c) no selection on clusters                      (implicit "align")
;
  IF params.Channel EQ -1 AND params.Cluster EQ -1 THEN BEGIN
     IF ~ params.SetNaN AND ~ params.SetNorm AND ~ params.ApplyCoadd THEN $
        data_out = ULONARR( !nadc.channelSize, !nadc.scienceChannels ) $
     ELSE $
        data_out = FLTARR( !nadc.channelSize, !nadc.scienceChannels )

     IF params.GiveCoadd THEN $
        coaddf = REPLICATE( 0b, !nadc.channelSize, !nadc.scienceChannels )

     FOR n_ch = 0, mds_det.num_chan-1 DO BEGIN
        num_clus = ISHFT(mds_det.data_src[n_ch].hdr.channel, -8 )
        chanID = ISHFT( mds_det.data_src[n_ch].hdr.channel, -4 ) $
                 AND (NOT ISHFT(NOT 0us, 4))
        IF params.SetNorm THEN BEGIN
           pet = GET_LV0_MDS_HK(mds_det, channel=chanID, /pet, virtual=ipix)
        ENDIF
        FOR n_cl = 0, num_clus-1 DO BEGIN
           IF (~ PTR_VALID(mds_det.data_src[n_ch].cluster[n_cl].data)) THEN $
              MESSAGE, "invalid pointer to data ... EXIT"

           start = mds_det.data_src[n_ch].cluster[n_cl].start 
           length = mds_det.data_src[n_ch].cluster[n_cl].length

           first = start MOD !nadc.channelSize
           last = first + length - 1
           IF last LT !nadc.channelSize THEN BEGIN
              data_out[first:last,chanID-1] = $
                 (*mds_det.data_src[n_ch].cluster[n_cl].data)
              IF params.GiveCoadd THEN BEGIN
                 coaddf[first:last,chanID-1] = $
                    mds_det.data_src[n_ch].cluster[n_cl].coaddf
              ENDIF
              IF params.ApplyCoadd THEN BEGIN
                 cbuff = mds_det.data_src[n_ch].cluster[n_cl].coaddf
                 data_out[first:last,chanID-1] /= cbuff[first:last]
              ENDIF
              IF params.SetNorm THEN BEGIN
                 IF ipix EQ 0 OR first LT ipix THEN $
                    data_out[first:last,chanID-1] /= pet[0] $
                 ELSE $
                    data_out[first:last,chanID-1] /= pet[1]
              ENDIF
           ENDIF
        ENDFOR
     ENDFOR
;
; select pixel range
     data_out = data_out[params.Pixels[0]:params.Pixels[1],*]
     IF params.GiveCoadd THEN $
        coaddf = coaddf[params.Pixels[0]:params.Pixels[1],*]
  ENDIF ELSE BEGIN              ; selection on channel or cluster
     n_ch = 0
     found = !FALSE
     FOR n_ch = 0, mds_det.num_chan-1 DO BEGIN
        i_ch = ISHFT( mds_det.data_src[n_ch].hdr.channel, -4 ) $
               AND (NOT ISHFT(NOT 0us, 4))
        IF i_ch EQ params.Channel THEN BEGIN
           found = !TRUE
           IF params.SetNorm THEN BEGIN
              pet = GET_LV0_MDS_HK( mds_det, channel=params.Channel, $
                                    /pet, virtual=ipix )
              IF N_ELEMENTS(ipix) EQ 1 THEN ipix = ipix[0]
           ENDIF
           BREAK                ; jump out of loop
        ENDIF
     ENDFOR
     IF ~ found THEN RETURN, data_out

     num_clus = ISHFT( mds_det.data_src[n_ch].hdr.channel, -8 )
     IF params.Cluster EQ -1 THEN BEGIN
        IF ~ params.SetNaN AND ~ params.SetNorm AND ~ params.ApplyCoadd THEN $
           data_out = ULONARR( !nadc.channelSize ) $
        ELSE $
           data_out = FLTARR( !nadc.channelSize )

        IF params.GiveCoadd THEN coaddf = REPLICATE( 0B, !nadc.channelSize )

        chanID = ISHFT( mds_det.data_src[n_ch].hdr.channel, -4 ) $
                 AND (NOT ISHFT(NOT 0us, 4))
        FOR n_cl = 0, num_clus-1 DO BEGIN
           IF (~ PTR_VALID(mds_det.data_src[n_ch].cluster[n_cl].data)) THEN $
              MESSAGE, "invalid pointer to data ... EXIT"

           start = mds_det.data_src[n_ch].cluster[n_cl].start
           first = start MOD !nadc.channelSize
           length = mds_det.data_src[n_ch].cluster[n_cl].length
           last = first + length - 1

           IF last LT !nadc.channelSize THEN BEGIN
              data_out[first:last] = $
                 (*mds_det.data_src[n_ch].cluster[n_cl].data)
              IF params.GiveCoadd THEN BEGIN
                 coaddf[first:last] = $
                    mds_det.data_src[n_ch].cluster[n_cl].coaddf
              ENDIF
              IF params.ApplyCoadd THEN BEGIN
                 data_out[first:last] /= $
                    mds_det.data_src[n_ch].cluster[n_cl].coaddf
              ENDIF 
              IF params.SetNorm THEN BEGIN
                 IF ipix EQ 0 OR first LT ipix THEN $
                    data_out[first:last] /= pet[0] $
                 ELSE $
                    data_out[first:last] /= pet[1]
              ENDIF
           ENDIF
        ENDFOR
;
; select pixel range
        IF params.Pixels[0] GT 0 $
           OR params.Pixels[1] LT (!nadc.channelSize-1) THEN BEGIN
           IF params.DoAlign THEN BEGIN
              buff = data_out
              data_out = REPLICATE( !VALUES.F_NAN, !nadc.channelSize )
              data_out[params.Pixels[0]:params.Pixels[1]] = $
                 buff[params.Pixels[0]:params.Pixels[1]]
           ENDIF ELSE BEGIN
              data_out = data_out[params.Pixels[0]:params.Pixels[1],*]
              IF params.GiveCoadd THEN  $
                 coaddf = coaddf[params.Pixels[0]:params.Pixels[1],*]
           ENDELSE
        ENDIF
    ENDIF ELSE BEGIN
       FOR n_cl = 0, num_clus-1 DO BEGIN
          IF mds_det.data_src[n_ch].cluster[n_cl].id EQ params.Cluster THEN $
             break
       ENDFOR
       IF n_cl EQ num_clus THEN RETURN, data_out

       IF (~ PTR_VALID(mds_det.data_src[n_ch].cluster[n_cl].data)) THEN $
          MESSAGE, "invalid pointer to data ... EXIT"
       
       start = mds_det.data_src[n_ch].cluster[n_cl].start
       first = start MOD !nadc.channelSize
       length = mds_det.data_src[n_ch].cluster[n_cl].length
       last = first + length - 1

       IF last LT !nadc.channelSize THEN BEGIN
          data_out = (*mds_det.data_src[n_ch].cluster[n_cl].data)

          cbuff = REPLICATE( mds_det.data_src[n_ch].cluster[n_cl].coaddf, $
                             length )
          IF params.GiveCoadd THEN coaddf = cbuff
          IF params.ApplyCoadd THEN data_out /= cbuff

          IF params.SetNorm THEN BEGIN
             IF ipix EQ 0 OR first LT ipix THEN $
                data_out /= pet[0] $
             ELSE $
                data_out /= pet[1]
          ENDIF
;
; select pixel range
          mn_indx = (params.Pixels[0] - first) > 0
          mx_indx = (params.Pixels[1] - first) < (length - 1)
          nrpix = mx_indx - mn_indx + 1

          IF params.Pixels[0] GT 0 $
             OR params.Pixels[1] LT (!nadc.channelSize-1) THEN BEGIN
             data_out = data_out[mn_indx:mx_indx]
             IF params.GiveCoadd THEN coaddf = coaddf[mn_indx:mx_indx]
          ENDIF

          IF params.DoAlign THEN BEGIN
             ubuff = data_out
             data_out = ULONARR( !nadc.channelSize )
             data_out[first:nrpix+first-1] = ubuff
             IF params.GiveCoadd THEN BEGIN
                cbuff = coaddf
                coaddf = BYTARR( !nadc.channelSize )
                coaddf[first:nrpix+first-1] = cbuff
             ENDIF
          ENDIF
       ENDIF
    ENDELSE
 ENDELSE

; check (optional) keywords
  IF params.GiveJulian THEN jday = GET_LV0_MDS_TIME( mds_det )

  RETURN, data_out
END

;++++++++++++++++++++++++++++++++++++++++++++++++++
FUNCTION GET_LV0_MDS_DATA, mds_in, status=status, coaddf=coaddf, jday=jday, $
                           _EXTRA=EXTRA
  compile_opt idl2,logical_predicate,hidden

  IF N_PARAMS() NE 1 THEN BEGIN
     MESSAGE, ' Usage: GET_LV0_MDS_DATA, mds_det state_id=state_id' $
              + ', channel=channel, cluster=cluster, pixels=pixels' $
              + ', coaddf=coaddf, jday=jday, status=status' $
              + ', /do_coadd, /norm, /align, /NaN, /debug', /INFO
     status = -1
     RETURN, -1
  ENDIF

  IF SIZE( mds_in, /TNAME ) EQ 'STRUCT' THEN BEGIN
     IF TAG_NAMES( mds_in, /struct) NE 'MDS0_DET' THEN BEGIN
        MESSAGE, 'Exit: Not a level 0 Detector MDS structure as input', /INFO
        status = -1
        RETURN, -1
     ENDIF
  ENDIF ELSE BEGIN
     status = -1
     RETURN, -1
  ENDELSE

; check/read (optional) keywords
  IF ARG_PRESENT( coaddf ) THEN coadd = 1
  IF ARG_PRESENT( jday ) THEN jday = 1
  params = GET_PARAM_MDS0_DATA( coaddf=coaddf, jday=jday, _EXTRA=EXTRA )

; initialisation of indx_data and return value
  status = 0
  indx_data = 0L
  data_out = -1
  
; select data records with requested stateID
  IF params.StateID EQ -1 THEN BEGIN
     indx = UNIQ( mds_in.data_hdr.state_id, SORT(mds_in.data_hdr.state_id) )
     IF N_ELEMENTS( indx ) GT 1 THEN BEGIN
        MESSAGE, 'Exit: MDS records do not have an unique state ID', /INFO
        status = -1
        RETURN, data_out
     ENDIF
     params.StateID = mds_in[indx].data_hdr.state_id
     mds_det = mds_in
  ENDIF ELSE BEGIN
     indx = WHERE( mds_in.data_hdr.state_id EQ params.StateID, count )
     IF count EQ 0 THEN BEGIN
        MESSAGE, 'No Detector MDS records with requested state_id', /INFO
        status = -1
        RETURN, data_out
     ENDIF
     mds_det = mds_in[indx]
  ENDELSE

; select data records with requested channel data
  IF params.Channel NE -1 THEN BEGIN
     chanID = ISHFT( mds_det.data_src.hdr.channel, -4 ) $
                 AND (NOT ISHFT(NOT 0us, 4))
     MdsIndex  = WHERE( TOTAL( (chanID EQ params.Channel), 1 ), count )
     IF count EQ 0 THEN BEGIN
        IF params.Debug THEN $
           MESSAGE, 'No Detector MDS records with requested chanID', /INFO
        status = 0
        RETURN, data_out
     ENDIF
     mds_det = mds_det[MdsIndex]
  ENDIF

  clusDef = GET_SCIA_CLUSDEF( params.StateID  )
  IF params.Cluster NE -1 THEN BEGIN
     IF params.Channel EQ -1 THEN $
        params.Channel = clusDef[params.Cluster-1].chanID
     params.Cluster = clusDef[params.Cluster-1].clusID
  ENDIF

; find the first valid record and read it
  coadd_0 = 1
  jday_0 = 1
  n_det = -1
  num_mds = N_ELEMENTS( mds_det )
  while n_det LT num_mds-1 do begin
     data_0 = GET_LV0_ONE_MDS_DATA( mds_det[++n_det], params, status=status, $
                                    coaddf=coadd_0, jday=jday_0 )
     IF status NE 0 THEN break
     IF (data_0)[0] NE -1 THEN break
  endwhile

; exit if failed or if only one record
  IF status NE 0 OR (data_0)[0] EQ -1 THEN RETURN, data_0

; extend output array for all other datasets
  sz = SIZE( data_0 )
  IF sz[0] EQ 1 THEN BEGIN
     IF params.SetNaN OR SIZE( data_0, /TYPE ) EQ 4 THEN $
        data_out = FLTARR( sz[1], num_mds ) $
     ELSE $
        data_out = ULONARR( sz[1], num_mds )
     IF params.SetNaN THEN REPLICATE_INPLACE, data_out, !VALUES.F_NAN

     data_out[*,n_det] = data_0
     IF params.GiveCoadd THEN BEGIN
        coaddf = REPLICATE( 0B, sz[1], num_mds )
        coaddf[*,n_det] = coadd_0
     ENDIF
     IF params.GiveJulian THEN BEGIN
        jday = REPLICATE( 0.d, sz[1], num_mds )
        jday[*,n_det] = jday_0
     ENDIF
  ENDIF ELSE BEGIN
     IF params.SetNaN OR SIZE( data_0, /TYPE ) EQ 4 THEN $
        data_out = FLTARR( sz[1], sz[2], num_mds ) $
     ELSE $
        data_out = ULONARR( sz[1], sz[2], num_mds )
     IF params.SetNaN THEN REPLICATE_INPLACE, data_out, !VALUES.F_NAN
     
     data_out[*,*,n_det] = data_0
     IF params.GiveCoadd THEN BEGIN
        coaddf = REPLICATE( 0B, sz[1], sz[2], num_mds )
        coaddf[*,*,n_det] = coadd_0
     ENDIF
     IF params.GiveJulian THEN BEGIN
        jday = REPLICATE( 0.d, sz[1], sz[2], num_mds )
        jday[*,*,n_det] = jday_0
     ENDIF
  ENDELSE

; read the other data sets
  FOR nd = n_det, num_mds-1 DO BEGIN
     data_0 = GET_LV0_ONE_MDS_DATA( mds_det[nd], params, status=status, $
                                    coaddf=coadd_0, jday=jday_0 )

     IF status EQ 0 AND data_0[0] NE -1 THEN BEGIN
        IF sz[0] EQ 1 THEN BEGIN
           data_out[*,nd] = data_0
           IF params.GiveCoadd  THEN coaddf[*,nd] = coadd_0
           IF params.GiveJulian THEN jday[*,nd] = jday_0
        ENDIF ELSE BEGIN
           data_out[*,*,nd] = data_0
           IF params.GiveCoadd  THEN coaddf[*,*,nd] = coadd_0
           IF params.GiveJulian THEN jday[*,*,nd] = jday_0
        ENDELSE
     ENDIF
  ENDFOR

  IF params.GiveCoadd  THEN coaddf = REFORM( coaddf )
  IF params.GiveJulian THEN jday = REFORM( jday )
  RETURN, REFORM(data_out)
END
