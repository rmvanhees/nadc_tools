/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2001 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_H5_IDL
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIAMACHY, HDF5, IDL
.LANGUAGE    ANSI C
.PURPOSE     IDL wrapper for writing SCIAMACHY data (hdf5)
.COMMENTS    None
.ENVIRONment None
.VERSION      1.0   17-Nov-2003	Created by R. M. van Hees 
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <hdf5.h>
#include <hdf5_hl.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_idl.h>

/*+++++ Static Variables +++++*/
static const char err_msg[] = "invalid number of function arguments";

/*+++++++++++++++++++++++++ Main Program or Functions +++++++++++++++*/
int IDL_STDCALL _SCIA_WR_H5_MEMCORR( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_WR_H5_MEMCORR";

     int     dimX, dimY;
     float   *memcorr;

     hid_t   file_id;
     hsize_t dims[2];

     const char mem_file[] = "MEMcorr.h5";
/*
 * check number of parameters
 */
     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     dimX = *(int *) argv[0];
     dimY = *(int *) argv[1];
     memcorr = (float *) argv[2];
/*
 * open output HDF5-file
 */
     file_id = H5Fcreate( mem_file, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT );
     if ( file_id < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, mem_file );
/*
 * write datasets
 */
     dims[0] = (hsize_t) dimY;
     dims[1] = (hsize_t) dimX;
     (void) H5LTmake_dataset_float( file_id, "MemTable", 2, dims, memcorr );
/*
 * write attributes
 */
     (void) H5LTset_attribute_string( file_id, "/", "Author", 
				      "G. Lichtenberg (SRON)" );
     (void) H5LTset_attribute_string( file_id, "/", "Reference", 
				      "SRON-SCIA-PhE-RP-11(issue 2)" );
     (void) H5LTset_attribute_string( file_id, "/", "History", 
				      "10/10/2003 - original release" );

     return (int) H5Fclose( file_id );
 done:
     return -1;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++*/
int IDL_STDCALL _SCIA_WR_H5_NLCORR( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_WR_H5_NLCORR";

     int     dimX, dimY;
     char    *CurveIndex;
     float   *nlcorr;

     hid_t   file_id;
     hsize_t dims[2];

     const char nl_file[] = "NLcorr.h5";
/*
 * check number of parameters
 */
     if ( argc != 4 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     dimX = *(int *) argv[0];
     dimY = *(int *) argv[1];
     CurveIndex = (char *) argv[2];
     nlcorr = (float *) argv[3];
/*
 * open output HDF5-file
 */
     file_id = H5Fcreate( nl_file, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT );
     if ( file_id < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, nl_file );
/*
 * write datasets
 */
     dims[0] = SCIENCE_PIXELS;
     (void) H5LTmake_dataset_char( file_id, "CurveIndex", 
				   1, dims, CurveIndex );

     dims[0] = (hsize_t) dimY;
     dims[1] = (hsize_t) dimX;
     (void) H5LTmake_dataset_float( file_id, "nLinTable", 2, dims, nlcorr );
/*
 * write attributes
 */
     (void) H5LTset_attribute_string( file_id, "/", "Author", 
				      "Q.L. Kleipool (SRON)" );
     (void) H5LTset_attribute_string( file_id, "/", "Reference", 
	       "SRON-SCIA-PhE-RP-13" );
     (void) H5LTset_attribute_string( file_id, "/", "History", 
	       "27/10/2003 - original release" );
     (void) H5LTset_attribute_string( file_id, "/", "History1", 
	       "04/11/2003 - solved oscillations at small PET" );
     (void) H5LTset_attribute_string( file_id, "/", "History2", 
	       "20/01/2004 - Changed format of H5-file, modified scale/offs" );

     return (int) H5Fclose( file_id );
 done:
     return -1;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++*/
int IDL_STDCALL _SCIA_WR_H5_STRAYLIGHT( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_WR_H5_STRAYLIGHT";

     int     dimX, dimY;
     float   *grid_out, *grid_in, *strayCorr;

     hid_t   file_id;
     hsize_t dims[2];

     const char stray_fl[] = "Straylight.h5";
/*
 * check number of parameters
 */
     if ( argc != 5 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     dimX = *(int *) argv[0];
     dimY = *(int *) argv[1];
     grid_in  = (float *) argv[2];
     grid_out = (float *) argv[3];
     strayCorr = (float *) argv[4];
/*
 * open output HDF5-file
 */
     file_id = H5Fcreate( stray_fl, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT );
     if ( file_id < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, stray_fl );
/*
 * write datasets
 */
     dims[0] = (hsize_t) dimX;
     (void) H5LTmake_dataset_float( file_id, "grid_in", 1, dims, grid_in );
     dims[0] = (hsize_t) dimY;
     (void) H5LTmake_dataset_float( file_id, "grid_out", 1, dims, grid_out );
     dims[0] = (hsize_t) dimY;
     dims[1] = (hsize_t) dimX;
     (void) H5LTmake_dataset_float( file_id, "strayCorr", 2, dims, strayCorr );
/*
 * write attributes
 */
     (void) H5LTset_attribute_string( file_id, "/", "Author", 
				      "R. Snel (SRON)" );
     (void) H5LTset_attribute_string( file_id, "/", "Reference", 
	       "SRON-SCIA-PhE-RP-22" );
     (void) H5LTset_attribute_string( file_id, "/", "History", 
	       "04/06/2010 - original release ADT V4.1" );

     return (int) H5Fclose( file_id );
 done:
     return -1;
}
