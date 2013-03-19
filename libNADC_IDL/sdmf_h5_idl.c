/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2009 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SDMF_H5_IDL
.AUTHOR      R.M. van Hees
.KEYWORDS    SDMF, HDF5, IDL
.LANGUAGE    ANSI C
.PURPOSE     IDL wrapper to interface with SRON SDMF (hdf5)
.COMMENTS    None
.ENVIRONment None
.VERSION     1.4     29-Mar-2012   big clean-up, RvH
             1.3     25-Aug-2010   rewrite of _SDMF_GET_NUM_STATES, RvH
             1.2     02-Aug-2009   added _SDMF_RD_EXTRACT_DARK, RvH
             1.1     26-Aug-2009   added _SDMF_OVERWRITE_METATABLE, PvdM
             1.0     09-Jan-2009   initial release by R. M. van Hees
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
#include <nadc_idl.h>
#include <nadc_sdmf.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
static const char err_msg[] = "invalid number of function arguments";

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
				/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
int IDL_STDCALL _SDMF_RD_PT_CLUSDEF( int argc, void *argv[] )
{
     const char prognm[] = "_SDMF_RD_PT_CLUSDEF";

     IDL_STRING         *dbName;
     struct clusdef_rec *ClusDef;

     hid_t  fid = -1;
     hid_t  data_id = -1;
     hid_t  type_id = -1;
/*
 * check number of parameters
 */
     if ( argc != 2 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     dbName   = (IDL_STRING *) argv[0];
     ClusDef  = (struct clusdef_rec *) argv[1];

     fid = H5Fopen( dbName->s, H5F_ACC_RDONLY, H5P_DEFAULT );
     if ( fid < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, dbName->s );
     
     if ( (data_id = H5Dopen( fid, "ClusDef", H5P_DEFAULT )) < 0 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DATA, "ClusDef" );

     if ( (type_id = H5Dget_type( data_id )) < 0 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, "ClusDef" );

     if ( H5LTread_dataset( fid, "ClusDef", type_id, ClusDef ) < 0 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_RD, "ClusDef" );

     (void) H5Tclose( type_id );
     (void) H5Dclose( data_id );
     (void) H5Fclose( fid );
     return 1;
 done:
     H5E_BEGIN_TRY {
	  (void) H5Tclose( type_id );
	  (void) H5Dclose( data_id );
	  (void) H5Fclose( fid );
     } H5E_END_TRY;
     return -1;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++*/
int IDL_STDCALL _SDMF_RD_PT_GEO_ATTR( int argc, void *argv[] )
{
     const char prognm[] = "_SDMF_RD_PT_GEO_ATTR";

     IDL_STRING      *dbName;
     unsigned short  state_id;
     unsigned short  *num_obs;

     char    grpName[9];
     hid_t   fid = -1, gid = -1;
/*
 * check number of parameters
 */
     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     dbName   = (IDL_STRING *) argv[0];
     state_id = *(unsigned short *) argv[1];
     num_obs  = (unsigned short *) argv[2];

     fid = H5Fopen( dbName->s, H5F_ACC_RDONLY, H5P_DEFAULT );
     if ( fid < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, dbName->s );

     (void) snprintf( grpName, 9, "State_%02hu", state_id );
     gid = H5Gopen( fid, grpName, H5P_DEFAULT );
     if ( gid < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, grpName );

     if ( H5LTget_attribute_ushort( gid, "pointing", "numObs", num_obs ) < 0 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_ATTR, "numObs" );

     (void) H5Gclose( gid );
     (void) H5Fclose( fid );
     return 1;
 done:
     H5E_BEGIN_TRY {
	  (void) H5Gclose( gid );
	  (void) H5Fclose( fid );
     } H5E_END_TRY;
     return -1;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++*/
int IDL_STDCALL _SDMF_RD_PT_CLUS_ATTR( int argc, void *argv[] )
{
     const char prognm[] = "_SDMF_RD_PT_CLUS_ATTR";

     IDL_STRING      *dbName;
     unsigned char   *coaddf;
     unsigned short  state_id, clus_id;
     unsigned short  *num_pixels, *num_obs;
     float           *pet;

     char    grpName[9], clusName[11];
     hid_t   fid = -1, gid = -1;
/*
 * check number of parameters
 */
     if ( argc != 7 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     dbName   = (IDL_STRING *) argv[0];
     state_id = *(unsigned short *) argv[1];
     clus_id  = *(unsigned short *) argv[2];
     coaddf   = (unsigned char *) argv[3];
     num_pixels = (unsigned short *) argv[4];
     num_obs  = (unsigned short *) argv[5];
     pet      = (float *) argv[6];

     fid = H5Fopen( dbName->s, H5F_ACC_RDONLY, H5P_DEFAULT );
     if ( fid < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, dbName->s );

     (void) snprintf( grpName, 9, "State_%02hu", state_id );
     gid = H5Gopen( fid, grpName, H5P_DEFAULT );
     if ( gid < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, grpName );

     (void) snprintf( clusName, 11, "cluster_%02hu", clus_id );
     (void) H5LTget_attribute_uchar( gid, clusName, "coaddf", coaddf );
     (void) H5LTget_attribute_ushort( gid, clusName, "numObs", num_obs );
     (void) H5LTget_attribute_ushort( gid, clusName,"numPixels", num_pixels );
     if ( H5LTget_attribute_float( gid, clusName, "PET", pet ) < 0 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_ATTR, "PET" );

     (void) H5Gclose( gid );
     (void) H5Fclose( fid );
     return 1;
 done:
     H5E_BEGIN_TRY {
	  (void) H5Gclose( gid );
	  (void) H5Fclose( fid );
     } H5E_END_TRY;
     return -1;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++*/
int IDL_STDCALL _SDMF_GET_PT_ORBITINDEX( int argc, void *argv[] )
{
     const char prognm[] = "_SDMF_GET_PT_ORBITINDEX";

     register size_t nr;

     IDL_STRING     *dbName;
     int            orbit;
     unsigned short state_id;
     unsigned int   *numIndx, *metaIndx;

     char   grpName[9];
     size_t C_numIndx, *C_metaIndx;
     hid_t  fid = -1, gid = -1;
/*
 * check number of parameters
 */
     if ( argc != 5 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     dbName   = (IDL_STRING *) argv[0];
     state_id = *(unsigned short *) argv[1];
     orbit    = *(int *) argv[2];
     numIndx  = (unsigned int *) argv[3];
     metaIndx = (unsigned int *) argv[4];

     fid = H5Fopen( dbName->s, H5F_ACC_RDONLY, H5P_DEFAULT );
     if ( fid < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, dbName->s );

     (void) snprintf( grpName, 9, "State_%02hu", state_id );
     gid = H5Gopen( fid, grpName, H5P_DEFAULT );
     if ( gid < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, grpName );

     C_numIndx  = (size_t) (*numIndx);
     C_metaIndx = (size_t *) malloc( C_numIndx * sizeof( size_t ));
     SDMF_get_pt_orbitIndex( gid, orbit, &C_numIndx, C_metaIndx );
     *numIndx = (unsigned int) C_numIndx;
     for ( nr = 0; nr < C_numIndx; nr++ )
	  metaIndx[nr] = (unsigned int) C_metaIndx[nr];
     free( C_metaIndx );

     (void) H5Gclose( gid );
     (void) H5Fclose( fid );
     return 1;
 done:
     H5E_BEGIN_TRY {
	  (void) H5Gclose( gid );
	  (void) H5Fclose( fid );
     } H5E_END_TRY;
     return -1;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++*/
int IDL_STDCALL _SDMF_RD_PT_POINTING( int argc, void *argv[] )
{
     const char prognm[] = "_SDMF_RD_PT_POINTING";

     register size_t nr;

     IDL_STRING        *dbName;
     unsigned short    state_id;
     unsigned int      numIndx, *metaIndx;
     struct geo_pt_rec *pointing;

     char   grpName[9];
     size_t C_numIndx, *C_metaIndx;
     hid_t  fid = -1, gid = -1;
/*
 * check number of parameters
 */
     if ( argc != 5 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     dbName   = (IDL_STRING *) argv[0];
     state_id = *(unsigned short *) argv[1];
     numIndx  = *(unsigned int *) argv[2];
     metaIndx = (unsigned int *) argv[3];
     pointing = (struct geo_pt_rec *) argv[4];

     if ( numIndx == 0 ) return 0;

     fid = H5Fopen( dbName->s, H5F_ACC_RDONLY, H5P_DEFAULT );
     if ( fid < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, dbName->s );

     (void) snprintf( grpName, 9, "State_%02hu", state_id );
     gid = H5Gopen( fid, grpName, H5P_DEFAULT );
     if ( gid < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, grpName );

     C_numIndx  = (size_t) numIndx;
     C_metaIndx = (size_t *) malloc( C_numIndx * sizeof( size_t ));
     for ( nr = 0; nr < C_numIndx; nr++ )
	  C_metaIndx[nr] = (size_t) metaIndx[nr];

     SDMF_rd_pt_pointing( gid, &C_numIndx, C_metaIndx, pointing );

     free( C_metaIndx );

     (void) H5Gclose( gid );
     (void) H5Fclose( fid );
     return 1;
 done:
     H5E_BEGIN_TRY {
	  (void) H5Gclose( gid );
	  (void) H5Fclose( fid );
     } H5E_END_TRY;
     return -1;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++*/
int IDL_STDCALL _SDMF_RD_PT_CLUSTER( int argc, void *argv[] )
{
     const char prognm[] = "_SDMF_RD_PT_CLUSTER";

     register size_t nr;

     IDL_STRING     *dbName;
     unsigned short clus_id, state_id;
     unsigned int   numIndx, *metaIndx;
     float          *pixel_val;
     struct mtbl_pt_rec *mtbl;

     char   grpName[9];
     size_t C_numIndx, *C_metaIndx;
     hid_t  fid = -1, gid = -1;
/*
 * check number of parameters
 */
     if ( argc != 7 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     dbName   = (IDL_STRING *) argv[0];
     state_id = *(unsigned short *) argv[1];
     clus_id  = *(unsigned short *) argv[2];
     numIndx  = *(unsigned int *) argv[3];
     metaIndx = (unsigned int *) argv[4];
     mtbl      = (struct mtbl_pt_rec *) argv[5];
     pixel_val = (float *) argv[6];

     if ( numIndx == 0 ) return 0;

     fid = H5Fopen( dbName->s, H5F_ACC_RDONLY, H5P_DEFAULT );
     if ( fid < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, dbName->s );

     (void) snprintf( grpName, 9, "State_%02hu", state_id );
     gid = H5Gopen( fid, grpName, H5P_DEFAULT );
     if ( gid < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, grpName );

     C_numIndx  = (size_t) numIndx;
     C_metaIndx = (size_t *) malloc( C_numIndx * sizeof( size_t ));
     for ( nr = 0; nr < C_numIndx; nr++ )
	  C_metaIndx[nr] = (size_t) metaIndx[nr];

     SDMF_rd_pt_metaTable( gid, &C_numIndx, C_metaIndx, &mtbl );
     SDMF_rd_pt_cluster( gid, clus_id, &C_numIndx, C_metaIndx, pixel_val );

     free( C_metaIndx );

     (void) H5Gclose( gid );
     (void) H5Fclose( fid );
     return 1;
 done:
     H5E_BEGIN_TRY {
	  (void) H5Gclose( gid );
	  (void) H5Fclose( fid );
     } H5E_END_TRY;
     return -1;
}
