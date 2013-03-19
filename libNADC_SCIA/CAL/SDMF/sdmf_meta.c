/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2007 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SDMF_META
.AUTHOR      R.M. van Hees
.KEYWORDS    SDMF - HDF5 metaTable
.LANGUAGE    ANSI C
.COMMENTS    contains SDMF_get_metaIndex, SDMF_get_metaIndex_range,
             SDMF_rd_metaTable
.ENVIRONment None
.VERSION     1.5     26-Sep-2011   replaced PyTable routines, RvH 
             1.4     20-Jan-2010   added force_replace flag to documentation
                                   feature was implemented by PvdM, RvH
             1.3     07-Jan-2010   bugs fixed in SDMF_get_metaIndex_range, RvH
             1.2     28-Jan-2008   silent break to avoid memory corruption, RvH
             1.1     11-Jan-2008   added SDMF_overwrite_metaTable, RS
             1.0     18-Dec-2007   initial release by R. M. van Hees
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
#include <math.h>
#include <time.h>

#include <hdf5.h>
#include <hdf5_hl.h>

/*+++++ Local Headers +++++*/
#include <nadc_sdmf.h>

/*+++++ Static Variables +++++*/
static const char  listName[] = "orbitList";
static const char  indexName[] = "orbitIndex";
static const char  tableName[] = "metaTable";

static const size_t mtbl_size = sizeof( struct mtbl_calib_rec );

static const size_t mtbl_offs[DIM_MTBL_CALIB] = {
     HOFFSET( struct mtbl_calib_rec, julianDay ),
     HOFFSET( struct mtbl_calib_rec, duration ),
     HOFFSET( struct mtbl_calib_rec, absOrbit ),
     HOFFSET( struct mtbl_calib_rec, entryDate ),
     HOFFSET( struct mtbl_calib_rec, procStage ),
     HOFFSET( struct mtbl_calib_rec, softVersion ),
     HOFFSET( struct mtbl_calib_rec, saaFlag ),
     HOFFSET( struct mtbl_calib_rec, rtsEnhFlag ),
     HOFFSET( struct mtbl_calib_rec, vorporFlag ),
     HOFFSET( struct mtbl_calib_rec, orbitPhase ),
     HOFFSET( struct mtbl_calib_rec, sunSemiDiam ),
     HOFFSET( struct mtbl_calib_rec, moonAreaSunlit ),
     HOFFSET( struct mtbl_calib_rec, longitude ),
     HOFFSET( struct mtbl_calib_rec, latitude ),
     HOFFSET( struct mtbl_calib_rec, asmAngle ),
     HOFFSET( struct mtbl_calib_rec, esmAngle ),
     HOFFSET( struct mtbl_calib_rec, obmTemp ),
     HOFFSET( struct mtbl_calib_rec, detectorTemp )
};

/*+++++++++++++++++++++++++ Static Function(s) +++++++++++++++*/

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SDMF_get_metaIndex
.PURPOSE     find index to entry of stateID and absOrbit
.INPUT/OUTPUT
  call as    row = SDMF_get_metaIndex( locID, absOrbit, &numIndx, metaIndx );
     input:
           hid_t fid             :  HDF5 identifier of file or group
	   int   absOrbit        :  orbit number
 in/output:
           int   *numIndx        :  input: dimension metaIndx
                                    output: number of indices found
    output:
           int   *metaIndx       :  array with indices to requested orbit

.RETURNS     Index to element of orbitIndx to be updated (required for write)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
int SDMF_get_metaIndex( hid_t locID, int absOrbit, 
			int *numIndx, int *metaIndx )
{
     const char prognm[] = "SDMF_get_metaIndex";
     
     register int nrr;

     int   nrows, rowIndex = 0;
     int   *orbitList = NULL;
     int   *orbitIndex = NULL;

     herr_t   stat;
     hsize_t  adim;

     const int dimArray = *numIndx;
/*
 * initialize return values
 */
     *numIndx = 0;            /* default: no matching index found */
     *metaIndx = 0;           /* default: append any new records */
/*
 * test if dataset "orbitList" exists
 */
     H5E_BEGIN_TRY {
          hid_t dataID = H5Dopen( locID, listName, H5P_DEFAULT );
	  if ( dataID < 0 ) return -1;
	  (void) H5Dclose( dataID );
     } H5E_END_TRY;
/*
 * read orbitList and orbitIndex
 */
     stat = H5LTget_dataset_info( locID, listName, &adim, NULL, NULL );
     if ( stat < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_SPACE, listName );
     nrows = (int) adim;

     if ( (orbitList = (int *) malloc( nrows * sizeof(int)))  == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "orbitList" );
     if ( H5LTread_dataset_int( locID, listName, orbitList ) < 0 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DATA, listName );

     if ( (orbitIndex = (int *) malloc( nrows * sizeof(int)))  == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "orbitIndex" );
     if ( H5LTread_dataset_int( locID, indexName, orbitIndex ) < 0 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DATA, indexName );
/*
 * quick check if new data is within stored orbit range
 */
     if ( absOrbit > orbitList[orbitIndex[nrows-1]] ) {
	  rowIndex = nrows;
	  goto done;
     }
     if ( absOrbit < orbitList[orbitIndex[0]] ) {
	  rowIndex = 0;
	  goto done;
     }
/*
 * search for matches
 */
     nrr = BinarySearch( nrows, orbitIndex, orbitList, absOrbit );
     if ( orbitList[orbitIndex[nrr]] == absOrbit ) {
	  register int num = 0;
	  do {
	       metaIndx[num++] = orbitIndex[nrr];
	       if ( num >= dimArray  ) break; 
	  } while ( ++nrr < nrows && orbitList[orbitIndex[nrr]] == absOrbit );
	  *numIndx = num;
     }
     rowIndex = nrr;
 done:
     if ( orbitList != NULL ) free( orbitList );
     if ( orbitIndex != NULL ) free( orbitIndex );
     return rowIndex;
}

