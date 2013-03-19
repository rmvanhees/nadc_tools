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

.IDENTifer   NADC_EXT_H5_ARRAY
.AUTHOR      R.M. van Hees
.KEYWORDS    HDF 5 unlimited arrays
.LANGUAGE    ANSI C
.PURPOSE     subroutines create HDF5 unlimited arrays
.CONTAINS    NADC_CRE_H5_EArray_uint8, NADC_CRE_H5_EArray_uint16,
             NADC_CRE_H5_EArray_int32, NADC_CRE_H5_EArray_uint32, 
	     NADC_CRE_H5_EArray_float, NADC_CRE_H5_EArray_struct,
	     NADC_CAT_H5_EArray, NADC_WR_H5_EArray,
             NADC_RD_H5_EArray_uint8, NADC_RD_H5_EArray_uint16,
	     NADC_RD_H5_EArray_int32, NADC_RD_H5_EArray_uint32,
	     NADC_RD_H5_EArray_float, NADC_CRE_H5_EArray_struct
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     1.4     08-Mar-2012   added struct routines for compounds, RvH
             1.3     26-Sep-2011   added int32 & uint32 routines, RvH
             1.2     26-Sep-2011   optimized chunk sizes + added shuffle, RvH
             1.1     04-Feb-2010   optimized chunk sizes, RvH
             1.0     02-Jul-2009   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_SOURCE 2

/*+++++ System headers +++++*/
#include <stdlib.h>
#include <string.h>

#include <hdf5.h>
#include <hdf5_hl.h>

/*+++++ Local Headers +++++*/
#include <nadc_common.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static inline
void NADC_set_chunk_sizes( const hsize_t xdim, int size_type,
			   /*@out@*/ hsize_t *dims_chunk )
{
     size_t nbyte;
     const hsize_t MAX_CHUNK_SIZE = 65536;

     if ( xdim == 1 ) 
	  nbyte = MAX_CHUNK_SIZE / (size_type * 4);
     else if ( xdim < 128 )
	  nbyte = MAX_CHUNK_SIZE / (size_type * 8);
     else if ( xdim < 256 )
	  nbyte = MAX_CHUNK_SIZE / (size_type * 128);
     else if ( xdim < 512 )
	  nbyte = MAX_CHUNK_SIZE / (size_type * 256);
     else if ( xdim < 1024 )
	  nbyte = MAX_CHUNK_SIZE / (size_type * 512);
     else
	  nbyte = MAX_CHUNK_SIZE / (size_type * 4096);

     dims_chunk[0] = (nbyte > 0) ? nbyte : 1;
     dims_chunk[1] = xdim;
}

