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

.IDENTifer   SDMF_PT_DB
.AUTHOR      R.M. van Hees
.KEYWORDS    SDMF - HDF5 (packet table)
.LANGUAGE    ANSI C
.PURPOSE     reoutines to access the SDMF packet table databases
.COMMENTS    contains: SDMF_get_pt_orbitIndex, SDMF_get_pt_jdayIndex,
                       SDMF_rd_pt_metaTable, SDMF_rd_pt_pointing,
		       SDMF_rd_pt_cluster

             The modules SDMF_fill_ll_msd1c and SDMF_fill_sun_msd1c are rather
	     high level read-routines, without calibration. To obtain 
	     calibrated values:
	     num_mds = SDMF_fill_sun_msd1c( fid, absOrbit, state_id, clus_id, &mds );
	     (void) strcpy( calib_str, "0+,1+,2+,5+,7k,9+" );
	     calib_mask = NADC_SCIA_CalibMask( calib_str );
	     SCIA_L1C_CAL( calib_mask, num_mds, mds_1c );

.ENVIRONment None
.VERSION     1.1     09-Jan-2009   renamed to include access to Sun db, RvH
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
#include <nadc_sdmf.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SDMF_get_pt_orbitIndex
.PURPOSE     obtain indices to last-limb/Sun records for given orbit number
.INPUT/OUTPUT
  call as    SDMF_get_pt_orbitIndex( locID, absOrbit, &numIndx, metaIndx );
     input:
           hid_t  locID        :  HDF5 identifier of group
	   int    absOrbit     :  orbit number
 in/output:
	   size_t *numIndx     :  [input]  dimension of metaIndx
                                  [output] number of indices found
    output:
	   size_t *metaIndx    :  array with requested indices

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
void SDMF_get_pt_orbitIndex( hid_t locID, int absOrbit, 
			     size_t *numIndx, size_t *metaIndx )
{
     const size_t dimArray = *numIndx;

     register size_t nr;

     hid_t    ptable;
     hsize_t  nrecords;

     int      *orbitList;
/*
 * initialize return values
 */
     *numIndx = 0;
     *metaIndx = 0;
/*
 * check if orbitList exists
 */
     if ( (ptable = H5PTopen( locID, "orbitList" )) == H5I_BADID )
          NADC_RETURN_ERROR( NADC_ERR_HDF_DATA, "orbitList" );

     (void) H5PTget_num_packets( ptable, &nrecords );
     if ( nrecords == 0 ) {
          (void) H5PTclose( ptable );
          return;
     }
/*
 * read list of orbits stored sofar
 */
     orbitList = (int *) malloc( (size_t) nrecords * sizeof(int) );
     if ( orbitList == NULL )
          NADC_GOTO_ERROR( NADC_ERR_ALLOC, "orbitList" );
     (void) H5PTread_packets( ptable, 0, (size_t) nrecords, orbitList );
/*
 * find all matches
 */
     for ( nr = 0; nr < (size_t) nrecords; nr++ ) {
          if ( orbitList[nr] == absOrbit ) {
	       metaIndx[*numIndx] = nr;
	       if ( ++(*numIndx) == dimArray ) break;
	  }
     }
     free( orbitList );
done:
     (void) H5PTclose( ptable );
}

/*+++++++++++++++++++++++++
.IDENTifer   SDMF_get_pt_jdayIndex
.PURPOSE     obtain indices to last-limb/Sun records for julianDay interval
.INPUT/OUTPUT
  call as    SDMF_get_pt_jdayIndex( locID, jdayRange, &numIndx, metaIndx );
     input:
           hid_t  locID        :  HDF5 identifier of group
	   double jdayRange[2] :  range in julianDay (min,max)
 in/output:
	   size_t *numIndx     :  [input]  dimension of metaIndx
                                  [output] number of indices found
    output:
	   size_t *metaIndx    :  array with requested indices

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
void SDMF_get_pt_jdayIndex( hid_t locID, const double jdayRange[],
			    size_t *numIndx, size_t *metaIndx )
{
     const size_t dimArray = *numIndx;

     register size_t nr;

     hid_t    ptable;
     hsize_t  nrecords;

     double   *jdayList;
/*
 * initialize return values
 */
     *numIndx = 0;
     *metaIndx = 0;
