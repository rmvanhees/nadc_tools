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
.VERSION      1.0   30-Dec-2017 created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

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
     //(void) fprintf(stdout, "%d %d %d %d %d %.10f\n",
     //tm.tm_year, tm.tm_yday, tm.tm_hour, tm.tm_min, tm.tm_sec,
     //delta_time);
     mjd->days = tm.tm_yday;
     tm.tm_year -= 100;
     for ( ni = 0; ni < tm.tm_year; ni++ ) 
	  mjd->days += (ni % 4 == 0 ? 366 : 365);
     mjd->secnd = 60 * (60 * tm.tm_hour + tm.tm_min) + tm.tm_sec;
     mjd->musec = NINT(1e6 * (delta_time - (int) delta_time));
}

static
void SCIA_NC_RD_MPH(hid_t fid, struct mph_envi *mph)
{
     hid_t gid = -1;
/*
 * open group /METADATA
 */
     gid = NADC_OPEN_HDF5_Group( fid, "/METADATA" );
     if ( gid < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_GRP, "/METADATA" );
/*
 * set return values
 */
     (void) H5Gclose( gid );
     return;
done:
     H5E_BEGIN_TRY {
          (void) H5Fclose( gid );
     } H5E_END_TRY;
     return;
}

static
void SCIA_LV1_NC_RD_SPH(hid_t fid, struct sph1_scia *sph)
{
     hid_t gid = -1;
/*
 * open group /METADATA
 */
     gid = NADC_OPEN_HDF5_Group( fid, "/METADATA" );
     if ( gid < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_GRP, "/METADATA" );
/*
 * set return values
 */
     (void) H5Gclose( gid );
     return;
done:
     H5E_BEGIN_TRY {
          (void) H5Fclose( gid );
     } H5E_END_TRY;
     return;
}

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
     (void) H5Dclose( dset_id );
     pmd2 = (unsigned short) cur_dims;
     if ( (dset_id = H5Dopen( gid, "spectral_channel", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "spectral_channel" );
     (void) H5LDget_dset_dims( dset_id, &cur_dims );
     (void) H5Dclose( dset_id );
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
     if (H5LTread_dataset( gid, "fixed_pattern_noise", type_id, dbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "fixed_pattern_noise" );
     for ( ns = 0; ns < spectral_channel; ns++ ) {
     	  clcp->fpn[ns] = (float) dbuff[ns];
     }
     free( dbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // fixed_pattern_noise_error Dataset {spectral_channel}
     if ( (dset_id = H5Dopen(gid, "fixed_pattern_noise_error", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "fixed_pattern_noise_error");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "fixed_pattern_noise_error");
     if ((dbuff = (double *) malloc(spectral_channel * sizeof(double))) == NULL)
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5LTread_dataset( gid, "fixed_pattern_noise_error", type_id, dbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "fixed_pattern_noise_error" );
     for ( ns = 0; ns < spectral_channel; ns++ ) {
     	  clcp->fpn_error[ns] = (float) dbuff[ns];
     }
     free( dbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // leakage_current          Dataset {spectral_channel}
     if ( (dset_id = H5Dopen(gid, "leakage_current", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "leakage_current");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "leakage_current");
     if ((dbuff = (double *) malloc(spectral_channel * sizeof(double))) == NULL)
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5LTread_dataset( gid, "leakage_current", type_id, dbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "leakage_current" );
     for ( ns = 0; ns < spectral_channel; ns++ ) {
     	  clcp->lc[ns] = (float) dbuff[ns];
     }
     free( dbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // leakage_current_error    Dataset {spectral_channel}
     if ( (dset_id = H5Dopen(gid, "leakage_current_error", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "leakage_current_error");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "leakage_current_error");
     if ((dbuff = (double *) malloc(spectral_channel * sizeof(double))) == NULL)
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5LTread_dataset( gid, "leakage_current_error", type_id, dbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "leakage_current_error" );
     for ( ns = 0; ns < spectral_channel; ns++ ) {
     	  clcp->lc_error[ns] = (float) dbuff[ns];
     }
     free( dbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // mean_noise               Dataset {spectral_channel}
     if ( (dset_id = H5Dopen(gid, "mean_noise", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "mean_noise");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "mean_noise");
     if ((dbuff = (double *) malloc(spectral_channel * sizeof(double))) == NULL)
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5LTread_dataset( gid, "mean_noise", type_id, dbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "mean_noise" );
     for ( ns = 0; ns < spectral_channel; ns++ ) {
     	  clcp->mean_noise[ns] = (float) dbuff[ns];
     }
     free( dbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // pmd_offset               Dataset {pmd2}
     if ( (dset_id = H5Dopen(gid, "pmd_offset", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "pmd_offset");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "pmd_offset");
     if ( (dbuff = (double *) malloc(pmd2 * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5LTread_dataset( gid, "pmd_offset", type_id, dbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "pmd_offset" );
     for ( ns = 0; ns < pmd2; ns++ ) {
     	  clcp->pmd_dark[ns] = (float) dbuff[ns];
     }
     free( dbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // pmd_offset_error         Dataset {pmd2}
     if ( (dset_id = H5Dopen(gid, "pmd_offset_error", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "pmd_offset_error");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "pmd_offset_error");
     if ( (dbuff = (double *) malloc(pmd2 * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5LTread_dataset( gid, "pmd_offset_error", type_id, dbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "pmd_offset_error" );
     for ( ns = 0; ns < pmd2; ns++ ) {
     	  clcp->pmd_dark_error[ns] = (float) dbuff[ns];
     }
     free( dbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
/*
 * set return values
 */
     (void) H5Gclose( gid );
     return 1;
done:
     H5E_BEGIN_TRY {
          (void) H5Tclose( type_id );
          (void) H5Dclose( dset_id );
          (void) H5Fclose( gid );
     } H5E_END_TRY;
     return 0;
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
     (void) H5Dclose( dset_id );
     num_pmd = (unsigned short) cur_dims;
     if ( (dset_id = H5Dopen( fid, "pmd_ir", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "pmd_ir" );
     (void) H5LDget_dset_dims( dset_id, &cur_dims );
     (void) H5Dclose( dset_id );
     num_pmd_ir = (unsigned short) cur_dims;
     if ( (dset_id = H5Dopen( gid, "temperature", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "temperature" );
     (void) H5LDget_dset_dims( dset_id, &cur_dims );
     (void) H5Dclose( dset_id );
     num_temp = (unsigned int) cur_dims;
     if ( (dset_id = H5Dopen( gid, "spectral_channel", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "spectral_channel" );
     (void) H5LDget_dset_dims( dset_id, &cur_dims );
     (void) H5Dclose( dset_id );
     spectral_channel = (unsigned short) cur_dims;
     if ( (dset_id = H5Dopen( gid, "spectral_channel_ir", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "spectral_channel_ir" );
     (void) H5LDget_dset_dims( dset_id, &cur_dims );
     (void) H5Dclose( dset_id );
     spectral_channel_ir = (unsigned short) cur_dims;
     if ( (dset_id = H5Dopen( gid, "orbit_phase", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "orbit_phase" );
     (void) H5LDget_dset_dims( dset_id, &cur_dims );
     (void) H5Dclose( dset_id );
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
     if ( (dbuff = (double *) malloc(num_vlcp * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5LTread_dataset( gid, "orbit_phase", type_id, dbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "orbit_phase" );
     for ( ni = 0; ni < num_vlcp; ni++ ) {
	  vlcp[ni].orbit_phase = (float) dbuff[ni];
     }
     free( dbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // channel_temperature      Dataset {num_vlcp, num_temp}
     dim_size = num_vlcp * num_temp;
     if ( (dset_id = H5Dopen(gid, "channel_temperature", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "channel_temperature");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "channel_temperature");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5LTread_dataset( gid, "channel_temperature", type_id, dbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "channel_temperature" );
     for ( nr = ni = 0; ni < num_vlcp; ni++ ) {
	  nr += 1;
	  for ( ns = 0; ns < num_temp-1; ns++ )
	       vlcp[ni].obm_pmd[ns] = (float) dbuff[nr++];
     }
     free( dbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // leakage_current          Dataset {num_vlcp, spectral_channel_ir}
     dim_size = num_vlcp * spectral_channel_ir;
     if ( (dset_id = H5Dopen(gid, "leakage_current", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "leakage_current");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "leakage_current");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5LTread_dataset( gid, "leakage_current", type_id, dbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "leakage_current" );
     for ( nr = ni = 0; ni < num_vlcp; ni++ ) {
	  for ( ns = 0; ns < spectral_channel_ir; ns++ )
	       vlcp[ni].var_lc[ns] = (float) dbuff[nr++];
     }
     free( dbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // leakage_current_error    Dataset {num_vlcp, spectral_channel_ir}
     dim_size = num_vlcp * spectral_channel_ir;
     if ( (dset_id = H5Dopen(gid, "leakage_current_error", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "leakage_current_error");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "leakage_current_error");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5LTread_dataset( gid, "leakage_current_error", type_id, dbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "leakage_current_error" );
     for ( nr = ni = 0; ni < num_vlcp; ni++ ) {
	  for ( ns = 0; ns < spectral_channel_ir; ns++ )
	       vlcp[ni].var_lc_error[ns] = (float) dbuff[nr++];
     }
     free( dbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // pmd_offset               Dataset {num_vlcp, num_pmd_ir}
     dim_size = num_vlcp * num_pmd_ir;
     if ( (dset_id = H5Dopen(gid, "pmd_offset", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "pmd_offset");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "pmd_offset");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5LTread_dataset( gid, "pmd_offset", type_id, dbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "pmd_offset" );
     for ( nr = ni = 0; ni < num_vlcp; ni++ ) {
	  for ( ns = 0; ns < num_pmd_ir; ns++ )
	       vlcp[ni].pmd_dark[ns] = (float) dbuff[nr++];
     }
     free( dbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // pmd_offset_error         Dataset {num_vlcp, num_pmd_ir}
     dim_size = num_vlcp * num_pmd_ir;
     if ( (dset_id = H5Dopen(gid, "pmd_offset_error", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "pmd_offset_error");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "pmd_offset_error");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5LTread_dataset( gid, "pmd_offset_error", type_id, dbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "pmd_offset_error" );
     for ( nr = ni = 0; ni < num_vlcp; ni++ ) {
	  for ( ns = 0; ns < num_pmd_ir; ns++ )
	       vlcp[ni].pmd_dark_error[ns] = (float) dbuff[nr++];
     }
     free( dbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // pmd_straylight           Dataset {num_vlcp, num_pmd}
     dim_size = num_vlcp * num_pmd;
     if ( (dset_id = H5Dopen(gid, "pmd_straylight", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "pmd_straylight");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "pmd_straylight");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5LTread_dataset( gid, "pmd_straylight", type_id, dbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "pmd_straylight" );
     for ( nr = ni = 0; ni < num_vlcp; ni++ ) {
	  for ( ns = 0; ns < num_pmd; ns++ )
	       vlcp[ni].pmd_stray[ns] = (float) dbuff[nr++];
     }
     free( dbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // pmd_straylight_error     Dataset {num_vlcp, num_pmd}
     dim_size = num_vlcp * num_pmd;
     if ( (dset_id = H5Dopen(gid, "pmd_straylight_error", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "pmd_straylight_error");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "pmd_straylight_error");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5LTread_dataset( gid, "pmd_straylight_error", type_id, dbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "pmd_straylight_error" );
     for ( nr = ni = 0; ni < num_vlcp; ni++ ) {
	  for ( ns = 0; ns < num_pmd; ns++ )
	       vlcp[ni].pmd_stray_error[ns] = (float) dbuff[nr++];
     }
     free( dbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // solar_straylight         Dataset {num_vlcp, spectral_channel}
     dim_size = num_vlcp * spectral_channel;
     if ( (dset_id = H5Dopen(gid, "solar_straylight", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "solar_straylight");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "solar_straylight");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5LTread_dataset( gid, "solar_straylight", type_id, dbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "solar_straylight" );
     for ( nr = ni = 0; ni < num_vlcp; ni++ ) {
	  for ( ns = 0; ns < spectral_channel; ns++ )
	       vlcp[ni].solar_stray[ns] = (float) dbuff[nr++];
     }
     free( dbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // solar_straylight_error   Dataset {num_vlcp, spectral_channel}
     dim_size = num_vlcp * spectral_channel;
     if ( (dset_id = H5Dopen(gid, "solar_straylight_error", H5P_DEFAULT)) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "solar_straylight_error");
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR(NADC_ERR_HDF_DTYPE, "solar_straylight_error");
     if ( (dbuff = (double *) malloc(dim_size * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if (H5LTread_dataset( gid, "solar_straylight_error", type_id, dbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "solar_straylight_error" );
     for ( nr = ni = 0; ni < num_vlcp; ni++ ) {
	  for ( ns = 0; ns < spectral_channel; ns++ )
	       vlcp[ni].solar_stray[ns] = (float) dbuff[nr++];
     }
     free( dbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
/*
 * set return values
 */
     (void) H5Gclose( gid );
     return num_vlcp;
done:
     H5E_BEGIN_TRY {
          (void) H5Tclose( type_id );
          (void) H5Dclose( dset_id );
          (void) H5Fclose( gid );
     } H5E_END_TRY;
     return 0;
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

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_NV_RD_PSPL
.PURPOSE     read Polarisation Sensitivity Parameters (limb)
.INPUT/OUTPUT
  call as   num = SCIA_LV1_RD_PSPL( fid, &pspl );
     input:
            hid_t fid        :         HDF5 file identifier 
    output:
            struct pspn_scia **pspl :  Polarisation Sensitivity Parameters
                                        (limb)
.RETURNS     number of data set records read (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/

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

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_NV_RD_PSPL
.PURPOSE     read Polarisation Sensitivity Parameters (occultation)
.INPUT/OUTPUT
  call as   num = SCIA_LV1_RD_PSPL( fid, &pspo );
     input:
            hid_t fid        :         HDF5 file identifier 
    output:
            struct pspn_scia **pspo :  Polarisation Sensitivity Parameters
                                        (occultation)
.RETURNS     number of data set records read (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/

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

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_NC_RD_RSPL
.PURPOSE     read Radiance Sensitivity Parameters (limb)
.INPUT/OUTPUT
  call as   num = SCIA_LV1_NC_RD_RSPL( fid, &rspl );
     input:
            hid_t fid        :         HDF5 file identifier
    output:
            struct rspn_scia **rspl :  Radiation Sensitivity Parameters
                                        (limb)
.RETURNS     number of data set records read (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/

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

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_NC_RD_RSPO
.PURPOSE     read Radiance Sensitivity Parameters (occultation)
.INPUT/OUTPUT
  call as   num = SCIA_LV1_NC_RD_RSPO( fid, &rspo );
     input:
            hid_t fid        :         HDF5 file identifier
    output:
            struct rspn_scia **rspo :  Radiation Sensitivity Parameters
                                        (occultation)
.RETURNS     number of data set records read (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/

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

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_NC_RD_SCP
.PURPOSE     read Spectral Calibration Parameters records
.INPUT/OUTPUT
  call as   num = SCIA_LV1_NC_RD_SCP( fid, &scp );
     input:
            hid_t fid        :        HDF5 file identifier
    output:
            struct scp_scia **scp :   Spectral Calibration Parameters

.RETURNS     number of data set records read (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/

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
unsigned int SCIA_LV1_NC_RD_STATE(hid_t fid, struct state1_scia **state_out)
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

     // attribute 'time_reference'
     (void) H5LTget_attribute_string(fid, "/", "time_reference", ref_date);
     (void) strptime(ref_date, "%Y-%m-%dT%H:%M:%S", &tm_ref);
/*
 * obtain dimensions
 */
     if ( (dset_id = H5Dopen( fid, "state", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "state" );
     (void) H5LDget_dset_dims( dset_id, &cur_dims );
     (void) H5Dclose( dset_id );
     num_state = (unsigned short) cur_dims;
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
     if ( H5LTread_dataset( gid, "number_of_clusters", type_id, cbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "number_of_clusters" );
     for ( ns = 0; ns < num_state; ns++ ) {
	  state[ns].num_clus = cbuff[ns];
     }
     free( cbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );

     // number_of_diff_ITs       Dataset {#state}
     if ( (dset_id = H5Dopen( gid, "number_of_diff_ITs", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "number_of_diff_ITs" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "number_of_diff_ITs" );
     if ( (sbuff = (short *) malloc(cur_dims * sizeof(short))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "sbuff" );
     if ( H5LTread_dataset( gid, "number_of_diff_ITs", type_id, sbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "number_of_diff_ITs" );
     for ( ns = 0; ns < num_state; ns++ ) {
	  state[ns].num_intg = sbuff[ns];
     }
     free( sbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );

     // channel_id               Dataset {#state, #cluster}
     if ( (dset_id = H5Dopen( gid, "channel_id", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "channel_id" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "channel_id" );
     if ( (cbuff = (char *) malloc(64 * cur_dims)) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "cbuff" );
     if ( H5LTread_dataset( gid, "channel_id", type_id, cbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "channel_id" );
     for ( ni = ns = 0; ns < num_state; ns++ ) {
	  for ( nc = 0; nc < state[ns].num_clus; nc++ )
	       state[ns].Clcon[nc].channel = cbuff[ni + nc];
	  ni += 64;
     }
     free( cbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // cluster_id               Dataset {#state, #cluster}
     if ( (dset_id = H5Dopen( gid, "cluster_id", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "cluster_id" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "cluster_id" );
     if ( (cbuff = (char *) malloc(64 * cur_dims)) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "cbuff" );
     if ( H5LTread_dataset( gid, "cluster_id", type_id,  cbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "cluster_id" );
     for ( ni = ns = 0; ns < num_state; ns++ ) {
	  for ( nc = 0; nc < state[ns].num_clus; nc++ )
	       state[ns].Clcon[nc].id = cbuff[ni + nc];
	  ni += 64;
     }
     free( cbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // coaddings                Dataset {#state, #cluster}
     if ( (dset_id = H5Dopen( gid, "coaddings", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "coaddings" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "coaddings" );
     if ( (cbuff = (char *) malloc(64 * cur_dims)) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "cbuff" );
     if ( H5LTread_dataset( gid, "coaddings", type_id, cbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "coaddings" );
     for ( ni = ns = 0; ns < num_state; ns++ ) {
	  for ( nc = 0; nc < state[ns].num_clus; nc++ )
	       state[ns].Clcon[nc].coaddf = cbuff[ni + nc];
	  ni += 64;
     }
     free( cbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // delta_time               Dataset {#state}
     if ( (dset_id = H5Dopen( gid, "delta_time", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "delta_time" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "delta_time" );
     if ( (dbuff = (double *) malloc(cur_dims * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );
     if ( H5LTread_dataset( gid, "delta_time", type_id, dbuff ) < 0 )
	  NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "delta_time" );
     for ( ns = 0; ns < num_state; ns++ ) {
	  struct mjd_envi mjd;
	  DELTA_TIME2MJD(tm_ref, dbuff[ns], &mjd);
	  (void) memcpy(&state[ns].mjd, &mjd, sizeof(struct mjd_envi));
     }
     free( dbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // different_integration_time Dataset {#state, #cluster}
     if ( (dset_id = H5Dopen( gid, "different_integration_time", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "different_integration_time" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "different_integration_time" );
     if ( (rbuff = (float *) malloc(64 * cur_dims * sizeof(float))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rbuff" );
     if ( H5LTread_dataset( gid, "different_integration_time", type_id, rbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "different_integration_time" );
     for ( ni = ns = 0; ns < num_state; ns++ ) {
	  for ( nc = 0; nc < state[ns].num_intg; nc++ )
	       state[ns].intg_times[nc] = (unsigned short)(16 * rbuff[ni + nc]);
	  ni += 64;
     }
     free( rbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // exposure_time            Dataset {#state, #cluster}
     if ( (dset_id = H5Dopen( gid, "exposure_time", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "exposure_time" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "exposure_time" );
     if ( (rbuff = (float *) malloc(64 * cur_dims * sizeof(float))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rbuff" );
     if ( H5LTread_dataset( gid, "exposure_time", type_id, rbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "exposure_time" );
     for ( ni = ns = 0; ns < num_state; ns++ ) {
	  for ( nc = 0; nc < state[ns].num_clus; nc++ )
	       state[ns].Clcon[nc].pet = rbuff[ni + nc];
	  ni += 64;
     }
     free( rbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // integration_time         Dataset {#state, #cluster}
     if ( (dset_id = H5Dopen( gid, "integration_time", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "integration_time" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "integration_time" );
     if ( (rbuff = (float *) malloc(64 * cur_dims * sizeof(float))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rbuff" );
     if ( H5LTread_dataset( gid, "integration_time", type_id, rbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "integration_time" );
     for ( ni = ns = 0; ns < num_state; ns++ ) {
	  for ( nc = 0; nc < state[ns].num_clus; nc++ )
	       state[ns].Clcon[nc].intg_time = (unsigned short)(16 * rbuff[ni + nc]);
	  ni += 64;
     }
     free( rbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // length                   Dataset {#state, #cluster}
     if ( (dset_id = H5Dopen( gid, "length", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "length" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "length" );
     if ( (sbuff = (short *) malloc(64 * cur_dims * sizeof(short))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "sbuff" );
     if ( H5LTread_dataset( gid, "length", type_id, sbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "length" );
     for ( ni = ns = 0; ns < num_state; ns++ ) {
	  for ( nc = 0; nc < state[ns].num_clus; nc++ )
	       state[ns].Clcon[nc].length = sbuff[ni + nc];
	  ni += 64;
     }
     free( sbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // longest_integration_time Dataset {#state}
     if ( (dset_id = H5Dopen( gid, "longest_integration_time", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "longest_integration_time" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "longest_integration_time" );
     if ( (rbuff = (float *) malloc(cur_dims * sizeof(float))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rbuff" );
     if ( H5LTread_dataset( gid, "longest_integration_time", type_id, rbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "longest_integration_time" );
     for ( ns = 0; ns < num_state; ns++ ) {
	  state[ns].longest_intg_time = (unsigned short)(16 * rbuff[ns]);
     }
     free( rbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // measurement_category     Dataset {#state}
     if ( (dset_id = H5Dopen( gid, "measurement_category", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "measurement_category" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "measurement_category" );
     if ( (cbuff = (char *) malloc(cur_dims)) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "cbuff" );
     if ( H5LTread_dataset( gid, "measurement_category", type_id, cbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "measurement_category" );
     for ( ns = 0; ns < num_state; ns++ ) {
	  state[ns].type_mds = cbuff[ns];
     }
     free( cbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // number_of_PMD_integrals  Dataset {#state}
     if ( (dset_id = H5Dopen( gid, "number_of_PMD_integrals", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "number_of_PMD_integrals" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "number_of_PMD_integrals" );
     if ( (sbuff = (short *) malloc(cur_dims * sizeof(short))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "sbuff" );
     if ( H5LTread_dataset( gid, "number_of_PMD_integrals", type_id, sbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "number_of_PMD_integrals" );
     for ( ns = 0; ns < num_state; ns++ ) {
	  state[ns].num_pmd = sbuff[ns];
     }
     free( sbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // orbit_phase              Dataset {#state}
     if ( (dset_id = H5Dopen( gid, "orbit_phase", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "orbit_phase" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "orbit_phase" );
     if ( (rbuff = (float *) malloc(cur_dims * sizeof(float))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rbuff" );
     if ( H5LTread_dataset( gid, "orbit_phase", type_id, rbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "orbit_phase" );
     for ( ns = 0; ns < num_state; ns++ ) {
	  state[ns].orbit_phase = rbuff[ns];
     }
     free( rbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // repetitions_longest_it   Dataset {#state}
     if ( (dset_id = H5Dopen( gid, "repetitions_longest_it", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "repetitions_longest_it" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "repetitions_longest_it" );
     if ( (sbuff = (short *) malloc(cur_dims * sizeof(short))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "sbuff" );
     if ( H5LTread_dataset( gid, "repetitions_longest_it", type_id, sbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "repetitions_longest_it" );
     for ( ns = 0; ns < num_state; ns++ ) {
	  state[ns].num_dsr = sbuff[ns];
     }
     free( sbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // repetitions_shortest_it  Dataset {#state}
     if ( (dset_id = H5Dopen( gid, "repetitions_shortest_it", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "repetitions_shortest_it" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "repetitions_shortest_it" );
     if ( (sbuff = (short *) malloc(cur_dims * sizeof(short))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "sbuff" );
     if ( H5LTread_dataset( gid, "repetitions_shortest_it", type_id, sbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "repetitions_shortest_it" );
     for ( ns = 0; ns < num_state; ns++ ) {
     	  state[ns].num_aux = sbuff[ns];
     }
     free( sbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // shortest_integration_time Dataset {#state}
     if ( (dset_id = H5Dopen( gid, "shortest_integration_time", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "shortest_integration_time" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "shortest_integration_time" );
     if ( (rbuff = (float *) malloc(cur_dims * sizeof(float))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rbuff" );
     if ( H5LTread_dataset( gid, "shortest_integration_time", type_id, rbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "shortest_integration_time" );
     //for ( ns = 0; ns < num_state; ns++ ) {
     //	  state[ns].orbit_phase = rbuff[ns];
     //   }
     free( rbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // start_pixel              Dataset {#state, #cluster}
     if ( (dset_id = H5Dopen( gid, "start_pixel", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "start_pixel" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "start_pixel" );
     if ( (sbuff = (short *) malloc(64 * cur_dims * sizeof(short))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "sbuff" );
     if ( H5LTread_dataset( gid, "start_pixel", type_id, sbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "start_pixel" );
     for ( ni = ns = 0; ns < num_state; ns++ ) {
	  for ( nc = 0; nc < state[ns].num_clus; nc++ )
	       state[ns].Clcon[nc].pixel_nr = sbuff[ni + nc];
	  ni += 64;
     }
     free( sbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // state_duration           Dataset {#state}
     if ( (dset_id = H5Dopen( gid, "state_duration", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "state_duration" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "state_duration" );
     if ( (rbuff = (float *) malloc(cur_dims * sizeof(float))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rbuff" );
     if ( H5LTread_dataset( gid, "state_duration", type_id, rbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "state_duration" );
     for ( ns = 0; ns < num_state; ns++ ) {
	  state[ns].dur_scan = (unsigned short)(16 * rbuff[ns]);
     }
     free( rbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // state_id                 Dataset {#state}
     if ( (dset_id = H5Dopen( gid, "state_id", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "state_id" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "state_id" );
     if ( (cbuff = (char *) malloc(cur_dims)) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rbuff" );
     if ( H5LTread_dataset( gid, "state_id", type_id, cbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "state_id" );
     for ( ns = 0; ns < num_state; ns++ ) {
	  state[ns].state_id = cbuff[ns];
     }
     free( cbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
     // state_index              Dataset {#state}
     if ( (dset_id = H5Dopen( gid, "state_index", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "state_index" );
     if ( (type_id = H5Dget_type( dset_id )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, "state_index" );
     if ( (sbuff = (short *) malloc(cur_dims * sizeof(short))) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "sbuff" );
     if ( H5LTread_dataset( gid, "state_index", type_id, sbuff ) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "state_index" );
     for ( ns = 0; ns < num_state; ns++ ) {
	  state[ns].indx = sbuff[ns];
     }
     free( sbuff );
     (void) H5Tclose( type_id );
     (void) H5Dclose( dset_id );
/*
 * set return values
 */
     (void) H5Gclose( gid );
     return num_state;
done:
     H5E_BEGIN_TRY {
          (void) H5Tclose( type_id );
          (void) H5Dclose( dset_id );
          (void) H5Fclose( gid );
     } H5E_END_TRY;
     return 0;
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


/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
int main(int argc, char *argv[])
     /*@globals  errno, stderr, stdout, nadc_stat, nadc_err_stack, 
       Use_Extern_Alloc;@*/
     /*@modifies errno, stderr, stdout, nadc_stat, nadc_err_stack@*/
{
     unsigned int num_dsd, num_dsr, num_state;
     unsigned int num;

     int is_scia_lv1c;

     hid_t  fid = 0;

     struct param_record param;
     struct base_scia   base;
     struct mph_envi    mph;
     struct sip_scia    sip;
     struct sph1_scia   sph;
     struct dsd_envi    *dsd;
     struct sqads1_scia *sqads;
     struct lads_scia   *lads;
     struct clcp_scia   clcp;
     struct vlcp_scia   *vlcp;
     struct ppg_scia    ppg;
     struct scp_scia    *scp;
     struct srs_scia    *srs;
     struct cal_options calopt;
     struct pspn_scia   *pspn;
     struct psplo_scia  *pspl;
     struct psplo_scia  *pspo;
     struct rspn_scia   *rspn;
     struct rsplo_scia  *rspl;
     struct rsplo_scia  *rspo;
     struct ekd_scia    ekd;
     struct asfp_scia   *asfp;
     struct sfp_scia    *sfp;
     struct state1_scia *state, *mds_state;
     struct mds1_pmd    *pmd;
     struct mds1_aux    *aux;
     struct lcpn_scia   *lcpn;
     struct ppgn_scia   *ppgn;
     struct dark_scia   *dark;
     struct scpn_scia   *scpn;
     struct srsn_scia   *srsn;
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
     SCIA_NC_RD_MPH(fid, &mph);
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "MPH");
/*
 * -------------------------
 * read/write Specific Product Header
 */
     SCIA_LV1_NC_RD_SPH(fid, &sph);
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "SPH");
/*
 * -------------------------
 * read/write Summary of Quality Flags per State records
 */
/*
 * -------------------------
 * read/write Geolocation of the States
 */
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
/*
 * -------------------------
 * read/write Precise Basis of the Spectral Calibration
 */
/*
 * -------------------------
 * read/write Spectral Calibration Parameters
 */
/*
 * -------------------------
 * read/write Sun Reference Spectrum 
 *    always write this GADS, because it is required by most retrievals
 */
/*
 * -------------------------
 * read/write Polarisation Sensitivity Parameters Nadir
 */
/*
 * -------------------------
 * read/write Polarisation Sensitivity Parameters Limb
 */
/*
 * -------------------------
 * read/write Polarisation Sensitivity Parameters Occultation
 */
/*
 * -------------------------
 * read/write Radiation Sensitivity Parameters Nadir
 */
/*
 * -------------------------
 * read/write Radiation Sensitivity Parameters Limb
 */
/*
 * -------------------------
 * read/write Radiation Sensitivity Parameters Occultation
 */
/*
 * -------------------------
 * read/write Errors on Key Data
 */
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
     if ( fid > 0 ) {
	  if ( ! IS_ERR_STAT_FATAL && param.write_pds == PARAM_SET ) {
	       if ( IS_ERR_STAT_FATAL )
		    NADC_ERROR(NADC_ERR_FATAL, "LV1_WR_DSD_UPDATE");
	  }
	  (void) H5Fclose(fid);
     }
/*
 * close HDF5 output file
 */
     if ( param.write_hdf5 == PARAM_SET ) {
	  if ( param.hdf_file_id >= 0 && H5Fclose(param.hdf_file_id) < 0 )
	       NADC_ERROR(NADC_ERR_HDF_FILE, param.hdf5_name);
     }
/*
 * display error messages?
 */
     if ( param.flag_silent == PARAM_UNSET ) NADC_Err_Trace(stderr);
     if ( IS_ERR_STAT_FATAL ) 
	  return NADC_ERR_FATAL;
     else
	  return NADC_ERR_NONE;
}
