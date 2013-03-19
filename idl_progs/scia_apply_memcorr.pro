;
; COPYRIGHT (c) 2009 - 2013 SRON (R.M.van.Hees@sron.nl)
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
;	SCIA_APPLY_MEMCORR
;
; PURPOSE:
;	apply memory correction on cluster data
;
; CATEGORY:
;	SCIAMACHY
;
; CALLING SEQUENCE:
;	SCIA_APPLY_MEMCORR, state_id, coaddf, pet, pixel_ids, pixel_val, $
;                           memcorr=memcorr, reset=reset
;
; INPUTS:
;   state_id:   scalar (integer) with State ID (range 1..70)
;     coaddf:	scalar (integer) with co-adding factor of data
;        pet:   scalar (float) with pixel exposure time
;  pixel_ids:   array with pixel numbers of data (range 0..8191)
;  pixel_val:   array with read_outs of a cluster
;
; KEYWORD PARAMETERS:
;    MEMCORR:	apply memory correction on Reticon data 
;               (channel 1-5), a named variable can be used to
;               speed-up a next call
;
;      RESET:	calculate correction of first readout after a state
;               reset. Always use this keyword, except when the scan
;               is incomplete, in particular, for the SDMF databases
;
;               IMPORTANT: value for reset should be orbit number, or
;               the leakage current of orbit 20000 will be used
;
; OUTPUTS:
;	This function corrects the data of a cluster (chan 1-5) for
;	memory effects
;
; EXAMPLE:
;	To Do !!!
;
; MODIFICATION HISTORY:
; 	Written by:	Richard van Hees (SRON), 15 January 2009
;       Modified:  RvH, 20 January 2009
;                       Use State setup time to correct first readout
;       Modified:  RvH, 18 May 2010
;                       Improved algorithm to correct first readout of state
;       Modified:  RvH, 23 May 2012
;                       Use SDMF_READ_STATEDARK in function CALC_RESET_FIRST
;-
;

; Function to calculate Signal measured during state setup 
;
FUNCTION CALC_RESET_FIRST, orbit_in, state_id, coaddf, pet, $
                           pixel_ids, pixel_val
  compile_opt idl2,logical_predicate,hidden

  setup_it = [ 421.875, 421.875, 421.875, 421.875, 421.875, 421.875,     $
               421.875, 421.875, 421.875, 421.875, 421.875, 421.875,     $
               421.875, 421.875, 421.875, 1269.53125, 421.875, 421.875,  $
               421.875, 421.875, 421.875, 421.875, 421.875, 421.875,     $
               421.875, 421.875, 421.875, 421.875, 421.875, 421.875,     $
               421.875, 421.875, 421.875, 421.875, 421.875, 421.875,     $
               421.875, 421.875, 421.875, 421.875, 421.875, 421.875,     $
               421.875, 421.875, 421.875, 421.875, 421.875, 1269.53125,  $
               421.875, 421.875, 421.875, 421.875, 421.875, 421.875,     $
               421.875, 421.875, 421.875, 421.875, 519.53125, 421.875,   $
               1269.53125, 421.875, 421.875, 421.875, 335.9375, 421.875, $
               421.875, 421.875, 519.53125, 1269.53125 ]

  orbit = LONG( orbit_in )
  tmp = pixel_ids / !nadc.channelSize
  channel = FIX(tmp[UNIQ(tmp)])+1
  SDMF_READ_STATEDARK, state_id, orbit, mtbl, signal, noise, $
                       pet=pet, channel=channel, status=status
  IF status NE 0 THEN RETURN, (pixel_val / coaddf)

  darkCorr = signal[pixel_ids]
  factor = setup_it[state_id-1] / (1000. * pet)
  SignFirst = ROUND( darkCorr + factor * (pixel_val / coaddf - darkCorr) )

  RETURN, (SignFirst > 0L < (2L^16 - 1))
END

;
; Procedure of main module
;
PRO SCIA_APPLY_MEMCORR, state_id_in, coaddf_in, pet_in, pixel_ids, pixel_val, $
                        memcorr=memcorr, reset=reset
  compile_opt idl2,logical_predicate,hidden

  IF N_PARAMS() NE 5 THEN BEGIN
     MESSAGE, ' Usage: SCIA_APPLY_MEMCORR, state_id, coaddf, pet' $
              + ', pixel_ids, pixel_val, MEMCORR=memcorr, RESET=absOrbit'
  ENDIF

