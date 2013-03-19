/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2003 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV1_PATCH_PPG
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIAMACHY
.LANGUAGE    ANSI C
.PURPOSE     copy/patch PDS annotation datasets between files

.INPUT/OUTPUT
  call as   SCIA_LV1_PATCH_PPG( patch_scia, num_dsd, dsd, fp_in, fp_out );
     input:
            unsigned short  patch_scia :  flag defining which patches to apply
            unsigned int num_dsd       :  number of DSDs
            struct dsd_envi *dsd       :  structure for the DSDs
	    FILE            *fp_in     :  file-pointer to input file
	    FILE            *fp_out    :  file-pointer to output file

.RETURNS     nothing
             error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     2.0     07-Oct-2008   update BDPM to SDMF v3.0, RvH
             1.0     24-Apr-2005   initial release by R. M. van Hees
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
#include <glob.h>

#include <hdf5.h>
#include <hdf5_hl.h>

/*+++++ Local Headers +++++*/
#define _SCIA_PATCH_1
#include <nadc_sdmf.h>

/*+++++ Macros +++++*/
#define FIRST_VALID_SDMF_BDPM 3899

/*+++++ Static Variables +++++*/
static const char sdmf_ppg_db[] = "/SCIA/share/SDMF/3.0/sdmf_ppg.h5";

/*+++++ Global Variables +++++*/
        /* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   getPPG_SRON
.PURPOSE     obtain Pixel-to-Pixel Gain factors from SRON Monitoring database
.INPUT/OUTPUT
  call as    getPPG_SRON( orbit, pixelGain );
     input:
           int orbit         :  absolute orbit number
    output:
	   float *pixelGain  :  Pixel-to-Pixel Gain factors

.RETURNS     flag: FALSE (no PPG found) or TRUE
.COMMENTS    none
-------------------------*/
static
bool getPPG_SRON( int orbit, /*@out@*/ float *pixelGain )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, pixelGain@*/
{
     const char prognm[] = "getPPG_SRON";

     register int delta = 0;

     char   str_msg[SHORT_STRING_LENGTH];

     bool   found = FALSE;

     int    numIndx, metaIndx;

     hid_t  fid = -1;
/*
 * initialise output arrays
 */
     (void) memset( pixelGain, 0, sizeof(float) * (size_t) SCIENCE_PIXELS );
/*
 * open output HDF5-file
 */
     if ( (fid = H5Fopen( sdmf_ppg_db, H5F_ACC_RDONLY, H5P_DEFAULT )) < 0 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, sdmf_ppg_db );
/*
 * find PPG values, requirements:
 *  - orbit number within a range MAX_DiffOrbitNumber
 */
     metaIndx = -1;
     do {
	  numIndx = 1;
	  (void) SDMF_get_metaIndex(fid, orbit + delta, &numIndx, &metaIndx);
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "SDMF_get_metaIndex" );
	  if ( metaIndx >= 0 ) {
	       found = TRUE;
	  } else {
	       delta = (delta > 0) ? (-delta) : (1 - delta);
	       if ( abs( delta ) > MAX_DiffOrbitNumber ) goto done;
	  }
     } while ( ! found );

     SDMF_rd_float_Array( fid, "pixelGain", 1, &metaIndx, NULL, pixelGain );

     (void) snprintf( str_msg, SHORT_STRING_LENGTH, 
		      "patched PPG data: 1-8[%-d]", orbit + delta );
     NADC_ERROR( prognm, NADC_ERR_NONE, str_msg );
/*
 * close SDMF pixelMask database
 */
 done:
     if ( fid != -1 ) (void) H5Fclose( fid );

     return found;
}

