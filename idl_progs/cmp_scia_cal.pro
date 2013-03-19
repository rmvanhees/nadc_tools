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
;	CMP_SCIA_CAL
;
; PURPOSE:
;	This procedure can be used to compare the calibration of
;	Sciamachy data (1b -> 1c) as calculated with the offical
;	SciaL1C processor and NL-SCIA-DC processor
;
; CATEGORY:
;	Sciamachy 1b
;
; CALLING SEQUENCE:
;	CMP_SCIA_CAL, scia_1b_fl, CALIB=CALIB
;
; INPUTS:
;	scia_1b_fl:	file name (including path) of a Sciamachy
;	                level 1b product 
;
; KEYWORD PARAMETERS:
;	CALIB:	string describing the calibration to be applied
;
; OUTPUTS:
;	several (ASCII-) files, one for each state and observation
;	mode: Nadir, Limb, Occultation and Monitoring. Listing the
;	pixel ID, average difference and standard deviation, minimum
;	and maximum of the difference
;
; EXAMPLE:
;        None
;
; MODIFICATION HISTORY:
; 	Written by:	Richard van Hees, October 2002
;       Modified:  RvH, 17 October 2005
;                    several minor bugfixes, added keyword for MDS type
;-
PRO CMP_SCIA_CAL, scia_1b_fl, calib=calib, MDStype=MDStype, $
                  NoSciaL1C=NoSciaL1C, debug=debug
  compile_opt idl2,logical_predicate

; initialisations
  str_type_mds = [ 'nadir', 'limb', 'occultation', 'monitoring' ]
  num_type_mds = N_ELEMENTS( str_type_mds )

  IF N_ELEMENTS( MDStype ) EQ 0 THEN BEGIN
     ids_type_mds = INDGEN( num_type_mds )
  ENDIF ELSE BEGIN
     ids_type_mds = REPLICATE( -1, num_type_mds )
     num = 0
     FOR ni = 0, num_type_mds-1 DO BEGIN
        IF STRMATCH( MDStype, '*' + str_type_mds[ni] + '*', /FOLD ) EQ 1 THEN $
           ids_type_mds[num++] = ni
     ENDFOR
     IF num EQ 0 THEN $
        MESSAGE, 'MDStype should contain at least one out of nadir, ' $
                 + 'limb, occultation or monitoring' $
     ELSE $
        ids_type_mds = ids_type_mds[0:num-1]
     num_type_mds = num
  ENDELSE

; take-out options unknown to SciaL1C
  SciaL1C_bin = '/opt/local/EOS/bin/scial1c'
  envi_calib = ''
  envi_extra = ''
  iindx = STRPOS( calib, '0' )
  IF  iindx GE 0 THEN envi_calib += '0'
  iindx = STRPOS( calib, ',1' ) 
  IF  iindx GE 0 THEN BEGIN
     envi_calib += ',1'
     IF STRPOS( calib, ',1L' ) GT 0 THEN $
        envi_extra = ' -darkflag LIMB' $
     ELSE $
        envi_extra = ' -darkflag GADS'
  ENDIF
  iindx = STRPOS( calib, ',2' ) 
  IF  iindx GE 0 THEN envi_calib += ',2'
  iindx = STRPOS( calib, ',3' ) 
  IF  iindx GE 0 THEN envi_calib += ',3'
  iindx = STRPOS( calib, ',4' ) 
  IF  iindx GE 0 THEN envi_calib += ',4'
  iindx = STRPOS( calib, ',5' ) 
  IF  iindx GE 0 THEN envi_calib += ',5'
  iindx = STRPOS( calib, ',6' ) 
  IF  iindx GE 0 THEN envi_calib += ',6'
  iindx = STRPOS( calib, ',7' ) 
  IF  iindx GE 0 THEN envi_calib += ',7'
;iindx = STRPOS( calib, ',8' ) 
;iindx = STRPOS( calib, ',9' ) 
  envi_calib += envi_extra
  PRINT, SciaL1C_bin + ' -cal ' + envi_calib