/*+++++++++++++++++++++++++
.IDENTifer   NADC_CRE_H5_EArray
.PURPOSE     create extensible HDF5 dataset
.INPUT/OUTPUT
  call as    stat = NADC_CRE_H5_EArray( locID, dset_name, 
			                rank, dims, dims_chunk, extdim, 
			                compress, fill_data, typeID, buff );
     input:
            hid_t locID           :  HDF5 identifier of file or group
	    char *dset_name       :  name of dataset
	    int rank              :  number of dimensions
	    hsize_t *dims         :  size of each dimension, fasted last!
	    hsize_t *dims_chunk   :  chunk sizes, fasted dimension last!
	    int extdim            :  index of expendable dimension
	    unsigned int compress :  compression level (zero for no compression)
	    void *fill_data       :  Fill value for data (or NULL)
	    hid_t typeID          :  data type (HDF5 identifier)
	    void *buffer          :  buffer with data to write (or NULL)
	    
.RETURNS     A negative value is returned on failure 
.COMMENTS    none
-------------------------*/
static
herr_t NADC_CRE_H5_EArray( hid_t locID, const char *dset_name, 
			   const int rank, const hsize_t *dims, 
			   const hsize_t *dims_chunk, int extdim, 
			   unsigned int compress, void *fill_data,
			   hid_t typeID, const void *buffer )
{
     const char prognm[] = "NADC_CRE_H5_EArray";

     hid_t   dataID  = -1;
     hid_t   spaceID = -1;
     hid_t   plistID = -1;

     herr_t  stat;

/* check if the array has to be chunked or not */
     if ( dims_chunk != NULL ) {
	  register int ni = 0;

	  hsize_t *maxdims = (hsize_t *) malloc( rank * sizeof(hsize_t) );
	  if ( maxdims == NULL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "maxdims" );

	  do {
	       if ( ni == extdim )
		    maxdims[ni] = H5S_UNLIMITED;
	       else
		    maxdims[ni] = 
			 dims[ni] < dims_chunk[ni] ? dims_chunk[ni] : dims[ni];
	  } while ( ++ni < rank );
	  spaceID = H5Screate_simple( rank, dims, maxdims );
	  free( maxdims );
	  if ( spaceID < 0 ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_SPACE, dset_name );

	  /* Modify dataset creation properties, i.e. enable chunking  */
	  plistID = H5Pcreate( H5P_DATASET_CREATE );
	  if ( H5Pset_chunk( plistID, rank, dims_chunk ) < 0 ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_PLIST, "set chunking" );

          /* set the fill value using a struct as the data type */
	  if ( fill_data != NULL 
	       && H5Pset_fill_value( plistID, typeID, fill_data ) < 0 )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_PLIST, "set FillValue" );

          /* dataset creation property list is modified to use */
	  if ( compress > 0 
	       && (H5Pset_shuffle( plistID ) < 0
		   || H5Pset_deflate( plistID, compress ) < 0) )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_PLIST, "set compression" );

          /* create the (chunked) dataset */
	  dataID = H5Dcreate( locID, dset_name, typeID, spaceID, 
			      H5P_DEFAULT, plistID, H5P_DEFAULT );
	  if ( dataID < 0 ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DATA, dset_name );

          /* end access to the property list */
	  if ( H5Pclose( plistID ) < 0 )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_PLIST, "failed to close" );
     } else {
	  if ( (spaceID = H5Screate_simple( rank, dims, NULL )) < 0 )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_SPACE, dset_name );

          /* create the dataset (not chunked) */
	  dataID = H5Dcreate( locID, dset_name, typeID, spaceID, 
			      H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
	  if ( dataID < 0 ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DATA, dset_name );
     }
/*
 * write the data
 */
     if ( buffer != NULL ) {
	  stat = H5Dwrite( dataID, typeID, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
			   buffer );
	  if ( stat < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, dset_name );
     }
     (void) H5Sclose( spaceID );
     (void) H5Dclose( dataID );
     return 0;
 done:
     if ( plistID > 0 ) (void) H5Pclose( plistID );
     if ( spaceID > 0 ) (void) H5Sclose( spaceID );
     if ( dataID > 0 ) (void) H5Dclose( dataID );
     return -1;
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   NADC_CRE_H5_EArray_uint8
.PURPOSE     create HDF5 dataset, X-dimension fixed, Y-dimension extensible 
.INPUT/OUTPUT
  call as    NADC_CRE_H5_EArray_uint8( locID, arrName, xdim, compression,
                                       buffer );
     input:
            hid_t locID     :  HDF5 identifier of file or group
	    char *arrName   :  name of dataset
	    hsize_t xdim    :  size if first an fasted varying dimension
	    int compression :  compression level (0 = no compression)
            unsigned char *buffer :  data to write

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void NADC_CRE_H5_EArray_uint8( hid_t locID, const char *arrName,
			       const hsize_t xdim, int compression,
			       const unsigned char *buffer )
{
     const char prognm[] = "NADC_CRE_H5_EArray_uint8";

     const int     extdim  = 0;
     const int     rank    = (xdim == 1) ? 1 : 2;
     const hsize_t dims[2] = { 1, xdim };

     hsize_t dims_chunk[2];
     herr_t  stat;

     NADC_set_chunk_sizes( xdim, sizeof(char), dims_chunk );
     stat = NADC_CRE_H5_EArray( locID, arrName, rank, dims, dims_chunk,
                                extdim, (unsigned int) compression, NULL, 
                                H5T_NATIVE_UCHAR, buffer );
     if ( stat < 0 ) NADC_ERROR( prognm, NADC_ERR_HDF_CRE, arrName );
}

