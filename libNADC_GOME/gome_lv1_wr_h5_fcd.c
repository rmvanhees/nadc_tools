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

.IDENTifer   GOME_LV1_WR_H5_FCD
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write GOME level 1 FCD data

.INPUT/OUTPUT
  call as    GOME_LV1_WR_H5_FCD( param, &fcd );

     input:  
             struct param_record param : struct holding user-defined settings
	     struct fcd_gome   *fcd  : Fixed Calibration Data Record

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      2.0   11-Nov-2001	moved to the new Error handling routines, RvH
              1.3   19-Jul-2001	pass structures using pointers, RvH 
             1.2     04-Nov-2000   let this module create its own groups, RvH
             1.1     23-Jul-1999   Using struct param, RvH
             1.0     15-Jun-1999   Created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System headers +++++*/
#include <stdlib.h>
#include <string.h>

#include <hdf5.h>

/*+++++ Local Headers +++++*/
#define _GOME_COMMON
#include <nadc_gome.h>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void GOME_LV1_WR_H5_FCD( struct param_record param, 
			 const struct fcd_gome *fcd )
{
     register hsize_t nr, nx, ny, nz;

     unsigned char  cbuff[7];
     short   *sbuff;
     float   *rbuff, *rpntr;
     double  *dbuff, *dpntr;
     size_t  nrpix, nrbyte;

     hid_t   grp_id;
     hbool_t compress;
     hsize_t dims[3];

     const char prognm[] = "GOME_LV1_WR_H5_FCD";
/*
 * set HDF5 boolean variable for compression
 */
     if ( param.flag_deflate == PARAM_SET )
          compress = TRUE;
     else
          compress = FALSE;
/*
 * create group FCD
 */
     grp_id = H5Gcreate( param.hdf_file_id, "/FCD", 
			 H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
     if ( grp_id < 0 ) NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "/FCD" );
/*
 * +++++ create/write attributes and datasets in the /FCD group
 *
 * detector confidence flags
 */
     dims[0] = 7;
     cbuff[0] = (unsigned char) fcd->detector_flags.flag_fields.array_1;
     cbuff[1] = (unsigned char) fcd->detector_flags.flag_fields.array_2;
     cbuff[2] = (unsigned char) fcd->detector_flags.flag_fields.array_3;
     cbuff[3] = (unsigned char) fcd->detector_flags.flag_fields.array_4;
     cbuff[4] = (unsigned char) fcd->detector_flags.flag_fields.pmd_1;
     cbuff[5] = (unsigned char) fcd->detector_flags.flag_fields.pmd_1;
     cbuff[6] = (unsigned char) fcd->detector_flags.flag_fields.pmd_1;
     NADC_WR_HDF5_Attribute( grp_id, "Detector Confidence Flags", 
			    H5T_NATIVE_UCHAR, 1, dims, cbuff );
/*
 * number of Leakage Correction Parameters
 */
     dims[0] = 1;
     NADC_WR_HDF5_Attribute( grp_id, "NrLeak", 
			    H5T_NATIVE_SHORT, 1, dims, &fcd->nleak );
/*
 * number of Hot pixels
 */
     NADC_WR_HDF5_Attribute( grp_id, "NrHot", 
			    H5T_NATIVE_SHORT, 1, dims, &fcd->nhot );
/*
 * number of Spectral Calibration Parameters
 */
     NADC_WR_HDF5_Attribute( grp_id, "NrSpec", 
			    H5T_NATIVE_SHORT, 1, dims, &fcd->nspec );
/*
 * number of Peltier filter coefficients used
 */
     NADC_WR_HDF5_Attribute( grp_id, "FPA_UsedCoeffs", 
			    H5T_NATIVE_SHORT, 1, dims, &fcd->npeltier );
/*
 * Bi-directional Scattering Distribution
 */
     NADC_WR_HDF5_Attribute( grp_id, "BSDF_0", 
			    H5T_NATIVE_FLOAT, 1, dims, &fcd->bsdf_0 );
     NADC_WR_HDF5_Attribute( grp_id, "Elevation", 
			    H5T_NATIVE_FLOAT, 1, dims, &fcd->elevation );
     NADC_WR_HDF5_Attribute( grp_id, "Azimuth", 
			    H5T_NATIVE_FLOAT, 1, dims, &fcd->azimuth );

     dims[0] = 8;
     NADC_WR_HDF5_Attribute( grp_id, "Coefficients", 
			    H5T_NATIVE_FLOAT, 1, dims, fcd->coeffs );
/*
 * Uniform straylight level
 */
     dims[0] = SCIENCE_CHANNELS;
     NADC_WR_HDF5_Attribute( grp_id, "StraylightUniform", 
			    H5T_NATIVE_FLOAT, 1, dims, fcd->stray_level );
/*
 * Symmetrical and Asymmetrical ghosts characteristics
 */
     dims[0] = 1;
     NADC_WR_HDF5_Attribute( grp_id, "StraylightTriangle", 
			    H5T_NATIVE_SHORT, 1, dims, &fcd->width_conv );
     dims[0] = SCIENCE_CHANNELS;
     dims[1] = NUM_STRAY_GHOSTS;
     NADC_WR_HDF5_Attribute( grp_id, "StraylightSymmetry", 
			    H5T_NATIVE_SHORT, 2, dims, fcd->ghost.symmetry );
     NADC_WR_HDF5_Attribute( grp_id, "StraylightCenter", 
			    H5T_NATIVE_SHORT, 2, dims, fcd->ghost.center );
     NADC_WR_HDF5_Attribute( grp_id, "StraylightDefocus", 
			    H5T_NATIVE_FLOAT, 2, dims, fcd->ghost.defocus );
     NADC_WR_HDF5_Attribute( grp_id, "StraylightEnergy", 
			    H5T_NATIVE_FLOAT, 2, dims, fcd->ghost.energy );
/*
 * filter Peltier coefficients
 */
     dims[0] = NUM_FPA_SCALE;
     NADC_WR_HDF5_Attribute( grp_id, "FPA_ScaleFactor", 
			    H5T_NATIVE_FLOAT, 1, dims, fcd->scale_peltier );

     dims[0] = (hsize_t) fcd->npeltier;
     NADC_WR_HDF5_Attribute( grp_id, "FPA_FilterCoeffs", 
			    H5T_NATIVE_FLOAT, 1, dims, fcd->filter_peltier );
/*
 * Relative Error Budget on Key data functions
 */
     dims[0] = SCIENCE_CHANNELS;
     NADC_WR_HDF5_Attribute( grp_id, "KDE_BSDF_1", H5T_NATIVE_FLOAT,
			    1, dims, &fcd->kde.bsdf_1 );
     NADC_WR_HDF5_Attribute( grp_id, "KDE_BSDF_2", H5T_NATIVE_FLOAT,
			    1, dims, &fcd->kde.bsdf_2 );
     NADC_WR_HDF5_Attribute( grp_id, "KDE_RESPONSE_1", H5T_NATIVE_FLOAT,
			    1, dims, &fcd->kde.resp_1 );
     NADC_WR_HDF5_Attribute( grp_id, "KDE_RESPONSE_2", H5T_NATIVE_FLOAT,
			    1, dims, &fcd->kde.resp_2 );
     NADC_WR_HDF5_Attribute( grp_id, "KDE_F2_1", H5T_NATIVE_FLOAT,
			    1, dims, &fcd->kde.f2_1 );
     NADC_WR_HDF5_Attribute( grp_id, "KDE_F2_2", H5T_NATIVE_FLOAT,
			    1, dims, &fcd->kde.f2_2 );
     NADC_WR_HDF5_Attribute( grp_id, "KDE_SMDEP_1", H5T_NATIVE_FLOAT,
			    1, dims, &fcd->kde.smdep_1 );
     NADC_WR_HDF5_Attribute( grp_id, "KDE_SMDEP_2", H5T_NATIVE_FLOAT,
			    1, dims, &fcd->kde.smdep_2 );
     NADC_WR_HDF5_Attribute( grp_id, "KDE_CHI_1", H5T_NATIVE_FLOAT,
			    1, dims, &fcd->kde.chi_1 );
     NADC_WR_HDF5_Attribute( grp_id, "KDE_CHI_2", H5T_NATIVE_FLOAT,
			    1, dims, &fcd->kde.chi_2 );
     NADC_WR_HDF5_Attribute( grp_id, "KDE_ETA_1", H5T_NATIVE_FLOAT,
			    1, dims, &fcd->kde.eta_1 );
     NADC_WR_HDF5_Attribute( grp_id, "KDE_ETA_2", H5T_NATIVE_FLOAT,
			    1, dims, &fcd->kde.eta_2 );
     NADC_WR_HDF5_Attribute( grp_id, "KDE_KSI_1", H5T_NATIVE_FLOAT,
			    1, dims, &fcd->kde.ksi_1 );
     NADC_WR_HDF5_Attribute( grp_id, "KDE_KSI_2", H5T_NATIVE_FLOAT,
			    1, dims, &fcd->kde.ksi_2 );
/*
 * KDE_Response_f2_SMDep
 */
     dims[1] = CHANNEL_SIZE; dims[0] = SCIENCE_CHANNELS;
     NADC_WR_HDF5_Dataset( compress, grp_id, "KDE_Response_f2_SMDep", 
			   H5T_NATIVE_FLOAT, 2, dims, &fcd->kde.rfs );
/*
 * Band Configuration records
 */
     dims[1] = 3; dims[0] = NUM_SPEC_BANDS;
     nrpix = (size_t) (dims[0] * dims[1]);
     sbuff = (short *) malloc( nrpix * sizeof( short ));
     if ( sbuff == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "sbuff" );
     nr = nx = 0;
     do {
	  sbuff[nr++] = fcd->bcr[nx].array_nr;
	  sbuff[nr++] = fcd->bcr[nx].start;
	  sbuff[nr++] = fcd->bcr[nx].end;
     } while ( ++nx < NUM_SPEC_BANDS );
     NADC_WR_HDF5_Dataset( compress, grp_id, "BCR", 
			   H5T_NATIVE_SHORT, 2, dims, sbuff ); 
     free( sbuff );
/*
 * Spectral parameter and errors
 */
     if ( fcd->nspec > 0 ) {
	  dims[0] = (hsize_t) fcd->nspec;
	  dims[1] = SCIENCE_CHANNELS; 
	  dims[2] = NUM_SPEC_COEFFS; 
	  nrpix = dims[0] * dims[1] * dims[2];
	  if ( (dbuff  = (double *) malloc( nrpix * sizeof(double))) == NULL ) 
	       NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "dbuff" );
	  nr = nz = 0;
	  do {
	       ny = 0;
	       do { 
		    nx = 0;
		    do { 
			 dbuff[nr++] = fcd->spec[nz].coeffs[ny][nx];
		    } while ( ++nx < dims[2] );
	       } while ( ++ny < dims[1] );
	  } while ( ++nz < dims[0] );
	  NADC_WR_HDF5_Dataset( compress, grp_id, "SpectralParameter", 
				H5T_NATIVE_DOUBLE, 3, dims, dbuff );
	  free( dbuff );
     }

     if ( fcd->nspec > 0 ) {
	  dims[0] = (hsize_t) fcd->nspec;
	  dims[1] = SCIENCE_CHANNELS; 
	  nrpix = dims[0] * dims[1];
	  nrbyte = dims[1] * sizeof( double );
	  if ( (dbuff = (double *) malloc( nrpix * sizeof(double))) == NULL ) 
	       NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "dbuff" );
	  dpntr = dbuff;
	  ny = 0;
	  do {
	       (void) memcpy( dpntr, fcd->spec[ny].error, nrbyte );
	       dpntr += dims[1];
	  } while ( ++ny < dims[0] );
	  NADC_WR_HDF5_Dataset( compress, grp_id, "SpectralError", 
				H5T_NATIVE_DOUBLE, 2, dims, dbuff );
	  free( dbuff );
     }
