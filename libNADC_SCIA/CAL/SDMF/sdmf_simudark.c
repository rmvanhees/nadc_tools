/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2008 - 2013 SRON (P.van.der.Meer@sron.nl)

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

.IDENTifer   SDMF_SIMUDARK
.AUTHOR      P. van der Meer
.KEYWORDS    SDMF - SCIA simudark
.LANGUAGE    ANSI C
.COMMENTS    contains SDMF_rd_simudarkTable
.ENVIRONment None
.VERSION     1.0     17-Jul-2008   initial release by P. van der Meer
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

static const size_t mtbl_size = sizeof( struct mtbl_simudark_rec );

static const size_t mtbl_offs[DIM_MTBL_SIMUDARK] = {
     HOFFSET( struct mtbl_simudark_rec, julianDate ),
     HOFFSET( struct mtbl_simudark_rec, absOrbit ),
     HOFFSET( struct mtbl_simudark_rec, entryDate ),
     HOFFSET( struct mtbl_simudark_rec, saaFlag ),
     HOFFSET( struct mtbl_simudark_rec, obmTemp ),
     HOFFSET( struct mtbl_simudark_rec, detTemp ),
     HOFFSET( struct mtbl_simudark_rec, quality ),
     HOFFSET( struct mtbl_simudark_rec, orbitPhase ),
     HOFFSET( struct mtbl_simudark_rec, sig_phase ),
     HOFFSET( struct mtbl_simudark_rec, phase2 ),
     HOFFSET( struct mtbl_simudark_rec, sig_phase2 ),
     HOFFSET( struct mtbl_simudark_rec, amp2 ),
     HOFFSET( struct mtbl_simudark_rec, sig_amp2 )
};

/*+++++++++++++++++++++++++
.IDENTifer   SDMF_rd_simudarkTable
.PURPOSE     read metaTable records from SDMF simultaneous dark signal parameter database
.INPUT/OUTPUT
  call as    SDMF_rd_simudarkTable( locID, &numIndx, metaIndx, &mtbl );
     input:
           hid_t locID            :  HDF5 identifier of file or group
           int   *metaIndx        :  array with indices to requested records
 in/output:
           int   *numIndx         :  input: dimension metaIndx (or zero)
                                     output: number records read
    output:
	   struct mtbl_simudark_rec **mtbl :  State meta-data records to read

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SDMF_rd_simudarkTable( hid_t locID, int *numIndx, int *metaIndx,
			  struct mtbl_simudark_rec **mtbl_out )
{
     const char prognm[] = "SDMF_rd_simudarkTable";

     hsize_t nfields, nrecords;
     herr_t  stat;

     struct mtbl_simudark_rec *mtbl;
/*
 * initialize return values
 */
     *mtbl_out = NULL;
/*
 * does the table already exist?
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
     if ( *numIndx == 0 ) *numIndx = (int) nrecords;
     if ( stat < 0 || nrecords == 0 ) return;

/*
 * allocate memory to store metaTable records
 */
     mtbl = (struct mtbl_simudark_rec *) 
	  malloc( (size_t) (*numIndx) * mtbl_size );
     if ( mtbl == NULL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "mtbl" );
/*
 * read table
 */
     if ( (*numIndx) == (int) nrecords ) {
	  stat = H5TBread_table( locID, tableName, mtbl_size, mtbl_offs, 
				 mtbl_simudark_sizes, mtbl );
	  if ( stat < 0 ) {
	       free( mtbl );
	       NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_RD, tableName );
	  } 
     } else {
	  register int nm;

	  for ( nm = 0; nm < (*numIndx); nm++ ) {
	       stat = H5TBread_records( locID, tableName, metaIndx[nm], 1,
					mtbl_size, mtbl_offs, 
					mtbl_simudark_sizes, mtbl+nm );
	       if ( stat < 0 ) {
		    free( mtbl );
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_RD, tableName );
	       } 
	  }
     }

     *mtbl_out = mtbl;
}
