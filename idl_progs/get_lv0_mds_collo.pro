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
;       GET_LV0_MDS_COLLO
;
; PURPOSE:
;       obtian index to MDS record given a julian date
;
; CATEGORY:
;       SCIAMACHY level 0
;
; CALLING SEQUENCE:
;       indx = GET_LV0_MDS_COLLO( jday, mds )
;
; INPUTS:
;      jday :    modified (decimal) julian date for the year 2000 (array)
;
;       mds :    an element or array of type structure
;                MDS0_AUX/MDS0_DET/MDS0_PMD
;
; KEYWORD PARAMETERS:
;    None
;
; OUTPUTS:
;       array of structures:
;           dt  :  (float) difference jday and collocated MDS records
;           i   :  (long)  index to the closest collocation in time
;           x   :  (long)  X-index (fastest) to the closest collocation
;           y   :  (long)  Y-index to the closest collocation
;           z   :  (long)  Z-index (slowest) to the closest collocation
;
; EXAMPLE:
;          jdate = GET_LV0_MDS_TIME( det[100] )
;
; obtain index to closest AUX-record
;          iindx = GET_LV0_MDS_COLLO( jdate, aux )
;   --> AUX[iindx.z].DATA_SRC[iindx.y].BCP[iindx.x]
;
; obtain index to closest PMD-record
;          iindx = GET_LV0_MDS_COLLO( jdate, aux )
;   --> PMD[iindx.y].DATA_SRC.PACKET[iindx.x]
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), 16 April 2002
;       Modified:  RvH, 19 September 2002
;                    removed confusing Info-message
;       Modified:  RvH, 19 September 2002
;                    updated documentation (added examples)
;       Modified:  RvH, 04 July 2005
;                    accepts an array of jday
;                    removed keyword dt_min (see output structure)
;                    returns structure
;-
FUNCTION GET_LV0_MDS_COLLO, jday, mds
  compile_opt idl2,logical_predicate,hidden

  scia_collo = { SciaCollo            $
                 , dt  :  0.d         $
                 , i   :  -1L         $
                 , x   :  0L          $
                 , y   :  0L          $
                 , z   :  0L          $
               }

; obtain array with decimal julian date of the MDS records
  mds_time = GET_LV0_MDS_TIME( mds )
  num_time = N_ELEMENTS( mds_time )
  valid_indx = WHERE( FINITE(mds_time), num_valid_time )
  IF num_valid_time EQ 0 THEN RETURN, {SciaCollo}

  num_jday = N_ELEMENTS( jday )
  indx_out = REPLICATE( {SciaCollo}, num_jday )

; find collocation(s)
  FOR nj = 0, num_jday-1 DO BEGIN
     abs_dtime = REPLICATE( !VALUES.D_NAN, num_time )
     abs_dtime[valid_indx] = ABS( mds_time[valid_indx] - jday[nj] )

     indx_out[nj].dt = MIN( abs_dtime, indx, /NaN )
     indx_out[nj].i  = indx 
  ENDFOR

  IF TAG_NAMES(mds, /struct) EQ 'MDS0_DET' THEN BEGIN
     indx_out.x = indx_out.i
     indx_out.y = -1
     indx_out.z = -1
  ENDIF ELSE IF TAG_NAMES(mds, /struct) EQ 'MDS0_PMD' THEN BEGIN
     indx_out.x = indx_out.i MOD !nadc.numLv0PMDpacket
     indx_out.y = indx_out.i / !nadc.numLv0PMDpacket
     indx_out.z = -1
  ENDIF ELSE IF TAG_NAMES(mds, /struct) EQ 'MDS0_AUX' THEN BEGIN
     xydim =  !nadc.numLv0AuxBCP * !nadc.numLv0AuxPMTC
     
     indx_out.x = (indx_out.i MOD xydim) MOD !nadc.numLv0AuxBCP
     indx_out.y = (indx_out.i MOD xydim) / !nadc.numLv0AuxBCP
     indx_out.z = indx_out.i / xydim
  ENDIF

  RETURN, indx_out
END
