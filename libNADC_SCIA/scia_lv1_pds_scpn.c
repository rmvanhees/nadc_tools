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

.IDENTifer   SCIA_LV1_PDS_SCPN
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 data
.LANGUAGE    ANSI C
.PURPOSE     read/write Spectral Calibration Parameters (newly calculated)
.COMMENTS    contains SCIA_LV1_RD_SCPN and SCIA_LV1_WR_SCPN
.ENVIRONment None
.VERSION      4.0   11-Oct-2005 use direct write, add usage of SCIA_LV1_ADD_DSD
              3.0   15-Apr-2005 added routine to write SCPN-struct to file, RvH
              2.3   22-Mar-2002	test number of DSD; can be zero, RvH
              2.2   06-Mar-2002	added check for presence of this dataset, RvH 
              2.1   16-Jan-2002	use of global Use_Extern_Alloc, RvH 
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              1.1   05-Oct-2001	changed input/output, RvH 
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
void Sun2Intel_SCPN( struct scpn_scia *scpn )
{
     register int ni;

     scpn->mjd.days = byte_swap_32( scpn->mjd.days );
     scpn->mjd.secnd = byte_swap_u32( scpn->mjd.secnd );
     scpn->mjd.musec = byte_swap_u32( scpn->mjd.musec );
     IEEE_Swap__FLT( &scpn->orbit_phase );
     for ( ni = 0; ni < (5 * SCIENCE_CHANNELS); ni++ ) {
	  IEEE_Swap__DBL( &scpn->coeffs[ni] );
     }
     for ( ni = 0; ni < SCIENCE_CHANNELS; ni++ ) {
	  scpn->num_lines[ni] = byte_swap_u16( scpn->num_lines[ni] );
	  IEEE_Swap__FLT( &scpn->wv_error_calib[ni] );
     }
     for ( ni = 0; ni < (SCIENCE_PIXELS); ni++ ) {
	  IEEE_Swap__FLT( &scpn->sol_spec[ni] );
     }
     for ( ni = 0; ni < (3 * SCIENCE_CHANNELS); ni++ ) {
	  IEEE_Swap__FLT( &scpn->line_pos[ni] );
     }
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_SCPN
.PURPOSE     read Spectral Calibration Parameters (newly calculated)
.INPUT/OUTPUT
  call as   nbyte = SCIA_LV1_RD_SCPN( fd, num_dsd, dsd, &scpn );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi *dsd  :   structure for the DSDs
    output:
            struct scpn_scia **scpn :  spectral calibration parameters (new)

.RETURNS     number of data set records read (unsigned int)
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV1_RD_SCPN( FILE *fd, unsigned int num_dsd, 
			       const struct dsd_envi *dsd,
			       struct scpn_scia **scpn_out )
{
     const char prognm[] = "SCIA_LV1_RD_SCPN";

     char         *scpn_pntr, *scpn_char = NULL;
     size_t       dsr_size, nr_byte;
     unsigned int indx_dsd;

     unsigned int nr_dsr = 0;

     struct scpn_scia *scpn;

     const char dsd_name[] = "NEW_SPECTRAL_CALIBRATION";
/*
 * get index to data set descriptor
 */
     NADC_ERR_SAVE();
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, dsd_name );
     if ( IS_ERR_STAT_ABSENT || dsd[indx_dsd].num_dsr == 0 ) {
	  NADC_ERR_RESTORE();
          scpn_out[0] = NULL;
	  return 0u;
     }
     dsr_size = (size_t) dsd[indx_dsd].dsr_size;
/*
 * allocate memory to store output structures
 */
     if ( ! Use_Extern_Alloc ) {
	  scpn_out[0] = (struct scpn_scia *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct scpn_scia));
     }
     if ( (scpn = scpn_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "scpn" );
/*
 * allocate memory to temporary store data for output structure
 */
     if ( (scpn_char = (char *) malloc( dsr_size )) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "scpn_char" );
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
/*
 * read data set records
 */
     do {
	  if ( fread( scpn_char, dsr_size, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
/*
 * read data buffer to SCPN structure
 */
	  (void) memcpy( &scpn->mjd.days, scpn_char, ENVI_INT );
	  scpn_pntr = scpn_char + ENVI_INT;
	  (void) memcpy( &scpn->mjd.secnd, scpn_pntr, ENVI_UINT );
	  scpn_pntr += ENVI_UINT;
	  (void) memcpy( &scpn->mjd.musec, scpn_pntr, ENVI_UINT );
	  scpn_pntr += ENVI_UINT;
	  (void) memcpy( &scpn->flag_mds, scpn_pntr, ENVI_UCHAR );
	  scpn_pntr += ENVI_UCHAR;
	  (void) memcpy( &scpn->orbit_phase, scpn_pntr, ENVI_FLOAT );
	  scpn_pntr += ENVI_FLOAT;

	  nr_byte = (size_t) (5 * SCIENCE_CHANNELS) * ENVI_DBLE;
	  (void) memcpy( scpn->coeffs, scpn_pntr, nr_byte );
	  scpn_pntr += nr_byte;
	  nr_byte = (size_t) SCIENCE_CHANNELS * ENVI_UCHAR;
	  (void) memcpy( scpn->srs_param, scpn_pntr, nr_byte );
	  scpn_pntr += nr_byte;
	  nr_byte = (size_t) SCIENCE_CHANNELS * ENVI_USHRT;
	  (void) memcpy( scpn->num_lines, scpn_pntr, nr_byte );
	  scpn_pntr += nr_byte;
	  nr_byte = (size_t) SCIENCE_CHANNELS * ENVI_FLOAT;
	  (void) memcpy( scpn->wv_error_calib, scpn_pntr, nr_byte );
	  scpn_pntr += nr_byte;
	  nr_byte = (size_t) (SCIENCE_PIXELS) * ENVI_FLOAT;
	  (void) memcpy( scpn->sol_spec, scpn_pntr, nr_byte );
	  scpn_pntr += nr_byte;
	  nr_byte = (size_t) (3 * SCIENCE_CHANNELS) * ENVI_FLOAT;
	  (void) memcpy( scpn->line_pos, scpn_pntr, nr_byte );
	  scpn_pntr += nr_byte;
/*
 * check if we read the whole DSR
 */
	  if ( (size_t)(scpn_pntr - scpn_char) != dsr_size )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, dsd_name );
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_SCPN( scpn );
#endif
	  scpn++;
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
/*
 * set return values
 */
     scpn_pntr = NULL;
 done:
     if ( scpn_char != NULL ) free( scpn_char );
     return nr_dsr;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_SCPN
.PURPOSE     write Spectral Calibration Parameters (newly calculated)
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_SCPN( fd, num_dsd, dsd, scpn );
     input:
            FILE *fd               :   stream pointer
	    unsigned int num_scpn  :   number of SCPN records
            struct scpn_scia *scpn :   spectral calibration parameters (new)

.RETURNS     nothing
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1_WR_SCPN( FILE *fd, unsigned int num_scpn,
		       const struct scpn_scia *scpn_in )
{
     const char prognm[] = "SCIA_LV1_WR_SCPN";

     size_t nr_byte;

     struct scpn_scia scpn;

     struct dsd_envi dsd = {
          "NEW_SPECTRAL_CALIBRATION", "A",
          "                                                              ",
          0u, 0u, 0u, 0
     };

     if ( num_scpn == 0u ) {
	  (void) strcpy( dsd.flname, "NOT USED" );
	  SCIA_LV1_ADD_DSD( &dsd );
	  return;
     }
/*
 * write data set records
 */
     do {
	  (void) memcpy( &scpn, scpn_in, sizeof( struct scpn_scia ));
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_SCPN( &scpn );
#endif
/*
 * write SCPN structures to file
 */
	  if ( fwrite( &scpn.mjd.days, ENVI_INT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_INT;
	  if ( fwrite( &scpn.mjd.secnd, ENVI_UINT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UINT;
	  if ( fwrite( &scpn.mjd.musec, ENVI_UINT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UINT;
	  if ( fwrite( &scpn.flag_mds, ENVI_UCHAR, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UCHAR;
	  if ( fwrite( &scpn.orbit_phase, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_FLOAT;

	  nr_byte = (size_t) (5 * SCIENCE_CHANNELS) * ENVI_DBLE;
	  if ( fwrite( scpn.coeffs, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  nr_byte = (size_t) SCIENCE_CHANNELS * ENVI_UCHAR;
	  if ( fwrite( scpn.srs_param, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  nr_byte = (size_t) SCIENCE_CHANNELS * ENVI_USHRT;
	  if ( fwrite( scpn.num_lines, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  nr_byte = (size_t) SCIENCE_CHANNELS * ENVI_FLOAT;
	  if ( fwrite( scpn.wv_error_calib, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  nr_byte = (size_t) (SCIENCE_PIXELS) * ENVI_FLOAT;
	  if ( fwrite( scpn.sol_spec, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  nr_byte = (size_t) (3 * SCIENCE_CHANNELS) * ENVI_FLOAT;
	  if ( fwrite( scpn.line_pos, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;

	  scpn_in++;
     } while ( ++dsd.num_dsr < num_scpn );
/*
 * update list of written DSD records
 */
     dsd.dsr_size = (int) (dsd.size / dsd.num_dsr);
     SCIA_LV1_ADD_DSD( &dsd );
}
