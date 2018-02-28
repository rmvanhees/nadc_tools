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

.IDENTifer   SCIA_LV1_PDS_ASFP
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 data
.LANGUAGE    ANSI C
.PURPOSE     read/write Small Aperture Slit function parameters
.COMMENTS    contains SCIA_LV1_RD_ASFP and SCIA_LV1_WR_ASFP
.ENVIRONment None
.VERSION      4.0   11-Oct-2005	use direct write, add usage of SCIA_LV1_ADD_DSD
              3.0   14-Apr-2005	added routine to write ASPF to file, RvH
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
#endif

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_ASFP
.PURPOSE     read Small Aperture Slit function parameters
.INPUT/OUTPUT
  call as   nr_dsr = SCIA_LV1_RD_ASFP( fd, num_dsd, dsd, &asfp );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi *dsd  :   structure for the DSDs
    output:
            struct asfp_scia **asfp :  structure for ASFP parameters

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV1_RD_ASFP( FILE *fd, unsigned int num_dsd, 
			       const struct dsd_envi *dsd,
			       struct asfp_scia **asfp_out )
{
     char         *asfp_pntr, *asfp_char = NULL;
     size_t       dsr_size;

     unsigned short usbuff;
     unsigned int indx_dsd;
     unsigned int nr_dsr = 0;  /* initialize the return value */
     float rbuff;

     struct asfp_scia *asfp;

     const char dsd_name[] = "SMALL_AP_SLIT_FUNCTION";
/*
 * get index to data set descriptor
 */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, dsd_name );
     if ( dsd[indx_dsd].num_dsr == 0 ) {
          asfp_out[0] = NULL;
          return 0u;
     }
     if ( ! Use_Extern_Alloc ) {
	  asfp_out[0] = (struct asfp_scia *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct asfp_scia));
     }
     if ( (asfp = asfp_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "asfp" );
/*
 * allocate memory to temporary store data for output structure
 */
     dsr_size = (size_t) dsd[indx_dsd].dsr_size;
     if ( (asfp_char = (char *) malloc( dsr_size )) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "asfp_char" );
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
/*
 * read data set records
 */
     do {
	  if ( fread( asfp_char, dsr_size, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "ASFP(dsr)" );
/*
 * read data buffer to ASFP structure
 */
	  asfp_pntr = asfp_char;
	  (void) memcpy( &usbuff, asfp_pntr, ENVI_USHRT );
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  asfp->pixel_position = (short) byte_swap_u16(usbuff);
#else
	  asfp->pixel_position = (short) usbuff;
#endif
	  asfp_pntr += ENVI_USHRT;
	  (void) memcpy( &asfp->type, asfp_pntr, ENVI_UCHAR );
	  asfp_pntr += ENVI_CHAR;
	  (void) memcpy( &rbuff, asfp_pntr, ENVI_FLOAT );
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  IEEE_Swap__FLT(&rbuff);
#endif
	  asfp->fwhm = (double) rbuff;
	  asfp_pntr += ENVI_FLOAT;
	  (void) memcpy( &rbuff, asfp_pntr, ENVI_FLOAT );
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  IEEE_Swap__FLT(&rbuff);
#endif
	  asfp->fwhm_gauss = (double) rbuff;
	  asfp_pntr += ENVI_FLOAT;
/*
 * check if we read the whole DSR
 */
	  if ( (size_t)(asfp_pntr - asfp_char) != dsr_size ) {
	       free( asfp_char );
	       NADC_GOTO_ERROR( NADC_ERR_PDS_SIZE, dsd_name );
	  }
	  asfp++;
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
     asfp_pntr = NULL;
/*
 * set return values
 */
 done:
     if ( asfp_char != NULL ) free( asfp_char );
     return nr_dsr;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_ASFP
.PURPOSE     write Small Aperture Slit function parameters
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_ASFP( fd, num_asfp, asfp );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_asfp :   number of ASFP records
            struct asfp_scia *asfp :  structure for ASFP parameters

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1_WR_ASFP( FILE *fd, unsigned int num_asfp,
		       const struct asfp_scia *asfp )
{
     unsigned short usbuff;
     float rbuff;
     
     struct dsd_envi dsd = {
          "SMALL_AP_SLIT_FUNCTION", "G",
          "                                                              ",
          0u, 0u, 0u, 0
     };

     if ( num_asfp == 0u ) {
	  (void) strcpy( dsd.flname, "NOT USED" );
	  SCIA_LV1_ADD_DSD( &dsd );
	  return;
     }
/*
 * read data set records
 */
     do {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  usbuff = (unsigned short) byte_swap_16(asfp->pixel_position);
#else
	  usbuff = asfp->pixel_position;
#endif
	  if ( fwrite( &usbuff, ENVI_USHRT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_USHRT;
	  if ( fwrite( &asfp->type, ENVI_UCHAR, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UCHAR;
	  
	  rbuff = (float) asfp->fwhm;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  IEEE_Swap__FLT(&rbuff);
#endif
	  if ( fwrite( &rbuff, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_FLOAT;
	  
	  rbuff = (float) asfp->fwhm_gauss;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  IEEE_Swap__FLT(&rbuff);
#endif
	  if ( fwrite( &rbuff, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_FLOAT;

	  asfp++;
     } while ( ++dsd.num_dsr < num_asfp );
/*
 * update list of written DSD records
 */
     dsd.dsr_size = (int) (dsd.size / dsd.num_dsr);
     SCIA_LV1_ADD_DSD( &dsd );
}