; make sure that we use a reasonable value for state_id, coaddf and pet
  sz = SIZE( state_id_in )
  IF sz[0] EQ 0 THEN $
     state_id = FIX( state_id_in ) $
  ELSE IF sz[0] EQ 1 AND sz[1] EQ 1 THEN $
     state_id = FIX( state_id_in[0] ) $
  ELSE $
     state_id = FIX((state_id_in[SORT(state_id_in)])[N_ELEMENTS(state_id_in)/2])

  sz = SIZE( coaddf_in )
  IF sz[0] EQ 0 THEN $
     coaddf = FIX( coaddf_in ) > 1 $
  ELSE IF sz[0] EQ 1 AND sz[1] EQ 1 THEN $
     coaddf = FIX( coaddf_in[0] ) > 1 $
  ELSE $
     coaddf = FIX((coaddf_in[SORT(coaddf_in)])[N_ELEMENTS(coaddf_in)/2]) > 1

  sz = SIZE( pet_in )
  IF sz[0] EQ 0 THEN $
     pet = FLOAT( pet_in ) > 0 $
  ELSE IF sz[0] EQ 1 AND sz[1] EQ 1 THEN $
     pet = FLOAT( pet_in[0] ) > 0 $
  ELSE $
     pet = FLOAT((pet_in[SORT(pet_in)])[N_ELEMENTS(pet_in)/2]) > 0

  reset_orbit = -1
  IF KEYWORD_SET( reset ) THEN BEGIN
     IF reset LT 1000 THEN $
        reset_orbit = 20000U $
     ELSE $
        reset_orbit = UINT(reset)
  ENDIF

; obtain channel ID (only channel 1-5)
  tmp = pixel_ids / !nadc.channelSize
  chanIndx = FIX(tmp[UNIQ(tmp)])
  IF chanIndx GE 5 THEN BEGIN
     MESSAGE, 'Apply memory correction only on Reticon detectors', /INFO
     RETURN
  ENDIF

; check dimensions of pixel values
  num_pixels = N_ELEMENTS( pixel_ids )
  sz = SIZE( REFORM(pixel_val) )
  IF num_pixels GT 1 AND sz[1] NE num_pixels THEN BEGIN
     MESSAGE, 'Dimensions of pixel_ids and pixel_val do not agree', /INFO
     RETURN
  ENDIF

  IF sz[0] EQ 2 THEN $
     num_obs = sz[2] $
  ELSE BEGIN
     IF num_pixels EQ 1 THEN $
        num_obs = sz[1] $
     ELSE $
        num_obs = 1
  ENDELSE

; read memory correction values
  IF SIZE( memcorr, /N_DIM ) LT 2 THEN SCIA_RD_H5_MEMCORR, memcorr

; assume constant signal during co-adding
  SignNorm = ROUND(pixel_val / coaddf) > 0L < (2L^16 - 1)

; correct for coadding during this readout (only for coaddf > 1)
  IF coaddf GT 1 THEN BEGIN
     pixel_val -= (coaddf - 1.) * memcorr[SignNorm, chanIndx]
  ENDIF

; correct with previous readout, handle Limb data differently
  IF num_obs GT 1 THEN BEGIN
     list_limb_id = [27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 40, 41, 55]
     IF WHERE( list_limb_id EQ state_id ) NE -1 THEN BEGIN
        factor = 3. / (16 * pet)

        corrVal = FLTARR( num_pixels, num_obs )
        indx = INDGEN( num_obs - 1 )
        corrVal[*,indx+1] = REFORM( memcorr[SignNorm[*,indx], chanIndx], $
                                    num_pixels, num_obs-1 )

        limb_reset = ROUND( 1.5 / (coaddf * pet) )
        num_reset = (num_obs / limb_reset) - 1            ; skip first readout
        indx_reset = limb_reset * (1 + INDGEN( num_reset ))
        IF limb_reset EQ 1 THEN BEGIN
           darkCorr = pixel_val[*,indx_reset[num_reset-1]:*]
        ENDIF ELSE BEGIN
           darkCorr = TOTAL( pixel_val[*,indx_reset[num_reset-1]:*], 2 ) $
                      / limb_reset
        ENDELSE
        darkCorr = darkCorr # FINDGEN( num_reset )
        SignNorm_0 = ROUND( darkCorr $
                            + factor * (SignNorm[*,indx_reset] - darkCorr) )
        corrVal[*,indx_reset] = REFORM( memcorr[SignNorm_0, chanIndx], $
                                        num_pixels, num_reset )
        pixel_val -= corrVal
     ENDIF ELSE BEGIN
        indx = INDGEN( num_obs - 1 )
        IF num_pixels EQ 1 THEN $
           pixel_val[indx+1] -= REFORM( memcorr[SignNorm[indx], chanIndx], $
                                        num_obs-1 ) $
        ELSE $
           pixel_val[*,indx+1] -= REFORM( memcorr[SignNorm[*,indx], chanIndx], $
                                          num_pixels, num_obs-1 )
     ENDELSE
  ENDIF

  ; correct the first readout
  IF KEYWORD_SET( reset ) THEN BEGIN
     SignNorm_0 = CALC_RESET_FIRST( reset_orbit, state_id, coaddf, pet, $
                                    pixel_ids, pixel_val[*,0] )
     pixel_val[*,0] -= memcorr[SignNorm_0, chanIndx]
  ENDIF ELSE BEGIN
     pixel_val[*,0] -= memcorr[SignNorm[*,0], chanIndx]         
  ENDELSE

  RETURN
END
