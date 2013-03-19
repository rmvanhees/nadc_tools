/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2003 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV1_PATCH_ADS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIAMACHY
.LANGUAGE    ANSI C
.PURPOSE     copy/patch PDS annotation datasets between files

.INPUT/OUTPUT
  call as   SCIA_LV1_PATCH_ADS( patch_scia, orbit, dsd, fp_in, fp_out );
     input:
             unsigned short  patch_scia :  flag defining which patches to apply
	     int             orbit      :  Absolute Orbit number
             struct dsd_envi dsd        :  DataSet descriptor
 in/output:  
	     FILE            *fp_in     :  file-pointer to input file
	     FILE            *fp_out    :  file-pointer to output file

.RETURNS     nothing
             error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     1.6     07-Dec-2005   add DSD's with function: SCIA_LV1_ADD_DSD, 
                                   RvH
             1.5     07-Feb-2005   more checks on return status of functions,
                                   added patching of PPG values, RvH
             1.3.1   26-May-2004   bugfix: index offset with PMD_4 in SIP, RvH
             1.3     07-Apr-2004   added patching of orbital darks, RvH
             1.2     31-Mar-2004   added patching of Frac Polarisation, RvH
             1.1     30-Mar-2004   added patching of SPECTRAL_BASE, RvH
             1.0     06-Oct-2003   initial release by R. M. van Hees
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
#define _SCIA_PATCH_1
#include <nadc_scia_patch.h>

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void SCIA_LV1_PATCH_ADS( unsigned short patch_scia, 
			 const struct dsd_envi dsd, 
			 FILE *fp_in, FILE *fp_out )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp_in, fp_out@*/
{
     const char prognm[] = "SCIA_LV1_PATCH_ADS";

     char   *ds_buff;
     size_t nr_byte;
/*
 * check size of current Dataset
 */
     if ( dsd.size == 0 ) return;
/*
 * do not process Measurement Data Sets (for now only level 1)
 */
     if ( strcmp( dsd.name, "NADIR" ) == 0 ) return;
     if ( strcmp( dsd.name, "LIMB" ) == 0 ) return;
     if ( strcmp( dsd.name, "OCCULTATION" ) == 0 ) return;
     if ( strcmp( dsd.name, "MONITORING" ) == 0 ) return;
/*
 * allocate memory to temporary store data for output structure
 */
     if ( (ds_buff = (char *) malloc( (size_t) dsd.size )) == NULL ) 
          NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "ds_buff" );
/*
 * rewind/read DS from in_file
 */
     (void) fseek( fp_in, (long) dsd.offset, SEEK_SET );
     nr_byte = fread( ds_buff, sizeof( char ), (size_t) dsd.size, fp_in );
     if ( nr_byte != (size_t) dsd.size )
       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, dsd.name );
/*
 * patch fractional polarisation values
 */
     if ( (patch_scia & SCIA_PATCH_POL) != USHRT_ZERO 
	  && strcmp( dsd.name, "INSTRUMENT_PARAMS" ) == 0 ) {
	  ds_buff[249 + (PMD_4-1)] = 't';
	  NADC_ERROR( prognm, NADC_ERR_NONE, 
		      "\n\tapplied Polarisation Improvements from J.M. Krijger"
		      " (SRON), Feb-2004" );
     }
/*
 * write the DS to out_file
 */
     (void) fseek( fp_out, (long) dsd.offset, SEEK_SET );
     if (  fwrite( ds_buff, (size_t) dsd.size, 1, fp_out ) == 1 )
	  SCIA_LV1_ADD_DSD( &dsd );
     else
	  NADC_ERROR( prognm, NADC_ERR_PDS_WR, dsd.name );
 done:
     free( ds_buff );
}
