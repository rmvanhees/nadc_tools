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
;       GET_LV0_MDS_HK
;
; PURPOSE:
;       obtain house keeping data from level 0 MDS records
;
; CATEGORY:
;       SCIAMACHY level 0
;
; CALLING SEQUENCE:
;       Result = GET_LV0_MDS_HK( mds [, posit=] [, status=] [, channel=] 
;                                [, /pet] [, /ir_pet] [, virtual=]
;                                [, /temp] [, /asm] [, /esm]
;                                [, /rad] [, /az] [, /elv] [, /obm] )
; INPUTS:
;       mds :    an element or array of type structure
;                MDS0_AUX/MDS0_DET/MDS0_PMD 
;
; KEYWORD PARAMETERS:
;     posit :    relative index or index-range [low,high] to MDS record(s)
;    status :    returns named variable with error status (0 = ok)
;
;     [MDS0_DET]
;   channel :    a scalar of type integer specifying ID of the
;                channel to be extracted, valid range = [1:8]
;       pet :    return pixel exposure time
;    ir_pet :    return pixel exposure time - chan 6-8 corrected
;                ref. TN-SCIA-0000DO/19,10.03.1999
;   virtual :    returns named variable indiacting the usage of
;                virtual channels, virtual equals zero if no virtual
;                channels are used. Only in combination with /pet and
;                for channel 1 and 2. 
;      temp :    return temperature of Detector block (K)
;
;     [MDS0_PMD]
;      temp :    return temperature of PMD electronics (K)
;
;     [MDS0_AUX]
;       asm :    return azimuth scan mirror position (degrees)
;       esm :    return elevation scan mirror position (degrees)
;  rad_temp :    return temperature near radiator (K)
;   az_temp :    return temperature near AZ scanner (K)
;  elv_temp :    return temperature near ELV scanner (K)
;[obm_]temp :    return temperature of the optical bench (K),
;                       (combination of AZ, ELV and RAD).
;
; EXAMPLES:
;       - extract temperature for all eight detector arrays
;            result = GET_LV0_MDS_HK( mds_det, /temp )
;
;       - extract temperature of channel 1 detector array
;            result = GET_LV0_MDS_HK( mds_det, /temp, channel=1 )
;
;       - extract temperature of the PMD electronics
;            result = GET_LV0_MDS_HK( mds_pmd, /temp )
;
;       - extract temperature of the optical bench
;            result = GET_LV0_MDS_HK( mds_aux, /temp )
;
;       - extract azimuth scan mirror positions
;            result = GET_LV0_MDS_HK( mds_aux, /asm )
;
; REFERENCE:
;       Measurement data definition and format description for
;       SCIAMACHY, volume 14. PO-ID-DOR-SY-0032
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), March 2002
;       Modified:  RvH added optical bench temperature, March 2002
;       Modified:  RvH, 19 March 2002
;                    Handle uint to float conversion correctly (MDS0_AUX)
;       Modified:  RvH, 19 March 2002
;                    replaced confusing bench_obm with bench_rad
;       Modified:  RvH, 19 March 2002
;                    check return values of GET_LV0_DET_TEMP
;       Modified:  RvH, 21 March 2002
;                    set keywords to -1 results in no selection
;       Modified:  RvH, 21 March 2002
;                    added keywords "esm" and "asm"
;       Modified:  RvH, 25 March 2002
;                    keyword /obm for Auxiliaray MDS did not work.
;       Modified:  RvH, 19 April 2002
;                    implemented PET for virtual channels
;       Modified:  RvH, 24 March 2003
;                    check: 0 <= posit[0] <= posit[1] < array-size
;       Modified:  RvH, 20 Januari 2004
;                    changed the azimuth offset according to an update
;                    of the PGS-ICD, dated 12.12.2001 
;       Modified:  RvH, 12 Augustus 2004
;                    apply correction to the PET of channel 6-8
;                    The correct integration-time can be obtained by
;                    using the corrected PET multiplied with the
;                    coadding factor, only for option /ir_pet
;                    ref. TN-SCIA-0000DO/19,10.03.1999
;       Modified: RvH, 3 March 2010
;                    fixed long standing bug: mixing radTemp & azTemp
;                    fixed computation of OBM temperature
;-
; +++++++++++++++++++++++++ LOCAL FUNCTIONS +++++++++++++++++++++++++
FUNCTION GET_LV0_DET_PET, chan_hdr, chan_id, virtual=virtual, $
                          ir_corr=ir_corr
  compile_opt idl2,logical_predicate,hidden

  idList = ISHFT(chan_hdr.channel, -4) AND (NOT ISHFT(NOT 0us, 4))
  chan_indx = WHERE( idList EQ chan_id, num_chan )
  IF num_chan LE 0 THEN BEGIN
     virtual = 0
     RETURN, !VALUES.F_NaN 
  ENDIF