/*+++++++++++++++++++++++++
.IDENTifer   SDMF_get_metaIndex_range
.PURPOSE     find index to entry of stateID and absOrbit
.INPUT/OUTPUT
  call as    row = SDMF_get_metaIndex_range( locID, orbit_range, numIndx, 
                                             metaIndx, use_neighbours );
     input:
       hid_t fid             :  HDF5 identifier of file or group
       int   *orbit_range    :  orbit range [lo, hi]
       int   use_neighbours  :  when no entries in range, use their neighbours
 in/output:
       int   *numIndx        :  input: dimension metaIndx (2nd pass only!)
                                output: number of indices found
    output:
       int   *metaIndx       :  array with indices to requested orbit
                                if NULL: no indices stored!

.RETURNS     Index to element of orbitIndx to be updated (required for write)
             error status passed by global variable ``nadc_stat''
.COMMENTS    May also be used as a two-pass function:
             first pass: get nr of indices found..
           row = SDMF_get_metaIndex_range( locID, orbit_range, &numIndx, NULL, 
                                           use_neighbours );
             second pass: get actual data..
           row = SDMF_get_metaIndex_range( locID, orbit_range, &numIndx, 
                                           metaIndx, use_neighbours );
-------------------------*/
int SDMF_get_metaIndex_range( hid_t locID, const int *orbit_range, 
			      int *numIndx, int *metaIndx, int use_neighbours )
{
     const char prognm[] = "SDMF_get_metaIndex_range";

     int   nrr_lo;
    
     int   nrows;
     int   rowIndex = -1;
     int   *orbitList = NULL;
     int   *orbitIndex = NULL;

     herr_t   stat;
     hsize_t  adim;

     int lo = orbit_range[0];
     int hi = orbit_range[1];
/*
 * two-pass call: metaIndx = NULL => return
 *            or: numIndx is dimension of metaIndx 
 */
     const int dimArray = (metaIndx != NULL) ? *numIndx : INT_MAX;
/*
 * initialize return values
 */
     *numIndx = 0;            /* default: no matching index found */
/*
 * test if dataset "orbitList" exists
 */
     H5E_BEGIN_TRY {
	  hid_t dataID = H5Dopen( locID, listName, H5P_DEFAULT );
	  if ( dataID < 0 ) return -1;
	  (void) H5Dclose( dataID );
     } H5E_END_TRY;
/*
 * read orbitList and orbitIndex
 */
     stat = H5LTget_dataset_info( locID, listName, &adim, NULL, NULL );
     if ( stat < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_SPACE, listName );
     nrows = (int) adim;

     if ( (orbitList = (int *) malloc( nrows * sizeof(int)))  == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "orbitList" );
     if ( H5LTread_dataset_int( locID, listName, orbitList ) < 0 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DATA, listName );

     if ( (orbitIndex = (int *) malloc( nrows * sizeof(int)))  == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "orbitIndex" );
     if ( H5LTread_dataset_int( locID, indexName, orbitIndex ) < 0 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DATA, indexName );
/*
 * quick check if new data is within stored orbit range, else clip or exit
 */
     /* clip if not in selected range */
     if ( hi > orbitList[orbitIndex[nrows-1]] ) {
	  rowIndex = nrows;
	  hi = orbitList[orbitIndex[nrows-1]];
	  if ( lo > orbitList[orbitIndex[nrows-1]] ) {
	       if ( use_neighbours )
		    lo = hi;
	       else
		    goto done;
	  }
     }
     if ( lo < orbitList[orbitIndex[0]] ) {
	  rowIndex = 0;
	  lo = orbitList[orbitIndex[0]];
	  if ( hi < orbitList[orbitIndex[0]] ) {
	       if ( use_neighbours )
		    hi = lo;
	       else
		    goto done;
	  }
     }
/*
 * search for matches
 */
     nrr_lo = BinarySearch( nrows, orbitIndex, orbitList, lo );
     /*
      * The index returned by the binary search always points to orbit
      *   number larger or equal (both for "lo" and "hi"). 
      * Only "lo" has to be adjusted in case it is not in orbitList
      * In case of "use_neighbours", we select the closest orbit, 
      *   otherwise an orbit larger than "lo" is selected
      * 
      */
     if ( use_neighbours && orbitList[orbitIndex[nrr_lo]] != lo ) {
	  int orbit_before = 
	       (nrr_lo > 0) ? orbitList[orbitIndex[nrr_lo-1]] : -1000000;
	  int orbit_after = orbitList[orbitIndex[nrr_lo]];

	  if ( (orbit_after - lo) <= (lo - orbit_before) ) {
	       lo = orbit_after;
	  } else {
	       lo = orbit_before;
	       while ( nrr_lo > 0 && orbitList[orbitIndex[nrr_lo-1]] == lo )
		       nrr_lo--;
	  }
     } else if ( orbitList[orbitIndex[nrr_lo]] > lo
		 && orbitList[orbitIndex[nrr_lo]] <= hi ) {
	  lo = orbitList[orbitIndex[nrr_lo]];
     }

     if ( orbitList[orbitIndex[nrr_lo]] == lo ) {
	  register int num = 0;

	  do {
	       if ( num >= dimArray ) break;
               if ( metaIndx != NULL)
	           metaIndx[num] = orbitIndex[nrr_lo];
	       num++;
	  } while ( ++nrr_lo < nrows && orbitList[orbitIndex[nrr_lo]] <= hi );
	  *numIndx = num;
     }
     rowIndex = nrr_lo;
done:
     if ( orbitList != NULL ) free( orbitList );
     if ( orbitIndex != NULL ) free( orbitIndex );
     return rowIndex;
}