; read headers of the level 1b file
  SCIA_HL_OPEN, scia_1b_fl, dsd_1b, status=status
  IF status NE 0 THEN NADC_ERR_TRACE
  SCIA_LV1_RD_CLCP, dsd_1b, clcp, status=status
  IF status NE 0 THEN NADC_ERR_TRACE
  SCIA_LV1_RD_VLCP, dsd_1b, vlcp, status=status
  IF status NE 0 THEN NADC_ERR_TRACE
  SCIA_LV1_RD_STATE, dsd_1b, state_1b, status=status
  IF status NE 0 THEN NADC_ERR_TRACE
  IF SCIA_FCLOSE() NE 0 THEN $
     MESSAGE, 'Fatal error has occurred in SCIA_FCLOSE'

; set name of the Sciamachy level 1c product
  indx = STRPOS( scia_1b_fl, '/', /REVERSE_SEARCH )
  IF indx GE 0 THEN $
     base_scia_fl = STRMID( scia_1b_fl, indx+1 ) $
  ELSE $
     base_scia_fl = scia_1b_fl
  CD, current=temp_dir
  scia_1c_fl = temp_dir + '/' + base_scia_fl + '.child'

  FOR ni = 0, num_type_mds-1  DO BEGIN
     indx = WHERE( state_1b.flag_mds EQ 0 $
                   AND state_1b.type_mds EQ ids_type_mds[ni]+1, nr_state )
     IF nr_state GT 0 THEN BEGIN
        state_id = state_1b[indx].state_id
        uniq_state_id = state_id[UNIQ(state_id, SORT(state_id))]
        diff_state_id = N_ELEMENTS( uniq_state_id )

; create level 1c product
        IF (~ KEYWORD_SET( NoSciaL1C )) THEN BEGIN
           FILE_DELETE, scia_1c_fl, /QUIET
           IF calib NE '' THEN BEGIN
              SPAWN, SciaL1C_bin $
                     + ' -type ' + str_type_mds[ids_type_mds[ni]] $
                     + ' -out ' + temp_dir $
                     + ' -cal ' + envi_calib  $
                     + ' ' + scia_1b_fl, EXIT_STATUS=status
           ENDIF ELSE BEGIN
              SPAWN, SciaL1C_bin $
                     + ' -type ' + str_type_mds[ids_type_mds[ni]] $
                     + ' -out ' + temp_dir $
                     + ' ' + scia_1b_fl, EXIT_STATUS=status
           ENDELSE
           IF status NE 0 THEN MESSAGE, 'Fatal error occurred during SciaL1C'
        ENDIF

; read headers of the level 1c file
        SCIA_HL_OPEN, scia_1c_fl, dsd_1c, status=status
        IF status NE 0 THEN NADC_ERR_TRACE
        SCIA_LV1_RD_STATE, dsd_1c, state_1c, status=status
        IF status NE 0 THEN NADC_ERR_TRACE
        IF SCIA_FCLOSE() NE 0 THEN $
           MESSAGE, 'Fatal error has occurred in SCIA_FCLOSE'

; loop through all different states
        FOR ns = 0, diff_state_id-1 DO BEGIN
           PRINT, FORMAT='( "State_ID = ", I0 )', uniq_state_id[ns]

; open level 1b file, and read MDS data
           IF SCIA_FOPEN( scia_1b_fl ) NE 0 THEN $
              MESSAGE, 'Fatal error has occurred in SCIA_FOPEN'
           SCIA_LV1_RD_MDS, dsd_1b, mds_1b, state_id=uniq_state_id[ns], $
                            calibration=calib, status=status
           IF status NE 0 THEN NADC_ERR_TRACE
           IF SCIA_FCLOSE() NE 0 THEN $
              MESSAGE, 'Fatal error has occurred in SCIA_FCLOSE'