/*+++++++++++++++++++++++++
.IDENTifer   NADC_CRE_H5_EArray_uint16
.PURPOSE     create HDF5 dataset, X-dimension fixed, Y-dimension extensible 
.INPUT/OUTPUT
  call as    NADC_CRE_H5_EArray_uint16( locID, arrName, xdim, compression, 
                                        buffer );
     input:
            hid_t locID     :  HDF5 identifier of file or group
	    char *arrName   :  name of dataset
	    hsize_t xdim    :  size if first an fasted varying dimension
	    int compression :  compression level (0 = no compression)
            unsigned short *buffer : data to write

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void NADC_CRE_H5_EArray_uint16( hid_t locID, const char *arrName,
				const hsize_t xdim, int compression,
				const unsigned short *buffer )
{
     const char prognm[] = "NADC_CRE_H5_EArray_uint16";

     const int     extdim  = 0;
     const int     rank    = (xdim == 1) ? 1 : 2;
     const hsize_t dims[2] = { 1, xdim };

     hsize_t dims_chunk[2];
     herr_t  stat;

     NADC_set_chunk_sizes( xdim, sizeof(short), dims_chunk );
     stat = NADC_CRE_H5_EArray( locID, arrName, rank, dims, dims_chunk,
                                extdim, (unsigned int) compression, NULL,
                                H5T_NATIVE_USHORT, buffer );
     if ( stat < 0 ) NADC_ERROR( prognm, NADC_ERR_HDF_CRE, arrName );
}

/*+++++++++++++++++++++++++
.IDENTifer   NADC_CRE_H5_EArray_int32
.PURPOSE     create HDF5 dataset, X-dimension fixed, Y-dimension extensible 
.INPUT/OUTPUT
  call as    NADC_CRE_H5_EArray_int32( locID, arrName, xdim, compression, 
                                       buffer );
     input:
            hid_t locID     :  HDF5 identifier of file or group
	    char *arrName   :  name of dataset
	    hsize_t xdim    :  size if first an fasted varying dimension
	    int compression :  compression level (0 = no compression)
            int *buffer     : data to write

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void NADC_CRE_H5_EArray_int32( hid_t locID, const char *arrName,
			       const hsize_t xdim, int compression,
			       const int *buffer )
{
     const char prognm[] = "NADC_CRE_H5_EArray_int32";

     const int     extdim  = 0;
     const int     rank    = (xdim == 1) ? 1 : 2;
     const hsize_t dims[2] = { 1, xdim };

     hsize_t dims_chunk[2];
     herr_t  stat;

     NADC_set_chunk_sizes( xdim, sizeof(int), dims_chunk );
     stat = NADC_CRE_H5_EArray( locID, arrName, rank, dims, dims_chunk,
                                extdim, (unsigned int) compression, NULL,
                                H5T_NATIVE_INT, buffer );
     if ( stat < 0 ) NADC_ERROR( prognm, NADC_ERR_HDF_CRE, arrName );
}

/*+++++++++++++++++++++++++
.IDENTifer   NADC_CRE_H5_EArray_uint32
.PURPOSE     create HDF5 dataset, X-dimension fixed, Y-dimension extensible 
.INPUT/OUTPUT
  call as    NADC_CRE_H5_EArray_iunt32( locID, arrName, xdim, compression, 
                                        buffer );
     input:
            hid_t locID     :  HDF5 identifier of file or group
	    char *arrName   :  name of dataset
	    hsize_t xdim    :  size if first an fasted varying dimension
	    int compression :  compression level (0 = no compression)
            unsigned int *buffer : data to write

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void NADC_CRE_H5_EArray_uint32( hid_t locID, const char *arrName,
			       const hsize_t xdim, int compression,
			       const unsigned int *buffer )
{
     const char prognm[] = "NADC_CRE_H5_EArray_uint32";

     const int     extdim  = 0;
     const int     rank    = (xdim == 1) ? 1 : 2;
     const hsize_t dims[2] = { 1, xdim };

     hsize_t dims_chunk[2];
     herr_t  stat;

     NADC_set_chunk_sizes( xdim, sizeof(int), dims_chunk );
     stat = NADC_CRE_H5_EArray( locID, arrName, rank, dims, dims_chunk,
                                extdim, (unsigned int) compression, NULL,
                                H5T_NATIVE_UINT, buffer );
     if ( stat < 0 ) NADC_ERROR( prognm, NADC_ERR_HDF_CRE, arrName );
}

/*+++++++++++++++++++++++++
.IDENTifer   NADC_CRE_H5_EArray_float
.PURPOSE     create HDF5 dataset, X-dimension fixed, Y-dimension extensible 
.INPUT/OUTPUT
  call as    NADC_CRE_H5_EArray_float( locID, arrName, xdim, compression, 
                                       buffer );
     input:
            hid_t locID     :  HDF5 identifier of file or group
	    char *arrName   :  name of dataset
	    hsize_t xdim    :  size if first an fasted varying dimension
	    int compression :  compression level (0 = no compression)
            float           :  data to write

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void NADC_CRE_H5_EArray_float( hid_t locID, const char *arrName,
			       const hsize_t xdim, int compression,
			       const float *buffer )
{
     const char prognm[] = "NADC_CRE_H5_EArray_float";

     const int     extdim  = 0;
     const int     rank    = (xdim == 1) ? 1 : 2;
     const hsize_t dims[2] = { 1, xdim };

     hsize_t dims_chunk[2];
     herr_t  stat;

     NADC_set_chunk_sizes( xdim, sizeof(float), dims_chunk );
     stat = NADC_CRE_H5_EArray( locID, arrName, rank, dims, dims_chunk,
                                extdim, (unsigned int) compression, NULL,
                                H5T_NATIVE_FLOAT, buffer );
     if ( stat < 0 ) NADC_ERROR( prognm, NADC_ERR_HDF_CRE, arrName );
}

/*+++++++++++++++++++++++++
.IDENTifer   NADC_CRE_H5_EArray_struct
.PURPOSE     create HDF5 dataset, X-dimension fixed, Y-dimension extensible 
.INPUT/OUTPUT
  call as    NADC_CRE_H5_EArray_struct( locID, arrName, xdim, compression, 
                                        typeID, buffer );
     input:
            hid_t locID     :  HDF5 identifier of file or group
	    char *arrName   :  name of dataset
	    hsize_t xdim    :  size if first an fasted varying dimension
	    int compression :  compression level (0 = no compression)
	    hid_t typeID    :  HDF5 type ID of compound data
            void            :  data to write

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void NADC_CRE_H5_EArray_struct( hid_t locID, const char *arrName,
				const hsize_t xdim, int compression,
				hid_t typeID, const void *buffer )
{
     const char prognm[] = "NADC_CRE_H5_EArray_struct";

     const int     extdim  = 0;
     const int     rank    = (xdim == 1) ? 1 : 2;
     const hsize_t dims[2] = { 1, xdim };

     hsize_t dims_chunk[2];
     herr_t  stat;

     NADC_set_chunk_sizes( xdim, H5Tget_size( typeID ) , dims_chunk );
     stat = NADC_CRE_H5_EArray( locID, arrName, rank, dims, dims_chunk,
                                extdim, (unsigned int) compression, NULL,
                                typeID, buffer );
     if ( stat < 0 ) NADC_ERROR( prognm, NADC_ERR_HDF_CRE, arrName );
}

/*+++++++++++++++++++++++++
.IDENTifer   NADC_CAT_H5_EArray
.PURPOSE     concatenate data to a HDF5 dataset
.INPUT/OUTPUT
  call as   NADC_CAT_H5_EArray( locID, dset_name, extdim, count, buffer );
     input:
            hid_t locID      :   HDF5 identifier of file or group
	    char  *dset_name :   name of dataset
	    int   extdim     :   dimension to extend
            int   count      :   number of rows to write
	    void  *buffer    :   data to write
	    
.RETURNS     Nothing, error status passed by global variable ``nadc_stat'' 
.COMMENTS    None
-------------------------*/
void NADC_CAT_H5_EArray( hid_t locID, const char *dset_name, 
			 int extdim, size_t count, const void *buffer )
{
     const char prognm[] = "NADC_CAT_H5_EArray";

     int      rank;

     hid_t    dataID;
     hid_t    spaceID = -1;
     hid_t    mem_spaceID = -1;
     hid_t    typeID = -1;
     hsize_t  *dims = NULL;
     hsize_t  *dims_ext = NULL;
     hsize_t  *offset = NULL;
     herr_t   stat;

/* check number of rows to add */
     if ( count <= 0 ) return;

/* open the dataset */
     if ( (dataID = H5Dopen( locID, dset_name, H5P_DEFAULT )) < 0 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DATA, dset_name );

/* get the dataspace handle */
     if ( (spaceID = H5Dget_space( dataID )) < 0 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_SPACE, dset_name );

/* get rank */
     if ( (rank = H5Sget_simple_extent_ndims( spaceID )) < 0 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_PLIST, "get rank" );

/* get dimensions */
     dims = (hsize_t *) malloc( rank * sizeof(hsize_t) );
     dims_ext = (hsize_t *) malloc( rank * sizeof(hsize_t) );
     offset = (hsize_t *) calloc( rank, sizeof(hsize_t) );
     if ( H5Sget_simple_extent_dims( spaceID, dims, NULL ) < 0 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_SPACE, dset_name );
     offset[extdim] = dims[extdim];
     (void) memcpy( dims_ext, dims, rank * sizeof(hsize_t) );
     dims_ext[extdim] = count;
     dims[extdim] += count;

/* terminate access to dataspace */
     if ( H5Sclose( spaceID ) < 0 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_SPACE, "failed to close" );

/* extend the dataset */
     if ( H5Dset_extent( dataID, dims ) < 0 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_PLIST, "failed to extend" );

/* select a hyperslab */
     if ( (spaceID = H5Dget_space( dataID )) < 0 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_SPACE, "dset_name" );
     stat = H5Sselect_hyperslab( spaceID, H5S_SELECT_SET, offset, NULL,
				 dims_ext, NULL );
     free( dims );
     free( offset );
     if ( stat < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_SPACE, dset_name );

/* define memory space */
     mem_spaceID = H5Screate_simple( rank, dims_ext, NULL );
     free( dims_ext );

/* get an identifier for datatype. */
     if ( (typeID = H5Dget_type( dataID )) < 0 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, dset_name );

/* write the data to hyperslab */
     stat = H5Dwrite( dataID, typeID, mem_spaceID, spaceID, H5P_DEFAULT, 
		      buffer );
     if ( stat < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, dset_name );

/* terminate access to datatype */
     (void) H5Tclose( typeID );

/* terminate access to dataspace */
     (void) H5Sclose( spaceID );
     (void) H5Sclose( mem_spaceID );

/* end access to dataset */
     (void) H5Dclose( dataID );
     return;
 done:
     if ( dims     != NULL ) free( dims );
     if ( dims_ext != NULL ) free( dims_ext );
     if ( offset   != NULL ) free( offset );
     if ( typeID > 0 ) (void) H5Tclose( typeID );
     if ( spaceID > 0 ) (void) H5Sclose( spaceID );
     if ( mem_spaceID > 0 ) (void) H5Sclose( mem_spaceID );
     if ( dataID > 0 ) (void) H5Dclose( dataID );
     return;
}

