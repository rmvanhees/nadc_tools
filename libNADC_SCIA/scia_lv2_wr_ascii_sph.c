/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 1999 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV2_WR_ASCII_SPH
.AUTHOR      R.M. van Hees
.KEYWORDS    PDS SCIAMACHY headers
.LANGUAGE    ANSI C
.PURPOSE     Dump Specific Product Header of the level 2 product
.INPUT/OUTPUT
  call as    SCIA_LV2_WR_ASCII_SPH( param, sph );
     input:  
             struct param_record param  : struct holding user-defined settings
	     struct sph2_scia *sph      : pointer to SPH structure

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     4.1     07-Dec-2005   write lat/lon values as doubles, RvH
             4.0     23-Aug-2001   Move to seperate module, RvH
             3.0     03-Jan-2001   Split the module "write_ascii", RvH
             2.2     21-Dec-2000   Added SCIA_LV1_WR_ASCII_NADIR, RvH
             2.1     20-Dec-2000   Use output filename given by the user, RvH
             2.0     17-Aug-2000   Major rewrite and standardization, RvH
             1.1     14-Jul-2000   Renamed: DEBUG -> SCIA_LV1_WR_ASCII, RvH
             1.0     02-Mar-1999   Created by R. M. van Hees
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
#define _SCIA_LEVEL_2
#include <nadc_scia.h>

#define MX_SZ_CBUFF   25

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV2_WR_ASCII_SPH( struct param_record param, 
			    const struct sph2_scia *sph )
{
     register unsigned int  ni, nr = 0;

     char           cbuff[MX_SZ_CBUFF];
     unsigned short ubuff[8];
     unsigned int   ndim;
     unsigned int   dims[2];

     const char prognm[] = "SCIA_LV2_WR_ASCII_SPH";

     FILE *outfl = CRE_ASCII_File( param.outfile, "sph" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of SPH record
 */
     nadc_write_header( outfl, nr, param.infile, 
			 "Specific Product Header of Level 2 Product" );
     nadc_write_text( outfl, ++nr, "SPH_DESCRIPTOR", sph->descriptor );
     nadc_write_short( outfl, ++nr, "STRIPLINE_CONTINUITY_INDICATOR", 
		       sph->stripline );
     nadc_write_short( outfl, ++nr, "SLICE_POSITION", sph->slice_pos );
     nadc_write_ushort( outfl, ++nr, "NUM_SLICES", sph->no_slice );
     nadc_write_text( outfl, ++nr, "START_TIME", sph->start_time );
     nadc_write_text( outfl, ++nr, "STOP_TIME", sph->stop_time );
     nadc_write_double( outfl, ++nr, "START_LAT", 6, sph->start_lat );
     nadc_write_double( outfl, ++nr, "START_LON", 6, sph->start_lon );
     nadc_write_double( outfl, ++nr, "STOP_LAT", 6, sph->stop_lat );
     nadc_write_double( outfl, ++nr, "STOP_LON", 6, sph->stop_lon );
     ++nr;    /* spare_1 */
     nadc_write_text( outfl, ++nr, "FITTING_ERROR_SUM", sph->fit_error );
     nadc_write_ushort( outfl, ++nr, "NO_OF_DOAS_FITTING_WIN", 
			 sph->no_doas_win );
     ndim = 2;
     for ( ni = 0; ni < sph->no_doas_win; ni++ ) {
	  (void) snprintf(cbuff, MX_SZ_CBUFF, "DOAS_FITTING_WINDOW_%02u", ni);
	  ubuff[0] = sph->doas_win[ni].wv_min;
	  ubuff[1] = sph->doas_win[ni].wv_max;
	  nadc_write_arr_ushort( outfl, ++nr, cbuff, 1, &ndim, ubuff );
     }
     nr += (MAX_DOAS_FITTING_WIN - sph->no_doas_win);
     nadc_write_ushort( outfl, ++nr, "NO_OF_BIAS_FITTING_WIN", 
			 sph->no_bias_win );
     for ( ni = 0; ni < sph->no_bias_win; ni++ ) {
	  (void) snprintf(cbuff, MX_SZ_CBUFF, "BIAS_FITTING_WINDOW_%02u", ni);
	  ubuff[0] = sph->bias_win[ni].wv_min;
	  ubuff[1] = sph->bias_win[ni].wv_max;
	  if ( sph->bias_win[ni].nr_micro == 0 )
	       nadc_write_arr_ushort( outfl, ++nr, cbuff, 1, &ndim, ubuff );
	  else {
	       register unsigned int nm, nb;

	       dims[0] = 2;
	       dims[1] = sph->bias_win[ni].nr_micro+1;
	       for ( nb = 2, nm = 0; nm < sph->bias_win[ni].nr_micro; nm++ ) {
		    ubuff[nb++] = sph->bias_win[ni].micro_min[nm];
		    ubuff[nb++] = sph->bias_win[ni].micro_max[nm];
	       }
	       nadc_write_arr_ushort( outfl, ++nr, cbuff, 2, dims, ubuff );
	  }
     }
     nadc_write_ushort( outfl, ++nr, "NO_OF_DOAS_MOL",  sph->no_doas_mol );
     for ( ni = 0; ni < MAX_DOAS_SPECIES; ni++ ) {
	  (void) snprintf( cbuff, MX_SZ_CBUFF, "DOAS_MOLECULE_%02u", ni );
          nadc_write_text( outfl, ++nr, cbuff, sph->doas_mol[ni] );
     }
     nadc_write_ushort( outfl, ++nr, "NO_OF_BIAS_MOL", sph->no_bias_mol );
     for ( ni = 0; ni < sph->no_bias_mol; ni++ ) {
	  (void) snprintf( cbuff, MX_SZ_CBUFF, "BIAS_MOLECULE_%02u", ni );
          nadc_write_text( outfl, ++nr, cbuff, sph->bias_mol[ni] );
     }
     (void) fclose( outfl );
}
