PRO GLR1_GOME__DEFINE
compile_opt idl2,hidden

NUM_COORDS = 5

struct = { GLR1_GOME ,$
           sun_glint        : 0b  ,$
           subsetcounter    : 0b  ,$
           utc_date         : 0ul ,$
           utc_time         : 0ul ,$
           sat_geo_height   : 0.0 ,$
           earth_radius     : 0.0 ,$
           north_sun_zen    : fltarr(3) ,$
           north_sun_azim   : fltarr(3) ,$
           north_sight_zen  : fltarr(3) ,$
           north_sight_azim : fltarr(3) ,$
           sat_sun_zen      : fltarr(3) ,$
           sat_sun_azim     : fltarr(3) ,$
           sat_sight_zen    : fltarr(3) ,$
           sat_sight_azim   : fltarr(3) ,$
           lon              : fltarr(NUM_COORDS) ,$
           lat              : fltarr(NUM_COORDS) $
         }

END
