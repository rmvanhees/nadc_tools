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

.IDENTifer   SCIA_LV1_PDS_SRS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 data
.LANGUAGE    ANSI C
.PURPOSE     read/write Sun Reference Spectrum records
.COMMENTS    contains SCIA_LV1_RD_SRS and SCIA_LV1_WR_SRS
.ENVIRONment None
.VERSION      4.0   11-Oct-2005 minor bugfix, add usage of SCIA_LV1_ADD_DSD,RvH
              3.0   18-Apr-2005 added routine to write SRS-struct to file, RvH
              2.2   22-Mar-2002	test number of DSD; can be zero, RvH
              2.1   16-Jan-2002	use of global Use_Extern_Alloc, RvH 
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              1.1   04-Oct-2001	changed input/output, RvH 
              1.0   09-Nov-1999 created by R. M. van Hees
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

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#ifdef _SWAP_TO_LITTLE_ENDIAN
#include <swap_bytes.h>

static
void Sun2Intel_SRS( struct srs_scia *srs )
{
     register int ni;

     IEEE_Swap__FLT( &srs->avg_asm );
     IEEE_Swap__FLT( &srs->avg_esm );
     IEEE_Swap__FLT( &srs->avg_elev_sun );
     IEEE_Swap__FLT( &srs->dopp_shift );
     for ( ni = 0; ni < (SCIENCE_PIXELS); ni++ ) {
	  IEEE_Swap__FLT( &srs->mean_sun[ni] );
	  IEEE_Swap__FLT( &srs->precision_sun[ni] );
	  IEEE_Swap__FLT( &srs->wvlen_sun[ni] );
	  IEEE_Swap__FLT( &srs->accuracy_sun[ni] );
	  IEEE_Swap__FLT( &srs->etalon[ni] );
     }
     for ( ni = 0; ni < PMD_NUMBER; ni++ ) {
	  IEEE_Swap__FLT( &srs->pmd_mean[ni] );
	  IEEE_Swap__FLT( &srs->pmd_out_nd_out[ni] );
	  IEEE_Swap__FLT( &srs->pmd_out_nd_in[ni] );
     }
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_SRS
.PURPOSE     read Sun Reference Spectrum records
.INPUT/OUTPUT
  call as   nr_dsr = SCIA_LV1_RD_SRS( fd, num_dsd, dsd, &srs );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi dsd   :   structure for the DSDs
    output:
            struct srs_scia **srs :  Sun reference spectrum

.RETURNS     number of data set records read (unsigned int)
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV1_RD_SRS( FILE *fd, unsigned int num_dsd, 
			      const struct dsd_envi *dsd,
			      struct srs_scia **srs_out )
{
     const char prognm[] = "SCIA_LV1_RD_SRS";

     char         *srs_pntr, *srs_char = NULL;
     size_t       dsr_size, nr_byte;
     unsigned int indx_dsd;

     unsigned int nr_dsr = 0;

     struct srs_scia *srs;

     const char dsd_name[] = "SUN_REFERENCE";

     if ( ! Use_Extern_Alloc ) srs_out[0] = NULL;
/*
 * get index to data set descriptor
 */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL ) {
	  NADC_ERROR( prognm, NADC_ERR_PDS_RD, dsd_name );
	  return 0;
     }
     if ( dsd[indx_dsd].num_dsr == 0 ) return 0;

     if ( ! Use_Extern_Alloc ) {
	  srs = (struct srs_scia *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct srs_scia));
	  if ( srs == NULL ) {
	       NADC_ERROR( prognm, NADC_ERR_ALLOC, "srs" );
	       return 0;
	  }
     } else if ( (srs = srs_out[0]) == NULL ) {
	  NADC_ERROR( prognm, NADC_ERR_ALLOC, "srs_out[0]" );
	  return 0;
     }
