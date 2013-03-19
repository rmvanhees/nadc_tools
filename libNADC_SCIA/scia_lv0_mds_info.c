/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2001 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV0_MDS_INFO
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 0 data
.LANGUAGE    ANSI C
.PURPOSE     obtain inventarisation of SCIA LV0 product
.COMMENTS    contains SCIA_LV0_RD_MDS_INFO

             The information stored in the info-records allows quick access
             to selected MDS records, without reading first the whole file.
.ENVIRONment None
.VERSION      2.5.1 20-Apr-2006	minor bug-fix to compile without HDF5, RvH
              2.5   16-Jan-2005	update documentation, RvH
              2.4   10-Jan-2005	store/read info-records in database, RvH
              2.3   01-Apr-2003	modified function parameters, RvH
              2.2   20-Dec-2001	renamed module, RvH 
              2.1   12-Dec-2001	updated documentation, RvH 
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              1.0   01-Nov-2001	Created by R. M. van Hees 
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_0
#include <nadc_scia.h>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_RD_MDS_INFO
.PURPOSE     obtain inventarisation of Sciamachy LV0 product
.INPUT/OUTPUT
  call as   nr_info = SCIA_LV0_RD_MDS_INFO( fd, num_dsd, dsd, info );
     input:
             FILE *fd                  : (open) stream pointer to scia-file
             unsigned int num_dsd      : number of DSD records
	     struct dsd_envi *dsd      : structure for the DSD records
    output:
             struct mds0_info **info   : structure for "info" records

.RETURNS     number of info records found (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV0_RD_MDS_INFO( FILE *fd, unsigned int num_dsd,
				   const struct dsd_envi *dsd, 
				   struct mds0_info **info_out )
{
     const char prognm[] = "SCIA_LV0_RD_MDS_INFO";

     unsigned int indx_dsd;
     unsigned int num_info;

     struct mph_envi  mph;
     struct mds0_info *info;

     const char dsd_name[] = "SCIAMACHY_SOURCE_PACKETS";
/*
 * read Main Product Header
 */
     ENVI_RD_MPH( fd, &mph );
/*
 * get index to data set descriptor
 */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_ABSENT ) {
	  NADC_ERROR( prognm, NADC_ERR_PDS_RD, dsd_name );
	  return 0u;
     }
/*
 * allocate memory to store info-records
 */
     num_info = dsd[indx_dsd].num_dsr;
     if ( ! Use_Extern_Alloc ) {
	  info_out[0] = (struct mds0_info *) 
	       malloc( (size_t) num_info * sizeof(struct mds0_info) );
     }
     if ( (info = info_out[0]) == NULL ) {
	  NADC_ERROR( prognm, NADC_ERR_ALLOC, "mds0_info" );
	  return 0u;
     }
/*
 * extract info-records from input file
 */
     return GET_SCIA_LV0_MDS_INFO( fd, mph, &dsd[indx_dsd], info );
}
