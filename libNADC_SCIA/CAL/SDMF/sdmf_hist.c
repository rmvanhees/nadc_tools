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

.IDENTifer   SDMF_HIST
.AUTHOR      R.M. van Hees
.KEYWORDS    SDMF - HDF5 histogram
.LANGUAGE    ANSI C
.PURPOSE     read histogram from readouts of one state execution
.COMMENTS    contains: SDMF30_rd_histTable, SDMF31_rd_histTable
.ENVIRONment None
.VERSION     2.1     22-Sep-2011   fixed bug in calculation histogram, RvH
             2.0     30-Sep-2010   modified structure sdmf_hist_offs, RvH
             1.1     16-Jan-2008   added SDMF_read_histTable, RvH
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
#include <limits.h>

#include <hdf5.h>
#include <hdf5_hl.h>

/*+++++ Local Headers +++++*/
#include <nadc_sdmf.h>

/*+++++ Static Variables +++++*/
static const char tableName[] = "histReadOut";

/*+++++ Global Variables +++++*/
      /* NONE */

/*+++++++++++++++++++++++++ Static Function(s) +++++++++++++++*/
      /* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SDMF30_rd_histTable
.PURPOSE     read histogram data
.INPUT/OUTPUT
  call as    SDMF30_rd_histTable( locID, numIndex, metaIndex, pixelRange,
                                  sdmf_hist );
     input:
           hid_t locID           :  HDF5 identifier of file or group
	   int   numIndex        :  number of indices to read
	   int   *metaIndex      :  array with indices of row to read
	   int   *pixelRange     :  two element array containing the pixelsIDs
                                    of the first and last pixel to be read
				    OR NULL, in case all pixels are read
    output:
	   struct sdmf_hist1_rec *sdmf_hist : histogram-data records

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    allocation requirements for sdmf_hist is
             (pixelRange != NULL)
	           sz_sdmf_hist * indexNum * (pixelRange[1]-pixelRange[0]+1)
             (pixelRange == NULL)
	           sz_sdmf_hist * indexNum * SCIENCE_PIXELS
-------------------------*/
void SDMF30_rd_histTable( hid_t locID, int indexNum, const int *metaIndex,
			  const int *pixelRange, 
			  struct sdmf_hist1_rec *sdmf_hist )
{
     const char prognm[] = "SDMF30_rd_histTable";

     register int nr;

     const hsize_t pixelStart = (pixelRange == NULL) ?  0 : pixelRange[0];
     const hsize_t pixelCount = 
	  (pixelRange == NULL) ? SCIENCE_PIXELS : (pixelRange[1] - pixelRange[0] + 1);

     hid_t  dataID, typeID = -1, spaceID = -1, memSpaceID = -1;
     herr_t stat;

     const int     rank = 2;

     const hsize_t count[] = {1, pixelCount};
/*
 * number of indices to read should be larger than zero
 */
     if ( indexNum <= 0 ) return;
/*
 * does the table already exists?
 */
     H5E_BEGIN_TRY {
          dataID = H5Dopen( locID, tableName, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( dataID < 0 )
          NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_RD, tableName );

     /* Get the data type ID */
     if ( (typeID = H5Dget_type( dataID )) < 0 )
          NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, tableName );

     /* Get the dataspace handle */
     if ( (spaceID = H5Dget_space( dataID )) < 0 )
          NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_SPACE, tableName );

     /* Create a memory dataspace handle */
     if ( (memSpaceID = H5Screate_simple( rank, count, NULL )) < 0 )
          NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_SPACE, tableName );

     for ( nr = 0; nr < indexNum; nr++ ) {
          hsize_t start[] = {metaIndex[nr], pixelStart};

          stat = H5Sselect_hyperslab( spaceID, H5S_SELECT_SET, 
				      start, NULL, count, NULL );
          if ( stat < 0 ) NADC_GOTO_ERROR(prognm, NADC_ERR_HDF_DATA, tableName);

          stat = H5Dread( dataID, typeID, memSpaceID, spaceID,
                          H5P_DEFAULT, sdmf_hist+(nr * pixelCount) );
          if ( stat < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_RD, tableName );
     }
