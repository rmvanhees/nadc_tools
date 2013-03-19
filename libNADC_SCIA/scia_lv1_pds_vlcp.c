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

.IDENTifer   SCIA_LV1_PDS_VLCP
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 data
.LANGUAGE    ANSI C
.PURPOSE     read/write Leakage Current Parameters (variable fraction) records
.COMMENTS    contains SCIA_LV1_RD_VLCP and SCIA_LV1_WR_VLCP
.ENVIRONment None
.VERSION      4.0   11-Oct-2005 use direct write, add usage of SCIA_LV1_ADD_DSD
              3.0   18-Apr-2005 added routine to write SRS-struct to file, RvH
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
void Sun2Intel_VLCP( struct vlcp_scia *vlcp )
{
     register int ni;

     IEEE_Swap__FLT( &vlcp->orbit_phase );
     for ( ni = 0; ni < (IR_CHANNELS + PMD_NUMBER); ni++ )
	  IEEE_Swap__FLT( &vlcp->obm_pmd[ni] );
     for ( ni = 0; ni < (IR_CHANNELS * CHANNEL_SIZE); ni++ ) {
	  IEEE_Swap__FLT( &vlcp->var_lc[ni] );
	  IEEE_Swap__FLT( &vlcp->var_lc_error[ni] );
     }
     for ( ni = 0; ni < (SCIENCE_PIXELS); ni++ ) {
	  IEEE_Swap__FLT( &vlcp->solar_stray[ni] );
	  IEEE_Swap__FLT( &vlcp->solar_stray_error[ni] );
     }
     for ( ni = 0; ni < PMD_NUMBER; ni++ ) {
	  IEEE_Swap__FLT( &vlcp->pmd_stray[ni] );
	  IEEE_Swap__FLT( &vlcp->pmd_stray_error[ni] );
     }
     for ( ni = 0; ni < 2; ni++ ) {
	  IEEE_Swap__FLT( &vlcp->pmd_dark[ni] );
	  IEEE_Swap__FLT( &vlcp->pmd_dark_error[ni] );
     }
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_VLCP
.PURPOSE     read Leakage Current Parameters (variable fraction) records
.INPUT/OUTPUT
  call as   nbyte = SCIA_LV1_RD_VLCP( fd, num_dsd, dsd, &vlcp );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi dsd   :   structure for the DSDs
    output:
            struct vlcp_scia **vlcp :  leakage current parameters (variable)

.RETURNS     number of data set records read (unsigned int)
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV1_RD_VLCP( FILE *fd, unsigned int num_dsd, 
			       const struct dsd_envi *dsd,
			       struct vlcp_scia **vlcp_out )
{
     const char prognm[]   = "SCIA_LV1_RD_VLCP";

     char         *vlcp_pntr, *vlcp_char = NULL;
     size_t       dsr_size, nr_byte;
     unsigned int indx_dsd;

     unsigned int nr_dsr = 0;

     struct vlcp_scia  *vlcp;

     const char dsd_name[] = "LEAKAGE_VARIABLE";

     if ( ! Use_Extern_Alloc ) vlcp_out[0] = NULL;
/*
 * get index to data set descriptor
 */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL ) {
	  NADC_ERROR( prognm, NADC_ERR_PDS_RD, dsd_name );
	  return 0;
     }
     if ( dsd[indx_dsd].num_dsr == 0 ) return 0;
/*
 * allocated memory or check externally allocated memory
 */
     if ( ! Use_Extern_Alloc ) {
	  vlcp = (struct vlcp_scia *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct vlcp_scia));
	  if ( vlcp == NULL ) {
	       NADC_ERROR( prognm, NADC_ERR_ALLOC, "vlcp" );
	       return 0;
	  }
     } else if ( (vlcp = vlcp_out[0]) == NULL ) {
	  NADC_ERROR( prognm, NADC_ERR_ALLOC, "vlcp_out[0]" );
	  return 0;
     }
