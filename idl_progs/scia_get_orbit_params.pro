; unit test rout
pro test_scia_get_orbit_params

data_dir = '/SCIA/LV0_07/P/'
flname = 'SCI_NL__0PPLRA20020823_082911_000060072008_00451_02509_6605.N1'

SCIA_HL_OPEN, data_dir + flname, dsd, status=status
IF status NE 0 THEN NADC_ERR_TRACE

; collect info about SCIAMACHY source packets
flname = STRMID(flname, STRPOS( flname, '/', /REVERSE_SEARCH )+1)
SCIA_LV0_RD_MDS_INFO, flname, dsd, info, status=status
IF status NE 0 THEN NADC_ERR_TRACE

; read Auxiliary source packets
PRINT, 'SCIA_LV0_RD_MDS_AUX'
SCIA_LV0_RD_AUX, info, mds_aux, status=status
IF status NE 0 THEN NADC_ERR_TRACE

; process PMD source packets
PRINT, 'SCIA_LV0_RD_MDS_PMD'
SCIA_LV0_RD_PMD, info, mds_pmd, status=status
IF status NE 0 THEN NADC_ERR_TRACE

; read Detector source packets
PRINT, 'SCIA_LV0_RD_MDS_DET'
SCIA_LV0_RD_DET, info, mds_det, status=status;, state_id=6
IF status NE 0 THEN NADC_ERR_TRACE

; close file
stat = SCIA_FCLOSE()

aux_idx = 850
idx = where(mds_aux.data_hdr.state_id eq 7)
if idx[0] eq -1 then message, "oh noes, no state 1 found!"
;aux_idx = idx[n_elements(idx)*3/4]
aux_idx = idx[0]

; idl tests lowest bit by default, so that's convinient ;)
if mds_aux[aux_idx].data_hdr.rdv then begin
    aziOff = -108.18143;
    eleOff = -19.2340;
endif else begin
    aziOff = -18.18943;
    eleOff = -109.2425;
endelse

el = mds_aux[aux_idx].data_src.bcp.ele_encode_cntr*360./640000 + eleoff
az = mds_aux[aux_idx].data_src.bcp.azi_encode_cntr*360./640000 + azioff

print, el
print, az

isps = mds_aux.isp
mjds = isps.days + (isps.secnd + isps.musec/1000000.d)/3600.d/24.d
; only MJD's in local aux packet, use BCP count
local_mjds = mjds[aux_idx] + mds_aux[aux_idx].data_src.bcp.bcps/16.d/3600.d/24.d
help,mjds
scia_get_orbit_params, local_mjds, esms=el, lats=lats, lons=lons, sunels=sunels, $
 sunazs=sunazs
help,lats
print,lats
help,lons
print,lons
help,sunels
print,sunels
help,sunazs
print,sunazs
device,decomposed=0
window,0,xsize=1700,ysize=1100
map_set,/cont,limit=[20,0,40,40] ; ,limit=[-70,330,-60,360] ;limit=[-70,340,-60,350]
oplot, lons,lats, ps=1
stop
end

;-------------------------------------------------------------------------------
; +
;
; PURPOSE:
;       find ENVISAT geolocation and solar angles for given julian dates
;
; INPUT:
;       mjds: array of modified julian dates (starting 2000, doubles)
;
; KEYWORDS
;       esms: esm angles (input, unit=degrees, floats, dimensions of 'mjds' 
;        array)
;       lats: latitudes (output, floats)
;       lons: longitudes (output, floats)
;       sunels: sun elevation angles (output, floats)
;       sunazs: solar azimuth angles (output, floats)
;
; EXAMPLE:
;       scia_get_orbit_params, [1900.d], lats=lats, lons=lons, sunels=sunels, $
;                              sunazs=sunazs
;       scia_get_orbit_params, [1900.d], esms=[-10.], lats=lats, lons=lons, $
;                              sunels=sunels, sunazs=sunazs
; -
;-------------------------------------------------------------------------------
pro scia_get_orbit_params, mjds, esms=esms, lats=lats, lons=lons, $
                           sunels=sunels, sunazs=sunazs
  compile_opt idl2,hidden
                                ;
                                ; initialise variables
                                ;
  if size(mjds, /tname) ne 'DOUBLE' then $
     message, 'error: input MJD not in double precision!'
  if n_elements(esms) ne 0 then begin
     if n_elements(mjds) ne n_elements(esms) then $
        message, 'error: esms should have same dimensions as mjds!'
  endif else esms = fltarr(n_elements(mjds)) + 360.
  n_points = n_elements(mjds)
  lats     = fltarr(n_points)
  lons     = fltarr(n_points)
  sunels   = fltarr(n_points)
  sunazs   = fltarr(n_points)
                                ;
                                ; sort julian dates 
                                ; (for speed optimisation reasons)
                                ;
  sidx     = sort(mjds)
  mjds_srt = mjds[sidx]
  esms_srt = float(esms[sidx])
                                ;
                                ; call the binary
                                ;
  ret = call_external(lib_name('libnadc_idl'), '_SCIA_GET_ORBIT_PARAMS', $
                      mjds_srt, esms_srt, n_points, lats, lons, sunels, $ 
                      sunazs, /CDECL)

;    if ret lt n_points then message, "_SCIA_GET_ORBIT_PARAMS failed!"
  if ret lt n_points then begin
     print,"  ;_SCIA_GET_ORBIT_PARAMS failed!"
     NADC_ERR_CLEAR
  endif
                                ;
                                ; sort arrays back to input order
                                ;
  lats[sidx]   = lats
  lons[sidx]   = lons
  sunels[sidx] = sunels
  sunazs[sidx] = sunazs
  return
end