; open level 1c file, and read MDS data
           IF SCIA_FOPEN( scia_1c_fl ) NE 0 THEN $
              MESSAGE, 'Fatal error has occurred in SCIA_FOPEN'
           SCIA_LV1C_RD_MDS, dsd_1c, mds_1c, state_id=uniq_state_id[ns], $
                             status=status
           IF status NE 0 THEN NADC_ERR_TRACE
           IF SCIA_FCLOSE() NE 0 THEN $
              MESSAGE, 'Fatal error has occurred in SCIA_FCLOSE'

           IF N_ELEMENTS( mds_1b ) NE N_ELEMENTS( mds_1c ) THEN stop
           IF KEYWORD_SET( debug ) THEN BEGIN
              data_1b = GET_LV1_MDS_DATA( mds_1b, -1 )
              data_1c = GET_LV1_MDS_DATA( mds_1c, -1 )
              STOP
           ENDIF

           num_rep_states = N_ELEMENTS( UNIQ(mds_1b.state_index) )
           indx_uniq = [-1,UNIQ(mds_1b.state_index)]
           FOR num_rep = 0, num_rep_states-1 DO BEGIN
; open output file and collect some statistics
              flname = str_type_mds[ids_type_mds[ni]] $
                       + STRING( FORMAT='(A,I0,A,I0,A,I0)', '_', calib, '_', $
                                 uniq_state_id[ns], '-', num_rep )
              OPENW, unit, flname , /Get_Lun

              wave_1b = 1
              wave_1c = 1
              ipix = 0
              FOR ch = 0, 7 DO BEGIN