/*
 * Pixel to Pixel gain
 */
     dims[1] = CHANNEL_SIZE; 
     dims[0] = SCIENCE_CHANNELS;
     NADC_WR_HDF5_Dataset( compress, grp_id, "Pixel_to_PixelGain", 
			   H5T_NATIVE_FLOAT, 2, dims, fcd->pixel_gain );
/*
 * Instrument Response
 */
     dims[1] = CHANNEL_SIZE; 
     dims[0] = SCIENCE_CHANNELS;
     NADC_WR_HDF5_Dataset( compress, grp_id, "InstrumentResponse", 
			   H5T_NATIVE_FLOAT, 2, dims, fcd->intensity );
/*
 * number of different scan mirror angles
 */
     dims[0] = 1;
     NADC_WR_HDF5_Attribute( grp_id, "NrAngl",
			    H5T_NATIVE_SHORT, 1, dims, &fcd->nang );
/*
 * Pre-Flight calibration data
 */
     if ( fcd->nang > 0 ) {
	  dims[0] = (hsize_t) fcd->nang;
	  dims[1] = CHANNEL_SIZE; 
	  nrpix = dims[0] * dims[1];
	  nrbyte = dims[1] * sizeof( float );
	  if ( (rbuff = (float *) malloc( nrpix * sizeof( float ))) == NULL ) 
	       NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "rbuff" );
	  rpntr = rbuff;
	  ny = 0;
	  do {
	       (void) memcpy( rpntr, fcd->calib[ny].eta_omega, nrbyte );
	       rpntr += dims[1];
	  } while ( ++ny < dims[0] );
	  NADC_WR_HDF5_Dataset( compress, grp_id, "EtaOmega", 
				H5T_NATIVE_FLOAT, 2, dims, rbuff );
	  ny = 0;
	  rpntr = rbuff;
	  do {
	       (void) memcpy( rpntr, fcd->calib[ny].response, nrbyte );
	       rpntr += dims[1];
	  } while ( ++ny < dims[0] );
	  NADC_WR_HDF5_Dataset( compress, grp_id, "RadianceResponse", 
				H5T_NATIVE_FLOAT, 2, dims, rbuff );
	  free( rbuff );
     }
     (void) H5Gclose( grp_id );
