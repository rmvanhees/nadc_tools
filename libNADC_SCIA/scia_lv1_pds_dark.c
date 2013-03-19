/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2000 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV1_PDS_DARK
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 data
.LANGUAGE    ANSI C
.PURPOSE     read/write average of the Dark Measurements per State
.COMMENTS    contains SCIA_LV1_RD_DARK and SCIA_LV1_WR_DARK
.ENVIRONment None
.VERSION      4.0   11-Oct-2005 use direct write, add usage of SCIA_LV1_ADD_DSD
              3.0   14-Apr-2005 added routine to write Dark-struct to file, RvH
              2.2   22-Mar-2002	test number of DSD; can be zero, RvH
              2.1   16-Jan-2002	use of global Use_Extern_Alloc, RvH 
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              1.1   04-Oct-2001	changed input/output, RvH 
              1.0   03-Jan-2001 created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#ifdef _SWAP_TO_LITTLE_ENDIAN
#include <swap_bytes.h>

static
void Sun2Intel_DARK( struct dark_scia *dark )
{
     register int ni;

     dark->mjd.days = byte_swap_32( dark->mjd.days );
     dark->mjd.secnd = byte_swap_u32( dark->mjd.secnd );
     dark->mjd.musec = byte_swap_u32( dark->mjd.musec );
     for ( ni = 0; ni < (SCIENCE_PIXELS); ni++ ) {
	  IEEE_Swap__FLT( &dark->dark_spec[ni] );
	  IEEE_Swap__FLT( &dark->sdev_dark_spec[ni] );
	  IEEE_Swap__FLT( &dark->sol_stray[ni] );
	  IEEE_Swap__FLT( &dark->sol_stray_error[ni] );
     }
     for ( ni = 0; ni < (2 * PMD_NUMBER); ni++ ) {
	  IEEE_Swap__FLT( &dark->pmd_off[ni] );
	  IEEE_Swap__FLT( &dark->pmd_off_error[ni] );
     }
     for ( ni = 0; ni < PMD_NUMBER; ni++ ) {
	  IEEE_Swap__FLT( &dark->pmd_stray[ni] );
	  IEEE_Swap__FLT( &dark->pmd_stray_error[ni] );
     }
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_DARK
.PURPOSE     read average of the Dark Measurements per State
.INPUT/OUTPUT
  call as   nr_dsr = SCIA_LV1_RD_DARK( fd, num_dsd, dsd, &dark );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi *dsd  :   structure for the DSDs
    output:
            struct dark_scia **dark :  average of the dark measurements

.RETURNS     number of data set records read (unsigned int)
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV1_RD_DARK( FILE *fd, unsigned int num_dsd, 
			       const struct dsd_envi * dsd,
			       struct dark_scia **dark_out )
{
     const char prognm[] = "SCIA_LV1_RD_DARK";

     char         *dark_pntr, *dark_char = NULL;
     size_t       dsr_size, nr_byte;
     unsigned int indx_dsd;

     unsigned int nr_dsr = 0;

     struct dark_scia *dark;

     const char dsd_name[] = "DARK_AVERAGE";
/*
 * get index to data set descriptor
 */
     NADC_ERR_SAVE();
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, dsd_name );
     if ( IS_ERR_STAT_ABSENT || dsd[indx_dsd].num_dsr == 0 ) {
	  NADC_ERR_RESTORE();
          dark_out[0] = NULL;
	  return 0u;
     }
     dsr_size = (size_t) dsd[indx_dsd].dsr_size;
/*
 * allocate memory to store output structures
 */
     if ( ! Use_Extern_Alloc ) {
	  dark_out[0] = (struct dark_scia *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct dark_scia));
     }
     if ( (dark = dark_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "dark" );
/*
 * allocate memory to temporary store data for output structure
 */
     if ( (dark_char = (char *) malloc( dsr_size )) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "dark_char" );
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
/*
 * read data set records
 */
     do {
	  if ( fread( dark_char, dsr_size, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
/*
 * read data buffer to DARK structure
 */
	  (void) memcpy( &dark->mjd.days, dark_char, ENVI_INT );
	  dark_pntr = dark_char + ENVI_INT;
	  (void) memcpy( &dark->mjd.secnd, dark_pntr, ENVI_UINT );
	  dark_pntr += ENVI_UINT;
	  (void) memcpy( &dark->mjd.musec, dark_pntr, ENVI_UINT );
	  dark_pntr += ENVI_UINT;
	  (void) memcpy( &dark->flag_mds, dark_pntr, ENVI_UCHAR );
	  dark_pntr += ENVI_UCHAR;

	  nr_byte = (size_t) (SCIENCE_PIXELS) * ENVI_FLOAT;
	  (void) memcpy( dark->dark_spec, dark_pntr, nr_byte );
	  dark_pntr += nr_byte;
	  (void) memcpy( dark->sdev_dark_spec, dark_pntr, nr_byte );
	  dark_pntr += nr_byte;
	  nr_byte = (size_t) (2 * PMD_NUMBER) * ENVI_FLOAT;
	  (void) memcpy( dark->pmd_off, dark_pntr, nr_byte );
	  dark_pntr += nr_byte;
	  (void) memcpy( dark->pmd_off_error, dark_pntr, nr_byte );
	  dark_pntr += nr_byte;
	  nr_byte = (size_t) (SCIENCE_PIXELS) * ENVI_FLOAT;
	  (void) memcpy( dark->sol_stray, dark_pntr, nr_byte );
	  dark_pntr += nr_byte;
	  (void) memcpy( dark->sol_stray_error, dark_pntr, nr_byte );
	  dark_pntr += nr_byte;
	  nr_byte = (size_t) PMD_NUMBER * ENVI_FLOAT;
	  (void) memcpy( dark->pmd_stray, dark_pntr, nr_byte );
	  dark_pntr += nr_byte;
	  (void) memcpy( dark->pmd_stray_error, dark_pntr, nr_byte );
	  dark_pntr += nr_byte;
/*
 * check if we read the whole DSR
 */
	  if ( (size_t)(dark_pntr - dark_char) != dsr_size )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, dsd_name );
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_DARK( dark );
#endif
	  dark++;
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
/*
 * set return values
 */
     dark_pntr = NULL;
 done:
     if ( dark_char != NULL ) free( dark_char );
     return nr_dsr;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_DARK
.PURPOSE     write average of the Dark Measurements per State
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_DARK( fd, num_dark, dark );
     input:
            FILE *fd               :   stream pointer
	    unsigned int num_dark  :   number of Dark records
            struct dark_scia *dark :  average of the dark measurements

.RETURNS     nothing
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1_WR_DARK( FILE *fd, unsigned int num_dark,
		       const struct dark_scia *dark_in )
{
     const char prognm[] = "SCIA_LV1_WR_DARK";

     size_t nr_byte;

     struct dark_scia dark;

     struct dsd_envi dsd = {
          "DARK_AVERAGE", "A",
          "                                                              ",
          0u, 0u, 0u, 0
     };

     if ( num_dark == 0u ) {
	  (void) strcpy( dsd.flname, "NOT USED" );
	  SCIA_LV1_ADD_DSD( &dsd );
	  return;
     }
/*
 * write data set records
 */
     do {
          (void) memcpy( &dark, dark_in, sizeof( struct dark_scia ) );
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_DARK( &dark );
#endif
/*
 * write DARK structures to file
 */
	  if ( fwrite( &dark.mjd.days, ENVI_INT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_INT;
	  if ( fwrite( &dark.mjd.secnd, ENVI_UINT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UINT;
	  if ( fwrite( &dark.mjd.musec, ENVI_UINT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UINT;
	  if ( fwrite( &dark.flag_mds, ENVI_UCHAR, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UCHAR;

	  nr_byte = (size_t) (SCIENCE_PIXELS) * ENVI_FLOAT;
	  if ( fwrite( dark.dark_spec, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( dark.sdev_dark_spec, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  nr_byte = (size_t) (2 * PMD_NUMBER) * ENVI_FLOAT;
	  if ( fwrite( dark.pmd_off, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( dark.pmd_off_error, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  nr_byte = (size_t) (SCIENCE_PIXELS) * ENVI_FLOAT;
	  if ( fwrite( dark.sol_stray, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( dark.sol_stray_error, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  nr_byte = (size_t) PMD_NUMBER * ENVI_FLOAT;
	  if ( fwrite( dark.pmd_stray, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( dark.pmd_stray_error, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;

	  dark_in++;
     } while ( ++dsd.num_dsr < num_dark );
/*
 * update list of written DSD records
 */
     dsd.dsr_size = (int) (dsd.size / dsd.num_dsr);
     SCIA_LV1_ADD_DSD( &dsd );
}
