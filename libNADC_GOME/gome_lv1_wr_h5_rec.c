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

.IDENTifer   GOME_LV1_WR_H5_REC
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write GOME level 1 REC data

.INPUT/OUTPUT
  call as    GOME_LV1_WR_H5_REC( flag_origin, param, band, 
                                 nr_rec, bcr_start, bcr_count, rec );
  input: 
             unsigned char flag_origin : FLAG_EARTH/FLAG_MOON/FLAG_SUN
	     
	     short  nband              : band index [1a,1b,2a,2b,3,4,...]
             struct param_record param : command-line parameters
	     short  nr_rec             : number of band data records
	     short  bcr_start,bcr_count: number of pixels per spectral band
	     struct rec_gome *rec    : Spectral Band Data Records

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      2.0   11-Nov-2001	moved to the new Error handling routines, RvH
              1.3   20-Jul-2001	modified rec_gome, RvH 
             1.2     04-Nov-2000   let this module create its own groups, RvH
             1.1     25-Feb-2000   changed HDF-structure, RvH
             1.0     29-Jun-1999   Created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System headers +++++*/
#include <stdlib.h>
#include <math.h>

#include <hdf5.h>

/*+++++ Local Headers +++++*/
#define _GOME_COMMON
#include <nadc_gome.h>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void GOME_LV1_WR_H5_REC( unsigned char flag_origin, short nband,
			 struct param_record param, 
			 short nr_rec, short bcr_start, short bcr_count, 
			 const struct rec_gome *rec )
{
     register hsize_t nr, nx, ny;

     unsigned char  *cbuff;
     unsigned short *usbuff;
     short          *sbuff;
     float          *rbuff;

     hid_t   grp_id, sub_grp_id;

     hbool_t compress;
     hsize_t nrpix, dims[2];

     const char prognm[] = "GOME_LV1_WR_H5_REC";
     const char *band_names[] = { 
	  "Band-1a", "Band-1b", "Band-2a", "Band-2b", "Band-3", 
	  "Band-4", "Blind", "Stray-1a", "Stray-1b", "Stray-2a"
     };
/*
 * check number of PCD records
 */
     if ( nr_rec == 0 ) return;
/*
 * set H5 boolean variable for compression
 */
     if ( param.flag_deflate == PARAM_SET )
	  compress = TRUE;
     else
	  compress = FALSE;
/*
 * open or create group Earth, SUN or MOON
 */
     if ( flag_origin == FLAG_EARTH ) {
	  if ( (grp_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/EARTH" )) < 0 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "/EARTH" );
     } else if ( flag_origin == FLAG_SUN ) {
	  if ( (grp_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/SUN" )) < 0 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "/SUN" );
     } else if ( flag_origin == FLAG_MOON ) {
	  if ( (grp_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/MOON" )) < 0 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "/MOON" );
     } else
	  NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "unknown data source" );

     sub_grp_id = H5Gcreate( grp_id, band_names[nband], 
			     H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
     if ( sub_grp_id < 0 ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, band_names[nband] );
/*
 * +++++ create/write attributes in the /PCD/BDR-group
 */
     dims[0] = 1;
     NADC_WR_HDF5_Attribute( sub_grp_id, "IntegrationIntervals", 
			    H5T_NATIVE_SHORT, 1, dims, &nr_rec );
     NADC_WR_HDF5_Attribute( sub_grp_id, "LengthSpectralBand", 
			    H5T_NATIVE_SHORT, 1, dims, &bcr_count );
/*
 * +++++ create/write datasets in the /PCD/BDR-group
 */
     dims[0] = nr_rec;
/*
 * Quality flags
 */
     cbuff = (unsigned char *) malloc( dims[0] * sizeof( unsigned char ));
     if ( cbuff == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "cbuff" );
     for ( nx = 0; nx < dims[0]; nx++ )
	  cbuff[nx] = (unsigned char) rec[nx].pixel_flags.flag_fields.dead;
     NADC_WR_HDF5_Dataset( compress, sub_grp_id, "FlagDeadPixels", 
			  H5T_NATIVE_UCHAR, 1, dims, cbuff );
     for ( nx = 0; nx < dims[0]; nx++ )
	  cbuff[nx] = (unsigned char) rec[nx].pixel_flags.flag_fields.hot;
     NADC_WR_HDF5_Dataset( compress, sub_grp_id, "FlagHotPixels", 
			  H5T_NATIVE_UCHAR, 1, dims, cbuff );
     for ( nx = 0; nx < dims[0]; nx++ )
	  cbuff[nx] = (unsigned char) rec[nx].pixel_flags.flag_fields.saturate;
     NADC_WR_HDF5_Dataset( compress, sub_grp_id, "FlagSaturatePixels", 
			  H5T_NATIVE_UCHAR, 1, dims, cbuff );
     for ( nx = 0; nx < dims[0]; nx++ )
	  cbuff[nx] = (unsigned char) rec[nx].pixel_flags.flag_fields.spectral;
     NADC_WR_HDF5_Dataset( compress, sub_grp_id, "FlagSpectralPixels", 
			  H5T_NATIVE_UCHAR, 1, dims, cbuff );
     free( cbuff );
/*
 * Indices
 */
     sbuff = (short *) malloc( dims[0] * sizeof( short ));
     if ( sbuff == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "sbuff" );
     for ( nx = 0; nx < dims[0]; nx++ )
	  sbuff[nx] = rec[nx].indx_psp;
     NADC_WR_HDF5_Dataset( compress, sub_grp_id, "IndexPolarisation", 
			  H5T_NATIVE_SHORT, 1, dims, sbuff );
     free( sbuff );
/*
 * Integration Times
 */
     rbuff = (float *) malloc( dims[0] * sizeof( float ));
     if ( rbuff == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "rbuff" );
     if ( nband == BAND_1b || nband == BAND_2b )
	  for ( nx = 0; nx < dims[0]; nx++ )
	       rbuff[nx] = rec[nx].integration[1];
     else
	  for ( nx = 0; nx < dims[0]; nx++ )
	       rbuff[nx] = rec[nx].integration[0];
     NADC_WR_HDF5_Dataset( compress, sub_grp_id, "IntegrationTime", 
			  H5T_NATIVE_FLOAT, 1, dims, rbuff );
     free( rbuff );
/*
 * Spectral data
 */
     dims[0] = nr_rec; dims[1] = bcr_count;
     nrpix = dims[0] * dims[1];

     if ( (flag_origin == FLAG_EARTH && param.calib_earth == CALIB_NONE)
	  || (flag_origin == FLAG_MOON && param.calib_moon == CALIB_NONE)
	  || (flag_origin == FLAG_SUN && param.calib_sun == CALIB_NONE)) {
	  usbuff = (unsigned short *) 
	       malloc( nrpix * sizeof( unsigned short ));
	  if ( usbuff == NULL ) 
	       NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "usbuff" );
	  for ( nr = ny = 0; ny < dims[0]; ny++ )
	       for ( nx = 0; nx < dims[1]; nx++ )
		    usbuff[nr++] = (unsigned short) rec[ny].data[nx+bcr_start];
	  NADC_WR_HDF5_Dataset( compress, sub_grp_id, "DataValues", 
			       H5T_NATIVE_USHORT, 2, dims, usbuff );
	  free( usbuff );
     } else {
	  if ( (rbuff = (float *) malloc( nrpix * sizeof( float ))) == NULL ) 
	       NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "rbuff" );
	  for ( nr = ny = 0; ny < dims[0]; ny++ )
	       for ( nx = 0; nx < dims[1]; nx++ )
		    rbuff[nr++] = rec[ny].data[nx+bcr_start];
	  NADC_WR_HDF5_Dataset( compress, sub_grp_id, "DataValues", 
			       H5T_NATIVE_FLOAT, 2, dims, rbuff );
	  free( rbuff );
     }
/*
 * close the interface
 */
     (void) H5Gclose( sub_grp_id );
     (void) H5Gclose( grp_id );
}


