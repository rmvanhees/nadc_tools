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
;	SCIA_WR_H5_MEMCORR
;
; PURPOSE:
;	write memory correction keydata to HDF5 file
;
; CATEGORY:
;	Sciamachy calibration
;
; CALLING SEQUENCE:
;       SCIA_WR_H5_MEMCORR, memcorr_fl=memcorr_fl
;
; KEYWORD PARAMETERS:
;       memcorr_fl:  input file with memory correction values
;                       [default: ./splmem_lut.txt]
; EXAMPLE:
;	None
;
; MODIFICATION HISTORY:
; 	Written by:	Richard van Hees (SRON), December 2003
;-
PRO SCIA_WR_H5_MEMCORR, memcorr_fl=memcorr_fl
  compile_opt idl2,logical_predicate,hidden

  IF N_ELEMENTS( memcorr_fl ) EQ 0 THEN memcorr_fl = './splmem_lut.txt'
  result = FILE_TEST( memcorr_fl, /READ )
  IF result EQ 0 THEN MESSAGE, 'File: ' + memcorr_fl + ' does not exist!'

  nr_chan = 0
  nr_line = 0
  tmpstring = ''
  OPENR, unit, memcorr_fl, /Get_Lun
  WHILE (~ EOF(unit)) DO BEGIN
     READF, unit, tmpstring
     IF STRPOS( tmpstring, '#' ) EQ (-1 ) THEN BEGIN
        nr_line++
     ENDIF ELSE IF STRPOS( tmpstring, '#CHANNEL' ) NE (-1 ) THEN BEGIN
        nr_chan++
     ENDIF
  ENDWHILE
  tmpvec = STRARR( nr_line )
  Point_Lun, unit, 0
  nr_line = 0
  WHILE (~ EOF(unit)) DO BEGIN
     READF, unit, tmpstring
     IF STRPOS( tmpstring, '#' ) EQ (-1 ) THEN BEGIN
        tmpvec[nr_line] = tmpstring
        nr_line++
     ENDIF
  ENDWHILE
  Free_Lun, unit

  outarr = fltarr( 2, nr_line )
  FOR nl = 0, nr_line-1 DO BEGIN
     tmpstring = STRTRIM( tmpvec[nl], 2 )
     tmpstring = STRCOMPRESS( tmpstring )
     row = STRSPLIT( tmpstring, ' ', /EXTRACT )
     IF (SIZE(row))[1] NE 2 THEN row = [row,'NaN']
     outarr[*,nl] = FLOAT(row)
  ENDFOR
  nr_bin = nr_line / nr_chan

  memDimX = 65536L
  memDimY = LONG( nr_chan )
  TableBinned = REFORM( outarr[1,*], nr_bin, nr_chan )
  xx = REFORM( outarr[0,*], nr_bin, nr_chan )
  tt = LINDGEN( memDimX )

  Table = FLTARR( memDimX, memDimY )
  FOR nc = 0, memDimY-1 DO $
     Table[*,nc] = SPLINE( xx[*,nc], TableBinned[*,nc], tt )

  stat = call_external( lib_name('libnadc_idl'), '_SCIA_WR_H5_MEMCORR', $
                        memDimX, memDimY, Table, /CDECL )

  RETURN
END
