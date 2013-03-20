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
;	SDMF_READ_LL
;
; PURPOSE:
;	read rows of given orbit from SDMF last-limb database.
;
; CATEGORY:
;	SDMF - SCIA calibration
;
; CALLING SEQUENCE:
;	SDMF_READ_LL, absOrbit, state_id, clus_id, mtbl, mds1c,
;                           status=status, SDMF_H5_DB=SDMF_H5_DB
;
; INPUTS:
;	absOrbit:	(absolute) Orbit number, a scalar
;	state_id:	State ID, a scalar, range [1..70]
;
;       clus_id:        Cluster ID, a scalar, range [1..40]
;
; OUTPUTS:
;	mtbl:           structure with rows of table metaTable
;       mds1c:          structure with last-limb measurements
;
; KEYWORD PARAMETERS:
;       SDMF_H5_DB:     path to the SDMF last-limb database
;                         default: /SCIA/SDMF30/sdmf_extract_ll.h5
;       status :        returns named variable with error flag (0 = ok)
;       nonlin :        flag to request non-linearity correction
;
; PROCEDURE:
;	Blah Blah Blah.
;
; EXAMPLE:
;	ToDo...
;
; MODIFICATION HISTORY:
;    	Written by:     Richard van Hees (SRON), January 2009
;    	Modified by:    Pieter van der Meer (SRON), August 2009
;    	                * added non-linearity correction
;-
PRO SDMF_READ_LL, absOrbit, state_id, clus_id, mtbl, mds1c, $
                  status=status, SDMF_H5_DB=SDMF_H5_DB, nonlin=nonlin
  compile_opt idl2,logical_predicate,hidden
  common SDMF_READ_LL, nlincorr

; initialize return values
  mtbl = -1
  mds1c = -1
  status = -1
  IF N_PARAMS() NE 5 THEN BEGIN
     MESSAGE, ' Usage: SDMF_READ_LL, absOrbit, state_id, clus_id, mtbl' $
              + ', pixel_val, status=status, SDMF_H5_DB=SDMF_H5_DB', /INFO
     RETURN
  ENDIF
  SecPerDay = 60.d * 60 * 24

  IF SIZE( absOrbit, /TNAME ) NE 'LONG' THEN absOrbit = LONG( absOrbit )
  IF SIZE( state_id, /TNAME ) NE 'UINT' THEN state_id = UINT( state_id )
  IF SIZE( clus_id, /TNAME ) NE 'UINT' THEN clus_id = UINT( clus_id )

  IF N_ELEMENTS( SDMF_H5_DB ) EQ 0 THEN $
     SDMF_H5_DB = GET_SDMF_PATH() + './sdmf_extract_ll.h5'
  IF (~ FILE_TEST( SDMF_H5_DB, /READ, /NOEXPAND )) THEN BEGIN
     MESSAGE, ' Can not read from database: ' + SDMF_H5_DB, /INFO
     RETURN
  ENDIF

; define some constants
  SecPerDay = 60.d * 60 * 24
  measCat   = [ 1b,  1b,  1b,  1b,  1b,  1b,  1b,  1b,  1b,  1b,  $
                1b,  1b,  1b,  1b,  1b,  1b,  1b,  1b,  1b,  1b,  $
                1b,  1b,  3b,  3b,  3b, 17b, 18b,  2b,  2b,  2b,  $
                2b,  2b,  2b,  2b,  2b,  2b,  2b,  2b,  2b,  2b,  $
                2b,  3b,  3b,  3b,  3b, 12b,  4b,  3b,  4b,  4b,  $
                5b,  8b,  9b,  7b, 14b,  6b,  6b,  9b, 10b,  9b,  $
                11b, 16b, 12b, 13b, 15b, 13b, 12b, 13b, 10b, 11b ]

; get cluster definitions from SDMF last-limb database
  ClusDef = REPLICATE( {clusdef_scia}, 40 )
  num = call_external( lib_name('libnadc_idl'), '_SDMF_RD_PT_CLUSDEF', $
                       SDMF_H5_DB, ClusDef, /CDECL )

