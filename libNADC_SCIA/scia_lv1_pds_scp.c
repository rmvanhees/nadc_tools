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

.IDENTifer   SCIA_LV1_PDS_SCP
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 data
.LANGUAGE    ANSI C
.PURPOSE     read/write Spectral Calibration Parameters records
.COMMENTS    contains SCIA_LV1_RD_SCP and SCIA_LV1_WR_SCP
.ENVIRONment None
.VERSION      4.0   11-Oct-2005 use direct write, 
                                add usage of SCIA_LV1_ADD_DSD, RvH
              3.0   15-Apr-2005 added routine to write SCP-struct to file, RvH
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
void Sun2Intel_SCP( struct scp_scia *scp )
{
     register int ni;

     IEEE_Swap__FLT( &scp->orbit_phase );
     for ( ni = 0; ni < (5 * SCIENCE_CHANNELS); ni++ ) {
	  IEEE_Swap__DBL( &scp->coeffs[ni] );
     }
     for ( ni = 0; ni < SCIENCE_CHANNELS; ni++ ) {
	  scp->num_lines[ni] = byte_swap_u16( scp->num_lines[ni] );
	  IEEE_Swap__FLT( &scp->wv_error_calib[ni] );
     }
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_SCP
.PURPOSE     read Spectral Calibration Parameters records
.INPUT/OUTPUT
  call as   nr_dsr = SCIA_LV1_RD_SCP( fd, num_dsd, dsd, &scp );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi dsd   :   structure for the DSDs
    output:
            struct scp_scia **scp :  Spectral Calibration Parameters

.RETURNS     number of data set records read (unsigned int)
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV1_RD_SCP( FILE *fd, unsigned int num_dsd, 
			      const struct dsd_envi *dsd,
			      struct scp_scia **scp_out )
{
     char         *scp_char, *scp_pntr;
     size_t       dsr_size, nr_byte;
     unsigned int indx_dsd;

     unsigned int nr_dsr = 0;

     struct scp_scia *scp;

     const char dsd_name[] = "SPECTRAL_CALIBRATION";

     if ( ! Use_Extern_Alloc ) scp_out[0] = NULL;
/*
 * get index to data set descriptor
 */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL ) {
	  NADC_ERROR( NADC_ERR_PDS_RD, dsd_name );
	  return 0;
     }
     if ( dsd[indx_dsd].num_dsr == 0 ) return 0;

     if ( ! Use_Extern_Alloc ) {
	  scp = (struct scp_scia *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct scp_scia));
	  if ( scp == NULL ) {
	       NADC_ERROR( NADC_ERR_ALLOC, "scp" );
	       return 0;
	  }
     } else if ( (scp = scp_out[0]) == NULL ) {
	  NADC_ERROR( NADC_ERR_ALLOC, "scp_out[0]" );
	  return 0;
     }
/*
 * allocate memory to temporary store data for output structure
 */
     dsr_size = (size_t) dsd[indx_dsd].dsr_size;
     if ( (scp_char = (char *) malloc( dsr_size )) == NULL ) {
	  if ( ! Use_Extern_Alloc ) free( scp );
	  NADC_ERROR( NADC_ERR_ALLOC, "scp_char" );
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
	  if ( fread( scp_char, dsr_size, 1, fd ) != 1 ) {
	       free( scp_char );
	       NADC_ERROR( NADC_ERR_PDS_RD, "" );
	       return 0;
	  }
/*
 * read data buffer to SCP structure
 */
	  scp_pntr = scp_char;
	  (void) memcpy( &scp[nr_dsr].orbit_phase, scp_pntr, ENVI_FLOAT );
	  scp_pntr += ENVI_FLOAT;
	  nr_byte = 5 * SCIENCE_CHANNELS * ENVI_DBLE;
	  (void) memcpy( scp[nr_dsr].coeffs, scp_pntr, nr_byte );
	  scp_pntr += nr_byte;
	  nr_byte = SCIENCE_CHANNELS * ENVI_USHRT;
	  (void) memcpy( scp[nr_dsr].num_lines, scp_pntr, nr_byte );
	  scp_pntr += nr_byte;
	  nr_byte = SCIENCE_CHANNELS * ENVI_FLOAT;
	  (void) memcpy( scp[nr_dsr].wv_error_calib, scp_pntr, nr_byte );
	  scp_pntr += nr_byte;
/*
 * check if we read the whole DSR
 */
	  if ( (size_t)(scp_pntr - scp_char) != dsr_size ) {
	       free( scp_char );
	       if ( ! Use_Extern_Alloc ) free( scp );
	       NADC_ERROR( NADC_ERR_PDS_SIZE, dsd_name );
	       return 0;
	  }
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_SCP( scp+nr_dsr );
#endif
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
/*
 * deallocate memory
 */
     scp_pntr = NULL;
     free( scp_char );
/*
 * set return values
 */
     scp_out[0] = scp;
     return nr_dsr;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_SCP
.PURPOSE     write Spectral Calibration Parameters records
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_SCP( fd, num_scp, scp );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_scp  :   number of SCP records
            struct scp_scia *scp  :   Spectral Calibration Parameters

.RETURNS     nothing
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1_WR_SCP( FILE *fd, unsigned int num_scp, 
		      const struct scp_scia *scp_in )
{
     size_t nr_byte;

     struct scp_scia scp;

     struct dsd_envi dsd = {
          "SPECTRAL_CALIBRATION", "G",
          "                                                              ",
          0u, 0u, 0u, 0
     };

     if ( num_scp == 0u ) {
	  (void) strcpy( dsd.flname, "NOT USED" );
	  SCIA_LV1_ADD_DSD( &dsd );
	  return;
     }
/*
 * write data set records
 */
     do {
	  (void) memcpy( &scp, scp_in, sizeof( struct scp_scia ));
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_SCP( &scp );
#endif
/*
 * write SCP structure to data buffer
 */
	  if ( fwrite( &scp.orbit_phase, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_FLOAT;
	  nr_byte = 5 * SCIENCE_CHANNELS * ENVI_DBLE;
	  if ( fwrite( scp.coeffs, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  nr_byte = SCIENCE_CHANNELS * ENVI_USHRT;
	  if ( fwrite( scp.num_lines, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  nr_byte = SCIENCE_CHANNELS * ENVI_FLOAT;
	  if ( fwrite( scp.wv_error_calib, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;

	  scp_in++;
     } while ( ++dsd.num_dsr < num_scp );
/*
 * update list of written DSD records
 */
     dsd.dsr_size = (int) (dsd.size / dsd.num_dsr);
     SCIA_LV1_ADD_DSD( &dsd );
}
