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
;       SCIA_OL2_FREE_LFIT
;
; PURPOSE:
;       release pointers in structure LFIT_SCIA
;
; CATEGORY:
;       SCIA level 2 data
;
; CALLING SEQUENCE:
;       SCIA_OL2_FREE_LFIT, lfit
;
; INPUTS:
;       lfit :    Limb/Occultation Fitting Window Application Data sets
;
; EXAMPLES:
;       None
;
; REFERENCE:
;       SCIAMACHY Level 1b to 2 Off-line Processing
;	Input/Output Data Definition
;	Ref. ENV-ID-DLR-SCI-2200-4
;	     Isue 4/A, 09 Aug 2002
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), February 2002
;       Modified:  RvH, 30 Januari 2009
;                    moved from scia_ol2_rd_lfit.pro to seperate module
;       Modified:  RvH, 26 Febuari 2009
;                    optimized the code for speed
;-
;---------------------------------------------------------------------------
PRO SCIA_OL2_FREE_LFIT, lfit
   COMPILE_OPT idl2,logical_predicate,hidden
   
   IF SIZE( lfit, /TNAME ) NE 'STRUCT' THEN RETURN
   IF TAG_NAMES( lfit, /STRUCT ) NE 'LFIT_SCIA' THEN RETURN
   
   PTR_FREE, lfit.tangh
   PTR_FREE, lfit.tangp
   PTR_FREE, lfit.tangt
   PTR_FREE, lfit.corrmatrix
   PTR_FREE, lfit.residuals
   PTR_FREE, lfit.addiag
   PTR_FREE, lfit.mainrec
   PTR_FREE, lfit.scaledrec
   PTR_FREE, lfit.mgrid
   PTR_FREE, lfit.statevec
   p = PTR_NEW( lfit, /NO_COPY ) & PTR_FREE, p
   
   RETURN
END
