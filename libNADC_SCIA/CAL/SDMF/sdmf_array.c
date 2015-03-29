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

.IDENTifer   SDMF_ARRAY
.AUTHOR      R.M. van Hees
.KEYWORDS    SDMF - HDF5 dataset
.LANGUAGE    ANSI C
.PURPOSE     read/write statistics of State executions
.COMMENTS    contains SDMF_rd_uint8_Array, SDMF_rd_uint8_Matrix, 
		      SDMF_rd_int16_Array, SDMF_rd_int16_Matrix,
		      SDMF_rd_uint16_Array, SDMF_rd_uint16_Matrix,
		      SDMF_rd_float_Array, SDMF_rd_float_Matrix
.ENVIRONment None
.VERSION     2.0     15-Sep-2010   update documentation 
                                   added several read functions, RvH
             1.0     05-Jan-2008   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <hdf5.h>
#include <hdf5_hl.h>

/*+++++ Local Headers +++++*/
#include <nadc_sdmf.h>

/*+++++ Static Variables +++++*/

/*+++++++++++++++++++++++++ Static Function(s) +++++++++++++++*/

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SDMF_rd_string_Array
.PURPOSE     read element from dataset ''arrName''
.INPUT/OUTPUT
  call as    SDMF_rd_string_Array( locID, arrName, Index, data );
     input:
            hid_t locID           :  HDF5 identifier of file or group
            char  arrName[]       :  name of dataset
            int   Index           :  number of index to read
    output:
           unsigned char *data    :  string space for data to read

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SDMF_rd_string_Array( const hid_t locID, const char *arrName, 
                             const int index, char *data )
{
     hid_t  dataID, spaceID = -1, memSpaceID = -1;
     hid_t strtype; /* HDF5 data type id. */
     herr_t stat;

     hsize_t count = 1;
     hsize_t stride = 1;
     hsize_t blocks = 1;
     hsize_t start  = (hsize_t) index;
     const int rank = 1;
/*
 * does the table already exists?
 */
     H5E_BEGIN_TRY {
	  dataID = H5Dopen( locID, arrName, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( dataID < 0 )
	  NADC_RETURN_ERROR( NADC_ERR_HDF_RD, arrName );

     /* Get the dataspace handle */
     if ( (spaceID = H5Dget_space( dataID )) < 0 )
	  NADC_GOTO_ERROR( NADC_ERR_HDF_SPACE, arrName );

    /* Create a memory dataspace handle */
     if ( (memSpaceID = H5Screate_simple( rank, &count, NULL )) < 0 )
	  NADC_GOTO_ERROR( NADC_ERR_HDF_SPACE, arrName );

     stat = H5Sselect_hyperslab( spaceID, H5S_SELECT_SET, &start, &stride,
				 &count, &blocks );
     if ( stat < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, arrName );

     /* set datatype string */
     if( (strtype=H5Tcopy(H5T_C_S1)) < 0 )
	  NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, arrName);
     /* set the length */
     if( (H5Tset_size(strtype, 20) ) < 0 )
	  NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, arrName);

     stat = H5Dread( dataID, strtype, memSpaceID, spaceID,
		     H5P_DEFAULT, data );
     if ( stat < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_RD, arrName );

done:
     if ( memSpaceID >= 0 ) (void) H5Sclose( memSpaceID );
     if ( spaceID >= 0 ) (void) H5Sclose( spaceID );
     (void) H5Dclose( dataID );
}

/*+++++++++++++++++++++++++
.IDENTifer   SDMF_rd_uint8_Array
.PURPOSE     read row(s) from dataset ''arrName''
.INPUT/OUTPUT
  call as    SDMF_rd_uint8_Array( locID, arrName, numIndex, metaIndex, 
                                    pixelRange, data );
     input:
           hid_t locID           :  HDF5 identifier of file or group
	   char  arrName[]       :  name of dataset
           int   numIndex        :  number of indices to read
           int   *metaIndex      :  array with indices of row to read
           int   *pixelRange     :  two element array containing the pixelsIDs
                                    of the first and last pixel to be read
                                    OR NULL, in case all pixels are read
    output:
           unsigned char *data   :  array for data to read

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SDMF_rd_uint8_Array( hid_t locID, const char *arrName, int indexNum, 
			     const int *metaIndex, const int *pixelRange, 
			     unsigned char *data )
{
     register int nr;

     hid_t  dataID, spaceID = -1, memSpaceID = -1;
     herr_t stat;

     const int pixelStart = (pixelRange == NULL) ? 0 : pixelRange[0];
     const int pixelNum   = (pixelRange == NULL) ? \
          SCIENCE_PIXELS : (pixelRange[1] - pixelRange[0] + 1);
     const int     rank = 2;
     const hsize_t count[] = {pixelNum, 1};
     const hsize_t *stride = NULL;
     const hsize_t *blocks = NULL;
/*
 * number of indices to read should be larger than zero
 */
     if ( indexNum <= 0 ) return;
