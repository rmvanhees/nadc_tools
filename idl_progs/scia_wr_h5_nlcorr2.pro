;
; COPYRIGHT (c) 2010 - 2013 SRON (R.M.van.Hees@sron.nl)
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
;	SCIA_WR_H5_NLCORR2
;
; PURPOSE:
;	write non-linearity correction keydata to HDF5 file
;
; CATEGORY:
;	Sciamachy calibration
;
; CALLING SEQUENCE:
;       SCIA_WR_H5_NLINCORR2
;
; KEYWORD PARAMETERS:
;	None
;
; EXAMPLE:
;	None
;
; MODIFICATION HISTORY:
; 	Written by:	Richard van Hees (SRON), December 2010
;-
PRO SCIA_WR_H5_NLCORR2
  compile_opt idl2,hidden

  ; read old non-linearity correction values from HDF5 file
  SCIA_RD_H5_NLCORR, nlcorr, CurveIndex

  ; update non-linearity correction values
  nlcorr_none = DBLARR( 2L^16 )

  ; -- channel 8 odd  [1:511] --- curve 11
  nlcorr_update = nlcorr_none
  res = [-1.270782571121e+04,  5.177976538453e+00, -8.608678184087e-04, $
         7.465791595711e-08, -3.557858343334e-12,   8.823363676145e-17, $
         -8.897736123323e-22]
  xfit = 9750 + dindgen(23500-9750+1)
  yfit = res[6] & FOR ii = 5, 0, -1 DO yfit = res[ii] + xfit * yfit
  nlcorr_update[9751:23501] = yfit
  X = [8000 + dindgen(501), 11000 + dindgen(501)]
  Y = [nlcorr_update[8001:8501], nlcorr_update[11001:11501]]
  X2 = 8000 + dindgen(11501 - 8001 + 1)
  Y2 = SPL_INIT( X, Y )
  yfit = SPL_INTERP( X, Y, Y2, X2 )
  nlcorr_update[8001:11501] = yfit
  X = [20000 + dindgen(1001), 28000 + dindgen(2001)]
  Y = [nlcorr_update[20001:21001], nlcorr[28001:30001,11]]
  X2 = 20000 + dindgen(8001)
  Y2 = SPL_INIT( X, Y )
  yfit = SPL_INTERP( X, Y, Y2, X2 )
  nlcorr_update[20001:28001] = yfit
  nlcorr[0:28001, 11] = nlcorr_update[0:28001]
  
  ; -- channel 8 even [0:510] --- curve 12
  nlcorr_even_low = nlcorr_none
  res = [-4.896363849274e+02,  1.388429309295e-01, -1.613754858655e-05, $
         9.967976697555e-10, -3.289896714881e-14,  4.163102558651e-19]
  xfit = 9750 + dindgen(26000-9750+1)
  yfit = res[5] & FOR ii = 4, 0, -1 DO yfit = res[ii] + xfit * yfit
  nlcorr_even_low[9751:26001] = yfit
  X = [9000 + dindgen(501), 12000 + dindgen(501)]
  Y = [nlcorr_even_low[9001:9501], nlcorr_even_low[12001:12501]]
  X2 = 9000 + dindgen(12501 - 9001 + 1)
  Y2 = SPL_INIT( X, Y )
  yfit = SPL_INTERP( X, Y, Y2, X2 )
  nlcorr_even_low[9001:12501] = yfit
  X = [22000 + dindgen(1001), 29000 + dindgen(1001)]
  Y = [nlcorr_even_low[22001:23001], nlcorr[29001:30001,12]]
  X2 = 22000 + dindgen(30001 - 22001 + 1)
  Y2 = SPL_INIT( X, Y )
  yfit = SPL_INTERP( X, Y, Y2, X2 )
  nlcorr_even_low[22001:30001] = yfit
  nlcorr[0:29001, 12] = nlcorr_even_low[0:29001]

  ; -- channel 8 odd  [513-1023] --- curve 13
  nlcorr_update = nlcorr_none
  res = [6.068209362079e+00, -3.928509706444e-03,  7.302302196298e-07, $
         -5.097309878590e-11,  1.159975373490e-15]
  xfit = 2000 + dindgen(9999-2000+1)
  yfit = res[4] & FOR ii = 3, 0, -1 DO yfit = res[ii] + xfit * yfit
  nlcorr_update[2001:10000] += yfit
  res = [5.947425620854e+03, -2.069313207365e+00,  2.903256827763e-04, $
        -2.097380625061e-08,  8.226807623793e-13, -1.668179190612e-17, $
         1.372046070475e-22]
  xfit = 10000 + dindgen(31000-10000+1)
  yfit = res[6] & FOR ii = 5, 0, -1 DO yfit = res[ii] + xfit * yfit
  nlcorr_update[10001:31001] += yfit
  X = [9000 + dindgen(501), 12000 + dindgen(501)]
  Y = [nlcorr_update[9001:9501], nlcorr_update[12001:12501]]
  X2 = 9000 + dindgen(12501 - 9001 + 1)
  Y2 = SPL_INIT( X, Y )
  yfit = SPL_INTERP( X, Y, Y2, X2 )
  nlcorr_update[9001:12501] = yfit
  X = [25000 + dindgen(1001), 34000 + dindgen(1001)]
  Y = [nlcorr_update[25001:26001], nlcorr[34001:35001,13]]
  X2 = 25000 + dindgen(35001 - 25001 + 1)
  Y2 = SPL_INIT( X, Y )
  yfit = SPL_INTERP( X, Y, Y2, X2 )
  nlcorr_update[25001:35001] = yfit
  nlcorr[0:34001, 13] = nlcorr_update[0:34001]

  ; -- channel 8 even [512-1022] --- curve 14
  nlcorr_even_high = nlcorr_none
  res = [4.141350621750e+04, -1.388540875155e+01,  1.849302684424e-03, $
         -1.223087734974e-07,  4.017909141528e-12, -5.248985078851e-17]
  xfit = 11700 + dindgen(20000-11700+1)
  yfit = res[5] & FOR ii = 4, 0, -1 DO yfit = res[ii] + xfit * yfit
  nlcorr_even_high[11701:20001] = yfit
  X = [18500 + dindgen(501), 20500 + dindgen(501)]
  Y = [nlcorr_even_high[18501:19001], nlcorr[20501:21001,14]]
  X2 = 18500 + dindgen(21001 - 18501 + 1)
  Y2 = SPL_INIT( X, Y )
  yfit = SPL_INTERP( X, Y, Y2, X2 )
  nlcorr_even_high[18501:21001] = yfit
  nlcorr[0:20501, 14] = nlcorr_even_high[0:20501]

  ; set non-linearity correction to zero for filling's above 60000 BU
  nlcorr[60000:-1, 11] = nlcorr[59999, 11]
  nlcorr[60000:-1, 12] = nlcorr[59999, 12]
  nlcorr[60000:-1, 13] = nlcorr[59999, 13]
  nlcorr[60000:-1, 14] = nlcorr[59999, 14]

  ; write the updated values
  stat = call_external( lib_name('libIDL_NADC'), '_SCIA_WR_H5_NLCORR', $
                        CurveIndex, nlcorr, /CDECL )
  PRINT, stat
  RETURN
END
