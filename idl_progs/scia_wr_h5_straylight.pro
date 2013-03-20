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
PRO SCIA_WR_H5_STRAYLIGHT, grid_in, grid_out, strayCorr, $
                           straylight_fl=straylight_fl
  compile_opt idl2,hidden

; check required parameters
  IF N_PARAMS() NE 3 THEN BEGIN
     MESSAGE, ' Usage: scia_wr_h5_straylight, strayCorr ' $
              + ', straylight_fl=straylight_fl', /INFO
     status = -1
     RETURN
  ENDIF

  IF N_ELEMENTS( straylight_fl ) EQ 0 THEN $
     straylight_fl = './keydata_final.sav'
  result = FILE_TEST( straylight_fl, /READ )
  IF result EQ 0 THEN MESSAGE, 'File: ' + straylight_fl + ' does not exist!'

; extract Straylight correction data
  RESTORE, straylight_fl
  grid_in  = keydata.stray_uniform.dim2
  grid_out = keydata.stray_uniform.dim1
  strayCorr = TRANSPOSE( keydata.stray_uniform.data )
  dimX = N_ELEMENTS( grid_in )
  dimY = N_ELEMENTS( grid_out )

  stat = call_external( lib_name('libnadc_idl'), '_SCIA_WR_H5_STRAYLIGHT', $
                        dimX, dimY, grid_in, grid_out, strayCorr, /CDECL )
  RETURN
END
