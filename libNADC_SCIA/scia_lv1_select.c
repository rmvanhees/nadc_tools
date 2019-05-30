/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2001 - 2019 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV1_SELECT
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1b data selection
.LANGUAGE    ANSI C
.PURPOSE     obtain state-records of selected MDS records
.INPUT/OUTPUT
  call as   nr_mds = SCIA_LV1_SELECT_MDS(source, num_dsd, dsd,
                                         num_state, state, &mds_state);
     input: 
            int source                 : data source (Nadir, Limb, ...)
	    unsigned int num_dsd       : number of DSD records
	    struct dsd_envi *dsd       : structure with DSD records
	    unsigned int num_state     : number of State records
	    struct state1_scia *state  : structure with States of the product
    output:  
	    struct state1_scia **mds_state: array with selected state-records

.RETURNS     number of selected records (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION      1.4   27-Jun-2007	bugfixed geolocation selection, RvH
              1.3   11-Oct-2005	add usage of SCIA_LV1_EXPORT_NUM_STATE, RvH
              1.2   18-Sep-2002	added geolocation selection (LADS, only), RvH
              1.1   13-Sep-2002	added cluster selection, RvH
              1.0   22-Feb-2002	created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_SOURCE 2

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#include "selected_channel.inc"

static inline
int IS_SELECTED_GEO(const struct coord_envi *coord_in)
{
     register short nc;

     int   lat_min, lat_max, lon_min, lon_max;
     float rbuff[2];

     int coord_lat_min = INT_MAX;
     int coord_lon_min = INT_MAX;
     int coord_lat_max = INT_MIN;
     int coord_lon_max = INT_MIN;

     struct coord_envi coord[NUM_CORNERS];

     nadc_get_param_range("latitude", rbuff);
     lat_min = (int) (1e6 * rbuff[0]);
     lat_max = (int) (1e6 * rbuff[1]);
     nadc_get_param_range("longitude", rbuff);
     lon_min = (int) (1e6 * rbuff[0]);
     lon_max = (int) (1e6 * rbuff[1]);

     /* copy the coordinates of the Sciamachy pixel/state */
     (void) memcpy(coord, coord_in, NUM_CORNERS * sizeof(struct coord_envi));
/*
 * at least one corner has to be within latitude/longitude range
 */
     for (nc = 0; nc < NUM_CORNERS; nc++) {
	  if ((lat_min <= coord[nc].lat && lat_max >= coord[nc].lat)
	       && (lon_min <= coord[nc].lon && lon_max >= coord[nc].lon))
	       return TRUE;
     }

     /* get range of pixel/state coordinates */
     for (nc = 0; nc < NUM_CORNERS; nc++) {
	  if (coord_lat_min > coord[nc].lat) coord_lat_min = coord[nc].lat;
	  if (coord_lat_max < coord[nc].lat) coord_lat_max = coord[nc].lat;
	  if (coord_lon_min > coord[nc].lon) coord_lon_min = coord[nc].lon;
	  if (coord_lon_max < coord[nc].lon) coord_lon_max = coord[nc].lon;
     }
     if (coord_lon_min < 0 && (coord_lon_max - coord_lon_min) > (int)210e6) {
	  int tmp = coord_lon_min;

	  coord_lon_min = coord_lon_max;
	  coord_lon_max = tmp + (int) 360e6;
     }

     /* both latitude coordinates outside region - longitude within */
     if ((coord_lat_min < lat_min && coord_lat_max > lat_max)
	  && ((coord_lon_min >= lon_min && coord_lon_min <= lon_max)
	      || (coord_lon_max >= lon_min && coord_lon_max <= lon_max)))
	  return TRUE;
     /* both longitude coordinates outside region - latitude within */
     if ((coord_lon_min < lon_min && coord_lon_max > lon_max)
	  && ((coord_lat_min >= lat_min && coord_lat_min <= lat_max)
	      || (coord_lat_max >= lat_min && coord_lat_max <= lat_max)))
	  return TRUE;
     /* both longitude/latitude coordinates outside region */
     if ((coord_lat_min < lat_min && coord_lat_max > lat_max)
	  && (coord_lon_min < lon_min && coord_lon_max > lon_max))
	  return TRUE;

     return FALSE;
}

static inline
int IS_SELECTED_CAT(unsigned short category)
{
     if (nadc_get_param_cat(category))
	  return TRUE;

     return FALSE;
}

static inline
int IS_SELECTED_STATE(unsigned short state_id)
{
     if (nadc_get_param_state(state_id))
	  return TRUE;

     return FALSE;
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned int SCIA_LV1_SELECT_MDS(int source, 
				  FILE *fp, unsigned int num_dsd, 
				  const struct dsd_envi *dsd,
				  struct state1_scia **mds_state)
{
     register unsigned short nc;
     register unsigned int ni = 0;
     register unsigned int num_not = 0;
     register unsigned int num_select = 0;

     char         *cpntr;
     bool         found;
     int          mjd2000;
     unsigned int secnd, mu_sec;
     unsigned int num_state;

     unsigned int indx_dsd, *indx_state = NULL;

     double       bgn_jdate = 0.;
     double       end_jdate = 0.;

     struct lads_scia   *lads = NULL;
     struct state1_scia *state = NULL;

     const double SecPerDay = 24. * 60. * 60.;

     const bool Save_Extern_Alloc = Use_Extern_Alloc;
/*
 * initialize output array
 */
     *mds_state = NULL;
/*
 * first check type of selected MDS
 */
     switch (source) {
     case SCIA_NADIR:
	  if (nadc_get_param_uint8("write_nadir") != PARAM_SET)
	       return 0u;
	  indx_dsd = ENVI_GET_DSD_INDEX(num_dsd, dsd, "NADIR");
	  if (IS_ERR_STAT_FATAL)
	       NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "NADIR");
	  if (dsd[indx_dsd].num_dsr == 0)
	       return 0u;
	  break;
     case SCIA_LIMB:
	  if (nadc_get_param_uint8("write_limb") != PARAM_SET)
	       return 0u;
	  indx_dsd = ENVI_GET_DSD_INDEX(num_dsd, dsd, "LIMB");
	  if (IS_ERR_STAT_FATAL)
	       NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "LIMB");
	  if (dsd[indx_dsd].num_dsr == 0)
	       return 0u;
	  break;
     case SCIA_OCCULT:
	  if (nadc_get_param_uint8("write_occ") != PARAM_SET)
	       return 0u;
	  indx_dsd = ENVI_GET_DSD_INDEX(num_dsd, dsd, "OCCULTATION");
	  if (IS_ERR_STAT_FATAL)
	       NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "OCCULTATION");
	  if (dsd[indx_dsd].num_dsr == 0)
	       return 0u;
	  break;
     case SCIA_MONITOR:
	  if (nadc_get_param_uint8("write_moni") != PARAM_SET)
	       return 0u;
	  indx_dsd = ENVI_GET_DSD_INDEX(num_dsd, dsd, "MONITORING");
	  if (IS_ERR_STAT_FATAL)
	       NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "MONITORING");
	  if (dsd[indx_dsd].num_dsr == 0)
	       return 0u;
	  break;
     default:
	  NADC_GOTO_ERROR(NADC_ERR_FATAL, "unknown MDS state");
     }