/*+++++++++++++++++++++++++
.IDENTifer   SDMF_rd_metaTable
.PURPOSE     read metaTable records from SDMF calibration state database
.INPUT/OUTPUT
  call as    SDMF_rd_metaTable( locID, &numIndx, metaIndx, &mtbl );
     input:
           hid_t locID            :  HDF5 identifier of file or group
           int   *metaIndx        :  array with indices to requested records
 in/output:
           int   *numIndx         :  input: dimension metaIndx (or zero)
                                     output: number records read
    output:
           struct mtbl_calib_rec **mtbl :  State meta-data records to read

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    Note that *numIndx has to be zero and/or *metaIndx equal to NULL 
             to read all metaTable records
-------------------------*/
void SDMF_rd_metaTable( hid_t locID, int *numIndx, const int *metaIndx,
                          struct mtbl_calib_rec **mtbl_out )
{
     const char prognm[] = "SDMF_rd_metaTable";

     int     nrows;
     herr_t  stat;
     hsize_t nfields, nrecords;

     struct mtbl_calib_rec *mtbl;
/*
 * initialize return values
 */
     *mtbl_out = NULL;
/*
 * does the table already exists?
 */
     H5E_BEGIN_TRY {
          hid_t dataID = H5Dopen( locID, tableName, H5P_DEFAULT );
          if ( dataID < 0 ) return;
          (void) H5Dclose( dataID );
     } H5E_END_TRY;
/*
 * obtain table info
 */
     stat = H5TBget_table_info(locID, tableName, &nfields, &nrecords );
     if ( stat < 0 ) NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_SPACE, tableName );
     nrows = (int) nrecords;
     if ( *numIndx == 0 || metaIndx == NULL ) *numIndx = nrows;
     if ( nrows == 0 ) return;
/*
 * allocate memory to store metaTable records
 */
     mtbl = (struct mtbl_calib_rec *) 
	  malloc( (size_t) (*numIndx) * mtbl_size );
     if ( mtbl == NULL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "mtbl" );
/*
 * read table
 */
     if ( (*numIndx) == nrows ) {
          stat = H5TBread_table( locID, tableName, mtbl_size, mtbl_offs, 
                                 mtbl_calib_sizes, mtbl );
          if ( stat < 0 ) {
               free( mtbl );
               NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_RD, tableName );
          }
     } else {
          register int nm;

          for ( nm = 0; nm < (*numIndx); nm++ ) {
               hsize_t start = metaIndx[nm];
               stat = H5TBread_records( locID, tableName, start, 1,
                                        mtbl_size, mtbl_offs, 
                                        mtbl_calib_sizes, mtbl+nm );
               if ( stat < 0 ) {
                    free( mtbl );
                    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_RD, tableName );
               } 
          }
     }
     *mtbl_out = mtbl;
}
