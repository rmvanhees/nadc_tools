/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2008 - 2013 SRON (R.M.van.Hees@sron.nl)

   This is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License, version 2, as
   published by the Free Software Foundation.

   The software is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA  02111-1307, USA.

.IDENTifer   MERIS_WR_ASCII_TIE
.AUTHOR      R.M. van Hees
.KEYWORDS    PDS MERIS geolocation
.LANGUAGE    ANSI C
.PURPOSE     Dump  -- in ASCII Format -- MERIS tie point annotations (LADS)
.INPUT/OUTPUT
  call as    MERIS_WR_ASCII_TIE( param, num_dsr, tie );
     input:  
             struct param_record param : struct holding user-defined settings
	     unsigned int     num_dsr  : number of TIE structures
	     struct tie_meris *tie     : pointer to TIE structure(s)

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     1.0     16-Oct-2008   Created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*+++++ Local Headers +++++*/
#define _MERIS_COMMON
#include <nadc_meris.h>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void MERIS_WR_ASCII_TIE( struct param_record param, unsigned int num_dsr,
			 const struct tie_meris *tie )
{
     register unsigned short ii;
     register unsigned int   nd = 0;

     unsigned int  count[2];
     float         rbuff[MERIS_NUM_TIE_POINT];

     char date_str[UTC_STRING_LENGTH];

     FILE *outfl = CRE_ASCII_File( param.outfile, "tie" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of TIE record
 */
     nadc_write_header( outfl, 0, param.infile, 
			 "Tie point annotation Meris Product" );
     for ( nd = 0; nd < num_dsr; nd++ ) {
	  register unsigned int nr = 0;

          (void) MJD_2_ASCII( tie[nd].mjd.days, tie[nd].mjd.secnd,
                              tie[nd].mjd.musec, date_str );
          nadc_write_text( outfl, ++nr, "Date", date_str );
          nadc_write_uchar( outfl, ++nr, "MDS DSR attached",
			    tie[nd].flag_mds );
	  count[0] = MERIS_NUM_TIE_POINT;
	  for ( ii = 0; ii < MERIS_NUM_TIE_POINT; ii++ )
	       rbuff[ii] = tie[nd].coord[ii].lat / 1e6;
	  nadc_write_arr_float( outfl, ++nr, "Latitude", 1, count, 8, rbuff );
	  for ( ii = 0; ii < MERIS_NUM_TIE_POINT; ii++ )
	       rbuff[ii] = tie[nd].coord[ii].lon / 1e6;
	  nadc_write_arr_float( outfl, nr, "Longitude", 1, count, 8, rbuff );
	  nadc_write_arr_int( outfl, ++nr, "DEM altitude", 
			      1, count, tie[nd].dem_altitude );
	  nadc_write_arr_uint( outfl, ++nr, "DEM roughness", 
			       1, count, tie[nd].dem_roughness );
	  nadc_write_arr_int( outfl, ++nr, "DEM latitude corrections", 
			      1, count, tie[nd].dem_lat_corr );
	  nadc_write_arr_int( outfl, ++nr, "DEM longitude corrections", 
			      1, count, tie[nd].dem_lon_corr );
	  nadc_write_arr_uint( outfl, ++nr, "Sun zenith angles", 
			       1, count, tie[nd].sun_zen_angle );
	  nadc_write_arr_int( outfl, ++nr, "Sun azimuth angles", 
			      1, count, tie[nd].sun_azi_angle );
	  nadc_write_arr_uint( outfl, ++nr, "Viewing zenith angles", 
			       1, count, tie[nd].view_zen_angle );
	  nadc_write_arr_int( outfl, ++nr, "Viewing azimuth angles", 
			      1, count, tie[nd].view_azi_angle );
	  nadc_write_arr_short( outfl, ++nr, "Zonal winds", 
				1, count, tie[nd].zonal_wind );
	  nadc_write_arr_short( outfl, ++nr, "Meridional winds", 
				1, count, tie[nd].merid_wind );
	  nadc_write_arr_ushort( outfl, ++nr, "Atmospheric pressures", 
				 1, count, tie[nd].atm_press );
	  nadc_write_arr_ushort( outfl, ++nr, "Total ozone", 
				 1, count, tie[nd].ozone );
	  nadc_write_arr_ushort( outfl, ++nr, "Relative humidity", 
				 1, count, tie[nd].humidity );
     }
     (void) fclose( outfl );
}

