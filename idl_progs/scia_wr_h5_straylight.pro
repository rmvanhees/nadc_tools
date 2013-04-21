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
;	SCIA_WR_H5_STRAYLIGHT
;
; PURPOSE:
;	write Straylight keydata to HDF5 file
;
; CATEGORY:
;	Sciamachy calibration
;
; CALLING SEQUENCE:
;	SCIA_WR_H5_STRAYLIGHT, grid_in, grid_out, strayCorr, $
;                              straylight_fl=straylight_fl
;
; INPUTS:
;	strayCorr:      Straylight correction keydata
;
; KEYWORD PARAMETERS:
;	straylight_fl:  input file with straylight correction values
;                       [default: ./keydata_final.sav]
;
; EXAMPLE:
;	None
;
; MODIFICATION HISTORY:
; 	Written by:	Richard van Hees (SRON), October 2007
;       Modified:  RvH, 6 July 2011
;                    straylight keydata for all channels
;-
PRO SCIA_WR_H5_STRAYLIGHT, stray_uniform, stray_ghost, $
                           keydata_fl=keydata_fl
  compile_opt idl2,hidden

; check required parameters
  IF N_PARAMS() NE 2 THEN BEGIN
     MESSAGE, ' Usage: scia_wr_h5_straylight, stray_uniform, stray_ghost ' $
              + ', keydata_fl=keydata_fl', /INFO
     status = -1
     RETURN
  ENDIF

  IF N_ELEMENTS( keydata_fl ) EQ 0 THEN $
     keydata_fl = './keydata_final.sav'
  result = FILE_TEST( keydata_fl, /READ )
  IF result EQ 0 THEN MESSAGE, 'File: ' + keydata_fl + ' does not exist!'

; extract Straylight keydata
  RESTORE, keydata_fl
  stray_uniform = keydata.stray_uniform
  stray_ghost = keydata.stray_ghost
  
; write Straylight keydata to HDF5 file
  grid_in  = keydata.stray_uniform.dim2
  grid_out = keydata.stray_uniform.dim1
  strayMatrix = TRANSPOSE( keydata.stray_uniform.data )
  strayGhost = keydata.stray_ghost.data

  fid = H5F_create( 'Straylight.h5' )
  type_id = H5T_idl_create( grid_in )
  space_id = H5S_create_simple( SIZE(grid_in, /DIMENSIONS) )
  data_id = H5D_create( fid, 'grid_in', type_id, space_id )
  H5D_write, data_id, grid_in
  H5T_close, type_id
  H5S_close, space_id
  H5D_close, data_id

  type_id = H5T_idl_create( grid_out )
  space_id = H5S_create_simple( SIZE(grid_out, /DIMENSIONS) )
  data_id = H5D_create( fid, 'grid_out', type_id, space_id )
  H5D_write, data_id, grid_out
  H5T_close, type_id
  H5S_close, space_id
  H5D_close, data_id

  type_id = H5T_idl_create( strayMatrix )
  space_id = H5S_create_simple( SIZE(strayMatrix, /DIMENSIONS) )
  data_id = H5D_create( fid, 'strayMatrix', type_id, space_id )
  H5D_write, data_id, strayMatrix
  H5T_close, type_id
  H5S_close, space_id
  H5D_close, data_id

  type_id = H5T_idl_create( strayGhost )
  space_id = H5S_create_simple( SIZE(strayGhost, /DIMENSIONS) )
  data_id = H5D_create( fid, 'strayGhost', type_id, space_id )
  H5D_write, data_id, strayGhost
  H5T_close, type_id
  H5S_close, space_id
  H5D_close, data_id

  H5F_close, fid
  RETURN
END
