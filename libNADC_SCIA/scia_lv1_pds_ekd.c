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

.IDENTifer   SCIA_LV1_PDS_EKD
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 data
.LANGUAGE    ANSI C
.PURPOSE     read/write Errors on Key Data
.COMMENTS    contains SCIA_LV1_RD_EKD and SCIA_LV1_WR_EKD
.ENVIRONment None
.VERSION      4.0   11-Oct-2005 always only one EKD records per file;
                                add usage of SCIA_LV1_ADD_DSD, RvH
              3.0   14-Apr-2005 added routine to write EKD to file, RvH
              2.2   22-Mar-2002	test number of DSD; can be zero, RvH
              2.1   16-Jan-2002	use of global Use_Extern_Alloc, RvH 
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              1.1   04-Oct-2001	changed input/output, RvH 
              1.0   10-Nov-1999 created by R. M. van Hees
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
void Sun2Intel_EKD( struct ekd_scia *ekd )
{
     register int ni = 0;

     do {
	  IEEE_Swap__FLT( &ekd->mu2_nadir[ni] );
	  IEEE_Swap__FLT( &ekd->mu3_nadir[ni] );
	  IEEE_Swap__FLT( &ekd->mu2_limb[ni] );
	  IEEE_Swap__FLT( &ekd->mu3_limb[ni] );
	  IEEE_Swap__FLT( &ekd->radiance_vis[ni] );
	  IEEE_Swap__FLT( &ekd->radiance_nadir[ni] );
	  IEEE_Swap__FLT( &ekd->radiance_limb[ni] );
	  IEEE_Swap__FLT( &ekd->radiance_sun[ni] );
	  IEEE_Swap__FLT( &ekd->bsdf[ni] );
     } while ( ++ni < (SCIENCE_PIXELS) );
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_EKD
.PURPOSE     read Errors on Key Data
.INPUT/OUTPUT
  call as   nr_dsr = SCIA_LV1_RD_EKD( fd, num_dsd, dsd, &ekd );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi *dsd  :   structure for the DSDs
    output:
            struct ekd_scia *ekd  :  errors on key data

.RETURNS     number of data set records read (unsigned int)
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV1_RD_EKD( FILE *fd, unsigned int num_dsd, 
			      const struct dsd_envi *dsd,
			      struct ekd_scia *ekd )
{
     char         *ekd_pntr, *ekd_char = NULL;
     size_t       dsr_size;
     unsigned int indx_dsd;

     const char dsd_name[] = "ERRORS_ON_KEY_DATA";
     const size_t nr_byte  = SCIENCE_PIXELS * ENVI_FLOAT;
/*
 * get index to data set descriptor
 */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL ) {
	  NADC_ERROR( NADC_ERR_PDS_RD, dsd_name );
	  return 0u;
     }
     if ( dsd[indx_dsd].num_dsr == 0 ) return 0u;
/*
 * allocate memory to temporary store data for output structure
 */
     dsr_size = (size_t) dsd[indx_dsd].dsr_size;
     if ( (ekd_char = (char *) malloc( dsr_size )) == NULL ) { 
	  NADC_ERROR( NADC_ERR_ALLOC, "ekd_char" );
	  return 0u;
     }
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
     if ( fread( ekd_char, dsr_size, 1, fd ) != 1 ) {
	  free( ekd_char );
	  NADC_ERROR( NADC_ERR_PDS_RD, "" );
	  return 0u;
     }
     ekd_pntr = ekd_char;
/*
 * read data buffer to EKD structure
 */
     (void) memcpy( ekd->mu2_nadir, ekd_pntr, nr_byte );
     ekd_pntr += nr_byte;
     (void) memcpy( ekd->mu3_nadir, ekd_pntr, nr_byte );
     ekd_pntr += nr_byte;
     (void) memcpy( ekd->mu2_limb, ekd_pntr, nr_byte );
     ekd_pntr += nr_byte;
     (void) memcpy( ekd->mu3_limb, ekd_pntr, nr_byte );
     ekd_pntr += nr_byte;
     (void) memcpy( ekd->radiance_vis, ekd_pntr, nr_byte );
     ekd_pntr += nr_byte;
     (void) memcpy( ekd->radiance_nadir, ekd_pntr, nr_byte );
     ekd_pntr += nr_byte;
     (void) memcpy( ekd->radiance_limb, ekd_pntr, nr_byte );
     ekd_pntr += nr_byte;
     (void) memcpy( ekd->radiance_sun, ekd_pntr, nr_byte );
     ekd_pntr += nr_byte;
     (void) memcpy( ekd->bsdf, ekd_pntr, nr_byte );
     ekd_pntr += nr_byte;
/*
 * check if we read the whole DSR
 */
     if ( (size_t)(ekd_pntr - ekd_char) != dsr_size ) {
	  free( ekd_char );
	  NADC_ERROR( NADC_ERR_PDS_SIZE, dsd_name );
	  return 0u;
     }
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_EKD( ekd );
#endif
/*
 * deallocate memory
 */
     ekd_pntr = NULL;
     free( ekd_char );
/*
 * set return values
 */
     return 1u;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_EKD
.PURPOSE     write Errors on Key Data
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_EKD( fd, num_ekd, ekd );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_ekd  :   number of EKD records
            struct ekd_scia  ekd  :   errors on key data

.RETURNS     nothing
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1_WR_EKD( FILE *fd, unsigned int num_ekd, 
		      const struct ekd_scia ekd_in )
{
     struct ekd_scia ekd;

     struct dsd_envi dsd = {
	  "ERRORS_ON_KEY_DATA", "G",
	  "                                                              ",
	  0u, 0u, 0u, 0
     };

     const size_t nr_byte  = SCIENCE_PIXELS * ENVI_FLOAT;

     if ( num_ekd == 0u ) {
	  (void) strcpy( dsd.flname, "NOT USED" );
	  SCIA_LV1_ADD_DSD( &dsd );
	  return;
     }
/*
 * copy data set records
 */
     (void) memcpy( &ekd, &ekd_in, sizeof( struct ekd_scia ) );
#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_EKD( &ekd );
#endif
/*
 * write EKD structure to file
 */
     if ( fwrite( ekd.mu2_nadir, nr_byte, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
     if ( fwrite( ekd.mu3_nadir, nr_byte, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
     if ( fwrite( ekd.mu2_limb, nr_byte, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
     if ( fwrite( ekd.mu3_limb, nr_byte, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
     if ( fwrite( ekd.radiance_vis, nr_byte, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
     if ( fwrite( ekd.radiance_nadir, nr_byte, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
     if ( fwrite( ekd.radiance_limb, nr_byte, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
     if ( fwrite( ekd.radiance_sun, nr_byte, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
     if ( fwrite( ekd.bsdf, nr_byte, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
/*
 * update list of written DSD records
 */
     dsd.num_dsr = 1u;
     dsd.dsr_size = (int) dsd.size;
     SCIA_LV1_ADD_DSD( &dsd );
}
