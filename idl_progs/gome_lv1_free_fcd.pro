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
;       GOME_LV1_FREE_FCD
;
; PURPOSE:
;       release pointers in structure FCD_GOME
;
; CATEGORY:
;       GOME level 1b data
;
; CALLING SEQUENCE:
;       GOME_LV1_FREE_FCD, fcd
;
; INPUTS:
;       mds :    structure for Measurement Data Sets (level 1c format)
;
; EXAMPLES:
;       None
;
; REFERENCE:
;       Product Specification Document
;	of the GOME Data Processor
;	Ref. ER-PS-DLR-GO-0016
;	     Iss./Rev.3/C
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), February 2002
;       Modified:  RvH, 30 Januari 2009
;                    moved from gome_lv1_rd_fcd.pro to seperate module
;       Modified:  RvH, 26 Febuari 2009
;                    optimized the code for speed
;-
;---------------------------------------------------------------------------
PRO GOME_LV1_FREE_FCD, fcd
   COMPILE_OPT idl2, logical_predicate,hidden
   
   IF SIZE( fcd, /TNAME ) NE 'STRUCT' THEN RETURN
   IF TAG_NAMES(fcd, /STRUCT) NE 'FCD_GOME' THEN RETURN
   
   PTR_FREE, fcd.leak
   PTR_FREE, fcd.hot
   PTR_FREE, fcd.spec
   PTR_FREE, fcd.calib
   p = PTR_NEW( fcd, /NO_COPY ) & PTR_FREE, p
   
   RETURN
END