/*+++++++++++++++++++++++++
.IDENTifer   NADC_WR_H5_EArray
.PURPOSE     write data to a HDF5 dataset (no append!)
.INPUT/OUTPUT
  call as    NADC_WR_H5_EArray( locID, dset_name, Ystart, Ycount, buffer );
     input:
            hid_t locID      :  HDF5 identifier of file or group
	    char *dset_name  :  name of dataset
	    size_t Ystart    :  index of first row to overwrite (> 1 !)
	    size_t Ycount    :  number of rows to write
	    void *buffer     :  data to write
	    
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void NADC_WR_H5_EArray( hid_t locID, const char *dset_name, 
			size_t Ystart, size_t Ycount, const void *buffer )
{
     const char prognm[] = "NADC_WR_H5_EArray";

     int      rank;

     herr_t   stat;

     hid_t    dataID;
     hid_t    spaceID = -1;
     hid_t    mem_spaceID = -1;
     hid_t    typeID = -1;

     hsize_t  count[2], dims[2];

     const hsize_t start[2] = {Ystart, 0};

/* open the dataset */
     if ( (dataID = H5Dopen( locID, dset_name, H5P_DEFAULT )) < 0 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DATA, dset_name );

/* get the dataspace handle */
     if ( (spaceID = H5Dget_space( dataID )) < 0 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_SPACE, dset_name );

