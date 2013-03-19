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
;       SCIA_LV1_RD_MDS
;
; PURPOSE:
;       read MDS of one state from a SCIAMACHY level 1b product
;
; CATEGORY:
;       SCIA level 1b data
;
; CALLING SEQUENCE:
;       SCIA_LV1_RD_MDS, dsd, mds, status=status, NotFree=NotFree, $
;              type_mds=type_mds, category=category, state_id=state_id, $
;              period=period, geolocation=geolocation, NoSAA=NoSAA, $
;              indx_state=indx_state, num_state=num_state, $
;              channels=channels, clusters=clusters, $
;              calibration=calibration, OrbitPhase=OrbitPhase, $
;              pmd=pmd, polV=polV, NoMDS=NoMDS
;
; INPUTS:
;       dsd :    structure for Data Set Descriptors
;
; OUTPUTS:
;       mds :    structure for Measurement Data Sets (level 1c format)
;
; KEYWORD PARAMETERS:
;    type_mds :  scalar of type integer to select MDS of type: 
;                {!nadc.sciaNadir=1, !nadc.sciaLimb=2, $
;                 !nadc.sciaOccult=3, !nadc.sciaMonitor=4}
;    category :  read only selected category of measurements (scalar or array)
;                (default or -1: all categories)
;    state_id :  read only selected states (scalar or array)
;                (default or -1: all states)
;  OrbitPhase :  selection on orbit phase, required are 2 values. The
;                orbit phase seletion is based on "States of the Product"
; geolocation :  vector of type float which defines the geografical
;                region as [lat_min,lat_max,lon_min,lon_max] (using LADS)
;       NoSAA :  set this keyword to inhibit to reject states with the
;                SAA set (using SQADS) 
;      period :  read only MDS within a time-window (scalar or array)
;                date must be given in decimal julian 2000 day
;                (default or -1: all data)
; state_posit :  relative index/indices to the state record(s), this
;                is last selection applied, only counting those MDS(s)
;                which contain data (default or -1: all data)
;  indx_state :  named variable which contains the indices to the
;                original state records as defined in the level 1b/1c
;                product
;   num_state :  named variable which contains the number of selected states
;
;    channels :  read only selected channels (scalar or array)
;                (default or -1: all channels)
;    clusters :  read only selected clusters (scalar or array)
;                (default or -1: all clusters)
; calibration :  string describing the calibration to be applied
;                (default or -1: no calibration of the MDS data)
;
;                0 : Memory Effect 
;                1 : Leakage Correction 
;                2 : PPG Correction 
;                3 : Etalon Correction 
;                4 : StrayLight Correction 
;                5 : WaveLength Calibration 
;                6 : Polarisation Sensitivity 
;                7 : Radiance Sensitivity 
;                8 : Division by Solar spectrum
;                9 : Bad/Dead pixel mask
;
;         pmd : set this keyword to a named variable that will
;                contain the level 1c PMD-MDS
;        polV : set this keyword to a named variable that will
;                contain the level 1c Polarisation values MDS
;       NoMDS : set this keyword to inhibit reading of MDS data. On
;                return the value of MDS will be zero
;     NotFree : set this keyword to inhibit deallocation of the
;                pointers in the input structure (Experts only!)
;      status : named variable which contains the error status (0 = ok)
;
; SIDE EFFECTS:
;       struct "mds" contains pointers to data. These have to be handled
;       carefully. The routine SCIA_LV1_FREE_MDS should be used to
;       release heap variables allocated by SCIA_LV1_RD_MDS. The
;       program SCIA_LV1_RD_MDS may be called more than once, using
;       the same output variable, because any valid pointer associated
;       with the output variable will be released using
;       SCIA_LV1_FREE_MDS
;
; ENVIRONMENT VARIABLE "SCIA_CORR_LOS":
;       The values of the level 1b line-of-sight zenith angles are
;       always larger than zero, and the azimuth angle jumps with 180
;       degrees while scanning through nadir. 
;       Setting the environment variable "SCIA_CORR_LOS" to one will
;       modify these values as follows: removing the jump in the
;       azimuth angles and returns negative zenith angles, when the
;       original azimuth angle was larger than 180 degree.
;
; EXAMPLES:
;       - read all Limb states
;       SCIA_LV1_RD_MDS, dsd, mds, type_mds=!nadc.sciaLimb, status=status
;
;       - read all Nadir states
;       SCIA_LV1_RD_MDS, dsd, mds, type_mds=!nadc.sciaNadir, status=status
;
;       - read all Nadir states with state_id=7
;       SCIA_LV1_RD_MDS, dsd, mds, type_mds=!nadc.sciaNadir, $
;                        state_id=7, status=status
;
;        - as above, but read no MDS records only PMD-MDS records
;       pmd = 1
;       SCIA_LV1_RD_MDS, dsd, mds, type_mds=!nadc.sciaNadir, $
;                        state_id=7, status=status, /NoMDS, pmd=pmd
;
;       - read all Nadir states with state_id=7, and return indices to
;         occurrence of the state in the Scia product
;       SCIA_LV1_RD_MDS, dsd, mds, type_mds=!nadc.sciaNadir, $
;                        state_id=7, indx_state=indx_state, status=status
;
;       - read all Nadir states with state_id=7, only channels 7 & 8
;       SCIA_LV1_RD_MDS, dsd, mds, type_mds=!nadc.sciaNadir, $
;                        state_id=7, channels=[7,8], status=status
;
;       - read all Nadir states with state_id=7, only channels 7 & 8,
;         apply calibration
;       SCIA_LV1_RD_MDS, dsd, mds, type_mds=!nadc.sciaNadir, $
;                        state_id=7, channels=[7,8], $
;                        calibration='0,1,2,3,4,5,6', status=status
;
; REFERENCE:
;       SCIAMACHY Level 0 to 1b Processing
;       Input/Output Data Definition
;       Ref. ENV-TN-DLR-SCIA-0005
;            Isue 5, 21 July 2000
;
; MODIFICATION HISTORY:
;       Written by:  Richard van Hees (SRON), February 2002
;       Modified:  RvH, 6 August 2002
;                    added keyword /NoMDS
;       Modified:  RvH, 6 August 2002
;                    complete rewrite
;       Modified:  RvH, 2 August 2002
;                    added selection of clusters
;       Modified:  RvH, 9 September 2002
;                    SCIA_LV1_RD_MDS: removed bug for option /NoMDS
;       Modified:  RvH, 13 September 2002
;                    SCIA_LV1_ONE_MDS: bugs for channel/cluster selection
;       Modified:  RvH, 9 October 2002
;                    SCIA_LV1C_ONE_MDS: Aaargh bug :-(
;                    number of MDS could exceed number of clusters
;       Modified:  RvH, 9 October 2002
;                    GET_LV1_STATES: forgot to check "state.flag_mds"
;       Modified:  RvH, 9 October 2002
;                    SCIA_LV1_FREE_MDS: bug in TAG_NAMES test
;       Modified:  RvH, 11 October 2002
;                    the external functions are modified to return -1 
;                    in case of an error. Thus now I check the return
;                    value of LV1C_RD_MDS_PMD & LV1C_RD_MDS_POLV,
;                    because a level 1c file may contain none of these
;                    structures
;       Modified:  RvH, 11 October 2002
;                    SCIA_LV1_RD_MDS & SCIA_LV1C_RD_MDS, did not check
;                    the variables returned by the reading routines
;                    to be a structure
;       Modified:  RvH, 31 October 2002
;                    keywords PMD and POLV are checked with ARG_PRESENT
;       Modified:  RvH, 15 November 2002
;                    implmented new calibration options: Set_SCIA_Calib
;       Modified:  RvH, 19 November 2002
;                    check input file on data format: 1b or 1c, RvH
;       Modified:  RvH, 19 November 2002
;                    implmented new cluster mask routine: Set_SCIA_ClusMask
;       Modified:  RvH, 11 December 2002
;                    bugfix: incorrect use of CAL_OPTIONS for level 1c product
;       Modified:  RvH, 12 December 2002
;                    added option to calculate the reflectance
;       Modified:  RvH, 16 April 2003
;                    check number of state after calling GET_LV1_STATES
;       Modified:  RvH, 17 April 2003
;                    check if type_mds has a valid value
;       Modified:  RvH, 12 May 2003
;                    debugged MDS selection using type_mds;
;                    silently ignore type_mds=-1
;       Modified:  RvH, 2 Dec 2003
;                    added integration time to structure polV for Thijs
;       Modified:  RvH, 15 Mar 2004
;                    added selection on SAA flag from SQADS
;       Modified:  RvH, 22 Mar 2004
;                    moved GET_LV1_STATES to separate module GET_LV1_MDS_STATE
;                    added geolocation selection
;                    removed obsolete paramter STATE
;       Modified:  RvH, 31 Aug 2004
;                    handle invalid cluster selection, gracefully
;                    make sure we release MDS data from previous calls
;       Modified:  RvH, 08 Dec 2004
;                    bug fix: status=-1 when /NoMDS is used
;                    added selection on orbit phase
;       Modified:  RvH, 19 Jan 2005
;                    bug fix: SCIA_LV1C_RD_POLV no need to use the
;                    routine: CREATE_STRUCT
;       Modified:  RvH, 07 Jul 2005
;                    Do not stop reading data when the Level 1c
;                    product doesn't contain PMD/polV records
;                    and the keywords PMD or POLV are used
;       Modified:  RvH, 07 December 2005
;                    renamed pixel_val_err to pixel_err
;       Modified:  RvH, 16 Januari 2006
;                    SCIA_LV1_FREE_MDS: did not release geoC for monitor MDS
;       Modified:  RvH, 30 Januari 2009
;                    put the different procedures in seperate modules
;-
;---------------------------------------------------------------------------
FUNCTION Set_SCIA_Calib, calib_str
  compile_opt idl2,hidden

  RETURN, call_external( lib_name('libIDL_NADC'), '_NADC_SCIA_CalibMask', $
                         calib_str, /UL_VALUE, /CDECL )
END

;---------------------------------------------------------------------------
FUNCTION Set_SCIA_ClusMask, state, channels=channels, clusters=clusters
  compile_opt idl2,logical_predicate,hidden

; selection on clusters
  IF N_ELEMENTS( clusters ) EQ 0 THEN BEGIN
     clus_mask = (NOT 0ull)
  ENDIF ELSE BEGIN
     IF clusters[0] EQ -1 THEN BEGIN
        clus_mask = (NOT 0ull)
     ENDIF ELSE BEGIN
        clus_mask = 0ull
        FOR nb = 0, N_ELEMENTS( clusters )-1 DO BEGIN
           clus_mask = clus_mask OR ISHFT( 1ull, clusters[nb]-1 )
        ENDFOR
     ENDELSE
  ENDELSE

  IF N_ELEMENTS( channels ) NE 0 THEN BEGIN
     IF channels[0] GT 0 THEN BEGIN
        chan_mask = 0ull
        FOR nb = 0, N_ELEMENTS( channels )-1 DO BEGIN
           iindx = WHERE( state.Clcon[*].channel EQ channels[nb], count )
           FOR nc = 0, count-1 DO BEGIN
              chan_mask = chan_mask $
                          OR ISHFT( 1ull, state.Clcon[iindx[nc]].id-1 )
           ENDFOR
        ENDFOR
        clus_mask = clus_mask AND chan_mask
     ENDIF
  ENDIF

  RETURN, clus_mask
END

;---------------------------------------------------------------------------
PRO SCIA_LV1_RD_ONE_MDS, state, mds, status=status, channels=channels, $
                         clusters=clusters, calibration=calibration
  compile_opt idl2,logical_predicate,hidden

; check required parameters
  IF N_PARAMS() NE 2 THEN BEGIN
     MESSAGE, ' Usage: SCIA_LV1_RD_ONE_MDS, state, mds, status=status', $
              + ', channels=channels, clusters=clusters', $
              + ', calibration=calibration', /INFO
     status = -1
     RETURN
  ENDIF

; does the state contains any data?
  IF state.num_dsr EQ 0 THEN RETURN

; initialisation of some returned variables
  status = 0

; definition of SCIAMACHY related constants
  NotSet = -1

; obtain calibration mask
  calib_mask = '0'xu
  IF N_ELEMENTS( calibration ) GT 0 THEN BEGIN
     calib_mask = Set_SCIA_Calib( calibration )
  ENDIF

; obtain cluster mask
  clus_mask = Set_SCIA_ClusMask( state, channels=channels, clusters=clusters )

; get number of MDS records, which can not exceed the number of
; clusters for this state 
  num_mds = 0
  FOR nb = 0, !nadc.maxCluster-1 DO BEGIN
     IF (clus_mask AND ISHFT( 1ull, nb )) GT 0ull THEN num_mds++
  ENDFOR
  num_mds = num_mds < state.num_clusters
  IF num_mds EQ 0 THEN RETURN

; calculate some constant for this state
  nm = 0
  num_obs = ULONARR( num_mds )
  num_pixels = ULONARR( num_mds )
  total_dsr = 0ul
  total_pixels = 0ul
  total_obs = 0ul
  FOR nc = 0, state.num_clusters-1 DO BEGIN
     IF (clus_mask AND ISHFT( 1ull, state.Clcon[nc].id-1 )) NE 0ull THEN BEGIN
        num_obs[nm] = state.Clcon[nc].n_read * ULONG(state.num_dsr)
        num_pixels[nm] = state.Clcon[nc].length

        total_dsr += num_obs[nm]
        total_pixels += num_pixels[nm]
        total_obs += (num_obs[nm] * num_pixels[nm])
        nm++
     ENDIF
  ENDFOR
  IF nm EQ 0 THEN BEGIN
     MESSAGE, 'no data found', /INFO
     RETURN
  ENDIF

; allocate memory for the Detector MDS records
  mds = REPLICATE( {mds1c_scia}, num_mds )
  pixel_ids = UINTARR( total_pixels )
  pixel_wv = FLTARR( total_pixels )
  pixel_wv_err = FLTARR( total_pixels )
  pixel_val = FLTARR( total_obs )
  pixel_err = FLTARR( total_obs )
  geoC = 0 & geoL = 0 & geoN = 0
  CASE FIX( state.type_mds ) OF
     !nadc.sciaNadir:   geoN = REPLICATE( {geoN_scia}, total_dsr )
     !nadc.sciaLimb:    geoL = REPLICATE( {geoL_scia}, total_dsr )
     !nadc.sciaOccult:  geoL = REPLICATE( {geoL_scia}, total_dsr )
     !nadc.sciaMonitor: geoC = REPLICATE( {geoC_scia}, total_dsr )
  ENDCASE

; read MDS data
  num = call_external( lib_name('libIDL_NADC'), '_SCIA_LV1_RD_MDS', $
                       state, clus_mask, calib_mask, mds, $
                       pixel_ids, pixel_wv, pixel_wv_err, pixel_val, $
                       pixel_err, geoC, geoL, geoN, /CDECL )
  IF num NE num_mds THEN BEGIN
     MESSAGE, 'Number of expected MDS differs from number found in file: ' $
              + STRING( FORMAT='(i2,a,i2)', num, ' != ', num_mds ), /INFO
     status = -1
     mds = 0
     RETURN
  ENDIF

  offs_pix = 0ul
  offs_obs = 0ul
  offs_dsr = 0ul
  FOR nr = 0, num_mds-1 DO BEGIN
     IF num_obs[nr] NE mds[nr].num_obs THEN $
        PRINT, 'Num_OBS: ', num_obs[nr], mds[nr].num_obs
     IF num_pixels[nr] NE mds[nr].num_pixels THEN $
        PRINT, 'Num_pixels: ', num_pixels[nr], mds[nr].num_pixels

     mds[nr].pixel_ids = $
        PTR_NEW( pixel_ids[offs_pix:offs_pix+num_pixels[nr]-1] )
     mds[nr].pixel_wv = $
        PTR_NEW( pixel_wv[offs_pix:offs_pix+num_pixels[nr]-1] )
     mds[nr].pixel_wv_err = $
        PTR_NEW( pixel_wv_err[offs_pix:offs_pix+num_pixels[nr]-1] )

     nrpix = num_obs[nr] * num_pixels[nr]
     mds[nr].pixel_val = $
        PTR_NEW( REFORM( pixel_val[offs_obs:offs_obs+nrpix-1], $
                         num_pixels[nr], num_obs[nr] ) )
     mds[nr].pixel_err = $
        PTR_NEW( REFORM( pixel_err[offs_obs:offs_obs+nrpix-1], $
                         num_pixels[nr], num_obs[nr] ) )
     CASE FIX( state.type_mds ) OF
        !nadc.sciaNadir  : BEGIN
           mds[nr].geoN = PTR_NEW( geoN[offs_dsr:offs_dsr+num_obs[nr]-1] )
        END
        !nadc.sciaLimb   : BEGIN
           mds[nr].geoL = PTR_NEW( geoL[offs_dsr:offs_dsr+num_obs[nr]-1] )
        END
        !nadc.sciaOccult : BEGIN
           mds[nr].geoL = PTR_NEW( geoL[offs_dsr:offs_dsr+num_obs[nr]-1] )
        END
        !nadc.sciaMonitor: BEGIN
           mds[nr].geoC = PTR_NEW( geoC[offs_dsr:offs_dsr+num_obs[nr]-1] )
        END
     ENDCASE
     offs_pix += num_pixels[nr]
     offs_obs += nrpix
     offs_dsr += num_obs[nr]
  ENDFOR

  RETURN
END

;---------------------------------------------------------------------------
PRO SCIA_LV1_RD_ONE_PMD, state_in, mds_pmd, status=status
  compile_opt idl2,logical_predicate,hidden

; check required parameters
  IF N_PARAMS() NE 2 THEN BEGIN
     MESSAGE, ' Usage: SCIA_LV1_RD_ONE_PMD, state, mds_pmd', $
              + ', status=status', /INFO
     status = -1
     RETURN
  ENDIF

; does the state contains any data?
  state = state_in
  IF state.num_aux EQ 0 THEN RETURN

; initialisation of some returned variables
  status = 0

; definition of SCIAMACHY related constants
  NotSet = -1

; calculate some constant for this state
  num_geo = state.num_pmd       ;

  mds_pmd = {mds1c_pmd}
  int_pmd = FLTARR( !nadc.numberPMD, state.num_pmd )
  geoL = 0 & geoN = 0
  CASE FIX( state.type_mds ) OF
     !nadc.sciaNadir  : geoN = REPLICATE( {geoN_scia}, num_geo )
     !nadc.sciaLimb   : geoL = REPLICATE( {geoL_scia}, num_geo )
     !nadc.sciaOccult : geoL = REPLICATE( {geoL_scia}, num_geo )
     ELSE: RETURN
  ENDCASE
  num = call_external( lib_name('libIDL_NADC'), '_SCIA_LV1_RD_MDS_PMD', $
                       state, mds_pmd, int_pmd, geoL, geoN, /CDECL )
  IF num LT 1 THEN BEGIN
     status = -1
     mds_pmd = 0
     RETURN
  ENDIF

  mds_pmd.int_pmd = PTR_NEW( int_pmd, /NO_COPY )
  CASE FIX( state.type_mds ) OF
     !nadc.sciaNadir  : BEGIN
        mds_pmd.geoN = PTR_NEW( geoN, /NO_COPY )
     END
     !nadc.sciaLimb   : BEGIN
        mds_pmd.geoL = PTR_NEW( geoL, /NO_COPY )
     END
     !nadc.sciaOccult : BEGIN
        mds_pmd.geoL = PTR_NEW( geoL, /NO_COPY )
     END
  ENDCASE

  RETURN
END

;---------------------------------------------------------------------------
PRO SCIA_LV1_RD_ONE_POLV, state_in, mds_polV, status=status
  compile_opt idl2,logical_predicate,hidden

; check required parameters
  IF N_PARAMS() NE 2 THEN BEGIN
     MESSAGE, ' Usage: SCIA_LV1_RD_ONE_POLV, state, mds_polV', $
              + ', status=status', /INFO
     status = -1
     RETURN
  ENDIF

; does the state contains any data?
  state = state_in
  IF state.num_aux EQ 0 THEN RETURN

; initialisation of some returned variables
  status = 0

; calculate some constant for this state
  total_polV = 0u
  FOR nr = 0, state.num_intg-1 DO total_polV += state.num_polar[nr]
  num_geo = total_polV

  mds_polV = {mds1c_polV}
  polV = REPLICATE( {polV_scia}, total_polV )
  geoL = 0 & geoN = 0
  CASE FIX( state[0].type_mds ) OF
     !nadc.sciaNadir  : geoN = REPLICATE( {geoN_scia}, num_geo )
     !nadc.sciaLimb   : geoL = REPLICATE( {geoL_scia}, num_geo )
     !nadc.sciaOccult : geoL = REPLICATE( {geoL_scia}, num_geo )
     ELSE: RETURN
  ENDCASE
  num = call_external( lib_name('libIDL_NADC'), '_SCIA_LV1_RD_MDS_POLV', $
                       state, mds_polV, polV, geoL, geoN, /CDECL )
  IF num LT 1 THEN BEGIN
     status = -1
     mds_polV = 0
     RETURN
  ENDIF

  mds_polV.polV = PTR_NEW( polV, /NO_COPY )
  CASE FIX( state.type_mds ) OF
     !nadc.sciaNadir  : BEGIN
        mds_polV.geoN = PTR_NEW( geoN, /NO_COPY )
     END
     !nadc.sciaLimb   : BEGIN
        mds_polV.geoL = PTR_NEW( geoL, /NO_COPY )
     END
     !nadc.sciaOccult : BEGIN
        mds_polV.geoL = PTR_NEW( geoL, /NO_COPY )
     END
  ENDCASE

  RETURN
END

;---------------------------------------------------------------------------
PRO SCIA_LV1_RD_MDS, dsd, mds, status=status, $
                     type_mds=type_mds, NotFree=NotFree, $
                     num_state=num_state, _EXTRA=EXTRA, $
                     NoMDS=NoMDS, pmd=pmd, polV=polV
  compile_opt idl2,logical_predicate,hidden

; definition of SCIAMACHY related constants
  NotSet = -1

; initialisation of some returned variables
  status = 0

  IF (~ KEYWORD_SET( NoMDS )) THEN $
     SetNoMDS = NotSet $
  ELSE BEGIN
     SetNoMDS = 1
     mds = NotSet
  ENDELSE

; check level of the Sciamachy product
  IF GET_SCIA_LEVEL() NE '1B' THEN BEGIN
     MESSAGE, 'FATAL: Input file is not a valid level 1B product', /INFO
     status = -1
     RETURN
  ENDIF

; check if value for type_mds is valid
  IF ARG_PRESENT( type_mds ) THEN BEGIN
     IF N_ELEMENTS( type_mds ) EQ 0 THEN BEGIN
        MESSAGE, 'No value for keyword: type_mds, no selection made!', /INFO
     ENDIF ELSE BEGIN
        type_mds = type_mds[0]
        SWITCH type_mds OF
           !nadc.sciaNadir:   BREAK
           !nadc.sciaLimb:    BREAK
           !nadc.sciaOccult:  BREAK
           !nadc.sciaMonitor: BREAK
           (-1): BREAK
           ELSE: BEGIN
              MESSAGE, 'Invalid value for type_mds', /INFO
              status = -1
              RETURN
           END
        ENDSWITCH
     ENDELSE
  ENDIF

; check required parameters and release previous allocated MDS data
  SWITCH N_PARAMS() OF
     2: BEGIN
        IF SIZE( dsd, /TNAME ) EQ 'STRUCT' THEN BEGIN
           IF TAG_NAMES( dsd, /STRUCT ) NE 'DSD_SCIA' THEN BEGIN
              MESSAGE, 'First parameter must be of type DSD_SCIA', /INFO
              status = -1
              RETURN
           ENDIF
        ENDIF ELSE BEGIN
           MESSAGE, 'First parameter must be of type DSD_SCIA', /INFO
           status = -1
           RETURN
        ENDELSE
        IF (~ KEYWORD_SET( NotFree )) THEN SCIA_LV1_FREE_MDS, mds
        BREAK
     END
     ELSE: BEGIN
        MESSAGE, ' Usage: SCIA_LV1_RD_MDS, dsd, mds, STATUS=status,' $
                 + ' TYPE_MDS=type_mds, CATEGORY=category, STATE_ID=state_id,'$
                 + ' PERIOD=period,  GEOLOCATION=geolocation, /NoSAA,' $
                 + ' INDX_STATE=indx_state, NUM_STATE=num_state,' $
                 + ' CHANNELS=channels, CLUSTERS=clusters,' $
                 + ' CALIBRATION=calibration, OrbitPhase=OrbitPhase,' $
                 + ' /NoMDS, /NotFree, PMD=pmd, POLV=polV,' , /INFO
        status = -1
        RETURN
     END
  ENDSWITCH

; release previouse allocated memory
  IF ARG_PRESENT( pmd ) THEN BEGIN
     GetPMD = 1
     IF (~ KEYWORD_SET( NotFree )) THEN SCIA_LV1_FREE_PMD, pmd
  ENDIF ELSE GetPMD = NotSet
  IF ARG_PRESENT( polV ) THEN BEGIN
     GetPOLV = 1
     IF (~ KEYWORD_SET( NotFree )) THEN SCIA_LV1_FREE_POLV, polV
  ENDIF ELSE GetPOLV = NotSet

; initialize number of state records, save record state for future use
  state = GET_LV1_MDS_STATE( dsd, type_mds=type_mds, status=status, $
                             count=num_state, _EXTRA=EXTRA )
  IF status NE 0 THEN RETURN
  IF num_state LE 0 THEN BEGIN
     MESSAGE, 'No state-records found for given selection criteria', /INFO
     status = -1
     RETURN
  ENDIF

; read data of the selected states
  FOR ns = 0, num_state-1 DO BEGIN
     IF SetNoMDS EQ NotSet THEN BEGIN
        SCIA_LV1_RD_ONE_MDS, state[ns], mds_one, status=status, _EXTRA=EXTRA
        IF status NE 0 THEN RETURN
        IF SIZE(mds_one, /TNAME) EQ 'STRUCT' THEN BEGIN
           IF SIZE(mds, /TNAME) NE 'STRUCT' THEN $
              mds = mds_one $
           ELSE $
              mds = [mds, mds_one]
        ENDIF
     ENDIF
     IF GetPMD NE NotSet THEN BEGIN
        SCIA_LV1_RD_ONE_PMD, state[ns], pmd_one, status=status
        IF status NE 0 THEN RETURN
        IF SIZE(pmd_one, /TNAME) EQ 'STRUCT' THEN BEGIN
           IF SIZE(pmd, /TNAME) NE 'STRUCT' THEN $
              pmd = pmd_one $
           ELSE $
              pmd = [pmd, pmd_one]
        ENDIF
     ENDIF
     IF GetPOLV NE NotSet THEN BEGIN
        SCIA_LV1_RD_ONE_POLV, state[ns], polV_one, status=status
        IF status NE 0 THEN RETURN
        IF SIZE(polV_one, /TNAME) EQ 'STRUCT' THEN BEGIN
           IF SIZE(polV, /TNAME) NE 'STRUCT' THEN $
              polV = polV_one $
           ELSE $
              polV = [polV, polV_one]
        ENDIF
     ENDIF
  ENDFOR

  RETURN
END
