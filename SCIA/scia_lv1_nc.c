/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2017 - 2018 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV1_NC
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1b (netCDF4 format)
.LANGUAGE    ANSI C
.PURPOSE     Read Envisat Sciamachy level 1b (netCDF4) products, extract
             subsets, optionally calibrate the science data, and write in a
             flexible binary format (HDF5) or dump, in human readable form, 
	     the contents of each PDS data set to a separate ASCII file
.INPUT/OUTPUT
  call as
            scia_lv1_nc [options] <input-file>

.RETURNS     non-negative on success, negative on failure
.COMMENTS    None
.ENVIRONment None
.VERSION      0.1   30-Dec-2017 created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

#define  _XOPEN_SOURCE   /* needed for function strptime() */

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <hdf5_hl.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

/*+++++ Global Variables +++++*/
/* 
 * Most routines to read SCIAMACHY data can allocate memory internally
 * However IDL requires the use of their own memory allocation routines
 */
bool Use_Extern_Alloc = FALSE;

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static
void DELTA_TIME2MJD(const struct tm tm_ref, double delta_time,
		    /*@out@*/ struct mjd_envi *mjd)
{
     register int ni;

     struct tm tm;
     
     if (delta_time > 1e36) {
	  mjd->days  = 0;
	  mjd->secnd = 0;
	  mjd->musec = 0;
	  return;
     }
     (void) memcpy(&tm, &tm_ref, sizeof(struct tm));
     while ( delta_time > (60 * 60 * 24) ) {
	  delta_time -= (60 * 60 * 24);
	  tm.tm_yday += 1;
     }
     while ( delta_time > (60 * 60) ) {
	  delta_time -= (60 * 60);
	  tm.tm_hour += 1;
     }
     while ( delta_time > 60 ) {
	  delta_time -= 60;
	  tm.tm_min += 1;
     }
     tm.tm_sec += (int) delta_time;
     mjd->days = tm.tm_yday;
     tm.tm_year -= 100;
     for ( ni = 0; ni < tm.tm_year; ni++ ) 
	  mjd->days += (ni % 4 == 0 ? 366 : 365);
     mjd->secnd = 60 * (60 * tm.tm_hour + tm.tm_min) + tm.tm_sec;
     mjd->musec = NINT(1e6 * (delta_time - (int) delta_time));
}

/* static */
/* void SCIA_NC_RD_MPH(hid_t fid, struct mph_envi *mph) */
/* { */
/*      hid_t gid = -1; */
/* /\* */
/*  * open group /METADATA */
/*  *\/ */
/*      gid = NADC_OPEN_HDF5_Group( fid, "/METADATA" ); */
/*      if ( gid < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_GRP, "/METADATA" ); */
/* /\* */
/*  * set return values */
/*  *\/ */
/*      (void) H5Gclose(gid); */
/*      return; */
/* done: */
/*      H5E_BEGIN_TRY { */
/*           (void) H5Gclose(gid); */
/*      } H5E_END_TRY; */
/*      return; */
/* } */

/* static */
/* void SCIA_LV1_NC_RD_SPH(hid_t fid, struct sph1_scia *sph) */
/* { */
/*      hid_t gid = -1; */
/* /\* */
/*  * open group /METADATA */
/*  *\/ */
/*      gid = NADC_OPEN_HDF5_Group( fid, "/METADATA" ); */
/*      if ( gid < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_GRP, "/METADATA" ); */
/* /\* */
/*  * set return values */
/*  *\/ */
/*      (void) H5Gclose(gid); */
/*      return; */
/* done: */
/*      H5E_BEGIN_TRY { */
/*           (void) H5Gclose(gid); */
/*      } H5E_END_TRY; */
/*      return; */
/* } */

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_NC_RD_EKD
.PURPOSE     read Errors on Key Data
.INPUT/OUTPUT
  call as   num = SCIA_LV1_NC_RD_EKD( fid, num_&ekd );
     input:
            hid_t fid             :  HDF5 file identifier
    output:
            struct ekd_scia *ekd  :  errors on key data

