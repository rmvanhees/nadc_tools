/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2010 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_H5_RD_KEYDATA
.AUTHOR      R.M. van Hees
.KEYWORDS    
.LANGUAGE    ANSI C
.PURPOSE     read Polarisation-/Radiance-Sensitivity Parameters 
                  and Sun Reference Spectra from external file
.INPUT/OUTPUT
  call as    Read_SCIA_H5_RSPN( scia_fl, &keydata );

     input:  
            char scia_fl : name of level 1b product (no path!)
    output:  
            struct keydata_rec *keydata :  pspn, rspn, pspl, rspl 
                                           and srs keydata
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     1.0     22-Nov-2010   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _POSIX_C_SOURCE  200112L

/*+++++ System headers +++++*/
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <hdf5.h>

/*+++++ Local Headers +++++*/
#define _SCIA_PATCH_1
#include <nadc_scia_patch.h>

/*+++++ Macros +++++*/
        /* NONE */

/*+++++ Global Variables +++++*/
        /* NONE */

/*+++++ Static Variables +++++*/
        /* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
                                /* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_H5_RD_KEYDATA( const char scia_fl[], 
			 /*@out@*/ struct keydata_rec *keydata )
{
     const char prognm[] = "SCIA_H5_RD_KEYDATA";

     char   key_file[MAX_STRING_LENGTH];

     hid_t   file_id = -1;
     hid_t   dset = -1;
     hid_t   space = -1;
     hid_t   arr_id, arr_c_id;
     hid_t   pspntype, pspltype, rspntype, rspltype, srstype;
     hid_t   memtype = -1;
     herr_t  ierr;
     hsize_t dims[2];

     struct stat sb;
/*
 * create the compound datatype for memory.
 */
     dims[0] = SCIENCE_PIXELS;
     arr_id = H5Tarray_create( H5T_NATIVE_DOUBLE, 1, dims );
     if ( arr_id < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "arr_id" );
     pspntype = H5Tcreate( H5T_COMPOUND, sizeof( struct pspn_scia ) );
     ierr = H5Tinsert( pspntype, "ELEVATION",
		       HOFFSET( struct pspn_scia, ang_esm), H5T_NATIVE_FLOAT );
     if ( ierr < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "elevation" );
     ierr = H5Tinsert( pspntype, "MU2",
		       HOFFSET( struct pspn_scia, mu2), arr_id );
     if ( ierr < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "mu2" );
     ierr = H5Tinsert( pspntype, "MU3",
		       HOFFSET( struct pspn_scia, mu3), arr_id );
     if ( ierr < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "mu3" );

     rspntype = H5Tcreate( H5T_COMPOUND, sizeof( struct rspn_scia ) );
     ierr = H5Tinsert( rspntype, "ELEVATION",
		       HOFFSET( struct rspn_scia, ang_esm), H5T_NATIVE_FLOAT );
     if ( ierr < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "elevation" );
     ierr = H5Tinsert( rspntype, "SENSITIVITY",
		       HOFFSET( struct rspn_scia, sensitivity), arr_id );
     if ( ierr < 0 ) NADC_GOTO_ERROR(prognm, NADC_ERR_HDF_DTYPE, "sensitivity");

     pspltype = H5Tcreate( H5T_COMPOUND, sizeof( struct psplo_scia ) );
     ierr = H5Tinsert( pspltype, "ELEVATION",
		       HOFFSET( struct psplo_scia, ang_esm), H5T_NATIVE_FLOAT );
     if ( ierr < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "elevation" );
     ierr = H5Tinsert( pspltype, "AZIMUTH",
		       HOFFSET( struct psplo_scia, ang_asm), H5T_NATIVE_FLOAT );
     if ( ierr < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "azimuth" );
     ierr = H5Tinsert( pspltype, "MU2",
		       HOFFSET( struct psplo_scia, mu2), arr_id );
     if ( ierr < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "mu2" );
     ierr = H5Tinsert( pspltype, "MU3",
		       HOFFSET( struct psplo_scia, mu3), arr_id );
     if ( ierr < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "mu3" );

     rspltype = H5Tcreate( H5T_COMPOUND, sizeof( struct rsplo_scia ) );
     ierr = H5Tinsert( rspltype, "ELEVATION",
		       HOFFSET( struct rsplo_scia, ang_esm), H5T_NATIVE_FLOAT );
     if ( ierr < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "elevation" );
     ierr = H5Tinsert( rspltype, "AZIMUTH",
		       HOFFSET( struct rsplo_scia, ang_asm), H5T_NATIVE_FLOAT );
     if ( ierr < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "azimuth" );
     ierr = H5Tinsert( rspltype, "SENSITIVITY",
		       HOFFSET( struct rsplo_scia, sensitivity), arr_id );
     if ( ierr < 0 ) NADC_GOTO_ERROR(prognm, NADC_ERR_HDF_DTYPE, "sensitivity");

     (void) H5Tclose( arr_id );
     dims[0] = SCIENCE_PIXELS;
     arr_id = H5Tarray_create( H5T_NATIVE_FLOAT, 1, dims );
     if ( arr_id < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "arr_id" );
     arr_c_id = H5Tcopy( H5T_C_S1 );
     if ( arr_c_id < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "arr_c_id" );
     (void) H5Tset_size( arr_c_id, 3 );
     srstype = H5Tcreate( H5T_COMPOUND, sizeof( struct srs_scia ) );
     ierr = H5Tinsert( srstype, "SUN_SPEC_ID",
		       HOFFSET( struct srs_scia, sun_spec_id), arr_c_id );
     if ( ierr < 0 ) NADC_GOTO_ERROR(prognm, NADC_ERR_HDF_DTYPE, "sun_spec_id");
     ierr = H5Tinsert( srstype, "AVG_AZIM",
		       HOFFSET( struct srs_scia, avg_asm), H5T_NATIVE_FLOAT );
     if ( ierr < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "avg_azim" );
     ierr = H5Tinsert( srstype, "AVG_ELEV",
		       HOFFSET( struct srs_scia, avg_esm), H5T_NATIVE_FLOAT );
     if ( ierr < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "avg_elev" );
     ierr = H5Tinsert( srstype, "AVG_ELE_ANG",
		       HOFFSET( struct srs_scia, avg_elev_sun), H5T_NATIVE_FLOAT );
     if ( ierr < 0 ) NADC_GOTO_ERROR(prognm, NADC_ERR_HDF_DTYPE, "avg_ele_ang");
     ierr = H5Tinsert( srstype, "DOPP_SHIFT",
		       HOFFSET( struct srs_scia, dopp_shift), H5T_NATIVE_FLOAT );
     if ( ierr < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "dopp_shift" );
     ierr = H5Tinsert( srstype, "WVLEN_REF_SPEC",
		       HOFFSET( struct srs_scia, wvlen_sun), arr_id );
     if ( ierr < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "wvlen_sun" );
     ierr = H5Tinsert( srstype, "MEAN_REF_SPEC",
		       HOFFSET( struct srs_scia, mean_sun), arr_id );
     if ( ierr < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "mean_sun" );
     ierr = H5Tinsert( srstype, "RAD_PRE_REF_SPEC",
		       HOFFSET( struct srs_scia, precision_sun), arr_id );
     if ( ierr < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "precision" );
     ierr = H5Tinsert( srstype, "RAD_ACC_REF_SPEC",
		       HOFFSET( struct srs_scia, accuracy_sun), arr_id );
     if ( ierr < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "accuracy" );
     ierr = H5Tinsert( srstype, "ETALON",
		       HOFFSET( struct srs_scia, etalon), arr_id );
     if ( ierr < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "etalon" );
     (void) H5Tclose( arr_id );
     dims[0] = PMD_NUMBER;
     arr_id = H5Tarray_create( H5T_NATIVE_FLOAT, 1, dims );
     if ( arr_id < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "arr_id" );
     ierr = H5Tinsert( srstype, "PMD_MEAN",
		       HOFFSET( struct srs_scia, pmd_mean), arr_id );
     if ( ierr < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "pmd_mean" );
     ierr = H5Tinsert( srstype, "PMD_OUT_ND_OUT",
		       HOFFSET( struct srs_scia, pmd_out_nd_out), arr_id );
     if ( ierr < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "pmd_out" );
     ierr = H5Tinsert( srstype, "PMD_OUT_ND_IN",
		       HOFFSET( struct srs_scia, pmd_out_nd_in), arr_id );
     if ( ierr < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "pmd_out" );
     (void) H5Tclose( arr_id );
     (void) H5Tclose( arr_c_id );

     memtype = H5Tcreate( H5T_COMPOUND, sizeof( struct keydata_rec ) );
     dims[0] = NUM_KEY_PSPN;
     arr_id = H5Tarray_create( pspntype, 1, dims );
     if ( arr_id < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "arr_id" );
     ierr = H5Tinsert( memtype, "PSPN",
		       HOFFSET( struct keydata_rec, pspn), arr_id );
     if ( ierr < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "PSPN" );
     (void) H5Tclose( arr_id );
     dims[0] = NUM_KEY_RSPN;
     arr_id = H5Tarray_create( rspntype, 1, dims );
     if ( arr_id < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "arr_id" );
     ierr = H5Tinsert( memtype, "RSPN",
		       HOFFSET( struct keydata_rec, rspn), arr_id );
     if ( ierr < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "RSPN" );
     (void) H5Tclose( arr_id );
     dims[0] = NUM_KEY_PSPL;
     arr_id = H5Tarray_create( pspltype, 1, dims );
     if ( arr_id < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "arr_id" );
     ierr = H5Tinsert( memtype, "PSPL",
		       HOFFSET( struct keydata_rec, pspl), arr_id );
     if ( ierr < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "PSPL" );
     (void) H5Tclose( arr_id );
     dims[0] = NUM_KEY_RSPL;
     arr_id = H5Tarray_create( rspltype, 1, dims );
     if ( arr_id < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "arr_id" );
     ierr = H5Tinsert( memtype, "RSPL",
		       HOFFSET( struct keydata_rec, rspl), arr_id );
     if ( ierr < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "RSPL" );
     (void) H5Tclose( arr_id );
     dims[0] = NUM_KEY_SRS;
     arr_id = H5Tarray_create( srstype, 1, dims );
     if ( arr_id < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "arr_id" );
     ierr = H5Tinsert( memtype, "SRS",
		       HOFFSET( struct keydata_rec, srs), arr_id );
     if ( ierr < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "SRS" );
     (void) H5Tclose( arr_id );
     (void) H5Tclose( pspntype );
     (void) H5Tclose( rspntype );
     (void) H5Tclose( pspltype );
     (void) H5Tclose( rspltype );
     (void) H5Tclose( srstype );
/*
 * open output HDF5-file
 */
     (void) snprintf( key_file, MAX_STRING_LENGTH, 
		      "%s/%s%s", DATA_DIR, scia_fl, ".h5" );
     if ( stat( key_file, &sb ) == -1 ) {
	  (void) snprintf( key_file, MAX_STRING_LENGTH, 
			   "%s/%s%s", ".", scia_fl, ".h5" );
	  if ( stat( key_file, &sb ) == -1 )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, key_file );
     }
     if ( (file_id = H5Fopen( key_file, H5F_ACC_RDONLY, H5P_DEFAULT )) < 0 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, key_file );
/*
 * read data from HDF5-file into rspn structs
 */
     dset = H5Dopen( file_id, "newkeydata", H5P_DEFAULT);
     if ( dset < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_RD, "newkeydata" );
     space = H5Dget_space( dset );
     (void) H5Sget_simple_extent_dims( space, dims, NULL );

     ierr = H5Dread( dset, memtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, keydata );
     if ( ierr < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_RD, "newkeydata" );
/*
 * give message to user
 */
     NADC_ERROR( prognm, NADC_ERR_NONE, 
                 "\n\tapplied auxiliary RSP, PSP and SRS keydata" );
 done:
     if ( memtype > 0 ) (void) H5Tclose( memtype );
     if ( space > 0 ) (void) H5Sclose( space );
     if ( dset > 0 ) (void) H5Dclose( dset );
     if ( file_id >= 0 ) (void) H5Fclose( file_id );
}
