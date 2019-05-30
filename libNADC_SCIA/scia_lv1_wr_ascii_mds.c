/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2000 - 2019 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV1_WR_ASCII_MDS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1b/1c - ASCII dump
.LANGUAGE    ANSI C
.PURPOSE     Dump SCIAMACHY level 1 Measurement Data Sets in ASCII
.COMMENTS    contains SCIA_LV1_WR_ASCII_MDS, SCIA_LV1C_WR_ASCII_MDS,
                SCIA_LV1C_WR_ASCII_MDS_PMD, SCIA_LV1C_WR_ASCII_MDS_POLV
.ENVIRONment None
.VERSION      5.0   07-Dev-2005 removed esig/esigc from MDS(1b)-struct,
			 	renamed pixel_val_err to pixel_err, RvH
              4.8   17-Oct-2005	write only one MDS_PMD or MDS_POLV record, RvH
              4.7   11-Oct-2005	pass state-record always by reference, RvH
              4.6   20-Mar-2003	Level 1c routines only require an unique 
                                ID (= index) of a state, RvH
              4.5   09-Aug-2002	do not attempt to write level 1c PMD/polV 
	                        for monitoring states, RvH
              4.4   07-Aug-2002	mds1c_pmd & mds1c_polV don't have geoC, RvH 
              4.3   11-Apr-2002	added level 1c routines, RvH
              4.2   07-Mar-2002	gave MDS structs more logical names, RvH 
              4.1   28-Feb-2002	modified struct mds1_scia, RvH 
              4.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              3.1   03-Aug-2001	Ahhhrgg SCIA_L01 format, RvH 
              3.0   03-Jan-2001 split the module "write_ascii", RvH
              2.2   21-Dec-2000 added SCIA_LV1_WR_ASCII_NADIR, RvH
              2.1   20-Dec-2000 use output filename given by the user, RvH
              2.0   17-Aug-2000 major rewrite and standardization, RvH
              1.1   14-Jul-2000 renamed: DEBUG -> SCIA_LV1_WR_ASCII, RvH
              1.0   02-Mar-1999 created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

