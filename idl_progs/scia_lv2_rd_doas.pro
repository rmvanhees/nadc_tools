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
;       SCIA_LV2_RD_DOAS
;
; PURPOSE:
;       read DOAS Fitting Window Application data sets
;
; CATEGORY:
;       SCIA level 2 NRT data products
;
; CALLING SEQUENCE:
;       scia_lv2_rd_doas, doas_name, dsd, doas, status=status
;
; INPUTS:
;       dsd :    structure for Data Set Descriptors
;
; OUTPUTS:
;      doas :    DOAS Fitting Window Application data sets
;
; KEYWORD PARAMETERS:
;    status :    returns named variable with error status (0 = ok)
;
; EXAMPLES:
;       None
;
; REFERENCE:
;       SCIAMACHY Level 1b to 2 NRT Processing
;	Input/Output Data Definition
;	Ref. ENV-TN-DLR-SCIA-0010
;	     Isue 3/B, 29 May 2000
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), February 2002
;       Modified:  RvH, 22 May 2002
;                    gave Offline and NRT parameters same names
;       Modified:  RvH, 12 Nov 2002
;                    increases MAX_COEFS_CROSS to n_fit = 10
;       Modified:  RvH, 20 Jan 2003
;                    added more failure checking, and documentation
;       Modified:  RvH, 30 Januari 2009
;                    moved function SCIA_LV2_FREE_DOAS to seperate module
;-
;---------------------------------------------------------------------------
PRO SCIA_LV2_RD_DOAS, doas_name, dsd, doas, status=status
  compile_opt idl2,logical_predicate,hidden

; initialize the return values
  status = 0
  doas = -1

; upper limit of the number of Cross-correlation parameters
  MAX_NUM_FITP = 15
  MAX_COEFS_CROSS = (MAX_NUM_FITP * (MAX_NUM_FITP-1)) / 2

; get index to data set descriptor 
  indx_dsd = WHERE( STRCOMPRESS(STRING(dsd[*].name), /REMOVE) $
                    EQ doas_name, count )
  IF count EQ 0 THEN $
     MESSAGE, ' FATAL, could not find keyword: ' + doas_name

;read Data Set Descriptor records
  num_doas = dsd[indx_dsd].num_dsr
  IF num_doas EQ 0 THEN BEGIN
     MESSAGE, ' no DOAS records of type ' + doas_name + ' found', /INFORM
     doas = 0
     RETURN
  ENDIF
  doas = replicate( {doas_scia}, num_doas )
  data = FLTARR( num_doas * MAX_COEFS_CROSS )

  num_dsd = ULONG( SIZE( dsd, /N_ELEMENTS ))
  num = call_external( lib_name('libnadc_idl'), '_SCIA_LV2_RD_DOAS', $
                       doas_name, num_dsd, dsd, doas, data, /CDECL )

; check error status
  IF num NE num_doas THEN BEGIN
     status = -1 
     RETURN
  ENDIF

  offs = 0L
  FOR nr = 0, num_doas-1 DO BEGIN
     n_cross = doas[nr].numfitp * (doas[nr].numfitp - 1) / 2
     doas[nr].corrpar = PTR_NEW( data[offs:offs+n_cross-1] )
     offs += n_cross
  ENDFOR

  RETURN
END
