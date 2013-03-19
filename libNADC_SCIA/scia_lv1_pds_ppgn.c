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

.IDENTifer   SCIA_LV1_PDS_PPGN
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 data
.LANGUAGE    ANSI C
.PURPOSE     read/write PPG/Etalon Parameters (newly calculated)
.COMMENTS    contains SCIA_LV1_RD_PPGN and SCIA_LV1_WR_PPGN
.ENVIRONment None
.VERSION      4.0   11-Oct-2005 use direct write, add usage of SCIA_LV1_ADD_DSD
              3.0   15-Apr-2005 added routine to write PPGN-struct to file, RvH
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
void Sun2Intel_PPGN( struct ppgn_scia *ppgn )
{
     register int ni = 0;

     ppgn->mjd.days = byte_swap_32( ppgn->mjd.days );
     ppgn->mjd.secnd = byte_swap_u32( ppgn->mjd.secnd );
     ppgn->mjd.musec = byte_swap_u32( ppgn->mjd.musec );
     do {
	  IEEE_Swap__FLT( &ppgn->gain_fact[ni] );
	  IEEE_Swap__FLT( &ppgn->etalon_fact[ni] );
	  IEEE_Swap__FLT( &ppgn->etalon_resid[ni] );
	  IEEE_Swap__FLT( &ppgn->avg_wls_spec[ni] );
	  IEEE_Swap__FLT( &ppgn->sd_wls_spec[ni] );
     } while ( ++ni < (SCIENCE_PIXELS) );
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_PPGN
.PURPOSE     read PPG/Etalon Parameters (newly calculated)
.INPUT/OUTPUT
  call as   nr_dsr = SCIA_LV1_RD_PPGN( fd, num_dsd, dsd, &ppgn );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi *dsd  :   structure for the DSDs
    output:
            struct ppgn_scia **ppgn :  PPG/Etalon Parameters (new)

.RETURNS     number of data set records read (unsigned int)
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV1_RD_PPGN( FILE *fd, unsigned int num_dsd, 
			       const struct dsd_envi *dsd,
			       struct ppgn_scia **ppgn_out )
{
     const char prognm[] = "SCIA_LV1_RD_PPGN";

     char         *ppgn_pntr, *ppgn_char = NULL;
     size_t       dsr_size, nr_byte;
     unsigned int indx_dsd;

     unsigned int nr_dsr = 0;

     struct ppgn_scia *ppgn;

     const char dsd_name[] = "NEW_PPG_ETALON";
/*
 * get index to data set descriptor
 */
     NADC_ERR_SAVE();
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, dsd_name );
     if ( IS_ERR_STAT_ABSENT || dsd[indx_dsd].num_dsr == 0 ) {
	  NADC_ERR_RESTORE();
          ppgn_out[0] = NULL;
	  return 0u;
     }
     dsr_size = (size_t) dsd[indx_dsd].dsr_size;
/*
 * allocate memory to store output structures
 */
     if ( ! Use_Extern_Alloc ) {
	  ppgn_out[0] = (struct ppgn_scia *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct ppgn_scia));
     }
     if ( (ppgn = ppgn_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "ppgn" );
/*
 * allocate memory to temporary store data for output structure
 */
     if ( (ppgn_char = (char *) malloc( dsr_size )) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "ppgn_char" );
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
/*
 * read data set records
 */
     do {
	  if ( fread( ppgn_char, dsr_size, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
/*
 * read data buffer to PPGN structure
 */
	  (void) memcpy( &ppgn->mjd.days, ppgn_char, ENVI_INT );
	  ppgn_pntr = ppgn_char + ENVI_INT;
	  (void) memcpy( &ppgn->mjd.secnd, ppgn_pntr, ENVI_UINT );
	  ppgn_pntr += ENVI_UINT;
	  (void) memcpy( &ppgn->mjd.musec, ppgn_pntr, ENVI_UINT );
	  ppgn_pntr += ENVI_UINT;
	  (void) memcpy( &ppgn->flag_mds, ppgn_pntr, ENVI_UCHAR );
	  ppgn_pntr += ENVI_UCHAR;

	  nr_byte = (size_t) (SCIENCE_PIXELS) * ENVI_FLOAT;
	  (void) memcpy( ppgn->gain_fact, ppgn_pntr, nr_byte );
	  ppgn_pntr += nr_byte;
	  (void) memcpy( ppgn->etalon_fact, ppgn_pntr, nr_byte );
	  ppgn_pntr += nr_byte;
	  (void) memcpy( ppgn->etalon_resid, ppgn_pntr, nr_byte );
	  ppgn_pntr += nr_byte;
	  (void) memcpy( ppgn->avg_wls_spec, ppgn_pntr, nr_byte );
	  ppgn_pntr += nr_byte;
	  (void) memcpy( ppgn->sd_wls_spec, ppgn_pntr, nr_byte );
	  ppgn_pntr += nr_byte;

	  nr_byte = (size_t) (SCIENCE_PIXELS) * ENVI_UCHAR;
	  (void) memcpy( ppgn->bad_pixel, ppgn_pntr, nr_byte );
	  ppgn_pntr += nr_byte;
/*
 * check if we read the whole DSR
 */
	  if ( (size_t)(ppgn_pntr - ppgn_char) != dsr_size )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, dsd_name );
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_PPGN( ppgn );
#endif
	  ppgn++;
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
/*
 * set return values
 */
     ppgn_pntr = NULL;
 done:
     if ( ppgn_char != NULL ) free( ppgn_char );
     return nr_dsr;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_PPGN
.PURPOSE     write PPG/Etalon Parameters (newly calculated)
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_PPGN( fd, num_dsd, dsd, ppgn );
     input:
            FILE *fd               :  stream pointer
	    unsigned int num_ppgn  :  number of PPGN records
            struct ppgn_scia *ppgn :  PPG/Etalon Parameters (new)

.RETURNS     nothing
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1_WR_PPGN( FILE *fd, unsigned int num_ppgn,
		       const struct ppgn_scia *ppgn_in )
{
     const char prognm[] = "SCIA_LV1_WR_PPGN";

     size_t nr_byte;

     struct ppgn_scia ppgn;

     struct dsd_envi dsd = {
          "NEW_PPG_ETALON", "A",
          "                                                              ",
          0u, 0u, 0u, 0
     };

     if ( num_ppgn == 0u ) {
	  (void) strcpy( dsd.flname, "NOT USED" );
	  SCIA_LV1_ADD_DSD( &dsd );
	  return;
     }
/*
 * write data set records
 */
     do {
          (void) memcpy( &ppgn, ppgn_in, sizeof( struct ppgn_scia ) );
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_PPGN( &ppgn );
#endif
/*
 * write PPGN structure to file
 */
	  if ( fwrite( &ppgn.mjd.days, ENVI_INT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_INT;
	  if ( fwrite( &ppgn.mjd.secnd, ENVI_UINT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UINT;
	  if ( fwrite( &ppgn.mjd.musec, ENVI_UINT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UINT;
	  if ( fwrite( &ppgn.flag_mds, ENVI_UCHAR, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UCHAR;

	  nr_byte = (size_t) (SCIENCE_PIXELS) * ENVI_FLOAT;
	  if ( fwrite( ppgn.gain_fact, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( ppgn.etalon_fact, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( ppgn.etalon_resid, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( ppgn.avg_wls_spec, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( ppgn.sd_wls_spec, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;

	  nr_byte = (size_t) (SCIENCE_PIXELS) * ENVI_UCHAR;
	  if ( fwrite( ppgn.bad_pixel, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;

	  ppgn_in++;
     } while ( ++dsd.num_dsr < num_ppgn );
/*
 * update list of written DSD records
 */
     dsd.dsr_size = (int) (dsd.size / dsd.num_dsr);
     SCIA_LV1_ADD_DSD( &dsd );
}
