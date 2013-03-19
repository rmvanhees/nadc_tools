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

.IDENTifer   SCIA_LV1_PDS_CLCP
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 data
.LANGUAGE    ANSI C
.PURPOSE     read/write Leakage Current Parameters (constant fraction) records
.COMMENTS    contains SCIA_LV1_RD_CLCP and SCIA_LV1_WR_CLCP
.ENVIRONment None
.VERSION      4.0   11-Oct-2005 use direct write, add usage of SCIA_LV1_ADD_DSD
              3.0   14-Apr-2005 added routine to write CLCP-struct to file, RvH
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
void Sun2Intel_CLCP( struct clcp_scia *clcp )
{
     register int ni;

     for ( ni = 0; ni < (SCIENCE_PIXELS); ni++ ) {
	  IEEE_Swap__FLT( &clcp->fpn[ni] );
	  IEEE_Swap__FLT( &clcp->fpn_error[ni] );
	  IEEE_Swap__FLT( &clcp->lc[ni] );
	  IEEE_Swap__FLT( &clcp->lc_error[ni] );
	  IEEE_Swap__FLT( &clcp->mean_noise[ni] );
     }
     for ( ni = 0; ni < (2 * PMD_NUMBER); ni++ ) {
	  IEEE_Swap__FLT( &clcp->pmd_dark[ni] );
	  IEEE_Swap__FLT( &clcp->pmd_dark_error[ni] );
     }
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_CLCP
.PURPOSE     read Leakage Current Parameters (constant fraction) records
.INPUT/OUTPUT
  call as   nr_dsr = SCIA_LV1_RD_CLCP( fd, num_dsd, dsd, &clcp );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi dsd   :   structure for the DSDs
    output:
            struct clcp_scia *clcp :  leakage current parameters (constant)

.RETURNS     number of data set records read (unsigned int)
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV1_RD_CLCP( FILE *fd, unsigned int num_dsd, 
			       const struct dsd_envi *dsd,
			       struct clcp_scia *clcp )
{
     const char prognm[]   = "SCIA_LV1_RD_CLCP";

     char         *clcp_pntr, *clcp_char = NULL;
     size_t       dsr_size, nr_byte;
     unsigned int indx_dsd;

     const char dsd_name[] = "LEAKAGE_CONSTANT";
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
     if ( (clcp_char = (char *) malloc( dsr_size )) == NULL ) {
	  NADC_ERROR( prognm, NADC_ERR_ALLOC, "clcp_char" );
	  return 0;
     }
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
     if ( fread( clcp_char, dsr_size, 1, fd ) != 1 ) {
	  free( clcp_char );
	  NADC_ERROR( prognm, NADC_ERR_PDS_RD, "" );
	  return 0u;
     }
     clcp_pntr = clcp_char;
/*
 * read data buffer to CLCP structure
 */
     nr_byte = (size_t) (SCIENCE_PIXELS) * ENVI_FLOAT;
     (void) memcpy( clcp->fpn, clcp_pntr, nr_byte );
     clcp_pntr += nr_byte;
     (void) memcpy( clcp->fpn_error, clcp_pntr, nr_byte );
     clcp_pntr += nr_byte;
     (void) memcpy( clcp->lc, clcp_pntr, nr_byte );
     clcp_pntr += nr_byte;
     (void) memcpy( clcp->lc_error, clcp_pntr, nr_byte );
     clcp_pntr += nr_byte;

     nr_byte = (size_t) (2 * PMD_NUMBER) * ENVI_FLOAT;
     (void) memcpy( clcp->pmd_dark, clcp_pntr, nr_byte );
     clcp_pntr += nr_byte;
     (void) memcpy( clcp->pmd_dark_error, clcp_pntr, nr_byte );
     clcp_pntr += nr_byte;

     nr_byte = (size_t) (SCIENCE_PIXELS) * ENVI_FLOAT;
     (void) memcpy( clcp->mean_noise, clcp_pntr, nr_byte );
     clcp_pntr += nr_byte;
/*
 * check if we read the whole DSR
 */
     if ( (size_t)(clcp_pntr - clcp_char) != dsr_size ) {
	  free( clcp_char );
	  NADC_ERROR( prognm, NADC_ERR_PDS_SIZE, dsd_name );
	  return 0;
     }
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_CLCP( clcp );
#endif
/*
 * deallocate memory
 */
     clcp_pntr = NULL;
     free( clcp_char );
/*
 * set return values
 */
     return 1u;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_CLCP
.PURPOSE     write Leakage Current Parameters (constant fraction) records
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_CLCP( fd, num_clcp, clcp );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_clcp :   number of CLCP records
            struct clcp_scia clcp :  leakage current parameters (constant)

.RETURNS     nothing
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1_WR_CLCP( FILE *fd, unsigned int num_clcp, 
		       const struct clcp_scia clcp_in )
{
     const char prognm[] = "SCIA_LV1_WR_CLCP";

     size_t nr_byte;

     struct clcp_scia clcp;

     struct dsd_envi dsd = {
	  "LEAKAGE_CONSTANT", "G",
	  "                                                              ",
	  0u, 0u, 0u, 0
     };

     if ( num_clcp == 0u ) {
	  (void) strcpy( dsd.flname, "NOT USED" );
	  SCIA_LV1_ADD_DSD( &dsd );
	  return;
     }
/*
 * copy CLCP structure data set records
 */
     (void) memcpy( &clcp, &clcp_in, sizeof( struct clcp_scia ) );
#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_CLCP( &clcp );
#endif
     nr_byte = (size_t) (SCIENCE_PIXELS) * ENVI_FLOAT;
     if ( fwrite( clcp.fpn, nr_byte, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
     if ( fwrite( clcp.fpn_error, nr_byte, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
     if ( fwrite( clcp.lc, nr_byte, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
     if ( fwrite( clcp.lc_error, nr_byte, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;

     nr_byte = (size_t) (2 * PMD_NUMBER) * ENVI_FLOAT;
     if ( fwrite( clcp.pmd_dark, nr_byte, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
     if ( fwrite( clcp.pmd_dark_error, nr_byte, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;

     nr_byte = (size_t) (SCIENCE_PIXELS) * ENVI_FLOAT;
     if ( fwrite( clcp.mean_noise, nr_byte, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
/*
 * update list of written DSD records
 */
     dsd.num_dsr = 1u;
     dsd.dsr_size = (int) dsd.size;
     SCIA_LV1_ADD_DSD( &dsd );
}
