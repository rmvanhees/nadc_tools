/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2000 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV1C_WR_ASCII_CALOPT
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1c data
.LANGUAGE    ANSI C
.PURPOSE     Dump the calibration options GADS to SciaL1C
.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION     1.1     07-Dec-2005   write lat/lon values as doubles, RvH
             1.0     25-Jul-2002   Created by R. M. van Hees
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
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1C_WR_ASCII_CALOPT( const struct param_record param,
				const struct cal_options *calopt )
{
     register unsigned int nr = 0;

     char date_str[UTC_STRING_LENGTH];

     unsigned int   count[1];

     const char prognm[] = "SCIA_LV1C_WR_ASCII_CALOPT";

     FILE *outfl = CRE_ASCII_File( param.outfile, "calopt" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of CAL_OPTIONS record
 */
     nadc_write_header( outfl, nr, param.infile, 
			 "Calibration Options GADS to SciaL1C" );
     nadc_write_text( outfl,++nr, "l1b_product_name", calopt->l1b_prod_name );
     if ( calopt->geo_filter != SCHAR_ZERO ) {
	  nadc_write_double( outfl, ++nr, "start_lat", 6, 
			     calopt->start_lat/1e6 );
	  nadc_write_double( outfl, ++nr, "start_lon", 6, 
			     calopt->start_lon/1e6 );
	  nadc_write_double( outfl, ++nr, "end_lat", 6, calopt->end_lat/1e6 );
	  nadc_write_double( outfl, ++nr, "end_lon", 6, calopt->end_lon/1e6 );
     } else
	  nr += 4;
     if ( calopt->time_filter != SCHAR_ZERO ) {
	  (void) MJD_2_ASCII( calopt->start_time.days, 
			      calopt->start_time.secnd,
                              calopt->start_time.musec, date_str );
          nadc_write_text( outfl, ++nr, "start_time", date_str );
	  (void) MJD_2_ASCII( calopt->stop_time.days, 
			      calopt->stop_time.secnd,
                              calopt->stop_time.musec, date_str );
          nadc_write_text( outfl, ++nr, "stop_time", date_str );

     } else
	  nr += 2;
     if ( calopt->category_filter != SCHAR_ZERO ) {
	  count[0] = 5;
          nadc_write_arr_ushort( outfl, ++nr, "category", 
                                  1, count, calopt->category );
     } else
	  nr += 1;
     if ( calopt->nadir_mds != SCHAR_ZERO )
	  nadc_write_text( outfl, ++nr, "nadir_mds_flag", "TRUE" );
     else
	  nadc_write_text( outfl, ++nr, "nadir_mds_flag", "FALSE" );
     if ( calopt->limb_mds != SCHAR_ZERO )
	  nadc_write_text( outfl, ++nr, "limb_mds_flag", "TRUE" );
     else
	  nadc_write_text( outfl, ++nr, "limb_mds_flag", "FALSE" );
     if ( calopt->occ_mds != SCHAR_ZERO )
	  nadc_write_text( outfl, ++nr, "occ_mds_flag", "TRUE" );
     else
	  nadc_write_text( outfl, ++nr, "occ_mds_flag", "FALSE" );
     if ( calopt->moni_mds != SCHAR_ZERO )
	  nadc_write_text( outfl, ++nr, "moni_mds_flag", "TRUE" );
     else
	  nadc_write_text( outfl, ++nr, "moni_mds_flag", "FALSE" );

     if ( calopt->pmd_mds != SCHAR_ZERO )
	  nadc_write_text( outfl, ++nr, "pmd_mds_flag", "TRUE" );
     else
	  nadc_write_text( outfl, ++nr, "pmd_mds_flag", "FALSE" );
     if ( calopt->frac_pol_mds != SCHAR_ZERO )
	  nadc_write_text( outfl, ++nr, "frac_pol_mds_flag", "TRUE" );
     else
	  nadc_write_text( outfl, ++nr, "frac_pol_mds_flag", "FALSE" );
     if ( calopt->slit_function != SCHAR_ZERO )
	  nadc_write_text( outfl, ++nr, "slit_function_gads_flag", "TRUE" );
     else
	  nadc_write_text( outfl, ++nr, "slit_function_gads_flag", "FALSE" );
     if ( calopt->sun_mean_ref != SCHAR_ZERO )
	  nadc_write_text( outfl, ++nr, "sun_mean_ref_gads_flag", "TRUE" );
     else
	  nadc_write_text( outfl, ++nr, "sun_mean_ref_gads_flag", "FALSE" );
     if ( calopt->leakage_current != SCHAR_ZERO )
	  nadc_write_text( outfl, ++nr, "leakage_current_gads_flag", "TRUE" );
     else
	  nadc_write_text( outfl, ++nr, "leakage_current_gads_flag", "FALSE");
     if ( calopt->spectral_cal != SCHAR_ZERO )
	  nadc_write_text( outfl, ++nr, "spectral_cal_gads_flag", "TRUE" );
     else
	  nadc_write_text( outfl, ++nr, "spectral_cal_gads_flag", "FALSE" );
     if ( calopt->pol_sens != SCHAR_ZERO )
	  nadc_write_text( outfl, ++nr, "pol_sens_gads_flag", "TRUE" );
     else
	  nadc_write_text( outfl, ++nr, "pol_sens_gads_flag", "FALSE" );
     if ( calopt->rad_sens != SCHAR_ZERO )
	  nadc_write_text( outfl, ++nr, "rad_sens_gads_flag", "TRUE" );
     else
	  nadc_write_text( outfl, ++nr, "rad_sens_gads_flag", "FALSE" );
     if ( calopt->ppg_etalon != SCHAR_ZERO )
	  nadc_write_text( outfl, ++nr, "ppg_etalon_gads_flag", "TRUE" );
     else
	  nadc_write_text( outfl, ++nr, "ppg_etalon_gads_flag", "FALSE" );
     nadc_write_ushort( outfl,++nr, "num_nadir_clusters", calopt->num_nadir );
     nadc_write_ushort( outfl,++nr, "num_limb_clusters", calopt->num_limb );
     nadc_write_ushort( outfl,++nr, "num_occ_clusters", calopt->num_occ );
     nadc_write_ushort( outfl,++nr, "num_moni_clusters", calopt->num_moni );
     count[0] = MAX_CLUSTER;
     nadc_write_arr_schar( outfl, ++nr, "nadir_cluster_flag", 
			    1, count, calopt->nadir_cluster );
     nadc_write_arr_schar( outfl, ++nr, "limb_cluster_flag", 
			    1, count, calopt->limb_cluster );
     nadc_write_arr_schar( outfl, ++nr, "occ_cluster_flag", 
			    1, count, calopt->occ_cluster );
     nadc_write_arr_schar( outfl, ++nr, "moni_cluster_flag", 
			    1, count, calopt->moni_cluster );
     if ( calopt->mem_effect_cal != SCHAR_ZERO )
	  nadc_write_text( outfl, ++nr, "mem_effect_cal_flag", "TRUE" );
     else
	  nadc_write_text( outfl, ++nr, "mem_effect_cal_flag", "FALSE" );
     if ( calopt->leakage_cal != SCHAR_ZERO )
	  nadc_write_text( outfl, ++nr, "leakage_cal_flag", "TRUE" );
     else
	  nadc_write_text( outfl, ++nr, "leakage_cal_flag", "FALSE" );
     if ( calopt->straylight_cal != SCHAR_ZERO )
	  nadc_write_text( outfl, ++nr, "straylight_cal_flag", "TRUE" );
     else
	  nadc_write_text( outfl, ++nr, "straylight_cal_flag", "FALSE" );
     if ( calopt->ppg_cal != SCHAR_ZERO )
	  nadc_write_text( outfl, ++nr, "ppg_cal_flag", "TRUE" );
     else
	  nadc_write_text( outfl, ++nr, "ppg_cal_flag", "FALSE" );
     if ( calopt->etalon_cal != SCHAR_ZERO )
	  nadc_write_text( outfl, ++nr, "etalon_cal_flag", "TRUE" );
     else
	  nadc_write_text( outfl, ++nr, "etalon_cal_flag", "FALSE" );
     if ( calopt->wave_cal != SCHAR_ZERO )
	  nadc_write_text( outfl, ++nr, "spectal_cal_flag", "TRUE" );
     else
	  nadc_write_text( outfl, ++nr, "spectral_cal_flag", "FALSE" );
     if ( calopt->polarisation_cal != SCHAR_ZERO )
	  nadc_write_text( outfl, ++nr, "polarisation_cal_flag", "TRUE" );
     else
	  nadc_write_text( outfl, ++nr, "polarisation_cal_flag", "FALSE" );
     if ( calopt->radiance_cal != SCHAR_ZERO )
	  nadc_write_text( outfl, ++nr, "radiance_cal_flag", "TRUE" );
     else
	  nadc_write_text( outfl, ++nr, "radiance_cal_flag", "FALSE" );

     (void) fclose( outfl );
}