/* get rank */
     if ( (rank = H5Sget_simple_extent_ndims( spaceID )) <= 0 ) {
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_PLIST, "get rank" );
     } else if ( rank > 2 ) {
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "rank limited to 2" );
     }
     (void) H5Sget_simple_extent_dims( spaceID, dims, NULL );

/* create a simple memory data space */
     count[0] = Ycount;
     count[1] = dims[1];
     if ( (mem_spaceID = H5Screate_simple( rank, count, NULL )) < 0 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_SPACE, dset_name );

/* define a hyperslab in the dataset */
     (void) H5Sselect_hyperslab( spaceID, H5S_SELECT_SET, 
				 start, NULL, count, NULL );

/* get an identifier for the datatype. */
     if ( (typeID = H5Dget_type( dataID )) < 0 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DTYPE, dset_name );

/* write data to hyperslap */
     stat = H5Dwrite( dataID, typeID, mem_spaceID, spaceID, H5P_DEFAULT, 
		      buffer );
     if ( stat < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, dset_name );
 done:
/* terminate access to the datatype */
     if ( typeID > 0 ) (void) H5Tclose( typeID );

/* terminate access to the dataspace */
     if ( mem_spaceID > 0 ) (void) H5Sclose( mem_spaceID );
     if ( spaceID > 0 ) (void) H5Sclose( spaceID );

