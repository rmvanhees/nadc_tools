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

.IDENTifer   GOME_LV1_BDR
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME level 1
.LANGUAGE    ANSI C
.PURPOSE     calibrate and write selected Earth/Moon/Sun Band Data Records
.INPUT/OUTPUT
  call as   PROCESS_PCD_BDR( nband, param, fsr, fcd, nr_pcd, indx_pcd, pcd, rec );
     input:  
            short  nband              : number of spectral band [1a=0,1b,2a..]
	    struct param_record param : command-line parameters
	    struct fsr1_gome *fsr  : structure for File Structure Record
	    struct fcd_gome  *fcd  : structure for Fixed Calibration Record
	    short nr_pcd           : number of Pixel Calibration Records
	    short *indx_pcd        : indices to selected PCDs
	    struct pcd_gome  *pcd  : structure for Pixel Calibration Records
	    struct rec_gome  *rec  : Spectral Band Records

  call as   PROCESS_SMCD_BDR( flag_origin, nband, param, fcd, nr_scd, indx_scd, scd, rec );
     input:  
            unsigned char flag_origin : Sun or Moon measurements
            short  nband              : number of spectral band [1a=0,1b,2a..]
	    struct param_record param : command-line parameters
	    struct fcd_gome   *fcd  : Fixed Calibration Record
	    short nr_smcd           : number of Pixel Calibration Records
	    short *indx_scd         : indices to selected PCDs
	    struct smcd_gome *scd   : Sun/Moon Specific Calibration Records
	    struct rec_gome   *rec  : Spectral Band Records

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      4.0   11-Nov-2001	moved to the new Error handling routines, RvH
              3.2   05-Sep-2001	compiles without HDF5 library, RvH 
              3.1   26-Jul-2001 combined PROCESS_PCD_BDR & PROCESS_SMCD_BDR 
              3.0   26-Jul-2001 rewrite to get calibration correct, RvH
              2.0   18-May-2001 bugs in pixel & wavelength selection, RvH
              1.0   02-Mar-2000 created by R. M. van Hees
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

#include <hdf5.h>

/*+++++ Local Headers +++++*/
#define _GOME_COMMON
#include <nadc_gome.h>

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#include "eval_poly.inc"

static
short SELECT_PCD_REC( short nr_pcd, const short *indx_pcd, 
		      short nr_rec, 
                      /*@unique@*/ const struct rec_gome *rec, 
		      /*@out@*/ struct rec_gome *rec_out )
{
     register short nl = 0, np = 0, nr = 0;

     do {
	  while ( np < nr_pcd && rec[nr].indx_pcd > indx_pcd[np] ) np++;
	  if ( np >= nr_pcd ) break;
	  if ( rec[nr].indx_pcd == indx_pcd[np] ) {
	       (void) memcpy( rec_out+nl, rec+nr, sizeof( struct rec_gome ));
	       nl++;
	  }
     } while ( ++nr < nr_rec );
     return nl;
}

static
short SELECT_SMCD_REC( short nband, short nr_smcd, const short *indx_smcd,
		       const struct smcd_gome *smcd,
		       /*@unique@*/ const struct rec_gome *rec,
		       /*@out@*/ struct rec_gome *rec_out )
{
     register short ni, nl = 0, nr = 0;

     do {
	  if ( (ni = smcd[indx_smcd[nr]].indx_bands[nband]) != -1 ) {
	       (void) memcpy( rec_out+nl, rec+ni, sizeof( struct rec_gome ));
	       nl++;
	  }
     } while ( ++nr < nr_smcd );

     return nl;
}

