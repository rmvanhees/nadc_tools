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
;	GET_SCIA_LEVEL
;
; PURPOSE:
;	This function identifies the SCIAMACHY product level
;
; CATEGORY:
;	SCIAMACHY
;
; CALLING SEQUENCE:
;	Result = GET_SCIA_LEVEL( flname=flname )
;
; INPUTS:
;
; KEYWORD PARAMETERS:
;      flname:	name of the SCIAMACHY file 
;
; OUTPUTS:
;	This function returns a string, which identifies the level of
;	the product: '0', '1B', '1C', '2N, '2O'', or 'UNKOWN'
;
; MODIFICATION HISTORY:
; 	Written by:	Richard van Hees (SRON), 28 June 2002
;       Modified:  RvH, 07 May 2003
;                    perform more checks on input-file
;       Modified:  RvH, 15 April 2005
;                    added detection of NRT and Offline level 2 products
;-
FUNCTION GET_SCIA_LEVEL, flname=flname, status=status
  compile_opt idl2,logical_predicate,hidden

; initialise return value
level = 'UNKNOWN'

IF N_ELEMENTS( flname ) GT 0 THEN BEGIN
   IF SCIA_FOPEN( flname ) NE 0 THEN BEGIN
      MESSAGE, 'FATAL error has occurred in SCIA_FOPEN', /INFO
      status = -1
      RETURN, level
   ENDIF
ENDIF
SCIA_RD_MPH, mph, status=status
IF status NE 0 THEN GOTO, Jump_Close_File

SCIA_LV0_RD_SPH, mph, sph, status=status
IF status EQ 0 THEN BEGIN
   level = '0'
   GOTO, Jump_Close_File
ENDIF

NADC_ERR_CLEAR
SCIA_LV2_RD_SPH, mph, sph, status=status
IF status EQ 0 THEN BEGIN
   level = '2N'
   GOTO, Jump_Close_File
ENDIF

NADC_ERR_CLEAR
SCIA_OL2_RD_SPH, mph, sph, status=status
IF status EQ 0 THEN BEGIN
   level = '2O'
   GOTO, Jump_Close_File
ENDIF

NADC_ERR_CLEAR
SCIA_LV1_RD_SPH, mph, sph, status=status
IF status NE 0 THEN GOTO, Jump_Close_File
SCIA_RD_DSD, mph, dsd, status=status
IF status NE 0 THEN NADC_ERR_TRACE
indx = WHERE( STRING(dsd.name) EQ 'CAL_OPTIONS', count )
IF count EQ 1 THEN $
   level = '1C' $
ELSE $
   level = '1B'

Jump_Close_File:
IF N_ELEMENTS( flname ) GT 0 THEN BEGIN
   IF SCIA_FCLOSE() NE 0 THEN BEGIN
      MESSAGE, 'FATAL error has occurred in SCIA_FCLOSE', /INFO
      status = -1
      RETURN, 'UNKNOWN'
   ENDIF
ENDIF

RETURN, level
END
