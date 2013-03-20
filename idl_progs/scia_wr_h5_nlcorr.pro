;
; COPYRIGHT (c) 2003 - 2013 SRON (R.M.van.Hees@sron.nl)
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
;	SCIA_WR_H5_NLCORR
;
; PURPOSE:
;	write non-linearity correction keydata to HDF5 file
;
; CATEGORY:
;	Sciamachy calibration
;
; CALLING SEQUENCE:
;       SCIA_WR_H5_NLINCORR, nlincorr_fl=nlincorr_fl
;
; KEYWORD PARAMETERS:
;       nlcorr_fl:  input file with non-linearity correction keydata
;                       [default: ./splmem_lut.txt]
; EXAMPLE:
;	None
;
; MODIFICATION HISTORY:
; 	Written by:	Richard van Hees (SRON), December 2003
;       Modified:  RvH, 03 March 2004
;                    restructed valid range for interplation to 60000
;-
PRO SCIA_WR_H5_NLCORR, nlcorr_fl=nlcorr_fl
  compile_opt idl2,hidden

  IF N_ELEMENTS( nlcorr_fl ) EQ 0 THEN nlcorr_fl = './NLDATA.R14.dat'
  result = FILE_TEST( nlcorr_fl, /READ )
  IF result EQ 0 THEN MESSAGE, 'File: ' + nlcorr_fl + ' does not exist!'

  curveDimX =   1024L
  curveDimY =      8L
  nLinDimX  =  65536L
  nLinDimY  =     15L

; first curve is zero for channels 1 to 5
; channels 6,6+,7,8 have four curves each: upper-lower/odd-even
  Status  = 'failed'
  Data    = FLTARR( nLinDimY+2, nLinDimX )
  ibuff   = INTARR( curveDimX * curveDimY )

  on_ioerror, FAILED
  OPENR, Unit, nlcorr_fl, /GET_LUN
  READU, Unit, ibuff
  READU, Unit, Data
  Status = 'success'
FAILED:
  IF Status EQ 'failed' THEN BEGIN
     PRINT, 'ApplyNonLinearityR14: error while reading non-linearity data.'
     RETURN
  ENDIF
  FREE_LUN, Unit

; skip 2-columns with NaN
  Data[5:nLinDimY-1,*] = Data[7:nLinDimY+1,*]
  Data = TRANSPOSE( Data[0:nLinDimY-1,*] )
  ibuff[WHERE(ibuff GE 7)] -= 2

  CurveLegend = BYTE( ibuff )
  stat = call_external( lib_name('libnadc_idl'), '_SCIA_WR_H5_NLCORR', $
                        nLinDimX, curveDimY, CurveLegend, Data, /CDECL )

  RETURN
END
