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

.IDENTifer   SCIA_LV1_PDS_SRSN
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 data
.LANGUAGE    ANSI C
.PURPOSE     read/write Sun Reference Spectrum (newly calculated)
.COMMENTS    contains SCIA_LV1_RD_SRSN and SCIA_LV1_WR_SRSN
.ENVIRONment None
.VERSION      4.0   11-Oct-2005 use direct write, add usage of SCIA_LV1_ADD_DSD
              3.0   18-Apr-2005 added routine to write SRSN-struct to file, RvH
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
void Sun2Intel_SRSN( struct srsn_scia *srsn )
{
     register int ni;

     srsn->mjd.days = byte_swap_32( srsn->mjd.days );
     srsn->mjd.secnd = byte_swap_u32( srsn->mjd.secnd );
     srsn->mjd.musec = byte_swap_u32( srsn->mjd.musec );

     IEEE_Swap__FLT( &srsn->avg_asm );
     IEEE_Swap__FLT( &srsn->avg_esm );
     IEEE_Swap__FLT( &srsn->avg_elev_sun );
     IEEE_Swap__FLT( &srsn->dopp_shift );
     for ( ni = 0; ni < (SCIENCE_PIXELS); ni++ ) {
	  IEEE_Swap__FLT( &srsn->mean_sun[ni] );
	  IEEE_Swap__FLT( &srsn->precision_sun[ni] );
	  IEEE_Swap__FLT( &srsn->wvlen_sun[ni] );
	  IEEE_Swap__FLT( &srsn->accuracy_sun[ni] );
	  IEEE_Swap__FLT( &srsn->etalon[ni] );
     }
     for ( ni = 0; ni < PMD_NUMBER; ni++ ) {
	  IEEE_Swap__FLT( &srsn->pmd_mean[ni] );
	  IEEE_Swap__FLT( &srsn->pmd_out[ni] );
     }
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_SRSN
.PURPOSE     read Sun Reference Spectrum (newly calculated)
.INPUT/OUTPUT
  call as   nr_dsr = SCIA_LV1_RD_SRSN( fd, num_dsd, dsd, &srsn );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi *dsd  :   structure for the DSDs
    output:
            struct srsn_scia **srsn :  Sun reference spectrum (new)

.RETURNS     number of data set records read (unsigned int)
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV1_RD_SRSN( FILE *fd, unsigned int num_dsd, 
			       const struct dsd_envi *dsd,
			       struct srsn_scia **srsn_out )
{
     const char prognm[] = "SCIA_LV1_RD_SRSN";

     char         *srsn_pntr, *srsn_char = NULL;
     size_t       dsr_size, nr_byte;
     unsigned int indx_dsd;

     unsigned int nr_dsr = 0;

     struct srsn_scia *srsn;
     
     const char dsd_name[] = "NEW_SUN_REFERENCE";
/*
 * get index to data set descriptor
 */
     NADC_ERR_SAVE();
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, dsd_name );
     if ( IS_ERR_STAT_ABSENT || dsd[indx_dsd].num_dsr == 0 ) {
	  NADC_ERR_RESTORE();
          srsn_out[0] = NULL;
	  return 0u;
     }
/*
 * allocate memory to store output structures
 */
     if ( ! Use_Extern_Alloc ) {
	  srsn_out[0] = (struct srsn_scia *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct srsn_scia));
     }
     if ( (srsn = srsn_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "srsn" );
/*
 * allocate memory to temporary store data for output structure
 */
     dsr_size = (size_t) dsd[indx_dsd].dsr_size;
     if ( (srsn_char = (char *) malloc( dsr_size )) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "srsn_char" );
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
/*
 * read data set records
 */
     do {
	  if ( fread( srsn_char, dsr_size, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
	  srsn_pntr = srsn_char;
/*
 * read data buffer to SRSN structure
 */
	  (void) memcpy( &srsn->mjd.days, srsn_pntr, ENVI_INT );
	  srsn_pntr += ENVI_INT;
	  (void) memcpy( &srsn->mjd.secnd, srsn_pntr, ENVI_UINT );
	  srsn_pntr += ENVI_UINT;
	  (void) memcpy( &srsn->mjd.musec, srsn_pntr, ENVI_UINT );
	  srsn_pntr += ENVI_UINT;
	  (void) memcpy( &srsn->flag_mds, srsn_pntr, ENVI_UCHAR );
	  srsn_pntr += ENVI_UCHAR;
	  (void) strlcpy( srsn->sun_spec_id, srsn_pntr, 3 );
	  srsn_pntr += 2;
	  (void) memcpy( &srsn->flag_neu, srsn_pntr, ENVI_UCHAR );
	  srsn_pntr += ENVI_UCHAR;
	  nr_byte = (size_t) (SCIENCE_PIXELS) * ENVI_FLOAT;
	  (void) memcpy( srsn->wvlen_sun, srsn_pntr, nr_byte );
	  srsn_pntr += nr_byte;
	  (void) memcpy( srsn->mean_sun, srsn_pntr, nr_byte );
	  srsn_pntr += nr_byte;
	  (void) memcpy( srsn->precision_sun, srsn_pntr, nr_byte );
	  srsn_pntr += nr_byte;
	  (void) memcpy( srsn->accuracy_sun, srsn_pntr, nr_byte );
	  srsn_pntr += nr_byte;
	  (void) memcpy( srsn->etalon, srsn_pntr, nr_byte );
	  srsn_pntr += nr_byte;
	  (void) memcpy( &srsn->avg_asm, srsn_pntr, ENVI_FLOAT );
	  srsn_pntr += ENVI_FLOAT;
	  (void) memcpy( &srsn->avg_esm, srsn_pntr, ENVI_FLOAT );
	  srsn_pntr += ENVI_FLOAT;
	  (void) memcpy( &srsn->avg_elev_sun, srsn_pntr, ENVI_FLOAT );
	  srsn_pntr += ENVI_FLOAT;
	  nr_byte = (size_t) PMD_NUMBER * ENVI_FLOAT;
	  (void) memcpy( srsn->pmd_mean, srsn_pntr, nr_byte );
	  srsn_pntr += nr_byte;
	  (void) memcpy( srsn->pmd_out, srsn_pntr, nr_byte );
	  srsn_pntr += nr_byte;
	  (void) memcpy( &srsn->dopp_shift, srsn_pntr, ENVI_FLOAT );
	  srsn_pntr += ENVI_FLOAT;
/*
 * check if we read the whole DSR
 */
	  if ( (size_t)(srsn_pntr - srsn_char) != dsr_size )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, dsd_name );
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_SRSN( srsn );
#endif
	  srsn++;
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
/*
 * set return values
 */
     srsn_pntr = NULL;
 done:
     if ( srsn_char != NULL ) free( srsn_char );
     return nr_dsr;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_SRSN
.PURPOSE     write Sun Reference Spectrum (newly calculated)
.INPUT/OUTPUT
  call as   nr_dsr = SCIA_LV1_WR_SRSN( fd, num_dsd, dsd, srsn );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi *dsd  :   structure for the DSDs
            struct srsn_scia *srsn :  Sun reference spectrum (new)

.RETURNS     number of data set records written (unsigned int)
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1_WR_SRSN( FILE *fd, unsigned int num_srsn,
		       const struct srsn_scia *srsn_in )
{
     const char prognm[] = "SCIA_LV1_WR_SRSN";

     size_t nr_byte;

     struct srsn_scia srsn;
     
     struct dsd_envi dsd = {
          "NEW_SUN_REFERENCE", "A",
          "                                                              ",
          0u, 0u, 0u, 0
     };

     if ( num_srsn == 0u ) {
	  (void) strcpy( dsd.flname, "NOT USED" );
	  SCIA_LV1_ADD_DSD( &dsd );
	  return;
     }
/*
 * write data set records
 */
     do {
	  (void) memcpy( &srsn, srsn_in, sizeof( struct srsn_scia ));
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_SRSN( &srsn );
#endif
/*
 * write SRSN structure to file
 */
	  if ( fwrite( &srsn.mjd.days, ENVI_INT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_INT;
	  if ( fwrite( &srsn.mjd.secnd, ENVI_UINT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UINT;
	  if ( fwrite( &srsn.mjd.musec, ENVI_UINT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UINT;
	  if ( fwrite( &srsn.flag_mds, ENVI_UCHAR, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UCHAR;
	  if ( fwrite( srsn.sun_spec_id, 2 * ENVI_CHAR, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += 2 * ENVI_CHAR;
	  if ( fwrite( &srsn.flag_neu, ENVI_UCHAR, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UCHAR;
	  nr_byte = (size_t) (SCIENCE_PIXELS * ENVI_FLOAT);
	  if ( fwrite( srsn.wvlen_sun, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( srsn.mean_sun, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( srsn.precision_sun, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( srsn.accuracy_sun, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( srsn.etalon, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( &srsn.avg_asm, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_FLOAT;
	  if ( fwrite( &srsn.avg_esm, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_FLOAT;
	  if ( fwrite( &srsn.avg_elev_sun, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_FLOAT;
	  nr_byte = (size_t) (PMD_NUMBER * ENVI_FLOAT);
	  if ( fwrite( srsn.pmd_mean, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( srsn.pmd_out, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( &srsn.dopp_shift, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_FLOAT;

	  srsn_in++;
     } while ( ++dsd.num_dsr < num_srsn );
/*
 * update list of written DSD records
 */
     dsd.dsr_size = (int) (dsd.size / dsd.num_dsr);
     SCIA_LV1_ADD_DSD( &dsd );
}
