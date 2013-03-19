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
;       SCIA_LV0_RD_MDS_INFO
;
; PURPOSE:
;       query the contents a SCIAMACHY level 0 product
;
; CATEGORY:
;       SCIAMACHY level 0
;
; CALLING SEQUENCE:
;       SCIA_LV0_RD_MDS_INFO, flname, dsd, info, status=status
;
; INPUTS:
;       flname :    string with the name of the Sciamachy file
;       dsd    :    structure for the DSD records
;
; OUTPUTS:
;      info    :    structure holding info about MDS records
;
; KEYWORD PARAMETERS:
;   status     :    returns named variable with error status
;                     0   = ok
;                    > 0  = could not read all records from product
;                    < 0  = product corrupt
;
; EXAMPLES:
;       
;
; REFERENCE:
;       
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), February 2002
;       Modified:  RvH, 12 April 2002
;                    added keywords: info_fl and write
;       Modified:  RvH, 28 May 2002
;                    exit when read fails
;       Modified:  RvH, 04 December 2002
;                    added bcps to info-struct, only for for mds0_det
;       Modified:  RvH, 20 December 2002
;                    bugfix: align the mds0_info structure
;       Modified:  RvH, 16 Januari 2006
;                    adopted new function call to C-routine 
;                    SCIA_LV0_RD_MDS_INFO, modified keywords accordingly
;-
PRO SCIA_LV0_RD_MDS_INFO, flname, dsd, info, info_fl=info_fl, status=status
  compile_opt idl2,logical_predicate,hidden

  info = -1
  IF N_PARAMS() NE 3 THEN BEGIN
     MESSAGE, ' Usage: SCIA_LV0_RD_MDS_INFO, flname, dsd, info' $
              + ', status=status'
  ENDIF

  IF N_ELEMENTS( info_fl ) NE 0 THEN $
     MESSAGE, 'keyword info_fl is obsolete, please update your code', /INFO

; collect info about SCIAMACHY source packets
  indx_dsd = WHERE( STRCOMPRESS(STRING(dsd[*].name),/REMOVE) $
                    EQ 'SCIAMACHY_SOURCE_PACKETS' )
  IF indx_dsd EQ (-1) THEN BEGIN
     status = -1
     MESSAGE, ' Fatal: file corrupt or no data-packages present', /INFO
     RETURN
  ENDIF

  num_dsd = ULONG( N_ELEMENTS( dsd ) )
  num_dsr = dsd[indx_dsd].num_dsr
  info = REPLICATE( {mds0_info}, num_dsr )

  num = call_external( lib_name('libIDL_NADC'), '_SCIA_LV0_RD_MDS_INFO', $
                       num_dsd, dsd, info, /CDECL )

; check error status
  IF num EQ num_dsr THEN BEGIN
     status = 0
  ENDIF ELSE BEGIN
     IF num GT 0 THEN BEGIN
        info = info[0:num-1]
        status = 1
     ENDIF ELSE BEGIN
        info = -1
        status = -1
     ENDELSE
  ENDELSE

  RETURN
END
