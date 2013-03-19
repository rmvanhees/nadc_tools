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
;	SCIA_RD_H5_NLCORR
;
; PURPOSE:
;	read non-linearity correction keydata to HDF5 file
;
; CATEGORY:
;	Sciamachy calibration
;
; CALLING SEQUENCE:
;       SCIA_RD_H5_NLCORR, nlcorr, curveIndex, NLCORR_DB=NLCORR_DB
;
; INPUTS:
;	nlcorr:        non-linearity correction keydata
;   curveIndex:        curve-to-pixel relation
;
; EXAMPLE:
;	None
;
; MODIFICATION HISTORY:
; 	Written by:	Richard van Hees (SRON), December 2003
;       Modified:  RvH, 03 March 2004
;                    restructed valid range for interplation to 60000
;       Modified:  RvH, 25 August 2011
;                    re-write using IDL HDF5 calls
;-
PRO SCIA_RD_H5_NLCORR, nlcorr, curveIndex, NLCORR_DB=NLCORR_DB
  compile_opt idl2,hidden

  IF N_PARAMS() NE 2 THEN BEGIN
     MESSAGE, ' Usage: SCIA_RD_H5_NLCORR, nlcorr, curveIndex,' $
              + ' NLCORR_DB=NLCORR_DB', /INFO
     RETURN
  ENDIF

  IF N_ELEMENTS( NLCORR_DB ) EQ 0 THEN BEGIN
     IF FILE_TEST( './NLcorr.h5', /READ, /NOEXPAND ) THEN $
        NLCORR_DB = './NLcorr.h5' $
     ELSE BEGIN
        DEFSYSV, '!nadc', exists=i
        IF i NE 1 THEN DEFS_NADC, '/SCIA/share/nadc_tools'
        IF !nadc.dataDir EQ '' THEN $
           NLCORR_DB = '/SCIA/share/nadc_tools/NLcorr.h5' $
        ELSE $
           NLCORR_DB = !nadc.dataDir + '/NLcorr.h5'
     ENDELSE
  ENDIF
  IF (~ FILE_TEST( NLCORR_DB, /READ, /NOEXPAND )) THEN BEGIN
     MESSAGE, ' Can not read from database: ' + NLCORR_DB, /INFO
     RETURN
  ENDIF

  ; open memory correction database
  fid = H5F_OPEN( NLCORR_DB )
  IF fid LT 0 THEN MESSAGE, 'Could not open database: ' + NLCORR_DB

  dd = H5D_OPEN( fid, 'nLinTable' )
  IF dd LT 0 THEN MESSAGE, 'Could not open dataset: nLinTable'
  nlcorr = H5D_READ( dd )
  H5D_CLOSE, dd
  dd = H5D_OPEN( fid, 'CurveIndex' )
  IF dd LT 0 THEN MESSAGE, 'Could not open dataset: CurveIndex'
  curveIndex = H5D_READ( dd )
  H5D_CLOSE, dd
  H5F_CLOSE, fid

  RETURN
END