/*
 * does the table already exists?
 */
     H5E_BEGIN_TRY {
          dataID = H5Dopen( locID, arrName, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( dataID < 0 )
          NADC_RETURN_ERROR( NADC_ERR_HDF_RD, arrName );

     /* Get the dataspace handle */
     if ( (spaceID = H5Dget_space( dataID )) < 0 )
	  NADC_GOTO_ERROR( NADC_ERR_HDF_SPACE, arrName );

     /* Create a memory dataspace handle */
     if ( (memSpaceID = H5Screate_simple( rank, count, NULL )) < 0 )
	  NADC_GOTO_ERROR( NADC_ERR_HDF_SPACE, arrName );

     for ( nr = 0; nr < indexNum; nr++ ) {
	  hsize_t start[] = { pixelStart, metaIndex[nr] };

	  stat = H5Sselect_hyperslab( spaceID, H5S_SELECT_SET, start, stride,
				      count, blocks );
	  if ( stat < 0 ) NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, arrName);

	  stat = H5Dread( dataID, H5T_NATIVE_UINT8, memSpaceID, spaceID, 
			  H5P_DEFAULT, data + nr * pixelNum );
	  if ( stat < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_RD, arrName );
     }
done:
     if ( memSpaceID >= 0 ) (void) H5Sclose( memSpaceID );
     if ( spaceID >= 0 ) (void) H5Sclose( spaceID );
     (void) H5Dclose( dataID );
}

/*+++++++++++++++++++++++++
.IDENTifer   SDMF_rd_uint8_Matrix
.PURPOSE     read row(s) from dataset ''arrName''
.INPUT/OUTPUT
  call as    SDMF_rd_uint8_Matrix( locID, arrName, Index, slabsize, 
                                     dims, data );
     input:
            hid_t locID           :  HDF5 identifier of file or group
            char  arrName[]       :  name of dataset
            int   Index           :  number of index to read
            int   slabsize        :  array of elements to read in all dimensions
            int   dims            :  number of dimensions of dataset
    output:
           unsigned char *data    :  matrix for data to read

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SDMF_rd_uint8_Matrix( const hid_t locID, const char *arrName, 
			     const int index, const int *slabsize, 
			     const int dims, unsigned char *data )
{
     register int nr;

     hid_t  dataID, spaceID = -1, memSpaceID = -1;
     herr_t stat;

     hsize_t count[dims];
     hsize_t stride[dims];
     hsize_t blocks[dims];
     hsize_t start[dims];
     const int rank = dims;
/*
 * number of indices to read should be larger than zero
 */
     for ( nr = 0; nr < dims; nr++ ) {
	  count[nr] = (hsize_t) slabsize[nr];
	  start[nr] = (hsize_t) 0;
	  stride[nr]= (hsize_t) 1;
	  blocks[nr]= (hsize_t) 1;
     }
     start[dims-1]= (hsize_t) index;  //is "orbit" dim
/*
 * does the table already exists?
 */
     //printf("Opening group %s\n",arrName);
     H5E_BEGIN_TRY {
	  dataID = H5Dopen( locID, arrName, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( dataID < 0 )
	  NADC_RETURN_ERROR( NADC_ERR_HDF_RD, arrName );

     /* Get the dataspace handle */
     if ( (spaceID = H5Dget_space( dataID )) < 0 )
	  NADC_GOTO_ERROR( NADC_ERR_HDF_SPACE, arrName );

     /* Create a memory dataspace handle */
     if ( (memSpaceID = H5Screate_simple( rank, count, NULL )) < 0 )
	  NADC_GOTO_ERROR( NADC_ERR_HDF_SPACE, arrName );

     stat = H5Sselect_hyperslab( spaceID, H5S_SELECT_SET, start, stride,
				 count, blocks );
     if ( stat < 0 ) NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, arrName);

     stat = H5Dread( dataID, H5T_NATIVE_UINT8, memSpaceID, spaceID,
		     H5P_DEFAULT, data );
     if ( stat < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_RD, arrName );

done:
     if ( memSpaceID >= 0 ) (void) H5Sclose( memSpaceID );
     if ( spaceID >= 0 ) (void) H5Sclose( spaceID );
     (void) H5Dclose( dataID );
}