/*
 * check if jdayList exists
 */
     if ( (ptable = H5PTopen( locID, "jdayList" )) == H5I_BADID )
          NADC_RETURN_ERROR( NADC_ERR_HDF_DATA, "jdayList" );

     (void) H5PTget_num_packets( ptable, &nrecords );
     if ( nrecords == 0 ) {
          (void) H5PTclose( ptable );
          return;
     }
/*
 * read list of orbits stored sofar
 */
     jdayList = (double *) malloc( (size_t) nrecords * sizeof(double) );
     if ( jdayList == NULL )
          NADC_GOTO_ERROR( NADC_ERR_ALLOC, "jdayList" );
     (void) H5PTread_packets( ptable, 0, (size_t) nrecords, jdayList );
/*
 * find all matches
 */
     for ( nr = 0; nr < (size_t) nrecords; nr++ ) {
          if ( jdayList[nr] >= jdayRange[0] && jdayList[nr] <= jdayRange[1] ) {
	       metaIndx[*numIndx] = nr;
	       if ( ++(*numIndx) == dimArray ) break;
	  }
     }
     free( jdayList );
done:
     (void) H5PTclose( ptable );
}

/*+++++++++++++++++++++++++
.IDENTifer   SDMF_rd_pt_metaTable
.PURPOSE     read metaTable records from SDMF last-limb/Sun databases
.INPUT/OUTPUT
  call as    SDMF_rd_pt_metaTable( locID, &numIndx, metaIndx, &mtbl );
     input:
           hid_t  locID        :  HDF5 identifier of group
	   size_t *metaIndx    :  array with requested indices
 in/output:
	   size_t *numIndx     :  [input]  dimension of metaIndx (or zero)
                                  [output] number of records read
    output:
           struct mtbl_pt_rec **mtbl : last-limb/Sun meta-table records

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
void SDMF_rd_pt_metaTable( hid_t locID, size_t *numIndx, size_t *metaIndx,
			     struct mtbl_pt_rec **mtbl_out )
{
     register size_t nr;

     hid_t   ptable;
     hsize_t nrecords;

     struct mtbl_pt_rec *mtbl;
/*
 * initialize return values 
 */
     if ( ! Use_Extern_Alloc ) *mtbl_out = NULL;
/*
 * does the table already exists?
 */
     if ( (ptable = H5PTopen( locID, "metaTable" )) == H5I_BADID ) return;
/*
 * obtain table info
 */
     (void) H5PTget_num_packets( ptable, &nrecords );
     if ( *numIndx == 0 ) {
	  *numIndx = (size_t) nrecords;
	  for ( nr = 0; nr < nrecords; nr++ ) metaIndx[nr] = nr;
     }
     if ( nrecords == 0 ) {
	  (void) H5PTclose( ptable );
          return;
     }
/*
 * allocate memory to store the metaTable records
 */
     if ( ! Use_Extern_Alloc ) {
	  mtbl = (struct mtbl_pt_rec *) 
	       malloc( (size_t) nrecords * sizeof(struct mtbl_pt_rec));
	  if ( mtbl == NULL )
	       NADC_RETURN_ERROR( NADC_ERR_ALLOC, "mtbl" );
     } else if ( (mtbl = mtbl_out[0]) == NULL )
	  NADC_RETURN_ERROR( NADC_ERR_ALLOC, "mtbl_out[0]" );
/*
 * read table records
 */
     if ( (*numIndx) == (size_t) nrecords ) {
	  (void) H5PTread_packets( ptable, 0, (size_t) nrecords, mtbl );
     } else {
	  nr = 0;
	  do {
	       (void) H5PTread_packets( ptable, (hsize_t) metaIndx[nr], 1, 
					mtbl+nr );
	  } while ( ++nr < (*numIndx) );
     }
     *mtbl_out = mtbl;
}

/*+++++++++++++++++++++++++
.IDENTifer   SDMF_rd_pt_pointing
.PURPOSE     read pointing infor for cluster records from SDMF database
.INPUT/OUTPUT
  call as    SDMF_rd_pt_pointing( locID, &numIndx, metaIndx, pointing );
     input:
           hid_t  locID          :  HDF5 identifier of group
	   size_t *metaIndx      :  array with requested indices
 in/output:
	   size_t *numIndx       :  [input]  dimension of metaIndx (or zero)
                                    [output] number of records read
    output:
           struct geo_pt_rec *pointing : pointing of cluster records

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
void SDMF_rd_pt_pointing( hid_t locID, size_t *numIndx, size_t *metaIndx,
			    struct geo_pt_rec *pointing )
{
     register size_t  nr;

     unsigned short num_obs;

     hid_t   ptable;
     hsize_t nrecords;
/*
 * does the dataset already exists?
 */
     if ( (ptable = H5PTopen( locID, "pointing" )) == H5I_BADID ) return;