/* end access to the dataset */
     if ( dataID > 0 ) (void) H5Dclose( dataID );
}

/*+++++++++++++++++++++++++
.IDENTifer   NADC_RD_H5_EArray
.PURPOSE     read data from a HDF5 dataset
.INPUT/OUTPUT
  call as    NADC_RD_H5_EArray_type( locID, dset_name, xdim, indexNum, 
                                     metaIndex, buffer );
     input:
            hid_t locID      :  HDF5 identifier of file or group
	    char  *dset_name :  name of dataset
	    hsize_t xdim     :  number of elements along fastest axis
	    int   indexNum   :  number of rows to read
	    int   *metaIndex :  indices to rows to be read
     output:
	    void *buffer     :  data buffer
	    
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void NADC_RD_H5_EArray_uint8( hid_t locID, const char *dset_name, 
			      hsize_t xdim, int indexNum, 
			      const int *metaIndex, unsigned char *buffer )
{
     const char prognm[] = "NADC_RD_H5_EArray_uint8";

     register int nr;

     hid_t  dataID, spaceID = -1, memSpaceID = -1;
     herr_t stat;

     const int     rank = 2;

     const hsize_t count[] = {1, xdim};
     const hsize_t *stride = NULL;
     const hsize_t *blocks = NULL;

     /* number of indices to read should be larger than zero */
     if ( indexNum <= 0 ) return;

     /* does the table already exists? */
     H5E_BEGIN_TRY {
          dataID = H5Dopen( locID, dset_name, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( dataID < 0 )
          NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_RD, dset_name );

     /* Get the dataspace handle */
     if ( (spaceID = H5Dget_space( dataID )) < 0 )
          NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_SPACE, dset_name );

     /* Create a memory dataspace handle */
     if ( (memSpaceID = H5Screate_simple( rank, count, NULL )) < 0 )
          NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_SPACE, dset_name );

     for ( nr = 0; nr < indexNum; nr++ ) {
          hsize_t start[] = {metaIndex[nr], 0};

          stat = H5Sselect_hyperslab( spaceID, H5S_SELECT_SET, start, stride,
                                      count, blocks );
          if ( stat < 0 ) NADC_GOTO_ERROR(prognm, NADC_ERR_HDF_DATA, dset_name);

          stat = H5Dread( dataID, H5T_NATIVE_UCHAR, memSpaceID, spaceID,
                          H5P_DEFAULT, &buffer[nr * xdim] );
          if ( stat < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_RD, dset_name );
     }