/*+++++++++++++++++++++++++
.IDENTifer   SDMF_rd_int16_Array
.PURPOSE     read row(s) from dataset ''arrName''
.INPUT/OUTPUT
  call as    SDMF_rd_int16_Array( locID, arrName, numIndex, metaIndex, 
                                     pixelRange, data );
     input:
           hid_t locID           :  HDF5 identifier of file or group
	   char  arrName[]       :  name of dataset
           int   numIndex        :  number of indices to read
           int   *metaIndex      :  array with indices of row to read
           int   *pixelRange     :  two element array containing the pixelsIDs
                                    of the first and last pixel to be read
                                    OR NULL, in case all pixels are read
    output:
           short *data           :  array for data to read

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SDMF_rd_int16_Array( hid_t locID, const char *arrName, int indexNum, 
			     const int *metaIndex, const int *pixelRange, 
			     short *data )
{
     register int nr;

     hid_t  dataID, spaceID = -1, memSpaceID = -1;
     herr_t stat;

     const int pixelStart = (pixelRange == NULL) ? 0 : pixelRange[0];
     const int pixelNum   = (pixelRange == NULL) ? \
          SCIENCE_PIXELS : (pixelRange[1] - pixelRange[0] + 1);
     const int     rank = 2;
     const hsize_t count[] = { pixelNum, 1 };
     const hsize_t *stride = NULL;
     const hsize_t *blocks = NULL;
/*
 * number of indices to read should be larger than zero
 */
     if ( indexNum <= 0 ) return;
/*
 * does the table already exists?
 */
     H5E_BEGIN_TRY {
          dataID = H5Dopen( locID, arrName, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( dataID < 0 )
          NADC_RETURN_ERROR( NADC_ERR_HDF_RD, arrName );

     /* Get the dataspace handle */
     if ( (spaceID = H5Dget_space( dataID )) < 0 )
	  NADC_GOTO_ERROR( NADC_ERR_HDF_SPACE, arrName );

     /* Create a memory dataspace handle */
     if ( (memSpaceID = H5Screate_simple( rank, count, NULL )) < 0 )
	  NADC_GOTO_ERROR( NADC_ERR_HDF_SPACE, arrName );

     for ( nr = 0; nr < indexNum; nr++ ) {
	  hsize_t start[] = { pixelStart, metaIndex[nr] };

	  stat = H5Sselect_hyperslab( spaceID, H5S_SELECT_SET, start, stride,
				      count, blocks );
	  if ( stat < 0 ) NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, arrName);

	  stat = H5Dread( dataID, H5T_NATIVE_INT16, memSpaceID, spaceID, 
			  H5P_DEFAULT, data + nr * pixelNum );
	  if ( stat < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_RD, arrName );
     }
done:
     if ( memSpaceID >= 0 ) (void) H5Sclose( memSpaceID );
     if ( spaceID >= 0 ) (void) H5Sclose( spaceID );
     (void) H5Dclose( dataID );
}