; obtain number of PET values to be returned
  sz = SIZE( chan_hdr.channel )
  IF sz[0] EQ 1 THEN $
     num_pet = 1 $
  ELSE $
     num_pet = sz[2]
;
; define output arrays
  virtual = UINTARR( num_pet )
  pet = REPLICATE( !VALUES.F_NaN, num_pet )

;
; use reflected detector command word for channel 1 to 5
;
  IF chan_id LE 5 THEN BEGIN
     etf = UINT(ISHFT( chan_hdr[chan_indx].command_vis, -18 ))
     ratio = UINT(ISHFT( chan_hdr[chan_indx].command_vis, -2 )) $
             AND ISHFT( NOT 0us, -11)
     section = UINT(ISHFT( chan_hdr[chan_indx].command_vis, -7 )) $
               AND ISHFT( NOT 0us, -7)
     chan_indx = chan_indx / !nadc.scienceChannels

     s_indx = WHERE( ratio NE 1us, count )
     IF count EQ 0 THEN BEGIN
        indx = WHERE( etf EQ 0us, count )
        IF count GT 0 THEN pet[chan_indx[indx]] = 31.25e-3
        indx = WHERE( etf NE 0us, count )
        IF count GT 0 THEN $
           pet[chan_indx[indx]] = 62.5e-3 * etf[indx] * ratio[indx]
     ENDIF ELSE IF WHERE(section[s_indx] EQ 0) NE -1 THEN BEGIN
        indx = WHERE( etf EQ 0us, count )
        IF count GT 0 THEN pet[chan_indx[indx]] = 31.25e-3
        indx = WHERE( etf NE 0us, count )
        IF count GT 0 THEN $
           pet[chan_indx[indx]] = 62.5e-3 * etf[indx]        
     ENDIF ELSE  BEGIN
        pet = REPLICATE( !VALUES.F_NaN, 2, num_pet )

        indx = WHERE( etf EQ 0us, count )
        IF count GT 0 THEN pet[0, chan_indx[indx]] = 31.25e-3
        indx = WHERE( etf NE 0us, count )
        IF count GT 0 THEN BEGIN
           pet[0, chan_indx[indx]] = 62.5e-3 * etf[indx] * ratio[indx]
           pet[1, chan_indx[indx]] = 62.5e-3 * etf[indx]
        ENDIF
        virtual[chan_indx[s_indx]] = section[s_indx] * 2
     ENDELSE
  ENDIF ELSE BEGIN
     etf = UINT(ISHFT( chan_hdr[chan_indx].command_ir, -18 ))
     mode = UINT(ISHFT( chan_hdr[chan_indx].command_ir, -16 )) $
            AND ISHFT( NOT 0us, -14 )
     spet = UINT(ISHFT( chan_hdr[chan_indx].command_ir, -2 )) $
            AND ISHFT( NOT 0us, -12 )
     chan_indx = chan_indx / !nadc.scienceChannels

     indx = WHERE( mode LT 0 OR mode GT 1, count )
     IF count GT 0 THEN pet[chan_indx[indx]] = -1.
     indx = WHERE( mode EQ 1, count )
     IF count GT 0 THEN pet[chan_indx[indx]] = 28.125e-6 * 2^(spet[indx] < 10)
     indx = WHERE( mode EQ 0 AND etf EQ 0, count )
     IF count GT 0 THEN pet[chan_indx[indx]] = 31.25e-3
     indx = WHERE( mode EQ 0 AND etf NE 0, count )
     IF count GT 0 THEN pet[chan_indx[indx]] = 62.5e-3 * etf[indx]

     IF (KEYWORD_SET( ir_corr )) THEN BEGIN
;PET offset in EPITAXX detectors (TN-SCIA-0000DO/19,10.03.1999)
        PET_OFFSET = 1.18125e-3

