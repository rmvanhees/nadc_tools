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
;	SCIA_HL_LV0_PIXELS
;
; PURPOSE:
;	This function reads detector readouts for given channelID and
;	pixelID from a sciamachy level 0 product
;
; CATEGORY:
;	NADC_TOOLS - SCIAMACHY - LEVEL 0
;
; CALLING SEQUENCE:
;	Result = SCIA_HL_LV0_PIXELS( flname, chanID, pixelID, jday=jday )
;
; INPUTS:
;      flname :  Filename of the Sciamachy level 0 product
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
; 	Written by:	Richard van Hees (SRON), 30 August 2005
;       Modified:  RvH, 31 August 2005
;                    added keywords COADD and NORM
;-
FUNCTION SCIA_HL_LV0_PIXELS, flname, chanID, pixelID, _EXTRA=EXTRA, $
                             status=status
  compile_opt idl2,logical_predicate,hidden

; check required parameters
  IF N_PARAMS() NE 3 THEN BEGIN
     MESSAGE, ' Usage: data = SCIA_HL_LV0_PIXELS( flname, chanID, pixelID' $
              + ', /coadd, /norm, status=status )', /INFO
     status = -1
     RETURN, -1
  ENDIF

; initialize
  num_data = 0
  struct = { info_data, $
             julian   :  0.d  ,$
             stateID  :  0b   ,$
             value    :  0.0  $
           }

; open file and read headers
  SCIA_HL_OPEN, flname, dsd, status=status
  IF status NE 0 THEN return, -1

; collect info about SCIAMACHY source packets
  flname = STRMID(flname, STRPOS( flname, '/', /REVERSE_SEARCH )+1)
  SCIA_LV0_RD_MDS_INFO, flname, dsd, info, status=status
  IF status NE 0 THEN return, -1

; remove non-detector readout info-records
  indxInfo = WHERE( info.packet_id EQ 1, infoCount )
  IF infoCount EQ 0 THEN return, -1
  info = info[indxInfo]

; get list of all uniq stateID's in the product
  stateIDs = info[UNIQ( info.state_id, SORT(info.state_id) )].state_id

  FOR ni = 0, N_ELEMENTS( stateIDs )-1 DO BEGIN
     SCIA_LV0_RD_DET, info, mds_det, state_id=stateIDs[ni], channel=chanID, $
                      status=status, count=count
     IF status NE 0 THEN return, -1
     values = GET_LV0_MDS_DATA( mds_det, channel=chanID, jday=jday, $
                                pixel=[pixelID,pixelID], _EXTRA=EXTRA, $
                                status=status )
     IF status EQ 0 THEN BEGIN
        num = N_ELEMENTS( jday )
        data_one = REPLICATE( {info_data}, num )
        data_one.julian = jday
        data_one.stateID = REPLICATE( stateIDs[ni], num )
        data_one.value = FLOAT(values)
        IF num_data EQ 0 THEN $
           data = data_one $
        ELSE $
           data = [data,data_one]
        num_data += num
     ENDIF
  ENDFOR
  SCIA_LV0_FREE_DET, mds_det

; close file
  stat = SCIA_FCLOSE()

; remove undefined values and sort the data according to Julian date
  indx = WHERE( data.julian GT 0 )
  data = data[indx]
  indx = SORT( data.julian )
  data = data[indx]

  RETURN, data
END
