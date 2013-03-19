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

.IDENTifer   SCIA_LV1_PATCH_HDR
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIAMACHY
.LANGUAGE    ANSI C
.PURPOSE     copy/patch PDS headers between files, except DSD records

.INPUT/OUTPUT
  call as   SCIA_LV1_PATCH_HDR( mph, fp_in, fp_out );

     input: 
             struct mph_envi mph        :  Main Product Header
 in/output:  
	     FILE            *fp_in     :  file-pointer to input file
	     FILE            *fp_out    :  file-pointer to output file

.RETURNS     nothing
             error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     2.1     07-Dec-2005   DSD records are handled by separate 
	                           functions, RvH
             2.0     21-Apr-2005   rewrite with the new PDS write-routines, RvH
             1.0     15-Jan-2004   initial release by R. M. van Hees
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
#include <defs_nadc.h>
#include "../VERSION"

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static 
void SCIA_PATCH_MPH( struct mph_envi  *mph )
{
/*      const char prognm[] = "SCIA_PATCH_MPH"; */

     char   *cpntr;
     size_t nbyte, nspace;
/*
 * modify Product File name
 */
     if ( strncmp( mph->product, "SCI_NL__1P", 10 ) == 0 )
	  mph->product[10] = SCIA_PATCH_ID[0];
/*
 * modify Processing Stage Flag to "Special product"
 */
     (void) strlcpy( mph->proc_stage, SCIA_PATCH_ID, 2 );
/*
 * append patch version to Software Version number of processing software
 */
     nbyte = strlen(mph->soft_version);
     cpntr = strchr( mph->soft_version, ' ' );
     nspace = nbyte - (cpntr - mph->soft_version);
     (void) snprintf( cpntr, nspace+1, "p%-6s", patch_version );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_PATCH_HDR( FILE *fp_in, FILE *fp_out )
{
     const char prognm[] = "SCIA_LV1_PATCH_HDR";

     struct mph_envi  mph;
     struct sph1_scia sph;
/*
 * copy MPH to output file
 */
     ENVI_RD_MPH( fp_in, &mph );
     if ( IS_ERR_STAT_FATAL ) 
          NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "MPH" );
     SCIA_PATCH_MPH( &mph );
     ENVI_WR_MPH( fp_out, mph );
     if ( IS_ERR_STAT_FATAL ) 
          NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "MPH" );
/*
 * copy SPH to output file
 */
     SCIA_LV1_RD_SPH( fp_in, mph, &sph );
     if ( IS_ERR_STAT_FATAL ) 
          NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "SPH" );
     SCIA_LV1_WR_SPH( fp_out, mph, sph );
     if ( IS_ERR_STAT_FATAL ) 
          NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "SPH" );
}
