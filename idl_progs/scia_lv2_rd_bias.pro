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
;       SCIA_LV2_RD_BIAS
;
; PURPOSE:
;       read BIAS Fitting Window Application Data set
;
; CATEGORY:
;       SCIA level 2 NRT data products
;
; CALLING SEQUENCE:
;       scia_lv2_rd_bias, bias_name, dsd, bias, status=status
;
; INPUTS:
;       dsd :    structure for Data Set Descriptors
;
; OUTPUTS:
;      bias :    BIAS Fitting Window Application Data sets
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
;                    moved function SCIA_LV2_FREE_BIAS to seperate module
;-
;---------------------------------------------------------------------------
PRO SCIA_LV2_RD_BIAS, bias_name, dsd, bias, status=status
  compile_opt idl2,logical_predicate,hidden

; initialize the return values
  status = 0
  bias = -1

; upper limit of the number of Cross-correlation parameters
  MAX_COEFS_CROSS = (10 * 9) / 2 ; n_fit = 10

; get index to data set descriptor 
  indx_dsd = WHERE( STRCOMPRESS(STRING(dsd[*].name), /REMOVE) $
                    EQ bias_name, count )
  IF count EQ 0 THEN $
     MESSAGE, ' FATAL, could not find keyword: ' + bias_name

;read Data Set Descriptor records
  num_bias = dsd[indx_dsd].num_dsr
  IF num_bias EQ 0 THEN BEGIN
     MESSAGE, ' no BIAS records of type ' + bias_name + ' found', /INFORM
     bias = 0
     RETURN
  ENDIF
  bias = replicate( {bias_scia}, num_bias )
  data = FLTARR( num_bias * MAX_COEFS_CROSS )

  num_dsd = ULONG( SIZE( dsd, /N_ELEMENTS ))
  num = call_external( lib_name('libIDL_NADC'), '_SCIA_LV2_RD_BIAS', $
                       bias_name, num_dsd, dsd, bias, data, /CDECL )

; check error status
  IF num NE num_bias THEN BEGIN
     status = -1 
     RETURN
  ENDIF

  offs = 0L
  FOR nr = 0, num_bias-1 DO BEGIN
     n_cross = bias[nr].numfitp * (bias[nr].numfitp - 1) / 2
     bias[nr].corrpar = PTR_NEW( data[offs:offs+n_cross-1] )
     offs += n_cross
  ENDFOR

  RETURN
END
