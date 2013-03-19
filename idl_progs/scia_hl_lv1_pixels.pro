;
; COPYRIGHT (c) 2005 - 2013 SRON (R.M.van.Hees@sron.nl)
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
;	SCIA_HL_LV1_PIXELS
;
; PURPOSE:
;	This function reads detector readouts for given channelID and
;	pixelID from a sciamachy level 1 product
;
; CATEGORY:
;	NADC_TOOLS - SCIAMACHY - LEVEL 1
;
; CALLING SEQUENCE:
;	Result = SCIA_HL_LV1_PIXELS( flname, chanID, pixelID, jday=jday )
;
; INPUTS:
;      flname :  Filename of the Sciamachy level 1 product
;
;      chanID :  Channel ID
;
;     pixelID :  Pixel ID
;
; KEYWORD PARAMETERS:
;       coadd :  divide pixel counts by co-adding factor
;
;        norm :  divide pixel counts by co-adding factor times pixel
;                exposure time
;
;      status :  returns named variable with error status (0 = ok)
;
; OUTPUTS:
;	This function returns the detector readouts for a given
;	channel and pixelID
;
; PROCEDURE:
;	<add text here>
;
; EXAMPLE:
;	<add examples here>
;
; MODIFICATION HISTORY:
; 	Written by:	Richard van Hees (SRON), 31 August 2005
;-
FUNCTION SCIA_HL_LV1_PIXELS, flname, chanID, pixelID, _EXTRA=EXTRA, $
  status=status
  compile_opt idl2,logical_predicate,hidden

; check required parameters
  IF N_PARAMS() NE 3 THEN BEGIN
     MESSAGE, ' Usage: data = SCIA_HL_LV1_PIXELS( flname, chanID, pixelID' $
              + ', /coadd, /norm, status=status )', /INFO
     status = -1
     RETURN, -1
  ENDIF

; initialize
  num_data = 0
  struct = { info_data, $
             julian   :  0.d  ,$
             state_id :  0b   ,$
             value    :  0.0  $
           }

; open file and read headers
  SCIA_HL_OPEN, flname, dsd, status=status
  IF status NE 0 THEN return, -1
  
; read States of the Product
  SCIA_LV1_RD_STATE, dsd, state, status=status
  IF status NE 0 THEN NADC_ERR_TRACE

  state = state[WHERE( state.flag_mds EQ 0 )]
  state_ids = state[UNIQ( state.state_id )].state_id
  FOR ni = 0, N_ELEMENTS( state_ids )-1 DO BEGIN
     SCIA_LV1_RD_MDS, dsd, mds, status=status, $
                     state_id=state_ids[ni], _EXTRA=EXTRA
     IF status NE 0 THEN return, -1

     values = GET_LV1_MDS_DATA( mds, state_ids[ni], channel=chanID, $
                                jday=jday, pixel=[pixelID,pixelID], $
                                _EXTRA=EXTRA, status=status )
     IF status EQ 0 THEN BEGIN
        num = N_ELEMENTS( jday )
        data_one = REPLICATE( {info_data}, num )
        data_one.julian = REFORM( jday, num, /OVERWRITE )
        data_one.state_id = REPLICATE( state_ids[ni], num )
        data_one.value = REFORM( values, num, /OVERWRITE )
        IF num_data EQ 0 THEN $
           data = data_one $
        ELSE $
           data = [data,data_one]
        num_data += num
     ENDIF
  ENDFOR
  SCIA_LV1_FREE_MDS, mds

; close file
  stat = SCIA_FCLOSE()

; remove undefined values and sort the data according to Julian date
  indx = WHERE( data.julian GT 0 )
  data = data[indx]
  indx = SORT( data.julian )
  data = data[indx]

  RETURN, data
END