;apply EPITAXX offset for ch. 6-8 and PET >= 31.25ms
        indx = WHERE( pet ge 0.03125, count )
        IF count GT 0 THEN pet[indx] -= PET_OFFSET
     ENDIF
  ENDELSE

  RETURN, pet
END

;++++++++++++++++++++++++++++++
PRO GET_LV0_DET_TEMP, chan_hdr, chan_id, temp
  compile_opt idl2,logical_predicate,hidden

  temp = !VALUES.F_NaN
  idList = ISHFT(chan_hdr.channel, -4) AND (NOT ISHFT(NOT 0us, 4))
  chan_indx = WHERE( idList EQ chan_id, count )
  IF count LE 0 THEN RETURN

  sz = SIZE( chan_hdr.channel )
  IF sz[0] EQ 2 THEN temp = REPLICATE( !VALUES.F_NaN, sz[2] )

  SWITCH chan_id OF
     1: BEGIN
        tab_tm = [0, 17876, 18312, 18741, 19161, 19574, 19980, 20379, $
                  20771, 21157, 21908, 22636, 24684, 26550, 28259, 65535]
        tab_temp = [179., 180., 185., 190., 195., 200., 205., 210., $
                    215., 220., 230., 240., 270., 300., 330., 331.]  
        BREAK
     END
     2: BEGIN
        tab_tm = [0, 18018, 18456, 18886, 19309, 19724, 20131, 20532, $
                  20926, 21313, 22068, 22798, 24852, 26724, 28436, 65535]
        tab_temp = [179., 180., 185., 190., 195., 200., 205., 210., $
                    215., 220., 230., 240., 270., 300., 330., 331.]
        BREAK
     END
     3: BEGIN
        tab_tm = [0, 20601, 20996, 21384, 21765, 22140, 22509, 22872, $
                  23229, 23581, 23927, 24932, 26201, 27396, 28523, 65535]
        tab_temp = [209., 210., 215., 220., 225., 230., 235., 240., $
                    245., 250., 255., 270., 290., 310., 330., 331.]
        BREAK
     END
     4: BEGIN
        tab_tm = [0, 20333, 20725, 21110, 21490, 21863, 22230, 22591, $
                  22946, 23295, 23640, 24640, 25905, 27097, 28222, 65535]
        tab_temp = [209., 210., 215., 220., 225., 230., 235., 240., $
                    245., 250., 255., 270., 290., 310., 330., 331.]
        BREAK
     END
     5: BEGIN
        tab_tm = [0, 20548, 20942, 21330, 21711, 22086, 22454, 22817, $
                  23174, 23525, 23871, 24875, 26144, 27339, 28466, 65535]
        tab_temp = [209., 210., 215., 220., 225., 230., 235., 240., $
                    245., 250., 255., 270., 290., 310., 330., 331.]
        BREAK
     END
     6: BEGIN
        tab_tm = [0, 17893, 18329, 18758, 19179, 19593, 20000, 20399, $
                  20792, 21178, 21931, 22659, 24709, 26578, 28289, 65535]
        tab_temp = [179., 180., 185., 190., 195., 200., 205., 210., $
                    215., 220., 230., 240., 270., 300., 330., 331.]
        BREAK
     END
     7: BEGIN
        tab_tm = [0, 12994, 13526, 14046, 14555, 15054, 15543, 16022, $
                  16492, 17850, 20352, 22609, 24656, 26523, 28232, 65535]
        tab_temp = [129., 130., 135., 140., 145., 150., 155., 160., $
                    165., 180., 210., 240., 270., 300., 330., 331.]
        BREAK
     END
     8: BEGIN
        tab_tm = [0, 13129, 13664, 14188, 14702, 15204, 15697, 16180, $
                  16653, 18019, 20536, 22804, 24860, 26733, 28447, 65535]
        tab_temp = [129., 130., 135., 140., 145., 150., 155., 160., $
                    165., 180., 210., 240., 270., 300., 330., 331.]
        BREAK
     END
     ELSE: BEGIN
        RETURN
     END
  ENDSWITCH

  itemp = FLOAT( chan_hdr[chan_indx].temp )
  IF SIZE( itemp, /N_DIM ) GT 0 THEN BEGIN
     itemp = REFORM([itemp])
  ENDIF
  temp[chan_indx / sz[1]] = INTERPOL( tab_temp, tab_tm, itemp )

  RETURN
