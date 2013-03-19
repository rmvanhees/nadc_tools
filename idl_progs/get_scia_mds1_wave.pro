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
;       GET_SCIA_MDS1_WAVE
;
; PURPOSE: 
;        obtain wavelength grid for a channel
;
; CATEGORY:
;        SCIA level 1b/1c data
;
; CALLING SEQUENCE:
;          result = GET_SCIA_MDS1_WAVE( dsd, chanID )
;
; INPUTS:
;         dsd :  structure for Data Set Descriptors
;      chanID :  channel number of science data to be returned (scalar)
;
; KEYWORD PARAMETERS:
;     stateID :  read selected state (scalar), ignored when stateIndex is given
;  stateIndex :  named variable which contains the indices to the
;                original state records as defined in the level 1b/1c
;                product, overrules stateID
;      status : named variable which contains the error status (0 = ok)
;
; EXAMPLE:
;       None
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), February 2007
;-
FUNCTION GET_SCIA_MDS1_WAVE, dsd, chanID, status=status, $
                             stateID=stateID, stateIndex=stateIndex
  compile_opt idl2,logical_predicate,hidden

; check level of the Sciamachy product
  scia_level = GET_SCIA_LEVEL()
  IF scia_level NE '1B' AND scia_level NE '1C' THEN BEGIN
     MESSAGE, 'FATAL: Input file is not a valid level 1B/1C product', /INFO
     status = -1
     RETURN, -1
  ENDIF

; check required parameters
  IF N_PARAMS() NE 2 THEN BEGIN
     MESSAGE, ' Usage: data = GET_SCIA_MDS1_WAVE( dsd, chanID' $
              + ', stateID=stateID, stateIndex=stateIndex, status=status )', $
              /INFO
     status = -1
     RETURN, -1
  ENDIF
  chanID = FIX(chanID[0])

; read States of the Product
  SCIA_LV1_RD_STATE, dsd, state, status=status
  IF status NE 0 THEN BEGIN
     status = -1
     RETURN, -1
  ENDIF

  SCIA_LV1_RD_BASE, dsd, base, status=status
  IF status NE 0 THEN BEGIN
     status = -1
     RETURN, -1
  ENDIF

  SCIA_LV1_RD_SCP, dsd, scp, status=status
  IF status NE 0 THEN BEGIN
     status = -1
     RETURN, -1
  ENDIF

; obtain info about requested state
  IF N_ELEMENTS( stateIndex ) GT 0 THEN BEGIN
     IF MAX(stateIndex) GE N_ELEMENTS(state) THEN BEGIN
        MESSAGE, ' Error: stateIndex is out-of-range', /INFO
        status = -1
        RETURN, -1
     ENDIF
     state = state[stateIndex]
  ENDIF ELSE IF N_ELEMENTS( stateID ) NE 0 THEN BEGIN
     index = WHERE( state.state_id EQ FIX(stateID[0]), stateCount )
     IF stateCount EQ 0 THEN BEGIN
        MESSAGE, ' Error: No states with requested ID found in this product', $
                 /INFO
        status = -1
        RETURN, -1
     ENDIF
     state = state[index]
  ENDIF

; 
  xi = (chanID-1) * !nadc.channelSize
  xj = chanID * !nadc.channelSize - 1
  ipixel = DINDGEN( !nadc.channelSize )
  wave = base.wvlen_det_pix[xi:xj]

  wave_out = 0
  coeffsOffset = (chanID-1) * 5
  FOR ns = 0, N_ELEMENTS( state )-1 DO BEGIN
     ni = MAX(WHERE( scp.orbit_phase LE state[ns].orbit_phase ))
     nj = MIN(WHERE( scp.orbit_phase GE state[ns].orbit_phase ))
     IF ni EQ nj THEN BEGIN
        dwave = ((((scp[ni].coeffs[coeffsOffset+4] $
                    * ipixel + scp[ni].coeffs[coeffsOffset+3]) $
                   * ipixel + scp[ni].coeffs[coeffsOffset+2]) $
                  * ipixel + scp[ni].coeffs[coeffsOffset+1]) $
                 * ipixel + scp[ni].coeffs[coeffsOffset])
     ENDIF ELSE BEGIN
        dwave_i = ((((scp[ni].coeffs[coeffsOffset+4] $
                      * ipixel + scp[ni].coeffs[coeffsOffset+3]) $
                     * ipixel + scp[ni].coeffs[coeffsOffset+2]) $
                    * ipixel + scp[ni].coeffs[coeffsOffset+1]) $
                   * ipixel + scp[ni].coeffs[coeffsOffset])
        dwave_j = ((((scp[nj].coeffs[coeffsOffset+4] $
                      * ipixel + scp[nj].coeffs[coeffsOffset+3]) $
                     * ipixel + scp[nj].coeffs[coeffsOffset+2]) $
                    * ipixel + scp[nj].coeffs[coeffsOffset+1]) $
                   * ipixel + scp[nj].coeffs[coeffsOffset])

        frac = (state[ns].orbit_phase - scp[ni].orbit_phase) $
               / (scp[nj].orbit_phase - scp[ni].orbit_phase)
        dwave = dwave_i + frac * (dwave_j - dwave_i)
     ENDELSE
     IF ns EQ 0 THEN $
        wave_out = FLOAT(wave + dwave) $
     ELSE $
        wave_out = [[wave_out],[FLOAT(wave + dwave)]]
  ENDFOR

  RETURN, wave_out
END
