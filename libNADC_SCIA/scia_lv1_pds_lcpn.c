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

.IDENTifer   SCIA_LV1_PDS_LCPN
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 data
.LANGUAGE    ANSI C
.PURPOSE     read/write Leakage Current Parameters (newly calculated)
.COMMENTS    contains SCIA_LV1_RD_LCPN and SCIA_LV1_WR_LCPN
.ENVIRONment None
.VERSION      4.0   11-Oct-2005 use direct write, add usage of SCIA_LV1_ADD_DSD
              3.0   15-Apr-2005 added routine to write LCPN to file, RvH
              2.3   22-Mar-2002	test number of DSD; can be zero, RvH
              2.2   06-Mar-2002	added check for presence of this dataset, RvH 
              2.1   16-Jan-2002	use of global Use_Extern_Alloc, RvH 
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              1.1   04-Oct-2001	changed input/output, RvH 
              1.0   09-Nov-1999 created by R. M. van Hees
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
void Sun2Intel_LCPN( struct lcpn_scia *lcpn )
{
     register int ni;

     lcpn->mjd.days = byte_swap_32( lcpn->mjd.days );
     lcpn->mjd.secnd = byte_swap_u32( lcpn->mjd.secnd );
     lcpn->mjd.musec = byte_swap_u32( lcpn->mjd.musec );

     lcpn->mjd_last.days = byte_swap_32( lcpn->mjd_last.days );
     lcpn->mjd_last.secnd = byte_swap_u32( lcpn->mjd_last.secnd );
     lcpn->mjd_last.musec = byte_swap_u32( lcpn->mjd_last.musec );

     IEEE_Swap__FLT( &lcpn->orbit_phase );
     for ( ni = 0; ni < (SCIENCE_PIXELS); ni++ ) {
	  IEEE_Swap__FLT( &lcpn->fpn[ni] );
	  IEEE_Swap__FLT( &lcpn->fpn_error[ni] );
	  IEEE_Swap__FLT( &lcpn->lc[ni] );
	  IEEE_Swap__FLT( &lcpn->lc_error[ni] );
	  IEEE_Swap__FLT( &lcpn->mean_noise[ni] );
     }
     for ( ni = 0; ni < (2 * PMD_NUMBER); ni++ ) {
	  IEEE_Swap__FLT( &lcpn->pmd_off[ni] );
	  IEEE_Swap__FLT( &lcpn->pmd_off_error[ni] );
     }
     for ( ni = 0; ni < (IR_CHANNELS + PMD_NUMBER); ni++ ) {
	  IEEE_Swap__FLT( &lcpn->obm_pmd[ni] );
     }
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_LCPN
.PURPOSE     read Leakage Current Parameters (newly calculated partial set)
.INPUT/OUTPUT
  call as   nr_dsr = SCIA_LV1_RD_LCPN( fd, num_dsd, dsd, &lcpn );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi *dsd  :   structure for the DSDs
    output:
            struct lcpn_scia **lcpn :  leakage current parameters

.RETURNS     number of data set records read (unsigned int)
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV1_RD_LCPN( FILE *fd, unsigned int num_dsd, 
			       const struct dsd_envi *dsd,
			       struct lcpn_scia **lcpn_out )
{
     const char prognm[] = "SCIA_LV1_RD_LCPN";

     char         *lcpn_pntr, *lcpn_char = NULL;
     size_t       dsr_size, nr_byte;
     unsigned int indx_dsd;

     unsigned int nr_dsr = 0;

     struct lcpn_scia *lcpn;

     const char dsd_name[] = "NEW_LEAKAGE";
/*
 * get index to data set descriptor
 */
     NADC_ERR_SAVE();
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, dsd_name );
     if ( IS_ERR_STAT_ABSENT || dsd[indx_dsd].num_dsr == 0 ) {
	  NADC_ERR_RESTORE();
          lcpn_out[0] = NULL;
	  return 0u;
     }
     dsr_size = (size_t) dsd[indx_dsd].dsr_size;
/*
 * allocate memory to store output structures
 */
     if ( ! Use_Extern_Alloc ) {
	  lcpn_out[0] = (struct lcpn_scia *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct lcpn_scia));
     }
     if ( (lcpn = lcpn_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "lcpn" );
/*
 * allocate memory to temporary store data for output structure
 */
     if ( (lcpn_char = (char *) malloc( dsr_size )) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "lcpn_char" );
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
/*
 * read data set records
 */
     do {
	  if ( fread( lcpn_char, dsr_size, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
/*
 * read data buffer to LCPN structure
 */
	  (void) memcpy( &lcpn->mjd.days, lcpn_char, ENVI_INT );
	  lcpn_pntr = lcpn_char + ENVI_INT;
	  (void) memcpy( &lcpn->mjd.secnd, lcpn_pntr, ENVI_UINT );
	  lcpn_pntr += ENVI_UINT;
	  (void) memcpy( &lcpn->mjd.musec, lcpn_pntr, ENVI_UINT );
	  lcpn_pntr += ENVI_UINT;
	  (void) memcpy( &lcpn->flag_mds, lcpn_pntr, ENVI_UCHAR );
	  lcpn_pntr += ENVI_UCHAR;
	  (void) memcpy( &lcpn->mjd_last.days, lcpn_pntr, ENVI_INT );
	  lcpn_pntr += ENVI_INT;
/*
 * BUG in the test dataset version 1.0
 */
	  lcpn->mjd_last.days = 0;                   /* remove this line!!! */
	  (void) memcpy( &lcpn->mjd_last.secnd, lcpn_pntr, ENVI_UINT );
	  lcpn_pntr += ENVI_UINT;
	  (void) memcpy( &lcpn->mjd_last.musec, lcpn_pntr, ENVI_UINT );
	  lcpn_pntr += ENVI_UINT;
	  (void) memcpy( &lcpn->orbit_phase, lcpn_pntr, ENVI_FLOAT );
	  lcpn_pntr += ENVI_FLOAT;

	  nr_byte = (IR_CHANNELS + PMD_NUMBER) * ENVI_FLOAT;
	  (void) memcpy( lcpn->obm_pmd, lcpn_pntr, nr_byte );
	  lcpn_pntr += nr_byte;

	  nr_byte = (size_t) (SCIENCE_PIXELS) * ENVI_FLOAT;
	  (void) memcpy( lcpn->fpn, lcpn_pntr, nr_byte );
	  lcpn_pntr += nr_byte;
	  (void) memcpy( lcpn->fpn_error, lcpn_pntr, nr_byte );
	  lcpn_pntr += nr_byte;

	  (void) memcpy( lcpn->lc, lcpn_pntr, nr_byte );
	  lcpn_pntr += nr_byte;
	  (void) memcpy( lcpn->lc_error, lcpn_pntr, nr_byte );
	  lcpn_pntr += nr_byte;

	  (void) memcpy( lcpn->mean_noise, lcpn_pntr, nr_byte );
	  lcpn_pntr += nr_byte;

	  nr_byte = (size_t) (2 * PMD_NUMBER) * ENVI_FLOAT;
	  (void) memcpy( lcpn->pmd_off, lcpn_pntr, nr_byte );
	  lcpn_pntr += nr_byte;
	  (void) memcpy( lcpn->pmd_off_error, lcpn_pntr, nr_byte );
	  lcpn_pntr += nr_byte;
/*
 * check if we read the whole DSR
 */
	  if ( (size_t)(lcpn_pntr - lcpn_char) != dsr_size ) {
	       free( lcpn_char );
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, dsd_name );
	  }
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_LCPN( lcpn );
#endif
	  lcpn++;
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
/*
 * set return values
 */
     lcpn_pntr = NULL;
 done:
     if ( lcpn_char != NULL ) free( lcpn_char );
     return nr_dsr;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_LCPN
.PURPOSE     write Leakage Current Parameters (newly calculated partial set)
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_LCPN( fd, num_dsd, dsd, lcpn );
     input:
            FILE *fd               :   stream pointer
	    unsigned int num_lcpn  :   number of LCPN records
            struct lcpn_scia *lcpn :   leakage current parameters

.RETURNS     nothing
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1_WR_LCPN( FILE *fd, unsigned int num_lcpn,
		       const struct lcpn_scia *lcpn_in )
{
     const char prognm[] = "SCIA_LV1_WR_LCPN";
     size_t nr_byte;

     struct lcpn_scia lcpn;

     struct dsd_envi dsd = {
          "NEW_LEAKAGE", "A",
          "                                                              ",
          0u, 0u, 0u, 0
     };

     if ( num_lcpn == 0u ) {
	  (void) strcpy( dsd.flname, "NOT USED" );
	  SCIA_LV1_ADD_DSD( &dsd );
	  return;
     }
/*
 * write data set records
 */
     do {
	  (void) memcpy( &lcpn, lcpn_in, sizeof( struct lcpn_scia ) );
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_LCPN( &lcpn );
#endif
/*
 * read LCPN structures to file
 */
	  if ( fwrite( &lcpn.mjd.days, ENVI_INT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_INT;
	  if ( fwrite( &lcpn.mjd.secnd, ENVI_UINT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UINT;
	  if ( fwrite( &lcpn.mjd.musec, ENVI_UINT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UINT;
	  if ( fwrite( &lcpn.flag_mds, ENVI_UCHAR, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UCHAR;
	  if ( fwrite( &lcpn.mjd_last.days, ENVI_INT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_INT;
/*
 * BUG in the test dataset version 1.0
 */
	  if ( fwrite( &lcpn.mjd_last.secnd, ENVI_UINT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UINT;
	  if ( fwrite( &lcpn.mjd_last.musec, ENVI_UINT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UINT;
	  if ( fwrite( &lcpn.orbit_phase, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_FLOAT;

	  nr_byte = (IR_CHANNELS + PMD_NUMBER) * ENVI_FLOAT;
	  if ( fwrite( lcpn.obm_pmd, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;

	  nr_byte = (size_t) (SCIENCE_PIXELS) * ENVI_FLOAT;
	  if ( fwrite( lcpn.fpn, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( lcpn.fpn_error, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;

	  if ( fwrite( lcpn.lc, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( lcpn.lc_error, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;

	  if ( fwrite( lcpn.mean_noise, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;

	  nr_byte = (size_t) (2 * PMD_NUMBER) * ENVI_FLOAT;
	  if ( fwrite( lcpn.pmd_off, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( lcpn.pmd_off_error, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;

	  lcpn_in++;
     } while ( ++dsd.num_dsr < num_lcpn );
/*
 * update list of written DSD records
 */
     dsd.dsr_size = (int) (dsd.size / dsd.num_dsr);
     SCIA_LV1_ADD_DSD( &dsd );
}
