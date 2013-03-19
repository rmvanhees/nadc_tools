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
;	SCIA_ORBIT_TO_JD
;
; PURPOSE:
;	This function returns MJD for given orbit number
;
; CATEGORY:
;	
;
; CALLING SEQUENCE:
;	Result = SCIA_ORBIT_TO_JD( orbit_list )
;
; INPUTS:
;	orbit_list:	list of orbit numbers
;
; OUTPUTS:
;	list of modified julian dates (MJD2000) matching the input
;
; RESTRICTIONS:
;	requires the Sciamachy Reference Orbit Event Excerpt to be available
;
; MODIFICATION HISTORY:
; 	Written by:	Richard van Hees (SRON), 22 September 2010
;-
FUNCTION SCIA_ORBIT_TO_JD, orbit_list
  compile_opt idl2, hidden
  common      scia_orbit_to_jd, ROE_jday

  IF n_ELEMENTS( ROE_jday ) EQ 0 THEN BEGIN
     DEFSYSV, '!nadc', EXISTS=defined
     IF defined EQ 1 THEN $
        fid = H5F_OPEN( !nadc.datadir + '/ROE_EXC_all.h5' ) $
     ELSE $
        fid = H5F_OPEN( '/SCIA/share/nadc_tools/ROE_EXC_all.h5' )
     dd = H5D_OPEN( fid, 'julianDay' )
     ROE_jday = H5D_READ( dd )
     H5D_CLOSE, dd
     H5F_CLOSE, fid
  ENDIF

  RETURN, ROE_jday[(orbit_list > 1)-1]
END
