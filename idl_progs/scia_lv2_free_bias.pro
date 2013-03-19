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
;       SCIA_LV2_FREE_BIAS
;
; PURPOSE:
;       release pointers in structure BIAS_SCIA
;
; CATEGORY:
;       SCIA level 2 data
;
; CALLING SEQUENCE:
;       SCIA_LV2_FREE_BIAS, bias
;
; INPUTS:
;       bias :    BIAS Fitting Window Application Data sets
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
;       Modified:  RvH, 30 Januari 2009
;                    moved from scia_lv2_rd_bias.pro to seperate module
;       Modified:  RvH, 26 Febuari 2009
;                    optimized the code for speed
;-
;---------------------------------------------------------------------------
PRO SCIA_LV2_FREE_BIAS, bias
   COMPILE_OPT idl2,logical_predicate,hidden
   
   IF SIZE( bias, /TNAME ) NE 'STRUCT' THEN RETURN
   IF TAG_NAMES( bias, /STRUCT ) NE 'BIAS_SCIA' THEN RETURN
   
   PTR_FREE, bias.corrpar
   p = PTR_NEW( bias, /NO_COPY ) & PTR_FREE, p
   
   RETURN
END