/*+++++++++++++++++++++++++
.IDENTifer   SDMF_rd_int16_Matrix
.PURPOSE     read slabs from dataset ''arrName''
.INPUT/OUTPUT
  call as    SDMF_rd_int16_Matrix( locID, arrName, metaIndex, 
                                     slabsize, dims, data );
     input:
           hid_t locID           :  HDF5 identifier of file or group
           char  arrName[]       :  name of dataset
           int   *metaIndex      :  index of orbit
           int   *slabsize       :  size of slab to read
           int   dims            : dimensions of dataset, only tested for 3D
           
    output:
           short *data           :  array for data to read

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SDMF_rd_int16_Matrix( hid_t locID, const char *arrName, 
			     const int *metaIndex, const int *slabsize, 
			     const int dims, short *data )
{
     register int nr;

     hid_t  dataID, spaceID = -1, memSpaceID = -1;
     herr_t stat;

     const int rank = dims;
     hsize_t count[dims];
     hsize_t start[dims];
     const hsize_t stride[] = {1,1,1};
     const hsize_t blocks[] = {1,1,1};
  
     for ( nr = 0; nr < dims; nr++ ) {
	  count[nr] = (hsize_t) slabsize[nr];
	  start[nr] = (hsize_t) 0;
     }
     //start[0]= (hsize_t) 0;//is "third" dim
     //start[1]= (hsize_t) 0;//is "pixel" dim
     start[dims-1]= (hsize_t) *metaIndex;//is "orbit" dim
/*
 * does the table already exists?
 */
     H5E_BEGIN_TRY {
	  dataID = H5Dopen( locID, arrName, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( dataID < 0 )
	  NADC_RETURN_ERROR( NADC_ERR_HDF_RD, arrName );

     /* Get the dataspace handle */
     if ( (spaceID = H5Dget_space( dataID )) < 0 )
	  NADC_GOTO_ERROR( NADC_ERR_HDF_SPACE, arrName );

     /* Create a memory dataspace handle */
     if ( (memSpaceID = H5Screate_simple( rank, count, NULL )) < 0 )
	  NADC_GOTO_ERROR( NADC_ERR_HDF_SPACE, arrName );

     stat = H5Sselect_hyperslab( spaceID, H5S_SELECT_SET, start, stride,
				 count, blocks );
     if ( stat < 0 ) NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, arrName );

     stat = H5Dread( dataID, H5T_NATIVE_INT16, memSpaceID, spaceID, 
		     H5P_DEFAULT, data );
     if ( stat < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_RD, arrName );
done:
     if ( memSpaceID >= 0 ) (void) H5Sclose( memSpaceID );
     if ( spaceID >= 0 ) (void) H5Sclose( spaceID );
     (void) H5Dclose( dataID );
}

/*+++++++++++++++++++++++++
.IDENTifer   SDMF_rd_uint16_Array
.PURPOSE     read row(s) from dataset ''arrName''
.INPUT/OUTPUT
  call as    SDMF_rd_uint16_Array( locID, arrName, numIndex, metaIndex, 
                                     pixelRange, data );
     input:
           hid_t locID           :  HDF5 identifier of file or group
	   char  arrName[]       :  name of dataset
           int   numIndex        :  number of indices to read
           int   *metaIndex      :  array with indices of row to read
           int   *pixelRange     :  two element array containing the pixelsIDs
                                    of the first and last pixel to be read
                                    OR NULL, in case all pixels are read
    output:
           unsigned short *data  :  array for data to read

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SDMF_rd_uint16_Array( hid_t locID, const char *arrName, int indexNum, 
			     const int *metaIndex, const int *pixelRange, 
			     unsigned short *data )
{
     register int nr;

     hid_t  dataID, spaceID = -1, memSpaceID = -1;
     herr_t stat;

     const int pixelStart = (pixelRange == NULL) ? 0 : pixelRange[0];
     const int pixelNum   = (pixelRange == NULL) ? \
          SCIENCE_PIXELS : (pixelRange[1] - pixelRange[0] + 1);
     const int     rank = 2;
     const hsize_t count[] = { pixelNum, 1 };
     const hsize_t *stride = NULL;
     const hsize_t *blocks = NULL;
/*
 * number of indices to read should be larger than zero
 */
     if ( indexNum <= 0 ) return;
/*
 * does the table already exists?
 */
     H5E_BEGIN_TRY {
          dataID = H5Dopen( locID, arrName, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( dataID < 0 )
          NADC_RETURN_ERROR( NADC_ERR_HDF_RD, arrName );

     /* Get the dataspace handle */
     if ( (spaceID = H5Dget_space( dataID )) < 0 )
	  NADC_GOTO_ERROR( NADC_ERR_HDF_SPACE, arrName );

     /* Create a memory dataspace handle */
     if ( (memSpaceID = H5Screate_simple( rank, count, NULL )) < 0 )
	  NADC_GOTO_ERROR( NADC_ERR_HDF_SPACE, arrName );

     for ( nr = 0; nr < indexNum; nr++ ) {
	  hsize_t start[] = { pixelStart, metaIndex[nr] };

	  stat = H5Sselect_hyperslab( spaceID, H5S_SELECT_SET, start, stride,
				      count, blocks );
	  if ( stat < 0 ) NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, arrName);

	  stat = H5Dread( dataID, H5T_NATIVE_UINT16, memSpaceID, spaceID, 
			  H5P_DEFAULT, data + nr * pixelNum );
	  if ( stat < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_RD, arrName );
     }
done:
     if ( memSpaceID >= 0 ) (void) H5Sclose( memSpaceID );
     if ( spaceID >= 0 ) (void) H5Sclose( spaceID );
     (void) H5Dclose( dataID );
}

