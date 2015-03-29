/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2001 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_RD_H5_MEM
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA - HDF5
.LANGUAGE    ANSI C
.PURPOSE     read table for Reticon non-linearity correction
.INPUT/OUTPUT
  call as    SCIA_RD_H5_MEM( &mem );
     output: 
             struct scia_memcorr *mem :
                    size_t dims[2]     : dimension of matrix (hdf5 definition)
                    float **matrix     : memory correction matrix

.RETURNS     Nothing
.COMMENTS    You need to release allocated memory, use SCIA_FREE_H5_MEM
.ENVIRONment Alternative path can be provided by using: SCIA_MEMCORR_DB
.VERSION      1.0   23-Sept-2011 created by R. M. van Hees
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
#include <nadc_scia_cal.h>

/*+++++ Static Variables +++++*/
        /* NONE */

/*+++++ Macros +++++*/
        /* NONE */

/*+++++ Global Variables +++++*/
        /* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
        /* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_RD_H5_MEM( struct scia_memcorr *mem )
{
     hid_t  file_id = 0;
     hsize_t hdims[2];

     char *env_str = getenv( "SCIA_MEMCORR_DB" );
/*
 * open memory correction database (HDF5-file)
 */
     if ( env_str == NULL ) {
	  const char nlin_fl[] = DATA_DIR"/MEMcorr.h5";

          file_id = H5Fopen( nlin_fl, H5F_ACC_RDONLY, H5P_DEFAULT );
          if ( file_id < 0 ) 
               NADC_RETURN_ERROR( NADC_ERR_HDF_FILE, nlin_fl );
     } else {
          file_id = H5Fopen( env_str, H5F_ACC_RDONLY, H5P_DEFAULT );
          if ( file_id < 0 ) 
               NADC_RETURN_ERROR( NADC_ERR_HDF_FILE, env_str );
     }
/*
 * read datasets
 */
     (void) H5LTget_dataset_info( file_id, "MemTable", hdims, NULL, NULL );
     mem->dims[0] = (size_t) hdims[0];
     mem->dims[1] = (size_t) hdims[1];
     mem->matrix = ALLOC_R2D( mem->dims[0], mem->dims[1] );
     if ( mem->matrix == NULL ) 
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "mem->matrix" );
     (void) H5LTread_dataset_float( file_id, "MemTable", mem->matrix[0] );
/*
 * give message to user
 */
     NADC_ERROR( NADC_ERR_NONE, 
             "\n\tapplied memory correction (SRON-SCIA-PhE-RP-011)" );
 done:
     if ( file_id >= 0 ) (void) H5Fclose( file_id );
}

void SCIA_FREE_H5_MEM( struct scia_memcorr *mem )
{
     if ( mem->matrix != NULL ) FREE_2D( (void **) mem->matrix );     
}

/*
 * compile code with (on x86_32):
 * gcc -Wall -Os -DDATA_DIR='"/SCIA/share/nadc_tools"' -DTEST_PROG -I$HDF5_DIR/include -I../include -o test_rd_h5_mem scia_rd_h5_mem.c -L$HOME/lib -lNADC -L$HDF5_DIR/lib -lhdf5_hl -lhdf5 -lz -lm
 */
#ifdef TEST_PROG
#include <stdio.h>

int main()
{
     register size_t ii;

     struct scia_memcorr mem = { {0,0}, NULL };

     SCIA_RD_H5_MEM( &mem );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, "SCIA_RD_H5_MEM" );
     (void) fprintf( stdout, "DIMS: %zd %zd\n", mem.dims[0], mem.dims[1] );
     for ( ii = 0; ii < mem.dims[1]; ii++ )
	  (void) fprintf( stdout, "%15.6E", mem.matrix[0][ii] );
     (void) fprintf( stdout, "\n" );

done:
     SCIA_FREE_H5_MEM( &mem );
     NADC_Err_Trace( stderr );
     if ( IS_ERR_STAT_FATAL ) 
          return NADC_ERR_FATAL;
     else
          return NADC_ERR_NONE;
}
#endif