END

;+++++++++++++++++++++++++ MAIN FUNCTION +++++++++++++++++++++++++
FUNCTION GET_LV0_MDS_HK, mds_in, channel=channel, posit=posit, $
                         pet=pet, ir_pet=ir_pet, virtual=virtual, $
                         esm=esm, asm=asm, temp=temp, $
                         rad_temp=rad_temp, obm_temp=obm_temp, $
                         az_temp=az_temp, elv_temp=elv_temp, $
                         status=status
  compile_opt idl2,logical_predicate,hidden
                       
; set some constants
  NotSet  = -1

; initialisation of status value and return values
  status = 0
  data_out = NotSet
  num_mds = N_ELEMENTS( mds_in )

  IF N_PARAMS() NE 1 THEN BEGIN
     MESSAGE, ' Usage: GET_LV0_MDS_HK, mds, posit=posit, channel=channel' $
              + ', /temp, /rad_tem, /az_temp, /elv_temp, /obm_temp' $
              + ', /pet, /ir_pet, /esm, /asm, status=status', /INFO
     status = -1
     RETURN, data_out
  ENDIF

; check input data array
  IF num_mds EQ 0 OR SIZE( mds_in, /TNAME ) NE 'STRUCT' THEN BEGIN
     MESSAGE, 'Fatal: input array MDS_IN is empty or not a structure', /INFO
     status = 1
     RETURN, data_out
  ENDIF

; check IDL keywords
  IF (~ KEYWORD_SET( pet )) AND (~ KEYWORD_SET( ir_pet )) THEN $
     SetPET = NotSet $
  ELSE $
     SetPET = 0

  IF (~ KEYWORD_SET( rad_temp )) THEN SetRAD = NotSet ELSE SetRAD = 0
  IF (~ KEYWORD_SET( az_temp )) THEN SetAZ = NotSet ELSE SetAZ = 0
  IF (~ KEYWORD_SET( elv_temp )) THEN SetELV = NotSet ELSE SetELV = 0
  IF (~ KEYWORD_SET( obm_temp )) THEN SetOBM = NotSet ELSE SetOBM = 0
  IF (~ KEYWORD_SET( temp )) THEN SetTemp = NotSet ELSE SetTemp = 0
  
  IF (~ KEYWORD_SET( esm )) THEN SetESM = NotSet ELSE SetESM = 0
  IF (~ KEYWORD_SET( asm )) THEN SetASM = NotSet ELSE SetASM = 0

  IF N_ELEMENTS( channel ) EQ 0 THEN $
     channel = NotSet $
  ELSE $
     channel = channel[0]

  IF N_ELEMENTS( posit ) EQ 0 THEN BEGIN
     mds = mds_in
  ENDIF ELSE IF N_ELEMENTS( posit ) EQ 1 THEN BEGIN
     IF posit[0] LT 0 OR posit[0] GE num_mds THEN BEGIN
        MESSAGE, 'Attempt to use subscript POSIT out of range.', /INFO
        status = 1
        RETURN, data_out
     ENDIF
     mds = mds_in[posit[0]]
     num_mds = 1
  ENDIF ELSE BEGIN
     IF posit[0] LT 0 OR posit[0] GE num_mds THEN BEGIN
        MESSAGE, 'Attempt to use subscript POSIT[0] out of range.', /INFO
        status = 1
        RETURN, data_out
     ENDIF
     IF posit[1] LT 0 OR posit[1] GE num_mds THEN BEGIN
        MESSAGE, 'Attempt to use subscript POSIT[1] out of range.', /INFO
        status = 1
        RETURN, data_out
     ENDIF
     IF posit[0] GT posit[1] THEN BEGIN
        MESSAGE, 'Subscript range values of the form low:high must be' $
                 + ' low <= high', /INFO
        status = 1
        RETURN, data_out
     ENDIF
     num_mds = posit[1] - posit[0] + 1
     mds = mds_in[posit[0]:posit[1]]
  ENDELSE