/*+++++++++++++++++++++++++
.IDENTifer   SDMF_rd_uint16_Matrix
.PURPOSE     read slabs from dataset ''arrName''
.INPUT/OUTPUT
  call as    SDMF_rd_uint16_Matrix( locID, arrName, metaIndex, 
                                      slabsize, dims, data );
     input:
           hid_t locID           :  HDF5 identifier of file or group
           char  arrName[]       :  name of dataset
           int   *metaIndex      :  index of orbit
           int   *slabsize       :  size of slab to read
           int   dims            : dimensions of dataset, only tested for 3D
           
    output:
           uint16 *data          :  array for data to read

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SDMF_rd_uint16_Matrix( hid_t locID, const char *arrName, 
			      const int *metaIndex, const int *slabsize, 
			      const int dims, unsigned short *data )
{
     register int nr;

     hid_t  dataID, spaceID = -1, memSpaceID = -1;
     herr_t stat;

     const int rank = dims;
     hsize_t count[dims];
     hsize_t start[dims];
     const hsize_t stride[] = {1,1,1};
     const hsize_t blocks[] = {1,1,1};
  
     for ( nr = 0; nr < dims; nr++ ) {
	  count[nr] = (hsize_t) slabsize[nr];
	  start[nr] = (hsize_t) 0;
     }
     //start[0]= (hsize_t) 0;//is "third" dim
     //start[1]= (hsize_t) 0;//is "pixel" dim
     start[dims-1]= (hsize_t) *metaIndex;//is "orbit" dim
/*
 * does the table already exists?
 */
     H5E_BEGIN_TRY {
	  dataID = H5Dopen( locID, arrName, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( dataID < 0 )
	  NADC_RETURN_ERROR( NADC_ERR_HDF_RD, arrName );

     /* Get the dataspace handle */
     if ( (spaceID = H5Dget_space( dataID )) < 0 )
	  NADC_GOTO_ERROR( NADC_ERR_HDF_SPACE, arrName );

     /* Create a memory dataspace handle */
     if ( (memSpaceID = H5Screate_simple( rank, count, NULL )) < 0 )
	  NADC_GOTO_ERROR( NADC_ERR_HDF_SPACE, arrName );

     stat = H5Sselect_hyperslab( spaceID, H5S_SELECT_SET, start, stride,
				 count, blocks );
     if ( stat < 0 ) NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, arrName );

     stat = H5Dread( dataID, H5T_NATIVE_UINT16, memSpaceID, spaceID, 
		     H5P_DEFAULT, data );
     if ( stat < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_RD, arrName );
done:
     if ( memSpaceID >= 0 ) (void) H5Sclose( memSpaceID );
     if ( spaceID >= 0 ) (void) H5Sclose( spaceID );
     (void) H5Dclose( dataID );
}

/*+++++++++++++++++++++++++
.IDENTifer   SDMF_rd_float_Array
.PURPOSE     read row(s) from dataset ''arrName''
.INPUT/OUTPUT
  call as    SDMF_rd_float_Array( locID, arrName, numIndex, metaIndex, 
                                    pixelRange, data );
     input:
           hid_t locID           :  HDF5 identifier of file or group
	   char  arrName[]       :  name of dataset
           int   numIndex        :  number of indices to read
           int   *metaIndex      :  array with indices of row to read
           int   *pixelRange     :  two element array containing the pixelsIDs
                                    of the first and last pixel to be read
                                    OR NULL, in case all pixels are read
    output:
           float *data           :  array for data to read

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SDMF_rd_float_Array( hid_t locID, const char *arrName, int indexNum, 
			    const int *metaIndex, const int *pixelRange, 
			    float *data )
{
     register int nr;

     hid_t   dataID, spaceID = -1, memSpaceID = -1;
     herr_t  stat;

     const int pixelStart = (pixelRange == NULL) ? 0 : pixelRange[0];
     const int pixelNum   = (pixelRange == NULL) ? \
          SCIENCE_PIXELS : (pixelRange[1] - pixelRange[0] + 1);
     const int     rank = 2;
     const hsize_t count[] = { pixelNum, 1 };
     const hsize_t *stride = NULL;
     const hsize_t *blocks = NULL;
/*
 * number of indices to read should be larger than zero
 */
     if ( indexNum <= 0 ) return;
