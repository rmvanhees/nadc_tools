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

.IDENTifer   SCIA_LV1_PDS_BASE
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 data
.LANGUAGE    ANSI C
.PURPOSE     read/write Precise Basis of Spectral Calibration Parameters
.COMMENTS    contains SCIA_LV1_RD_BASE and SCIA_LV1_WR_BASE
.ENVIRONment None
.VERSION      4.0   11-Oct-2005 use direct write, add usage of SCIA_LV1_ADD_DSD
              3.0   14-Apr-2005 added routine to write BASE-struct to file, RvH
              2.2   22-Mar-2002	test number of DSD; can be zero, RvH
              2.1   16-Jan-2002	more error return status checking, RvH 
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              1.1   04-Oct-2001	changed input/output, RvH 
              1.0   18-Dec-2000 created by R. M. van Hees
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
void Sun2Intel_BASE( struct base_scia *base )
{
     register int ni = 0;

     do {
	  IEEE_Swap__FLT( &base->wvlen_det_pix[ni] );
     } while ( ++ni < SCIENCE_PIXELS );
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_BASE
.PURPOSE     read the Precise Basis of the Spectral Calibration Parameters
.INPUT/OUTPUT
  call as   nr_dsr = SCIA_LV1_RD_BASE( fd, num_dsd, dsd, &base );
     input:  
            FILE   *fd             :  (open) stream pointer
	    unsigned int num_dsd   :  number of DSDs
	    struct dsd_envi dsd    :  structure for the DSDs
    output:  
            struct base_scia *base :  structure for the Spectral BASE

.RETURNS     number of data set records read (unsigned int)
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV1_RD_BASE( FILE *fd, unsigned int num_dsd, 
			       const struct dsd_envi *dsd,
			       struct base_scia *base )
{
     const char prognm[] = "SCIA_LV1_RD_BASE";

     size_t       dsr_size;
     unsigned int indx_dsd;

     const char dsd_name[] = "SPECTRAL_BASE";
/*
 * get index to data set descriptor
 */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL ) {
	  NADC_ERROR( prognm, NADC_ERR_PDS_RD, dsd_name );
	  return 0u;
     }
     if ( dsd[indx_dsd].num_dsr == 0 ) return 0u;
     dsr_size = (size_t) dsd[indx_dsd].dsr_size;
/*
 * rewind/read input data file into  BASE structure
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
     if ( fread( base->wvlen_det_pix, dsr_size, 1, fd ) != 1 ) {
	  NADC_ERROR( prognm, NADC_ERR_PDS_RD, "" );
	  return 0u;
     }
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_BASE( base );
#endif
/*
 * set return values
 */
     return 1u;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_BASE
.PURPOSE     write the Precise Basis of the Spectral Calibration Parameters
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_BASE( fd, base );
     input:  
            FILE   *fd             :  (open) stream pointer
            struct base_scia *base :  structure for the Spectral BASE

.RETURNS     nothing
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1_WR_BASE( FILE *fd, const struct base_scia *base_in )
{
     const char prognm[] = "SCIA_LV1_WR_BASE";

     size_t nr_byte;

     struct base_scia base;

     struct dsd_envi dsd = {
	  "SPECTRAL_BASE", "G",
	  "                                                              ",
	  0u, 0u, 0u, 0
     };
/*
 * write BASE structure to data buffer
 */
     (void) memcpy( &base, base_in, sizeof( struct base_scia ) );
#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_BASE( &base );
#endif
     nr_byte = (size_t) (SCIENCE_PIXELS * ENVI_FLOAT);
     if ( fwrite( base.wvlen_det_pix, nr_byte, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
/*
 * update list of written DSD records
 */
     dsd.num_dsr = 1u;
     dsd.dsr_size = (int) dsd.size;
     SCIA_LV1_ADD_DSD( &dsd );
}
