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

.IDENTifer   SCIA_LV1_WR_ADS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 2
.LANGUAGE    ANSI C
.PURPOSE     dump Annotation Data Sets in ASCII
.RETURNS     Nothing
.COMMENTS    contains SCIA_OL2_WR_ASCII_SQADS, SCIA_OL2_WR_ASCII_NGEO, 
                SCIA_OL2_WR_ASCII_LGEO
.ENVIRONment None
.VERSION     1.2   07-Dec-2005	write lat/lon values as doubles, RvH
	     1.1   22-Jan-2003	bugfix, errors in the product definition, RvH
             1.0   13-Sep-2001  Created by R. M. van Hees
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
.IDENTifer  SCIA_OL2_WR_ASCII_SQADS
.PURPOSE    dump -- in ASCII Format -- the SQADS annotation dataset
.INPUT/OUTPUT
  call as   SCIA_OL2_WR_ASCII_SQADS(num_dsr, sqads);
     input:
	    unsigned int num_dsr      : number of data sets
	    struct sqads_sci_ol *sqads: pointer to SQADS records
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_OL2_WR_ASCII_SQADS(unsigned int num_dsr,
			     const struct sqads_sci_ol *sqads)
{
     register unsigned int nd, nr;

     char date_str[UTC_STRING_LENGTH];
     unsigned int  count[2];

     char *cpntr;
     FILE *outfl;

     cpntr = nadc_get_param_string("outfile");
     if ((outfl = CRE_ASCII_File(cpntr, "sqads")) == NULL || IS_ERR_STAT_FATAL)
	  NADC_RETURN_ERROR(NADC_ERR_FILE_CRE, cpntr);
     free(cpntr);
/*
 * write ASCII dump of SQADS record
 */
     cpntr = nadc_get_param_string("infile");
     nadc_write_header(outfl, 0, cpntr, "Summary of Quality Flags per State");
     free(cpntr);
     for (nd = 0; nd < num_dsr; nd++) {
	  nr = 1;
	  (void) MJD_2_ASCII(sqads[nd].mjd.days, sqads[nd].mjd.secnd,
			      sqads[nd].mjd.musec, date_str);
	  nadc_write_text(outfl, nr++, "starttime", date_str);
	  nadc_write_uchar(outfl, nr++, "attached", sqads[nd].flag_mds);
	  count[0] = OL2_SQADS_PQF_FLAGS;
	  nadc_write_arr_uchar(outfl, nr++, "quality", 
				 1, count, sqads[nd].flag_pqf);
     }
     (void) fclose(outfl);
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_OL2_WR_ASCII_NGEO
.PURPOSE    dump -- in ASCII Format -- the NADIR GEO annotation dataset
.INPUT/OUTPUT
  call as   SCIA_OL2_WR_ASCII_NGEO(num_dsr, geo);
     input:
	    unsigned int num_dsr      : number of data sets
	    struct ngeo_scia *geo     : pointer to Geolocation (NADIR) records
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_OL2_WR_ASCII_NGEO(unsigned int num_dsr, const struct ngeo_scia *geo)
{
     register unsigned int nd, ni, nr, ny;

     char   date_str[UTC_STRING_LENGTH];
     double rbuff[2 * NUM_CORNERS];
     unsigned int  count[2];

     char *cpntr;
     FILE *outfl;

     cpntr = nadc_get_param_string("outfile");
     if ((outfl = CRE_ASCII_File(cpntr, "ngeo")) == NULL || IS_ERR_STAT_FATAL)
	  NADC_RETURN_ERROR(NADC_ERR_FILE_CRE, cpntr);
     free(cpntr);
/*
 * write ASCII dump of NGEO record
 */
     cpntr = nadc_get_param_string("infile");
     nadc_write_header(outfl, 0, cpntr, "Nadir Geolocation Data set(s)");
     free(cpntr);
     for (nd = 0; nd < num_dsr; nd++) {
	  nr = 0;
	  (void) MJD_2_ASCII(geo[nd].mjd.days, geo[nd].mjd.secnd,
			      geo[nd].mjd.musec, date_str);
	  nadc_write_text(outfl, ++nr, "starttime", date_str);
	  nadc_write_uchar(outfl, ++nr, "attached", 
			    geo[nd].flag_mds);
	  nadc_write_uchar(outfl, ++nr, "pixelType", 
			    geo[nd].pixel_type);
	  nadc_write_ushort(outfl, ++nr, "inttime",
			      geo[nd].intg_time);
	  count[0] = 3;
	  nadc_write_arr_float(outfl, ++nr, "solarzen",
			       1, count, 6, geo[nd].sun_zen_ang);
	  nadc_write_arr_float(outfl, ++nr, "loszen",
			       1, count, 6, geo[nd].los_zen_ang);
	  nadc_write_arr_float(outfl, ++nr, "relazi",
			       1, count, 6, geo[nd].rel_azi_ang);
	  nadc_write_float(outfl, ++nr, "height", 8, geo[nd].sat_h);
	  nadc_write_float(outfl, ++nr, "radius", 8, geo[nd].radius);
	  count[0] = 2;
	  count[1] = NUM_CORNERS;
	  rbuff[0] = geo[nd].subsat.lat / 1e6;
	  rbuff[1] = geo[nd].subsat.lon / 1e6;
	  nadc_write_arr_double(outfl, ++nr, "subsat",
				 1, count, 6, rbuff);

	  for (ni = ny = 0; ny < count[1]; ny++) {
	       rbuff[ni++] = geo[nd].corner[ny].lat / 1e6;
	       rbuff[ni++] = geo[nd].corner[ny].lon / 1e6;
	  }
	  nadc_write_arr_double(outfl, ++nr, "corners",
				 2, count, 6, rbuff);
	  
	  rbuff[0] = geo[nd].center.lat / 1e6;
	  rbuff[1] = geo[nd].center.lon / 1e6;
	  nadc_write_arr_double(outfl, ++nr, "center",
				 1, count, 6, rbuff);
     }
     (void) fclose(outfl);
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_OL2_WR_ASCII_LGEO
.PURPOSE    dump -- in ASCII Format -- the LIMB GEO annotation dataset
.INPUT/OUTPUT
  call as   SCIA_OL2_WR_ASCII_LGEO(num_dsr, geo);
     input:
	    unsigned int num_dsr      : number of data sets
	    struct lgeo_scia *geo     : pointer to Geolocation (LIMB) records
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_OL2_WR_ASCII_LGEO(unsigned int num_dsr, const struct lgeo_scia *geo)
{
     register unsigned int nd, ni, nr, ny;

     char   date_str[UTC_STRING_LENGTH];
     double rbuff[2 * 3];
     unsigned int  count[2];

     char *cpntr;
     FILE *outfl;

     cpntr = nadc_get_param_string("outfile");
     if ((outfl = CRE_ASCII_File(cpntr, "lgeo")) == NULL || IS_ERR_STAT_FATAL)
	  NADC_RETURN_ERROR(NADC_ERR_FILE_CRE, cpntr);
     free(cpntr);
/*
 * write ASCII dump of LGEO record
 */
     cpntr = nadc_get_param_string("infile");
     nadc_write_header(outfl, 0, cpntr, "Limb Geolocation Data set(s)");
     free(cpntr);
     for (nd = 0; nd < num_dsr; nd++) {
	  nr = 0;
	  (void) MJD_2_ASCII(geo[nd].mjd.days, geo[nd].mjd.secnd,
			      geo[nd].mjd.musec, date_str);
	  nadc_write_text(outfl, ++nr, "starttime", date_str);
	  nadc_write_uchar(outfl, ++nr, "attached", 
			     geo[nd].flag_mds);
	  nadc_write_ushort(outfl, ++nr, "inttime",
			      geo[nd].intg_time);
	  count[0] = 3;
	  nadc_write_arr_float(outfl, ++nr, "solarzen",
				 1, count, 6, geo[nd].sun_zen_ang);
	  nadc_write_arr_float(outfl, ++nr, "loszen",
				 1, count, 6, geo[nd].los_zen_ang);
	  nadc_write_arr_float(outfl, ++nr, "relazi",
				 1, count, 6, geo[nd].rel_azi_ang);
	  nadc_write_float(outfl, ++nr, "height", 8, geo[nd].sat_h);
	  nadc_write_float(outfl, ++nr, "radius", 8, geo[nd].radius);
	  count[0] = 2;
	  count[1] = 3;
	  rbuff[0] = geo[nd].subsat.lat / 1e6;
	  rbuff[1] = geo[nd].subsat.lon / 1e6;
	  nadc_write_arr_double(outfl, ++nr, "subsat",
				 1, count, 6, rbuff);

	  for (ni = ny = 0; ny < count[1]; ny++) {
	       rbuff[ni++] = geo[nd].tang[ny].lat / 1e6;
	       rbuff[ni++] = geo[nd].tang[ny].lon / 1e6;
	  }
	  nadc_write_arr_double(outfl, ++nr, "tanggrdpoint",
				 2, count, 6, rbuff);
	  count[0] = 3;
	  nadc_write_arr_float(outfl, ++nr, "tangheight", 
				 1, count, 8, geo[nd].tan_h);
     }
     (void) fclose(outfl);
}