/*
 * does the table already exists?
 */

     H5E_BEGIN_TRY {
          dataID = H5Dopen( locID, arrName, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( dataID < 0 )
          NADC_RETURN_ERROR( NADC_ERR_HDF_RD, arrName );

     /* Get the dataspace handle */
     if ( (spaceID = H5Dget_space( dataID )) < 0 )
	  NADC_GOTO_ERROR( NADC_ERR_HDF_SPACE, arrName );

     /* Create a memory dataspace handle */
     if ( (memSpaceID = H5Screate_simple( rank, count, NULL )) < 0 )
	  NADC_GOTO_ERROR( NADC_ERR_HDF_SPACE, arrName );

     for ( nr = 0; nr < indexNum; nr++ ) {
	  hsize_t start[] = { pixelStart, metaIndex[nr] };

	  stat = H5Sselect_hyperslab( spaceID, H5S_SELECT_SET, start, stride, 
				      count, blocks );
	  if ( stat < 0 ) NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, arrName);

	  stat = H5Dread( dataID, H5T_NATIVE_FLOAT, memSpaceID, spaceID, 
			  H5P_DEFAULT, data + nr * pixelNum );
	  if ( stat < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_RD, arrName );
     }
done:
     if ( memSpaceID >= 0 ) (void) H5Sclose( memSpaceID );
     if ( spaceID >= 0 ) (void) H5Sclose( spaceID );
     (void) H5Dclose( dataID );
}

/*+++++++++++++++++++++++++
.IDENTifer   SDMF_rd_float_Matrix
.PURPOSE     read slabs from dataset ''arrName''
.INPUT/OUTPUT
  call as    SDMF_rd_float_Matrix( locID, arrName, metaIndex, 
                                     slabsize, dims, data );
     input:
           hid_t locID           :  HDF5 identifier of file or group
           char  arrName[]       :  name of dataset
           int   *metaIndex      :  index of orbit
           int   *slabsize       :  size of slab to read
           int   dims            :  dimensions of dataset, only tested for 3D
           
    output:
           float *data           :  array for data to read

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SDMF_rd_float_Matrix( hid_t locID, const char *arrName, 
			     const int *metaIndex, const int *slabsize, 
                             const int dims, float *data )
{
     register int nr;

     hid_t  dataID, spaceID = -1, memSpaceID = -1;
     herr_t stat;

/*  const int pixelStart = (pixelRange == NULL) ? 0 : pixelRange[0];
    const int pixelNum   = (pixelRange == NULL) ? \
                           SCIENCE_PIXELS : (pixelRange[1] - pixelRange[0] + 1);
*/
     const int     rank = dims;
     hsize_t count[dims]; // = { pixelNum, 1 };
     hsize_t start[dims];
     hsize_t stride[dims];
     hsize_t blocks[dims];
  
     for ( nr = 0; nr < dims; nr++ ) {
	  count[nr]  = (hsize_t) slabsize[nr];
	  start[nr]  = (hsize_t) 0;
	  stride[nr] = (hsize_t) 1;
	  blocks[nr] = (hsize_t) 1;
     }
     //start[0]= (hsize_t) 0;//is "third" dim
     //start[1]= (hsize_t) 0;//is "pixel" dim
     start[dims-1]= (hsize_t) *metaIndex;//is "orbit" dim
/*
 * does the table already exists?
 */
     H5E_BEGIN_TRY {
	  dataID = H5Dopen( locID, arrName, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( dataID < 0 )
	  NADC_RETURN_ERROR( NADC_ERR_HDF_RD, arrName );

     /* Get the dataspace handle */
     if ( (spaceID = H5Dget_space( dataID )) < 0 )
	  NADC_GOTO_ERROR( NADC_ERR_HDF_SPACE, arrName );

     /* Create a memory dataspace handle */
     if ( (memSpaceID = H5Screate_simple( rank, count, NULL )) < 0 )
	  NADC_GOTO_ERROR( NADC_ERR_HDF_SPACE, arrName );
     
     stat = H5Sselect_hyperslab( spaceID, H5S_SELECT_SET, start, stride,
				 count, blocks );
     if ( stat < 0 ) NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, arrName);
     
     stat = H5Dread( dataID, H5T_NATIVE_FLOAT, memSpaceID, spaceID, 
		     H5P_DEFAULT, data);
     if ( stat < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_RD, arrName );
done:
     if ( memSpaceID >= 0 ) (void) H5Sclose( memSpaceID );
     if ( spaceID >= 0 ) (void) H5Sclose( spaceID );
     (void) H5Dclose( dataID );
}