.RETURNS     number of data set records read (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
static
unsigned int SCIA_LV1_NC_RD_EKD(hid_t fid, /*@out@*/ struct ekd_scia *ekd)
{
     register unsigned short ns;

     unsigned short spectral_channel;

     double *dbuff;

     hid_t gid = -1;
     hid_t dset_id = -1;
     hid_t type_id = -1;
     hsize_t cur_dims;
/*
 * open group /CALIBRATION/KEYDATA_ERRORS 
 */
     gid = NADC_OPEN_HDF5_Group( fid, "/CALIBRATION/KEYDATA_ERRORS" );
     if ( gid < 0 )
	  NADC_GOTO_ERROR(NADC_ERR_HDF_GRP, "/CALIBRATION/KEYDATA_ERRORS");
/*
 * obtain dimensions
 */
     if ( (dset_id = H5Dopen( gid, "spectral_channel", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "spectral_channel" );
     (void) H5LDget_dset_dims( dset_id, &cur_dims );
     (void) H5Dclose(dset_id);
     spectral_channel = (unsigned short) cur_dims;
/*
 * read KEYDATA_ERRORS datasets
 */
     // bsdf_error               Dataset {spectral_channel}
     if ( (dset_id = H5Dopen(gid, "bsdf_error", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "bsdf_error");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "bsdf_error");
     dbuff = (double *) malloc(spectral_channel * sizeof(double));
     if ( dbuff == NULL ) NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "bsdf_error" );
     for ( ns = 0; ns < spectral_channel; ns++ )
	  ekd->bsdf[ns] = (float) dbuff[ns];
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // mu2_accuracy             Dataset {spectral_channel}
     if ( (dset_id = H5Dopen(gid, "mu2_accuracy", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "mu2_accuracy");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "mu2_accuracy");
     dbuff = (double *) malloc(spectral_channel * sizeof(double));
     if ( dbuff == NULL ) NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "mu2_accuracy" );
     for ( ns = 0; ns < spectral_channel; ns++ )
	  ekd->mu2_nadir[ns] = (float) dbuff[ns];
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // mu2dl_accuracy           Dataset {spectral_channel}
     if ( (dset_id = H5Dopen(gid, "mu2dl_accuracy", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "mu2dl_accuracy");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "mu2dl_accuracy");
     dbuff = (double *) malloc(spectral_channel * sizeof(double));
     if ( dbuff == NULL ) NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "mu2dl_accuracy" );
     for ( ns = 0; ns < spectral_channel; ns++ )
	  ekd->mu2_limb[ns] = (float) dbuff[ns];
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // mu3_accuracy             Dataset {spectral_channel}
     if ( (dset_id = H5Dopen(gid, "mu3_accuracy", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "mu3_accuracy");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "mu3_accuracy");
     dbuff = (double *) malloc(spectral_channel * sizeof(double));
     if ( dbuff == NULL ) NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "mu3_accuracy" );
     for ( ns = 0; ns < spectral_channel; ns++ )
	  ekd->mu3_nadir[ns] = (float) dbuff[ns];
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // mu3dl_accuracy           Dataset {spectral_channel}
     if ( (dset_id = H5Dopen(gid, "mu3dl_accuracy", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "mu3dl_accuracy");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "mu3dl_accuracy");
     dbuff = (double *) malloc(spectral_channel * sizeof(double));
     if ( dbuff == NULL ) NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "mu3dl_accuracy" );
     for ( ns = 0; ns < spectral_channel; ns++ )
	  ekd->mu3_limb[ns] = (float) dbuff[ns];
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // radsens_limb_error       Dataset {spectral_channel}
     if ( (dset_id = H5Dopen(gid, "radsens_limb_error", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "radsens_limb_error");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "radsens_limb_error");
     dbuff = (double *) malloc(spectral_channel * sizeof(double));
     if ( dbuff == NULL ) NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "radsens_limb_error" );
     for ( ns = 0; ns < spectral_channel; ns++ )
	  ekd->radiance_limb[ns] = (float) dbuff[ns];
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // radsens_nadir_error      Dataset {spectral_channel}
     if ( (dset_id = H5Dopen(gid, "radsens_nadir_error", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "radsens_nadir_error");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "radsens_nadir_error");
     dbuff = (double *) malloc(spectral_channel * sizeof(double));
     if ( dbuff == NULL ) NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "radsens_nadir_error" );
     for ( ns = 0; ns < spectral_channel; ns++ )
	  ekd->radiance_nadir[ns] = (float) dbuff[ns];
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // radsens_optical_bench_error Dataset {spectral_channel}
     if ( (dset_id = H5Dopen(gid, "radsens_optical_bench_error", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "radsens_optical_bench_error");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "radsens_optical_bench_error");
     dbuff = (double *) malloc(spectral_channel * sizeof(double));
     if ( dbuff == NULL ) NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "radsens_optical_bench_error" );
     for ( ns = 0; ns < spectral_channel; ns++ )
	  ekd->radiance_vis[ns] = (float) dbuff[ns];
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // radsens_sun_error        Dataset {spectral_channel}
     if ( (dset_id = H5Dopen(gid, "radsens_sun_error", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "radsens_sun_error");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "radsens_sun_error");
     dbuff = (double *) malloc(spectral_channel * sizeof(double));
     if ( dbuff == NULL ) NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "radsens_sun_error" );
     for ( ns = 0; ns < spectral_channel; ns++ )
	  ekd->radiance_sun[ns] = (float) dbuff[ns];
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
/*
 * set return values
 */
     (void) H5Gclose(gid);
     return 1;
done:
     H5E_BEGIN_TRY {
          (void) H5Tclose(type_id);
          (void) H5Dclose(dset_id);
          (void) H5Gclose(gid);
     } H5E_END_TRY;
     return 0u;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_NC_RD_CLCP
.PURPOSE     read Leakage Current Parameters (constant fraction) records
.INPUT/OUTPUT
  call as   num = SCIA_LV1_NC_RD_CLCP( fid, &clcp );
     input:
            hid_t fid              :  HDF5 file identifier
    output:
            struct clcp_scia *clcp :  leakage current parameters (constant)

.RETURNS     number of data set records read (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
static
unsigned int SCIA_LV1_NC_RD_CLCP(hid_t fid, /*@out@*/ struct clcp_scia *clcp)
{
     register unsigned short ns;
     
     unsigned short spectral_channel, pmd2;

     double *dbuff;

     hid_t gid = -1;
     hid_t dset_id = -1;
     hid_t type_id = -1;
     hsize_t cur_dims;
/*
 * open group /CALIBRATION/LEAKAGE_CONSTANT
 */
     gid = NADC_OPEN_HDF5_Group( fid, "/CALIBRATION/LEAKAGE_CONSTANT" );
     if ( gid < 0 )
	  NADC_GOTO_ERROR( NADC_ERR_HDF_GRP, "/CALIBRATION/LEAKAGE_CONSTANT" );
/*
 * obtain dimensions
 */
     if ( (dset_id = H5Dopen( fid, "pmd2", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "pmd2" );
     (void) H5LDget_dset_dims( dset_id, &cur_dims );
     (void) H5Dclose(dset_id);
     pmd2 = (unsigned short) cur_dims;
     if ( (dset_id = H5Dopen( gid, "spectral_channel", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "spectral_channel" );
     (void) H5LDget_dset_dims( dset_id, &cur_dims );
     (void) H5Dclose(dset_id);
     spectral_channel = (unsigned short) cur_dims;
/*
 * read LEAKAGE_CURRENT datasets
 */
     // fixed_pattern_noise      Dataset {spectral_channel}
     if ( (dset_id = H5Dopen(gid, "fixed_pattern_noise", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "fixed_pattern_noise");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "fixed_pattern_noise");
     if ((dbuff = (double *) malloc(spectral_channel * sizeof(double))) == NULL)
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "fixed_pattern_noise" );
     for ( ns = 0; ns < spectral_channel; ns++ ) {
     	  clcp->fpn[ns] = (float) dbuff[ns];
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // fixed_pattern_noise_error Dataset {spectral_channel}
     if ( (dset_id = H5Dopen(gid, "fixed_pattern_noise_error", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "fixed_pattern_noise_error");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "fixed_pattern_noise_error");
     if ((dbuff = (double *) malloc(spectral_channel * sizeof(double))) == NULL)
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "fixed_pattern_noise_error" );
     for ( ns = 0; ns < spectral_channel; ns++ ) {
     	  clcp->fpn_error[ns] = (float) dbuff[ns];
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // leakage_current          Dataset {spectral_channel}
     if ( (dset_id = H5Dopen(gid, "leakage_current", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "leakage_current");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "leakage_current");
     if ((dbuff = (double *) malloc(spectral_channel * sizeof(double))) == NULL)
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "leakage_current" );
     for ( ns = 0; ns < spectral_channel; ns++ ) {
     	  clcp->lc[ns] = (float) dbuff[ns];
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // leakage_current_error    Dataset {spectral_channel}
     if ( (dset_id = H5Dopen(gid, "leakage_current_error", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "leakage_current_error");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "leakage_current_error");
     if ((dbuff = (double *) malloc(spectral_channel * sizeof(double))) == NULL)
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "leakage_current_error" );
     for ( ns = 0; ns < spectral_channel; ns++ ) {
     	  clcp->lc_error[ns] = (float) dbuff[ns];
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // mean_noise               Dataset {spectral_channel}
     if ( (dset_id = H5Dopen(gid, "mean_noise", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "mean_noise");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "mean_noise");
     if ((dbuff = (double *) malloc(spectral_channel * sizeof(double))) == NULL)
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "mean_noise" );
     for ( ns = 0; ns < spectral_channel; ns++ ) {
     	  clcp->mean_noise[ns] = (float) dbuff[ns];
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // pmd_offset               Dataset {pmd2}
     if ( (dset_id = H5Dopen(gid, "pmd_offset", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "pmd_offset");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "pmd_offset");
     if ( (dbuff = (double *) malloc(pmd2 * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "pmd_offset" );
     for ( ns = 0; ns < pmd2; ns++ ) {
     	  clcp->pmd_dark[ns] = (float) dbuff[ns];
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // pmd_offset_error         Dataset {pmd2}
     if ( (dset_id = H5Dopen(gid, "pmd_offset_error", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "pmd_offset_error");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "pmd_offset_error");
     if ( (dbuff = (double *) malloc(pmd2 * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "pmd_offset_error" );
     for ( ns = 0; ns < pmd2; ns++ ) {
     	  clcp->pmd_dark_error[ns] = (float) dbuff[ns];
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
/*
 * set return values
 */
     (void) H5Gclose(gid);
     return 1;
done:
     H5E_BEGIN_TRY {
          (void) H5Tclose(type_id);
          (void) H5Dclose(dset_id);
          (void) H5Gclose(gid);
     } H5E_END_TRY;
     return 0u;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_NC_RD_VLCP
.PURPOSE     read Leakage Current Parameters (variable fraction)
.INPUT/OUTPUT
  call as   num = SCIA_LV1_NC_RD_VLCP( fid, &vlcp );
     input:
            hid_t fid             :   HDF5 file identifier
    output:
            struct vlcp_scia **vlcp :  leakage current parameters (variable)

.RETURNS     number of data set records read (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
static
unsigned int SCIA_LV1_NC_RD_VLCP(hid_t fid, struct vlcp_scia **vlcp_out)
{
     register unsigned short ns;
     register unsigned int   ni, nr;

     unsigned short num_temp;
     unsigned short num_pmd, num_pmd_ir;
     unsigned short spectral_channel, spectral_channel_ir;
     unsigned int   num_vlcp;
     size_t         dim_size;

     float  *rbuff;
     double *dbuff;

     struct vlcp_scia *vlcp;
     
     hid_t gid = -1;
     hid_t dset_id = -1;
     hid_t type_id = -1;
     hsize_t cur_dims;
/*
 * open group /CALIBRATION/LEAKAGE_VARIABLE
 */
     gid = NADC_OPEN_HDF5_Group( fid, "/CALIBRATION/LEAKAGE_VARIABLE" );
     if ( gid < 0 )
	  NADC_GOTO_ERROR( NADC_ERR_HDF_GRP, "/CALIBRATION/LEAKAGE_VARIABLE" );
/*
 * obtain dimensions
 */
     if ( (dset_id = H5Dopen( fid, "pmd", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "pmd" );
     (void) H5LDget_dset_dims( dset_id, &cur_dims );
     (void) H5Dclose(dset_id);
     num_pmd = (unsigned short) cur_dims;
     if ( (dset_id = H5Dopen( fid, "pmd_ir", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "pmd_ir" );
     (void) H5LDget_dset_dims( dset_id, &cur_dims );
     (void) H5Dclose(dset_id);
     num_pmd_ir = (unsigned short) cur_dims;
     if ( (dset_id = H5Dopen( gid, "temperature", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "temperature" );
     (void) H5LDget_dset_dims( dset_id, &cur_dims );
     (void) H5Dclose(dset_id);
     num_temp = (unsigned int) cur_dims;
     if ( (dset_id = H5Dopen( gid, "spectral_channel", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "spectral_channel" );
     (void) H5LDget_dset_dims( dset_id, &cur_dims );
     (void) H5Dclose(dset_id);
     spectral_channel = (unsigned short) cur_dims;
     if ( (dset_id = H5Dopen( gid, "spectral_channel_ir", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "spectral_channel_ir" );
     (void) H5LDget_dset_dims( dset_id, &cur_dims );
     (void) H5Dclose(dset_id);
     spectral_channel_ir = (unsigned short) cur_dims;
     if ( (dset_id = H5Dopen( gid, "orbit_phase", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "orbit_phase" );
     (void) H5LDget_dset_dims( dset_id, &cur_dims );
     (void) H5Dclose(dset_id);
     num_vlcp = (unsigned int) cur_dims;
/*
 * allocate memory
 */
     if ( ! Use_Extern_Alloc ) {
          vlcp_out[0] = (struct vlcp_scia *) 
               calloc((size_t) num_vlcp, sizeof(struct vlcp_scia));
     }
     if ( (vlcp = vlcp_out[0]) == NULL ) 
          NADC_GOTO_ERROR( NADC_ERR_ALLOC, "vlcp" );
/*
 * read LEAKAGE_CURRENT datasets
 */
     // orbit_phase              Dataset {num_vlcp}
     if ( (dset_id = H5Dopen(gid, "orbit_phase", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "orbit_phase");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "orbit_phase");
     if ( (rbuff = (float *) malloc(num_vlcp * sizeof(float))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, rbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "orbit_phase" );
     for ( ni = 0; ni < num_vlcp; ni++ ) {
	  vlcp[ni].orbit_phase = rbuff[ni];
     }
     free( rbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // channel_temperature      Dataset {num_vlcp, num_temp}
     dim_size = num_vlcp * num_temp;
     if ( (dset_id = H5Dopen(gid, "channel_temperature", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "channel_temperature");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "channel_temperature");
     if ( (rbuff = (float *) malloc(dim_size * sizeof(float))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, rbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "channel_temperature" );
     for ( nr = ni = 0; ni < num_vlcp; ni++ ) {
	  for ( ns = 0; ns < num_temp; ns++ )
	       vlcp[ni].obm_pmd[ns] = rbuff[nr++];
     }
     free( rbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // leakage_current          Dataset {num_vlcp, spectral_channel_ir}
     dim_size = num_vlcp * spectral_channel_ir;
     if ( (dset_id = H5Dopen(gid, "leakage_current", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "leakage_current");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "leakage_current");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "leakage_current" );
     for ( nr = ni = 0; ni < num_vlcp; ni++ ) {
	  for ( ns = 0; ns < spectral_channel_ir; ns++ )
	       vlcp[ni].var_lc[ns] = (float) dbuff[nr++];
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // leakage_current_error    Dataset {num_vlcp, spectral_channel_ir}
     dim_size = num_vlcp * spectral_channel_ir;
     if ( (dset_id = H5Dopen(gid, "leakage_current_error", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "leakage_current_error");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "leakage_current_error");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "leakage_current_error" );
     for ( nr = ni = 0; ni < num_vlcp; ni++ ) {
	  for ( ns = 0; ns < spectral_channel_ir; ns++ )
	       vlcp[ni].var_lc_error[ns] = (float) dbuff[nr++];
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // pmd_offset               Dataset {num_vlcp, num_pmd_ir}
     dim_size = num_vlcp * num_pmd_ir;
     if ( (dset_id = H5Dopen(gid, "pmd_offset", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "pmd_offset");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "pmd_offset");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "pmd_offset" );
     for ( nr = ni = 0; ni < num_vlcp; ni++ ) {
	  for ( ns = 0; ns < num_pmd_ir; ns++ )
	       vlcp[ni].pmd_dark[ns] = (float) dbuff[nr++];
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // pmd_offset_error         Dataset {num_vlcp, num_pmd_ir}
     dim_size = num_vlcp * num_pmd_ir;
     if ( (dset_id = H5Dopen(gid, "pmd_offset_error", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "pmd_offset_error");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "pmd_offset_error");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "pmd_offset_error" );
     for ( nr = ni = 0; ni < num_vlcp; ni++ ) {
	  for ( ns = 0; ns < num_pmd_ir; ns++ )
	       vlcp[ni].pmd_dark_error[ns] = (float) dbuff[nr++];
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // pmd_straylight           Dataset {num_vlcp, num_pmd}
     dim_size = num_vlcp * num_pmd;
     if ( (dset_id = H5Dopen(gid, "pmd_straylight", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "pmd_straylight");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "pmd_straylight");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "pmd_straylight" );
     for ( nr = ni = 0; ni < num_vlcp; ni++ ) {
	  for ( ns = 0; ns < num_pmd; ns++ )
	       vlcp[ni].pmd_stray[ns] = (float) dbuff[nr++];
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // pmd_straylight_error     Dataset {num_vlcp, num_pmd}
     dim_size = num_vlcp * num_pmd;
     if ( (dset_id = H5Dopen(gid, "pmd_straylight_error", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "pmd_straylight_error");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "pmd_straylight_error");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "pmd_straylight_error" );
     for ( nr = ni = 0; ni < num_vlcp; ni++ ) {
	  for ( ns = 0; ns < num_pmd; ns++ )
	       vlcp[ni].pmd_stray_error[ns] = (float) dbuff[nr++];
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // solar_straylight         Dataset {num_vlcp, spectral_channel}
     dim_size = num_vlcp * spectral_channel;
     if ( (dset_id = H5Dopen(gid, "solar_straylight", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "solar_straylight");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "solar_straylight");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "solar_straylight" );
     for ( nr = ni = 0; ni < num_vlcp; ni++ ) {
	  for ( ns = 0; ns < spectral_channel; ns++ )
	       vlcp[ni].solar_stray[ns] = (float) dbuff[nr++];
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // solar_straylight_error   Dataset {num_vlcp, spectral_channel}
     dim_size = num_vlcp * spectral_channel;
     if ( (dset_id = H5Dopen(gid, "solar_straylight_error", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "solar_straylight_error");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "solar_straylight_error");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "solar_straylight_error" );
     for ( nr = ni = 0; ni < num_vlcp; ni++ ) {
	  for ( ns = 0; ns < spectral_channel; ns++ )
	       vlcp[ni].solar_stray[ns] = (float) dbuff[nr++];
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
/*
 * set return values
 */
     (void) H5Gclose(gid);
     return num_vlcp;
done:
     H5E_BEGIN_TRY {
          (void) H5Tclose(type_id);
          (void) H5Dclose(dset_id);
          (void) H5Gclose(gid);
     } H5E_END_TRY;
     return 0u;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_NC_RD_SRS
.PURPOSE     read mean Sun Reference
.INPUT/OUTPUT
  call as   num = SCIA_LV1_NC_RD_SRS( fid, &srs );
     input:
            hid_t fid             :  HDF5 file identifier
    output:
            struct srs_scia **srs :  mean Sun reference

.RETURNS     number of data set records read (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
static
unsigned int SCIA_LV1_NC_RD_SRS(hid_t fid, struct srs_scia **srs_out)
{
     register unsigned short ns;
     register unsigned int   ni, nr;

     unsigned short num_pmd, spectral_channel;
     unsigned int   num_srs;
     size_t         dim_size;

     char   **cbuff;
     float  *rbuff;
     double *dbuff;

     struct srs_scia *srs;
     
     hid_t gid = -1;
     hid_t space_id = -1;
     hid_t dset_id = -1;
     hid_t type_id = -1;
     hsize_t cur_dims;
/*
 * open group /CALIBRATION/MEAN_SUN_REFERENCE 
 */
     gid = NADC_OPEN_HDF5_Group(fid, "/CALIBRATION/MEAN_SUN_REFERENCE");
     if ( gid < 0 )
	  NADC_GOTO_ERROR(NADC_ERR_HDF_GRP, "/CALIBRATION/MEAN_SUN_REFERENCE");
/*
 * obtain dimensions
 */
     if ( (dset_id = H5Dopen( fid, "pmd", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "pmd" );
     (void) H5LDget_dset_dims( dset_id, &cur_dims );
     (void) H5Dclose(dset_id);
     num_pmd = (unsigned short) cur_dims;
     if ( (dset_id = H5Dopen( gid, "spectral_channel", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "spectral_channel" );
     (void) H5LDget_dset_dims( dset_id, &cur_dims );
     (void) H5Dclose(dset_id);
     spectral_channel = (unsigned short) cur_dims;
     if ( (dset_id = H5Dopen( gid, "record", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "record" );
     (void) H5LDget_dset_dims( dset_id, &cur_dims );
     (void) H5Dclose(dset_id);
     num_srs = (unsigned int) cur_dims;   /* keep cur_dims == num_srs */
/*
 * allocate memory
 */
     if ( ! Use_Extern_Alloc ) {
          srs_out[0] = (struct srs_scia *) 
               calloc((size_t) num_srs, sizeof(struct srs_scia));
     }
     if ( (srs = srs_out[0]) == NULL ) 
          NADC_GOTO_ERROR( NADC_ERR_ALLOC, "srs" );
/*
 * read MEAN_SUN_REFERENCE datasets
 */
     // doppler_shift            Dataset {num_srs}
     if ( (dset_id = H5Dopen(gid, "doppler_shift", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "doppler_shift");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "doppler_shift");
     if ( (rbuff = (float *) malloc(num_srs * sizeof(float))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, rbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "doppler_shift" );
     for ( ni = 0; ni < num_srs; ni++ ) {
	  srs[ni].dopp_shift = rbuff[ni];
     }
     free( rbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // mean_sun_azimuth         Dataset {num_srs}
     if ( (dset_id = H5Dopen(gid, "mean_sun_azimuth", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "mean_sun_azimuth");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "mean_sun_azimuth");
     if ( (rbuff = (float *) malloc(num_srs * sizeof(float))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, rbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "mean_sun_azimuth" );
     for ( ni = 0; ni < num_srs; ni++ ) {
	  srs[ni].avg_asm = rbuff[ni];
     }
     free( rbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // mean_sun_elevation       Dataset {num_srs}
     if ( (dset_id = H5Dopen(gid, "mean_sun_elevation", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "mean_sun_elevation");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "mean_sun_elevation");
     if ( (rbuff = (float *) malloc(num_srs * sizeof(float))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, rbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "mean_sun_elevation" );
     for ( ni = 0; ni < num_srs; ni++ ) {
	  srs[ni].avg_esm = rbuff[ni];
     }
     free( rbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // mean_sun_solar_elevation Dataset {num_srs}
     if ( (dset_id = H5Dopen(gid, "mean_sun_solar_elevation", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "mean_sun_solar_elevation");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "mean_sun_solar_elevation");
     if ( (rbuff = (float *) malloc(num_srs * sizeof(float))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, rbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "mean_sun_solar_elevation" );
     for ( ni = 0; ni < num_srs; ni++ ) {
	  srs[ni].avg_elev_sun = rbuff[ni];
     }
     free( rbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // type                     Dataset {num_srs, S(3)}
     if ( (dset_id = H5Dopen(gid, "type", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "type");
     type_id = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size (type_id, H5T_VARIABLE);
     space_id = H5Screate_simple(1, &cur_dims, NULL);
     if ( (cbuff = (char **) malloc(num_srs * sizeof(char *))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "cbuff" );
     if (H5Dread(dset_id, type_id, space_id, H5S_ALL, H5S_ALL, cbuff) < 0)
	  NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "type" );
     for ( ni = 0; ni < num_srs; ni++ ) {
	  (void) nadc_strlcpy(srs[ni].sun_spec_id, cbuff[ni], 3);
     }
     (void) H5Sclose(space_id);
     free(cbuff);
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // lambda_mean_sun          Dataset {num_srs, spectral_channel}
     dim_size = num_srs * spectral_channel;
     if ( (dset_id = H5Dopen(gid, "lambda_mean_sun", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "lambda_mean_sun");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "lambda_mean_sun");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "lambda_mean_sun" );
     for ( nr = ni = 0; ni < num_srs; ni++ ) {
	  for ( ns = 0; ns < spectral_channel; ns++ )
	       srs[ni].wvlen_sun[ns] = (float) dbuff[nr++];
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // mean_sun_accuracy        Dataset {num_srs, spectral_channel}
     dim_size = num_srs * spectral_channel;
     if ( (dset_id = H5Dopen(gid, "mean_sun_accuracy", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "mean_sun_accuracy");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "mean_sun_accuracy");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "mean_sun_accuracy" );
     for ( nr = ni = 0; ni < num_srs; ni++ ) {
	  for ( ns = 0; ns < spectral_channel; ns++ )
	       srs[ni].accuracy_sun[ns] = (float) dbuff[nr++];
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // mean_sun_etalon          Dataset {num_srs, spectral_channel}
     dim_size = num_srs * spectral_channel;
     if ( (dset_id = H5Dopen(gid, "mean_sun_etalon", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "mean_sun_etalon");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "mean_sun_etalon");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "mean_sun_etalon" );
     for ( nr = ni = 0; ni < num_srs; ni++ ) {
	  for ( ns = 0; ns < spectral_channel; ns++ )
	       srs[ni].etalon[ns] = (float) dbuff[nr++];
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // mean_sun_precision       Dataset {num_srs, spectral_channel}
     dim_size = num_srs * spectral_channel;
     if ( (dset_id = H5Dopen(gid, "mean_sun_precision", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "mean_sun_precision");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "mean_sun_precision");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "mean_sun_precision" );
     for ( nr = ni = 0; ni < num_srs; ni++ ) {
	  for ( ns = 0; ns < spectral_channel; ns++ )
	       srs[ni].precision_sun[ns] = (float) dbuff[nr++];
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // mean_sun_reference       Dataset {num_srs, spectral_channel}
     dim_size = num_srs * spectral_channel;
     if ( (dset_id = H5Dopen(gid, "mean_sun_reference", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "mean_sun_reference");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "mean_sun_reference");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "mean_sun_reference" );
     for ( nr = ni = 0; ni < num_srs; ni++ ) {
	  for ( ns = 0; ns < spectral_channel; ns++ )
	       srs[ni].mean_sun[ns] = (float) dbuff[nr++];
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // mean_sun_pmd             Dataset {num_srs, num_pmd}
     dim_size = num_srs * num_pmd;
     if ( (dset_id = H5Dopen(gid, "mean_sun_pmd", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "mean_sun_pmd");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "mean_sun_pmd");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "mean_sun_pmd" );
     for ( nr = ni = 0; ni < num_srs; ni++ ) {
	  for ( ns = 0; ns < num_pmd; ns++ )
	       srs[ni].pmd_mean[ns] = (float) dbuff[nr++];
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // mean_sun_out_of_band_signal_nd_in Dataset {num_srs, num_pmd}
     dim_size = num_srs * num_pmd;
     if ( (dset_id = H5Dopen(gid, "mean_sun_out_of_band_signal_nd_in", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "mean_sun_out_of_band_signal_nd_in");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "mean_sun_out_of_band_signal_nd_in");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "mean_sun_out_of_band_signal_nd_in" );
     for ( nr = ni = 0; ni < num_srs; ni++ ) {
	  for ( ns = 0; ns < num_pmd; ns++ )
	       srs[ni].pmd_out_nd_in[ns] = (float) dbuff[nr++];
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // mean_sun_out_of_band_signal_nd_out Dataset {num_srs, num_pmd}
     dim_size = num_srs * num_pmd;
     if ( (dset_id = H5Dopen(gid, "mean_sun_out_of_band_signal_nd_out", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "mean_sun_out_of_band_signal_nd_out");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "mean_sun_out_of_band_signal_nd_out");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "mean_sun_out_of_band_signal_nd_out" );
     for ( nr = ni = 0; ni < num_srs; ni++ ) {
	  for ( ns = 0; ns < num_pmd; ns++ )
	       srs[ni].pmd_out_nd_out[ns] = (float) dbuff[nr++];
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
/*
 * set return values
 */
     (void) H5Gclose(gid);
     return num_srs;
done:
     H5E_BEGIN_TRY {
          (void) H5Tclose(type_id);
          (void) H5Dclose(dset_id);
          (void) H5Gclose(gid);
     } H5E_END_TRY;
     return 0u;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_NV_RD_PSPL
.PURPOSE     read Polarisation Sensitivity Parameters (limb)
.INPUT/OUTPUT
  call as   num = SCIA_LV1_RD_PSPL( fid, &pspl );
     input:
            hid_t fid        :         HDF5 file identifier 
    output:
            struct psplo_scia **pspl :  Polarisation Sensitivity Parameters
                                        (limb)
.RETURNS     number of data set records read (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
static
unsigned int SCIA_LV1_NC_RD_PSPL(hid_t fid, struct psplo_scia **pspl_out)
{
     register unsigned short ni, nj, ns;
     register unsigned int   np, nr;

     unsigned short num_asm, num_esm, spectral_channel;
     unsigned int   num_pspl;
     size_t         dim_size;

     double *val_asm = NULL, *val_esm = NULL;
     double *val_mu2 = NULL, *val_mu3 = NULL;

     struct psplo_scia *pspl;
     
     hid_t gid = -1;
     hid_t dset_id = -1;
     hid_t type_id = -1;
     hsize_t cur_dims;
     /*
      * define name of group and data sets
      */
     const char grp_name[] = \
	  "/CALIBRATION/POLARISATION_SENSITIVITY_LIMB_OCCULTATION";
     const char ds_name_asm[] = "angle_asm_limb";
     const char ds_name_esm[] = "angle_esm_limb";
     const char ds_name_mu2[] = "polarisation_sensitivity_limb_mu2";
     const char ds_name_mu3[] = "polarisation_sensitivity_limb_mu3";
/*
 * open group /CALIBRATION
 */
     gid = NADC_OPEN_HDF5_Group(fid, "/CALIBRATION");
     if ( gid < 0 ) NADC_GOTO_ERROR(NADC_ERR_HDF_GRP, "/CALIBRATION");
/*
 * obtain dimensions
 */
     // angle_asm_limb           Dataset {23}
     if ( (dset_id = H5Dopen(gid, ds_name_asm, H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, ds_name_asm );
     (void) H5LDget_dset_dims(dset_id, &cur_dims);
     (void) H5Dclose(dset_id);
     num_asm = (unsigned short) cur_dims;
     // angle_esm_limb           Dataset {5}
     if ( (dset_id = H5Dopen(gid, ds_name_esm, H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, ds_name_esm );
     (void) H5LDget_dset_dims(dset_id, &cur_dims);
     (void) H5Dclose(dset_id);
     num_esm = (unsigned short) cur_dims;

     (void) H5Gclose(gid);
     num_pspl = num_asm * num_esm;
/*
 * open group /CALIBRATION/POLARISATION_SENSITIVITY_LIMB_OCCULTATION
 */
     gid = NADC_OPEN_HDF5_Group(fid, grp_name);
     if ( gid < 0 ) NADC_GOTO_ERROR(NADC_ERR_HDF_GRP, grp_name);
/*
 * obtain dimensions
 */
     if ( (dset_id = H5Dopen(gid, "spectral_channel", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "spectral_channel");
     (void) H5LDget_dset_dims(dset_id, &cur_dims);
     (void) H5Dclose(dset_id);
     spectral_channel = (unsigned short) cur_dims;
/*
 * allocate memory
 */
     if ( ! Use_Extern_Alloc ) {
          pspl_out[0] = (struct psplo_scia *) 
               calloc((size_t) num_pspl, sizeof(struct psplo_scia));
     }
     if ( (pspl = pspl_out[0]) == NULL ) 
          NADC_GOTO_ERROR(NADC_ERR_ALLOC, "pspl");
/*
 * read POLARISATION_SENSITIVITY_LIMB_OCCULTATION datasets
 */
     // angle_asm_limb           Dataset {num_asm}
     if ( (dset_id = H5Dopen(gid, ds_name_asm, H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, ds_name_asm);
     if ( (type_id = H5Dget_type(dset_id)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, ds_name_asm);
     if ( (val_asm = (double *) malloc(num_asm * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "val_asm" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, val_asm) < 0)
          NADC_GOTO_ERROR(NADC_ERR_HDF_RD, ds_name_asm);
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // angle_esm_limb           Dataset {num_esm}
     if ( (dset_id = H5Dopen(gid, ds_name_esm, H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, ds_name_esm);
     if ( (type_id = H5Dget_type(dset_id)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, ds_name_esm);
     if ( (val_esm = (double *) malloc(num_esm * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "val_esm" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, val_esm) < 0)
          NADC_GOTO_ERROR(NADC_ERR_HDF_RD, ds_name_esm);
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // polarisation_sensitivity_limb_mu2 Dataset {num_esm, num_asm, 8192}
     dim_size = num_pspl * spectral_channel;
     if ( (dset_id = H5Dopen(gid, ds_name_mu2, H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, ds_name_mu2);
     if ( (type_id = H5Dget_type(dset_id)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, ds_name_mu2);
     if ( (val_mu2 = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "val_mu2" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, val_mu2) < 0)
          NADC_GOTO_ERROR(NADC_ERR_HDF_RD, ds_name_mu2);
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // polarisation_sensitivity_limb_mu3 Dataset {num_esm, num_asm, 8192}
     dim_size = num_pspl * spectral_channel;
     if ( (dset_id = H5Dopen(gid, ds_name_mu3, H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, ds_name_mu3);
     if ( (type_id = H5Dget_type(dset_id)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, ds_name_mu3);
     if ( (val_mu3 = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "val_mu3" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, val_mu3) < 0)
          NADC_GOTO_ERROR(NADC_ERR_HDF_RD, ds_name_mu3);
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
/*
 * store data in output structure
 */
     np = nr = 0;
     for ( nj = 0; nj < num_esm; nj++ ) {
	  for ( ni = 0; ni < num_asm; ni++ ) {
	       for ( ns = 0; ns < spectral_channel; ns++ ) {
		    pspl[np].mu2[ns] = val_mu2[nr];
		    pspl[np].mu3[ns] = val_mu3[nr];
		    nr++;
	       }
	       pspl[np].ang_esm = (float) val_esm[nj];
	       pspl[np].ang_asm = (float) val_asm[ni];
	       np++;
	  }
     }
/*
 * set return values
 */
     free(val_asm);
     free(val_esm);
     free(val_mu2);
     free(val_mu3);
     (void) H5Gclose(gid);
     return num_pspl;
done:
     if (val_asm != NULL ) free(val_asm);
     if (val_esm != NULL ) free(val_esm);
     if (val_mu2 != NULL ) free(val_mu2);
     if (val_mu3 != NULL ) free(val_mu3);
     H5E_BEGIN_TRY {
          (void) H5Tclose(type_id);
          (void) H5Dclose(dset_id);
          (void) H5Gclose(gid);
     } H5E_END_TRY;
     return 0u;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_NV_RD_PSPN
.PURPOSE     read Polarisation Sensitivity Parameters (nadir)
.INPUT/OUTPUT
  call as   num = SCIA_LV1_RD_PSPN( fid, &pspn );
     input:
            hid_t fid        :         HDF5 file identifier 
    output:
            struct pspn_scia **pspn :  Polarisation Sensitivity Parameters
                                        (nadir)
.RETURNS     number of data set records read (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
static
unsigned int SCIA_LV1_NC_RD_PSPN(hid_t fid, struct pspn_scia **pspn_out)
{
     register unsigned short ns;
     register unsigned int   np, nr;

     unsigned short num_esm, spectral_channel;
     unsigned int   num_pspn;
     size_t         dim_size;

     double *val_esm = NULL;
     double *val_mu2 = NULL, *val_mu3 = NULL;

     struct pspn_scia *pspn;
     
     hid_t gid = -1;
     hid_t dset_id = -1;
     hid_t type_id = -1;
     hsize_t cur_dims;
     /*
      * define name of group and data sets
      */
     const char grp_name[] = \
	  "/CALIBRATION/POLARISATION_SENSITIVITY_NADIR";
     const char ds_name_esm[] = "angle_esm_nadir";
     const char ds_name_mu2[] = "polarisation_sensitivity_nadir_mu2";
     const char ds_name_mu3[] = "polarisation_sensitivity_nadir_mu3";
/*
 * open group /CALIBRATION
 */
     gid = NADC_OPEN_HDF5_Group(fid, "/CALIBRATION");
     if ( gid < 0 ) NADC_GOTO_ERROR(NADC_ERR_HDF_GRP, "/CALIBRATION");
/*
 * obtain dimensions
 */
     // angle_esm_nadir           Dataset {17}
     if ( (dset_id = H5Dopen(gid, ds_name_esm, H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, ds_name_esm );
     (void) H5LDget_dset_dims(dset_id, &cur_dims);
     (void) H5Dclose(dset_id);
     num_esm = (unsigned short) cur_dims;

     (void) H5Gclose(gid);
     num_pspn = num_esm;
/*
 * open group /CALIBRATION/POLARISATION_SENSITIVITY_NADIR
 */
     gid = NADC_OPEN_HDF5_Group(fid, grp_name);
     if ( gid < 0 ) NADC_GOTO_ERROR(NADC_ERR_HDF_GRP, grp_name);
/*
 * obtain dimensions
 */
     if ( (dset_id = H5Dopen(gid, "spectral_channel", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "spectral_channel");
     (void) H5LDget_dset_dims(dset_id, &cur_dims);
     (void) H5Dclose(dset_id);
     spectral_channel = (unsigned short) cur_dims;
/*
 * allocate memory
 */
     if ( ! Use_Extern_Alloc ) {
          pspn_out[0] = (struct pspn_scia *) 
               calloc((size_t) num_pspn, sizeof(struct pspn_scia));
     }
     if ( (pspn = pspn_out[0]) == NULL ) 
          NADC_GOTO_ERROR(NADC_ERR_ALLOC, "pspn");
/*
 * read POLARISATION_SENSITIVITY_NADIR datasets
 */
     // angle_esm_nadir           Dataset {num_esm}
     if ( (dset_id = H5Dopen(gid, ds_name_esm, H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, ds_name_esm);
     if ( (type_id = H5Dget_type(dset_id)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, ds_name_esm);
     if ( (val_esm = (double *) malloc(num_esm * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "val_esm" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, val_esm) < 0)
          NADC_GOTO_ERROR(NADC_ERR_HDF_RD, ds_name_esm);
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // polarisation_sensitivity_nadir_mu2 Dataset {num_esm, spectral_channel}
     dim_size = num_pspn * spectral_channel;
     if ( (dset_id = H5Dopen(gid, ds_name_mu2, H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, ds_name_mu2);
     if ( (type_id = H5Dget_type(dset_id)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, ds_name_mu2);
     if ( (val_mu2 = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "val_mu2" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, val_mu2) < 0)
          NADC_GOTO_ERROR(NADC_ERR_HDF_RD, ds_name_mu2);
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // polarisation_sensitivity_nadir_mu3 Dataset {num_esm, spectral_channel}
     dim_size = num_pspn * spectral_channel;
     if ( (dset_id = H5Dopen(gid, ds_name_mu3, H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, ds_name_mu3);
     if ( (type_id = H5Dget_type(dset_id)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, ds_name_mu3);
     if ( (val_mu3 = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "val_mu3" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, val_mu3) < 0)
          NADC_GOTO_ERROR(NADC_ERR_HDF_RD, ds_name_mu3);
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
/*
 * store data in output structure
 */
     for ( nr = np = 0; np < num_esm; np++ ) {
	  for ( ns = 0; ns < spectral_channel; ns++ ) {
	       pspn[np].mu2[ns] = val_mu2[nr];
	       pspn[np].mu3[ns] = val_mu3[nr];
	       nr++;
	  }
	  pspn[np].ang_esm = (float) val_esm[np];
     }
/*
 * set return values
 */
     free(val_esm);
     free(val_mu2);
     free(val_mu3);
     (void) H5Gclose(gid);
     return num_pspn;
done:
     if (val_esm != NULL ) free(val_esm);
     if (val_mu2 != NULL ) free(val_mu2);
     if (val_mu3 != NULL ) free(val_mu3);
     H5E_BEGIN_TRY {
          (void) H5Tclose(type_id);
          (void) H5Dclose(dset_id);
          (void) H5Gclose(gid);
     } H5E_END_TRY;
     return 0u;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_NV_RD_PSPO
.PURPOSE     read Polarisation Sensitivity Parameters (occultation)
.INPUT/OUTPUT
  call as   num = SCIA_LV1_RD_PSPO( fid, &pspo );
     input:
            hid_t fid        :         HDF5 file identifier 
    output:
            struct psplo_scia **pspo :  Polarisation Sensitivity Parameters
                                        (occultation)
.RETURNS     number of data set records read (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
static
unsigned int SCIA_LV1_NC_RD_PSPO(hid_t fid, struct psplo_scia **pspo_out)
{
     register unsigned short ni, nj, ns;
     register unsigned int   np, nr;

     unsigned short num_asm, num_esm, spectral_channel;
     unsigned int   num_pspo;
     size_t         dim_size;

     double *val_asm = NULL, *val_esm = NULL;
     double *val_mu2 = NULL, *val_mu3 = NULL;

     struct psplo_scia *pspo;
     
     hid_t gid = -1;
     hid_t dset_id = -1;
     hid_t type_id = -1;
     hsize_t cur_dims;
     /*
      * define name of group and data sets
      */
     const char grp_name[] = \
	  "/CALIBRATION/POLARISATION_SENSITIVITY_LIMB_OCCULTATION_NDF";
     const char ds_name_asm[] = "angle_asm_limb_ndf";
     const char ds_name_esm[] = "angle_esm_limb_ndf";
     const char ds_name_mu2[] = "polarisation_sensitivity_limb_mu2_NDF";
     const char ds_name_mu3[] = "polarisation_sensitivity_limb_mu3_NDF";
/*
 * open group /CALIBRATION
 */
     gid = NADC_OPEN_HDF5_Group(fid, "/CALIBRATION");
     if ( gid < 0 ) NADC_GOTO_ERROR(NADC_ERR_HDF_GRP, "/CALIBRATION");
/*
 * obtain dimensions
 */
     // angle_asm_limb_ndf       Dataset {9}
     if ( (dset_id = H5Dopen(gid, ds_name_asm, H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, ds_name_asm );
     (void) H5LDget_dset_dims(dset_id, &cur_dims);
     (void) H5Dclose(dset_id);
     num_asm = (unsigned short) cur_dims;
     // angle_esm_limb_ndf       Dataset {5}
     if ( (dset_id = H5Dopen(gid, ds_name_esm, H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, ds_name_esm );
     (void) H5LDget_dset_dims(dset_id, &cur_dims);
     (void) H5Dclose(dset_id);
     num_esm = (unsigned short) cur_dims;

     (void) H5Gclose(gid);
     num_pspo = num_asm * num_esm;
/*
 * open group /CALIBRATION/POLARISATION_SENSITIVITY_LIMB_OCCULTATION_NDF
 */
     gid = NADC_OPEN_HDF5_Group(fid, grp_name);
     if ( gid < 0 ) NADC_GOTO_ERROR(NADC_ERR_HDF_GRP, grp_name);
/*
 * obtain dimensions
 */
     if ( (dset_id = H5Dopen(gid, "spectral_channel", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "spectral_channel");
     (void) H5LDget_dset_dims(dset_id, &cur_dims);
     (void) H5Dclose(dset_id);
     spectral_channel = (unsigned short) cur_dims;
/*
 * allocate memory
 */
     if ( ! Use_Extern_Alloc ) {
          pspo_out[0] = (struct psplo_scia *) 
               calloc((size_t) num_pspo, sizeof(struct psplo_scia));
     }
     if ( (pspo = pspo_out[0]) == NULL ) 
          NADC_GOTO_ERROR(NADC_ERR_ALLOC, "pspo");
/*
 * read POLARISATION_SENSITIVITY_LIMB_OCCULTATION datasets
 */
     // angle_asm_limb_ndf       Dataset {num_asm}
     if ( (dset_id = H5Dopen(gid, ds_name_asm, H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, ds_name_asm);
     if ( (type_id = H5Dget_type(dset_id)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, ds_name_asm);
     if ( (val_asm = (double *) malloc(num_asm * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "val_asm" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, val_asm) < 0)
          NADC_GOTO_ERROR(NADC_ERR_HDF_RD, ds_name_asm);
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // angle_esm_limb_ndf       Dataset {num_esm}
     if ( (dset_id = H5Dopen(gid, ds_name_esm, H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, ds_name_esm);
     if ( (type_id = H5Dget_type(dset_id)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, ds_name_esm);
     if ( (val_esm = (double *) malloc(num_esm * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "val_esm" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, val_esm) < 0)
          NADC_GOTO_ERROR(NADC_ERR_HDF_RD, ds_name_esm);
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // polarisation_sensitivity_limb_mu2_ndf Dataset {num_esm, num_asm, 8192}
     dim_size = num_pspo * spectral_channel;
     if ( (dset_id = H5Dopen(gid, ds_name_mu2, H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, ds_name_mu2);
     if ( (type_id = H5Dget_type(dset_id)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, ds_name_mu2);
     if ( (val_mu2 = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "val_mu2" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, val_mu2) < 0)
          NADC_GOTO_ERROR(NADC_ERR_HDF_RD, ds_name_mu2);
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // polarisation_sensitivity_limb_mu3 Dataset {num_esm, num_asm, 8192}
     dim_size = num_pspo * spectral_channel;
     if ( (dset_id = H5Dopen(gid, ds_name_mu3, H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, ds_name_mu3);
     if ( (type_id = H5Dget_type(dset_id)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, ds_name_mu3);
     if ( (val_mu3 = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "val_mu3" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, val_mu3) < 0)
          NADC_GOTO_ERROR(NADC_ERR_HDF_RD, ds_name_mu3);
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
/*
 * store data in output structure
 */
     np = nr = 0;
     for ( nj = 0; nj < num_esm; nj++ ) {
	  for ( ni = 0; ni < num_asm; ni++ ) {
	       for ( ns = 0; ns < spectral_channel; ns++ ) {
		    pspo[np].mu2[ns] = val_mu2[nr];
		    pspo[np].mu3[ns] = val_mu3[nr];
		    nr++;
	       }
	       pspo[np].ang_esm = (float) val_esm[nj];
	       pspo[np].ang_asm = (float) val_asm[ni];
	       np++;
	  }
     }
/*
 * set return values
 */
     free(val_asm);
     free(val_esm);
     free(val_mu2);
     free(val_mu3);
     (void) H5Gclose(gid);
     return num_pspo;
done:
     if (val_asm != NULL ) free(val_asm);
     if (val_esm != NULL ) free(val_esm);
     if (val_mu2 != NULL ) free(val_mu2);
     if (val_mu3 != NULL ) free(val_mu3);
     H5E_BEGIN_TRY {
          (void) H5Tclose(type_id);
          (void) H5Dclose(dset_id);
          (void) H5Gclose(gid);
     } H5E_END_TRY;
     return 0u;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_NC_RD_PPG
.PURPOSE     read PPG/Etalon Parameters
.INPUT/OUTPUT
  call as   num = SCIA_LV1_NC_RD_PPG( fid, &ppg );
     input:
            hid_t fid       :        HDF5 file identifier
    output:
            struct ppg_scia *ppg  :  PPG/Etalon Parameters

.RETURNS     number of data set records read (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
static
unsigned int SCIA_LV1_NC_RD_PPG(hid_t fid,  /*@out@*/ struct ppg_scia *ppg)
{
     register unsigned short ns;

     unsigned short spectral_channel;

     char   *cbuff;
     double *dbuff;

     hid_t gid = -1;
     hid_t dset_id = -1;
     hid_t type_id = -1;
     hsize_t cur_dims;
/*
 * open group /CALIBRATION/PPG_ETALON
 */
     gid = NADC_OPEN_HDF5_Group( fid, "/CALIBRATION/PPG_ETALON" );
     if ( gid < 0 )
	  NADC_GOTO_ERROR( NADC_ERR_HDF_GRP, "/CALIBRATION/PPG_ETALON" );
/*
 * obtain dimensions
 */
     // spectral_channel         Dataset {8192}
     if ( (dset_id = H5Dopen( gid, "spectral_channel", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "spectral_channel" );
     (void) H5LDget_dset_dims( dset_id, &cur_dims );
     (void) H5Dclose(dset_id);
     spectral_channel = (unsigned short) cur_dims;
/*
 * read PPG_ETALON datasets
 */
     // bad_pixel_mask           Dataset {8192}
     if ( (dset_id = H5Dopen(gid, "bad_pixel_mask", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "bad_pixel_mask");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "bad_pixel_mask");
     cbuff = (char *) malloc(spectral_channel);
     if ( cbuff == NULL ) NADC_GOTO_ERROR( NADC_ERR_ALLOC, "cbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, cbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "bad_pixel_mask" );
     for ( ns = 0; ns < spectral_channel; ns++ )
	  ppg->bad_pixel[ns] = (unsigned char) cbuff[ns];
     free( cbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // etalon                   Dataset {8192}
     if ( (dset_id = H5Dopen(gid, "etalon", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "etalon");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "etalon");
     dbuff = (double *) malloc(spectral_channel * sizeof(double));
     if ( dbuff == NULL ) NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "etalon" );
     for ( ns = 0; ns < spectral_channel; ns++ )
	  ppg->etalon_fact[ns] = (float) dbuff[ns];
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // etalon_residual          Dataset {8192}
     if ( (dset_id = H5Dopen(gid, "etalon_residual", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "etalon_residual");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "etalon_residual");
     dbuff = (double *) malloc(spectral_channel * sizeof(double));
     if ( dbuff == NULL ) NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "etalon_residual" );
     for ( ns = 0; ns < spectral_channel; ns++ ) {
	  ppg->etalon_resid[ns] = (float) dbuff[ns];
	  ppg->wls_deg_fact[ns] = 0.f;
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // ppg                      Dataset {8192}
     if ( (dset_id = H5Dopen(gid, "ppg", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "ppg");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "ppg");
     dbuff = (double *) malloc(spectral_channel * sizeof(double));
     if ( dbuff == NULL ) NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "ppg" );
     for ( ns = 0; ns < spectral_channel; ns++ )
	  ppg->ppg_fact[ns] = (float) dbuff[ns];
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
/*
 * set return values
 */
     (void) H5Gclose(gid);
     return 1u;
done:
     H5E_BEGIN_TRY {
          (void) H5Tclose(type_id);
          (void) H5Dclose(dset_id);
          (void) H5Gclose(gid);
     } H5E_END_TRY;
     return 0u;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_NC_RD_RSPL
.PURPOSE     read Radiance Sensitivity Parameters (limb)
.INPUT/OUTPUT
  call as   num = SCIA_LV1_NC_RD_RSPL( fid, &rspl );
     input:
            hid_t fid        :         HDF5 file identifier
    output:
            struct rsplo_scia **rspl :  Radiation Sensitivity Parameters
                                        (limb)
.RETURNS     number of data set records read (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
static
unsigned int SCIA_LV1_NC_RD_RSPL(hid_t fid, struct rsplo_scia **rspl_out)
{
     register unsigned short ni, nj, ns;
     register unsigned int   np, nr;

     unsigned short num_asm, num_esm, spectral_channel;
     unsigned int   num_rspl;
     size_t         dim_size;

     double *val_asm = NULL, *val_esm = NULL;
     double *val_rsl = NULL;

     struct rsplo_scia *rspl;
     
     hid_t gid = -1;
     hid_t dset_id = -1;
     hid_t type_id = -1;
     hsize_t cur_dims;
     /*
      * define name of group and data sets
      */
     const char grp_name[] = \
	  "/CALIBRATION/RADIANCE_SENSITIVITY_LIMB_OCCULTATION";
     const char ds_name_asm[] = "angle_asm_limb";
     const char ds_name_esm[] = "angle_esm_limb";
     const char ds_name_rsl[] = "radiance_sensitivity_limb";
/*
 * open group /CALIBRATION
 */
     gid = NADC_OPEN_HDF5_Group(fid, "/CALIBRATION");
     if ( gid < 0 ) NADC_GOTO_ERROR(NADC_ERR_HDF_GRP, "/CALIBRATION");
/*
 * obtain dimensions
 */
     // angle_asm_limb           Dataset {23}
     if ( (dset_id = H5Dopen(gid, ds_name_asm, H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, ds_name_asm );
     (void) H5LDget_dset_dims(dset_id, &cur_dims);
     (void) H5Dclose(dset_id);
     num_asm = (unsigned short) cur_dims;
     // angle_esm_limb           Dataset {5}
     if ( (dset_id = H5Dopen(gid, ds_name_esm, H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, ds_name_esm );
     (void) H5LDget_dset_dims(dset_id, &cur_dims);
     (void) H5Dclose(dset_id);
     num_esm = (unsigned short) cur_dims;

     (void) H5Gclose(gid);
     num_rspl = num_asm * num_esm;
/*
 * open group /CALIBRATION/RADIANCE_SENSITIVITY_LIMB_OCCULTATION
 */
     gid = NADC_OPEN_HDF5_Group(fid, grp_name);
     if ( gid < 0 ) NADC_GOTO_ERROR(NADC_ERR_HDF_GRP, grp_name);
/*
 * obtain dimensions
 */
     if ( (dset_id = H5Dopen(gid, "spectral_channel", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "spectral_channel");
     (void) H5LDget_dset_dims(dset_id, &cur_dims);
     (void) H5Dclose(dset_id);
     spectral_channel = (unsigned short) cur_dims;
/*
 * allocate memory
 */
     if ( ! Use_Extern_Alloc ) {
          rspl_out[0] = (struct rsplo_scia *) 
               calloc((size_t) num_rspl, sizeof(struct rsplo_scia));
     }
     if ( (rspl = rspl_out[0]) == NULL ) 
          NADC_GOTO_ERROR(NADC_ERR_ALLOC, "rspl");
/*
 * read RADIANCE_SENSITIVITY_LIMB_OCCULTATION datasets
 */
     // angle_asm_limb           Dataset {num_asm}
     if ( (dset_id = H5Dopen(gid, ds_name_asm, H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, ds_name_asm);
     if ( (type_id = H5Dget_type(dset_id)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, ds_name_asm);
     if ( (val_asm = (double *) malloc(num_asm * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "val_asm" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, val_asm) < 0)
          NADC_GOTO_ERROR(NADC_ERR_HDF_RD, ds_name_asm);
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // angle_esm_limb           Dataset {num_esm}
     if ( (dset_id = H5Dopen(gid, ds_name_esm, H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, ds_name_esm);
     if ( (type_id = H5Dget_type(dset_id)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, ds_name_esm);
     if ( (val_esm = (double *) malloc(num_esm * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "val_esm" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, val_esm) < 0)
          NADC_GOTO_ERROR(NADC_ERR_HDF_RD, ds_name_esm);
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // radiance_sensitivity_limb Dataset {num_esm, num_asm, spectral_channel}
     dim_size = num_rspl * spectral_channel;
     if ( (dset_id = H5Dopen(gid, ds_name_rsl, H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, ds_name_rsl);
     if ( (type_id = H5Dget_type(dset_id)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, ds_name_rsl);
     if ( (val_rsl = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "val_rsl" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, val_rsl) < 0)
          NADC_GOTO_ERROR(NADC_ERR_HDF_RD, ds_name_rsl);
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
/*
 * store data in output structure
 */
     np = nr = 0;
     for ( nj = 0; nj < num_esm; nj++ ) {
	  for ( ni = 0; ni < num_asm; ni++ ) {
	       for ( ns = 0; ns < spectral_channel; ns++ )
		    rspl[np].sensitivity[ns] = val_rsl[nr++];
	       rspl[np].ang_esm = (float) val_esm[nj];
	       rspl[np].ang_asm = (float) val_asm[ni];
	       np++;
	  }
     }
/*
 * set return values
 */
     free(val_asm);
     free(val_esm);
     free(val_rsl);
     (void) H5Gclose(gid);
     return num_rspl;
done:
     if (val_asm != NULL ) free(val_asm);
     if (val_esm != NULL ) free(val_esm);
     if (val_rsl != NULL ) free(val_rsl);
     H5E_BEGIN_TRY {
          (void) H5Tclose(type_id);
          (void) H5Dclose(dset_id);
          (void) H5Gclose(gid);
     } H5E_END_TRY;
     return 0u;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_NC_RD_RSPN
.PURPOSE     read Radiance Sensitivity Parameters (nadir)
.INPUT/OUTPUT
  call as   num = SCIA_LV1_NC_RD_RSPN( fid, &rspn );
     input:
            hid_t fid        :         HDF5 file identifier
    output:
            struct rspn_scia **rspn :  Radiation Sensitivity Parameters
                                        (nadir)
.RETURNS     number of data set records read (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
static
unsigned int SCIA_LV1_NC_RD_RSPN(hid_t fid, struct rspn_scia **rspn_out)
{
     register unsigned short ns;
     register unsigned int   np, nr;

     unsigned short num_esm, spectral_channel;
     unsigned int   num_rspn;
     size_t         dim_size;

     double *val_esm = NULL;
     double *val_rsn = NULL;

     struct rspn_scia *rspn;
     
     hid_t gid = -1;
     hid_t dset_id = -1;
     hid_t type_id = -1;
     hsize_t cur_dims;
     /*
      * define name of group and data sets
      */
     const char grp_name[] = \
	  "/CALIBRATION/RADIANCE_SENSITIVITY_NADIR";
     const char ds_name_esm[] = "angle_esm_nadir";
     const char ds_name_rsn[] = "radiance_sensitivity_nadir";
/*
 * open group /CALIBRATION
 */
     gid = NADC_OPEN_HDF5_Group(fid, "/CALIBRATION");
     if ( gid < 0 ) NADC_GOTO_ERROR(NADC_ERR_HDF_GRP, "/CALIBRATION");
/*
 * obtain dimensions
 */
     // angle_esm_nadir           Dataset {17}
     if ( (dset_id = H5Dopen(gid, ds_name_esm, H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, ds_name_esm );
     (void) H5LDget_dset_dims(dset_id, &cur_dims);
     (void) H5Dclose(dset_id);
     num_esm = (unsigned short) cur_dims;

     (void) H5Gclose(gid);
     num_rspn = num_esm;
/*
 * open group /CALIBRATION/RADIANCE_SENSITIVITY_NADIR
 */
     gid = NADC_OPEN_HDF5_Group(fid, grp_name);
     if ( gid < 0 ) NADC_GOTO_ERROR(NADC_ERR_HDF_GRP, grp_name);
/*
 * obtain dimensions
 */
     if ( (dset_id = H5Dopen(gid, "spectral_channel", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "spectral_channel");
     (void) H5LDget_dset_dims(dset_id, &cur_dims);
     (void) H5Dclose(dset_id);
     spectral_channel = (unsigned short) cur_dims;
/*
 * allocate memory
 */
     if ( ! Use_Extern_Alloc ) {
          rspn_out[0] = (struct rspn_scia *) 
               calloc((size_t) num_rspn, sizeof(struct rspn_scia));
     }
     if ( (rspn = rspn_out[0]) == NULL ) 
          NADC_GOTO_ERROR(NADC_ERR_ALLOC, "rspn");
/*
 * read RADIANCE_SENSITIVITY_NADIR datasets
 */
     // angle_esm_nadir           Dataset {num_esm}
     if ( (dset_id = H5Dopen(gid, ds_name_esm, H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, ds_name_esm);
     if ( (type_id = H5Dget_type(dset_id)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, ds_name_esm);
     if ( (val_esm = (double *) malloc(num_esm * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "val_esm" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, val_esm) < 0)
          NADC_GOTO_ERROR(NADC_ERR_HDF_RD, ds_name_esm);
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // radiance_sensitivity_nadir Dataset {num_esm, spectral_channel}
     dim_size = num_rspn * spectral_channel;
     if ( (dset_id = H5Dopen(gid, ds_name_rsn, H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, ds_name_rsn);
     if ( (type_id = H5Dget_type(dset_id)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, ds_name_rsn);
     if ( (val_rsn = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "val_rsn" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, val_rsn) < 0)
          NADC_GOTO_ERROR(NADC_ERR_HDF_RD, ds_name_rsn);
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
/*
 * store data in output structure
 */
     for ( nr = np = 0; np < num_esm; np++ ) {
	  for ( ns = 0; ns < spectral_channel; ns++ )
	       rspn[np].sensitivity[ns] = val_rsn[nr++];
	  rspn[np].ang_esm = (float) val_esm[np];
     }
/*
 * set return values
 */
     free(val_esm);
     free(val_rsn);
     (void) H5Gclose(gid);
     return num_rspn;
done:
     if (val_esm != NULL ) free(val_esm);
     if (val_rsn != NULL ) free(val_rsn);
     H5E_BEGIN_TRY {
          (void) H5Tclose(type_id);
          (void) H5Dclose(dset_id);
          (void) H5Gclose(gid);
     } H5E_END_TRY;
     return 0u;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_NC_RD_RSPO
.PURPOSE     read Radiance Sensitivity Parameters (occultation)
.INPUT/OUTPUT
  call as   num = SCIA_LV1_NC_RD_RSPO( fid, &rspo );
     input:
            hid_t fid        :         HDF5 file identifier
    output:
            struct rsplo_scia **rspo :  Radiation Sensitivity Parameters
                                        (occultation)
.RETURNS     number of data set records read (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
static
unsigned int SCIA_LV1_NC_RD_RSPO(hid_t fid, struct rsplo_scia **rspo_out)
{
     register unsigned short ni, nj, ns;
     register unsigned int   np, nr;

     unsigned short num_asm, num_esm, spectral_channel;
     unsigned int   num_rspo;
     size_t         dim_size;

     double *val_asm = NULL, *val_esm = NULL;
     double *val_rsl = NULL;

     struct rsplo_scia *rspo;
     
     hid_t gid = -1;
     hid_t dset_id = -1;
     hid_t type_id = -1;
     hsize_t cur_dims;
     /*
      * define name of group and data sets
      */
     const char grp_name[] = \
	  "/CALIBRATION/RADIANCE_SENSITIVITY_LIMB_OCCULTATION_NDF";
     const char ds_name_asm[] = "angle_asm_limb_ndf";
     const char ds_name_esm[] = "angle_esm_limb_ndf";
     const char ds_name_rsl[] = "radiance_sensitivity_limb_ndf";
/*
 * open group /CALIBRATION
 */
     gid = NADC_OPEN_HDF5_Group(fid, "/CALIBRATION");
     if ( gid < 0 ) NADC_GOTO_ERROR(NADC_ERR_HDF_GRP, "/CALIBRATION");
/*
 * obtain dimensions
 */
     // angle_asm_limb_ndf           Dataset {23}
     if ( (dset_id = H5Dopen(gid, ds_name_asm, H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, ds_name_asm );
     (void) H5LDget_dset_dims(dset_id, &cur_dims);
     (void) H5Dclose(dset_id);
     num_asm = (unsigned short) cur_dims;
     // angle_esm_limb_ndf           Dataset {5}
     if ( (dset_id = H5Dopen(gid, ds_name_esm, H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, ds_name_esm );
     (void) H5LDget_dset_dims(dset_id, &cur_dims);
     (void) H5Dclose(dset_id);
     num_esm = (unsigned short) cur_dims;

     (void) H5Gclose(gid);
     num_rspo = num_asm * num_esm;
/*
 * open group /CALIBRATION/RADIANCE_SENSITIVITY_LIMB_OCCULTATION_NDF
 */
     gid = NADC_OPEN_HDF5_Group(fid, grp_name);
     if ( gid < 0 ) NADC_GOTO_ERROR(NADC_ERR_HDF_GRP, grp_name);
/*
 * obtain dimensions
 */
     if ( (dset_id = H5Dopen(gid, "spectral_channel", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "spectral_channel");
     (void) H5LDget_dset_dims(dset_id, &cur_dims);
     (void) H5Dclose(dset_id);
     spectral_channel = (unsigned short) cur_dims;
/*
 * allocate memory
 */
     if ( ! Use_Extern_Alloc ) {
          rspo_out[0] = (struct rsplo_scia *) 
               calloc((size_t) num_rspo, sizeof(struct rsplo_scia));
     }
     if ( (rspo = rspo_out[0]) == NULL ) 
          NADC_GOTO_ERROR(NADC_ERR_ALLOC, "rspo");
/*
 * read RADIANCE_SENSITIVITY_LIMB_OCCULTATION_NDF datasets
 */
     // angle_asm_limb           Dataset {num_asm}
     if ( (dset_id = H5Dopen(gid, ds_name_asm, H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, ds_name_asm);
     if ( (type_id = H5Dget_type(dset_id)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, ds_name_asm);
     if ( (val_asm = (double *) malloc(num_asm * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "val_asm" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, val_asm) < 0)
          NADC_GOTO_ERROR(NADC_ERR_HDF_RD, ds_name_asm);
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // angle_esm_limb           Dataset {num_esm}
     if ( (dset_id = H5Dopen(gid, ds_name_esm, H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, ds_name_esm);
     if ( (type_id = H5Dget_type(dset_id)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, ds_name_esm);
     if ( (val_esm = (double *) malloc(num_esm * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "val_esm" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, val_esm) < 0)
          NADC_GOTO_ERROR(NADC_ERR_HDF_RD, ds_name_esm);
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // radiance_sensitivity_limb_ndf Dataset {num_esm, num_asm, spectral_channel}
     dim_size = num_rspo * spectral_channel;
     if ( (dset_id = H5Dopen(gid, ds_name_rsl, H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, ds_name_rsl);
     if ( (type_id = H5Dget_type(dset_id)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, ds_name_rsl);
     if ( (val_rsl = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "val_rsl" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, val_rsl) < 0)
          NADC_GOTO_ERROR(NADC_ERR_HDF_RD, ds_name_rsl);
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
/*
 * store data in output structure
 */
     np = nr = 0;
     for ( nj = 0; nj < num_esm; nj++ ) {
	  for ( ni = 0; ni < num_asm; ni++ ) {
	       for ( ns = 0; ns < spectral_channel; ns++ )
		    rspo[np].sensitivity[ns] = val_rsl[nr++];
	       rspo[np].ang_esm = (float) val_esm[nj];
	       rspo[np].ang_asm = (float) val_asm[ni];
	       np++;
	  }
     }
/*
 * set return values
 */
     free(val_asm);
     free(val_esm);
     free(val_rsl);
     (void) H5Gclose(gid);
     return num_rspo;
done:
     if (val_asm != NULL ) free(val_asm);
     if (val_esm != NULL ) free(val_esm);
     if (val_rsl != NULL ) free(val_rsl);
     H5E_BEGIN_TRY {
          (void) H5Tclose(type_id);
          (void) H5Dclose(dset_id);
          (void) H5Gclose(gid);
     } H5E_END_TRY;
     return 0u;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_NC_RD_SFP
.PURPOSE     read Slit function parameters
.INPUT/OUTPUT
  call as   num = SCIA_LV1_NC_RD_SFP( fid, &sfp );
     input:
            hid_t fid        :        HDF5 file identifier
    output:
            struct sfp_scia **sfp :   structure for Slit Parameters

.RETURNS     number of data set records read (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
static
unsigned int SCIA_LV1_NC_RD_SFP(hid_t fid, struct sfp_scia **sfp_out)
{
     register unsigned int   ni, nr;

     unsigned short spectral_channel;
     unsigned int   num_sfp;
     size_t         dim_size;

     double *dbuff;

     struct sfp_scia *sfp;
     
     hid_t gid = -1;
     hid_t dset_id = -1;
     hid_t type_id = -1;
     hsize_t cur_dims;
     /*
      * define name of group and data sets
      */
     const char grp_name[] = "/CALIBRATION/SLIT_FUNCTION";
/*
 * open group /CALIBRATION/
 */
     gid = NADC_OPEN_HDF5_Group( fid, grp_name );
     if ( gid < 0 )
	  NADC_GOTO_ERROR( NADC_ERR_HDF_GRP, grp_name );
/*
 * obtain dimensions
 */
     if ( (dset_id = H5Dopen( gid, "spectral_channel", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "spectral_channel" );
     (void) H5LDget_dset_dims( dset_id, &cur_dims );
     (void) H5Dclose(dset_id);
     spectral_channel = (unsigned short) cur_dims;
     if ( (dset_id = H5Dopen( gid, "orbit_phase", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "orbit_phase" );
     (void) H5LDget_dset_dims( dset_id, &cur_dims );
     (void) H5Dclose(dset_id);
     num_sfp = (unsigned int) cur_dims;
/*
 * allocate memory
 */
     if ( ! Use_Extern_Alloc ) {
          sfp_out[0] = (struct sfp_scia *) 
               calloc((size_t) num_sfp, sizeof(struct sfp_scia));
     }
     if ( (sfp = sfp_out[0]) == NULL ) 
          NADC_GOTO_ERROR( NADC_ERR_ALLOC, "sfp" );
/*
 * read SLIT_FUNCTION datasets
 */
     // 
     dim_size = num_sfp * spectral_channel;
     if ( (dset_id = H5Dopen(gid, "", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "" );
     for ( nr = ni = 0; ni < num_sfp; ni++ ) {
	  sfp[ni].fwhm_slit_fun = (float) dbuff[nr++];
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
/*
 * set return values
 */
     (void) H5Gclose(gid);
     return num_sfp;
done:
     H5E_BEGIN_TRY {
          (void) H5Tclose(type_id);
          (void) H5Dclose(dset_id);
          (void) H5Gclose(gid);
     } H5E_END_TRY;
     return 0u;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_NC_RD_ASFP
.PURPOSE     read Small Aperture Slit function parameters
.INPUT/OUTPUT
  call as   num = SCIA_LV1_NC_RD_ASFP( fid, &asfp );
     input:
            hid_t fid        :         HDF5 file identifier
    output:
            struct asfp_scia **asfp :  structure for ASFP parameters

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
static
unsigned int SCIA_LV1_NC_RD_ASFP(hid_t fid, struct asfp_scia **asfp_out)
{
     register unsigned int   ni, nr;

     unsigned short spectral_channel;
     unsigned int   num_asfp;
     size_t         dim_size;

     double *dbuff;

     struct asfp_scia *asfp;
     
     hid_t gid = -1;
     hid_t dset_id = -1;
     hid_t type_id = -1;
     hsize_t cur_dims;
     /*
      * define name of group and data sets
      */
     const char grp_name[] = "/CALIBRATION/SMALL_AP_SLIT_FUNCTION";
/*
 * open group SMALL_AP_SLIT_FUNCTION
 */
     gid = NADC_OPEN_HDF5_Group( fid, grp_name );
     if ( gid < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_GRP, grp_name );
/*
 * obtain dimensions
 */
     if ( (dset_id = H5Dopen( gid, "spectral_channel", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "spectral_channel" );
     (void) H5LDget_dset_dims( dset_id, &cur_dims );
     (void) H5Dclose(dset_id);
     spectral_channel = (unsigned short) cur_dims;
     if ( (dset_id = H5Dopen( gid, "orbit_phase", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "orbit_phase" );
     (void) H5LDget_dset_dims( dset_id, &cur_dims );
     (void) H5Dclose(dset_id);
     num_asfp = (unsigned int) cur_dims;
/*
 * allocate memory
 */
     if ( ! Use_Extern_Alloc ) {
          asfp_out[0] = (struct asfp_scia *) 
               calloc((size_t) num_asfp, sizeof(struct asfp_scia));
     }
     if ( (asfp = asfp_out[0]) == NULL ) 
          NADC_GOTO_ERROR( NADC_ERR_ALLOC, "asfp" );
/*
 * read SMALL_AP_SLIT_FUNCTION datasets
 */
     // 
     dim_size = num_asfp * spectral_channel;
     if ( (dset_id = H5Dopen(gid, "", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "" );
     for ( nr = ni = 0; ni < num_asfp; ni++ ) {
	  asfp[ni].fwhm_slit_fun = (float) dbuff[nr++];
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
/*
 * set return values
 */
     (void) H5Gclose(gid);
     return num_asfp;
done:
     H5E_BEGIN_TRY {
          (void) H5Tclose(type_id);
          (void) H5Dclose(dset_id);
          (void) H5Gclose(gid);
     } H5E_END_TRY;
     return 0u;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_NC_RD_SCP
.PURPOSE     read Spectral Calibration Parameters records
.INPUT/OUTPUT
  call as   num = SCIA_LV1_NC_RD_SCP( fid, &scp );
     input:
            hid_t fid        :        HDF5 file identifier
    output:
            struct base_scia *scpc : Spectral Calibration Parameters (constant)
            struct scp_scia **scpv : Spectral Calibration Parameters (variable)

.RETURNS     number of data set records read (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
static
unsigned int SCIA_LV1_NC_RD_SCP(hid_t fid, struct base_scia *scpc,
				struct scp_scia **scpv_out)
{
     register unsigned short nc, ni, nj, ns;
     register unsigned int   nr;

     unsigned short coefficient, detector_channel, orbit_phase,
	  spectral_channel;
     size_t         dim_size;

     float  *rbuff;
     double *dbuff;

     struct scp_scia *scpv;
     
     hid_t gid = -1;
     hid_t dset_id = -1;
     hid_t type_id = -1;
     hsize_t cur_dims;
     /*
      * define name of group and data sets
      */
     const char grp_name[] = "/CALIBRATION/SPECTRAL_CALIBRATION";
/*
 * open group /CALIBRATION/SPECTRAL_CALIBRATION
 */
     gid = NADC_OPEN_HDF5_Group(fid, grp_name);
     if ( gid < 0 )
	  NADC_GOTO_ERROR(NADC_ERR_HDF_GRP, grp_name);
/*
 * obtain dimensions
 */
     // coefficient              Dataset {5}
     if ( (dset_id = H5Dopen(fid, "coefficient", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "coefficient");
     (void) H5LDget_dset_dims(dset_id, &cur_dims);
     (void) H5Dclose(dset_id);
     coefficient = (unsigned short) cur_dims;
     // detector_channel         Dataset {8}
     if ( (dset_id = H5Dopen(gid, "detector_channel", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "detector_channel");
     (void) H5LDget_dset_dims(dset_id, &cur_dims);
     (void) H5Dclose(dset_id);
     detector_channel = (unsigned short) cur_dims;
     // orbit_phase              Dataset {12}
     if ( (dset_id = H5Dopen(gid, "orbit_phase", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "orbit_phase");
     (void) H5LDget_dset_dims(dset_id, &cur_dims);
     (void) H5Dclose(dset_id);
     orbit_phase = (unsigned short) cur_dims;
     // spectral_channel         Dataset {8192}
     if ( (dset_id = H5Dopen(gid, "spectral_channel", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "spectral_channel");
     (void) H5LDget_dset_dims(dset_id, &cur_dims);
     (void) H5Dclose(dset_id);
     spectral_channel = (unsigned short) cur_dims;
/*
 * allocate memory
 */
     if ( ! Use_Extern_Alloc ) {
          scpv_out[0] = (struct scp_scia *) 
               calloc((size_t) orbit_phase, sizeof(struct scp_scia));
     }
     if ( (scpv = scpv_out[0]) == NULL ) 
          NADC_GOTO_ERROR( NADC_ERR_ALLOC, "scpv" );
/*
 * read LEAKAGE_CURRENT datasets
 */
     // orbit_phase              Dataset {orbit_phase}
     dim_size = orbit_phase;
     if ( (dset_id = H5Dopen(gid, "orbit_phase", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "orbit_phase");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "orbit_phase");
     if ( (rbuff = (float *) malloc(dim_size * sizeof(float))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, rbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "orbit_phase" );
     for ( ni = 0; ni < orbit_phase; ni++ ) {
	  scpv[ni].orbit_phase = rbuff[ni];
     }
     free(rbuff);
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // calibration_error        Dataset {orbit_phase, detector_channel}
     dim_size = orbit_phase * detector_channel;
     if ( (dset_id = H5Dopen(gid, "calibration_error", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "calibration_error");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "calibration_error");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "calibration_error" );
     for ( nr = ni = 0; ni < orbit_phase; ni++ ) {
	  for ( ns = 0; ns < detector_channel; ns++ )
	       scpv[ni].wv_error_calib[ns] = (float) dbuff[nr++];
     }
     free(dbuff);
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // num_lines                Dataset {orbit_phase, detector_channel}
     dim_size = orbit_phase * detector_channel;
     if ( (dset_id = H5Dopen(gid, "num_lines", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "num_lines");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "num_lines");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "num_lines" );
     for ( nr = ni = 0; ni < orbit_phase; ni++ ) {
	  for ( ns = 0; ns < detector_channel; ns++ )
	       scpv[ni].num_lines[ns] = (float) dbuff[nr++];
     }
     free(dbuff);
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // spectral_coefficients    Dataset {orbit_phase, detector_channel, coefficient}
     dim_size = orbit_phase * detector_channel * coefficient;
     if ( (dset_id = H5Dopen(gid, "spectral_coefficients", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "spectral_coefficients");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "spectral_coefficients");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "spectral_coefficients" );
     for ( nr = ni = 0; ni < orbit_phase; ni++ ) {
	  for ( nj = ns = 0; ns < detector_channel; ns++ ) {
	       for ( nc = 0; nc < coefficient; nc++ )
		    scpv[ni].coeffs[nj++] = (float) dbuff[nr++];
	  }
     }
     free(dbuff);
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // precise_basis_spectrum   Dataset {spectral_channel}
     dim_size = spectral_channel;
     if ( (dset_id = H5Dopen(gid, "precise_basis_spectrum", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "precise_basis_spectrum");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "precise_basis_spectrum");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "precise_basis_spectrum" );
     for ( ns = 0; ns < spectral_channel; ns++ ) {
	  scpc->wvlen_det_pix[ns] = (float) dbuff[ns];
     }
     free(dbuff);
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // wavelength               Dataset {orbit_phase, spectral_channel}
     dim_size = orbit_phase * spectral_channel;
     if ( (dset_id = H5Dopen(gid, "wavelength", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "wavelength");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "wavelength");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "wavelength" );
     free(dbuff);
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // wavelength_alternative   Dataset {spectral_channel}
     dim_size = spectral_channel;
     if ( (dset_id = H5Dopen(gid, "wavelength_alternative", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "wavelength_alternative");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "wavelength_alternative");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "wavelength_alternative" );
     free(dbuff);
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
/*
 * set return values
 */
     (void) H5Gclose(gid);
return (unsigned int) orbit_phase;
done:
     H5E_BEGIN_TRY {
          (void) H5Tclose(type_id);
          (void) H5Dclose(dset_id);
          (void) H5Gclose(gid);
     } H5E_END_TRY;
     return 0u;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_NC_RD_STATE
.PURPOSE     read States of the Product
.INPUT/OUTPUT
  call as   num = SCIA_LV1_NC_RD_STATE( fid, &state );
     input:
            hid_t fid        :            HDF5 file identifier
    output:
            struct state1_scia **state :  States of the product

.RETURNS     number of data set records read (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
static
unsigned int SCIA_LV1_NC_RD_STATE(hid_t fid,
				  /*@out@*/ struct state1_scia **state_out)
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, *state@*/
{
     register unsigned char  nc;
     register unsigned short ni;
     register unsigned int   ns;
     
     unsigned int num_state = 0;

     char   ref_date[25];
     
     char   *cbuff;
     short  *sbuff;
     float  *rbuff;
     double *dbuff;

     struct state1_scia *state;
     
     hid_t gid = -1;
     hid_t dset_id = -1;
     hid_t type_id = -1;
     hsize_t cur_dims;

     struct tm tm_ref;
/*
 * obtain dimensions
 */
     if ( (dset_id = H5Dopen( fid, "state", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "state" );
     (void) H5LDget_dset_dims( dset_id, &cur_dims );
     (void) H5Dclose(dset_id);
     num_state = (unsigned int) cur_dims;
/*
 * allocate memory
 */
     if ( ! Use_Extern_Alloc ) {
          state_out[0] = (struct state1_scia *) 
               calloc((size_t) num_state, sizeof(struct state1_scia));
     }
     if ( (state = state_out[0]) == NULL ) 
          NADC_GOTO_ERROR( NADC_ERR_ALLOC, "state" );
/*
 * read global attribute "time_reference"
 */
     (void) H5LTget_attribute_string(fid, "/", "time_reference", ref_date);
     (void) strptime(ref_date, "%Y-%m-%dT%H:%M:%S", &tm_ref);
/*
 * open group /STATES
 */
     gid = NADC_OPEN_HDF5_Group( fid, "/STATES" );
     if ( gid < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_GRP, "/STATES" );
/*
 * read datasets and combine in legacy struct state_rec
 */
     // number_of_clusters       Dataset {#state}
     if ( (dset_id = H5Dopen( gid, "number_of_clusters", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "number_of_clusters" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "number_of_clusters" );
     if ( (cbuff = (char *) malloc(cur_dims)) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "cbuff" );
     if ( H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, cbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "number_of_clusters" );
     for ( ns = 0; ns < num_state; ns++ ) {
	  state[ns].num_clus = cbuff[ns];
     }
     free( cbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);

     // number_of_diff_ITs       Dataset {#state}
     if ( (dset_id = H5Dopen( gid, "number_of_diff_ITs", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "number_of_diff_ITs" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "number_of_diff_ITs" );
     if ( (sbuff = (short *) malloc(cur_dims * sizeof(short))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "sbuff" );
     if ( H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, sbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "number_of_diff_ITs" );
     for ( ns = 0; ns < num_state; ns++ ) {
	  state[ns].num_intg = sbuff[ns];
     }
     free( sbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);

     // channel_id               Dataset {#state, #cluster}
     if ( (dset_id = H5Dopen( gid, "channel_id", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "channel_id" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "channel_id" );
     if ( (cbuff = (char *) malloc(64 * cur_dims)) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "cbuff" );
     if ( H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, cbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "channel_id" );
     for ( ni = ns = 0; ns < num_state; ns++ ) {
	  for ( nc = 0; nc < state[ns].num_clus; nc++ )
	       state[ns].Clcon[nc].channel = cbuff[ni + nc];
	  ni += 64;
     }
     free( cbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // cluster_id               Dataset {#state, #cluster}
     if ( (dset_id = H5Dopen( gid, "cluster_id", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "cluster_id" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "cluster_id" );
     if ( (cbuff = (char *) malloc(64 * cur_dims)) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "cbuff" );
     if ( H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, cbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "cluster_id" );
     for ( ni = ns = 0; ns < num_state; ns++ ) {
	  for ( nc = 0; nc < state[ns].num_clus; nc++ )
	       state[ns].Clcon[nc].id = cbuff[ni + nc];
	  ni += 64;
     }
     free( cbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // coaddings                Dataset {#state, #cluster}
     if ( (dset_id = H5Dopen( gid, "coaddings", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "coaddings" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "coaddings" );
     if ( (cbuff = (char *) malloc(64 * cur_dims)) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "cbuff" );
     if ( H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, cbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "coaddings" );
     for ( ni = ns = 0; ns < num_state; ns++ ) {
	  for ( nc = 0; nc < state[ns].num_clus; nc++ )
	       state[ns].Clcon[nc].coaddf = cbuff[ni + nc];
	  ni += 64;
     }
     free( cbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // delta_time               Dataset {#state}
     if ( (dset_id = H5Dopen( gid, "delta_time", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "delta_time" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "delta_time" );
     if ( (dbuff = (double *) malloc(cur_dims * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if ( H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
	  NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "delta_time" );
     for ( ns = 0; ns < num_state; ns++ ) {
	  struct mjd_envi mjd;
	  DELTA_TIME2MJD(tm_ref, dbuff[ns], &mjd);
	  (void) memcpy(&state[ns].mjd, &mjd, sizeof(struct mjd_envi));
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // different_integration_time Dataset {#state, #cluster}
     if ( (dset_id = H5Dopen( gid, "different_integration_time", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "different_integration_time" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "different_integration_time" );
     if ( (rbuff = (float *) malloc(64 * cur_dims * sizeof(float))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rbuff" );
     if ( H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, rbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "different_integration_time" );
     for ( ni = ns = 0; ns < num_state; ns++ ) {
	  for ( nc = 0; nc < state[ns].num_intg; nc++ )
	       state[ns].intg_times[nc] = (unsigned short)(16 * rbuff[ni + nc]);
	  ni += 64;
     }
     free( rbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // exposure_time            Dataset {#state, #cluster}
     if ( (dset_id = H5Dopen( gid, "exposure_time", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "exposure_time" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "exposure_time" );
     if ( (rbuff = (float *) malloc(64 * cur_dims * sizeof(float))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rbuff" );
     if ( H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, rbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "exposure_time" );
     for ( ni = ns = 0; ns < num_state; ns++ ) {
	  for ( nc = 0; nc < state[ns].num_clus; nc++ )
	       state[ns].Clcon[nc].pet = rbuff[ni + nc];
	  ni += 64;
     }
     free( rbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // integration_time         Dataset {#state, #cluster}
     if ( (dset_id = H5Dopen( gid, "integration_time", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "integration_time" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "integration_time" );
     if ( (rbuff = (float *) malloc(64 * cur_dims * sizeof(float))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rbuff" );
     if ( H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, rbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "integration_time" );
     for ( ni = ns = 0; ns < num_state; ns++ ) {
	  for ( nc = 0; nc < state[ns].num_clus; nc++ )
	       state[ns].Clcon[nc].intg_time = (unsigned short)(16 * rbuff[ni + nc]);
	  ni += 64;
     }
     free( rbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // length                   Dataset {#state, #cluster}
     if ( (dset_id = H5Dopen( gid, "length", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "length" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "length" );
     if ( (sbuff = (short *) malloc(64 * cur_dims * sizeof(short))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "sbuff" );
     if ( H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, sbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "length" );
     for ( ni = ns = 0; ns < num_state; ns++ ) {
	  for ( nc = 0; nc < state[ns].num_clus; nc++ )
	       state[ns].Clcon[nc].length = sbuff[ni + nc];
	  ni += 64;
     }
     free( sbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // longest_integration_time Dataset {#state}
     if ( (dset_id = H5Dopen( gid, "longest_integration_time", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "longest_integration_time" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "longest_integration_time" );
     if ( (rbuff = (float *) malloc(cur_dims * sizeof(float))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rbuff" );
     if ( H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, rbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "longest_integration_time" );
     for ( ns = 0; ns < num_state; ns++ ) {
	  state[ns].longest_intg_time = (unsigned short)(16 * rbuff[ns]);
     }
     free( rbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // measurement_category     Dataset {#state}
     if ( (dset_id = H5Dopen( gid, "measurement_category", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "measurement_category" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "measurement_category" );
     if ( (cbuff = (char *) malloc(cur_dims)) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "cbuff" );
     if ( H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, cbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "measurement_category" );
     for ( ns = 0; ns < num_state; ns++ ) {
	  state[ns].category = cbuff[ns];
     }
     free( cbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // number_of_PMD_integrals  Dataset {#state}
     if ( (dset_id = H5Dopen( gid, "number_of_PMD_integrals", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "number_of_PMD_integrals" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "number_of_PMD_integrals" );
     if ( (sbuff = (short *) malloc(cur_dims * sizeof(short))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "sbuff" );
     if ( H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, sbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "number_of_PMD_integrals" );
     for ( ns = 0; ns < num_state; ns++ ) {
	  state[ns].num_pmd = sbuff[ns];
     }
     free( sbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // orbit_phase              Dataset {#state}
     if ( (dset_id = H5Dopen( gid, "orbit_phase", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "orbit_phase" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "orbit_phase" );
     if ( (rbuff = (float *) malloc(cur_dims * sizeof(float))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rbuff" );
     if ( H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, rbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "orbit_phase" );
     for ( ns = 0; ns < num_state; ns++ ) {
	  state[ns].orbit_phase = rbuff[ns];
     }
     free( rbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // repetitions_longest_it   Dataset {#state}
     if ( (dset_id = H5Dopen( gid, "repetitions_longest_it", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "repetitions_longest_it" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "repetitions_longest_it" );
     if ( (sbuff = (short *) malloc(cur_dims * sizeof(short))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "sbuff" );
     if ( H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, sbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "repetitions_longest_it" );
     for ( ns = 0; ns < num_state; ns++ ) {
	  state[ns].num_dsr = sbuff[ns];
     }
     free( sbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // repetitions_shortest_it  Dataset {#state}
     if ( (dset_id = H5Dopen( gid, "repetitions_shortest_it", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "repetitions_shortest_it" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "repetitions_shortest_it" );
     if ( (sbuff = (short *) malloc(cur_dims * sizeof(short))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "sbuff" );
     if ( H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, sbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "repetitions_shortest_it" );
     for ( ns = 0; ns < num_state; ns++ ) {
     	  state[ns].num_aux = sbuff[ns];
     }
     free( sbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // shortest_integration_time Dataset {#state}
     if ( (dset_id = H5Dopen( gid, "shortest_integration_time", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "shortest_integration_time" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "shortest_integration_time" );
     if ( (rbuff = (float *) malloc(cur_dims * sizeof(float))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rbuff" );
     if ( H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, rbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "shortest_integration_time" );
     //for ( ns = 0; ns < num_state; ns++ ) {
     //	  state[ns].orbit_phase = rbuff[ns];
     //   }
     free( rbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // start_pixel              Dataset {#state, #cluster}
     if ( (dset_id = H5Dopen( gid, "start_pixel", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "start_pixel" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "start_pixel" );
     if ( (sbuff = (short *) malloc(64 * cur_dims * sizeof(short))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "sbuff" );
     if ( H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, sbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "start_pixel" );
     for ( ni = ns = 0; ns < num_state; ns++ ) {
	  for ( nc = 0; nc < state[ns].num_clus; nc++ )
	       state[ns].Clcon[nc].pixel_nr = sbuff[ni + nc];
	  ni += 64;
     }
     free( sbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // state_duration           Dataset {#state}
     if ( (dset_id = H5Dopen( gid, "state_duration", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "state_duration" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "state_duration" );
     if ( (rbuff = (float *) malloc(cur_dims * sizeof(float))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rbuff" );
     if ( H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, rbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "state_duration" );
     for ( ns = 0; ns < num_state; ns++ ) {
	  state[ns].dur_scan = (unsigned short)(16 * rbuff[ns]);
     }
     free( rbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // state_id                 Dataset {#state}
     if ( (dset_id = H5Dopen( gid, "state_id", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "state_id" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "state_id" );
     if ( (cbuff = (char *) malloc(cur_dims)) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rbuff" );
     if ( H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, cbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "state_id" );
     for ( ns = 0; ns < num_state; ns++ ) {
	  state[ns].state_id = cbuff[ns];
     }
     free( cbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // state_index              Dataset {#state}
     if ( (dset_id = H5Dopen( gid, "state_index", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "state_index" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "state_index" );
     if ( (sbuff = (short *) malloc(cur_dims * sizeof(short))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "sbuff" );
     if ( H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, sbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "state_index" );
     for ( ns = 0; ns < num_state; ns++ ) {
	  state[ns].indx = sbuff[ns];
     }
     free( sbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
/*
 * set return values
 */
     (void) H5Gclose(gid);
     return num_state;
done:
     H5E_BEGIN_TRY {
          (void) H5Tclose(type_id);
          (void) H5Dclose(dset_id);
          (void) H5Gclose(gid);
     } H5E_END_TRY;
     return 0u;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_NC_RD_LADS
.PURPOSE     read Geolocation of the state records
.INPUT/OUTPUT
  call as   num = SCIA_LV1_NC_RD_LADS( fid, &lads );
     input:
            hid_t fid          :         HDF5 file identifier
    output:
            struct lads_scia **lads :    geolocation of states

.RETURNS     number of data set records read (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
static
unsigned int SCIA_LV1_NC_RD_LADS(hid_t fid, struct lads_scia **lads_out)
{
     register unsigned short ns;
     register unsigned int   ni, nr;

     unsigned short num_corner;
     unsigned int   num_state;
     size_t         dim_size;

     double *dbuff;

     struct tm tm_ref;
     struct lads_scia *lads;

     char   ref_date[25];
     
     hid_t gid = -1;
     hid_t dset_id = -1;
     hid_t type_id = -1;
     hsize_t cur_dims;
/*
 * read global attribute "time_reference"
 */
     (void) H5LTget_attribute_string(fid, "/", "time_reference", ref_date);
     (void) strptime(ref_date, "%Y-%m-%dT%H:%M:%S", &tm_ref);
/*
 * open group STATES_GEOLOCATION
 */
     gid = NADC_OPEN_HDF5_Group( fid, "/STATES_GEOLOCATION" );
     if ( gid < 0 )
	  NADC_GOTO_ERROR( NADC_ERR_HDF_GRP, "/STATES_GEOLOCATION" );
/*
 * obtain dimensions
 */
     if ( (dset_id = H5Dopen( fid, "state", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "state" );
     (void) H5LDget_dset_dims( dset_id, &cur_dims );
     (void) H5Dclose(dset_id);
     num_state = (unsigned int) cur_dims;
     if ( (dset_id = H5Dopen( fid, "corner", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "corner" );
     (void) H5LDget_dset_dims( dset_id, &cur_dims );
     (void) H5Dclose(dset_id);
     num_corner = (unsigned short) cur_dims;
/*
 * allocate memory
 */
     if ( ! Use_Extern_Alloc ) {
          lads_out[0] = (struct lads_scia *) 
               calloc((size_t) num_state, sizeof(struct lads_scia));
     }
     if ( (lads = lads_out[0]) == NULL ) 
          NADC_GOTO_ERROR( NADC_ERR_ALLOC, "lads" );
/*
 * read STATES_GEOLOCATION datasets
 */
     // delta_time               Dataset {num_state}
     if ( (dset_id = H5Dopen(gid, "delta_time", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "delta_time");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "delta_time");
     if ( (dbuff = (double *) malloc(num_state * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "delta_time" );
     for ( ni = 0; ni < num_state; ni++ ) {
	  struct mjd_envi mjd;
	  DELTA_TIME2MJD(tm_ref, dbuff[ni], &mjd);
	  (void) memcpy(&lads[ni].mjd, &mjd, sizeof(struct mjd_envi));
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // latitude_bounds          Dataset {num_state, num_corner}
     dim_size = num_state * num_corner;
     if ( (dset_id = H5Dopen(gid, "latitude_bounds", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "latitude_bounds");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "latitude_bounds");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "latitude_bounds" );
     for ( nr = ni = 0; ni < num_state; ni++ ) {
	  for ( ns = 0; ns < num_corner; ns++ ) {
	       if ( dbuff[nr] >= -90. && dbuff[nr] <= 90 )
		    lads[ni].corner[ns].lat = NINT(1e6 * dbuff[nr]);
	       else
		    lads[ni].corner[ns].lat = 0.;
	       nr++;
	  }
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // longitude_bounds         Dataset {num_state, num_corner}
     dim_size = num_state * num_corner;
     if ( (dset_id = H5Dopen(gid, "longitude_bounds", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "longitude_bounds");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "longitude_bounds");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "longitude_bounds" );
     for ( nr = ni = 0; ni < num_state; ni++ ) {
	  for ( ns = 0; ns < num_corner; ns++ ) {
	       if ( dbuff[nr] >= -180. && dbuff[nr] <= 180.  )
		    lads[ni].corner[ns].lon = NINT(1e6 * dbuff[nr]);
	       else
		    lads[ni].corner[ns].lon = 0.;
	       nr++;
	  }
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
/*
 * set return values
 */
     (void) H5Gclose(gid);
     return num_state;
done:
     H5E_BEGIN_TRY {
          (void) H5Tclose(type_id);
          (void) H5Dclose(dset_id);
          (void) H5Gclose(gid);
     } H5E_END_TRY;
     return 0u;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_NC_RD_SQADS
.PURPOSE     read Summary of Quality flags per state records
.INPUT/OUTPUT
  call as   num = SCIA_LV1_NC_RD_SQADS( fid, &sqads );
     input:
            hid_t fid          :          HDF5 file identifier
    output:
            struct sqads1_scia **sqads :  summary of quality flags per state

.RETURNS     number of data set records read (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
static
unsigned int SCIA_LV1_NC_RD_SQADS(hid_t fid, struct sqads1_scia **sqads_out)
{
     register unsigned short ns;
     register unsigned int   ni, nr;

     unsigned short detector, num_channel;
     unsigned int   num_state;
     size_t         dim_size;

     char   *cbuff;
     short  *sbuff;
     float  *rbuff;
     double *dbuff;

     char   ref_date[25];

     struct tm tm_ref;
     struct sqads1_scia *sqads;
     
     hid_t gid = -1;
     hid_t dset_id = -1;
     hid_t type_id = -1;
     hsize_t cur_dims;
/*
 * read global attribute "time_reference"
 */
     (void) H5LTget_attribute_string(fid, "/", "time_reference", ref_date);
     (void) strptime(ref_date, "%Y-%m-%dT%H:%M:%S", &tm_ref);
/*
 * open group /STATES_QUALITY
 */
     gid = NADC_OPEN_HDF5_Group( fid, "/STATES_QUALITY" );
     if ( gid < 0 )
	  NADC_GOTO_ERROR( NADC_ERR_HDF_GRP, "/STATES_QUALITY" );
/*
 * obtain dimensions
 */
     if ( (dset_id = H5Dopen( fid, "detector", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "detector" );
     (void) H5LDget_dset_dims( dset_id, &cur_dims );
     (void) H5Dclose(dset_id);
     detector = (unsigned short) cur_dims;
     if ( (dset_id = H5Dopen( gid, "detector_channel", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "detector_channel" );
     (void) H5LDget_dset_dims( dset_id, &cur_dims );
     (void) H5Dclose(dset_id);
     num_channel = (unsigned short) cur_dims;
     if ( (dset_id = H5Dopen( fid, "state", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "state" );
     (void) H5LDget_dset_dims( dset_id, &cur_dims );
     (void) H5Dclose(dset_id);
     num_state = (unsigned int) cur_dims;
/*
 * allocate memory
 */
     if ( ! Use_Extern_Alloc ) {
          sqads_out[0] = (struct sqads1_scia *) 
               calloc((size_t) num_state, sizeof(struct sqads1_scia));
     }
     if ( (sqads = sqads_out[0]) == NULL ) 
          NADC_GOTO_ERROR( NADC_ERR_ALLOC, "sqads" );
/*
 * read STATES_QUALITY datasets
 */
     // decontamination_flag     Dataset {num_state, num_channel}
     dim_size = num_state * num_channel;
     if ( (dset_id = H5Dopen(gid, "decontamination_flag", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "decontamination_flag");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "decontamination_flag");
     if ( (cbuff = (char *) malloc(dim_size)) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "cbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, cbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "decontamination_flag" );
     //for ( nr = ni = 0; ni < num_state; ni++ ) {
     //for ( ns = 0; ns < num_channel; ns++ )
     //not_used = (unsigned char) cbuff[nr++];
     //}
     free( cbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // delta_time               Dataset {num_state}
     if ( (dset_id = H5Dopen(gid, "delta_time", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "delta_time");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "delta_time");
     if ( (dbuff = (double *) malloc(num_state * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "delta_time" );
     for ( ni = 0; ni < num_state; ni++ ) {
	  struct mjd_envi mjd;
	  DELTA_TIME2MJD(tm_ref, dbuff[ni], &mjd);
	  (void) memcpy(&sqads[ni].mjd, &mjd, sizeof(struct mjd_envi));
     }
     free( dbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // diff_fraunhofer          Dataset {num_state, num_channel}
     dim_size = num_state * num_channel;
     if ( (dset_id = H5Dopen(gid, "diff_fraunhofer", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "diff_fraunhofer");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "diff_fraunhofer");
     if ( (rbuff = (float *) malloc(dim_size * sizeof(float))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, rbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "diff_fraunhofer" );
     for ( nr = ni = 0; ni < num_state; ni++ ) {
	  for ( ns = 0; ns < num_channel; ns++ )
	       sqads[ni].mean_wv_diff[ns] = rbuff[nr++];
     }
     free( rbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // hot_pixel_counter        Dataset {num_state, detector}
     dim_size = num_state * detector;
     if ( (dset_id = H5Dopen(gid, "hot_pixel_counter", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "hot_pixel_counter");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "hot_pixel_counter");
     if ( (sbuff = (short *) malloc(dim_size * sizeof(short))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "sbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, sbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "hot_pixel_counter" );
     for ( nr = ni = 0; ni < num_state; ni++ ) {
	  for ( ns = 0; ns < detector; ns++ )
	       sqads[ni].hotpixel[ns] = (unsigned short) sbuff[nr++];
     }
     free( sbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // leakage_quality          Dataset {num_state, detector}
     dim_size = num_state * detector;
     if ( (dset_id = H5Dopen(gid, "leakage_quality", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "leakage_quality");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "leakage_quality");
     if ( (rbuff = (float *) malloc(dim_size * sizeof(float))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, rbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "leakage_quality" );
     for ( nr = ni = 0; ni < num_state; ni++ ) {
	  for ( ns = 0; ns < detector; ns++ ) {
	       if ( rbuff[nr] < 1e36 ) 
		    sqads[ni].mean_diff_leak[ns] = rbuff[nr];
	       nr++;
	  }
     }
     free( rbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // overall_quality_flag     Dataset {num_state}
     if ( (dset_id = H5Dopen(gid, "overall_quality_flag", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "overall_quality_flag");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "overall_quality_flag");
     if ( (cbuff = (char *) malloc(num_state)) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "cbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, cbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "overall_quality_flag" );
     for ( ni = 0; ni < num_state; ni++ ) {
	  sqads[ni].flag_mds = (unsigned char) cbuff[ni];
     }
     free( cbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // rainbow_flag             Dataset {num_state}
     if ( (dset_id = H5Dopen(gid, "rainbow_flag", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "rainbow_flag");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "rainbow_flag");
     if ( (cbuff = (char *) malloc(num_state)) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "cbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, cbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "rainbow_flag" );
     for ( ni = 0; ni < num_state; ni++ ) {
	  sqads[ni].flag_rainbow = (unsigned char) cbuff[ni];
     }
     free( cbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // saa_flag                 Dataset {num_state}
     if ( (dset_id = H5Dopen(gid, "saa_flag", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "saa_flag");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "saa_flag");
     if ( (cbuff = (char *) malloc(num_state)) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "cbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, cbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "saa_flag" );
     for ( ni = 0; ni < num_state; ni++ ) {
	  sqads[ni].flag_saa_region = (unsigned char) cbuff[ni];
     }
     free( cbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // stddev_fraunhofer        Dataset {num_state, num_channel}
     dim_size = num_state * num_channel;
     if ( (dset_id = H5Dopen(gid, "stddev_fraunhofer", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "stddev_fraunhofer");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "stddev_fraunhofer");
     if ( (rbuff = (float *) malloc(dim_size * sizeof(float))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, rbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "stddev_fraunhofer" );
     for ( nr = ni = 0; ni < num_state; ni++ ) {
	  for ( ns = 0; ns < num_channel; ns++ )
	       sqads[ni].sdev_wv_diff[ns] = rbuff[nr++];
     }
     free( rbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
     // sun_glint_flag           Dataset {num_state}
     if ( (dset_id = H5Dopen(gid, "sun_glint_flag", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "sun_glint_flag");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "sun_glint_flag");
     if ( (cbuff = (char *) malloc(num_state)) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "cbuff" );
     if (H5Dread(dset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, cbuff) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "sun_glint_flag" );
     for ( nr = ni = 0; ni < num_state; ni++ ) {
	  sqads[ni].flag_glint = (unsigned char) cbuff[ni];
     }
     free( cbuff );
     (void) H5Tclose(type_id);
     (void) H5Dclose(dset_id);
/*
 * set return values
 */
     (void) H5Gclose(gid);
     return num_state;
done:
     H5E_BEGIN_TRY {
          (void) H5Tclose(type_id);
          (void) H5Dclose(dset_id);
          (void) H5Gclose(gid);
     } H5E_END_TRY;
     return 0u;
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
int main(int argc, char *argv[])
     /*@globals  errno, stderr, stdout, nadc_stat, nadc_err_stack, 
       Use_Extern_Alloc;@*/
     /*@modifies errno, stderr, stdout, nadc_stat, nadc_err_stack@*/
{
     unsigned int num_state;
     unsigned int num;

     hid_t  fid = 0;

     struct param_record param;
     //struct mph_envi    mph;
     //struct sip_scia    sip;
     //struct sph1_scia   sph;
     struct sqads1_scia  *sqads = NULL;
     struct lads_scia    *lads = NULL;
     struct clcp_scia    clcp;
     struct vlcp_scia    *vlcp = NULL;
     struct ppg_scia     ppg;
     struct base_scia    scpc;
     struct scp_scia     *scpv = NULL;
     struct srs_scia     *srs = NULL;
     //struct cal_options calopt;
     struct pspn_scia    *pspn = NULL;
     struct psplo_scia   *pspl = NULL;
     struct psplo_scia   *pspo = NULL;
     struct rspn_scia    *rspn = NULL;
     struct rsplo_scia   *rspl = NULL;
     struct rsplo_scia   *rspo = NULL;
     struct ekd_scia     ekd;
     //struct asfp_scia   *asfp;
     //struct sfp_scia    *sfp;
     struct state1_scia  *state = NULL;
     //struct mds1_pmd    *pmd;
     //struct mds1_aux    *aux;
     //struct lcpn_scia   *lcpn;
     //struct ppgn_scia   *ppgn;
     //struct dark_scia   *dark;
     //struct scpn_scia   *scpn;
     //struct srsn_scia   *srsn;
/*
 * initialization of command-line parameters
 */
     SCIA_SET_PARAM(argc, argv, SCIA_LEVEL_1, &param);
     if ( IS_ERR_STAT_FATAL ) 
          NADC_GOTO_ERROR(NADC_ERR_PARAM, "NADC_INIT_PARAM");
/*
 * check if we have to display version and exit
 */
     if ( param.flag_version == PARAM_SET ) {
	  SCIA_SHOW_VERSION(stdout, "scia_lv1_nc");
	  exit(EXIT_SUCCESS);
     }
/*
 * dump command-line parameters
 */
     if ( param.flag_show == PARAM_SET ) {
	  SCIA_SHOW_PARAM(SCIA_LEVEL_1, param);
	  exit(EXIT_SUCCESS);
     }
/*
 * open input-file
 */
     fid = H5Fopen(param.infile, H5F_ACC_RDONLY, H5P_DEFAULT);
     if ( fid < 0 ) NADC_GOTO_ERROR(NADC_ERR_HDF_FILE, param.infile);
/*
 * -------------------------
 * read/write Main Product Header
 */
     /* SCIA_NC_RD_MPH(fid, &mph); */
     /* if ( IS_ERR_STAT_FATAL )  */
     /* 	  NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "MPH"); */
/*
 * -------------------------
 * read/write Specific Product Header
 */
     /* SCIA_LV1_NC_RD_SPH(fid, &sph); */
     /* if ( IS_ERR_STAT_FATAL )  */
     /* 	  NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "SPH"); */
/*
 * -------------------------
 * read/write Summary of Quality Flags per State records
 */
     (void) fprintf(stdout, "start SCIA_LV1_NC_RD_SQADS\n");
     num = SCIA_LV1_NC_RD_SQADS(fid, &sqads);
     if ( IS_ERR_STAT_FATAL || num ==  0 )
	  NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "SQADS");
     SCIA_LV1_WR_ASCII_SQADS(param, num, sqads);
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "SQADS" );
/*
 * -------------------------
 * read/write Geolocation of the States
 */
     (void) fprintf(stdout, "start SCIA_LV1_NC_RD_LADS\n");
     num = SCIA_LV1_NC_RD_LADS(fid, &lads);
     if ( IS_ERR_STAT_FATAL || num ==  0 )
	  NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "LADS");
     SCIA_WR_ASCII_LADS(param, num, lads);
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "LADS" );
/*
 * -------------------------
 * read/write Static Instrument Parameters
 */
/*
 * -------------------------
 * read/write Leakage Current Parameters (constant fraction)
 */
     (void) fprintf(stdout, "start SCIA_LV1_NC_RD_CLCP\n");
     num = SCIA_LV1_NC_RD_CLCP(fid, &clcp);
     if ( IS_ERR_STAT_FATAL || num ==  0 )
	  NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "CLCP");
     SCIA_LV1_WR_ASCII_CLCP(param, &clcp);
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "CLCP" );
/*
 * -------------------------
 * read/write Leakage Current Parameters (variable fraction)
 */
     (void) fprintf(stdout, "start SCIA_LV1_NC_RD_VLCP\n");
     num = SCIA_LV1_NC_RD_VLCP(fid, &vlcp);
     if ( IS_ERR_STAT_FATAL || num ==  0 )
	  NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "VLCP");
     SCIA_LV1_WR_ASCII_VLCP(param, num, vlcp);
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "VLCP" );
     free(vlcp);
/*
 * -------------------------
 * read/write PPG/Etalon Parameters
 */
     (void) fprintf(stdout, "start SCIA_LV1_NC_RD_PPG\n");
     num = SCIA_LV1_NC_RD_PPG(fid, &ppg);
     if ( IS_ERR_STAT_FATAL || num ==  0 )
	  NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "PPG");
     SCIA_LV1_WR_ASCII_PPG(param, &ppg);
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR(NADC_ERR_FILE_WR, "PPG");
/*
 * -------------------------
 * read/write Precise Basis of the Spectral Calibration
 * read/write Spectral Calibration Parameters
 */
     (void) fprintf(stdout, "start SCIA_LV1_NC_RD_SCP\n");
     num = SCIA_LV1_NC_RD_SCP(fid, &scpc, &scpv);
     if ( IS_ERR_STAT_FATAL || num ==  0 )
	  NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "SCP");
     SCIA_LV1_WR_ASCII_BASE(param, &scpc);
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "BASE" );
     SCIA_LV1_WR_ASCII_SCP(param, num, scpv);
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "SCP" );
     free(scpv);
/*
 * -------------------------
 * read/write Sun Reference Spectrum 
 *    always write this GADS, because it is required by most retrievals
 */
     (void) fprintf(stdout, "start SCIA_LV1_NC_RD_SRS\n");
     num = SCIA_LV1_NC_RD_SRS(fid, &srs);
     if ( IS_ERR_STAT_FATAL || num ==  0 )
     	  NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "SRS");
     SCIA_LV1_WR_ASCII_SRS(param, num, srs);
     if ( IS_ERR_STAT_FATAL )
     	  NADC_GOTO_ERROR(NADC_ERR_FILE_WR, "SRS");
     free(srs);
/*
 * -------------------------
 * read/write Polarisation Sensitivity Parameters Nadir
 */
     (void) fprintf(stdout, "start SCIA_LV1_NC_RD_PSPN\n");
     num = SCIA_LV1_NC_RD_PSPN(fid, &pspn);
     if ( IS_ERR_STAT_FATAL || num ==  0 )
     	  NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "PSPN");
     SCIA_LV1_WR_ASCII_PSPN(param, num, pspn);
     if ( IS_ERR_STAT_FATAL )
     	  NADC_GOTO_ERROR(NADC_ERR_FILE_WR, "PSPN");
     free(pspn);
/*
 * -------------------------
 * read/write Polarisation Sensitivity Parameters Limb
 */
     (void) fprintf(stdout, "start SCIA_LV1_NC_RD_PSPL\n");
     num = SCIA_LV1_NC_RD_PSPL(fid, &pspl);
     if ( IS_ERR_STAT_FATAL || num ==  0 )
     	  NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "PSPL");
     SCIA_LV1_WR_ASCII_PSPL(param, num, pspl);
     if ( IS_ERR_STAT_FATAL )
     	  NADC_GOTO_ERROR(NADC_ERR_FILE_WR, "PSPL");
     free(pspl);
/*
 * -------------------------
 * read/write Polarisation Sensitivity Parameters Occultation
 */
     (void) fprintf(stdout, "start SCIA_LV1_NC_RD_PSPO\n");
     num = SCIA_LV1_NC_RD_PSPO(fid, &pspo);
     if ( IS_ERR_STAT_FATAL || num ==  0 )
     	  NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "PSPO");
     SCIA_LV1_WR_ASCII_PSPO(param, num, pspo);
     if ( IS_ERR_STAT_FATAL )
     	  NADC_GOTO_ERROR(NADC_ERR_FILE_WR, "PSPO");
     free(pspo);
/*
 * -------------------------
 * read/write Radiation Sensitivity Parameters Nadir
 */
     (void) fprintf(stdout, "start SCIA_LV1_NC_RD_RSPN\n");
     num = SCIA_LV1_NC_RD_RSPN(fid, &rspn);
     if ( IS_ERR_STAT_FATAL || num ==  0 )
     	  NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "RSPN");
     SCIA_LV1_WR_ASCII_RSPN(param, num, rspn);
     if ( IS_ERR_STAT_FATAL )
     	  NADC_GOTO_ERROR(NADC_ERR_FILE_WR, "RSPN");
     free(rspn);
/*
 * -------------------------
 * read/write Radiation Sensitivity Parameters Limb
 */
     (void) fprintf(stdout, "start SCIA_LV1_NC_RD_RSPL\n");
     num = SCIA_LV1_NC_RD_RSPL(fid, &rspl);
     if ( IS_ERR_STAT_FATAL || num ==  0 )
     	  NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "RSPL");
     SCIA_LV1_WR_ASCII_RSPL(param, num, rspl);
     if ( IS_ERR_STAT_FATAL )
     	  NADC_GOTO_ERROR(NADC_ERR_FILE_WR, "RSPL");
     free(rspl);
/*
 * -------------------------
 * read/write Radiation Sensitivity Parameters Occultation
 */
     (void) fprintf(stdout, "start SCIA_LV1_NC_RD_RSPO\n");
     num = SCIA_LV1_NC_RD_RSPO(fid, &rspo);
     if ( IS_ERR_STAT_FATAL || num ==  0 )
     	  NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "RSPO");
     SCIA_LV1_WR_ASCII_RSPO(param, num, rspo);
     if ( IS_ERR_STAT_FATAL )
     	  NADC_GOTO_ERROR(NADC_ERR_FILE_WR, "RSPO");
     free(rspo);
/*
 * -------------------------
 * read/write Errors on Key Data
 */
     (void) fprintf(stdout, "start SCIA_LV1_NC_RD_EKD\n");
     num = SCIA_LV1_NC_RD_EKD(fid, &ekd);
     if ( IS_ERR_STAT_FATAL || num ==  0 )
	  NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "EKD");
     SCIA_LV1_WR_ASCII_EKD(param, &ekd);
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "EKD" );
/*
 * -------------------------
 * read/write Slit Function Parameters
 */
/*
 * -------------------------
 * read/write Small Aperture Slit Function Parameters
 */
/* -------------------------
 * read/write States of the Product
 */
     (void) fprintf(stdout, "start SCIA_LV1_NC_RD_STATE\n");
     num_state = SCIA_LV1_NC_RD_STATE(fid, &state);
     if ( IS_ERR_STAT_FATAL || num_state  ==  0 )
	  NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "STATE");
     SCIA_LV1_WR_ASCII_STATE(param, num_state, state);
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "STATE" );
     free(state);
/*
 * -------------------------
 * read/write PMD Data Packets
 */
/*
 * -------------------------
 * read/write Auxiliary Data Packets
 */
/*
 * -------------------------
 * read/write Leakage Current Parameters (newly calculated)
 */
/*
 * -------------------------
 * read/write Average of the Dark Measurements per State
 */
/*
 * -------------------------
 * read/write PPG/Etalon Parameters (newly calculated)
 */
/*
 * -------------------------
 * read/write Spectral Calibration Parameters (newly calculated)
 */
/*
 * -------------------------
 * read/write Sun Reference Spectrum (newly calculated)
 */
/*
 * -------------------------
 * read/write Nadir MDS
 */
/*
 * -------------------------
 * read/write Limb MDS
 */
/*
 * -------------------------
 * read/write Occultation MDS
 */
/*
 * -------------------------
 * read/write Monitoring MDS
 */
/*
 * free allocated memory
 */
/*
 * when an error has occurred we jump to here:
 */
 done:
/*
 * close input file and PDS file
 */
     (void) fprintf(stdout, "close input file\n");
     if ( fid > 0 ) {
	  (void) H5Fclose(fid);
     }
/*
 * display error messages?
 */
     (void) fprintf(stdout, "any messages?\n");
     if ( param.flag_silent == PARAM_UNSET ) NADC_Err_Trace(stderr);
     if ( IS_ERR_STAT_FATAL ) 
	  return NADC_ERR_FATAL;
     else
	  return NADC_ERR_NONE;
}
