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
;	SDMF_READ_SUN
;
; PURPOSE:
;	read rows of given orbit from SDMF Sun database.
;
; CATEGORY:
;	SDMF - SCIA calibration
;
; CALLING SEQUENCE:
;	SDMF_READ_SUN, absOrbit, state_id, clus_id, mtbl, mds1c,
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
;       mds1c:          structure with Sun measurements
;
; KEYWORD PARAMETERS:
;       SDMF_H5_DB:     path to SDMF Sun measurements database
;                         default: /SCIA/SDMF/3.0/sdmf_extract_sun.h5
;       status :        returns named variable with error flag (0 = ok)
;
; PROCEDURE:
;	Blah Blah Blah.
;
; EXAMPLE:
;	ToDo...
;
; MODIFICATION HISTORY:
;    	Written by:     Richard van Hees (SRON), January 2009
;       Modified:  RvH, 22 January 2009
;                    export geolocation data in structure geoL_scia
;-
PRO SDMF_READ_SUN, absOrbit, state_id, clus_id, mtbl, mds1c, $
                   status=status, SDMF_H5_DB=SDMF_H5_DB
  compile_opt idl2,logical_predicate,hidden

; initialize return values
  mtbl = -1
  mds1c = -1
  status = -1
  IF N_PARAMS() NE 5 THEN BEGIN
     MESSAGE, ' Usage: SDMF_READ_SUN, absOrbit, state_id, clus_id, mtbl' $
              + ', pixel_val, status=status, SDMF_H5_DB=SDMF_H5_DB', /INFO
     RETURN
  ENDIF

  IF SIZE( absOrbit, /TNAME ) NE 'LONG' THEN absOrbit = LONG( absOrbit )
  IF SIZE( state_id, /TNAME ) NE 'UINT' THEN state_id = UINT( state_id )
  IF SIZE( clus_id, /TNAME ) NE 'UINT' THEN clus_id = UINT( clus_id )

  IF N_ELEMENTS( SDMF_H5_DB ) EQ 0 THEN $
     SDMF_H5_DB = GET_SDMF_PATH() + './sdmf_extract_sun.h5'
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

; get cluster definitions from SDMF Sun database
  ClusDef = REPLICATE( {clusdef_scia}, 40 )
  num = call_external( lib_name('libnadc_idl'), '_SDMF_RD_PT_CLUSDEF', $
                       SDMF_H5_DB, ClusDef, /CDECL )

; get cluster attributes for given state and cluster
  coaddf = 0b & num_pixels = 0us & num_obs = 0us & pet = 0.
  num = call_external( lib_name('libnadc_idl'), '_SDMF_RD_PT_CLUS_ATTR', $
                       SDMF_H5_DB, state_id, clus_id, coaddf, num_pixels, $
                       num_obs, pet, /CDECL )

; get indices to rows in SDMF database for given state and orbit number
  numIndx = 10ul
  metaIndx = ULONARR( numIndx )
  num = call_external( lib_name('libnadc_idl'), '_SDMF_GET_PT_ORBITINDEX', $
                       SDMF_H5_DB, state_id, absOrbit, numIndx, metaIndx, $
                       /CDECL )

; read pointing information for cluster data
  num_obs_geo = 0us
  num = call_external( lib_name('libnadc_idl'), '_SDMF_RD_PT_GEO_ATTR', $
                       SDMF_H5_DB, state_id, num_obs_geo, /CDECL )
  prod = num_obs_geo * numIndx
  IF prod EQ 0 THEN BEGIN
     message, 'no matches found in sun db', /info
     RETURN
  ENDIF
  pointing = REPLICATE( {sdmf_pt_geo}, prod )
  num = call_external( lib_name('libnadc_idl'), '_SDMF_RD_PT_POINTING', $
                       SDMF_H5_DB, state_id, numIndx, metaIndx, $
                       pointing, /CDECL )

; read cluster data and meta data from SDMF Sun database
  total_pixels = LONG(num_pixels) * num_obs
  mtbl = REPLICATE( {sdmf_pt_meta}, numIndx )
  pixel_val = FLTARR( total_pixels * numIndx )
  num = call_external( lib_name('libnadc_idl'), '_SDMF_RD_PT_CLUSTER', $
                       SDMF_H5_DB, state_id, clus_id, numIndx, metaIndx, $
                       mtbl, pixel_val, /CDECL )

; initialize pixel IDs and reform array with pixel values
  mds1c = REPLICATE( {mds1c_scia}, numIndx )
  pixel_ids = ClusDef[clus_id-1].start + UINDGEN( num_pixels )
  pixel_val = REFORM( pixel_val, num_pixels, num_obs, numIndx )

; fill level 1c MDS
  FOR ni = 0, numIndx-1 DO BEGIN
     mds1c[ni].mjd.days    = LONG( mtbl[ni].julianDate )
     dsec = SecPerDay * (mtbl[ni].julianDate - mds1c[ni].mjd.days) ;
     mds1c[ni].mjd.secnd   = ULONG( dsec )
     mds1c[ni].mjd.musec   = ULONG( 1e6 * (dsec - mds1c[ni].mjd.secnd))
     mds1c[ni].type_mds    = !nadc.sciaMonitor
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
     no = 0us & ng = 0us
     WHILE ng LT num_obs DO BEGIN
        (*mds1c[ni].geoL)[ng].pos_esm = pointing[no].esmAngle
        (*mds1c[ni].geoL)[ng].pos_asm = pointing[no].asmAngle
        (*mds1c[ni].geoL)[ng].sun_azi_ang = REPLICATE(pointing[no].sunAz, 3)
        (*mds1c[ni].geoL)[ng].sun_zen_ang = REPLICATE(pointing[no].sunEl, 3)
        (*mds1c[ni].geoL)[ng].sub_sat_point.lat = LONG(mtbl[ni].latitude * 1e6)
        (*mds1c[ni].geoL)[ng].sub_sat_point.lon = LONG(mtbl[ni].longitude * 1e6)
        ng++
        no += (num_obs_geo / num_obs)
     ENDWHILE
  ENDFOR
  pixel_val = 0
  status = 0
  RETURN
END
