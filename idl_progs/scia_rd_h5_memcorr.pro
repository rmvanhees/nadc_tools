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
;	SCIA_RD_H5_MEMCORR
;
; PURPOSE:
;	read memory correction keydata to HDF5 file
;
; CATEGORY:
;	Sciamachy calibration
;
; CALLING SEQUENCE:
;       SCIA_RD_H5_MEMCORR, memcorr, MEMCORR_DB=MEMCORR_DB
;
; INPUTS:
;	memcorr:      memory correction keydata
;
; EXAMPLE:
;	None
;
; MODIFICATION HISTORY:
; 	Written by:	Richard van Hees (SRON), December 2003
;       Modified:  RvH, 25 August 2011
;                    re-write using IDL HDF5 calls
;-
PRO SCIA_RD_H5_MEMCORR, memcorr, MEMCORR_DB=MEMCORR_DB
  compile_opt idl2,hidden

  IF N_PARAMS() NE 1 THEN BEGIN
     MESSAGE, ' Usage: SCIA_RD_H5_MEMCORR, memcorr,', $
              + ' MEMCORR_DB=MEMCORR_DB', /INFO
     RETURN
  ENDIF

  IF N_ELEMENTS( MEMCORR_DB ) EQ 0 THEN BEGIN
     IF FILE_TEST( './MEMcorr.h5', /READ, /NOEXPAND ) THEN $
        MEMCORR_DB = './MEMcorr.h5' $
     ELSE BEGIN
        DEFSYSV, '!nadc', exists=i
        IF i NE 1 THEN DEFS_NADC, '/SCIA/share/nadc_tools'
        IF !nadc.dataDir EQ '' THEN $
           MEMCORR_DB = '/SCIA/share/nadc_tools/MEMcorr.h5' $
        ELSE $
           MEMCORR_DB = !nadc.dataDir + '/MEMcorr.h5'
     ENDELSE
  ENDIF
  IF (~ FILE_TEST( MEMCORR_DB, /READ, /NOEXPAND )) THEN BEGIN
     MESSAGE, ' Can not read from database: ' + MEMCORR_DB, /INFO
     RETURN
  ENDIF

  ; open memory correction database
  fid = H5F_OPEN( MEMCORR_DB )
  IF fid LT 0 THEN MESSAGE, 'Could not open database: ' + MEMCORR_DB

  dd = H5D_OPEN( fid, 'MemTable' )
  IF dd LT 0 THEN MESSAGE, 'Could not open dataset: MemTable'
  memcorr = H5D_READ( dd )
  H5D_CLOSE, dd
  H5F_CLOSE, fid

  RETURN
END
