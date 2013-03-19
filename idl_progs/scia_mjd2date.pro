;
; COPYRIGHT (c) 2002 - 2013 SRON (R.M.van.Hees@sron.nl)
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
;       SCIA_MJD2DATE
;
; PURPOSE:
;       Sciamachy date conversion routine
;
; CALLING SEQUENCE:
;       Result = SCIA_MJD2DATE( mjd )
;
; INPUTS:
;    mjd    :    structure holding the modified julian time for year 2000
;
; OUTPUTS:
;  date_str :    string: dd/mm/yyyy hh:mm:ss.ssssss
;
; EXAMPLES:
;       None
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), February 2002
;
;-
FUNCTION SCIA_MJD2DATE, mjd
  compile_opt idl2,logical_predicate,hidden

  jul2000 = JULDAY(1,1,2000,0,0,0)

  mjd_time = DOUBLE(mjd.days) + mjd.secnd / (60.d * 60 * 24)

  CALDAT, jul2000 + mjd_time, month, day, year, hour, minute, second
  second = second + mjd.musec / 1d6

  RETURN, STRING( FORMAT='(i2.2,a1,i2.2,a1,i4,a2,i2.2,a1,i2.2,a1,f9.6)', $
                  day, '/', month, '/', year, '  ', $
                  hour, ':', minute, ':', second )
END