static inline
void Add_Wavelength( short channel, const struct lv1_spec spec, 
                     /*@out@*/ float wave[] )
{
     register short  nr = 0;

     do {
	  wave[nr] = (float) Eval_Poly( nr, spec.coeffs[channel] );
     } while ( ++nr < CHANNEL_SIZE );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void PROCESS_PCD_BDR( short nband, struct param_record param, 
		      const struct fsr1_gome *fsr, 
		      const struct fcd_gome *fcd, 
		      short nr_pcd, const short *indx_pcd, 
		      const struct pcd_gome *pcd,
		      const struct rec_gome *rec )
{
     register short nr;

     short  nr_rec = 0;
     short  wv_start, wv_count;

     struct rec_gome *rec_out;

     const short channel  = fcd->bcr[nband].array_nr - 1;
/*
 * initialize
 */
     if ( nr_pcd == 0 || indx_pcd == NULL || pcd == NULL || rec == NULL ) 
	  return;
/*
 * select requested data records
 */
     rec_out = (struct rec_gome *) 
          malloc( (size_t) nr_pcd * sizeof( struct rec_gome ));
     if ( rec_out == NULL ) 
	  NADC_RETURN_ERROR( NADC_ERR_ALLOC, "rec_out" );

     nr_rec = SELECT_PCD_REC( nr_pcd, indx_pcd, nr_rec, rec, rec_out );
     if ( nr_rec == 0 ) {
	  free( rec_out );
	  return;
     }
/*
 * add wavelength of detector data
 */
     nr = 0;
     do {
	  Add_Wavelength( channel,
			  fcd->spec[pcd[rec_out[nr].indx_pcd].indx_spec], 
			  &rec_out[nr].wave[0] );
     } while ( ++nr < nr_rec );
/*
 * select wavelength interval
 */
     wv_start = fcd->bcr[nband].start;
     wv_count = fcd->bcr[nband].end - fcd->bcr[nband].start + 1;
     if ( param.flag_wave == PARAM_SET ){
	  register short wv_min = fcd->bcr[nband].start;
	  register short wv_max = fcd->bcr[nband].end;

	  nr = 0;
	  do {
	       while ( wv_min <= wv_max 
		       && rec_out[nr].wave[wv_min] < param.wave[0] )
		    wv_min++;
	       while ( wv_max >= wv_min
		       && rec_out[nr].wave[wv_max] > param.wave[1] )
		    wv_max--;
	  } while ( ++nr < nr_rec );
/*
 * return when no data is selected
 */
	  if ( wv_min > wv_max ) {
	       free( rec_out );
	       return;
	  }
	  wv_start = wv_min;
	  wv_count = wv_max - wv_min + 1;
     }
/*
 * calibrate the data
 */
     switch ( nband ) {
     case BAND_1a:
     case BAND_1b:
     case BAND_2a:
     case BAND_2b:
     case BAND_3:
     case BAND_4:
	  CALIB_PCD_BDR( param.calib_earth, nband, fcd, fsr->nr_pcd, pcd, 
			 nr_rec, rec_out );
	  break;
     default:
	  break;
     }
/*
 * write data to HDF5 file
 */
     if ( param.write_hdf5 == PARAM_SET )
	  GOME_LV1_WR_H5_REC( FLAG_EARTH, nband, param, 
			      nr_rec, wv_start, wv_count, rec_out );
/*
 * write data to ASCII file
 */
     if ( param.write_ascii == PARAM_SET )
	  GOME_LV1_WR_ASCII_REC( FLAG_EARTH, nband, param, 
				 nr_rec, wv_start, wv_count, rec_out );
/*
 * free allocated memory
 */
     free( rec_out );
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void PROCESS_SMCD_BDR( unsigned char flag_origin, short nband, 
		       struct param_record param, 
		       const struct fcd_gome *fcd, 
		       short nr_smcd, const short *indx_smcd,
		       const struct smcd_gome *smcd,
		       const struct rec_gome *rec )
{
     register short nr, num;

     short  nr_rec, wv_start, wv_count;

     struct rec_gome *rec_out;

     const short  channel = fcd->bcr[nband].array_nr - 1;
     const size_t size_rec = sizeof( struct rec_gome );
/*
 * initialize
 */
     if ( nr_smcd == 0 || smcd == NULL ) return;
/*
 * 
 */
     rec_out = (struct rec_gome *) malloc( (size_t) nr_smcd * size_rec );
     if ( rec_out == NULL ) NADC_RETURN_ERROR( NADC_ERR_ALLOC, "rec_out" );
/*
 * select requested data records
 */
     nr_rec = SELECT_SMCD_REC( nband, nr_smcd, indx_smcd, smcd, rec, rec_out );
     if ( nr_rec == 0 ) {
	  free( rec_out );
	  return;
     }
/*
 * add wavelength of detector data
 */
     nr = 0;
     do {
	  Add_Wavelength( channel, fcd->spec[smcd[indx_smcd[nr]].indx_spec],
			  &rec_out[nr].wave[0] );
     } while ( ++nr < nr_smcd );
/*
 * select wavelength interval
 */
     wv_start = fcd->bcr[nband].start;
     wv_count = fcd->bcr[nband].end - fcd->bcr[nband].start + 1;
     if ( param.flag_wave == PARAM_SET ){
	  register short wv_min = fcd->bcr[nband].start;
	  register short wv_max = fcd->bcr[nband].end;

	  nr = 0;
	  do {
	       while ( wv_min <= wv_max 
		       && rec_out[nr].wave[wv_min] < param.wave[0] )
		    wv_min++;
	       while ( wv_max >= wv_min
		       && rec_out[nr].wave[wv_max] > param.wave[1] )
		    wv_max--;
	  } while ( ++nr < nr_smcd );
/*
 * return when no data is selected
 */
	  if ( wv_min > wv_max ) {
	       free( rec_out );
	       return;
	  }
	  wv_start = wv_min;
	  wv_count = wv_max - wv_min + 1;
     }
/*
 * calibrate the data
 */
     switch ( nband ) {
     case BAND_1a:
     case BAND_1b:
     case BAND_2a:
     case BAND_2b:
     case BAND_3:
     case BAND_4:
	  if ( flag_origin == FLAG_MOON 
	       && param.calib_moon != CALIB_NONE ) {
	       CALIB_SMCD_BDR( param.calib_moon, channel, fcd, smcd, 
			       nr_smcd, rec_out );
	  } else if ( flag_origin == FLAG_SUN 
		      && param.calib_sun != CALIB_NONE ) {
	       CALIB_SMCD_BDR( param.calib_sun, channel, fcd, smcd, 
			       nr_smcd, rec_out );
	  }
	  break;
     default:
	  break;
     }
/*
 * write only selected Moon/Sun data records
 */
     for ( num = nr = 0; nr < nr_smcd; nr++ ) {
	  if ( SELECT_SMCD( param, smcd+indx_smcd[nr] ) != 0 ) {
	       if ( num != nr )
		    (void) memcpy( rec_out+num, rec_out+nr, size_rec );
	       num++;
	  }
     }
     nr_rec = num;
/*
 * write data to HDF5 file
 */
     if ( param.write_hdf5 == PARAM_SET )
	  GOME_LV1_WR_H5_REC( flag_origin, nband, param, 
			      nr_rec, wv_start, wv_count, rec_out );
/*
 * write data to ASCII file
 */
     if ( param.write_ascii == PARAM_SET ) {
	  GOME_LV1_WR_ASCII_REC( flag_origin, nband, param, 
			   nr_rec, wv_start, wv_count, rec_out );
     }
/*
 * free allocated memory
 */
     free( rec_out );
}