done:
     if ( memSpaceID >= 0 ) (void) H5Sclose( memSpaceID );
     if ( spaceID >= 0 ) (void) H5Sclose( spaceID );
     if ( typeID >= 0 ) (void) H5Tclose( typeID );
     (void) H5Dclose( dataID );
}

/*+++++++++++++++++++++++++
.IDENTifer   SDMF30_rd_histTable
.PURPOSE     read histogram data
.INPUT/OUTPUT
  call as    SDMF31_rd_histTable( locID, numIndex, metaIndex, pixelRange,
                                  sdmf_hist );
     input:
           hid_t locID           :  HDF5 identifier of file or group
	   int   numIndex        :  number of indices to read
	   int   *metaIndex      :  array with indices of row to read
	   int   *pixelRange     :  two element array containing the pixelsIDs
                                    of the first and last pixel to be read
				    OR NULL, in case all pixels are read
    output:
	   struct sdmf_hist2_rec *sdmf_hist : histogram-data records

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    allocation requirements for sdmf_hist is
             (pixelRange != NULL)
	           sz_sdmf_hist * indexNum * (pixelRange[1]-pixelRange[0]+1)
             (pixelRange == NULL)
	           sz_sdmf_hist * indexNum * SCIENCE_PIXELS
-------------------------*/
void SDMF31_rd_histTable( hid_t locID, int indexNum, const int *metaIndex,
			  const int *pixelRange, 
			  struct sdmf_hist2_rec *sdmf_hist )
{
     const char prognm[] = "SDMF31_rd_histTable";

     register int nr;

     const hsize_t pixelStart = (pixelRange == NULL) ?  0 : pixelRange[0];
     const hsize_t pixelCount = 
	  (pixelRange == NULL) ? SCIENCE_PIXELS : (pixelRange[1] - pixelRange[0] + 1);

     hid_t  dataID, typeID = -1, spaceID = -1, memSpaceID = -1;
     herr_t stat;

     const int     rank = 2;

     const hsize_t count[] = {1, pixelCount};
/*
 * number of indices to read should be larger than zero
 */
     if ( indexNum <= 0 ) return;
/*
 * does the table already exists?
 */
     H5E_BEGIN_TRY {
          dataID = H5Dopen( locID, tableName, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( dataID < 0 )
          NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_RD, tableName );

     /* Get the data type ID */
     if ( (typeID = H5Dget_type( dataID )) < 0 )
          NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, tableName );

     /* Get the dataspace handle */
     if ( (spaceID = H5Dget_space( dataID )) < 0 )
          NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_SPACE, tableName );

     /* Create a memory dataspace handle */
     if ( (memSpaceID = H5Screate_simple( rank, count, NULL )) < 0 )
          NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_SPACE, tableName );

     for ( nr = 0; nr < indexNum; nr++ ) {
          hsize_t start[] = {metaIndex[nr], pixelStart};

          stat = H5Sselect_hyperslab( spaceID, H5S_SELECT_SET, 
				      start, NULL, count, NULL );
          if ( stat < 0 ) NADC_GOTO_ERROR(prognm, NADC_ERR_HDF_DATA, tableName);

          stat = H5Dread( dataID, typeID, memSpaceID, spaceID,
                          H5P_DEFAULT, sdmf_hist+(nr * pixelCount) );
          if ( stat < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_RD, tableName );
     }
done:
     if ( memSpaceID >= 0 ) (void) H5Sclose( memSpaceID );
     if ( spaceID >= 0 ) (void) H5Sclose( spaceID );
     if ( typeID >= 0 ) (void) H5Tclose( typeID );
     (void) H5Dclose( dataID );
}