;
; process data
;
  IF TAG_NAMES(mds, /struct) EQ 'MDS0_DET' THEN BEGIN
     chan_hdr = REFORM([mds.data_src.hdr])

     IF SetPET NE NotSet THEN BEGIN
        IF channel NE NotSet THEN BEGIN
           IF KEYWORD_SET( ir_pet ) THEN BEGIN
              data_out = GET_LV0_DET_PET( chan_hdr, channel, $
                                          virtual=virtual, /ir_corr )
           ENDIF ELSE BEGIN
              data_out = GET_LV0_DET_PET( chan_hdr, channel, $
                                          virtual=virtual )
           ENDELSE
        ENDIF ELSE BEGIN
           sz = SIZE( chan_hdr )
           IF sz[0] EQ 1 THEN BEGIN 
              data_out = FLTARR( sz[1] )
           ENDIF ELSE BEGIN
              IF sz[2] GT 1 THEN $
                 data_out = FLTARR( sz[1], sz[2] ) $
              ELSE $
                 data_out = FLTARR( sz[1] )
           ENDELSE
           FOR n_chan = 1, sz[1] DO BEGIN
              IF KEYWORD_SET( ir_pet ) THEN BEGIN
                 data_out[n_chan-1,*] = GET_LV0_DET_PET( chan_hdr, n_chan, $
                                                         virtual=virtual, $
                                                         /ir_corr )
              ENDIF ELSE BEGIN
                 data_out[n_chan-1,*] = GET_LV0_DET_PET( chan_hdr, n_chan, $
                                                         virtual=virtual )
              ENDELSE
           ENDFOR
        ENDELSE
     ENDIF ELSE IF SetTemp NE NotSet THEN BEGIN
        IF channel NE NotSet THEN BEGIN
           GET_LV0_DET_TEMP, chan_hdr, channel, data_out
        ENDIF ELSE BEGIN
           sz = SIZE( chan_hdr )
           IF sz[0] EQ 1 THEN $
              data_out = REPLICATE( !VALUES.F_NaN, sz[1] ) $
           ELSE $
              data_out = REPLICATE( !VALUES.F_NaN, sz[1], sz[2] )
           FOR n_chan = 1, sz[1] DO BEGIN
              GET_LV0_DET_TEMP, chan_hdr, n_chan, temp
                data_out[n_chan-1,*] = temp
             ENDFOR
        ENDELSE
     ENDIF
  ENDIF ELSE IF TAG_NAMES(mds, /struct) EQ 'MDS0_AUX' THEN BEGIN
     xdim = 5
     ydim = num_mds
     IF SetESM NE NotSet THEN BEGIN
        scale = 360.d / 640000.d
        sz = SIZE( mds.data_src.bcp.ele_encode_cntr )
        data_buff = DBLARR( sz[1], sz[2], /NOZERO )
        IF sz[0] EQ 2 THEN $
           data_out = DBLARR( sz[1], sz[2], /NOZERO ) $
        ELSE $
           data_out = DBLARR( sz[1], sz[2], sz[3], /NOZERO )
        FOR ii = 0, num_mds-1 DO BEGIN
           conf_id = ISHFT( mds[ii].data_hdr.rdv, -8 )
           IF (conf_id MOD 2) EQ 1 THEN $
              el_off = -19.2340 $
           ELSE $
              el_off = -109.2425

           el_cntr = mds[ii].data_src.bcp.ele_encode_cntr
           indx = WHERE( el_cntr NE 0, count )

           REPLICATE_INPLACE, data_buff, !VALUES.D_NaN
           IF count GT 0 THEN $
              data_buff[indx] = scale * el_cntr[indx] + el_off
           data_out[*,*,ii] = data_buff
        ENDFOR
     ENDIF ELSE IF SetASM NE NotSet THEN BEGIN
        scale = 360.d / 640000.d
        sz = SIZE( mds.data_src.bcp.ele_encode_cntr )
        data_buff = DBLARR( sz[1], sz[2], /NOZERO )
        IF sz[0] EQ 2 THEN $
           data_out = DBLARR( sz[1], sz[2], /NOZERO ) $
        ELSE $
           data_out = DBLARR( sz[1], sz[2], sz[3], /NOZERO )
        FOR ii = 0, num_mds-1 DO BEGIN
           conf_id = ISHFT( mds[ii].data_hdr.rdv, -8 )
           IF (conf_id MOD 2) EQ 1 THEN $
              az_off = -108.18143 $
           ELSE $
              az_off = -18.18943

           az_cntr = mds[ii].data_src.bcp.azi_encode_cntr
           indx = WHERE( az_cntr NE 0, count )

           REPLICATE_INPLACE, data_buff, !VALUES.D_NaN
           IF count GT 0 THEN $
              data_buff[indx] = scale * az_cntr[indx] + az_off
           data_out[*,*,ii] = data_buff
        ENDFOR
     ENDIF ELSE BEGIN
        IF SetTemp NE NotSet THEN SetOBM = 0
        az_temp = REPLICATE( !VALUES.D_NaN, xdim, ydim )
        IF SetOBM NE NotSet OR SetAZ NE NotSet THEN BEGIN
           CO = 74419.32288d
           QP = 8.d / 65536UL
           FF = 0.184583046d

           aa = 9.3590d-4
           bb = 2.2119d-4
           cc = 1.2683d-7

           tm_data = ISHFT(REFORM([mds.data_src.bench_az]), -1 )
           R_l = CO * ( 2.d * tm_data * QP - FF )
           R_t = (1e6 * R_l) / (1e6 - R_l)
           indx = WHERE( R_t GE 1.d-12, count )
           IF count GT 0 THEN BEGIN
              az_temp[indx] = $
                 1.d / (aa + bb * ALOG(R_t[indx]) + cc * ALOG(R_t[indx])^3)
           ENDIF
           IF SetTemp EQ NotSet THEN  data_out = az_temp
        ENDIF 
        elv_temp = REPLICATE( !VALUES.D_NaN, xdim, ydim )
        IF SetOBM NE NotSet OR SetELV NE NotSet THEN BEGIN
           CO = 74439.72096d
           QP = 8.d / 65536UL
           FF = 0.184288384d

           aa = 9.2998d-4
           bb = 2.2188d-4
           cc = 1.2568d-7

           tm_data = ISHFT(REFORM([mds.data_src.bench_elv]), -1 )
           R_l = CO * ( 2.d * tm_data * QP - FF )
           R_t = (1e6 * R_l) / (1e6 - R_l)
           indx = WHERE( R_t GE 1.d-12, count )
           IF count GT 0 THEN BEGIN
              elv_temp[indx] = $
                 1.d / (aa + bb * ALOG(R_t[indx]) + cc * ALOG(R_t[indx])^3)
           ENDIF
           IF SetTemp EQ NotSet THEN  data_out = elv_temp
        ENDIF
        rad_temp = REPLICATE( !VALUES.D_NaN, xdim, ydim )
        IF SetOBM NE NotSet OR SetRAD NE NotSet THEN BEGIN
           CO = 74423.0912d
           QP = 8.d / 65536UL
           FF = 0.183497965d

           aa = 9.3809d-4
           bb = 2.2099d-4
           cc = 1.2655d-7

           tm_data = ISHFT(REFORM([mds.data_src.bench_rad]), -1 )
           R_l = CO * ( 2.d * tm_data * QP - FF )
           R_t = (1e6 * R_l) / (1e6 - R_l)
           indx = WHERE( R_t GE 1.d-12, count )
           IF count GT 0 THEN BEGIN
              rad_temp[indx] = $
                 1.d / (aa + bb * ALOG(R_t[indx]) + cc * ALOG(R_t[indx])^3)
           ENDIF
           IF SetTemp EQ NotSet THEN  data_out = rad_temp
        ENDIF
        IF SetOBM NE NotSet THEN BEGIN
           data_out = REPLICATE( !VALUES.D_NaN, xdim, ydim )
           indx = WHERE( FINITE(az_temp) AND FINITE(elv_temp), count )
           IF count GT 0 THEN BEGIN
              data_out[indx] = 0.5 * (az_temp[indx] + elv_temp[indx]) - 2.2
           ENDIF
        ENDIF
     ENDELSE
  ENDIF ELSE IF TAG_NAMES(mds, /struct) EQ 'MDS0_PMD' THEN BEGIN
     IF SetTemp NE NotSet THEN BEGIN
        tab_tm = [1008, 1762, 2167, 3448, 5446, 8971, 13940, 14871, 15828, $
                  16809, 17812, 20761, 23208, 25944, 27960, 30884]
        tab_temp = [60., 45., 41., 27., 13., -2., -16., -18., -20., -22., $
                    -24., -30., -35., -41., -46., -55.]

        tm_data = FLOAT( REFORM([mds.data_src.temp]))
        data_out = INTERPOL( tab_temp + 273.15, tab_tm, tm_data )
     ENDIF
  ENDIF

  RETURN, data_out
END