done:
     if ( memSpaceID >= 0 ) (void) H5Sclose( memSpaceID );
     if ( spaceID >= 0 ) (void) H5Sclose( spaceID );
     (void) H5Dclose( dataID );
}

void NADC_RD_H5_EArray_uint16( hid_t locID, const char *dset_name, 
			       hsize_t xdim, int indexNum, 
			       const int *metaIndex, unsigned short *buffer )
{
     const char prognm[] = "NADC_RD_H5_EArray_uint16";

     register int nr;

     hid_t  dataID, spaceID = -1, memSpaceID = -1;
     herr_t stat;

     const int     rank = 2;

     const hsize_t count[] = {1, xdim};
     const hsize_t *stride = NULL;
     const hsize_t *blocks = NULL;

     /* number of indices to read should be larger than zero */
     if ( indexNum <= 0 ) return;

     /* does the table already exists? */
     H5E_BEGIN_TRY {
          dataID = H5Dopen( locID, dset_name, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( dataID < 0 )
          NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_RD, dset_name );

     /* Get the dataspace handle */
     if ( (spaceID = H5Dget_space( dataID )) < 0 )
          NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_SPACE, dset_name );

     /* Create a memory dataspace handle */
     if ( (memSpaceID = H5Screate_simple( rank, count, NULL )) < 0 )
          NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_SPACE, dset_name );

     for ( nr = 0; nr < indexNum; nr++ ) {
          hsize_t start[] = {metaIndex[nr], 0};

          stat = H5Sselect_hyperslab( spaceID, H5S_SELECT_SET, start, stride,
                                      count, blocks );
          if ( stat < 0 ) NADC_GOTO_ERROR(prognm, NADC_ERR_HDF_DATA, dset_name);

          stat = H5Dread( dataID, H5T_NATIVE_USHORT, memSpaceID, spaceID,
                          H5P_DEFAULT, &buffer[nr * xdim] );
          if ( stat < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_RD, dset_name );
     }
done:
     if ( memSpaceID >= 0 ) (void) H5Sclose( memSpaceID );
     if ( spaceID >= 0 ) (void) H5Sclose( spaceID );
     (void) H5Dclose( dataID );
}

void NADC_RD_H5_EArray_int32( hid_t locID, const char *dset_name, 
			      hsize_t xdim, int indexNum, 
			      const int *metaIndex, int *buffer )
{
     const char prognm[] = "NADC_RD_H5_EArray_int32";

     register int nr;

     hid_t  dataID, spaceID = -1, memSpaceID = -1;
     herr_t stat;

     const int     rank = 2;

     const hsize_t count[] = {1, xdim};
     const hsize_t *stride = NULL;
     const hsize_t *blocks = NULL;

     /* number of indices to read should be larger than zero */
     if ( indexNum <= 0 ) return;

     /* does the table already exists? */
     H5E_BEGIN_TRY {
          dataID = H5Dopen( locID, dset_name, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( dataID < 0 )
          NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_RD, dset_name );

     /* Get the dataspace handle */
     if ( (spaceID = H5Dget_space( dataID )) < 0 )
          NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_SPACE, dset_name );

     /* Create a memory dataspace handle */
     if ( (memSpaceID = H5Screate_simple( rank, count, NULL )) < 0 )
          NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_SPACE, dset_name );

     for ( nr = 0; nr < indexNum; nr++ ) {
          hsize_t start[] = {metaIndex[nr], 0};

          stat = H5Sselect_hyperslab( spaceID, H5S_SELECT_SET, start, stride,
                                      count, blocks );
          if ( stat < 0 ) NADC_GOTO_ERROR(prognm, NADC_ERR_HDF_DATA, dset_name);

          stat = H5Dread( dataID, H5T_NATIVE_INT, memSpaceID, spaceID,
                          H5P_DEFAULT, &buffer[nr * xdim] );
          if ( stat < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_RD, dset_name );
     }
