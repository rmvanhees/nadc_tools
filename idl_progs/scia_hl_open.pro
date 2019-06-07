;
; COPYRIGHT (c) 2004 - 2013 SRON (R.M.van.Hees@sron.nl)
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
;	SCIA_HL_OPEN
;
; PURPOSE:
;	Open a Sciamachy file and read the PDS headers: MPH, SPH and DSD
;
; CATEGORY:
;	NADC Sciamachy high level API
;
; CALLING SEQUENCE:
;	SCIA_HL_OPEN, flname, dsd, $
;	              mph=mph, sph=sph, level=level, status=status 
; INPUTS:
;	flname :    string with the name of the Sciamachy file
;
; KEYWORD PARAMETERS:
;	mph:    Main Product Header
;       sph:    Specific Product Header
;       level:  data product level: '0', '1B', '1C', '2', 'UNKNOWN'
;       status: 0 = OK, 
;               use function NADC_ERR_TRACE to examine source of error
;
; OUTPUTS:
;	dsd:    Data Set Descriptor records
;
; SIDE EFFECTS:
;	After a succesful call the Sciamachy file is opened and can be
;	closed using the function SCIA_FCLOSE
;
; PROCEDURE:
;
; EXAMPLE:
;
; MODIFICATION HISTORY:
; 	Written by:	Richard van Hees (SRON), February (2004)
;-
PRO SCIA_HL_OPEN, flname, dsd, mph=mph, sph=sph, level=level, status=status
  compile_opt idl2,logical_predicate,hidden

  IF N_PARAMS() NE 2 THEN BEGIN
     MESSAGE, ' Usage: SCIA_HL_OPEN, flname, dsd, status=status' $
              +      ' [, mph=mph] [, sph=sph] [, level=level]', /INFO
     status = -1
     RETURN
  ENDIF

  IF FILE_TEST( flname, /READ, /NOEXPAND_PATH ) NE 1 THEN BEGIN
     MESSAGE, 'FATAL could not open file for reading', /INFO
     status = -1
     RETURN
  ENDIF
    
  IF SCIA_FOPEN( flname ) NE 0 THEN BEGIN
     MESSAGE, 'FATAL error has occurred while opening file for reading', /INFO
     status = -1
     RETURN
  ENDIF

; read Main Product Header
  SCIA_RD_MPH, mph, status=status
  IF status NE 0 THEN RETURN

  level = GET_SCIA_LEVEL( status=status )
  SWITCH level OF
     '0': BEGIN
        SCIA_LV0_RD_SPH, mph, sph, status=status
        BREAK
     END
     '1B': 
     '1C': BEGIN
        SCIA_LV1_RD_SPH, mph, sph, status=status
        BREAK
     END
     '2O': BEGIN
        SCIA_OL2_RD_SPH, mph, sph, status=status
        BREAK
     END
     'UNKNOWN': BEGIN
        MESSAGE, 'FATAL error unknown Sciamachy data product', /INFO
        status = -1
        RETURN
     END
  ENDSWITCH
  IF status NE 0 THEN RETURN

; read Data Set Descriptor records
  SCIA_RD_DSD, mph, dsd, status=status

  RETURN
END
