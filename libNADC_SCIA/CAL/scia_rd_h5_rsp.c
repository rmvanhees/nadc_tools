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
.PURPOSE     read Radiance Sensitivity Parameters from external file
.COMMENTS    - contains SCIA_RD_H5_RSPN, SCIA_RD_H5_RSPL, SCIA_RD_H5_RSPO
             and SCIA_RD_H5_RSPD
.ENVIRONment None
.VERSION      1.0   31-Oct-2007 created by R. M. van Hees
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
static 
void Read_SCIA_H5_RSPD_key_ppg0( hid_t file_id, const char *group_nm,
				 /*@out@*/ float *data )
{
     const char prognm[] = "Read_SCIA_H5_RSPD_key_ppg0";

     hid_t grp_id;
     hsize_t adim;

     /* open group */
     if ( (grp_id = H5Gopen( file_id, group_nm, H5P_DEFAULT )) < 0 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_GRP, group_nm );
     /* read data_info */
     (void) H5LTget_dataset_info( grp_id, group_nm, &adim, NULL, NULL );
     if (adim != SCIENCE_PIXELS)
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, group_nm );
     /* read data */
     (void) H5LTread_dataset_float( grp_id, group_nm, data );
 done:
     if ( grp_id >= 0 ) (void) H5Gclose( grp_id );
}

static 
void Read_SCIA_H5_RSPD_key_fix_sub( hid_t file_id, const char *group_nm,
				    /*@out@*/ float *wl, 
				    /*@out@*/ float *data )
{
     const char prognm[] = "Read_SCIA_H5_RSPD_key_fix_sub";

     hid_t grp_id;
     hsize_t adim;

     /* open group */
     if ( (grp_id = H5Gopen( file_id, group_nm, H5P_DEFAULT  )) < 0 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_GRP, group_nm );
     /* read WL info */
     (void) H5LTget_dataset_info( grp_id, "Axis 1  Wavelength", &adim, 
				  NULL, NULL );
     if (adim != SCIENCE_PIXELS)
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, group_nm );
     /* read Axis */
     (void) H5LTread_dataset_float( grp_id, "Axis 1  Wavelength", wl );
     /* read data info */
     (void) H5LTget_dataset_info( grp_id, group_nm, &adim, NULL, NULL );
     if (adim != SCIENCE_PIXELS)
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, group_nm );
     /* read data */
     (void) H5LTread_dataset_float( grp_id, group_nm, data );
 done:
     if ( grp_id >= 0 ) (void) H5Gclose( grp_id );
}


static 
void Read_SCIA_H5_RSPD_key_fix( hid_t file_id, 
				/*@out@*/ struct rspd_key_fix_scia *key_fix )
{
     Read_SCIA_H5_RSPD_key_ppg0( file_id, "PPG0",  key_fix->PPG0 );
     Read_SCIA_H5_RSPD_key_fix_sub( file_id, "OBM_s_p", key_fix->wl, 
				    key_fix->OBM_s_p );
     Read_SCIA_H5_RSPD_key_fix_sub( file_id, "ABS_RAD", key_fix->wl, 
				    key_fix->ABS_RAD );
     Read_SCIA_H5_RSPD_key_fix_sub( file_id, "NDF"    , key_fix->wl, 
				    key_fix->NDF );
     Read_SCIA_H5_RSPD_key_fix_sub( file_id, "NDF_s_p", key_fix->wl, 
				    key_fix->NDF_s_p );
}

static
void Read_SCIA_H5_RSPD_axis( const hid_t grp_id, const char *axis_name,
			     /*@out@*/ size_t *dims, /*@out@*/ float **data )
{
     const char prognm[] = "Read_SCIA_H5_RSPD_axis";

     hsize_t adim;

     (void) H5LTget_dataset_info( grp_id, axis_name, &adim, NULL, NULL );
     *dims = (size_t) adim;
     *data = (float *) malloc( (*dims) * sizeof(float) );
     if ( (*data) == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "data" );
     (void) H5LTread_dataset_float( grp_id, axis_name, *data );
}