/*
 * allocate memory to temporary store data for output structure
 */
     dsr_size = (size_t) dsd[indx_dsd].dsr_size;
     if ( (srs_char = (char *) malloc( dsr_size )) == NULL ) {
	  NADC_ERROR( prognm, NADC_ERR_ALLOC, "srs_char" );
	  return 0;
     }
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
/*
 * read data set records
 */
     do {
	  if ( fread( srs_char, dsr_size, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
	  srs_pntr = srs_char;
/*
 * read data buffer to SRS structure
 */
	  (void) nadc_strlcpy( srs[nr_dsr].sun_spec_id, srs_pntr, 3 );
	  srs_pntr += 2;
	  nr_byte = (size_t) (SCIENCE_PIXELS) * ENVI_FLOAT;
	  (void) memcpy( srs[nr_dsr].wvlen_sun, srs_pntr, nr_byte );
	  srs_pntr += nr_byte;
	  (void) memcpy( srs[nr_dsr].mean_sun, srs_pntr, nr_byte );
	  srs_pntr += nr_byte;
	  (void) memcpy( srs[nr_dsr].precision_sun, srs_pntr, nr_byte );
	  srs_pntr += nr_byte;
	  (void) memcpy( srs[nr_dsr].accuracy_sun, srs_pntr, nr_byte );
	  srs_pntr += nr_byte;
	  (void) memcpy( srs[nr_dsr].etalon, srs_pntr, nr_byte );
	  srs_pntr += nr_byte;
	  (void) memcpy( &srs[nr_dsr].avg_asm, srs_pntr, ENVI_FLOAT );
	  srs_pntr += ENVI_FLOAT;
	  (void) memcpy( &srs[nr_dsr].avg_esm, srs_pntr, ENVI_FLOAT );
	  srs_pntr += ENVI_FLOAT;
	  (void) memcpy( &srs[nr_dsr].avg_elev_sun, srs_pntr, ENVI_FLOAT);
	  srs_pntr += ENVI_FLOAT;
	  nr_byte = (size_t) PMD_NUMBER * ENVI_FLOAT;
	  (void) memcpy( srs[nr_dsr].pmd_mean, srs_pntr, nr_byte );
	  srs_pntr += nr_byte;
	  (void) memcpy( srs[nr_dsr].pmd_out_nd_out, srs_pntr, nr_byte );
	  srs_pntr += nr_byte;
	  (void) memcpy( srs[nr_dsr].pmd_out_nd_in, srs_pntr, nr_byte );
	  srs_pntr += nr_byte;
	  (void) memcpy( &srs[nr_dsr].dopp_shift, srs_pntr, ENVI_FLOAT );
	  srs_pntr += ENVI_FLOAT;
/*
 * check if we read the whole DSR
 */
	  if ( (size_t)(srs_pntr - srs_char) != dsr_size ) {
	       free( srs_char );
	       if ( ! Use_Extern_Alloc ) free( srs );
	       NADC_ERROR( prognm, NADC_ERR_PDS_SIZE, dsd_name );
	       return 0;
	  }
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_SRS( srs+nr_dsr );
#endif
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
/*
 * set return values
 */
     srs_pntr = NULL;
     srs_out[0] = srs;
done:
     if ( srs_char != NULL ) free( srs_char );
     return nr_dsr;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_SRS
.PURPOSE     write Sun Reference Spectrum records
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_SRS( fd, num_srs, srs );
     input:
            FILE *fd             :   stream pointer
	    unsigned int num_srs :   number of SRS records
            struct srs_scia *srs :   Sun reference spectrum

.RETURNS     nothing
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1_WR_SRS( FILE *fd, unsigned int num_srs, 
		      const struct srs_scia *srs_in )
{
     const char prognm[] = "SCIA_LV1_WR_SRS";

     size_t nr_byte;

     struct srs_scia srs;

     struct dsd_envi dsd = {
          "SUN_REFERENCE", "G",
          "                                                              ",
          0u, 0u, 0u, 0
     };

     if ( num_srs == 0u ) {
	  (void) strcpy( dsd.flname, "NOT USED" );
	  SCIA_LV1_ADD_DSD( &dsd );
	  return;
     }
/*
 * write data set records
 */
     do {
	  (void) memcpy( &srs, srs_in, sizeof( struct srs_scia ));
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_SRS( &srs );
#endif
/*
 * write SRS structure to data buffer
 */
	  if ( fwrite( srs.sun_spec_id, 2 * ENVI_CHAR, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += 2 * ENVI_CHAR;
	  nr_byte = (size_t) (SCIENCE_PIXELS) * ENVI_FLOAT;
	  if ( fwrite( srs.wvlen_sun, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( srs.mean_sun, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( srs.precision_sun, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( srs.accuracy_sun, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( srs.etalon, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( &srs.avg_asm, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_FLOAT;
	  if ( fwrite( &srs.avg_esm, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_FLOAT;
	  if ( fwrite( &srs.avg_elev_sun, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_FLOAT;
	  nr_byte = (size_t) PMD_NUMBER * ENVI_FLOAT;
	  if ( fwrite( srs.pmd_mean, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( srs.pmd_out_nd_out, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( srs.pmd_out_nd_in, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( &srs.dopp_shift, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_FLOAT;

	  srs_in++;
     } while ( ++dsd.num_dsr < num_srs );
/*
 * update list of written DSD records
 */
     dsd.dsr_size = (int) (dsd.size / dsd.num_dsr);
     SCIA_LV1_ADD_DSD( &dsd );
}