#define INDX_GEON 444u
#define INDX_GEOL 555u
#define INDX_GEOC 666u
#define INDX_POLV 777u

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static 
void SCIA_LV1_WR_ASCII_GEON(FILE *outfl, unsigned short nr_geo,
			     const struct geoN_scia *geoN)
{
     register unsigned int ni, nx, ny;

     unsigned char *cbuff;
     unsigned int  count[2];
     float         *rbuff;

     count[0] = nr_geo;
     cbuff = (unsigned char *) malloc((size_t) count[0]);
     if (cbuff == NULL) 
	  NADC_RETURN_ERROR(NADC_ERR_ALLOC, "cbuff");
     for (nx = 0; nx < count[0]; nx++)
	  cbuff[nx] = geoN[nx].pixel_type;
     nadc_write_arr_uchar(outfl, INDX_GEON, "Pixel Type (backscan=0)", 
			    1, count, cbuff);
     for (nx = 0; nx < count[0]; nx++)
	  cbuff[nx] = geoN[nx].glint_flag;
     nadc_write_arr_uchar(outfl, INDX_GEON, "Sun glint/Rainbow flag", 
			    1, count, cbuff);
     free(cbuff);
     rbuff = (float *) malloc((size_t) count[0] * sizeof(float));
     if (rbuff == NULL) 
	  NADC_RETURN_ERROR(NADC_ERR_ALLOC, "rbuff");
     for (nx = 0; nx < count[0]; nx++)
	  rbuff[nx] = geoN[nx].pos_esm;
     nadc_write_arr_float(outfl, INDX_GEON, "Position ESM", 
			    1, count, 5, rbuff);
     count[1] = 3;
     rbuff = (float *) realloc(rbuff, (size_t) (count[0] * count[1])
				* sizeof(float));
     if (rbuff == NULL) 
	  NADC_RETURN_ERROR(NADC_ERR_ALLOC, "rbuff");
     for (ni = ny = 0; ny < count[1]; ny++) {
	  for (nx = 0; nx < count[0]; nx++) 
	       rbuff[ni++] = geoN[nx].sun_zen_ang[ny];
     }
     nadc_write_arr_float(outfl, INDX_GEON, "Sun zenith angles", 
			    2, count, 5, rbuff);
     for (ni = ny = 0; ny < count[1]; ny++) {
	  for (nx = 0; nx < count[0]; nx++) 
	       rbuff[ni++] = geoN[nx].sun_azi_ang[ny];
     }
     nadc_write_arr_float(outfl, INDX_GEON, "Sun azimuth angles", 
			    2, count, 5, rbuff);
     for (ni = ny = 0; ny < count[1]; ny++) {
	  for (nx = 0; nx < count[0]; nx++) 
	       rbuff[ni++] = geoN[nx].los_zen_ang[ny];
     }
     nadc_write_arr_float(outfl, INDX_GEON, "LOS zenith angles", 
			    2, count, 5, rbuff);
     for (ni = ny = 0; ny < count[1]; ny++) {
	  for (nx = 0; nx < count[0]; nx++) 
	       rbuff[ni++] = geoN[nx].los_azi_ang[ny];
     }
     nadc_write_arr_float(outfl, INDX_GEON, "LOS azimuth angles", 
			    2, count, 5, rbuff);
     for (nx = 0; nx < count[0]; nx++)
	  rbuff[nx] = geoN[nx].sat_h;
     nadc_write_arr_float(outfl, INDX_GEON, "Satellite height", 
			    1, count, 5, rbuff);
     for (nx = 0; nx < count[0]; nx++)
	  rbuff[nx] = geoN[nx].earth_rad;
     nadc_write_arr_float(outfl, INDX_GEON, "Earth radius", 
			    1, count, 5, rbuff);
     count[1] = 2;
     rbuff = (float *) realloc(rbuff, (size_t) (count[0] * count[1])
				* sizeof(float));
     if (rbuff == NULL) 
	  NADC_RETURN_ERROR(NADC_ERR_ALLOC, "rbuff");
     for (nx = 0; nx < count[0]; nx++) {
	  rbuff[nx] = geoN[nx].sub_sat_point.lat / 1e6;
	  rbuff[count[0] + nx] = 
	       geoN[nx].sub_sat_point.lon / 1e6;
     }
     nadc_write_arr_float(outfl, INDX_GEON, "sub-Satellite point", 
			    2, count, 6, rbuff);
     for (nx = 0; nx < count[0]; nx++) {
	  rbuff[nx] = geoN[nx].corner[0].lat / 1e6;
	  rbuff[count[0] + nx] = geoN[nx].corner[0].lon / 1e6;
     }
     nadc_write_arr_float(outfl, INDX_GEON, "corner_coord[1]",
			    2, count, 6, rbuff);
     for (nx = 0; nx < count[0]; nx++) {
	  rbuff[nx] = geoN[nx].corner[1].lat / 1e6;
	  rbuff[count[0] + nx] = geoN[nx].corner[1].lon / 1e6;
     }
     nadc_write_arr_float(outfl, INDX_GEON, "corner_coord[2]", 
			    2, count, 6, rbuff);
     for (nx = 0; nx < count[0]; nx++) {
	  rbuff[nx] = geoN[nx].corner[2].lat / 1e6;
	  rbuff[count[0] + nx] = geoN[nx].corner[2].lon / 1e6;
     }
     nadc_write_arr_float(outfl, INDX_GEON, "corner_coord[3]", 
			    2, count, 6, rbuff);
     for (nx = 0; nx < count[0]; nx++) {
	  rbuff[nx] = geoN[nx].corner[3].lat / 1e6;
	  rbuff[count[0] + nx] = geoN[nx].corner[3].lon / 1e6;
     }
     nadc_write_arr_float(outfl, INDX_GEON, "corner_coord[4]", 
			    2, count, 6, rbuff);
     for (nx = 0; nx < count[0]; nx++) {
	  rbuff[nx] = geoN[nx].center.lat / 1e6;
	  rbuff[count[0]+nx] = geoN[nx].center.lon / 1e6;
     }
     nadc_write_arr_float(outfl, INDX_GEON, "Center coordinates", 
			    2, count, 6, rbuff);
     free(rbuff);
}

