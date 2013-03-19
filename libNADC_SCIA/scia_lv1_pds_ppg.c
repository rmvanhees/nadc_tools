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

.IDENTifer   SCIA_LV1_PDS_PPG
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 data
.LANGUAGE    ANSI C
.PURPOSE     read/write PPG/Etalon Parameters records
.COMMENTS    contains SCIA_LV1_RD_PPG and SCIA_LV1_WR_PPG
.ENVIRONment None
.VERSION      4.0   11-Oct-2005 use direct write, add usage of SCIA_LV1_ADD_DSD
              3.0   15-Apr-2005 added routine to write PPG-struct to file, RvH
              2.3   17-Jul-2002	there is only one PPG-struct, RvH
              2.2   22-Mar-2002	test number of DSD; can be zero, RvH 
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
void Sun2Intel_PPG( struct ppg_scia *ppg )
{
     register int ni = 0;

     do {
	  IEEE_Swap__FLT( &ppg->ppg_fact[ni] );
	  IEEE_Swap__FLT( &ppg->etalon_fact[ni] );
	  IEEE_Swap__FLT( &ppg->etalon_resid[ni] );
	  IEEE_Swap__FLT( &ppg->wls_deg_fact[ni] );
     } while ( ++ni < (SCIENCE_PIXELS) );
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_PPG
.PURPOSE     read PPG/Etalon Parameters records
.INPUT/OUTPUT
  call as   nbyte = SCIA_LV1_RD_PPG( fd, num_dsd, dsd, &ppg );
     input:
            FILE *fd              :   (open) stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi dsd   :   structure for the DSDs
    output:
            struct ppg_scia *ppg  :  PPG/Etalon Parameters

.RETURNS     number of data set records read (unsigned int)
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV1_RD_PPG( FILE *fd, unsigned int num_dsd, 
			      const struct dsd_envi *dsd,
			      struct ppg_scia *ppg )
{
     const char prognm[]   = "SCIA_LV1_RD_PPG";

     char         *ppg_char, *ppg_pntr;
     size_t       dsr_size, nr_byte;
     unsigned int indx_dsd;

     const char dsd_name[] = "PPG_ETALON";
/*
 * get index to data set descriptor
 */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL ) {
	  NADC_ERROR( prognm, NADC_ERR_PDS_RD, dsd_name );
	  return 0u;
     }
     if ( dsd[indx_dsd].num_dsr == 0 ) return 0;
/*
 * allocate memory to temporary store data for output structure
 */
     dsr_size = (size_t) dsd[indx_dsd].dsr_size;
     if ( (ppg_char = (char *) malloc( dsr_size )) == NULL ) {
	  NADC_ERROR( prognm, NADC_ERR_ALLOC, "ppg_char" );
	  return 0u;
     }
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
     if ( fread( ppg_char, dsr_size, 1, fd ) != 1 ) {
	  free( ppg_char );
	  NADC_ERROR( prognm, NADC_ERR_PDS_RD, "" );
	  return 0u;
     }
     ppg_pntr = ppg_char;
/*
 * read data buffer to PPG structure
 */
     nr_byte = (size_t) (SCIENCE_PIXELS) * ENVI_FLOAT;
     (void) memcpy( &ppg->ppg_fact, ppg_pntr, nr_byte );
     ppg_pntr += nr_byte;
     (void) memcpy( &ppg->etalon_fact, ppg_pntr, nr_byte );
     ppg_pntr += nr_byte;
     (void) memcpy( &ppg->etalon_resid, ppg_pntr, nr_byte );
     ppg_pntr += nr_byte;
     (void) memcpy( &ppg->wls_deg_fact, ppg_pntr, nr_byte );
     ppg_pntr += nr_byte;
     nr_byte = (size_t) (SCIENCE_PIXELS) * ENVI_UCHAR;
     (void) memcpy( &ppg->bad_pixel, ppg_pntr, nr_byte );
     ppg_pntr += nr_byte;
/*
 * check if we read the whole DSR
 */
     if ( (size_t)(ppg_pntr - ppg_char) != dsr_size ) {
	  free( ppg_char );
	  NADC_ERROR( prognm, NADC_ERR_PDS_SIZE, dsd_name );
	  return 0u;
     }
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_PPG( ppg );
#endif
     ppg_pntr = NULL;
     free( ppg_char );
/*
 * set return values
 */
     return 1u;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_PPG
.PURPOSE     write PPG/Etalon Parameters records
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_PPG( fd, num_ppg, ppg );
     input:
            FILE *fd              :  (open) stream pointer
	    unsigned int num_ppg  :  number of PPG records
            struct ppg_scia ppg   :  PPG/Etalon Parameters

.RETURNS     number of data set records read (unsigned int)
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1_WR_PPG( FILE *fd, unsigned int num_ppg, 
		      const struct ppg_scia ppg_in )
{
     const char prognm[] = "SCIA_LV1_WR_PPG(";

     size_t nr_byte;

     struct ppg_scia ppg;

     struct dsd_envi dsd = {
	  "PPG_ETALON", "G",
	  "                                                              ",
	  0u, 0u, 0u, 0
     };

     if ( num_ppg == 0u ) {
	  (void) strcpy( dsd.flname, "NOT USED" );
	  SCIA_LV1_ADD_DSD( &dsd );
	  return;
     }
/*
 * copy PPG structure to data buffer
 */
     (void) memcpy( &ppg, &ppg_in, sizeof( struct ppg_scia ) );
#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_PPG( &ppg );
#endif
     nr_byte = (size_t) (SCIENCE_PIXELS * ENVI_FLOAT);
     if ( fwrite( &ppg.ppg_fact, nr_byte, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
     if ( fwrite( &ppg.etalon_fact, nr_byte, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
     if ( fwrite( &ppg.etalon_resid, nr_byte, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
     if ( fwrite( &ppg.wls_deg_fact, nr_byte, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
     nr_byte = (size_t) (SCIENCE_PIXELS * ENVI_UCHAR);
     if ( fwrite( &ppg.bad_pixel, nr_byte, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
/*
 * update list of written DSD records
 */
     dsd.num_dsr = 1u;
     dsd.dsr_size = (int) dsd.size;
     SCIA_LV1_ADD_DSD( &dsd );
}
