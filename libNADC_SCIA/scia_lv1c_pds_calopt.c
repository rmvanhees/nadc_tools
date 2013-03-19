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

.IDENTifer   SCIA_LV1C_PDS_CALOPT
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1c data
.LANGUAGE    ANSI C
.PURPOSE     read/write the calibration options GADS to SciaL1C
.COMMENTS    contains SCIA_LV1C_RD_CALOPT, SCIA_LV1C_UPDATE_CALOPT 
                      and SCIA_LV1C_WR_CALOPT
.ENVIRONment None
.VERSION      4.0   11-Oct-2005 use direct write, add usage of SCIA_LV1_ADD_DSD
              3.0   06-Oct-2005 added routine SCIA_LV1C_UPDATE_CALOPT,
                                      to update Calibration Options GADS
              2.0   18-Apr-2005 added routine SCIA_LV1C_WR_CALOPT
                                      to write CALOPT-struct to file
              1.1   12-Dec-2002 bugfix: did not read element moni_mds, RvH
              1.1   11-Dec-2002 rearranged elements of structure "cal_options"
              1.0   24-Jul-2002 created by R. M. van Hees
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

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#ifdef _SWAP_TO_LITTLE_ENDIAN
#include <swap_bytes.h>

static
void Sun2Intel_CALOPT( struct cal_options *calopt )
{
     calopt->start_lat = byte_swap_32( calopt->start_lat );
     calopt->start_lon = byte_swap_32( calopt->start_lon );
     calopt->end_lat = byte_swap_32( calopt->end_lat );
     calopt->end_lon = byte_swap_32( calopt->end_lon );
     calopt->start_time.days = byte_swap_32( calopt->start_time.days );
     calopt->start_time.secnd = byte_swap_u32( calopt->start_time.secnd );
     calopt->start_time.musec = byte_swap_u32( calopt->start_time.musec );
     calopt->stop_time.days = byte_swap_32( calopt->stop_time.days );
     calopt->stop_time.secnd = byte_swap_u32( calopt->stop_time.secnd );
     calopt->stop_time.musec = byte_swap_u32( calopt->stop_time.musec );
     calopt->category[0] = byte_swap_u16( calopt->category[0] );
     calopt->category[1] = byte_swap_u16( calopt->category[1] );
     calopt->category[2] = byte_swap_u16( calopt->category[2] );
     calopt->category[3] = byte_swap_u16( calopt->category[3] );
     calopt->category[4] = byte_swap_u16( calopt->category[4] );
     calopt->num_nadir = byte_swap_u16( calopt->num_nadir );
     calopt->num_limb = byte_swap_u16( calopt->num_limb );
     calopt->num_occ = byte_swap_u16( calopt->num_occ );
     calopt->num_moni = byte_swap_u16( calopt->num_moni );
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1C_RD_CALOPT
.PURPOSE     read the calibration options GADS from SciaL1C file
.INPUT/OUTPUT
  call as   nr_dsr = SCIA_LV1C_RD_CALOPT( fd, num_dsd, dsd, &calopt );
     input:  
            FILE   *fd             :  (open) stream pointer
	    unsigned int num_dsd   :  number of DSDs
	    struct dsd_envi dsd    :  structure for the DSDs
    output:  
            struct cal_options *calopt :  structure for L1c calibration options

.RETURNS     number of data set records read (unsigned int)
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV1C_RD_CALOPT( FILE *fd, unsigned int num_dsd, 
				  const struct dsd_envi *dsd,
				  struct cal_options *calopt )
{
     const char prognm[] = "SCIA_LV1C_RD_CALOPT";

     char         *calopt_pntr, *calopt_char = NULL;
     size_t       dsr_size;
     unsigned int indx_dsd;

     const char dsd_name[] = "CAL_OPTIONS";
/*
 * get index to data set descriptor
 */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL ){
	  NADC_ERROR( prognm, NADC_ERR_PDS_RD, dsd_name );
	  return 0u;
     }
     if ( dsd[indx_dsd].num_dsr == 0 ) return 0u;
/*
 * allocate memory to temporary store data for output structure
 */
     dsr_size = (size_t) dsd[indx_dsd].dsr_size;
     if ( (calopt_char = (char *) malloc( dsr_size )) == NULL )  { 
	  NADC_ERROR( prognm, NADC_ERR_ALLOC, "calopt_char" );
	  return 0u;
     }
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
     if ( fread( calopt_char, dsr_size, 1, fd ) != 1 ) {
	  free( calopt_char );
	  NADC_ERROR( prognm, NADC_ERR_PDS_RD, "" );
	  return 0u;
     }
     calopt_pntr = calopt_char;
/*
 * read data buffer to CALOPT structure
 */
     (void) memcpy( calopt->l1b_prod_name, calopt_pntr, 62 );
     calopt->l1b_prod_name[62] = '\0';
     calopt_pntr += 62;
     (void) memcpy( &calopt->geo_filter, calopt_pntr, ENVI_CHAR );
     calopt_pntr += ENVI_CHAR;
     (void) memcpy( &calopt->start_lat, calopt_pntr, ENVI_INT );
     calopt_pntr += ENVI_INT;
     (void) memcpy( &calopt->start_lon, calopt_pntr, ENVI_INT );
     calopt_pntr += ENVI_INT;
     (void) memcpy( &calopt->end_lat, calopt_pntr, ENVI_INT );
     calopt_pntr += ENVI_INT;
     (void) memcpy( &calopt->end_lon, calopt_pntr, ENVI_INT );
     calopt_pntr += ENVI_INT;

     (void) memcpy( &calopt->time_filter, calopt_pntr, ENVI_CHAR );
     calopt_pntr += ENVI_CHAR;
     (void) memcpy( &calopt->start_time.days, calopt_pntr, ENVI_INT );
     calopt_pntr += ENVI_INT;
     (void) memcpy( &calopt->start_time.secnd, calopt_pntr, ENVI_UINT );
     calopt_pntr += ENVI_UINT;
     (void) memcpy( &calopt->start_time.musec, calopt_pntr, ENVI_UINT );
     calopt_pntr += ENVI_UINT;
     (void) memcpy( &calopt->stop_time.days, calopt_pntr, ENVI_INT );
     calopt_pntr += ENVI_INT;
     (void) memcpy( &calopt->stop_time.secnd, calopt_pntr, ENVI_UINT );
     calopt_pntr += ENVI_UINT;
     (void) memcpy( &calopt->stop_time.musec, calopt_pntr, ENVI_UINT );
     calopt_pntr += ENVI_UINT;

     (void) memcpy( &calopt->category_filter, calopt_pntr, ENVI_CHAR );
     calopt_pntr += ENVI_CHAR;
     (void) memcpy( calopt->category, calopt_pntr, 5 * ENVI_USHRT );
     calopt_pntr += 5 * ENVI_USHRT;

     (void) memcpy( &calopt->nadir_mds, calopt_pntr, ENVI_CHAR );
     calopt_pntr += ENVI_CHAR;
     (void) memcpy( &calopt->limb_mds, calopt_pntr, ENVI_CHAR );
     calopt_pntr += ENVI_CHAR;
     (void) memcpy( &calopt->occ_mds, calopt_pntr, ENVI_CHAR );
     calopt_pntr += ENVI_CHAR;
     (void) memcpy( &calopt->moni_mds, calopt_pntr, ENVI_CHAR );
     calopt_pntr += ENVI_CHAR;
     (void) memcpy( &calopt->pmd_mds, calopt_pntr, ENVI_CHAR );
     calopt_pntr += ENVI_CHAR;
     (void) memcpy( &calopt->frac_pol_mds, calopt_pntr, ENVI_CHAR );
     calopt_pntr += ENVI_CHAR;

     (void) memcpy( &calopt->slit_function, calopt_pntr, ENVI_CHAR );
     calopt_pntr += ENVI_CHAR;
     (void) memcpy( &calopt->sun_mean_ref, calopt_pntr, ENVI_CHAR );
     calopt_pntr += ENVI_CHAR;
     (void) memcpy( &calopt->leakage_current, calopt_pntr, ENVI_CHAR );
     calopt_pntr += ENVI_CHAR;
     (void) memcpy( &calopt->spectral_cal, calopt_pntr, ENVI_CHAR );
     calopt_pntr += ENVI_CHAR;
     (void) memcpy( &calopt->pol_sens, calopt_pntr, ENVI_CHAR );
     calopt_pntr += ENVI_CHAR;
     (void) memcpy( &calopt->rad_sens, calopt_pntr, ENVI_CHAR );
     calopt_pntr += ENVI_CHAR;
     (void) memcpy( &calopt->ppg_etalon, calopt_pntr, ENVI_CHAR );
     calopt_pntr += ENVI_CHAR;

     (void) memcpy( &calopt->num_nadir, calopt_pntr, ENVI_USHRT );
     calopt_pntr += ENVI_USHRT;
     (void) memcpy( &calopt->num_limb, calopt_pntr, ENVI_USHRT );
     calopt_pntr += ENVI_USHRT;
     (void) memcpy( &calopt->num_occ, calopt_pntr, ENVI_USHRT );
     calopt_pntr += ENVI_USHRT;
     (void) memcpy( &calopt->num_moni, calopt_pntr, ENVI_USHRT );
     calopt_pntr += ENVI_USHRT;
     (void) memcpy( calopt->nadir_cluster, calopt_pntr, MAX_CLUSTER );
     calopt_pntr += MAX_CLUSTER;
     (void) memcpy( calopt->limb_cluster, calopt_pntr, MAX_CLUSTER );
     calopt_pntr += MAX_CLUSTER;
     (void) memcpy( calopt->occ_cluster, calopt_pntr, MAX_CLUSTER );
     calopt_pntr += MAX_CLUSTER;
     (void) memcpy( calopt->moni_cluster, calopt_pntr, MAX_CLUSTER );
     calopt_pntr += MAX_CLUSTER;

     (void) memcpy( &calopt->mem_effect_cal, calopt_pntr, ENVI_CHAR );
     calopt_pntr += ENVI_CHAR;
     (void) memcpy( &calopt->leakage_cal, calopt_pntr, ENVI_CHAR );
     calopt_pntr += ENVI_CHAR;
     (void) memcpy( &calopt->straylight_cal, calopt_pntr, ENVI_CHAR );
     calopt_pntr += ENVI_CHAR;
     (void) memcpy( &calopt->ppg_cal, calopt_pntr, ENVI_CHAR );
     calopt_pntr += ENVI_CHAR;
     (void) memcpy( &calopt->etalon_cal, calopt_pntr, ENVI_CHAR );
     calopt_pntr += ENVI_CHAR;
     (void) memcpy( &calopt->wave_cal, calopt_pntr, ENVI_CHAR );
     calopt_pntr += ENVI_CHAR;
     (void) memcpy( &calopt->polarisation_cal, calopt_pntr, ENVI_CHAR );
     calopt_pntr += ENVI_CHAR;
     (void) memcpy( &calopt->radiance_cal, calopt_pntr, ENVI_CHAR );
     calopt_pntr += ENVI_CHAR;
/*
 * check if we read the whole DSR
 */
     if ( (size_t)(calopt_pntr - calopt_char) != dsr_size ) {
	  free( calopt_char );
	  NADC_ERROR( prognm, NADC_ERR_PDS_SIZE, dsd_name );
	  return 0u;
     }
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_CALOPT( calopt );
#endif
/*
 * deallocate memory
 */
     calopt_pntr = NULL;
     free( calopt_char );
/*
 * set return values
 */
     return 1u;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1C_UPDATE_CALOPT
.PURPOSE    update Calibration Options GADS
.INPUT/OUTPUT
  call as   nr_dsr = SCIA_LV1C_UPDATE_CALOPT( do_init, param, &calopt );
     input:  
            int do_init                :  initialise calopt-record
            struct param_record param  :  struct holding user-defined settings
    output:  
            struct cal_options *calopt :  structure for L1c calibration options

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void SCIA_LV1C_UPDATE_CALOPT( int is_scia_lv1c, 
			      const struct param_record param,
			      struct cal_options *calopt )
{
     register unsigned int nr;

     if ( is_scia_lv1c ) {                        /* update calopt-structure */
	  if ( param.flag_geoloc == PARAM_SET ) {
	       calopt->geo_filter = (signed char) -1;
	       calopt->start_lat = (int) (1e6 * param.geo_lat[0]);
	       calopt->start_lon = (int) (1e6 * param.geo_lon[0]);
	       calopt->end_lat = (int) (1e6 * param.geo_lat[1]);
	       calopt->end_lon = (int) (1e6 * param.geo_lon[1]);
	  }
	  if ( param.flag_period == PARAM_SET ) {
	       int          mjd2000;
	       unsigned int secnd, mu_sec;

	       calopt->time_filter = (signed char) -1;
	       ASCII_2_MJD( param.bgn_date, &mjd2000, &secnd, &mu_sec );
	       calopt->start_time.days = mjd2000;
	       calopt->start_time.secnd = secnd;
	       calopt->start_time.musec = mu_sec;
	       ASCII_2_MJD( param.end_date, &mjd2000, &secnd, &mu_sec );
	       calopt->stop_time.days = mjd2000;
	       calopt->stop_time.secnd = secnd;
	       calopt->stop_time.musec = mu_sec;
	  }	  
	  if ( param.catID_nr == PARAM_SET )
	       calopt->category_filter = (signed char) -1;
	  else
	       calopt->category_filter = SCHAR_ZERO;

	  if ( is_scia_lv1c && calopt->nadir_mds == SCHAR_ZERO )
	       calopt->nadir_mds = SCHAR_ZERO;
	  else {
	       if ( param.write_nadir == PARAM_SET ) 
		    calopt->nadir_mds = (signed char) -1;
	       else
		    calopt->nadir_mds = SCHAR_ZERO;
	  }

	  if ( param.write_limb == PARAM_SET )
	       calopt->limb_mds = (signed char) -1;
	  else
	       calopt->limb_mds = SCHAR_ZERO;
	  if ( param.write_occ == PARAM_SET )
	       calopt->occ_mds = (signed char) -1;
	  else
	       calopt->occ_mds = SCHAR_ZERO;
	  if ( param.write_moni == PARAM_SET )
	       calopt->moni_mds = (signed char) -1;
	  else
	       calopt->moni_mds = SCHAR_ZERO;
	  if ( param.write_pmd == PARAM_SET )
	       calopt->pmd_mds = (signed char) -1;
	  else
	       calopt->pmd_mds = SCHAR_ZERO;
	  if ( param.write_polV == PARAM_SET )
	       calopt->frac_pol_mds = (signed char) -1;
	  else
	       calopt->frac_pol_mds = SCHAR_ZERO;

	  if ( param.clusID_nr != UCHAR_ZERO ) {
	       calopt->num_nadir = 0;
	       calopt->num_limb = 0;
	       calopt->num_occ = 0;
	       calopt->num_moni = 0;
	       for ( nr = 0; nr < MAX_CLUSTER; nr++ ) {
		    if ( Get_Bit_LL( param.clus_mask, 
				     (unsigned char) nr ) != 0ULL ) {
			 calopt->nadir_cluster[nr] = (signed char) -1;
			 calopt->num_nadir++;
			 calopt->limb_cluster[nr] = (signed char) -1;
			 calopt->num_limb++;
			 calopt->occ_cluster[nr] = (signed char) -1;
			 calopt->num_occ++;
			 calopt->moni_cluster[nr] = (signed char) -1;
			 calopt->num_moni++;
		    } else {
			 calopt->nadir_cluster[nr] = SCHAR_ZERO;
			 calopt->limb_cluster[nr] = SCHAR_ZERO;
			 calopt->occ_cluster[nr] = SCHAR_ZERO;
			 calopt->moni_cluster[nr] = SCHAR_ZERO;
		    }	       
	       }
	  }
     } else {                                       /* fill calopt-structure */
	  char *pntr = strrchr( param.infile, '/' );

	  if ( pntr != NULL )
	       (void) strlcpy(calopt->l1b_prod_name, pntr+1, 
			      ENVI_FILENAME_SIZE);
	  else
	       (void) strlcpy( calopt->l1b_prod_name, param.infile, 
			       ENVI_FILENAME_SIZE );

	  if ( param.flag_geoloc == PARAM_SET ) {
	       calopt->geo_filter = (signed char) -1;
	       calopt->start_lat = (int) (1e6 * param.geo_lat[0]);
	       calopt->start_lon = (int) (1e6 * param.geo_lon[0]);
	       calopt->end_lat = (int) (1e6 * param.geo_lat[1]);
	       calopt->end_lon = (int) (1e6 * param.geo_lon[1]);
	  } else {
	       calopt->geo_filter = SCHAR_ZERO;
	       calopt->start_lat = 0;
	       calopt->start_lon = 0;
	       calopt->end_lat = 0;
	       calopt->end_lon = 0;
	  }
	  if ( param.flag_period == PARAM_SET ) {
	       int          mjd2000;
	       unsigned int secnd, mu_sec;

	       calopt->time_filter = (signed char) -1;
	       ASCII_2_MJD( param.bgn_date, &mjd2000, &secnd, &mu_sec );
	       calopt->start_time.days = mjd2000;
	       calopt->start_time.secnd = secnd;
	       calopt->start_time.musec = mu_sec;
	       ASCII_2_MJD( param.end_date, &mjd2000, &secnd, &mu_sec );
	       calopt->stop_time.days = mjd2000;
	       calopt->stop_time.secnd = secnd;
	       calopt->stop_time.musec = mu_sec;
	  } else {
	       calopt->time_filter = SCHAR_ZERO;
	       calopt->start_time.days = 0;
	       calopt->start_time.secnd = 0;
	       calopt->start_time.musec = 0;
	       calopt->stop_time.days = 0;
	       calopt->stop_time.secnd = 0;
	       calopt->stop_time.musec = 0;
	  }
	  if ( param.catID_nr == PARAM_SET )
	       calopt->category_filter = (signed char) -1;
	  else
	       calopt->category_filter = SCHAR_ZERO;
	  if ( ! is_scia_lv1c ) {
	       calopt->category[0] = 0;
	       calopt->category[1] = 0;
	       calopt->category[2] = 0;
	       calopt->category[3] = 0;
	       calopt->category[4] = 0;
	  }
	  if ( param.write_nadir == PARAM_SET ) {
	       calopt->nadir_mds = (signed char) -1;
	       if ( param.clusID_nr == UCHAR_ZERO ) {
		    calopt->num_nadir = MAX_CLUSTER;
		    for ( nr = 0; nr < MAX_CLUSTER; nr++ )
			 calopt->nadir_cluster[nr] = (signed char) -1;
	       } else {
		    calopt->num_nadir = 0;
		    for ( nr = 0; nr < MAX_CLUSTER; nr++ ) {
			 if ( Get_Bit_LL( param.clus_mask,
					  (unsigned char) nr ) != 0ULL ) {
			      calopt->nadir_cluster[nr] = (signed char) -1;
			      calopt->num_nadir++;
			 } else {
			      calopt->nadir_cluster[nr] = SCHAR_ZERO;
			 }
		    }
	       }
	  } else {
	       calopt->nadir_mds = SCHAR_ZERO;
	       calopt->num_nadir = 0;
	       for ( nr = 0; nr < MAX_CLUSTER; nr++ )
		    calopt->nadir_cluster[nr] = SCHAR_ZERO;
	  }
	  if ( param.write_limb == PARAM_SET ) {
	       calopt->limb_mds = (signed char) -1;
	       if ( param.clusID_nr == UCHAR_ZERO ) {
		    calopt->num_limb = MAX_CLUSTER;
		    for ( nr = 0; nr < MAX_CLUSTER; nr++ )
			 calopt->limb_cluster[nr] = (signed char) -1;
	       } else {
		    calopt->num_limb = 0;
		    for ( nr = 0; nr < MAX_CLUSTER; nr++ ) {
			 if ( Get_Bit_LL( param.clus_mask,
					  (unsigned char) nr ) != 0ULL ) {
			      calopt->limb_cluster[nr] = (signed char) -1;
			      calopt->num_limb++;
			 } else {
			      calopt->limb_cluster[nr] = SCHAR_ZERO;
			 }
		    }
	       }
	  } else {
	       calopt->limb_mds = SCHAR_ZERO;
	       calopt->num_limb = 0;
	       for ( nr = 0; nr < MAX_CLUSTER; nr++ )
		    calopt->limb_cluster[nr] = SCHAR_ZERO;
	  }
	  if ( param.write_occ == PARAM_SET ) {
	       calopt->occ_mds = (signed char) -1;
	       if ( param.clusID_nr == UCHAR_ZERO ) {
		    calopt->num_occ = MAX_CLUSTER;
		    for ( nr = 0; nr < MAX_CLUSTER; nr++ )
			 calopt->occ_cluster[nr] = (signed char) -1;
	       } else {
		    calopt->num_occ = 0;
		    for ( nr = 0; nr < MAX_CLUSTER; nr++ ) {
			 if ( Get_Bit_LL( param.clus_mask,
					  (unsigned char) nr ) != 0ULL ) {
			      calopt->occ_cluster[nr] = (signed char) -1;
			      calopt->num_occ++;
			 } else {
			      calopt->occ_cluster[nr] = SCHAR_ZERO;
			 }
		    }
	       }
	  } else {
	       calopt->occ_mds = SCHAR_ZERO;
	       calopt->num_occ = 0;
	       for ( nr = 0; nr < MAX_CLUSTER; nr++ )
		    calopt->occ_cluster[nr] = SCHAR_ZERO;
	  }
	  if ( param.write_moni == PARAM_SET ) {
	       calopt->moni_mds = (signed char) -1;
	       if ( param.clusID_nr == UCHAR_ZERO ) {
		    calopt->num_moni = MAX_CLUSTER;
		    for ( nr = 0; nr < MAX_CLUSTER; nr++ )
			 calopt->moni_cluster[nr] = (signed char) -1;
	       } else {
		    calopt->num_moni = 0;
		    for ( nr = 0; nr < MAX_CLUSTER; nr++ ) {
			 if ( Get_Bit_LL( param.clus_mask,
					  (unsigned char) nr ) != 0ULL ) {
			      calopt->moni_cluster[nr] = (signed char) -1;
			      calopt->num_moni++;
			 } else {
			      calopt->moni_cluster[nr] = SCHAR_ZERO;
			 }
		    }
	       }
	  } else {
	       calopt->moni_mds = SCHAR_ZERO;
	       calopt->num_moni = 0;
	       for ( nr = 0; nr < MAX_CLUSTER; nr++ )
		    calopt->moni_cluster[nr] = SCHAR_ZERO;
	  }
	  if ( param.write_pmd == PARAM_SET )
	       calopt->pmd_mds = (signed char) -1;
	  else
	       calopt->pmd_mds = SCHAR_ZERO;
	  if ( param.write_polV == PARAM_SET )
	       calopt->frac_pol_mds = (signed char) -1;
	  else
	       calopt->frac_pol_mds = SCHAR_ZERO;

	  if ( param.write_gads == PARAM_SET ) {
	       calopt->slit_function = (signed char) -1;
	       calopt->sun_mean_ref = (signed char) -1;
	       calopt->leakage_current = (signed char) -1;
	       calopt->spectral_cal = (signed char) -1;
	       calopt->pol_sens = (signed char) -1;
	       calopt->rad_sens = (signed char) -1;
	       calopt->ppg_etalon = (signed char) -1;
	  } else {
	       calopt->slit_function = SCHAR_ZERO;
	       calopt->sun_mean_ref = SCHAR_ZERO;
	       calopt->leakage_current = SCHAR_ZERO;
	       calopt->spectral_cal = SCHAR_ZERO;
	       calopt->pol_sens = SCHAR_ZERO;
	       calopt->rad_sens = SCHAR_ZERO;
	       calopt->ppg_etalon = SCHAR_ZERO;
	  }
	  if ( (param.calib_scia & (DO_CORR_VIS_MEM|DO_CORR_IR_NLIN)) 
	       != UINT_ZERO )
	       calopt->mem_effect_cal = (signed char) -1;
	  else
	       calopt->mem_effect_cal = SCHAR_ZERO;
	  if ( (param.calib_scia & (DO_CORR_AO|DO_CORR_DARK|DO_CORR_VDARK)) 
	       != UINT_ZERO )
	       calopt->leakage_cal = (signed char) -1;
	  else
	       calopt->leakage_cal = SCHAR_ZERO;
	  if ( (param.calib_scia & DO_CORR_STRAY) != UINT_ZERO )
	       calopt->straylight_cal = (signed char) -1;
	  else
	       calopt->straylight_cal = SCHAR_ZERO;
	  if ( (param.calib_scia & DO_CORR_PPG) != UINT_ZERO )
	       calopt->ppg_cal = (signed char) -1;
	  else
	       calopt->ppg_cal = SCHAR_ZERO;
	  if ( (param.calib_scia & DO_CORR_ETALON) != UINT_ZERO )
	       calopt->etalon_cal = (signed char) -1;
	  else
	       calopt->etalon_cal = SCHAR_ZERO;
	  if ( (param.calib_scia & DO_CALIB_WAVE) != UINT_ZERO )
	       calopt->wave_cal = (signed char) -1;
	  else
	       calopt->wave_cal = SCHAR_ZERO;
	  if ( (param.calib_scia & DO_CORR_POL) != UINT_ZERO )
	       calopt->polarisation_cal = (signed char) -1;
	  else
	       calopt->polarisation_cal = SCHAR_ZERO;
	  if ( (param.calib_scia & DO_CORR_RAD) != UINT_ZERO )
	       calopt->radiance_cal = (signed char) -1;
	  else
	       calopt->radiance_cal = SCHAR_ZERO;
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1C_WR_CALOPT
.PURPOSE     write Calibration Options GADS to SciaL1C file
.INPUT/OUTPUT
  call as    SCIA_LV1C_WR_CALOPT( fd, num_calopt, &calopt );
     input:  
            FILE   *fd              :  (open) stream pointer
	    unsigned int num_calopt :  number of CalOpt records
            struct cal_options calopt :  structure for the calibration options

.RETURNS     nothing
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1C_WR_CALOPT( FILE *fd, unsigned int num_calopt,
			  const struct cal_options calopt_in )
{
     const char prognm[] = "SCIA_LV1C_WR_CALOPT";

     struct cal_options calopt;

     struct dsd_envi dsd = {
	  "CAL_OPTIONS", "G",
	  "                                                              ",
	  0u, 0u, 0u, 0
     };

     if ( num_calopt == 0u ) return;
/*
 * copy data set records
 */
     (void) memcpy( &calopt, &calopt_in, sizeof( struct cal_options ));
#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_CALOPT( &calopt );
#endif
/*
 * write CALOPT structure to file
 */
     if ( fwrite( calopt.l1b_prod_name, ENVI_FILENAME_SIZE-1, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_FILENAME_SIZE-1;
     if ( fwrite( &calopt.geo_filter, ENVI_CHAR, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_CHAR;
     if ( fwrite( &calopt.start_lat, ENVI_INT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_INT;
     if ( fwrite( &calopt.start_lon, ENVI_INT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_INT;
     if ( fwrite( &calopt.end_lat, ENVI_INT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_INT;
     if ( fwrite( &calopt.end_lon, ENVI_INT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_INT;

     if ( fwrite( &calopt.time_filter, ENVI_CHAR, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_CHAR;
     if ( fwrite( &calopt.start_time.days, ENVI_INT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_INT;
     if ( fwrite( &calopt.start_time.secnd, ENVI_UINT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_UINT;
     if ( fwrite( &calopt.start_time.musec, ENVI_UINT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_UINT;
     if ( fwrite( &calopt.stop_time.days, ENVI_INT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_INT;
     if ( fwrite( &calopt.stop_time.secnd, ENVI_UINT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_UINT;
     if ( fwrite( &calopt.stop_time.musec, ENVI_UINT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_UINT;

     if ( fwrite( &calopt.category_filter, ENVI_CHAR, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_CHAR;
     if ( fwrite( calopt.category, 5 * ENVI_USHRT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += 5 * ENVI_USHRT;

     if ( fwrite( &calopt.nadir_mds, ENVI_CHAR, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_CHAR;
     if ( fwrite( &calopt.limb_mds, ENVI_CHAR, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_CHAR;
     if ( fwrite( &calopt.occ_mds, ENVI_CHAR, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_CHAR;
     if ( fwrite( &calopt.moni_mds, ENVI_CHAR, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_CHAR;
     if ( fwrite( &calopt.pmd_mds, ENVI_CHAR, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_CHAR;
     if ( fwrite( &calopt.frac_pol_mds, ENVI_CHAR, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_CHAR;

     if ( fwrite( &calopt.slit_function, ENVI_CHAR, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_CHAR;
     if ( fwrite( &calopt.sun_mean_ref, ENVI_CHAR, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_CHAR;
     if ( fwrite( &calopt.leakage_current, ENVI_CHAR, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_CHAR;
     if ( fwrite( &calopt.spectral_cal, ENVI_CHAR, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_CHAR;
     if ( fwrite( &calopt.pol_sens, ENVI_CHAR, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_CHAR;
     if ( fwrite( &calopt.rad_sens, ENVI_CHAR, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_CHAR;
     if ( fwrite( &calopt.ppg_etalon, ENVI_CHAR, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_CHAR;

     if ( fwrite( &calopt.num_nadir, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_USHRT;
     if ( fwrite( &calopt.num_limb, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_USHRT;
     if ( fwrite( &calopt.num_occ, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_USHRT;
     if ( fwrite( &calopt.num_moni, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_USHRT;
     if ( fwrite( calopt.nadir_cluster, MAX_CLUSTER, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += MAX_CLUSTER;
     if ( fwrite( calopt.limb_cluster, MAX_CLUSTER, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += MAX_CLUSTER;
     if ( fwrite( calopt.occ_cluster, MAX_CLUSTER, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += MAX_CLUSTER;
     if ( fwrite( calopt.moni_cluster, MAX_CLUSTER, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += MAX_CLUSTER;

     if ( fwrite( &calopt.mem_effect_cal, ENVI_CHAR, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_CHAR;
     if ( fwrite( &calopt.leakage_cal, ENVI_CHAR, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_CHAR;
     if ( fwrite( &calopt.straylight_cal, ENVI_CHAR, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_CHAR;
     if ( fwrite( &calopt.ppg_cal, ENVI_CHAR, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_CHAR;
     if ( fwrite( &calopt.etalon_cal, ENVI_CHAR, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_CHAR;
     if ( fwrite( &calopt.wave_cal, ENVI_CHAR, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_CHAR;
     if ( fwrite( &calopt.polarisation_cal, ENVI_CHAR, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_CHAR;
     if ( fwrite( &calopt.radiance_cal, ENVI_CHAR, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_CHAR;
/*
 * update list of written DSD records
 */
     dsd.num_dsr = 1u;
     dsd.dsr_size = (int) dsd.size;
     SCIA_LV1_ADD_DSD( &dsd );
}
