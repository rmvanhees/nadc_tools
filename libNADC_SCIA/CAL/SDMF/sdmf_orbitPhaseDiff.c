/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2012 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SDMF_orbitPhaseDiff
.AUTHOR      R.M. van Hees
.KEYWORDS    SDMF - orbitPhase definition
.LANGUAGE    ANSI C
.PURPOSE     obtain orbitPhase difference between SDMF v2.4 and ATBD definition
.INPUT/OUTPUT
  call as    orbitPhaseDiff = SDMF_orbitPhaseDiff( absOrbit );
     input:  
           unsigned short absOrbit :  orbit number

.RETURNS     orbitPhase difference (float)
             error status passed by global variable ``nadc_stat''
.ENVIRONment None
.VERSION     1.0     31-May-2012   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <stdlib.h>
/* #include <string.h> */

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

/*+++++++++++++++++++++++++ SDMF version 2.4 ++++++++++++++++++++++++++++++*/
float SDMF_orbitPhaseDiff( int orbit )
{
     register hsize_t ni;

     char   rsp_file[MAX_STRING_LENGTH];

     hsize_t nfields, num_roe;
     hid_t   file_id;
     int     *orbitList = NULL;
     float   orbitPhaseDiff = 0.092f;

     const char   roe_flname[]  = "ROE_EXC_all.h5";
     const char   roe_tblname[] = "roe_entry";
/*
 * open output HDF5-file
 */
     (void) snprintf( rsp_file, MAX_STRING_LENGTH, "./%s", roe_flname );
     H5E_BEGIN_TRY {
          file_id = H5Fopen( rsp_file, H5F_ACC_RDONLY, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( file_id < 0 ) {
          (void) snprintf( rsp_file, MAX_STRING_LENGTH, 
                           "%s/%s", DATA_DIR, roe_flname );
          file_id = H5Fopen( rsp_file, H5F_ACC_RDONLY, H5P_DEFAULT );
          if ( file_id < 0 )
               NADC_GOTO_ERROR( NADC_ERR_HDF_FILE, rsp_file );
     }
/*
 * read info_h5 records
 */
     (void) H5TBget_table_info( file_id, roe_tblname, &nfields, &num_roe );
/*
 * read Orbit column
 */
     if ( num_roe == 0 ) return orbitPhaseDiff;

     orbitList = (int *) malloc( num_roe * sizeof( int ));
     if ( orbitList == NULL ) 
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "orbitList" );
     if ( H5LTread_dataset_int( file_id, "orbitList", orbitList ) < 0 )
	  NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "orbitList" );

     for ( ni = 0; ni < num_roe; ni++ ) {
	  if ( orbitList[ni] == orbit ) break;
     }

     if ( ni < num_roe ) {
	  double ECL_Exit, ECL_Entry, Period;

	  const size_t field_offset = 0;
	  const size_t dst_sizes    = sizeof( double );

	  H5TBread_fields_name( file_id, roe_tblname, "ECL_EXIT", ni, 1, 
				sizeof(double), &field_offset, &dst_sizes, 
				&ECL_Exit );
	  H5TBread_fields_name( file_id, roe_tblname, "ECL_ENTRY", ni, 1,
				sizeof(double), &field_offset, &dst_sizes, 
				&ECL_Entry );
	  H5TBread_fields_name( file_id, roe_tblname, "PERIOD", ni, 1,
				sizeof(double), &field_offset, &dst_sizes, 
				&Period );
	  orbitPhaseDiff = (float) ((ECL_Entry - ECL_Exit) / Period - 0.5) / 2;
     }
     (void) H5Fclose( file_id );
done:
     if ( orbitList != NULL ) free( orbitList );
     return orbitPhaseDiff;
}

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
#ifdef TEST_PROG
bool Use_Extern_Alloc = FALSE;

int main( int argc, char *argv[] )
{
     register unsigned short orbit = 200;

     do {
	  (void) printf( "%5hu %12.6g\n", orbit, SDMF_orbitPhaseDiff( orbit ) );
     } while( ++orbit < 52868 );

     NADC_Err_Trace( stderr );
     if ( IS_ERR_STAT_FATAL )
          exit( EXIT_FAILURE );
     else
          exit( EXIT_SUCCESS );
}
#endif /* TEST_PROG */
