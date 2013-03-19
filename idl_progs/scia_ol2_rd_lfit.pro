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
;       SCIA_OL2_RD_LFIT
;
; PURPOSE:
;       read Limb/Occultation Fitting Window Application Data set
;
; CATEGORY:
;       SCIA level 2 Off-line data products
;
; CALLING SEQUENCE:
;       scia_ol2_rd_lfit, lfit_name, dsd, lfit, status=status
;
; INPUTS:
;       dsd :    structure for Data Set Descriptors
;
; OUTPUTS:
;      lfit :    Limb/Occultation Fitting Window Application Data sets
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
;-
;---------------------------------------------------------------------------
PRO SCIA_OL2_RD_LFIT, lfit_name, dsd, lfit, status=status
  compile_opt idl2,logical_predicate,hidden

; initialize the return values
  status = 0
  lfit = -1

; upper limit of several variable size entries in lfit_scia
  MAX_RLEVEL = 30
  MAX_MLEVEL = 20
  MAX_SPECIES = 2
;MAX_CLOSURE = 4
;MAX_OTHER = 5
  MAX_SCALE = 2
  MAX_STVEC = 120
  MAX_ADDD  = 2048
  MAX_ITER  = 20
  MAX_CORRM = (MAX_STVEC * (MAX_STVEC - 1)) / 2
  MAX_RESS  = MAX_STVEC * MAX_ITER

; get index to data set descriptor
  IF lfit_name EQ 'LIM_CLOUDS' THEN return
  indx_dsd = WHERE( STRCOMPRESS(STRING(dsd[*].name), /REMOVE) $
                    EQ lfit_name, count )
  IF count EQ 0 THEN $
     MESSAGE, ' FATAL, could not find keyword: ' + lfit_name

;read Data Set Descriptor records
  num_lfit = dsd[indx_dsd].num_dsr
  IF num_lfit EQ 0 THEN BEGIN
     MESSAGE, ' no LFIT records of type ' + lfit_name + ' found', /INFORM
     lfit = 0
     RETURN
  ENDIF
  lfit = replicate( {lfit_scia}, num_lfit )
  tangh = FLTARR( num_lfit * MAX_RLEVEL )
  tangp = FLTARR( num_lfit * MAX_RLEVEL )
  tangt = FLTARR( num_lfit * MAX_RLEVEL )
  corrmatrix = FLTARR( num_lfit * MAX_CORRM )
  residuals = FLTARR( num_lfit * MAX_RESS )
  addiag = FLTARR( num_lfit * MAX_ADDD )
  mainrec = replicate( {layer_rec}, num_lfit * MAX_RLEVEL * MAX_SPECIES )
  scaledrec = replicate( {layer_rec}, num_lfit * MAX_RLEVEL * MAX_SCALE )
  mgrid = replicate( {meas_grid}, num_lfit * MAX_MLEVEL )
  statevec = replicate( {state_vec}, num_lfit * MAX_STVEC )

  num_dsd = ULONG( SIZE( dsd, /N_ELEMENTS ))
  num = call_external( lib_name('libIDL_NADC'), '_SCIA_OL2_RD_LFIT', $
                       lfit_name, num_dsd, dsd, lfit, tangh, tangp, tangt, $
                       corrmatrix, residuals, addiag, mainrec, scaledrec, $
                       mgrid, statevec, /CDECL )

; check error status
  IF num NE num_lfit THEN BEGIN
     status = -1 
     RETURN
  ENDIF