; get cluster attributes for given state and cluster
  coaddf = 0b & num_pixels = 0us & num_obs = 0us & pet = 0.
  num = call_external( lib_name('libnadc_idl'), '_SDMF_RD_PT_CLUS_ATTR', $
                       SDMF_H5_DB, state_id, clus_id, coaddf, num_pixels, $
                       num_obs, pet, /CDECL )

; get indices to rows in SDMF database for given state and orbit number
  numIndx = 10u
  metaIndx = ULONARR( numIndx )
  num = call_external( lib_name('libnadc_idl'), '_SDMF_GET_PT_ORBITINDEX', $
                       SDMF_H5_DB, state_id, absOrbit, numIndx, metaIndx, $
                       /CDECL )

; read cluster data and meta data from SDMF last limb database
  total_pixels = num_pixels * num_obs
  if numIndx eq 0 then return
  mtbl = REPLICATE( {sdmf_pt_meta}, numIndx )
  pixel_val = FLTARR( total_pixels * numIndx )
  num = call_external( lib_name('libnadc_idl'), '_SDMF_RD_PT_CLUSTER', $
                       SDMF_H5_DB, state_id, clus_id, numIndx, metaIndx, $
                       mtbl, pixel_val, /CDECL )

; initialize pixel IDs and reform array with pixel values
  print, num_pixels, num_obs, numIndx
  mds1c = REPLICATE( {mds1c_scia}, numIndx )
  pixel_ids = ClusDef[clus_id-1].start + UINDGEN( num_pixels )
  pixel_val = REFORM( pixel_val, num_pixels, num_obs, numIndx )

  if keyword_set(nonlin) then begin
    ;print, 'correcting nlin!'
    if n_elements(nlincorr) le 0 then begin
       SCIA_RD_H5_NLCORR, table, CurveLegend
       nlincorr = { NLINCORR, Table:Table, CurveLegend:CurveLegend }
    endif
    mi = min(pixel_ids, max=ma)
    for i = mi, ma do begin
       slice = reform(pixel_val[i-mi,*,*])
       index = where(finite(slice) and (slice ne 0), count_index)
       if count_index gt 0 then begin
          CurveIndex           = nlincorr.CurveLegend[i]
          pixel_val[i-mi,*,*] -= nlincorr.Table[round(slice),CurveIndex]
       endif
    endfor
  endif

; fill level 1c MDS
  FOR ni = 0, numIndx-1 DO BEGIN
     mds1c[ni].mjd.days    = LONG( mtbl[ni].julianDate )
     dsec = SecPerDay * (mtbl[ni].julianDate - mds1c[ni].mjd.days) ;
     mds1c[ni].mjd.secnd   = ULONG( dsec )
     mds1c[ni].mjd.musec   = ULONG( 1e6 * (dsec - mds1c[ni].mjd.secnd))
     mds1c[ni].type_mds    = !nadc.sciaLimb
     mds1c[ni].coaddf      = coaddf
     mds1c[ni].category    = measCat[State_id-1]
     mds1c[ni].state_id    = state_id
     mds1c[ni].chan_id     = ClusDef[clus_id-1].chanID ;
     mds1c[ni].clus_id     = clus_id
     mds1c[ni].num_obs     = num_obs
     mds1c[ni].num_pixels  = num_pixels
     mds1c[ni].orbit_phase = mtbl[ni].orbitPhase ;
     mds1c[ni].pet         = pet
     mds1c[ni].pixel_ids   = PTR_NEW( pixel_ids )
     mds1c[ni].pixel_val   = PTR_NEW( pixel_val[*,*,ni] )
     mds1c[ni].geoL        = PTR_NEW( REPLICATE({geoL_scia},num_obs), /NO_COPY )
  ENDFOR
  pixel_val = 0

  status = 0
  RETURN
END