/*
 * allocate memory to temporary store data for output structure
 */
     dsr_size = (size_t) dsd[indx_dsd].dsr_size;
     if ( (vlcp_char = (char *) malloc( dsr_size )) == NULL ) {
	  if ( ! Use_Extern_Alloc ) free( vlcp );
	  NADC_ERROR( prognm, NADC_ERR_ALLOC, "vlcp_char" );
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
	  if ( fread( vlcp_char, dsr_size, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
/*
 * read data buffer to VLCP structure
 */
	  vlcp_pntr = vlcp_char;
	  (void) memcpy( &vlcp[nr_dsr].orbit_phase, vlcp_pntr, ENVI_FLOAT );
	  vlcp_pntr += ENVI_FLOAT;
	  nr_byte = (IR_CHANNELS + PMD_NUMBER) * ENVI_FLOAT;
	  (void) memcpy( vlcp[nr_dsr].obm_pmd, vlcp_pntr, nr_byte );
	  vlcp_pntr += nr_byte;

	  nr_byte = (size_t) (IR_CHANNELS * CHANNEL_SIZE) * ENVI_FLOAT;
	  (void) memcpy( vlcp[nr_dsr].var_lc, vlcp_pntr, nr_byte );
	  vlcp_pntr += nr_byte;
	  (void) memcpy( vlcp[nr_dsr].var_lc_error, vlcp_pntr, nr_byte );
	  vlcp_pntr += nr_byte;

	  nr_byte = (size_t) (SCIENCE_PIXELS) * ENVI_FLOAT;
	  (void) memcpy( vlcp[nr_dsr].solar_stray, vlcp_pntr, nr_byte );
	  vlcp_pntr += nr_byte;
	  (void) memcpy( vlcp[nr_dsr].solar_stray_error, vlcp_pntr, nr_byte );
	  vlcp_pntr += nr_byte;

	  nr_byte = (size_t) PMD_NUMBER * ENVI_FLOAT;
	  (void) memcpy( vlcp[nr_dsr].pmd_stray, vlcp_pntr, nr_byte );
	  vlcp_pntr += nr_byte;
	  (void) memcpy( vlcp[nr_dsr].pmd_stray_error, vlcp_pntr, nr_byte );
	  vlcp_pntr += nr_byte;

	  nr_byte = (size_t) IR_PMD_NUMBER * ENVI_FLOAT;
	  (void) memcpy( vlcp[nr_dsr].pmd_dark, vlcp_pntr, nr_byte );
	  vlcp_pntr += nr_byte;
	  (void) memcpy( vlcp[nr_dsr].pmd_dark_error, vlcp_pntr, nr_byte );
	  vlcp_pntr += nr_byte;
/*
 * check if we read the whole DSR
 */
	  if ( (size_t)(vlcp_pntr - vlcp_char) != dsr_size ) {
	       free( vlcp_char );
	       if ( ! Use_Extern_Alloc ) free( vlcp );
	       NADC_ERROR( prognm, NADC_ERR_PDS_SIZE, dsd_name );
	       return 0;
	  }
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_VLCP( vlcp+nr_dsr );
#endif
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
/*
 * set return values
 */
     vlcp_pntr = NULL;
     vlcp_out[0] = vlcp;
done:
     if (vlcp_char != NULL ) free( vlcp_char );
     return nr_dsr;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_VLCP
.PURPOSE     write Leakage Current Parameters (variable fraction) records
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_VLCP( fd, num_vlcp, vlcp );
     input:
            FILE *fd               :  stream pointer
	    unsigned int num_vlcp  :  number of VLCP records
            struct vlcp_scia *vlcp :  leakage current parameters (variable)

.RETURNS     nothing
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1_WR_VLCP( FILE *fd, unsigned int num_vlcp, 
		       const struct vlcp_scia *vlcp_in )
{
     const char prognm[] = "SCIA_LV1_WR_VLCP";

     size_t nr_byte;

     struct vlcp_scia  vlcp;

     struct dsd_envi dsd = {
          "LEAKAGE_VARIABLE", "G",
          "                                                              ",
          0u, 0u, 0u, 0
     };

     if ( num_vlcp == 0u ) {
	  (void) strcpy( dsd.flname, "NOT USED" );
	  SCIA_LV1_ADD_DSD( &dsd );
	  return;
     }
/*
 * write data set records
 */
     do {
	  (void) memcpy( &vlcp, vlcp_in, sizeof( struct vlcp_scia ));
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_VLCP( &vlcp );
#endif
/*
 * write VLCP structure to file
 */
	  if ( fwrite( &vlcp.orbit_phase, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_FLOAT;
	  nr_byte = (IR_CHANNELS + PMD_NUMBER) * ENVI_FLOAT;
	  if ( fwrite( vlcp.obm_pmd, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;

	  nr_byte = (size_t) (IR_CHANNELS * CHANNEL_SIZE) * ENVI_FLOAT;
	  if ( fwrite( vlcp.var_lc, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( vlcp.var_lc_error, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;

	  nr_byte = (size_t) (SCIENCE_PIXELS) * ENVI_FLOAT;
	  if ( fwrite( vlcp.solar_stray, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( vlcp.solar_stray_error, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;

	  nr_byte = (size_t) PMD_NUMBER * ENVI_FLOAT;
	  if ( fwrite( vlcp.pmd_stray, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( vlcp.pmd_stray_error, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;

	  nr_byte = (size_t) IR_PMD_NUMBER * ENVI_FLOAT;
	  if ( fwrite( vlcp.pmd_dark, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( vlcp.pmd_dark_error, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;

	  vlcp_in++;
     } while ( ++dsd.num_dsr < num_vlcp );
/*
 * update list of written DSD records
 */
     dsd.dsr_size = (int) (dsd.size / dsd.num_dsr);
     SCIA_LV1_ADD_DSD( &dsd );
}
