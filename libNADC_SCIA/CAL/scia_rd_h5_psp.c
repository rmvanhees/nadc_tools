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

.IDENTifer   SCIA_RD_H5_RSP
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     read Polarisation Sensitivity Parameters from external file
.COMMENTS    - contains SCIA_RD_H5_PSPN, SCIA_RD_H5_PSPL, SCIA_RD_H5_PSPO
.ENVIRONment None
.VERSION      1.0   01-Nov-2007 created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <stdlib.h>
#include <string.h>

#include <hdf5.h>
#include <hdf5_hl.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

/*+++++ Static Variables +++++*/
        /* NONE */

/*+++++ Macros +++++*/
        /* NONE */

/*+++++ Global Variables +++++*/
        /* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_RD_H5_PSPN
.PURPOSE     read Polarization Sensitivity Parameters (nadir) from HDF5 file
.INPUT/OUTPUT
  call as    num_psp = SCIA_RD_H5_PSPN( &pspn );
    output:
            struct pspn_scia **pspn :  Polarization Sensitivity Parameters
	                                (nadir)
.RETURNS     number of data set records read (unsigned short)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned short SCIA_RD_H5_PSPN( /*@out@*/ struct pspn_scia **pspn_out )
{
     const char prognm[] = "SCIA_RD_H5_PSPN";

     unsigned short num_psp = 0;

     register hsize_t ni;
     register unsigned short np;
     register unsigned int offs;

     char   psp_file[MAX_STRING_LENGTH];

     float *ang_esm, *mu2, *mu3;

     struct pspn_scia *pspn;

     hid_t  file_id = -1, grp_id = -1;
     H5T_class_t class_id[2];
     size_t  tsize[2];
     hsize_t dims[2];
/*
 * open output HDF5-file
 */
     (void) strcpy( psp_file, "./psp_patch.h5" );
     H5E_BEGIN_TRY {
          file_id = H5Fopen( psp_file, H5F_ACC_RDONLY, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( file_id < 0 ) {
          (void) snprintf( psp_file, MAX_STRING_LENGTH, 
                           "%s/psp_patch.h5", DATA_DIR );
          file_id = H5Fopen( psp_file, H5F_ACC_RDONLY, H5P_DEFAULT );
          if ( file_id < 0 )
               NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, psp_file );
     }
/*
 * open group /PSPN
 */
     if ( (grp_id = H5Gopen( file_id, "/PSPN", H5P_DEFAULT )) < 0 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_GRP, "/PSPN" );
/*
 * read data from HDF5-file into pspn structs
 */
     (void) H5LTget_dataset_info( grp_id, "elevation", dims, class_id, tsize );
     ang_esm = (float *) malloc( (size_t) dims[0] * sizeof(float) );
     if ( ang_esm == NULL ) NADC_GOTO_ERROR(prognm, NADC_ERR_ALLOC, "ang_esm");
     (void) H5LTread_dataset_float( grp_id, "elevation", ang_esm );

     (void) H5LTget_dataset_info( grp_id, "mu2", dims, class_id, tsize );
     mu2 = (float *) malloc( (size_t) (dims[0] * dims[1]) * sizeof(float) );
     if ( mu2 == NULL ) NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "mu2" );
     (void) H5LTread_dataset_float( grp_id, "mu2", mu2 );

     (void) H5LTget_dataset_info( grp_id, "mu3", dims, class_id, tsize );
     mu3 = (float *) malloc( (size_t) (dims[0] * dims[1]) * sizeof(float) );
     if ( mu3 == NULL ) NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "mu3" );
     (void) H5LTread_dataset_float( grp_id, "mu3", mu3 );

     if ( ! Use_Extern_Alloc ) {
	  pspn_out[0] = (struct pspn_scia *) 
	       malloc( (size_t) dims[0] * sizeof(struct pspn_scia));
     }
     if ( (pspn = pspn_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "pspn" );

     for ( offs = 0, ni = 0; ni < dims[0]; ni++ ) {
	  pspn[ni].ang_esm = ang_esm[ni];
	  for ( np = 0; np < SCIENCE_PIXELS; np++ ) {
	       pspn[ni].mu2[np] = mu2[offs];
	       pspn[ni].mu3[np] = mu3[offs++];
	  }
	  num_psp++;
     }
     free( ang_esm );
     free( mu2 );
     free( mu3 );
/*
 * give message to user
 */
     NADC_ERROR( prognm, NADC_ERR_NONE, 
                 "\n\tapplied auxiliary Polarization Sensitivity Parameters" );
 done:
     if ( grp_id >= 0 ) (void) H5Gclose( grp_id );
     if ( file_id >= 0 ) (void) H5Fclose( file_id );

     return num_psp;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_RD_H5_PSPL
.PURPOSE     read Polarization Sensitivity Parameters (limb) from external file
.INPUT/OUTPUT
  call as    num_psp = SCIA_RD_H5_PSPL( &pspl );
    output:
            struct psplo_scia **pspl :  Polarization Sensitivity Parameters
	                                (limb)
.RETURNS     number of data set records read (unsigned short)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned short SCIA_RD_H5_PSPL( /*@out@*/ struct psplo_scia **pspl_out )
{
     const char prognm[] = "SCIA_RD_H5_PSPL";

     unsigned short num_psp = 0;

     register hsize_t ni;
     register unsigned short np;
     register unsigned int offs;

     char   psp_file[MAX_STRING_LENGTH];

     float  *ang_asm, *ang_esm, *mu2, *mu3;

     struct psplo_scia *pspl;

     hid_t  file_id = -1, grp_id = -1;
     H5T_class_t class_id[2];
     size_t  tsize[2];
     hsize_t dims[2];
/*
 * open output HDF5-file
 */
     (void) strcpy( psp_file, "./psp_patch.h5" );
     H5E_BEGIN_TRY {
          file_id = H5Fopen( psp_file, H5F_ACC_RDONLY, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( file_id < 0 ) {
          (void) snprintf( psp_file, MAX_STRING_LENGTH, 
                           "%s/psp_patch.h5", DATA_DIR );
          file_id = H5Fopen( psp_file, H5F_ACC_RDONLY, H5P_DEFAULT );
          if ( file_id < 0 )
               NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, psp_file );
     }
/*
 * open group /PSPL
 */
     if ( (grp_id = H5Gopen( file_id, "/PSPL", H5P_DEFAULT )) < 0 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_GRP, "/PSPL" );
/*
 * read data from HDF5-file into pspl structs
 */
     (void) H5LTget_dataset_info( grp_id, "azimuth", dims, class_id, tsize );
     ang_asm = (float *) malloc( (size_t) dims[0] * sizeof(float));
     if ( ang_asm == NULL ) NADC_GOTO_ERROR(prognm, NADC_ERR_ALLOC, "ang_asm");
     (void) H5LTread_dataset_float( grp_id, "azimuth", ang_asm );

     (void) H5LTget_dataset_info( grp_id, "elevation", dims, class_id, tsize );
     ang_esm = (float *) malloc( (size_t) dims[0] * sizeof(float));
     if ( ang_esm == NULL ) NADC_GOTO_ERROR(prognm, NADC_ERR_ALLOC, "ang_esm");
     (void) H5LTread_dataset_float( grp_id, "elevation", ang_esm );

     (void) H5LTget_dataset_info( grp_id, "mu2", dims, class_id, tsize );
     mu2 = (float *) malloc( (size_t) (dims[0] * dims[1]) * sizeof(float));
     if ( mu2 == NULL ) NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "mu2" );
     (void) H5LTread_dataset_float( grp_id, "mu2", mu2 );

     (void) H5LTget_dataset_info( grp_id, "mu3", dims, class_id, tsize );
     mu3 = (float *) malloc( (size_t) (dims[0] * dims[1]) * sizeof(float) );
     if ( mu3 == NULL ) NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "mu3" );
     (void) H5LTread_dataset_float( grp_id, "mu3", mu3 );

     if ( ! Use_Extern_Alloc ) {
	  pspl_out[0] = (struct psplo_scia *) 
	       malloc( (size_t) dims[0] * sizeof(struct psplo_scia));
     }
     if ( (pspl = pspl_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "pspl" );
     for ( offs = 0, ni = 0; ni < dims[0]; ni++ ) {
	  pspl[ni].ang_asm = ang_asm[ni];
	  pspl[ni].ang_esm = ang_esm[ni];
	  for ( np = 0; np < SCIENCE_PIXELS; np++ ) {
	       pspl[ni].mu2[np] = mu2[offs];
	       pspl[ni].mu3[np] = mu3[offs++];
	  }
	  num_psp++;
     }
     free( ang_asm );
     free( ang_esm );
     free( mu2 );
     free( mu3 );
/*
 * give message to user
 */
     NADC_ERROR( prognm, NADC_ERR_NONE,
                 "\n\tapplied auxiliary Polarization Sensitivity Parameters" );
 done:
     if ( grp_id >= 0 ) (void) H5Gclose( grp_id );
     if ( file_id >= 0 ) (void) H5Fclose( file_id );

     return num_psp;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_RD_H5_PSPO
.PURPOSE     read Polarization Sensitivity Parameters (limb) from external file
.INPUT/OUTPUT
  call as    num_psp = SCIA_RD_H5_PSPO( &pspo );
    output:
            struct psplo_scia **pspo :  Polarization Sensitivity Parameters
	                                (limb)
.RETURNS     number of data set records read (unsigned short)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned short SCIA_RD_H5_PSPO( /*@out@*/ struct psplo_scia **pspo_out )
{
     const char prognm[] = "SCIA_RD_H5_PSPO";

     unsigned short num_psp = 0;

     register hsize_t ni;
     register unsigned short np;
     register unsigned int offs;

     char   psp_file[MAX_STRING_LENGTH];

     float  *ang_asm, *ang_esm, *mu2, *mu3;

     struct psplo_scia *pspo;

     hid_t  file_id = -1, grp_id = -1;
     H5T_class_t class_id[2];
     size_t  tsize[2];
     hsize_t dims[2];
/*
 * open output HDF5-file
 */
     (void) strcpy( psp_file, "./psp_patch.h5" );
     H5E_BEGIN_TRY {
          file_id = H5Fopen( psp_file, H5F_ACC_RDONLY, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( file_id < 0 ) {
          (void) snprintf( psp_file, MAX_STRING_LENGTH, 
                           "%s/psp_patch.h5", DATA_DIR );
          file_id = H5Fopen( psp_file, H5F_ACC_RDONLY, H5P_DEFAULT );
          if ( file_id < 0 )
               NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, psp_file );
     }
/*
 * open group /PSPO
 */
     if ( (grp_id = H5Gopen( file_id, "/PSPO", H5P_DEFAULT )) < 0 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_GRP, "/PSPO" );
/*
 * read data from HDF5-file into pspo structs
 */
     (void) H5LTget_dataset_info( grp_id, "azimuth", dims, class_id, tsize );
     ang_asm = (float *) malloc( (size_t) dims[0] * sizeof(float) );
     if ( ang_asm == NULL ) NADC_GOTO_ERROR(prognm, NADC_ERR_ALLOC, "ang_asm");
     (void) H5LTread_dataset_float( grp_id, "azimuth", ang_asm );

     (void) H5LTget_dataset_info( grp_id, "elevation", dims, class_id, tsize );
     ang_esm = (float *) malloc( (size_t) dims[0] * sizeof(float) );
     if ( ang_esm == NULL ) NADC_GOTO_ERROR(prognm, NADC_ERR_ALLOC, "ang_esm");
     (void) H5LTread_dataset_float( grp_id, "elevation", ang_esm );

     (void) H5LTget_dataset_info( grp_id, "mu2", dims,class_id,tsize );
     mu2 = (float *) malloc( (size_t) (dims[0] * dims[1]) * sizeof(float) );
     if ( mu2 == NULL ) NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "mu2" );
     (void) H5LTread_dataset_float( grp_id, "mu2", mu2 );

     (void) H5LTget_dataset_info( grp_id, "mu3", dims,class_id,tsize );
     mu3 = (float *) malloc( (size_t) (dims[0] * dims[1]) * sizeof(float) );
     if ( mu3 == NULL ) NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "mu3" );
     (void) H5LTread_dataset_float( grp_id, "mu3", mu3 );

     if ( ! Use_Extern_Alloc ) {
	  pspo_out[0] = (struct psplo_scia *) 
	       malloc( (size_t) dims[0] * sizeof(struct psplo_scia));
     }
     if ( (pspo = pspo_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "pspo" );
     for ( offs = 0, ni = 0; ni < dims[0]; ni++ ) {
	  pspo[ni].ang_asm = ang_asm[ni];
	  pspo[ni].ang_esm = ang_esm[ni];
	  for ( np = 0; np < SCIENCE_PIXELS; np++ ) {
	       pspo[ni].mu2[np] = mu2[offs];
	       pspo[ni].mu3[np] = mu3[offs++];
	  }
	  num_psp++;
     }
     free( ang_asm );
     free( ang_esm );
     free( mu2 );
     free( mu3 );
/*
 * give message to user
 */
     NADC_ERROR( prognm, NADC_ERR_NONE, 
                 "\n\tapplied auxiliary Polarization Sensitivity Parameters" );
 done:
     if ( grp_id >= 0 ) (void) H5Gclose( grp_id );
     if ( file_id >= 0 ) (void) H5Fclose( file_id );

     return num_psp;
}
