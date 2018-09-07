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

.IDENTifer   SCIA_LV1_PDS_SFP
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 data
.LANGUAGE    ANSI C
.PURPOSE     read/write Slit function parameters
.COMMENTS    contains SCIA_LV1_RD_SFP and SCIA_LV1_WR_SFP
.ENVIRONment none
.VERSION      4.0   11-Oct-2005 use direct write, add usage of SCIA_LV1_ADD_DSD
              3.0   15-Apr-2005 added routine to write SFP-struct to file, RvH
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
.IDENTifer   SCIA_LV1_RD_SFP
.PURPOSE     read Slit function parameters
.INPUT/OUTPUT
  call as   nr_dsr = SCIA_LV1_RD_SFP( fd, num_dsd, dsd, &sfp );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi *dsd  :   structure for the DSDs
    output:
            struct sfp_scia **sfp :  structure for Slit Parameters

.RETURNS     number of data set records read (unsigned int)
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV1_RD_SFP( FILE *fd, unsigned int num_dsd, 
			      const struct dsd_envi *dsd,
			      struct sfp_scia **sfp_out )
{
     char         *sfp_pntr, *sfp_char = NULL;
     size_t       dsr_size;

     unsigned short usbuff;
     unsigned int indx_dsd;
     unsigned int nr_dsr = 0;  /* initialize the return value */
     float rbuff;

     struct sfp_scia *sfp;

     const char dsd_name[] = "SLIT_FUNCTION";
/*
 * get index to data set descriptor
 */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, dsd_name );
     if ( dsd[indx_dsd].num_dsr == 0 ) {
          sfp_out[0] = NULL;
          return 0u;
     }
     if ( ! Use_Extern_Alloc ) {
	  sfp_out[0] = (struct sfp_scia *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct sfp_scia));
     }
     if ( (sfp = sfp_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "sfp" );
/*
 * allocate memory to temporary store data for output structure
 */
     dsr_size = (size_t) dsd[indx_dsd].dsr_size;
     if ( (sfp_char = (char *) malloc( dsr_size )) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "sfp_char" );
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
/*
 * read data set records
 */
     do {
	  if ( fread( sfp_char, dsr_size, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "SFP(dsr)" );
/*
 * read data buffer to SFP structure
 */
	  sfp_pntr = sfp_char;
	  (void) memcpy( &usbuff, sfp_pntr, ENVI_USHRT );
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  sfp->pixel_position = (short) byte_swap_u16(usbuff);
#else
	  sfp->pixel_position = (short) usbuff;
#endif
	  sfp_pntr += ENVI_USHRT;
	  (void) memcpy( &sfp->type, sfp_pntr, ENVI_UCHAR );
	  sfp_pntr += ENVI_CHAR;
	  (void) memcpy( &rbuff, sfp_pntr, ENVI_FLOAT );
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  IEEE_Swap__FLT(&rbuff);
#endif
	  sfp->fwhm = (double) rbuff;
	  sfp_pntr += ENVI_FLOAT;
	  (void) memcpy( &rbuff, sfp_pntr, ENVI_FLOAT );
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  IEEE_Swap__FLT(&rbuff);
#endif
	  sfp->fwhm_gauss = (double) rbuff;
	  sfp_pntr += ENVI_FLOAT;
/*
 * check if we read the whole DSR
 */
	  if ( (size_t)(sfp_pntr - sfp_char) != dsr_size ) {
	       free( sfp_char );
	       NADC_GOTO_ERROR( NADC_ERR_PDS_SIZE, dsd_name );
	  }
	  sfp++;
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
     sfp_pntr = NULL;
/*
 * set return values
 */
 done:
     if ( sfp_char != NULL ) free(sfp_char);
     return nr_dsr;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_SFP
.PURPOSE     write Slit function parameters
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_SFP( fd, num_sfp, sfp );
     input:
            FILE *fd              :  stream pointer
	    unsigned int num_sfp  :  number of SFP records
            struct sfp_scia *sfp  :  structure for Slit Parameters

.RETURNS     nothing
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1_WR_SFP( FILE *fd, unsigned int num_sfp,
		      const struct sfp_scia *sfp )
{
     unsigned short usbuff;
     float rbuff;

     struct dsd_envi dsd = {
          "SLIT_FUNCTION", "G",
          "                                                              ",
          0u, 0u, 0u, 0
     };

     if ( num_sfp == 0u ) {
	  (void) strcpy( dsd.flname, "NOT USED" );
	  SCIA_LV1_ADD_DSD( &dsd );
	  return;
     }
/*
 * read data set records
 */
     do {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  usbuff = (unsigned short) byte_swap_16(sfp->pixel_position);
#else
	  usbuff = sfp->pixel_position;
#endif
	  if ( fwrite( &usbuff, ENVI_USHRT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_USHRT;
	  if ( fwrite( &sfp->type, ENVI_UCHAR, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UCHAR;
	  
	  rbuff = (float) sfp->fwhm;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  IEEE_Swap__FLT(&rbuff);
#endif
	  if ( fwrite( &rbuff, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_FLOAT;
	  
	  rbuff = (float) sfp->fwhm_gauss;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  IEEE_Swap__FLT(&rbuff);
#endif
	  if ( fwrite( &rbuff, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_FLOAT;

	  sfp++;
     } while ( ++dsd.num_dsr < num_sfp );
/*
 * update list of written DSD records
 */
     dsd.dsr_size = (int) (dsd.size / dsd.num_dsr);
     SCIA_LV1_ADD_DSD( &dsd );
}