static 
void SCIA_LV1_WR_ASCII_GEOL(FILE *outfl, unsigned short nr_geo,
			     const struct geoL_scia *geoL)
{
     register unsigned int ni, nx, ny;

     unsigned int count[2];
     float *rbuff;

     count[0] = nr_geo;
     rbuff = (float *) malloc((size_t) count[0] * sizeof(float));
     if (rbuff == NULL) 
	  NADC_RETURN_ERROR(NADC_ERR_ALLOC, "rbuff");
     for (nx = 0; nx < count[0]; nx++)
	  rbuff[nx] = geoL[nx].pos_esm;
     nadc_write_arr_float(outfl, INDX_GEOL, "Position ESM", 
			    1, count, 5, rbuff);
     for (nx = 0; nx < count[0]; nx++)
	  rbuff[nx] = geoL[nx].pos_asm;
     nadc_write_arr_float(outfl, INDX_GEOL, "Position ASM", 
			    1, count, 5, rbuff);
     count[1] = 3;
     rbuff = (float *) realloc(rbuff, (size_t) (count[0] * count[1])
				* sizeof(float));
     if (rbuff == NULL) 
	  NADC_RETURN_ERROR(NADC_ERR_ALLOC, "rbuff");
     for (ni = ny = 0; ny < count[1]; ny++) {
	  for (nx = 0; nx < count[0]; nx++) 
	       rbuff[ni++] = geoL[nx].sun_zen_ang[ny];
     }
     nadc_write_arr_float(outfl, INDX_GEOL, "Sun zenith angles", 
			    2, count, 5, rbuff);
     for (ni = ny = 0; ny < count[1]; ny++) {
	  for (nx = 0; nx < count[0]; nx++) 
	       rbuff[ni++] = geoL[nx].sun_azi_ang[ny];
     }
     nadc_write_arr_float(outfl, INDX_GEOL, "Sun azimuth angles", 
			    2, count, 5, rbuff);
     for (ni = ny = 0; ny < count[1]; ny++) {
	  for (nx = 0; nx < count[0]; nx++) 
	       rbuff[ni++] = geoL[nx].los_zen_ang[ny];
     }
     nadc_write_arr_float(outfl, INDX_GEOL, "LOS zenith angles", 
			    2, count, 5, rbuff);
     for (ni = ny = 0; ny < count[1]; ny++) {
	  for (nx = 0; nx < count[0]; nx++) 
	       rbuff[ni++] = geoL[nx].los_azi_ang[ny];
     }
     nadc_write_arr_float(outfl, INDX_GEOL, "LOS azimuth angles", 
			    2, count, 5, rbuff);
     for (nx = 0; nx < count[0]; nx++)
	  rbuff[nx] = geoL[nx].sat_h;
     nadc_write_arr_float(outfl, INDX_GEOL, "Satellite height", 
			    1, count, 5, rbuff);
     for (nx = 0; nx < count[0]; nx++)
	  rbuff[nx] = geoL[nx].earth_rad;
     nadc_write_arr_float(outfl, INDX_GEOL, "Earth radius", 
			    1, count, 5, rbuff);
     for (ni = ny = 0; ny < count[1]; ny++) {
	  for (nx = 0; nx < count[0]; nx++) 
	       rbuff[ni++] = geoL[nx].tan_h[ny];
     }
     nadc_write_arr_float(outfl, INDX_GEOL, "Tangent height", 
			    2, count, 5, rbuff);
     for (nx = 0; nx < count[0]; nx++)
	  rbuff[nx] = geoL[nx].dopp_shift;
     nadc_write_arr_float(outfl, INDX_GEOL, "Doppler shift (500nm)", 
			    1, count, 5, rbuff);
     count[1] = 2;
     rbuff = (float *) realloc(rbuff, (size_t) (count[0] * count[1])
				* sizeof(float));
     if (rbuff == NULL) 
	  NADC_RETURN_ERROR(NADC_ERR_ALLOC, "rbuff");
     for (nx = 0; nx < count[0]; nx++) {
	  rbuff[nx] = geoL[nx].sub_sat_point.lat / 1e6;
	  rbuff[count[0] + nx] = 
	       geoL[nx].sub_sat_point.lon / 1e6;
     }
     nadc_write_arr_float(outfl, INDX_GEOL, "sub-Satellite point", 
			    2, count, 6, rbuff);
     for (nx = 0; nx < count[0]; nx++) {
	  rbuff[nx] = geoL[nx].tang_ground_point[0].lat / 1e6;
	  rbuff[count[0] + nx] = 
	       geoL[nx].tang_ground_point[0].lon / 1e6;
     }
     nadc_write_arr_float(outfl, INDX_GEOL, "tang_ground_point[1]",
			    2, count, 6, rbuff);
     for (nx = 0; nx < count[0]; nx++) {
	  rbuff[nx] = geoL[nx].tang_ground_point[1].lat / 1e6;
	  rbuff[count[0] + nx] = 
	       geoL[nx].tang_ground_point[1].lon / 1e6;
     }
     nadc_write_arr_float(outfl, INDX_GEOL, "tang_ground_point[2]", 
			    2, count, 6, rbuff);
     for (nx = 0; nx < count[0]; nx++) {
	  rbuff[nx] = geoL[nx].tang_ground_point[2].lat / 1e6;
	  rbuff[count[0] + nx] = 
	       geoL[nx].tang_ground_point[2].lon / 1e6;
     }
     nadc_write_arr_float(outfl, INDX_GEOL, "tang_ground_point[3]", 
			    2, count, 6, rbuff);
     free(rbuff);
}

static 
void SCIA_LV1_WR_ASCII_GEOC(FILE *outfl, unsigned short nr_geo,
			     const struct geoC_scia *geoC)
{
     register unsigned int nx;

     unsigned int count[2];
     float *rbuff;

     count[0] = nr_geo;
     rbuff = (float *) malloc((size_t) count[0] * sizeof(float));
     if (rbuff == NULL) 
	  NADC_RETURN_ERROR(NADC_ERR_ALLOC, "rbuff");
     for (nx = 0; nx < count[0]; nx++)
	  rbuff[nx] = geoC[nx].pos_esm;
     nadc_write_arr_float(outfl, INDX_GEOC, "Position ESM", 
			    1, count, 5, rbuff);
     for (nx = 0; nx < count[0]; nx++)
	  rbuff[nx] = geoC[nx].pos_asm;
     nadc_write_arr_float(outfl, INDX_GEOC, "Position ASM", 
			    1, count, 5, rbuff);
     for (nx = 0; nx < count[0]; nx++)
	  rbuff[nx] = geoC[nx].sun_zen_ang;
     nadc_write_arr_float(outfl, INDX_GEOC, "Sun zenith angle", 
			    1, count, 5, rbuff);
     count[1] = 2;
     rbuff = (float *) realloc(rbuff, (size_t) (count[0] * count[1])
				* sizeof(float));
     if (rbuff == NULL) 
	  NADC_RETURN_ERROR(NADC_ERR_ALLOC, "rbuff");
     for (nx = 0; nx < count[0]; nx++) {
	  rbuff[nx] = geoC[nx].sub_sat_point.lat / 1e6;
	  rbuff[count[0] + nx] = 
	       geoC[nx].sub_sat_point.lon / 1e6;
     }
     nadc_write_arr_float(outfl, INDX_GEOC, "sub-Satellite point", 
			    2, count, 6, rbuff);
     free(rbuff);
}