; check array sizes
  IF (WHERE( lfit.num_rlevel GT MAX_RLEVEL ))[0] NE (-1) THEN BEGIN
     MESSAGE, 'Fatal: MAX_RLEVEL too small, please increase its value', /INFO
     status = 1
     RETURN
  ENDIF
  IF (WHERE( lfit.num_mlevel GT MAX_MLEVEL ))[0] NE (-1) THEN BEGIN
     MESSAGE, 'Fatal: MAX_MLEVEL too small, please increase its value', /INFO
     status = 1
     RETURN
  ENDIF
  IF (WHERE( lfit.num_species GT MAX_SPECIES ))[0] NE (-1) THEN BEGIN
     MESSAGE, 'Fatal: MAX_SPECIES too small, please increase its value', /INFO
     status = 1
     RETURN
  ENDIF
  IF (WHERE( lfit.num_scale GT MAX_SCALE ))[0] NE (-1) THEN BEGIN
     MESSAGE, 'Fatal: MAX_SCALE too small, please increase its value', /INFO
     status = 1
     RETURN
  ENDIF
  IF (WHERE( lfit.stvec_size GT MAX_STVEC ))[0] NE (-1) THEN BEGIN
     MESSAGE, 'Fatal: MAX_STVEC too small, please increase its value', /INFO
     status = 1
     RETURN
  ENDIF
  IF (WHERE( lfit.ressize GT MAX_RESS ))[0] NE (-1) THEN BEGIN
     MESSAGE, 'Fatal: MAX_RESS too small, please increase its value', /INFO
     status = 1
     RETURN
  ENDIF
  IF (WHERE( lfit.num_adddiag GT MAX_ADDD ))[0] NE (-1) THEN BEGIN
     MESSAGE, 'Fatal: MAX_ADDD too small, please increase its value', /INFO
     status = 1
     RETURN
  ENDIF

; dynamically allocate memory to store variable size arrays in output structure
  off_rlev = 0L
  off_mlev = 0L
  off_corrm = 0L
  off_ress = 0L
  off_addd = 0L
  off_main = 0L
  off_scale = 0L
  off_grid = 0L
  off_stvec = 0L
  FOR nr = 0, num_lfit-1 DO BEGIN
     IF lfit[nr].num_rlevel GT 0 THEN BEGIN
        lfit[nr].tangh = $
           PTR_NEW( tangh[off_rlev:off_rlev+lfit[nr].num_rlevel-1] )
        lfit[nr].tangp = $
           PTR_NEW( tangp[off_rlev:off_rlev+lfit[nr].num_rlevel-1] )
        lfit[nr].tangt = $
           PTR_NEW( tangt[off_rlev:off_rlev+lfit[nr].num_rlevel-1] )
        off_rlev += lfit[nr].num_rlevel
     ENDIF
     IF lfit[nr].stvec_size GT 0 THEN BEGIN
        nr_rec = lfit[nr].stvec_size
        lfit[nr].statevec = PTR_NEW( statevec[off_stvec:off_stvec+nr_rec-1] )
        off_stvec += nr_rec
     ENDIF
     IF lfit[nr].cmatrixsize GT 0 THEN BEGIN
        lfit[nr].corrmatrix = $
           PTR_NEW( corrmatrix[off_corrm:off_corrm+lfit[nr].cmatrixsize-1] )
        off_corrm += lfit[nr].cmatrixsize
     ENDIF
     IF lfit[nr].ressize GT 0 THEN BEGIN
        lfit[nr].residuals = $
           PTR_NEW( residuals[off_ress:off_ress+lfit[nr].ressize-1] )
        off_ress += lfit[nr].ressize
     ENDIF
     IF lfit[nr].num_adddiag GT 0 THEN BEGIN
        lfit[nr].addiag = $
           PTR_NEW( addiag[off_addd:off_addd+lfit[nr].num_adddiag-1] )
        off_addd += lfit[nr].num_adddiag
     ENDIF
     nr_rec = lfit[nr].num_rlevel * lfit[nr].num_species
     IF nr_rec GT 0 THEN BEGIN
        lfit[nr].mainrec = $
           PTR_NEW( mainrec[off_main:off_main+nr_rec-1] )
        off_main += nr_rec
     ENDIF
     nr_rec = lfit[nr].num_rlevel * lfit[nr].num_scale
     IF nr_rec GT 0 THEN BEGIN
        lfit[nr].scaledrec = $
           PTR_NEW( scaledrec[off_scale:off_scale+nr_rec-1] )
        off_scale += nr_rec
     ENDIF
     nr_rec = lfit[nr].num_mlevel
     IF nr_rec GT 0 THEN BEGIN
        lfit[nr].mgrid = $
           PTR_NEW( mgrid[off_grid:off_grid+nr_rec-1] )
        off_grid += nr_rec
     ENDIF
  ENDFOR

  RETURN
END
