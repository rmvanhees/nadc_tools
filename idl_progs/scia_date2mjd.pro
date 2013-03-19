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
;       SCIA_DATE2MJD
;
; PURPOSE:
;        Sciamachy date conversion routine
;
; CALLING SEQUENCE:
;       Result  = SCIA_DATE2MJD( date_str )
;
; INPUTS:
;  date_str :    string in format dd/mm/yyyy hh:mm:ss.ssssss
;
; OUTPUTS:
;  mjd      :    structure holding the modified julian time for year 2000
;
; EXAMPLES:
;       None
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), February 2002
;
;-
FUNCTION SCIA_DATE2MJD, date
  compile_opt idl2,logical_predicate,hidden

  READS, date, day, month, year, hour, minute, secnd, $
         FORMAT='(i2.2,1x,i2.2,1x,i4,2x,i2.2,1x,i2.2,1x,d9.6)'

  mjd = {mjd_scia}
  mjd.musec = ULONG(1d6 * (secnd - ULONG( secnd )))
  mjd.secnd = ULONG((hour * 60L) + minute) * 60L + LONG( secnd )
  mjd.days = LONG(JULDAY(month, day, year, 0,0,0) - JULDAY(1,1,2000,0,0,0))

  RETURN, mjd
END 
