;
; COPYRIGHT (c) 2002 - 2015 SRON (R.M.van.Hees@sron.nl)
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
;       SCIA_LV0_RD_DET
;
; PURPOSE:
;       read selected SCIAMACHY level 0 Detector MDS
;
; CATEGORY:
;       SCIAMACHY level 0
;
; CALLING SEQUENCE:
;       SCIA_LV0_RD_DET, info, mds_det, count=count, category=category, $
;                        state_id=state_id, channels=channels, period=period, $
;                        indx_state=indx_state, num_state=num_state, $
;                        status=status, 
;
; INPUTS:
;      info :    structure holding info about MDS records
;
; OUTPUTS:
;   mds_det :    structure with Detector MDS records (MDS0_DET)
;
; KEYWORD PARAMETERS:
;  count      :  number of selected MDS records
;  category   :  read only selected category of measurements (scalar or array)
;                (default or -1: all categories)
;  state_id   :  read only selected states (scalar or array)
;                (default or -1: all states)
;  indx_state :  named variable which contains the indices to the
;                original state records as defined in the level 1b/1c
;                product
;  num_state  :  named variable which contains the number of selected states
;  channels   :  read only selected channels (scalar or array)
;                (default or -1: all channels)
;  period     :  read only MDS within a time-window (scalar or array)
;                date must be given in decimal julian 2000 day
;                (default or -1: all data)
;  status     :  returns named variable with error status (0 = ok)
;
; SIDE EFFECTS:
;       mds_det contains pointers to data. These have to be handled
;       carefully. The routine SCIA_LV0_FREE_DET should be used to
;       release heap variables allocated by SCIA_LV0_RD_DET. The
;       program SCIA_LV0_RD_DET may be called more than once, using
;       the same output variable, because any valid pointer associated
;       with the output variable will be released using
;       SCIA_LV0_FREE_DET 
;
; EXAMPLES:
;       - read all Detector MDS 
;         (on exit, status should be equal to zero):
;       SCIA_LV0_RD, info, det, status=status
;
;       - release all memory allocated by the previous read action:
;       SCIA_LV0_FREE_DET, det
;
;       - readl all infra red Detector MDS
;       SCIA_LV0_RD, info, det, status=status, channel=[6,7,8]
;
;       - read only Nadir pointing measurements
;       SCIA_LV0_RD, info, det, status=status, category=1
;
;       - read only Detector MDS of particular state, within a time
;         window, and of these only the 2-nd and the fifth.
;       SCIA_LV0_RD, info, det, status=status, state_id=[3,65,70],
;       period=[702.6245629035,702.7645189], posit=[2,5]
;
;       - Read bitfield "packet_hdr.api"
;       vcid    = ISHFT(packet_hdr.api, -5) AND (NOT ISHFT(NOT 0us, 5))
;       op_mode = packet_hdr.api AND (NOT ISHFT(NOT 0us, 5))
;
;       - Read bitfield "data_hdr.rvd"
;       config_id = ISHFT(data_hdr.rvd, -8)
;       hsm       = ISHFT(data_hdr.rvd, -6) AND (NOT ISHFT(NOT 0us, 2))
;       atc_id    = data_hdr.rvd AND (NOT ISHFT(NOT 0us, 6))
;
;       - Read bitfield "data_hdr.id"
;        = ISHFT(data_hdr.id, -4) AND (NOT ISHFT(NOT 0us, 4))
;        = ISHFT(data_hdr.id, -8) AND (NOT ISHFT(NOT 0us, 4))
;
;       - Read bitfield "pmtc_hdr.pmtc_1"
;       ncwm  = ISHFT(pmtc_hdr.pmtc_1, -14)
;       apsm  = ISHFT(pmtc_hdr.pmtc_1, -12) AND (NOT ISHFT(NOT 0us, 2))
;       wls   = ISHFT(pmtc_hdr.pmtc_1, -10) AND (NOT ISHFT(NOT 0us, 2))
;       sls   = ISHFT(pmtc_hdr.pmtc_1, -8) AND (NOT ISHFT(NOT 0us, 2))
;       phase = ISHFT(pmtc_hdr.pmtc_1, -2) AND (NOT ISHFT(NOT 0us, 4))
;       ndfm  = pmtc_hdr.pmtc_1 AND (NOT ISHFT(NOT 0us, 2))
;
;       - Read bitfield "pmtc_hdr.az_param"
;       type   = ISHFT(pmtc_hdr.az_param, -31)
;       centre = ISHFT(pmtc_hdr.az_param, -30) AND (NOT ISHFT(NOT 0us, 1))
;       filter = ISHFT(pmtc_hdr.az_param, -29) AND (NOT ISHFT(NOT 0us, 1))
;       invert = ISHFT(pmtc_hdr.az_param, -28) AND (NOT ISHFT(NOT 0us, 1))
;       corr   = ISHFT(pmtc_hdr.az_param, -24) AND (NOT ISHFT(NOT 0us, 1))
;       rel    = ISHFT(pmtc_hdr.az_param, -20) AND (NOT ISHFT(NOT 0us, 4))
;       h_w    = ISHFT(pmtc_hdr.az_param, -16) AND (NOT ISHFT(NOT 0us, 4))
;       basic  = ISHFT(pmtc_hdr.az_param, -12) AND (NOT ISHFT(NOT 0us, 4))
;       repeat = pmtc_hdr.az_param AND (NOT ISHFT(NOT 0us, 12))
;
;       - Read bitfield "pmtc_hdr.elv_param"
;       centre = ISHFT(pmtc_hdr.elv_param, -30) AND (NOT ISHFT(NOT 0us, 1))
;       filter = ISHFT(pmtc_hdr.elv_param, -29) AND (NOT ISHFT(NOT 0us, 1))
;       invert = ISHFT(pmtc_hdr.elv_param, -28) AND (NOT ISHFT(NOT 0us, 1))
;       corr   = ISHFT(pmtc_hdr.elv_param, -24) AND (NOT ISHFT(NOT 0us, 4))
;       rel    = ISHFT(pmtc_hdr.elv_param, -20) AND (NOT ISHFT(NOT 0us, 4))
;       basic  = ISHFT(pmtc_hdr.elv_param, -12) AND (NOT ISHFT(NOT 0us, 4))
;       repeat = pmtc_hdr.elv_param AND (NOT ISHFT(NOT 0us, 12))
;
;       - Read bitfield "hdr.channel"
;       clusters = ISHFT(hdr.channel, -8)
;       chan_id  = ISHFT(hdr.channel, -4) AND (NOT ISHFT(NOT 0us, 4))
;       is       = ISHFT(hdr.channel, -2) AND (NOT ISHFT(NOT 0us, 2))
;       lu       = hdr.channel AND (NOT ISHFT(NOT 0us, 2))
;       
;       - Read bitfield "hdr.ratio_hdr"
;       frame  = ISHFT(hdr.ratio_hdr, -8)
;       ratio  = ISHFT(hdr.ratio_hdr, -3) AND (NOT ISHFT(NOT 0us, 5))
;       status = hdr.ratio_hdr AND (NOT ISHFT(NOT 0us, 3))
; 
;       - Read bitfield "hdr.command_vis"
;       etf   = ISHFT( hdr.command_vis, -18)
;       mode  = ISHFT(hdr.command_vis, -16) AND (NOT ISHFT(NOT 0us, 2))
;       sec   = ISHFT(hdr.command_vis, -7) AND (NOT ISHFT(NOT 0us, 9))
;       ratio = ISHFT(hdr.command_vis, -2) AND (NOT ISHFT(NOT 0us, 5))
;       cntrl = hdr.command_vis AND (NOT ISHFT(NOT 0us, 2))
;       
;       - Read bitfield "hdr.command_ir"
;       etf   = ISHFT(hdr.command_ir, -18)
;       mode  = ISHFT(hdr.command_ir, -16) AND (NOT ISHFT(NOT 0us, 2))
;       comp  = ISHFT(hdr.command_ir, -14) AND (NOT ISHFT(NOT 0us, 2))
;       bias  = ISHFT(hdr.command_ir, -8) AND (NOT ISHFT(NOT 0us, 3))
;       pet   = ISHFT(hdr.command_ir, -2) AND (NOT ISHFT(NOT 0us, 4))
;       cntrl = hdr.command_ir AND (NOT ISHFT(NOT 0us, 3))
;
; REFERENCE:
;       Measurement data definition and format description for
;       SCIAMACHY, volume 14. PO-ID-DOR-SY-0032
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), February 2002
;       Modified:  RvH, February 2002
;                    Uses the new function GET_KEY_CHAN_HDR
;       Modified:  RvH, February 2002
;                    added keyword posit
;       Modified:  RvH, 19 March 2002
;                    obsoletes function GET_KEY_CHAN_HDR, added examples
;       Modified:  RvH, 21 March 2002
;                    set keywords to -1 results in no selection
;       Modified:  RvH, 22 March 2002
;                    debugged data set position selection
;       Modified:  RvH, 31 May 2002
;                    added keyword count, which returns the number of
;                    MDS records
;       Modified:  RvH, 4 October 2002
;                    modified struct chan_hdr: renamed "counter" to "bcps"
;       Modified:  RvH, 25 February 2003
;                    added check on negative "posit" values
;       Modified:  RvH, 24 March 2003
;                    check: 0 <= posit[0] <= posit[1] < array-size
;       Modified:  RvH, 30 August 2005
;                    fixed several "brown-paperbag" bugs:
;                     - dit not release memeory properly
;                     - did not select mds_det for given channel
;   	Modified: JMK, 22 Feb 2010
;   	  	     keyword added to return selected info, info_det
;       Modified:  RvH, 22 March 2010
;                    added keywords: state_posit, indx_state, num_state
;                    use new function GET_LV0_MDS_STATE
;       Modified:  RvH, March 2015
;                Works again for nadc_tools v2.x; removed obsolete keywords
;-
;---------------------------------------------------------------------------
FUNCTION _GET_LV0_DET_SIZE, state_id, category, orbit, channels
 compile_opt idl2,logical_predicate 

 ; obtain Sciamachy cluster definition
 SCIA_CLUSDEF, state_id, orbit

 duration = ULONG(!scia.duration)
 IF category EQ 2 OR category EQ 26 THEN BEGIN
    duration -= 2
    num_scan = FIX(duration / 27)
    duration -= (num_scan-1) * 3
 ENDIF
 
 sz_data = 0UL
 FOR ncl = 0, !scia.num_clus-1 DO BEGIN
    IF WHERE(channels EQ !scia.clusDef[ncl].chan_id) LT 0 THEN $
       CONTINUE

    intg = ULONG(!scia.clusDef[ncl].intg)
    IF !scia.clusDef[ncl].pet EQ 0.03125 $
       AND !scia.clusDef[ncl].coaddf > 1 THEN intg *= 2

    sz_data += !scia.clusDef[ncl].length * (duration / intg)
 ENDFOR

 RETURN, sz_data
