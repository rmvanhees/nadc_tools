;
; COPYRIGHT (c) 2007 - 2013 SRON (R.M.van.Hees@sron.nl)
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
;	SCIA_RD_H5_STRAYLIGHT
;
; PURPOSE:
;	read Straylight keydata to HDF5 file
;
; CATEGORY:
;	Sciamachy calibration
;
; CALLING SEQUENCE:
;	SCIA_RD_H5_STRAYLIGHT, grid_in, grid_out, strayGhost, strayMatrix, STRAY_DB=STRAY_DB
;
; INPUTS:
;	strayGhost:      Straylight correction keydata (ghosts
;	strayMatrix:     Straylight correction keydata (uniform)
;
; EXAMPLE:
;	None
;
; MODIFICATION HISTORY:
; 	Written by:	Richard van Hees (SRON), October 2007
;       Modified:  RvH, 6 July 2011
;                    straylight keydata for all channels
;       Modified:  RvH, 25 August 2011
;                    re-write using IDL HDF5 calls
;-
PRO SCIA_RD_H5_STRAYLIGHT, grid_in, grid_out, strayGhost, strayMatrix, $
                           STRAY_DB=STRAY_DB
  compile_opt idl2,hidden

  IF N_PARAMS() NE 4 THEN BEGIN
     MESSAGE, ' Usage: SCIA_RD_H5_STRAYLIGHT, grid_in, grid_out, strayGhost,' $
              + ' strayMatrix, STRAY_DB=STRAY_DB', /INFO
     RETURN
  ENDIF

  IF N_ELEMENTS( STRAY_DB ) EQ 0 THEN BEGIN
     IF FILE_TEST( './Straylight.h5', /READ, /NOEXPAND ) THEN $
        STRAY_DB = './Straylight.h5' $
     ELSE $
        STRAY_DB = !nadc.dataDir + '/Straylight.h5'
  ENDIF
  IF (~ FILE_TEST( STRAY_DB, /READ, /NOEXPAND )) THEN BEGIN
     MESSAGE, ' Can not read from database: ' + STRAY_DB, /INFO
     RETURN
  ENDIF

  ; open memory correction database
  fid = H5F_OPEN( STRAY_DB )
  IF fid LT 0 THEN MESSAGE, 'Could not open database: ' + STRAY_DB

  dd = H5D_OPEN( fid, 'grid_in' )
  IF dd LT 0 THEN MESSAGE, 'Could not open dataset: grid_in'
  grid_in = H5D_READ( dd )
  H5D_CLOSE, dd
  dd = H5D_OPEN( fid, 'grid_out' )
  IF dd LT 0 THEN MESSAGE, 'Could not open dataset: grid_out'
  grid_out = H5D_READ( dd )
  H5D_CLOSE, dd
  dd = H5D_OPEN( fid, 'strayGhost' )
  IF dd LT 0 THEN MESSAGE, 'Could not open dataset: strayGhost'
  strayGhost = H5D_READ( dd )
  H5D_CLOSE, dd
  dd = H5D_OPEN( fid, 'strayMatrix' )
  IF dd LT 0 THEN MESSAGE, 'Could not open dataset: strayMatrix'
  strayMatrix = H5D_READ( dd )
  H5D_CLOSE, dd
  H5F_CLOSE, fid

  RETURN
END