; obtain Science data
                 indx_mn = indx_uniq[num_rep] + 1
                 indx_mx = indx_uniq[num_rep+1]
                 data_1b = GET_LV1_MDS_DATA( mds_1b[indx_mn:indx_mx], -1, $
                                             channel=ch+1, wave=wave_1b )
                 data_1c = GET_LV1_MDS_DATA( mds_1c[indx_mn:indx_mx], -1, $
                                             channel=ch+1, wave=wave_1c )

                 sz = SIZE( data_1b )
                 IF str_type_mds[ids_type_mds[ni]] EQ 'limb' THEN BEGIN
                    IF sz[0] EQ 2 THEN BEGIN ; skip last limb measurements
                       data_1b = data_1b[*,0:sz[2]-2]
                       data_1c = data_1c[*,0:sz[2]-2]
                    ENDIF ELSE BEGIN
                       data_1b = data_1b[*,0:sz[2]-2,*]
                       data_1c = data_1c[*,0:sz[2]-2,*]
                    ENDELSE
                    sz[2]--
                 ENDIF
                 REPLICATE_INPLACE, (val_1b = data_1b), !VALUES.D_NAN
                 REPLICATE_INPLACE, (val_1c = data_1c), !VALUES.D_NAN
                 REPLICATE_INPLACE, (diff = data_1b), !VALUES.D_NAN
                 REPLICATE_INPLACE, (derror = data_1b), 1.d
                 ijndx = WHERE( data_1b GT 0 AND $
                                FINITE(data_1b) AND FINITE(data_1c), count )
                 IF count GT 0 THEN BEGIN
                    val_1b[ijndx] = data_1b[ijndx]
                    val_1c[ijndx] = data_1c[ijndx]
                    diff[ijndx] = DOUBLE(data_1c[ijndx]) - data_1b[ijndx]
                    derror[ijndx] = DOUBLE(data_1c[ijndx]) / data_1b[ijndx]
                 ENDIF

                 IF STRPOS( calib, '5' ) GE 0 THEN BEGIN
                    wv_diff = wave_1c - wave_1b
                    FOR ip = 0, sz[1]-1 DO BEGIN
                       ijndx = WHERE( FINITE(diff[ip,*]), count )
                       IF count EQ 0 THEN BEGIN
                          val_1b_avg    = !VALUES.D_NAN
                          val_1c_avg    = !VALUES.D_NAN
                          res_diff_avg  = !VALUES.D_NAN
                          res_diff_mdev = 0.d
                          res_err_avg   = !VALUES.D_NAN
                          res_err_mdev  = 0
                       ENDIF ELSE IF count EQ 1 THEN BEGIN
                          val_1b_avg    = val_1b[ip]
                          val_1c_avg    = val_1c[ip]
                          res_diff_avg  = diff[ip] 
                          res_diff_mdev = 0.d
                          res_err_avg   = derror[ip] 
                          res_err_mdev  = 0.d
                       ENDIF ELSE BEGIN
                          val_1b_avg    = MEAN( val_1b[ip,*], /NaN )
                          val_1c_avg    = MEAN( val_1c[ip,*], /NaN )
                          res_diff_avg  = MEAN( diff[ip,*], /NaN )
                          res_diff_mdev = MEANABSDEV( diff[ip,*], /NaN )
                          res_err_avg   = MEAN( derror[ip,*], /NaN )
                          res_err_mdev  = MEANABSDEV(derror[ip,*], /NaN )
                       ENDELSE
                            
                       IF SIZE( wv_diff, /N_DIM ) EQ 2 THEN BEGIN
                          wv_result_avg  = MEAN( wv_diff[ip], /NaN )
                          wv_result_mdev = MEANABSDEV(wv_diff[ip], /NaN )
                       ENDIF ELSE BEGIN
                          wv_result_avg  = wv_diff[ip]
                          wv_result_mdev = 0.
                       ENDELSE
                       PRINTF, FORMAT='(I5,6G14.6,2G14.8)', unit, $
                               ipix, res_err_avg, SQRT( res_err_mdev ), $
                               res_diff_avg, SQRT( res_diff_mdev ), $
                               val_1b_avg, val_1c_avg, $
                               wv_result_avg, SQRT( wv_result_mdev )
                       ipix = ipix + 1
                    ENDFOR
                 ENDIF ELSE BEGIN
                    FOR ip = 0, sz[1]-1 DO BEGIN
                       ijndx = WHERE( FINITE(diff[ip,*]), count )
                       IF count EQ 0 THEN BEGIN
                          val_1b_avg    = !VALUES.D_NAN
                          val_1c_avg    = !VALUES.D_NAN
                          res_diff_avg  = !VALUES.D_NAN
                          res_diff_mdev = 0.d
                          res_err_avg   = !VALUES.D_NAN
                          res_err_mdev  = 0
                       ENDIF ELSE IF count EQ 1 THEN BEGIN
                          val_1b_avg    = val_1b[ip]
                          val_1c_avg    = val_1c[ip]
                          res_diff_avg  = diff[ip] 
                          res_diff_mdev = 0.d
                          res_err_avg   = derror[ip] 
                          res_err_mdev  = 0.d
                       ENDIF ELSE BEGIN
                          val_1b_avg    = MEAN( val_1b[ip,*], /NaN )
                          val_1c_avg    = MEAN( val_1c[ip,*], /NaN )
                          res_diff_avg  = MEAN( diff[ip,*], /NaN )
                          res_diff_mdev = MEANABSDEV( diff[ip,*], /NaN )
                          res_err_avg   = MEAN( derror[ip,*], /NaN )
                          res_err_mdev  = MEANABSDEV(derror[ip,*], /NaN )
                       ENDELSE
                       PRINTF, FORMAT='(I5,4G14.6,2G14.8)', unit, $
                               ipix, res_err_avg, SQRT( res_err_mdev ), $
                               res_diff_avg, SQRT( res_diff_mdev ), $
                               val_1b_avg, val_1c_avg
                       ipix = ipix + 1
                    ENDFOR
                 ENDELSE
              ENDFOR
              FREE_LUN, unit
           ENDFOR

; release allocated memory
           SCIA_LV1_FREE_MDS, mds_1b
           SCIA_LV1_FREE_MDS, mds_1c
        ENDFOR
     ENDIF
  ENDFOR

  RETURN
END