END

;--------------------------------------------------
PRO SCIA_LV0_RD_DET, info_all, mds_det, count=count, category=category, $
                     state_id=state_id, period=period, channels=channels, $
                     indx_state=indx_state, num_state=num_state, $
                     status=status
  compile_opt idl2,logical_predicate,hidden

; initialize the return values
  indx_state = -1
  num_state = 0
  status = 0

; check required parameters
  IF N_PARAMS() NE 2 THEN BEGIN
     MESSAGE, ' Usage: scia_lv0_rd_det, info, mds_det, count=count' $
              + ', category=category, state_id=state_id' $
              + ', channels=channels, period=period' $
              + ', indx_state=indx_state, num_state=num_state' $
              + ', status=status', /INFO
     status = -1
     RETURN
  ENDIF
  NotSet = -1

; release previous allocated MDS data
  SCIA_LV0_FREE_DET, mds_det

  IF N_ELEMENTS( channels ) EQ 0 THEN BEGIN    ; select data of all channels
     channels = 1B + bindgen(!nadc.scienceChannels)
  ENDIF ELSE IF channels[0] EQ -1 THEN BEGIN   ; select data of all channels
     channels = 1B + bindgen(!nadc.scienceChannels)
  ENDIF ELSE BEGIN
     channels = BYTE(channels)
  ENDELSE

