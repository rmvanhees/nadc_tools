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
;	GET_SCIA_ROE_INFO
;
; PURPOSE:
;	obtain get orbit phase and SAA flag from ROE records
;
; CATEGORY:
;	
;
; CALLING SEQUENCE:
;       GET_SCIA_ROE_INFO, eclipseMode, jday, absOrbit, saaFlag, orbitPhase
;
; INPUTS:
;	eclipseMode :  TRUE  - orbit phase used for SDMF (v2.4)
;                      FALSE - orbit phase as used by ESA
;       jday        :  julian Day (# days since 2000-01-01)
;
; KEYWORD PARAMETERS:
;
; OUTPUTS:
;	absOrbit    :  absolute orbit number
;       saaFlag     :  in-precise SAA flag
;       orbitPhase  :  orbit phase [0,1]
;
; PROCEDURE:
;	none
;
; EXAMPLE:
;	none
;
; MODIFICATION HISTORY:
; 	Written by:	R.M. van Hees (SRON), 23-10-2012
;-
PRO GET_SCIA_ROE_INFO, eclipseMode, jday, absOrbit, saaFlag, orbitPhase
  compile_opt idl2,logical_predicate,hidden

  eclipseMode = BYTE( eclipseMode )
  jday        = DOUBLE( jday )
  absOrbit    = 0L
  saaFlag     = 0B
  orbitPhase  = 0.
  
  result = call_external( lib_name('libIDL_NADC'), '_GET_SCIA_ROE_INFO', $
                          eclipseMode, jday, absOrbit, saaFlag, orbitPhase, $
                          /CDECL )
  RETURN
END