/*
 * create group /FCD/Leakage
 */
     grp_id = H5Gcreate( param.hdf_file_id, "/FCD/Leakage", 
			 H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
     if ( grp_id < 0 ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "/FCD/Leakage" );
/*
 * +++++ create datasets in the /FCD/Leakage group
 *
 * Dark current
 */
     dims[2] = CHANNEL_SIZE; 
     dims[1] = SCIENCE_CHANNELS; 
     dims[0] = (hsize_t) fcd->nleak;
     nrpix = dims[0] * dims[1] * dims[2];
     nrbyte = SCIENCE_PIXELS * sizeof( float );
     rpntr = rbuff = (float *) malloc( nrpix * sizeof( float ));
     if ( rbuff == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "rbuff" );
     nz = 0;
     do {
	  (void) memcpy( rpntr, fcd->leak[nz].dark, nrbyte );
	  rpntr += SCIENCE_PIXELS;
     } while ( ++nz < dims[0] );
     NADC_WR_HDF5_Dataset( compress, grp_id, "DarkCurrent", 
			   H5T_NATIVE_FLOAT, 3, dims, rbuff );
     free( rbuff );
/*
 * +++++ create attributes in the /FCD/Leakage group
 */
     dims[0] = (hsize_t) fcd->nleak;
     rbuff = (float *) malloc( dims[0] * sizeof( float ));
     if ( rbuff == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "rbuff" );
     nr = 0;
     do { rbuff[nr] = fcd->leak[nr].noise; } while ( ++nr < dims[0] );
     NADC_WR_HDF5_Attribute( grp_id, "ArrayNoise",
				   H5T_NATIVE_FLOAT, 1, dims, rbuff );
     nr = 0;
     do { rbuff[nr] = fcd->leak[nr].pmd_noise; } while ( ++nr < dims[0] );
     NADC_WR_HDF5_Attribute( grp_id, "PMDNoise",
			    H5T_NATIVE_FLOAT, 1, dims, rbuff );
     free( rbuff );

     dims[1] = 3; 
     dims[0] = (hsize_t) fcd->nleak;
     nrpix = dims[0] * dims[1];
     rbuff = (float *) malloc( nrpix * sizeof( float ));
     if ( rbuff == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "rbuff" );
     nr = ny = 0;
     do {
	  nx = 0;
	  do { 
	       rbuff[nr++] = fcd->leak[ny].pmd_offs[nx]; 
	  } while ( ++nx < dims[0] );
     } while ( ++ny < dims[1] );
     NADC_WR_HDF5_Attribute( grp_id, "PMDOffset",
			    H5T_NATIVE_FLOAT, 2, dims, rbuff );
     free( rbuff );
     (void) H5Gclose( grp_id );
/*
 * create group /FCD/Sun
 */
     grp_id = H5Gcreate( param.hdf_file_id, "/FCD/Sun", 
			 H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
     if ( grp_id < 0 ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "/FCD/Sun" );
/*
 * Sun Mean Reference Spectrum with relative errors
 */
     dims[1] = CHANNEL_SIZE; 
     dims[0] = SCIENCE_CHANNELS;
     NADC_WR_HDF5_Dataset( compress, grp_id, "SunReference", 
			   H5T_NATIVE_FLOAT, 2, dims, fcd->sun_ref );
     NADC_WR_HDF5_Dataset( compress, grp_id, "SunReferenceError", 
			   H5T_NATIVE_FLOAT, 2, dims, fcd->sun_precision );
/*
 * +++++ create attributes in the /FCD/Sun group
 */
/*
 * Sun Reference Date & Time
 */
     NADC_WR_HDF5_Attribute( grp_id, "UTC_date",
			    H5T_NATIVE_UINT, 1, dims, &fcd->sun_date );
     NADC_WR_HDF5_Attribute( grp_id, "UTC_time",
			    H5T_NATIVE_UINT, 1, dims, &fcd->sun_time );
/*
 * Sun Mean PMD Wavelength
 */
     dims[0] = 3;
     NADC_WR_HDF5_Attribute( grp_id, "MeanPMD_wv",
			    H5T_NATIVE_FLOAT, 1, dims, &fcd->sun_pmd_wv );
     NADC_WR_HDF5_Attribute( grp_id, "MeanPMD",
			    H5T_NATIVE_FLOAT, 1, dims, &fcd->sun_pmd );
     (void) H5Gclose( grp_id );
}
