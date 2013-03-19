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
;       GET_LV0_STATE_TEMP
;
; PURPOSE:
;       obtain house keeping data from level 0 MDS records
;
; CATEGORY:
;       SCIAMACHY level 0
;
; CALLING SEQUENCE:
;       GET_LV0_STATE_TEMP, info, temp_list, state_list=state_list
;
; INPUTS:
;       info  :    structure holding info about MDS records
; OUTPUTS:
;   temp_list :    temperatures (chan1..chan8,OBM,PMD) per state
;
; KEYWORD PARAMETERS:
;  state_list :    listing of stateID in SCIA LV0 product
;
; EXAMPLES:
;
; REFERENCE:
;       Measurement data definition and format description for
;       SCIAMACHY, volume 14. PO-ID-DOR-SY-0032
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), May 2006
;-
;
PRO GET_LV0_STATE_TEMP, info_in, temp_list, state_list=state_list
  compile_opt idl2,logical_predicate,hidden

;----- read detector array temperatures
  indx = WHERE( info_in.packet_id EQ 1, num_info )
  IF num_info EQ 0 THEN RETURN
  info = info_in[indx]
  state_list = info[UNIQ(info.state_id)].state_id
  temp_list = REPLICATE(!VALUES.F_NAN, $
                        (!nadc.scienceChannels+2), N_ELEMENTS(state_list) )

  indx_low = 0
  FOR ns = 0, N_ELEMENTS(state_list)-1 DO BEGIN
     indx_hgh = indx_low
     WHILE (indx_hgh+1) LT num_info DO BEGIN
        IF info[indx_hgh+1].state_id EQ state_list[ns] THEN $
           indx_hgh++ $
        ELSE $
           break
     ENDWHILE

     SCIA_LV0_RD_DET, info, mds_det, status=status, posit=[indx_low,indx_hgh]
     IF status EQ 0 THEN BEGIN
        temp = GET_LV0_MDS_HK( mds_det, /TEMP )

        sz = SIZE( temp )
        IF sz[2] GT 1 THEN BEGIN
           temp_list[0:!nadc.scienceChannels-1,ns] = MEDIAN( temp, DIM=2, /EVEN )
        ENDIF ELSE IF sz[1] EQ !nadc.scienceChannels THEN BEGIN
           temp_list[0:!nadc.scienceChannels-1,ns] = temp
        ENDIF
     ENDIF
     SCIA_LV0_FREE_DET, mds_det
     indx_low = indx_hgh
  ENDFOR
  print, N_ELEMENTS(state_list)

;----- read OBM temperatures
  indx = WHERE( info_in.packet_id EQ 2, num_info )
  IF num_info EQ 0 THEN RETURN
  info = info_in[indx]
  state_list = info[UNIQ(info.state_id)].state_id

  indx_low = 0
  FOR ns = 0, N_ELEMENTS(state_list)-1 DO BEGIN
     indx_hgh = indx_low
     WHILE (indx_hgh+1) LT num_info DO BEGIN
        IF info[indx_hgh+1].state_id EQ state_list[ns] THEN $
           indx_hgh++ $
        ELSE $
           break
     ENDWHILE

     SCIA_LV0_RD_AUX, info, mds_aux, status=status, posit=[indx_low,indx_hgh]
     IF status EQ 0 THEN BEGIN
        temp = GET_LV0_MDS_HK( mds_aux, /TEMP )

        temp_list[!nadc.scienceChannels,ns] = MEDIAN( temp, /EVEN )
     ENDIF
     indx_low = indx_hgh
  ENDFOR
  print, N_ELEMENTS(state_list)
  
;----- read PMD temperatures
  indx = WHERE( info_in.packet_id EQ 3, num_info )
  IF num_info EQ 0 THEN RETURN
  info = info_in[indx]
  state_list = info[UNIQ(info.state_id)].state_id

  indx_low = 0
  FOR ns = 0, N_ELEMENTS(state_list)-1 DO BEGIN
     indx_hgh = indx_low
     WHILE (indx_hgh+1) LT num_info DO BEGIN
        IF info[indx_hgh+1].state_id EQ state_list[ns] THEN $
           indx_hgh++ $
        ELSE $
           break
     ENDWHILE

     SCIA_LV0_RD_PMD, info, mds_pmd, status=status, posit=[indx_low,indx_hgh]
     IF status EQ 0 THEN BEGIN
        temp = GET_LV0_MDS_HK( mds_pmd, /TEMP )

        temp_list[!nadc.scienceChannels+1,ns] = MEDIAN( temp, /EVEN )
     ENDIF
     indx_low = indx_hgh
  ENDFOR
  print, N_ELEMENTS(state_list)

  RETURN
END
