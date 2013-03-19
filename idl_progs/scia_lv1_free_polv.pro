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
;       SCIA_LV1_FREE_POLV
;
; PURPOSE:
;       release pointers in structure MDS1C_POLV
;
; CATEGORY:
;       SCIA level 1b and 1c data
;
; CALLING SEQUENCE:
;       SCIA_LV1_FREE_POLV, polv
;
; INPUTS:
;       polv :    structure for Polarisation Measurement Data Sets
;
; EXAMPLES:
;       None
;
; REFERENCE:
;       SCIAMACHY Level 0 to 1b Processing
;       Input/Output Data Definition
;       Ref. ENV-TN-DLR-SCIA-0005
;            Isue 5, 21 July 2000
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), February 2002
;       Modified:  RvH, 30 Januari 2009
;                    moved from scia_lv1_rd_mds.pro to seperate module
;       Modified:  RvH, 26 Febuari 2009
;                    optimized the code for speed
;-
;---------------------------------------------------------------------------
PRO SCIA_LV1_FREE_POLV, polV
   COMPILE_OPT idl2,logical_predicate,hidden
   
   IF SIZE( polV, /TNAME ) NE 'STRUCT' THEN RETURN
   IF TAG_NAMES( polV, /STRUCT ) NE 'MDS1C_POLV' THEN RETURN

   PTR_FREE, polV.polV
   PTR_FREE, polV.geoN
   PTR_FREE, polV.geoL
   p = PTR_NEW( polV, /NO_COPY ) & PTR_FREE, p
   
   RETURN
END
