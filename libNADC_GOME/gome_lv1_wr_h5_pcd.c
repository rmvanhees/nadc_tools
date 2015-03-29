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

.IDENTifer   GOME_LV1_WR_H5_PCD
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write GOME level 1 PCD data

.INPUT/OUTPUT
  call as    GOME_LV1_WR_H5_PCD( param, nr_pcd, indx_pcd, pcd );

  input: 
             struct param_record param : struct holding user-defined settings
	     int   nr_pcd              : number of selected PCD records
	     short *indx_pcd           : indices to selected PCD records
	     struct pcd_gome *pcd    : Pixel Specific Calibration Record(s)

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      5.0   08-Jun-2009 update to product version 2, RvH
              4.1   09-Apr-2003	write gelocation as a structure, RvH
              4.0   11-Nov-2001	moved to the new Error handling routines, RvH
              3.1   04-Nov-2000	let this module create its own group, RvH 
              3.0   08-Feb-2000 read all PCD for calibration, write selected
              2.2   20-Aug-1999 write prism-temp from IHR, RvH
              2.1   23-Jul-1999 using struct param, RvH
              2.0   12-Jun-1999 rewritten, RvH
              1.0   03-Jun-1999 created by R. M. van Hees
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
void GOME_LV1_WR_H5_PCD( struct param_record param, short nr_pcd, 
			 const short *indx_pcd, const struct pcd_gome *pcd )
{
     register hsize_t ni, nr, nx, ny;

     unsigned short *usbuff;
     short          *sbuff;
     float          *rbuff;

     hid_t    type_id;
     hid_t    grp_id;
     hbool_t  compress;
     hsize_t  nrpix, dims[2];

     struct   glr1_gome *glr;
     struct   cr1_gome  *cld;
/*
 * check number of PCD records
 */
     if ( nr_pcd == 0 || indx_pcd == NULL || pcd == NULL ) return;
/*
 * set HDF5 boolean variable for compression
 */
     if ( param.flag_deflate == PARAM_SET )
          compress = TRUE;
     else
          compress = FALSE;
/*
 * open or create group Earth
 */
     grp_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/EARTH" );
     if ( grp_id < 0 ) NADC_RETURN_ERROR( NADC_ERR_HDF_GRP, "/EARTH" );
     (void) H5Gclose( grp_id );
/*
 * create group /EARTH/PCD
 */
     grp_id = H5Gcreate( param.hdf_file_id, "/EARTH/PCD", 
			 H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
     if ( grp_id < 0 ) 
	  NADC_RETURN_ERROR( NADC_ERR_HDF_GRP, "/EARTH/PCD" );
/*
 * +++++ create datasets in the /EARTH/PCD group
 */
     dims[0] = (size_t) nr_pcd;
/*
 * Write geolocation information
 */
     type_id = H5Topen( param.hdf_file_id, "glr", H5P_DEFAULT );
     glr = (struct glr1_gome *)
	  malloc( dims[0] * sizeof( struct glr1_gome ));
     if ( glr == NULL ) NADC_RETURN_ERROR( NADC_ERR_ALLOC, "glr" );
     for ( nr = 0; nr < dims[0]; nr++ )
	  (void) memcpy( glr+nr, &pcd[nr].glr, sizeof( struct glr1_gome ) );
     NADC_WR_HDF5_Dataset( compress, grp_id, "glr", 
			  type_id, 1, dims, glr );
     free( glr );
     (void) H5Tclose( type_id );
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_HDF_WR, "glr" );
/*
 * Write geolocation information
 */
     type_id = H5Topen( param.hdf_file_id, "cld", H5P_DEFAULT );
     cld = (struct cr1_gome *)
	  malloc( dims[0] * sizeof( struct cr1_gome ));
     if ( cld == NULL ) NADC_RETURN_ERROR( NADC_ERR_ALLOC, "cld" );
     for ( nr = 0; nr < dims[0]; nr++ )
	  (void) memcpy( cld+nr, &pcd[nr].cld, sizeof( struct cr1_gome ) );
     NADC_WR_HDF5_Dataset( compress, grp_id, "cld", 
			   type_id, 1, dims, cld );
     free( cld );
     (void) H5Tclose( type_id );
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_HDF_WR, "cld" );
/*
 * Dark current and Noise Correction Factor
 */
     rbuff = (float *) malloc( dims[0] * sizeof( float ));
     if ( rbuff == NULL ) NADC_RETURN_ERROR( NADC_ERR_ALLOC, "rbuff" );
     for ( ny = 0; ny < dims[0]; ny++ )
	  rbuff[ny] = pcd[indx_pcd[ny]].dark_current;
     NADC_WR_HDF5_Dataset( compress, grp_id, "DarkCurrent", 
			  H5T_NATIVE_FLOAT, 1, dims, rbuff );
     for ( ny = 0; ny < dims[0]; ny++ )
	  rbuff[ny] = pcd[indx_pcd[ny]].noise_factor;
     NADC_WR_HDF5_Dataset( compress, grp_id, "NoiseCorrection", 
			  H5T_NATIVE_FLOAT, 1, dims, rbuff );
