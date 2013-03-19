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
;	SCIA_APPLY_NLINCORR
;
; PURPOSE:
;	apply non-linearity correction on cluster data
;
; CATEGORY:
;	SCIAMACHY
;
; CALLING SEQUENCE:
;	SCIA_APPLY_NLINCORR, coaddf, pixel_ids, pixel_val, nlcorr=nlcorr
;
; INPUTS:
;     coaddf:	scalar (byte) with co-adding factor of data
;  pixel_ids:   array with pixel numbers of data (range [0,8191])
;  pixel_val:   array with read_outs of a cluster
;
; KEYWORD PARAMETERS:
;   NLCORR:	apply non-linearity correction on Epitaxx data 
;               (channel 6-8), a named variable can be used to
;               speed-up a next call
;
; OUTPUTS:
;	This function corrects the data of a cluster (chan 6-8) for
;	non-linearity effects
;
; EXAMPLE:
;	To Do !!!
;
; MODIFICATION HISTORY:
; 	Written by:	Richard van Hees (SRON), 15 January 2009
;-
PRO SCIA_APPLY_NLINCORR, coaddf_in, pixel_ids, pixel_val, nlcorr=nlcorr
  compile_opt idl2,logical_predicate,hidden

  IF N_PARAMS() NE 3 THEN BEGIN
     MESSAGE, ' Usage: SCIA_APPLY_NLINCORR, coaddf' $
              + ', pixel_ids, pixel_val, nlcorr=nlcorr'
  ENDIF

  ; make sure that we use a reasonable value for the co-adding factor
  sz = SIZE( coaddf_in )
  IF sz[0] EQ 0 THEN $
     coaddf = FIX( coaddf_in ) > 1 $
  ELSE IF sz[0] EQ 1 and sz[1] EQ 1 THEN $
     coaddf = FIX( coaddf_in[0] ) > 1 $
  ELSE $
     coaddf = FIX((coaddf_in[SORT(coaddf_in)])[N_ELEMENTS(coaddf_in)/2]) > 1

  ; obtain channel ID
  tmp = pixel_ids / !nadc.channelSize
  chanIndx = FIX(tmp[UNIQ(tmp)])
  IF chanIndx LT 5 THEN RETURN

  ; check dimensions of pixel values
  num_pixels = N_ELEMENTS( pixel_ids )
  sz = SIZE( REFORM(pixel_val) )
  IF num_pixels GT 1 AND sz[1] NE num_pixels THEN $
     MESSAGE, 'Dimensions of pixel_ids and pixel_val do not agree'

  IF sz[0] EQ 2 THEN $
     num_obs = sz[2] $
  ELSE BEGIN
     IF num_pixels EQ 1 THEN $
        num_obs = sz[1] $
     ELSE $
        num_obs = 1
  ENDELSE

  ; read non-linearity correction values
  IF SIZE( nlcorr, /TNAME ) NE 'STRUCT' THEN $
     do_init_nl = !TRUE $
  ELSE IF TAG_NAMES( nlcorr, /STRUCT ) NE 'NLCORR' THEN $
     do_init_nl = !TRUE $
  ELSE BEGIN
     do_init_nl = !FALSE
     table = nlcorr.table
  ENDELSE
  IF do_init_nl THEN BEGIN
     SCIA_RD_H5_NLCORR, table, CurveLegend
     nlcorr = { NLCORR, Table:Table, CurveLegend:CurveLegend }
  ENDIF

  ; assume constant signal during co-adding
  SignNorm = ROUND(pixel_val / coaddf) > 0L < (2L^16 - 1)

  ; apply non-linearity correction
  CurveIndex = nlcorr.CurveLegend[pixel_ids]
  FOR nc = 0, (SIZE(table, /DIM))[1]-1 DO BEGIN
     indx = WHERE( CurveIndex EQ nc, count )
     IF count GT 0 THEN BEGIN
        IF num_pixels EQ 1 THEN BEGIN
           corrVal = nlcorr.Table[SignNorm, nc] 
           pixel_val -= coaddf * corrVal
        ENDIF ELSE BEGIN
           corrVal = REFORM( nlcorr.Table[SignNorm[indx,*], nc], $
                             count, num_obs )
           pixel_val[indx,*] -= coaddf * corrVal
        ENDELSE
     ENDIF
  ENDFOR
  
  RETURN
END
