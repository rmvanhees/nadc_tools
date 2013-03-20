;
; COPYRIGHT (c) 2003 - 2013 SRON (R.M.van.Hees@sron.nl)
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
;       SCIA_OL2_RD_NFIT
;
; PURPOSE:
;       read Nadir Fitting Window Application Data set
;
; CATEGORY:
;       SCIA level 2 Off-line data products
;
; CALLING SEQUENCE:
;       scia_ol2_rd_nfit, nfit_name, dsd, nfit, status=status
;
; INPUTS:
;       dsd :    structure for Data Set Descriptors
;
; OUTPUTS:
;      nfit :    Nadir Fitting Window Application Data sets
;
; KEYWORD PARAMETERS:
;    status :    returns named variable with error status (0 = ok)
;
; EXAMPLES:
;       None
;
; REFERENCE:
;       Volume 15: Envisat-1 Product Specifications
;	
;	Ref. IDEAS-SER-IPF-SPE-0398
;	     Isue 3/L, 21 Jan 2010
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), January 2003
;       Modified:  RvH, 25 March 2003
;                    added checks on allocated memory and array sizes
;       Modified:  RvH, 30 Januari 2009
;                    moved function SCIA_OL2_FREE_NFIT to seperate module
;-
;---------------------------------------------------------------------------
PRO SCIA_OL2_RD_NFIT, nfit_name, dsd, nfit, status=status
  compile_opt idl2,logical_predicate,hidden

; initialize the return values
  status = 0
  nfit = -1

; upper limit of several variable size entries in nfit_scia
  MAX_NUM_VCD = 15
  MAX_LIN_FIT = 15
  MAX_LIN_CROSS = (MAX_LIN_FIT * (MAX_LIN_FIT-1)) / 2
  MAX_NLIN_FIT = 12
  MAX_NLIN_CROSS = (MAX_NLIN_FIT * (MAX_NLIN_FIT-1)) / 2

; get index to data set descriptor 
  indx_dsd = WHERE( STRCOMPRESS(STRING(dsd[*].name), /REMOVE) $
                    EQ nfit_name, count )
  IF count EQ 0 THEN $
     MESSAGE, ' FATAL, could not find keyword: ' + nfit_name

;read Data Set Descriptor records
  num_nfit = dsd[indx_dsd].num_dsr
  IF num_nfit EQ 0 THEN BEGIN
     MESSAGE, ' no NFIT records of type ' + nfit_name + ' found', /INFORM
     nfit = 0
     RETURN
  ENDIF
  nfit = replicate( {nfit_scia}, num_nfit )
  vcd = FLTARR( num_nfit * MAX_NUM_VCD )
  errvcd = FLTARR( num_nfit * MAX_NUM_VCD )
  linpars = FLTARR( num_nfit * MAX_LIN_FIT )
  errlinpars = FLTARR( num_nfit * MAX_LIN_FIT )
  lincorrm = FLTARR( num_nfit * MAX_LIN_CROSS )
  nlinpars = FLTARR( num_nfit * MAX_NLIN_FIT )
  errnlinpars = FLTARR( num_nfit * MAX_NLIN_FIT )
  nlincorrm = FLTARR( num_nfit * MAX_NLIN_CROSS )

  num_dsd = ULONG( SIZE( dsd, /N_ELEMENTS ))
  num = call_external( lib_name('libnadc_idl'), '_SCIA_OL2_RD_NFIT', $
                       nfit_name, num_dsd, dsd, nfit, vcd, errvcd, $
                       linpars, errlinpars, lincorrm, $
                       nlinpars, errnlinpars, nlincorrm, /CDECL )
; check error status
  IF num NE num_nfit THEN BEGIN
     status = -1 
     RETURN
  ENDIF

; check array sizes
  IF (WHERE( nfit.numvcd GT MAX_NUM_VCD ))[0] NE (-1) THEN BEGIN
     MESSAGE, 'Fatal: MAX_NUM_VCD too small, please increase its value', /INFO
     status = 1
     RETURN
  ENDIF
  IF (WHERE( nfit.num_fitp GT MAX_LIN_FIT ))[0] NE (-1) THEN BEGIN
     MESSAGE, 'Fatal: MAX_LIN_FIT too small, please increase its value', /INFO
     status = 1
     RETURN
  ENDIF
  IF (WHERE( nfit.num_nfitp GT MAX_NLIN_FIT ))[0] NE (-1) THEN BEGIN
     MESSAGE, 'Fatal: MAX_NLIN_FIT too small, please increase its value', /INFO
     status = 1
     RETURN
  ENDIF

; dynamically allocate memory to store variable size arrays in output structure
  off_vcd = 0L
  off_lin = 0L
  off_nlin = 0L
  off_cross = 0L
  off_ncross = 0L
  FOR nr = 0, num_nfit-1 DO BEGIN
     IF nfit[nr].numvcd GT 0 THEN BEGIN
        nfit[nr].vcd = PTR_NEW( vcd[off_vcd:off_vcd+nfit[nr].numvcd-1] )
        nfit[nr].errvcd = PTR_NEW( errvcd[off_vcd:off_vcd+nfit[nr].numvcd-1] )
        off_vcd += nfit[nr].numvcd
     ENDIF
     IF nfit[nr].num_fitp GT 0 THEN BEGIN
        nfit[nr].linpars = $
           PTR_NEW( linpars[off_lin:off_lin+nfit[nr].num_fitp-1] )
        nfit[nr].errlinpars = $
           PTR_NEW( errlinpars[off_lin:off_lin+nfit[nr].num_fitp-1] )
        off_lin += nfit[nr].num_fitp

        n_cross = nfit[nr].num_fitp * (nfit[nr].num_fitp - 1) / 2
        nfit[nr].lincorrm = PTR_NEW( lincorrm[off_cross:off_cross+n_cross-1] )
        off_cross += n_cross
     ENDIF
     IF nfit[nr].num_nfitp GT 0 THEN BEGIN
        nfit[nr].nlinpars = $
           PTR_NEW( nlinpars[off_nlin:off_nlin+nfit[nr].num_nfitp-1] )
        nfit[nr].errnlinpars = $
           PTR_NEW( errnlinpars[off_nlin:off_nlin+nfit[nr].num_nfitp-1] )
        off_nlin += nfit[nr].num_nfitp
        
        n_cross = nfit[nr].num_nfitp * (nfit[nr].num_nfitp - 1) / 2
        nfit[nr].nlincorrm = $
           PTR_NEW( nlincorrm[off_ncross:off_ncross+n_cross-1] )
        off_ncross += n_cross
     ENDIF
  ENDFOR

  RETURN
END