/*
 * Plane of the Polarisation
 */
     for ( ny = 0; ny < dims[0]; ny++ )
	  rbuff[ny] = pcd[indx_pcd[ny]].polar.chi;
     NADC_WR_HDF5_Dataset( compress, grp_id, "PolarisationPlaneAngle",
			  H5T_NATIVE_FLOAT, 1, dims, rbuff );
     free( rbuff );
/*
 * Indices
 */
     sbuff = (short *) malloc( dims[0] * sizeof( short ));
     if ( sbuff == NULL ) NADC_RETURN_ERROR( NADC_ERR_ALLOC, "sbuff" );
     for ( ny = 0; ny < dims[0]; ny++ )
	  sbuff[ny] = pcd[indx_pcd[ny]].indx_spec;
     NADC_WR_HDF5_Dataset( compress, grp_id, "SpectralCalibrationIndex",
			  H5T_NATIVE_SHORT, 1, dims, sbuff );

     for ( ny = 0; ny < dims[0]; ny++ )
	  sbuff[ny] = pcd[indx_pcd[ny]].indx_leak;
     NADC_WR_HDF5_Dataset( compress, grp_id, "LeakageCorrectionIndex",
			  H5T_NATIVE_SHORT, 1, dims, sbuff );
     free( sbuff );
/*
 * Polarisation parameters
 */
     dims[0] = nr_pcd; dims[1] = NUM_POLAR_COEFFS;
     nrpix = dims[0] * dims[1];
     rbuff = (float *) malloc( nrpix * sizeof( float ));
     if ( rbuff == NULL ) NADC_RETURN_ERROR( NADC_ERR_ALLOC, "rbuff" );
     for ( nr = ny = 0; ny < dims[0]; ny++ )
	  for ( nx = 0; nx < dims[1]; nx++ )
	       rbuff[nr++] = pcd[indx_pcd[ny]].polar.wv[nx];
     NADC_WR_HDF5_Dataset( compress, grp_id, "PolarisationWavelength",
			  H5T_NATIVE_FLOAT, 2, dims, rbuff );
     for ( nr = ny = 0; ny < dims[0]; ny++ )
	  for ( nx = 0; nx < dims[1]; nx++ )
	       rbuff[nr++] = pcd[indx_pcd[ny]].polar.coeff[nx];
     NADC_WR_HDF5_Dataset( compress, grp_id, "PolarisationCoefficient",
			  H5T_NATIVE_FLOAT, 2, dims, rbuff );

     for ( nr = ny = 0; ny < dims[0]; ny++ )
	  for ( nx = 0; nx < dims[1]; nx++ )
	       rbuff[nr++] = pcd[indx_pcd[ny]].polar.error[nx];
     NADC_WR_HDF5_Dataset( compress, grp_id, "PolarisationError",
			  H5T_NATIVE_FLOAT, 2, dims, rbuff );
     free( rbuff );
/*
 * ------------------------- Instrument Header Structure
 * Sub set counter
 */
     usbuff = (unsigned short *) malloc( dims[0] * sizeof( short ));
     if ( usbuff == NULL ) 
	  NADC_RETURN_ERROR( NADC_ERR_ALLOC, "usbuff" );
     for ( ny = 0; ny < dims[0]; ny++ )
	  usbuff[ny] = pcd[indx_pcd[ny]].ihr.subsetcounter;
     NADC_WR_HDF5_Dataset( compress, grp_id, "SubSetCounter",
			  H5T_NATIVE_USHORT, 1, dims, usbuff );