done:
     if ( memSpaceID >= 0 ) (void) H5Sclose( memSpaceID );
     if ( spaceID >= 0 ) (void) H5Sclose( spaceID );
     (void) H5Dclose( dataID );
}

void NADC_RD_H5_EArray_uint32( hid_t locID, const char *dset_name, 
			      hsize_t xdim, int indexNum, 
			      const int *metaIndex, unsigned int *buffer )
{
     const char prognm[] = "NADC_RD_H5_EArray_uint32";

     register int nr;

     hid_t  dataID, spaceID = -1, memSpaceID = -1;
     herr_t stat;

     const int     rank = 2;

     const hsize_t count[] = {1, xdim};
     const hsize_t *stride = NULL;
     const hsize_t *blocks = NULL;

     /* number of indices to read should be larger than zero */
     if ( indexNum <= 0 ) return;

     /* does the table already exists? */
     H5E_BEGIN_TRY {
          dataID = H5Dopen( locID, dset_name, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( dataID < 0 )
          NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_RD, dset_name );

     /* Get the dataspace handle */
     if ( (spaceID = H5Dget_space( dataID )) < 0 )
          NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_SPACE, dset_name );

     /* Create a memory dataspace handle */
     if ( (memSpaceID = H5Screate_simple( rank, count, NULL )) < 0 )
          NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_SPACE, dset_name );

     for ( nr = 0; nr < indexNum; nr++ ) {
          hsize_t start[] = {metaIndex[nr], 0};

          stat = H5Sselect_hyperslab( spaceID, H5S_SELECT_SET, start, stride,
                                      count, blocks );
          if ( stat < 0 ) NADC_GOTO_ERROR(prognm, NADC_ERR_HDF_DATA, dset_name);

          stat = H5Dread( dataID, H5T_NATIVE_UINT, memSpaceID, spaceID,
                          H5P_DEFAULT, &buffer[nr * xdim] );
          if ( stat < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_RD, dset_name );
     }
done:
     if ( memSpaceID >= 0 ) (void) H5Sclose( memSpaceID );
     if ( spaceID >= 0 ) (void) H5Sclose( spaceID );
     (void) H5Dclose( dataID );
}

void NADC_RD_H5_EArray_float( hid_t locID, const char *dset_name, 
			      hsize_t xdim, int indexNum, 
			      const int *metaIndex, float *buffer )
{
     const char prognm[] = "NADC_RD_H5_EArray_float";

     register int nr;

     hid_t  dataID, spaceID = -1, memSpaceID = -1;
     herr_t stat;

     const int     rank = 2;

     const hsize_t count[] = {1, xdim};
     const hsize_t *stride = NULL;
     const hsize_t *blocks = NULL;

     /* number of indices to read should be larger than zero */
     if ( indexNum <= 0 ) return;

     /* does the table already exists? */
     H5E_BEGIN_TRY {
          dataID = H5Dopen( locID, dset_name, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( dataID < 0 )
          NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_RD, dset_name );

     /* Get the dataspace handle */
     if ( (spaceID = H5Dget_space( dataID )) < 0 )
          NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_SPACE, dset_name );

     /* Create a memory dataspace handle */
     if ( (memSpaceID = H5Screate_simple( rank, count, NULL )) < 0 )
          NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_SPACE, dset_name );

     for ( nr = 0; nr < indexNum; nr++ ) {
          hsize_t start[] = {metaIndex[nr], 0};

          stat = H5Sselect_hyperslab( spaceID, H5S_SELECT_SET, start, stride,
                                      count, blocks );
          if ( stat < 0 ) NADC_GOTO_ERROR(prognm, NADC_ERR_HDF_DATA, dset_name);

          stat = H5Dread( dataID, H5T_NATIVE_FLOAT, memSpaceID, spaceID,
                          H5P_DEFAULT, &buffer[nr * xdim] );
          if ( stat < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_RD, dset_name );
     }
done:
     if ( memSpaceID >= 0 ) (void) H5Sclose( memSpaceID );
     if ( spaceID >= 0 ) (void) H5Sclose( spaceID );
     (void) H5Dclose( dataID );
}