static
void Read_SCIA_H5_RSPD_sensitivity( const hid_t grp_id, const char *data_name,
				    /*@out@*/ int *rank, 
				    /*@out@*/ size_t *dims, 
				    /*@out@*/ float **data )
{
     const char prognm[] = "Read_SCIA_H5_RSPD_sensitivity";

     register int nr; 

     size_t  n_elements = 1;
     hsize_t adim[3];

     /* get rank */
     H5LTget_dataset_ndims( grp_id, data_name, rank );
     /* get dimension */
     (void) H5LTget_dataset_info( grp_id, data_name, adim, NULL, NULL );
     /* calc elements */
     for ( nr = 0; nr < (*rank); nr++ ) {
	  dims[nr] = (size_t) adim[nr];
	  n_elements *= dims[nr];
     }
     /* reserve memory */
     *data = (float *) malloc( (size_t) n_elements * sizeof(float) );
     if ( (*data) == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "data" );
     /* read dataset */
     (void) H5LTread_dataset_float( grp_id, data_name, *data );
}

static
void Read_SCIA_H5_RSPD_el_az( hid_t file_id, const char *group_nm,
			      /*@out@*/ size_t *n_el_az, 
			      /*@out@*/ struct rspd_EL_AZ_scia **el_az_out )
{
     const char prognm[] = "Read_SCIA_H5_RSPD_el_az";

     register size_t ni, n2, n3, offs;

     struct rspd_EL_AZ_scia *el_az;
     float  *wl = NULL;
     float  *ang_esm = NULL;
     float  *ang_az  = NULL;
     float  *sensitivity = NULL;

     hid_t  grp_id = -1;
     size_t dims[3];
     size_t dims_sens[3];
     int rank;
/*
 * open group /RSPL
 */
     if ( (grp_id = H5Gopen( file_id, group_nm, H5P_DEFAULT )) < 0 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_GRP, group_nm );
/*
 * read data from HDF5-file into rspl structs
 */
     Read_SCIA_H5_RSPD_axis( grp_id, "Axis 1  wavelength", 
			     &dims[2], &wl);
     Read_SCIA_H5_RSPD_axis( grp_id, "Axis 2  elevation angle", 
			     &dims[1], &ang_esm);
     Read_SCIA_H5_RSPD_axis( grp_id, "Axis 3  azimuth angle",
			     &dims[0], &ang_az);

     Read_SCIA_H5_RSPD_sensitivity( grp_id ,group_nm,
				    &rank, dims_sens, &sensitivity );

     if ( ! Use_Extern_Alloc ) {
	  el_az_out[0] = (struct rspd_EL_AZ_scia *) 
	       malloc( dims[0] * dims[1] * sizeof( struct rspd_EL_AZ_scia ));
     }
     if ( (el_az_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "el_az" );
     el_az = el_az_out[0];
     
     for ( n3 = 0; n3 < dims[0]; n3++ ) {
	  for ( n2 = 0; n2 < dims[1]; n2++ ) {
	       ni = n3 * (unsigned int) dims[1] + n2;
	       el_az[ni].n_wl = dims[2];
	       el_az[ni].elevat_angle  = ang_esm[n2];
	       el_az[ni].azimuth_angle = ang_az  [n3];
	       (void) memcpy (el_az[ni].wl, wl, dims[2] * sizeof(float) );
	       offs = n3 * dims[1] * dims[2] + n2 * dims[2];
	       (void) memcpy( el_az[ni].sensitivity, 
			      sensitivity + offs,
			      dims[2] * sizeof(float) );
	  }
     }
     *n_el_az = dims[0] * dims[1]; 
 done:
     if ( wl != NULL ) free( wl );
     if ( ang_esm != NULL ) free( ang_esm );
     if ( ang_az != NULL ) free( ang_az  ); 
     if ( sensitivity != NULL ) free( sensitivity );
     if ( grp_id >= 0 ) (void) H5Gclose( grp_id );
}

static
void Read_SCIA_H5_RSPD_brdf( hid_t file_id, const char *group_nm,
			     /*@out@*/ size_t *n_brdf, 
			     /*@out@*/ struct rspd_BRDF_scia **brdf_out )
{
     const char prognm[] = "Read_SCIA_H5_RSPD_brdf";

     register size_t ni, n2, n3, offs;

     struct rspd_BRDF_scia *brdf;

     float  *wl = NULL;
     float  *ang_esm = NULL;
     float  *ang_asm  = NULL;
     float  *sensitivity = NULL;

     hid_t  grp_id = -1;
     size_t dims[3];
     size_t dims_sens[3];
     int    rank;

     if ( (grp_id = H5Gopen( file_id, group_nm, H5P_DEFAULT )) < 0 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_GRP, group_nm );
/*
 * read data from HDF5-file into rspl structs
 */
     Read_SCIA_H5_RSPD_axis( grp_id, "Axis 1  vacuum wavelength",
			     &dims[2], &wl );
     Read_SCIA_H5_RSPD_axis( grp_id, "Axis 2  elevation angle",
			     &dims[1], &ang_esm );
     Read_SCIA_H5_RSPD_axis( grp_id, "Axis 3  ASM angle",
			     &dims[0], &ang_asm );
     Read_SCIA_H5_RSPD_sensitivity( grp_id, group_nm, &rank,
				    dims_sens, &sensitivity );

     if ( ! Use_Extern_Alloc ) {
	  brdf_out[0] = (struct rspd_BRDF_scia *) 
	       malloc( dims[0] * dims[1] * sizeof( struct rspd_BRDF_scia ));
     }
     if ( (brdf_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "brdf" );
     brdf = brdf_out[0];
     
     for ( n3 = 0; n3 < dims[0]; n3++ ) {
	  for ( n2 = 0; n2 < dims[1]; n2++ ) {
	       ni = n3 * dims[1] + n2;
	       brdf[ni].n_wl = dims[2];
	       brdf[ni].elevat_angle  = ang_esm[n2];
	       brdf[ni].asm_angle = ang_asm[n3];
	       (void) memcpy( brdf[ni].wl, wl, dims[2] * sizeof(float) );
	       offs = n3 * dims[1] * dims[2] + n2 * dims[2];
	       (void) memcpy( brdf[ni].sensitivity, sensitivity + offs,
			      dims[2] * sizeof(float) );
	  }
     }
     *n_brdf = dims[0] * dims[1]; 
 done:
     if ( wl != NULL ) free( wl );
     if ( ang_esm != NULL ) free( ang_esm );
     if ( ang_asm != NULL ) free( ang_asm  ); 
     if ( sensitivity != NULL ) free( sensitivity );
     if ( grp_id >= 0 ) (void) H5Gclose( grp_id );
}

static
void Read_SCIA_H5_RSPD_elev( hid_t file_id, const char *group_nm,
			     /*@out@*/ size_t *n_elev, 
			     /*@out@*/ struct rspd_ELEV_scia **elev_out )
{
     const char prognm[] = "Read_SCIA_H5_RSPD_elev";

     register unsigned int ni, n2;

     struct rspd_ELEV_scia *elev;
     float  *wl = NULL;
     float  *ang_esm = NULL; 
     float  *sensitivity = NULL;

     hid_t  grp_id = -1;
     size_t dims[3];
     size_t dims_sens[3];
     int    rank;

     if ( (grp_id = H5Gopen( file_id, group_nm, H5P_DEFAULT )) < 0 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_GRP, group_nm );
/*
 * read data from HDF5-file into rspl structs
 */
     Read_SCIA_H5_RSPD_axis( grp_id, "Axis 1  wavelength",
			     &dims[1], &wl );
     Read_SCIA_H5_RSPD_axis( grp_id, "Axis 2  elevation angle",
			     &dims[0], &ang_esm );
     Read_SCIA_H5_RSPD_sensitivity( grp_id ,group_nm, &rank,
				    dims_sens, &sensitivity);

     if ( ! Use_Extern_Alloc ) {
	  elev_out[0] = (struct rspd_ELEV_scia *) 
	       malloc( dims[0] * sizeof( struct rspd_ELEV_scia ));
     }
     if ( (elev_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "elev" );
     elev = elev_out[0];
     
     for ( n2 = 0; n2 < dims[0]; n2++ ) {
	  ni = n2;
	  elev[ni].n_wl = dims[1];
	  elev[ni].elevat_angle  = ang_esm[n2];

	  (void) memcpy( elev[ni].wl, wl, dims[1] * sizeof(float) );
	  (void) memcpy( elev[ni].sensitivity, sensitivity + n2 * dims[1],
			 dims[1] * sizeof(float) );
     }
     *n_elev = dims[0] ; 
 done:
     if ( wl != NULL ) free( wl );
     if ( ang_esm != NULL ) free( ang_esm );
     if ( sensitivity != NULL ) free( sensitivity );
     if ( grp_id >= 0 ) (void) H5Gclose( grp_id );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_RD_H5_RSPN
.PURPOSE     read Radiance Sensitivity Parameters (nadir) from external file
.INPUT/OUTPUT
  call as    num_rsp = SCIA_RD_H5_RSPN( &rspn );
    output:
            struct rspn_scia **rspn : radiance sensitivity parameters (Nadir)

.RETURNS     number of data set records read (unsigned short)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned short SCIA_RD_H5_RSPN( /*@out@*/ struct rspn_scia **rspn_out )
{
     const char prognm[] = "SCIA_RD_H5_RSPN";

     unsigned short num_rsp = 0;

     register size_t         ni;
     register unsigned short np;
     register unsigned int   offs;

     char   rsp_file[MAX_STRING_LENGTH];

     float *ang_esm, *sensitivity;

     struct rspn_scia *rspn;

     hid_t  file_id = -1, grp_id = -1;
     hsize_t dims[2];
/*
 * open output HDF5-file
 */
     (void) strcpy( rsp_file, "./rsp_patch.h5" );
     H5E_BEGIN_TRY {
          file_id = H5Fopen( rsp_file, H5F_ACC_RDONLY, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( file_id < 0 ) {
          (void) snprintf( rsp_file, MAX_STRING_LENGTH, 
                           "%s/rsp_patch.h5", DATA_DIR );
          file_id = H5Fopen( rsp_file, H5F_ACC_RDONLY, H5P_DEFAULT );
          if ( file_id < 0 )
               NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, rsp_file );
     }
/*
 * open group /RSPN
 */
     if ( (grp_id = H5Gopen( file_id, "/RSPN", H5P_DEFAULT )) < 0 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_GRP, "/RSPN" );
/*
 * read data from HDF5-file into rspn structs
 */
     (void) H5LTget_dataset_info( grp_id, "elevation", dims, NULL, NULL );
     ang_esm = (float *) malloc( (size_t) dims[0] * sizeof(float) );
     if ( ang_esm == NULL ) NADC_GOTO_ERROR(prognm, NADC_ERR_ALLOC, "ang_esm");
     (void) H5LTread_dataset_float( grp_id, "elevation", ang_esm );

     (void) H5LTget_dataset_info( grp_id, "sensitivity", dims, NULL, NULL );
     sensitivity = (float *) 
	  malloc( (size_t) (dims[0] * dims[1]) * sizeof(float) );
     if ( sensitivity == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "sensitivity" );
     (void) H5LTread_dataset_float( grp_id, "sensitivity", sensitivity );

     if ( ! Use_Extern_Alloc ) {
	  rspn_out[0] = (struct rspn_scia *) 
	       malloc( (size_t) dims[0] * sizeof(struct rspn_scia));
     }
     if ( (rspn = rspn_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "rspn" );

     for ( offs = 0, ni = 0; ni < (size_t) dims[0]; ni++ ) {
	  rspn[ni].ang_esm = ang_esm[ni];
	  for ( np = 0; np < SCIENCE_PIXELS; np++ )
	       rspn[ni].sensitivity[np] = sensitivity[offs++];

	  num_rsp++;
     }
     free( ang_esm );
     free( sensitivity );
/*
 * give message to user
 */
     NADC_ERROR( prognm, NADC_ERR_NONE, 
                 "\n\tapplied auxiliary Radiance Sensitivity Parameters" );
 done:
     if ( grp_id >= 0 ) (void) H5Gclose( grp_id );
     if ( file_id >= 0 ) (void) H5Fclose( file_id );

     return num_rsp;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_RD_H5_RSPL
.PURPOSE     read Radiance Sensitivity Parameters (limb) from external file
.INPUT/OUTPUT
  call as    num_rsp = SCIA_RD_H5_RSPL( &rspl );
    output:
            struct rsplo_scia **rspl :  Radiance Sensitivity Parameters
	                                (limb)
.RETURNS     number of data set records read (unsigned short)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned short SCIA_RD_H5_RSPL( /*@out@*/ struct rsplo_scia **rspl_out )
{
     const char prognm[] = "SCIA_RD_H5_RSPL";

     unsigned short num_rsp = 0;

     register size_t         ni;
     register unsigned short np;
     register unsigned int   offs;

     char   rsp_file[MAX_STRING_LENGTH];

     float  *ang_asm, *ang_esm, *sensitivity;

     struct rsplo_scia *rspl;

     hid_t  file_id = -1, grp_id = -1;
     hsize_t dims[2];
/*
 * open output HDF5-file
 */
     (void) strcpy( rsp_file, "./rsp_patch.h5" );
     H5E_BEGIN_TRY {
          file_id = H5Fopen( rsp_file, H5F_ACC_RDONLY, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( file_id < 0 ) {
          (void) snprintf( rsp_file, MAX_STRING_LENGTH, 
                           "%s/rsp_patch.h5", DATA_DIR );
          file_id = H5Fopen( rsp_file, H5F_ACC_RDONLY, H5P_DEFAULT );
          if ( file_id < 0 )
               NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, rsp_file );
     }
/*
 * open group /RSPL
 */
     if ( (grp_id = H5Gopen( file_id, "/RSPL", H5P_DEFAULT )) < 0 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_GRP, "/RSPL" );
/*
 * read data from HDF5-file into rspl structs
 */
     (void) H5LTget_dataset_info( grp_id, "azimuth", dims, NULL, NULL );
     ang_asm = (float *) malloc( (size_t) dims[0] * sizeof(float) );
     if ( ang_asm == NULL ) NADC_GOTO_ERROR(prognm, NADC_ERR_ALLOC, "ang_asm");
     (void) H5LTread_dataset_float( grp_id, "azimuth", ang_asm );

     (void) H5LTget_dataset_info( grp_id, "elevation", dims, NULL, NULL );
     ang_esm = (float *) malloc( (size_t) dims[0] * sizeof(float) );
     if ( ang_esm == NULL ) NADC_GOTO_ERROR(prognm, NADC_ERR_ALLOC, "ang_esm");
     (void) H5LTread_dataset_float( grp_id, "elevation", ang_esm );

     (void) H5LTget_dataset_info( grp_id, "sensitivity", dims, NULL, NULL );
     sensitivity = (float *) 
	  malloc( (size_t) (dims[0] * dims[1]) * sizeof(float) );
     if ( sensitivity == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "sensitivity" );
     (void) H5LTread_dataset_float( grp_id, "sensitivity", sensitivity );

     if ( ! Use_Extern_Alloc ) {
	  rspl_out[0] = (struct rsplo_scia *) 
	       malloc( (size_t) dims[0] * sizeof(struct rsplo_scia));
     }
     if ( (rspl = rspl_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "rspl" );
     for ( offs = 0, ni = 0; ni < (size_t) dims[0]; ni++ ) {
	  rspl[ni].ang_asm = ang_asm[ni];
	  rspl[ni].ang_esm = ang_esm[ni];
	  for ( np = 0; np < SCIENCE_PIXELS; np++ )
	       rspl[ni].sensitivity[np] = sensitivity[offs++];

	  num_rsp++;
     }
     free( ang_asm );
     free( ang_esm );
     free( sensitivity );
/*
 * give message to user
 */
     NADC_ERROR( prognm, NADC_ERR_NONE, 
                 "\n\tapplied auxiliary Radiance Sensitivity Parameters" );
 done:
     if ( grp_id >= 0 ) (void) H5Gclose( grp_id );
     if ( file_id >= 0 ) (void) H5Fclose( file_id );

     return num_rsp;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_RD_H5_RSPO
.PURPOSE     read Radiance Sensitivity Parameters (limb) from external file
.INPUT/OUTPUT
  call as    num_rsp = SCIA_RD_H5_RSPO( &rspo );
    output:
            struct rsplo_scia **rspo :  Radiance Sensitivity Parameters
	                                (limb)
.RETURNS     number of data set records read (unsigned short)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned short SCIA_RD_H5_RSPO( /*@out@*/ struct rsplo_scia **rspo_out )
{
     const char prognm[] = "SCIA_RD_H5_RSPO";

     unsigned short num_rsp = 0;

     register size_t ni;
     register unsigned short np;
     register unsigned int offs;

     char   rsp_file[MAX_STRING_LENGTH];

     float  *ang_asm, *ang_esm, *sensitivity;

     struct rsplo_scia *rspo;

     hid_t  file_id = -1, grp_id = -1;
     hsize_t dims[2];
/*
 * open output HDF5-file
 */
     (void) strcpy( rsp_file, "./rsp_patch.h5" );
     H5E_BEGIN_TRY {
          file_id = H5Fopen( rsp_file, H5F_ACC_RDONLY, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( file_id < 0 ) {
          (void) snprintf( rsp_file, MAX_STRING_LENGTH, 
                           "%s/rsp_patch.h5", DATA_DIR );
          file_id = H5Fopen( rsp_file, H5F_ACC_RDONLY, H5P_DEFAULT );
          if ( file_id < 0 )
               NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, rsp_file );
     }
/*
 * open group /RSPO
 */
     if ( (grp_id = H5Gopen( file_id, "/RSPO", H5P_DEFAULT )) < 0 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_GRP, "/RSPO" );
/*
 * read data from HDF5-file into rspo structs
 */
     (void) H5LTget_dataset_info( grp_id, "azimuth", dims, NULL, NULL );
     ang_asm = (float *) malloc( (size_t) dims[0] * sizeof(float) );
     if ( ang_asm == NULL ) NADC_GOTO_ERROR(prognm, NADC_ERR_ALLOC, "ang_asm");
     (void) H5LTread_dataset_float( grp_id, "azimuth", ang_asm );

     (void) H5LTget_dataset_info( grp_id, "elevation", dims, NULL, NULL );
     ang_esm = (float *) malloc( (size_t) dims[0] * sizeof(float) );
     if ( ang_esm == NULL ) NADC_GOTO_ERROR(prognm, NADC_ERR_ALLOC, "ang_esm");
     (void) H5LTread_dataset_float( grp_id, "elevation", ang_esm );

     (void) H5LTget_dataset_info( grp_id, "sensitivity", dims, NULL, NULL );
     sensitivity = (float *) 
	  malloc( (size_t) (dims[0] * dims[1]) * sizeof(float) );
     if ( sensitivity == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "sensitivity" );
     (void) H5LTread_dataset_float( grp_id, "sensitivity", sensitivity );

     if ( ! Use_Extern_Alloc ) {
	  rspo_out[0] = (struct rsplo_scia *) 
	       malloc( (size_t) dims[0] * sizeof(struct rsplo_scia));
     }
     if ( (rspo = rspo_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "rspo" );
     for ( offs = 0, ni = 0; ni < (size_t) dims[0]; ni++ ) {
	  rspo[ni].ang_asm = ang_asm[ni];
	  rspo[ni].ang_esm = ang_esm[ni];
	  for ( np = 0; np < SCIENCE_PIXELS; np++ )
	       rspo[ni].sensitivity[np] = sensitivity[offs++];

	  num_rsp++;
     }
     free( ang_asm );
     free( ang_esm );
     free( sensitivity );
/*
 * give message to user
 */
     NADC_ERROR( prognm, NADC_ERR_NONE, 
                 "\n\tapplied auxiliary Radiance Sensitivity Parameters" );
 done:
     if ( grp_id >= 0 ) (void) H5Gclose( grp_id );
     if ( file_id >= 0 ) (void) H5Fclose( file_id );

     return num_rsp;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_RD_H5_RSPD
.PURPOSE     read keydata to calculate Radiance Sensitivity Parameters 
             (diffuser measurement) from external file
.INPUT/OUTPUT
  call as    Read_SCIA_H5_RSPD( &rspd_key );
    output:
            struct rspd_key *key :  keydata radiance sensitivity parameters

.RETURNS     number of data set records read (unsigned short)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_RD_H5_RSPD( /*@out@*/ struct rspd_key *key )
{
     const char prognm[] = "SCIA_RD_H5_RSPD";

     char   rsp_file[MAX_STRING_LENGTH];

     hid_t  file_id = -1;
/*
 * open input HDF5-file
 */
     (void) strcpy( rsp_file, "./key_radsens.h5" );
     H5E_BEGIN_TRY {
          file_id = H5Fopen( rsp_file, H5F_ACC_RDONLY, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( file_id < 0 ) {
          (void) snprintf( rsp_file, MAX_STRING_LENGTH, 
                           "%s/key_radsens.h5", DATA_DIR );
          file_id = H5Fopen( rsp_file, H5F_ACC_RDONLY, H5P_DEFAULT );
          if ( file_id < 0 )
               NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, rsp_file );
     }
     Read_SCIA_H5_RSPD_key_fix( file_id, &key->key_fix );
     Read_SCIA_H5_RSPD_el_az( file_id, "EL_AZ_p", 
			      &key->n_el_az, &key->el_az_p );
     Read_SCIA_H5_RSPD_el_az( file_id, "EL_AZ_s", 
			      &key->n_el_az, &key->el_az_s );
     Read_SCIA_H5_RSPD_brdf( file_id, "BRDF_p", &key->n_brdf, &key->brdf_p );
     Read_SCIA_H5_RSPD_brdf( file_id, "BRDF_s", &key->n_brdf, &key->brdf_s );
     Read_SCIA_H5_RSPD_elev( file_id, "ELEV_p", &key->n_elev, &key->elev_p );
     Read_SCIA_H5_RSPD_elev( file_id, "ELEV_s", &key->n_elev, &key->elev_s );
 done:
     if ( file_id >= 0 ) (void) H5Fclose( file_id );
}
