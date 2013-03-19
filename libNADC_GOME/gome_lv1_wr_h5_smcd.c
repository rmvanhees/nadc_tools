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

.IDENTifer   GOME_LV1_WR_H5_SMCD
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write GOME level 1 SMCD data to file

.INPUT/OUTPUT
  call as    GOME_LV1_WR_H5_SMCD( flag_origin, param, nr_smcd, smcd );

  input: 
             unsigned char flag_origin :  FLAG_SUN or FLAG_MOON
	     struct param_record param :  command-line parameters
	     int           nr_smcd     :  size of SMCD array
	     struct smcd_gome *smcd  :  Sun/Moon Specific Calibration Record

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   11-Nov-2001	moved to the new Error handling routines, RvH
              2.3   04-Nov-2000	let this module create its own group, RvH 
              2.2   20-Aug-1999 write prism-temp from IHR, RvH
              2.1   23-Jul-1999 using struct param, RvH
              2.0   15-Jun-1999 rewritten, RvH
              1.0   03-Jun-1999 created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System headers +++++*/
#include <stdlib.h>

#include <hdf5.h>

/*+++++ Local Headers +++++*/
#define _GOME_COMMON
#include <nadc_gome.h>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void GOME_LV1_WR_H5_SMCD( unsigned char flag_origin, 
			  struct param_record param, 
			  short nr_smcd, const short *indx_smcd,
			  const struct smcd_gome *smcd )
{
     register hsize_t ni, nr, nx, ny;

     unsigned short *usbuff;
     short          *sbuff;
     unsigned int   *ubuff;
     float          *rbuff;

     hid_t   grp_id;
     hbool_t compress;
     hsize_t nrpix, dims[2];

     const char prognm[] = "GOME_LV1_WR_H5_SMCD";
/*
 * check number of SMCD records
 */
     if ( nr_smcd == 0 || indx_smcd == NULL || smcd == NULL ) return;
/*
 * set H5 boolean variable for compression
 */
     if ( param.flag_deflate == PARAM_SET )
	  compress = TRUE;
     else
	  compress = FALSE;
/*
 * open or create group SUN or MOON
 */
     if ( flag_origin == FLAG_SUN ) {
	  if ( (grp_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/SUN" )) < 0 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "/SUN" );
     } else {
	  if ( (grp_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/MOON" )) < 0 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "/MOON" );
     }
     (void) H5Gclose( grp_id );
/*
 * create group /SUN/SCD or /MOON/MCD
 */
     if ( flag_origin == FLAG_SUN ) {
	  grp_id = H5Gcreate( param.hdf_file_id, "/SUN/SCD", 
			      H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
	  if ( grp_id < 0 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "/SUN/SCD" );
     } else {
	  grp_id = H5Gcreate( param.hdf_file_id, "/MOON/MCD", 
			      H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
	  if ( grp_id < 0 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "/MOON/MCD" );
     }
/*
 * +++++ create/write attributes
 */
     dims[0] = (hsize_t) nr_smcd;
/*
 * UTC date and time
 */
     ubuff = (unsigned int *) malloc( dims[0] * sizeof( unsigned int));
     if ( ubuff == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "ubuff" );
     ny = 0;
     do { ubuff[ny] = smcd[indx_smcd[ny]].utc_date; } while ( ++ny < dims[0] );
     NADC_WR_HDF5_Dataset( compress, grp_id, "UTC_date", 
			  H5T_NATIVE_UINT, 1, dims, ubuff );
     ny = 0;
     do { ubuff[ny] = smcd[indx_smcd[ny]].utc_time; } while ( ++ny < dims[0] );
     NADC_WR_HDF5_Dataset( compress, grp_id, "UTC_time", 
			  H5T_NATIVE_UINT, 1, dims, ubuff );
     free( ubuff );
/*
 * Sun zenith and azimuth angle
 */
     rbuff = (float *) malloc( dims[0] * sizeof( float ));
     if ( rbuff == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "rbuff" );
     ny = 0;
     do {
	  rbuff[ny] = smcd[indx_smcd[ny]].north_sun_zen;
     } while ( ++ny < dims[0] );
     NADC_WR_HDF5_Dataset( compress, grp_id, "SunZenith_North", 
			  H5T_NATIVE_FLOAT, 1, dims, rbuff );
     ny = 0;
     do {
	  rbuff[ny] = smcd[indx_smcd[ny]].north_sun_azim;
     } while ( ++ny < dims[0] );
     NADC_WR_HDF5_Dataset( compress, grp_id, "SunAzimuth_North", 
			  H5T_NATIVE_FLOAT, 1, dims, rbuff );
/*
 * BSDF/Moon zenith and azimuth angle
 */
     ny = 0;
     do { 
	  rbuff[ny] = smcd[indx_smcd[ny]].north_sm_zen;
     } while ( ++ny < dims[0] );
     if ( flag_origin == FLAG_SUN )
	  NADC_WR_HDF5_Dataset( compress, grp_id, "BSDF_Zenith_North", 
			       H5T_NATIVE_FLOAT, 1, dims, rbuff );
     else
	  NADC_WR_HDF5_Dataset( compress, grp_id, "MoonZenith_North", 
			       H5T_NATIVE_FLOAT, 1, dims, rbuff );
     ny = 0;
     do { 
	  rbuff[ny] = smcd[indx_smcd[ny]].north_sm_azim; 
     } while ( ++ny < dims[0] );
     if ( flag_origin == FLAG_SUN )
	  NADC_WR_HDF5_Dataset( compress, grp_id, "BSDF_Azimuth_North", 
			       H5T_NATIVE_FLOAT, 1, dims, rbuff );
     else
	  NADC_WR_HDF5_Dataset( compress, grp_id, "MoonAzimuth_North", 
			       H5T_NATIVE_FLOAT, 1, dims, rbuff );
/*
 * Sun reference spectrum/Moon Disk
 */
     ny = 0;
     do { 
	  rbuff[ny] = smcd[indx_smcd[ny]].sun_or_moon; 
     } while ( ++ny < dims[0] );
     if ( flag_origin == FLAG_SUN )
	  NADC_WR_HDF5_Dataset( compress, grp_id, "SunReferenceSpectrum", 
			       H5T_NATIVE_FLOAT, 1, dims, rbuff );
     else
	  NADC_WR_HDF5_Dataset( compress, grp_id, "IlluminatedMoonDisk", 
			       H5T_NATIVE_FLOAT, 1, dims, rbuff );
/*
 * Dark current and Noise Correction Factor
 */
     ny = 0;
     do { 
	  rbuff[ny] = smcd[indx_smcd[ny]].dark_current; 
     } while ( ++ny < dims[0] );
     NADC_WR_HDF5_Dataset( compress, grp_id, "DarkCurrent", 
			  H5T_NATIVE_FLOAT, 1, dims, rbuff );
     ny = 0;
     do { 
	  rbuff[ny] = smcd[indx_smcd[ny]].noise_factor;
     } while ( ++ny < dims[0] );
     NADC_WR_HDF5_Dataset( compress, grp_id, "NoiseCorrection", 
			  H5T_NATIVE_FLOAT, 1, dims, rbuff );
     free( rbuff );
/*
 * Indices
 */
     sbuff = (short *) malloc( (size_t) nr_smcd * sizeof( short ));
     if ( sbuff == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "sbuff" );
     ny = 0;
     do { 
	  sbuff[ny] = smcd[indx_smcd[ny]].indx_spec; 
     } while ( ++ny < dims[0] );
     NADC_WR_HDF5_Dataset( compress, grp_id, "SpectralCalibrationIndex", 
			  H5T_NATIVE_SHORT, 1, dims, sbuff );
     ny = 0;
     do { 
	  sbuff[ny] = smcd[indx_smcd[ny]].indx_leak; 
     } while ( ++ny < dims[0] );
     NADC_WR_HDF5_Dataset( compress, grp_id, "LeakageCorrectionIndex", 
			  H5T_NATIVE_SHORT, 1, dims, sbuff );
     free( sbuff );
/*
 * ------------------------- Instrument Header Structure
 * Sub set counter
 */
     usbuff = (unsigned short *) malloc( dims[0] * sizeof( short ));
     if ( usbuff == NULL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "usbuff" );
     ny = 0;
     do {
	  usbuff[ny] = smcd[indx_smcd[ny]].ihr.subsetcounter;
     } while ( ++ny < dims[0] );
     NADC_WR_HDF5_Dataset( compress, grp_id, "SubSetCounter",
			  H5T_NATIVE_USHORT, 1, dims, usbuff );
/*
 * Average Mode
 */
     for ( ny = 0; ny < dims[0]; ny++ )
	  usbuff[ny] = smcd[indx_smcd[ny]].ihr.averagemode;
     NADC_WR_HDF5_Dataset( compress, grp_id, "AverageMode",
			  H5T_NATIVE_USHORT, 1, dims, usbuff );
     free( usbuff );
/*
 * Peltier values
 */
     dims[1] = SCIENCE_CHANNELS; 
     nrpix = dims[0] * dims[1];
     sbuff = (short *) malloc( nrpix * sizeof( short ));
     if ( sbuff == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "sbuff" );
     nr = ny = 0;
     do {
	  nx = 0;
	  do {
	       sbuff[nr++] = smcd[indx_smcd[ny]].ihr.peltier[nx];
	  } while ( ++nx < dims[1] );
     } while ( ++ny < dims[0] );
     NADC_WR_HDF5_Dataset( compress, grp_id, "Peltier",
			  H5T_NATIVE_SHORT, 2, dims, sbuff );
     free( sbuff );
/*
 * Pre-disperser prism temperature
 */
     rbuff = (float *) malloc( dims[0] * sizeof( float ));
     if ( rbuff == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "rbuff" );
     ny = 0;
     do {
	  rbuff[ny] = (float) (-1.721 + 6.104e-3 * 
			       smcd[indx_smcd[ny]].ihr.prism_temp);
     } while ( ++ny < dims[0] );
     NADC_WR_HDF5_Dataset( compress, grp_id, "PrismTemperature",
			  H5T_NATIVE_FLOAT, 1, dims, rbuff );
     free( rbuff );
     (void) H5Gclose( grp_id );
/*
 * create group /SUN/PMD or /MOON/PMD
 */
     if ( flag_origin == FLAG_SUN ) {
	  grp_id = H5Gcreate( param.hdf_file_id, "/SUN/PMD", 
			      H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
	  if ( grp_id < 0 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "/SUN/PMD" );
     } else {
	  grp_id = H5Gcreate( param.hdf_file_id, "/MOON/PMD", 
			      H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
	  if ( grp_id < 0 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "/MOON/PMD" );
     }
/*
 * write PMD values
 */
     dims[1] = PMD_NUMBER * PMD_IN_GRID;
     nrpix = (size_t) (dims[0] * dims[1]);
     rbuff = (float *) malloc( nrpix * sizeof( float ));
     if ( rbuff == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "rbuff" );
     ni = 0; nr = 0;
     do {
	  ny = 0;
	  do {
	       nx = 0;
	       do {
		    rbuff[ni++] = smcd[indx_smcd[nr]].pmd[ny].value[nx];
	       } while ( ++nx < PMD_NUMBER );
	  } while ( ++ny < PMD_IN_GRID );
     } while ( ++nr < (hsize_t) nr_smcd );
     NADC_WR_HDF5_Dataset( compress, grp_id, "value",
                           H5T_NATIVE_FLOAT, 2, dims, rbuff );
     free( rbuff );
/*
 * close interface
 */
     (void) H5Gclose( grp_id );
}
