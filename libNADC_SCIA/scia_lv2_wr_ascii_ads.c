/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2001 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV2_WR_ASCII_ADS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 2
.LANGUAGE    ANSI C
.PURPOSE     Dump Annotation Data Sets in ASCII
.RETURNS     Nothing
.COMMENTS    contains SCIA_LV2_WR_ASCII_SQADS, SCIA_LV2_WR_ASCII_STATE, 
                SCIA_LV2_WR_ASCII_GEO
.ENVIRONment None
.VERSION     1.1     07-Dec-2005   write lat/lon values as doubles, RvH
             1.0     13-Sep-2001   Created by R. M. van Hees
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
#define _SCIA_LEVEL_2
#include <nadc_scia.h>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV2_WR_ASCII_SQADS
.PURPOSE    dump -- in ASCII Format -- the SQADS
.INPUT/OUTPUT
  call as   SCIA_LV2_WR_ASCII_SQADS( param, num_dsr, sqads );
     input:
            struct param_record param : struct holding user-defined settings
	    unsigned int num_dsr      : number of data sets
	    struct sqads2_scia *sqads : pointer to SQADS records
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV2_WR_ASCII_SQADS( struct param_record param, unsigned int num_dsr,
			      const struct sqads2_scia *sqads )
{
     register unsigned int nd, nr;

     char date_str[UTC_STRING_LENGTH];
     unsigned int  count[2];

     const char prognm[] = "SCIA_LV2_WR_ASCII_SQADS";

     FILE *outfl = CRE_ASCII_File( param.outfile, "sqads" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of SQADS record
 */
     nadc_write_header( outfl, 0, param.infile,  
			 "Summary of Quality Flags per State" );
     for ( nd = 0; nd < num_dsr; nd++ ) {
	  nr = 1;
	  (void) MJD_2_ASCII( sqads[nd].mjd.days, sqads[nd].mjd.secnd,
			      sqads[nd].mjd.musec, date_str );
	  nadc_write_text( outfl, nr++, "Date", date_str );
	  nadc_write_uchar( outfl, nr++, "MDS DSR attached", 
			     sqads[nd].flag_mds );
	  count[0] = NL2_SQADS_PQF_FLAGS;
	  nadc_write_arr_uchar( outfl, nr++, "summary of quality (PQF)", 
				 1, count, sqads[nd].flag_pqf );
     }
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV2_WR_ASCII_STATE
.PURPOSE    dump -- in ASCII Format -- the STATE records
.INPUT/OUTPUT
  call as   SCIA_LV2_WR_ASCII_STATE( param, num_dsr, state );
     input:
            struct param_record param : struct holding user-defined settings
	    unsigned int num_dsr      : number of data sets
	    struct state2_scia *state : pointer to STATE records
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV2_WR_ASCII_STATE( struct param_record param, unsigned int num_dsr,
			      const struct state2_scia *state )
{
     register unsigned int nd, nr;

     char date_str[UTC_STRING_LENGTH];

     const char prognm[] = "SCIA_LV2_WR_ASCII_STATE";

     FILE *outfl = CRE_ASCII_File( param.outfile, "state" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of STATE record
 */
     nadc_write_header( outfl, 0, param.infile, "States of the Product" );
     for ( nd = 0; nd < num_dsr; nd++ ) {
	  nr = 1;
	  (void) MJD_2_ASCII( state[nd].mjd.days, state[nd].mjd.secnd,
			      state[nd].mjd.musec, date_str );
	  nadc_write_text( outfl, nr, "MJD (Date Time)", date_str );
	  nadc_write_double( outfl, nr++, "MJD (Julian Day)", 16, 
			     (double) state[nd].mjd.days + 
			     ((state[nd].mjd.secnd + state[nd].mjd.musec / 1e6)
			      / (24. * 60 * 60)) );
	  nadc_write_uchar( outfl, nr++, "MDS DSR attached", 
			     state[nd].flag_mds );
	  nadc_write_ushort( outfl, nr++, "State ID", 
			      state[nd].state_id );
	  nadc_write_ushort( outfl, nr++, "Duration of scan phase", 
			      state[nd].duration );
	  nadc_write_ushort( outfl, nr++, "Longest integration time", 
			      state[nd].longest_intg_time );
	  nadc_write_ushort( outfl, nr++, "Shortest integration time", 
			      state[nd].shortest_intg_time );
	  nadc_write_ushort( outfl, nr++, "Number of Observations", 
			      state[nd].num_obs_state );
     }
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV2_WR_ASCII_GEO
.PURPOSE    dump -- in ASCII Format -- the records
.INPUT/OUTPUT
  call as   SCIA_LV2_WR_ASCII_GEO( param, num_dsr, geo );
     input:
            struct param_record param : struct holding user-defined settings
	    unsigned int num_dsr      : number of data sets
	    struct geo_scia *geo      : pointer to Geolocation records
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV2_WR_ASCII_GEO( struct param_record param, unsigned int num_dsr,
			    const struct geo_scia *geo )
{
     register unsigned int nd, ni, nr, ny;

     char  date_str[UTC_STRING_LENGTH];
     double rbuff[2 * 4];
     unsigned int  count[2];

     const char prognm[] = "SCIA_LV2_WR_ASCII_GEO";

     FILE *outfl = CRE_ASCII_File( param.outfile, "geo" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of GEO record
 */
     nadc_write_header( outfl, 0, param.infile,  
			 "Geolocation Data set(s)" );
     for ( nd = 0; nd < num_dsr; nd++ ) {
	  nr = 0;
	  (void) MJD_2_ASCII( geo[nd].mjd.days, geo[nd].mjd.secnd,
			      geo[nd].mjd.musec, date_str );
	  nadc_write_text( outfl, ++nr, "Date", date_str );
	  nadc_write_uchar( outfl, ++nr, "MDS DSR attached", 
			     geo[nd].flag_mds );
	  nadc_write_ushort( outfl, ++nr, "Integration time",
			      geo[nd].intg_time );
	  count[0] = 3;
	  nadc_write_arr_float( outfl, ++nr, "Solar zenith angles",
				 1, count, 5, geo[nd].sun_zen_ang );
	  nadc_write_arr_float( outfl, ++nr, "Line-of-Sight zenith angles",
				 1, count, 5, geo[nd].los_zen_ang );
	  nadc_write_arr_float( outfl, ++nr, "Relative Azimuth angles",
				 1, count, 5, geo[nd].rel_azi_ang );
	  nadc_write_float( outfl, ++nr, "Satellite geodetic height",
			     5, geo[nd].sat_h );
	  nadc_write_float( outfl, ++nr, "Earth radius",
			     5, geo[nd].earth_rad );
	  count[0] = 2;
	  count[1] = 4;
	  for ( ni = ny = 0; ny < count[1]; ny++ ) {
	       rbuff[ni++] = geo[nd].corner[ny].lat / 1e6;
	       rbuff[ni++] = geo[nd].corner[ny].lon / 1e6;
	  }
	  nadc_write_arr_double( outfl, ++nr, "Corner coordinates",
				 2, count, 6, rbuff );
	  
	  rbuff[0] = geo[nd].center.lat / 1e6;
	  rbuff[1] = geo[nd].center.lon / 1e6;
	  nadc_write_arr_double( outfl, ++nr, "Center coordinates",
				 1, count, 6, rbuff );
     }
     (void) fclose( outfl );
}