; select Detector source packets
  info_det = GET_LV0_MDS_STATE( info_all, period=period, $
                                category=category, state_id=state_id, $
                                indx_state=indx_state, num_state=num_state )
  IF num_state LE 0 THEN BEGIN
     MESSAGE, 'No state-records found for given selection criteria', /INFO
     status = -1
     RETURN
  ENDIF
  num_det = N_ELEMENTS( info_det )
  IF num_det LE 0 THEN RETURN

; allocate memory for the Detector MDS records
  mds_det = replicate( {mds0_det}, num_det )

; obtain orbit number
  SCIA_RD_MPH, mph

; determine size of the data array
  sz_data = 0UL
  FOR nd = 0L, num_det-1 DO BEGIN
     sz_data += _GET_LV0_DET_SIZE( info_det[nd].state_id, $
                                   info_det[nd].category,$
                                   mph.abs_orbit, channels )

     on_board_time = info_det[nd].on_board_time
     REPEAT BEGIN
        nd += 1L
        IF nd GE num_det THEN break
     ENDREP UNTIL info_det[nd].on_board_time NE on_board_time
  ENDFOR
  data = ULONARR( sz_data )

; read Detector source packets
  count = call_external( lib_name('libnadc_idl'), '_SCIA_LV0_RD_DET', $
                         N_ELEMENTS(channels), channels, info_det, $
                         num_det, mds_det, data, /CDECL, /UL_VALUE )
; check error status
  IF count NE num_det THEN BEGIN
     status = -1
     RETURN
  ENDIF

  offs = 0UL
  FOR nd = 0L, num_det-1 DO BEGIN
     num_chan = mds_det[nd].num_chan
     FOR n_ch = 0L, num_chan-1 DO BEGIN
        num_clus = ISHFT( mds_det[nd].data_src[n_ch].hdr.channel, -8 )
        FOR n_cl = 0L, num_clus-1 DO BEGIN
           length = ULONG(mds_det[nd].data_src[n_ch].cluster[n_cl].length)
           mds_det[nd].data_src[n_ch].cluster[n_cl].data = $
              PTR_NEW( data[offs:offs+length-1] )
           offs = offs + length
        ENDFOR
     ENDFOR
  ENDFOR

  RETURN
END