static 
void SCIA_LV1_WR_ASCII_POLV(FILE *outfl, unsigned short nr_pol,
			     const struct polV_scia *polV)
{
     register unsigned int ni;

     unsigned int count[2];
     float *rbuff;

     count[0] = nr_pol;
     rbuff = (float *) malloc((size_t) count[0] * sizeof(float));
     if (rbuff == NULL) 
	  NADC_RETURN_ERROR(NADC_ERR_ALLOC, "rbuff");
     for (ni = 0; ni < count[0]; ni++) rbuff[ni] = polV[ni].gdf.p_bar;
     nadc_write_arr_float(outfl, INDX_POLV, "GDF Pbar", 
			    1, count, 4, rbuff);
     for (ni = 0; ni < count[0]; ni++) rbuff[ni] = polV[ni].gdf.beta;
     nadc_write_arr_float(outfl, INDX_POLV, "GDF beta", 
			    1, count, 4, rbuff);
     for (ni = 0; ni < count[0]; ni++) rbuff[ni] = polV[ni].gdf.w0;
     nadc_write_arr_float(outfl, INDX_POLV, "GDF w0", 
			    1, count, 4, rbuff);
     
     count[0] = NUM_FRAC_POLV;
     count[1] = nr_pol;
     rbuff = (float *) realloc(rbuff, (size_t) (count[0] * count[1])
				* sizeof(float));
     if (rbuff == NULL) 
	  NADC_RETURN_ERROR(NADC_ERR_ALLOC, "rbuff");
     ni = 0;
     do {
	  (void) memcpy(rbuff, polV[ni].Q, count[0] * sizeof(float));
	  rbuff += count[0];
     } while (++ni < count[1]);
     rbuff -= (count[0] * count[1]);
     nadc_write_arr_float(outfl, INDX_POLV, "Fractional polarisation Q",
			    2, count, 4, rbuff);
     ni = 0;
     do {
	  (void) memcpy(rbuff, polV[ni].error_Q, count[0] * sizeof(float));
	  rbuff += count[0];
     } while (++ni < count[1]);
     rbuff -= (count[0] * count[1]);
     nadc_write_arr_float(outfl, INDX_POLV, "Error Q", 
			    2, count, 4, rbuff);
     ni = 0;
     do {
	  (void) memcpy(rbuff, polV[ni].U, count[0] * sizeof(float));
	  rbuff += count[0];
     } while (++ni < count[1]);
     rbuff -= (count[0] * count[1]);
     nadc_write_arr_float(outfl, INDX_POLV, "Fractional polarisation U",
			    2, count, 4, rbuff);
     ni = 0;
     do {
	  (void) memcpy(rbuff, polV[ni].error_U, count[0] * sizeof(float));
	  rbuff += count[0];
     } while (++ni < count[1]);
     rbuff -= (count[0] * count[1]);
     nadc_write_arr_float(outfl, INDX_POLV, "Error U", 
			    2, count, 4, rbuff);
     count[0] = NUM_FRAC_POLV+1;
     rbuff = (float *) realloc(rbuff, (size_t) (count[0] * count[1])
				* sizeof(float));
     if (rbuff == NULL) 
	  NADC_RETURN_ERROR(NADC_ERR_ALLOC, "rbuff");
     ni = 0;
     do {
	  (void) memcpy(rbuff, polV[ni].rep_wv, count[0] * sizeof(float));
	  rbuff += count[0];
     } while (++ni < count[1]);
     rbuff -= (count[0] * count[1]);
     nadc_write_arr_float(outfl, INDX_POLV, "Repeated wavelength", 
			    2, count, 4, rbuff);
     free(rbuff);
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_WR_ASCII_MDS
.PURPOSE    dump -- in ASCII Format -- the MDS records (Lv1b)
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_ASCII_MDS(num_mds, mds);
     input:  
	     unsigned int num_mds        : number of MDS records
	     struct mds1_scia  *mds      : MDS struct (level 1b)

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1_WR_ASCII_MDS(unsigned int num_mds, const struct mds1_scia *mds)
{
     register unsigned short nc, nj;
     register unsigned int   ni, nm;
     register unsigned int   nr;

     char  ext_str[10], date_str[UTC_STRING_LENGTH];
     int   *ibuff;
     unsigned int count[2];
     char  *cpntr;
     FILE  *outfl;
/*
 * set variable source (= type of MDS)
 */
     const int source = (int) mds->type_mds;
/*
 * create unique output file
 */
     if (source == SCIA_NADIR)
	  (void) snprintf(ext_str, 10, "nadir_%02u", mds->state_index % 100);
     else if (source == SCIA_LIMB)
	  (void) snprintf(ext_str, 10, "limb_%02u", mds->state_index % 100);
     else if (source == SCIA_MONITOR)
	  (void) snprintf(ext_str, 10, "moni_%02u", mds->state_index % 100);
     else if (source == SCIA_OCCULT)
	  (void) snprintf(ext_str, 10, "occult_%02u", mds->state_index % 100);
     else
	  NADC_RETURN_ERROR(NADC_ERR_PDS_KEY, "unknown MDS type");
     
     cpntr = nadc_get_param_string("outfile");
     if ((outfl = CRE_ASCII_File(cpntr, ext_str)) == NULL || IS_ERR_STAT_FATAL)
	  NADC_RETURN_ERROR(NADC_ERR_FILE_CRE, cpntr);
     free(cpntr);
/*
 * write ASCII dump of MDS record
 */
     cpntr = nadc_get_param_string("infile");
     if (source == SCIA_NADIR)
	  nadc_write_header(outfl, 0, cpntr, "Nadir Measurements");
     else if (source == SCIA_LIMB)
	  nadc_write_header(outfl, 0, cpntr, "Limb Measurements");
     else if (source == SCIA_OCCULT)
	  nadc_write_header(outfl, 0, cpntr, "Occultaticpntron Measurements");
     else
	  nadc_write_header(outfl, 0, cpntr, "Monitoring Measurements");
     free(cpntr);

     for (nm = 0; nm < num_mds; nm++, mds++) {
	  nr = 0;
	  (void) MJD_2_ASCII(mds->mjd.days, mds->mjd.secnd,
			      mds->mjd.musec, date_str);
	  nadc_write_text(outfl, ++nr, "DSR date", date_str);
	  nadc_write_uint(outfl, ++nr, "DSR length", mds->dsr_length);
	  nadc_write_schar(outfl, ++nr, "Quality indicator",
			     mds->quality_flag);
	  count[0] = SCIENCE_CHANNELS;
	  nadc_write_arr_uchar(outfl, ++nr, "Scale factor", 
				 1, count, mds->scale_factor);
	  count[0] = mds->n_aux;
	  nadc_write_arr_uchar(outfl, ++nr, "Saturation flags", 
				 1, count, mds->sat_flags);
	  count[0] = mds->n_clus * mds->n_aux;
	  nadc_write_arr_uchar(outfl, ++nr, "Red grass flags", 
				 1, count, mds->red_grass);
/*
 * write geolocation for mds measurements
 */
	  switch (source) {
	  case SCIA_NADIR:
	       SCIA_LV1_WR_ASCII_GEON(outfl, mds->n_aux, mds->geoN);
	       break;
	  case SCIA_LIMB:
	  case SCIA_OCCULT:
	       SCIA_LV1_WR_ASCII_GEOL(outfl, mds->n_aux, mds->geoL);
	       break;
	  case SCIA_MONITOR:
	       SCIA_LV1_WR_ASCII_GEOC(outfl, mds->n_aux, mds->geoC);
	       break;
	  }
/*
 * end of geolocation data block
 */
	  if (source != SCIA_MONITOR) {
	       count[0] = mds->n_pmd;
	       nadc_write_arr_float(outfl, ++nr, "Integrated PMD values", 
				      1, count, 4, mds->int_pmd);
	  }
/*
 * write fractional polarisation values
 */
	  if (source != SCIA_MONITOR)
	       SCIA_LV1_WR_ASCII_POLV(outfl, mds->n_pol, mds->polV);
/*
 * write reticon detector data
 */
	  nr++;
	  for (nc = 0; nc < mds->n_clus; nc++) {
	       nadc_write_ushort(outfl, nr, "clusterID", nc+1);
	       if (mds->clus[nc].n_sig > 0u) {
		    count[0] = 3;
		    count[1] = mds->clus[nc].n_sig;
		    ibuff = (int *) malloc((size_t) (count[0] * count[1])
					    * sizeof(int));
		    if (ibuff == NULL) 
			 NADC_GOTO_ERROR(NADC_ERR_ALLOC, "ibuff");
		    for (ni=0, nj=0; nj < mds->clus[nc].n_sig; nj++) {
			 ibuff[ni++] = (int) mds->clus[nc].sig[nj].corr;
			 ibuff[ni++] = (int) mds->clus[nc].sig[nj].sign;
			 ibuff[ni++] = (int) mds->clus[nc].sig[nj].stray;
		    }
		    nadc_write_arr_int(outfl, nr, "Sig", 2, count, ibuff);
		    free(ibuff);
	       } else if (mds->clus[nc].n_sigc > 0u) {
		    count[0] = 3;
		    count[1] = mds->clus[nc].n_sigc;
		    ibuff = (int *) malloc((size_t) (count[0] * count[1])
					    * sizeof(int));
		    if (ibuff == NULL) 
			 NADC_GOTO_ERROR(NADC_ERR_ALLOC, "ibuff");
		    for (ni=0, nj=0; nj < mds->clus[nc].n_sigc; nj++) {
			 ibuff[ni++] = (int) 
			      mds->clus[nc].sigc[nj].det.field.corr;
			 ibuff[ni++] = (int) 
			      mds->clus[nc].sigc[nj].det.field.sign;
			 ibuff[ni++] = (int) 
			      mds->clus[nc].sigc[nj].stray;
		    }
		    nadc_write_arr_int(outfl, nr, "Sigc", 2, count, ibuff);
		    free(ibuff);
	       }
	  }
     }
 done:
     (void) fclose(outfl);
     return;
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1C_WR_ASCII_MDS
.PURPOSE    dump -- in ASCII Format -- the MDS (Lv1c) records
.INPUT/OUTPUT
  call as    SCIA_LV1C_WR_ASCII_MDS(num_mds, mds_1c);
     input:  
	     unsigned int num_mds        : number of MDS records
	     struct mds1c_scia *mds_1c   : MDS struct (level 1c)

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1C_WR_ASCII_MDS(unsigned int num_mds,
			    const struct mds1c_scia *mds_1c)
{
     register unsigned int   nm;
     register unsigned int   nr;

     char  ext_str[10], date_str[UTC_STRING_LENGTH];
     unsigned int count[2];
     char  *cpntr;
     FILE  *outfl;
/*
 * set variable source (= type of MDS)
 */
     const int source = (int) mds_1c->type_mds;
/*
 * create unique output file
 */
     if (source == SCIA_NADIR)
	  (void) snprintf(ext_str, 10, "nadir_%02u",
			   mds_1c->state_index % 100);
     else if (source == SCIA_LIMB)
	  (void) snprintf(ext_str, 10, "limb_%02u",
			   mds_1c->state_index % 100);
     else if (source == SCIA_MONITOR)
	  (void) snprintf(ext_str, 10, "moni_%02u",
			   mds_1c->state_index % 100);
     else if (source == SCIA_OCCULT)
	  (void) snprintf(ext_str, 10, "occult_%02u",
			   mds_1c->state_index % 100);
     else
	  NADC_RETURN_ERROR(NADC_ERR_PDS_KEY, "unknown MDS type");
     cpntr = nadc_get_param_string("outfile");
     if ((outfl = CRE_ASCII_File(cpntr, ext_str)) == NULL || IS_ERR_STAT_FATAL)
	  NADC_RETURN_ERROR(NADC_ERR_FILE_CRE, cpntr);
     free(cpntr);
/*
 * write ASCII dump of MDS record
 */
     cpntr = nadc_get_param_string("infile");
     if (source == SCIA_NADIR)
	  nadc_write_header(outfl, 0, cpntr, "Nadir Measurements");
     else if (source == SCIA_LIMB)
	  nadc_write_header(outfl, 0, cpntr, "Limb Measurements");
     else if (source == SCIA_OCCULT)
	  nadc_write_header(outfl, 0, cpntr, "Occultation Measurements");
     else
	  nadc_write_header(outfl, 0, cpntr, "Monitoring Measurements");
     free(cpntr);

     for (nm = 0; nm < num_mds; nm++, mds_1c++) {
	  nr = 0;
	  (void) MJD_2_ASCII(mds_1c->mjd.days, mds_1c->mjd.secnd,
			      mds_1c->mjd.musec, date_str);
	  nadc_write_text(outfl, ++nr, "DSR date", date_str);
	  nadc_write_uint(outfl, ++nr, "DSR length", mds_1c->dsr_length);
	  nadc_write_schar(outfl, ++nr, "Quality indicator",
			     mds_1c->quality_flag);
	  nadc_write_float(outfl, ++nr, "Orbit phase", 
			     6, mds_1c->orbit_phase);
	  nadc_write_uchar(outfl, ++nr, "Measurement category", 
			      mds_1c->category);
	  nadc_write_uchar(outfl, ++nr, "State ID", mds_1c->state_id);
	  nadc_write_uchar(outfl, ++nr, "Cluster ID", mds_1c->clus_id);
	  nadc_write_ushort(outfl, ++nr, "Number of observation", 
			      mds_1c->num_obs);
	  nadc_write_ushort(outfl, ++nr, "Number of pixels", 
			      mds_1c->num_pixels);
	  nadc_write_schar(outfl, ++nr, "Pixel Unit indicator", 
			     mds_1c->rad_units_flag);
	  count[0] = mds_1c->num_pixels;
	  if (mds_1c->pixel_ids != NULL)
	       nadc_write_arr_ushort(outfl, ++nr, "Pixel IDs", 1, count,
				      mds_1c->pixel_ids);
	  if (mds_1c->pixel_wv != NULL)
	       nadc_write_arr_float(outfl, ++nr, "Wavelength", 
				     1, count, 5, mds_1c->pixel_wv);
	  if (mds_1c->pixel_wv_err != NULL)
	       nadc_write_arr_float(outfl, ++nr, "Wavelength error", 
				     1, count, 5, mds_1c->pixel_wv_err);
	  count[1] = mds_1c->num_obs;
	  if (mds_1c->pixel_val != NULL)
	       nadc_write_arr_float(outfl, ++nr, "Signal values", 
				     2, count, 9, mds_1c->pixel_val);
	  if (mds_1c->pixel_err != NULL)
	       nadc_write_arr_float(outfl, ++nr, "Signal error values", 
				     2, count, 9, mds_1c->pixel_err);
/*
 * write geolocation for mds measurements
 */
	  switch (source) {
	  case SCIA_NADIR:
	       if (mds_1c->geoN != NULL)
		    SCIA_LV1_WR_ASCII_GEON(outfl, mds_1c->num_obs, mds_1c->geoN);
	       break;
	  case SCIA_LIMB:
	  case SCIA_OCCULT:
	       if (mds_1c->geoL != NULL)
		    SCIA_LV1_WR_ASCII_GEOL(outfl, mds_1c->num_obs, mds_1c->geoL);
	       break;
	  case SCIA_MONITOR:
	       if (mds_1c->geoC != NULL)
		    SCIA_LV1_WR_ASCII_GEOC(outfl, mds_1c->num_obs, mds_1c->geoC);
	       break;
	  }
     }
/* done: */
     (void) fclose(outfl);
     return;
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1C_WR_ASCII_MDS_PMD
.PURPOSE    dump -- in ASCII Format -- the PMD MDS (Lv1c)
.INPUT/OUTPUT
  call as    SCIA_LV1C_WR_ASCII_MDS_PMD(mds_pmd);
     input:  
	     struct mds1c_pmd  *mds_pmd  : level 1c PMD MDS struct

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1C_WR_ASCII_MDS_PMD(const struct mds1c_pmd *pmd)
{
     register unsigned int   nr;

     char  ext_str[14], date_str[UTC_STRING_LENGTH];
     unsigned int count[2];
     char  *cpntr;
     FILE  *outfl;
/*
 * set variable source (= type of MDS)
 */
     const int source = (int) pmd->type_mds;
/*
 * check number of MDS records
 */
     if (source == SCIA_MONITOR) return;
/*
 * create unique output file
 */
     if (source == SCIA_NADIR)
	  (void) snprintf(ext_str, 14, "nadir_pmd_%02u",
			   pmd->state_index % 100);
     else if (source == SCIA_LIMB)
	  (void) snprintf(ext_str, 14, "limb_pmd_%02u",
			   pmd->state_index % 100);
     else if (source == SCIA_MONITOR)
	  (void) snprintf(ext_str, 14, "moni_pmd_%02u",
			   pmd->state_index % 100);
     else if (source == SCIA_OCCULT)
	  (void) snprintf(ext_str, 14, "occult_pmd_%02u",
			   pmd->state_index % 100);
     else
	  NADC_RETURN_ERROR(NADC_ERR_PDS_KEY, "unknown MDS type");
     cpntr = nadc_get_param_string("outfile");
     if ((outfl = CRE_ASCII_File(cpntr, ext_str)) == NULL || IS_ERR_STAT_FATAL)
	  NADC_RETURN_ERROR(NADC_ERR_FILE_CRE, cpntr);
     free(cpntr);
/*
 * write ASCII dump of MDS record
 */
     cpntr = nadc_get_param_string("infile");
     if (source == SCIA_NADIR)
	  nadc_write_header(outfl, 0, cpntr, "Nadir Measurements");
     else if (source == SCIA_LIMB)
	  nadc_write_header(outfl, 0, cpntr, "Limb Measurements");
     else if (source == SCIA_OCCULT)
	  nadc_write_header(outfl, 0, cpntr, "Occultation Measurements");
     else
	  nadc_write_header(outfl, 0, cpntr, "Monitoring Measurements");
     free(cpntr);

     nr = 0;
     (void) MJD_2_ASCII(pmd->mjd.days, pmd->mjd.secnd,
			 pmd->mjd.musec, date_str);
     nadc_write_text(outfl, ++nr, "DSR date", date_str);
     nadc_write_uint(outfl, ++nr, "DSR length", pmd->dsr_length);
     nadc_write_schar(outfl, ++nr, "Quality indicator",
		       pmd->quality_flag);
     nadc_write_float(outfl, ++nr, "Orbit phase", 6, pmd->orbit_phase);
     nadc_write_uchar(outfl, ++nr, "Measurement category", 
		       pmd->category);
     nadc_write_uchar(outfl, ++nr, "State ID", pmd->state_id);
     nadc_write_ushort(outfl, ++nr, "Duration of Scan Phase", 
			pmd->dur_scan);
     nadc_write_ushort(outfl, ++nr, "Number of integrated PMD values", 
			pmd->num_pmd);
     nadc_write_ushort(outfl, ++nr, "Number of Geolocation records", 
			pmd->num_geo);
     count[0] = pmd->num_pmd;
     nadc_write_arr_float(outfl, ++nr, "Integrated PMD values", 
			   1, count, 5, pmd->int_pmd);
/*
 * write geolocation for PMD MDS measurements
 */
     switch (source) {
     case SCIA_NADIR:
	  SCIA_LV1_WR_ASCII_GEON(outfl, pmd->num_geo, pmd->geoN);
	  break;
     case SCIA_LIMB:
     case SCIA_OCCULT:
	  SCIA_LV1_WR_ASCII_GEOL(outfl, pmd->num_geo, pmd->geoL);
	  break;
     }
     (void) fclose(outfl);
     return;
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1C_WR_ASCII_MDS_POLV
.PURPOSE    dump -- in ASCII Format -- the  POLV MDS (Lv1c)
.INPUT/OUTPUT
  call as    SCIA_LV1C_WR_ASCII_MDS_POLV(mds_polV);
     input:  
	     struct mds1c_polV *mds_polV : level 1c polV MDS struct

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1C_WR_ASCII_MDS_POLV(const struct mds1c_polV *polV)
{
     register unsigned int   nr;

     char  ext_str[15], date_str[UTC_STRING_LENGTH];
     unsigned int count[2];
     char  *cpntr;
     FILE  *outfl;
/*
 * set variable source (= type of MDS)
 */
     const int source = (int) polV->type_mds;
/*
 * check number of MDS records
 */
     if (source == SCIA_MONITOR) return;
/*
 * create unique output file
 */
     if (source == SCIA_NADIR)
	  (void) snprintf(ext_str, sizeof(ext_str), "nadir_polV_%02u",
			   polV->state_index % 100);
     else if (source == SCIA_LIMB)
	  (void) snprintf(ext_str, sizeof(ext_str), "limb_polV_%02u",
			   polV->state_index % 100);
     else if (source == SCIA_OCCULT)
	  (void) snprintf(ext_str, sizeof(ext_str), "occult_polV_%02u",
			   polV->state_index % 100);
     else if (source == SCIA_MONITOR)
	  return;
     else
	  NADC_RETURN_ERROR(NADC_ERR_PDS_KEY, "unknown MDS type");
     cpntr = nadc_get_param_string("outfile");
     outfl = CRE_ASCII_File(cpntr, ext_str);
     if (outfl == NULL || IS_ERR_STAT_FATAL)
	  NADC_RETURN_ERROR(NADC_ERR_FILE_CRE, cpntr);
     free(cpntr);
/*
 * write ASCII dump of MDS record
 */
     cpntr = nadc_get_param_string("infile");
     if (source == SCIA_NADIR)
	  nadc_write_header(outfl, 0, cpntr, "Nadir Measurements");
     else if (source == SCIA_LIMB)
	  nadc_write_header(outfl, 0, cpntr, "Limb Measurements");
     else if (source == SCIA_OCCULT)
	  nadc_write_header(outfl, 0, cpntr, "Occultation Measurements");
     else
	  nadc_write_header(outfl, 0, cpntr, "Monitoring Measurements");
     free(cpntr);
     
     nr = 0;
     (void) MJD_2_ASCII(polV->mjd.days, polV->mjd.secnd,
			 polV->mjd.musec, date_str);
     nadc_write_text(outfl, ++nr, "DSR date", date_str);
     nadc_write_uint(outfl, ++nr, "DSR length", polV->dsr_length);
     nadc_write_schar(outfl, ++nr, "Quality indicator",
		       polV->quality_flag);
     nadc_write_float(outfl, ++nr, "Orbit phase", 6, 
		       polV->orbit_phase);
     nadc_write_uchar(outfl, ++nr, "Measurement category", 
		       polV->category);
     nadc_write_uchar(outfl, ++nr, "State ID", polV->state_id);
     nadc_write_ushort(outfl, ++nr, "Duration of Scan Phase", 
			polV->dur_scan);
     nadc_write_ushort(outfl, ++nr, "Number of Geolocation records", 
			polV->num_geo);
     nadc_write_ushort(outfl, ++nr, 
			"Number of Fractional Polarisation records", 
			polV->total_polV);
     nadc_write_ushort(outfl, ++nr, 
			"Number of different integration times", 
			polV->num_diff_intg);
     count[0] = polV->num_diff_intg;
     nadc_write_arr_ushort(outfl, ++nr, "Integration times", 
			    1, count, polV->intg_times);
     nadc_write_arr_ushort(outfl, ++nr, "Repetition Factors", 
			    1, count, polV->num_polar);
/*
 * write polarisation values
 */
     SCIA_LV1_WR_ASCII_POLV(outfl, polV->total_polV, polV->polV);
/*
 * write geolocation for POLV MDS measurements
 */
     switch (source) {
     case SCIA_NADIR:
	  SCIA_LV1_WR_ASCII_GEON(outfl, polV->num_geo, polV->geoN);
	  break;
     case SCIA_LIMB:
     case SCIA_OCCULT:
	  SCIA_LV1_WR_ASCII_GEOL(outfl, polV->num_geo, polV->geoL);
	  break;
     }
     (void) fclose(outfl);
     return;
}