/*
 * Average Mode
 */
     for ( ny = 0; ny < dims[0]; ny++ )
	  usbuff[ny] = pcd[indx_pcd[ny]].ihr.averagemode;
     NADC_WR_HDF5_Dataset( compress, grp_id, "AverageMode",
			  H5T_NATIVE_USHORT, 1, dims, usbuff );
     free( usbuff );
/*
 * Peltier values
 */
     dims[0] = nr_pcd; 
     dims[1] = SCIENCE_CHANNELS; 
     nrpix = dims[0] * dims[1];
     sbuff = (short *) malloc( nrpix * sizeof( short ));
     if ( sbuff == NULL ) NADC_RETURN_ERROR( NADC_ERR_ALLOC, "sbuff" );
     for ( nr = ny = 0; ny < dims[0]; ny++)
	  for ( nx = 0; nx < dims[1]; nx++ )
	       sbuff[nr++] = pcd[indx_pcd[ny]].ihr.peltier[nx];
     NADC_WR_HDF5_Dataset( compress, grp_id, "Peltier",
			  H5T_NATIVE_SHORT, 2, dims, sbuff );
     free( sbuff );
/*
 * Pre-disperser prism temperature
 */
     rbuff = (float *) malloc( dims[0] * sizeof( float ));
     if ( rbuff == NULL ) NADC_RETURN_ERROR( NADC_ERR_ALLOC, "rbuff" );
     for ( ny = 0; ny < dims[0]; ny++ )
	  rbuff[ny] = (float) (-1.721 + 6.104e-3 * 
			       pcd[indx_pcd[ny]].ihr.prism_temp);
     NADC_WR_HDF5_Dataset( compress, grp_id, "PrismTemperature",
			  H5T_NATIVE_FLOAT, 1, dims, rbuff );
     free( rbuff );
     (void) H5Gclose( grp_id );
/*
 * create group /EARTH/PCD
 */
     grp_id = H5Gcreate( param.hdf_file_id, "/EARTH/PMD", 
			 H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
     if ( grp_id < 0 ) 
	  NADC_RETURN_ERROR( NADC_ERR_HDF_GRP, "/EARTH/PMD" );
/*
 * write PMD geolocations (only for Earth observations)
 */
     dims[0] = nr_pcd * PMD_IN_GRID;
     if ( param.write_pmd_geo == PARAM_SET ) {
          type_id = H5Topen( param.hdf_file_id, "glr", H5P_DEFAULT );
          glr = (struct glr1_gome *)
               malloc( dims[0] * sizeof( struct glr1_gome ));
          if ( glr == NULL )
               NADC_RETURN_ERROR( NADC_ERR_ALLOC, "glr" );
          for ( nr = ny = 0; ny < (hsize_t) nr_pcd; ny++ ) {
	       for ( nx = 0; nx < PMD_IN_GRID; nx++ )
		    (void) memcpy( &glr[nr++], &pcd[indx_pcd[ny]].pmd[nx].glr,
				   sizeof( struct glr1_gome ) );
	  }
          NADC_WR_HDF5_Dataset( compress, grp_id, "glr",
                                type_id, 1, dims, glr );
          free( glr );
          (void) H5Tclose( type_id );
          if ( IS_ERR_STAT_FATAL )
               NADC_RETURN_ERROR( NADC_ERR_HDF_WR, "glr" );
     }
/*
 * write PMD values
 */
     dims[1] = PMD_NUMBER;
     nrpix = (size_t) (dims[0] * dims[1]);
     rbuff = (float *) malloc( nrpix * sizeof( float ));
     if ( rbuff == NULL ) NADC_RETURN_ERROR( NADC_ERR_ALLOC, "rbuff" );
     ni = 0; nr = 0;
     do {
	  ny = 0;
	  do {
	       nx = 0;
	       do {
		    rbuff[ni++] = pcd[indx_pcd[nr]].pmd[ny].value[nx];
	       } while ( ++nx < PMD_NUMBER );
	  } while ( ++ny < PMD_IN_GRID );
     } while ( ++nr < (hsize_t) nr_pcd );
     NADC_WR_HDF5_Dataset( compress, grp_id, "value",
                           H5T_NATIVE_FLOAT, 2, dims, rbuff );
     free( rbuff );
/*
 * close interface
 */
     (void) H5Gclose( grp_id );
}