/*+++++++++++++++++++++++++
.IDENTifer   getBadPixelMaskSRON
.PURPOSE     obtain bad/dead pixel mask from SRON Monitoring database
.INPUT/OUTPUT
  call as    getBadPixelMaskSRON( orbit, bdpm );
     input:
           int orbit             :  absolute orbitnumber
    output:
	   unsigned short *bdpm  :  bad/death pixel mask (1 = bad/death)

.RETURNS     flag: FALSE (no mask found) or TRUE
.COMMENTS    none
-------------------------*/
static
bool getBadPixelMaskSRON( int orbit, /*@out@*/ unsigned short *bdpm )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, bdpm@*/
{
     const char prognm[] = "getBadPixelMaskSRON";

     const char sdmf_bdpm_db[] = "/SCIA/share/SDMF/3.0/sdmf_pixelmask.h5";

     const int  pixelRange[] = {VIS_SCIENCE_PIXELS, SCIENCE_PIXELS-1};

     register int delta = 0;

     char   str_msg[SHORT_STRING_LENGTH];

     bool   found = FALSE;

     int    numIndx, metaIndx;

     hid_t  fid = -1;
     hid_t  gid = -1;
/*
 * initialise output arrays
 */
     (void) memset( bdpm, 0, 3 * sizeof(short) * (size_t) CHANNEL_SIZE );
/*
 * open output HDF5-file
 */
     if ( (fid = H5Fopen( sdmf_bdpm_db, H5F_ACC_RDONLY, H5P_DEFAULT )) < 0 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, sdmf_bdpm_db );
     if ( (gid = H5Gopen( fid, "/smoothMask", H5P_DEFAULT )) < 0 ) {
          NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_GRP, "/smoothMask" );
     }
/*
 * find masker with dead/bad pixel mask, requirements:
 *  - orbit number within a range MAX_DiffOrbitNumber
 */
     metaIndx = -1;
     do {
	  numIndx = 1;
	  (void) SDMF_get_metaIndex(gid, orbit + delta, &numIndx, &metaIndx);
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "SDMF_get_metaIndex" );
	  if ( metaIndx >= 0 ) {
	       found = TRUE;
	  } else {
	       delta = (delta > 0) ? (-delta) : (1 - delta);
	       if ( abs( delta ) > MAX_DiffOrbitNumber ) goto done;
	  }
     } while ( ! found );

     SDMF_rd_uint16_Array( gid, "combined", 1, &metaIndx, 
			     pixelRange, bdpm );

     (void) snprintf( str_msg, SHORT_STRING_LENGTH, 
		      "patched BPDM channel: 6-8[%-d]", orbit + delta );
     NADC_ERROR( prognm, NADC_ERR_NONE, str_msg );
/*
 * close SDMF pixelMask database
 */
 done:
     if ( gid != -1 ) (void) H5Gclose( gid );
     if ( fid != -1 ) (void) H5Fclose( fid );

     return found;
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_PATCH_PPG( unsigned short patch_scia, int orbit,
			 unsigned int num_dsd, const struct dsd_envi *dsd, 
			 FILE *fp_in, FILE *fp_out )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp_in, fp_out@*/
{
     const char prognm[] = "SCIA_LV1_PATCH_PPG";

     struct ppg_scia ppg;
/*
 * read (G)ADS
 */
     (void) SCIA_LV1_RD_PPG( fp_in, num_dsd, dsd, &ppg );
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "PPG" );
/*
 * patch Pixel-to-Pixel gain
 */
     if ( (patch_scia & SCIA_PATCH_PPG) != USHRT_ZERO ) {
	  float ppg_fact[SCIENCE_PIXELS];

	  const size_t nr_byte = SCIENCE_PIXELS * ENVI_FLOAT;

	  if ( getPPG_SRON( orbit, ppg_fact ) )
	       (void) memcpy( ppg.ppg_fact, ppg_fact, nr_byte );
     }
/*
 * patch dead/bad pixel mask
 */
     if ( (patch_scia & SCIA_PATCH_BDPM) != USHRT_ZERO ) {
	  register unsigned short np;

	  unsigned short bdpm[3 * CHANNEL_SIZE];

	  if ( orbit < FIRST_VALID_SDMF_BDPM ) orbit = FIRST_VALID_SDMF_BDPM;
	  if ( getBadPixelMaskSRON( orbit, bdpm ) ) {
	       unsigned char *pntr_bdmp = ppg.bad_pixel + VIS_SCIENCE_PIXELS;

	       for ( np = 0; np < IR_SCIENCE_PIXELS; np++ )
		    *pntr_bdmp++ = (unsigned char) bdpm[np];
	  }
     }
/*
 * write (G)ADS
 */
     (void) SCIA_LV1_WR_PPG( fp_out, 1, ppg );
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "PPG" );
}
