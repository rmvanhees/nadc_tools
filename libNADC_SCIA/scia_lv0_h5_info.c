/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2008 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV0_H5_INFO
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 0 
.LANGUAGE    ANSI C
.PURPOSE     read/write info-records from HDF5 Packet Table database
.RETURNS     non-negative on success, negative on failure
.COMMENTS    contains SCIA_LV0_WR_H5_INFO_DB, SCIA_LV0_RD_H5_INFO_DB
.ENVIRONment None
.VERSION     1.1     26-Jan-2009   renamed modules, RvH
             1.0     12-Nov-2008   initial release by R. M. van Hees
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
#define _SCIA_LEVEL_0
#include <nadc_scia.h>

/*+++++ Macros +++++*/
        /* NONE */

/*+++++ Static Variables +++++*/
        /* NONE */

/*+++++ Global Variables +++++*/
        /* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
        /* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_RD_H5_INFO_DB
.PURPOSE     read info-records to database
.INPUT/OUTPUT
  call as    num_info = SCIA_LV0_RD_H5_INFO_DB( prodName, &info );
     input:
           char prodName[]         : HDF5 identifier of file or group
	   struct mds0_info **info : level 0 info-records

.RETURNS     number of info-records read from database (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV0_RD_H5_INFO_DB( const char prodName[],
				     struct mds0_info **info_out )
{
     const char prognm[] = "SCIA_LV0_RD_H5_INFO_DB";

     char   dbname[MAX_STRING_LENGTH];

     register bool found;
     register unsigned int nr;

     hid_t   fid = -1;
     hid_t   gid = -1;
     hid_t   ptable;
     hsize_t nrecords, start;

     unsigned int num_info = 0u;
     unsigned int magicNumber, *magicList;

     struct offs_size_rec offsLength;
     struct mds0_info     *info;
     struct h5_mds0_info  *h5_info;

     info_out[0] = NULL;
/*
 * open HDF5 database
 */
     (void) snprintf( dbname, MAX_STRING_LENGTH, "./%s", SCIA_INFO_DB_NAME );
     H5E_BEGIN_TRY {
          fid = H5Fopen( dbname, H5F_ACC_RDONLY, H5P_DEFAULT );
          if ( fid < 0 ) {
               (void) snprintf( dbname, MAX_STRING_LENGTH,
                                "%s/%s", DATA_DIR, SCIA_INFO_DB_NAME );
               fid = H5Fopen( dbname, H5F_ACC_RDONLY, H5P_DEFAULT );
          }
     } H5E_END_TRY;
     if ( fid < 0 ) goto done;
/*
 * check if product is already stored in the database
 */
     if ( (ptable = H5PTopen( fid, "productIDs" )) == H5I_BADID )
	  goto done;

     (void) H5PTget_num_packets( ptable, &nrecords );
     if ( nrecords == 0 ) {
	  (void) H5PTclose( ptable );
	  goto done;
     }
     magicList = (unsigned int *) malloc( (size_t) nrecords * sizeof(int) );
     if ( magicList == NULL ) {
	  (void) H5PTclose( ptable );
          NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "magicList" );
     }
     (void) H5PTread_packets( ptable, 0, (size_t) nrecords, magicList );

     /* loop over product IDs */
     magicNumber = GET_SCIA_MAGIC_ID( prodName );

     found = FALSE;
     nr = 0;
     do {
	  if ( magicNumber == magicList[nr] ) found = TRUE;
     } while ( ! found && --nr < (unsigned int) nrecords );
     free( magicList );
     (void) H5PTclose( ptable );
     if ( ! found ) goto done;
     start = nr;
/*
 * process Detector packets
 */
     gid = H5Gopen( fid, "DET", H5P_DEFAULT );
     if ( gid < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_GRP, "DET" );

     if ( (ptable = H5PTopen( gid, "offsLength" )) == H5I_BADID )
	  goto done;
     (void) H5PTread_packets( ptable, start, 1, &offsLength );
     (void) H5PTclose( ptable );

     info = (struct mds0_info *)
	  calloc( offsLength.length, sizeof(struct mds0_info) );
     if ( info == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "info" );
     if ( (ptable = H5PTopen( gid, "info_clus" )) == H5I_BADID )
	  goto done;
     (void) H5PTread_packets( ptable, offsLength.offset, 
			      (size_t) offsLength.length, info );
     (void) H5PTclose( ptable );
     (void) H5Gclose( gid );
     num_info = offsLength.length;
/*
 * process Auxiliary packets
 */
     gid = H5Gopen( fid, "AUX", H5P_DEFAULT );
     if ( gid < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_GRP, "AUX" );

      if ( (ptable = H5PTopen( gid, "offsLength" )) == H5I_BADID )
	  goto done;
     (void) H5PTread_packets( ptable, start, 1, &offsLength );
     (void) H5PTclose( ptable );

     h5_info = (struct h5_mds0_info *)
	  calloc( offsLength.length, sizeof(struct h5_mds0_info) );
     if ( h5_info == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "h5_info" );
     
     if ( (ptable = H5PTopen( gid, "info" )) == H5I_BADID )
	  goto done;
     (void) H5PTread_packets( ptable, offsLength.offset, 
			      (size_t) offsLength.length, h5_info );
     (void) H5PTclose( ptable );

     info = (struct mds0_info *)
	  realloc( info, 
		   (num_info + offsLength.length) * sizeof(struct mds0_info) );
     if ( info == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "info" );
     for ( nr = 0; nr < offsLength.length; nr++ ) {
	  (void) memcpy( &info[num_info+nr],
			 h5_info+nr, sizeof(struct h5_mds0_info) );
	  (void) memset( &info[num_info+nr].cluster, 0,
			 MAX_CLUSTER * sizeof(struct info_clus) );
     }
     free( h5_info );
     (void) H5Gclose( gid );
     num_info += offsLength.length;
/*
 * process PMD packets
 */
     gid = H5Gopen( fid, "PMD", H5P_DEFAULT );
     if ( gid < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_GRP, "PMD" );

     if ( (ptable = H5PTopen( gid, "offsLength" )) == H5I_BADID )
	  goto done;
     (void) H5PTread_packets( ptable, start, 1, &offsLength );
     (void) H5PTclose( ptable );

     h5_info = (struct h5_mds0_info *)
	  calloc( offsLength.length, sizeof(struct h5_mds0_info) );
     if ( h5_info == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "h5_info" );
     
     if ( (ptable = H5PTopen( gid, "info" )) == H5I_BADID )
	  goto done;
     (void) H5PTread_packets( ptable, offsLength.offset, 
			      (size_t) offsLength.length, h5_info );
     (void) H5PTclose( ptable );

     info = (struct mds0_info *)
	  realloc( info, 
		   (num_info + offsLength.length) * sizeof(struct mds0_info) );
     if ( info == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "info" );
     for ( nr = 0; nr < offsLength.length; nr++ ) {
	  (void) memcpy( &info[num_info+nr],
			 h5_info+nr, sizeof(struct h5_mds0_info) );
	  (void) memset( &info[num_info+nr].cluster, 0,
			 MAX_CLUSTER * sizeof(struct info_clus) );
     }
     free( h5_info );
     (void) H5Gclose( gid );
     (void) H5Fclose( fid );

     num_info += offsLength.length;
     info_out[0] = info;
     return num_info;
 done:
     if ( gid > 0 ) (void) H5Gclose( gid );
     if ( fid > 0 ) (void) H5Fclose( fid );

     return num_info;
}