/*
 * initialize begin and end julian date of time window
 */
     if (nadc_get_param_uint8("flag_period") == PARAM_SET) {
	  cpntr = nadc_get_param_string("bgn_date");
	  ASCII_2_MJD(cpntr, &mjd2000, &secnd, &mu_sec);
	  bgn_jdate = mjd2000 + (secnd + mu_sec / 1e6) / SecPerDay;
	  free(cpntr);
	  cpntr = nadc_get_param_string("end_date");
	  ASCII_2_MJD(cpntr, &mjd2000, &secnd, &mu_sec);
	  end_jdate = mjd2000 + (secnd + mu_sec / 1e6) / SecPerDay;
	  free(cpntr);
     }
/*
 * read State of the Products (ADS)
 */
     Use_Extern_Alloc = FALSE;
     num_state = SCIA_LV1_RD_STATE(fp, num_dsd, dsd, &state);
     Use_Extern_Alloc = Save_Extern_Alloc;
     if (IS_ERR_STAT_FATAL)
          NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "STATE");
/*
 * read Geolocation of States (LADS)
 */
     Use_Extern_Alloc = FALSE;
     (void) SCIA_RD_LADS(fp, num_dsd, dsd, &lads);
     Use_Extern_Alloc = Save_Extern_Alloc;
     if (IS_ERR_STAT_FATAL)
          NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "LADS");
/*
 * allocate memory to store indices to selected MDS records
 */
     indx_state = (unsigned int *) malloc(num_state * sizeof(unsigned int));
     if (indx_state == NULL) 
	  NADC_GOTO_ERROR(NADC_ERR_ALLOC, "indx_state");
/*
 * go through all state-records
 */
     do {
	  if ((int) state[ni].type_mds == source 
	       && state[ni].flag_mds == MDS_ATTACHED ) {

	       if (nadc_get_param_uint8("flag_period") == PARAM_SET) {
		    double mjd_date;

		    mjd_date = state[ni].mjd.days 
			 + (state[ni].mjd.secnd 
			    + state[ni].mjd.musec / 1e6) / SecPerDay;
		    if (mjd_date < bgn_jdate) goto Not_Selected;

		    mjd_date += (state[ni].dur_scan / 16.) / SecPerDay;
		    if (mjd_date > end_jdate) goto Not_Selected;
	       }

	       if (! IS_SELECTED_CAT(state[ni].category))
		    goto Not_Selected;

	       if (! IS_SELECTED_STATE(state[ni].state_id))
		    goto Not_Selected;

	       nc = 0;
	       found = FALSE;
	       do {
		    if (nadc_get_param_clus(nc)) {
			 found = TRUE;
			 break;
		    }
	       } while (++nc < state[ni].num_clus);
	       if (! found) goto Not_Selected;

	       nc = 0;
	       found = FALSE;
	       do {
		    if (nadc_get_param_chan(state[ni].Clcon[nc].channel)) {
			 found = TRUE;
			 break;
		    }
	       } while (++nc < state[ni].num_clus);
	       if (! found) goto Not_Selected;

	       if (nadc_get_param_uint8("flag_geoloc") == PARAM_SET) {
		    if (! IS_SELECTED_GEO(lads[ni].corner))
			 goto Not_Selected;
	       }

	       indx_state[num_select++] = ni;
	  Not_Selected:
	       num_not++;  /* FAKE counter, NOT used! */ 
	  }
     } while (++ni < num_state);
/*
 * copy selected state-records to output array
 */
     SCIA_LV1_EXPORT_NUM_STATE(source, (unsigned short) num_select);
     if (num_select > 0u) {
	  *mds_state = (struct state1_scia *) 
	       malloc(num_select * sizeof(struct state1_scia));
	  if (*mds_state == NULL) {
	       num_select = 0u;
	       NADC_GOTO_ERROR(NADC_ERR_ALLOC, "mds_state");
	  }
	  for (ni = 0; ni < num_select; ni++)
	       (void) memcpy(&(*mds_state)[ni], &state[indx_state[ni]],
			      sizeof(struct state1_scia));
     }
 done:
     if (lads != NULL) free(lads);
     if (state != NULL) free(state);
     if (indx_state != NULL) free(indx_state);
     return num_select;
}