/*
 * obtain table info
 */
     (void) H5PTget_num_packets( ptable, &nrecords );
     if ( *numIndx == 0 ) {
	  *numIndx = (size_t) nrecords;
	  for ( nr = 0; nr < nrecords; nr++ ) metaIndx[nr] = nr;
     }
     if ( nrecords == 0 ) {
	  (void) H5PTclose( ptable );
          return;
     }
     (void) H5LTget_attribute_ushort( locID, "pointing", "numObs", &num_obs );
/*
 * read pointing records
 */
     nr = 0;
     do {
	  if ( nr > 0 ) pointing += num_obs;
	  (void) H5PTread_packets( ptable, (hsize_t) metaIndx[nr], 
				   1, pointing );
     } while ( ++nr < (*numIndx) );
}

/*+++++++++++++++++++++++++
.IDENTifer   SDMF_rd_pt_cluster
.PURPOSE     read cluster records from SDMF last-limb/Sun database
.INPUT/OUTPUT
  call as    SDMF_rd_pt_cluster( locID, clus_id, &numIndx, metaIndx, 
                                   pixel_val );
     input:
           hid_t  locID          :  HDF5 identifier of group
	   unsigned char clus_id :  Cluster ID, range 1 - 40
	   size_t *metaIndx      :  array with requested indices
 in/output:
	   size_t *numIndx       :  [input]  dimension of metaIndx (or zero)
                                    [output] number of records read
    output:
           float *pixel_val      : pixel values of SDMF last-limb/Sun database

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
void SDMF_rd_pt_cluster( hid_t locID, unsigned char clus_id, 
			   size_t *numIndx, size_t *metaIndx,
			   float *pixel_val )
{
     register size_t  nr, np;

     char           clusName[11];
     unsigned char  coaddf;
     unsigned short num_pixels, num_obs, *usbuff;
     unsigned int   *ubuff;

     hid_t   ptable;
     hsize_t nrecords;
     size_t  total_values;
/*
 * does the dataset already exists?
 */
     (void) snprintf( clusName, 11, "cluster_%02hhu", clus_id );
     if ( (ptable = H5PTopen( locID, clusName )) == H5I_BADID ) return;
/*
 * obtain table info
 */
     (void) H5PTget_num_packets( ptable, &nrecords );
     if ( *numIndx == 0 ) {
	  *numIndx = (size_t) nrecords;
	  for ( nr = 0; nr < nrecords; nr++ ) metaIndx[nr] = nr;
     }
     if ( nrecords == 0 ) {
	  (void) H5PTclose( ptable );
          return;
     }
     (void) H5LTget_attribute_uchar( locID, clusName, "coaddf", &coaddf );
     (void) H5LTget_attribute_ushort( locID, clusName, "numObs", &num_obs );
     (void) H5LTget_attribute_ushort( locID, clusName,"numPixels",&num_pixels );
     total_values = (size_t) num_obs * num_pixels;
/*
 * allocate temporary buffer to store cluster data
 */
     if ( coaddf == (unsigned char) 1 ) {
	  usbuff = (unsigned short *) malloc( total_values * sizeof(short) );
	  ubuff = NULL;
     } else {
	  usbuff = NULL;
	  ubuff = (unsigned int *) malloc( total_values * sizeof(int) );
     }
/*
 * read cluster records
 */
     nr = 0;
     do {
	  if ( coaddf == (unsigned char) 1 ) {
	       (void) H5PTread_packets( ptable, (hsize_t) metaIndx[nr], 
					1, usbuff );
	       for ( np = 0; np < total_values; np++ )
		    *pixel_val++ = (float) usbuff[np];
	  } else {
	       (void) H5PTread_packets( ptable, (hsize_t) metaIndx[nr], 
					1, ubuff );
	       for ( np = 0; np < total_values; np++ ) {
		    *pixel_val++ = (float) ubuff[np];
	       }
	  }
     } while ( ++nr < (*numIndx) );

     if ( usbuff != NULL ) free( usbuff );
     if ( ubuff != NULL  ) free( ubuff );
}
