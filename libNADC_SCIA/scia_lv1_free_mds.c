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

.IDENTifer   SCIA_LV1_FREE_MDS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1b/1c data
.LANGUAGE    ANSI C
.PURPOSE     free memory allocated by SCIA_LV1_RD_MDS, SCIA_LV1C_RD_MDS, etc
.RETURNS     nothing
.COMMENTS    contains SCIA_LV1_FREE_MDS, SCIA_LV1C_FREE_MDS, 
               SCIA_LV1C_FREE_MDS_PMD, SCIA_LV1C_FREE_MDS_POLV
.ENVIRONment None
.VERSION      5.0   17-Dev-2005 removed esig/esigc from MDS(1b)-struct,
				renamed pixel_val_err to pixel_err, RvH
              4.3   17-Oct-2005	SCIA_LV1_FREE_MDS don't use break in inner-loop
                                reverse previous patch: 
                       only process one MDS_PMD or one MDS_POLV record per call
              4.2   11-Oct-2005	allow arrays of PMD/PolV records to function 
                                calls of SCIA_LV1C_FREE_MDS_PMD and
                                SCIA_LV1C_FREE_MDS_POLV, RvH
              4.1   27-Mar-2002	add releasing memory allocated for level 1c 
                                specific DSD's, RvH
              4.0   27-Mar-2002	separate modules for reading MDS, 
                                and releasing memory allocated for MDS, RvH
              3.3   26-Mar-2002	added read routine for level 1c Nadir MDS, RvH
              3.2   07-Mar-2002	gave MDS structs more logical names, RvH 
              3.1   28-Feb-2002	debugging and optimalisation, RvH 
              3.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              2.2   03-Aug-2001	lots of debugging and tuning, RvH 
              2.1   09-Jan-2001 combined SCIA_LV1_RD_NADIR, READ_LIMB, ...
                                to one general SCIA_LV1_RD_MDS routine, RvH
              2.0   21-Dec-2000 updated SCIA_L01 01.00 format, RvH
              1.0   09-Nov-2000 created by R. M. van Hees
------------------------------------------------------------*/
/*
 * This program is not comform the POSIX standard
 * due to the use of "snprintf"
 */

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_FREE_MDS
.PURPOSE     free memory allocated by SCIA_LV1_RD_MDS
.INPUT/OUTPUT
  call as   SCIA_LV1_FREE_MDS( source, nr_mds, mds );
     input:  
            int    source          : source of MDS (Nadir, Limb, ... )
            unsigned short nr_mds  : number of level 1b MDS
    output:  
            struct mds1_scia *mds  : level 1b MDS records

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void SCIA_LV1_FREE_MDS( int source, unsigned int nr_mds, 
			struct mds1_scia *mds )
{
     register unsigned short nc;
     register unsigned int   nm;

     for ( nm = 0; nm < nr_mds; nm++ ) {
	  register bool last_clus = FALSE;

	  if ( mds[nm].n_aux > 0u ) {
	       switch ( source ) {
	       case SCIA_NADIR:
		    if ( mds[nm].geoN != NULL ) free( mds[nm].geoN );
		    break;
	       case SCIA_LIMB:
	       case SCIA_OCCULT:
		    if ( mds[nm].geoL != NULL ) free( mds[nm].geoL );
		    break;
	       case SCIA_MONITOR:
		    if ( mds[nm].geoC != NULL ) free( mds[nm].geoC );
		    break;
	       }
	       if ( mds[nm].sat_flags != NULL ) free( mds[nm].sat_flags );
	       if ( mds[nm].red_grass != NULL ) free( mds[nm].red_grass );
	       if ( mds[nm].lv0       != NULL ) free( mds[nm].lv0 );
	  }

	  if ( mds[nm].n_pol > 0u && mds[nm].polV != NULL ) 
	       free( mds[nm].polV );
	  if ( mds[nm].n_pmd > 0u && mds[nm].int_pmd != NULL ) 
	       free( mds[nm].int_pmd );

	  nc = 0;
	  do {
	       if ( mds[nm].clus[nc].sig != NULL )
		    free( mds[nm].clus[nc].sig );
	       else if ( mds[nm].clus[nc].sigc != NULL )
		    free( mds[nm].clus[nc].sigc );
	       else 
		    last_clus = TRUE;
	  } while ( ! last_clus && ++nc < MAX_CLUSTER );
     }
     free( mds );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1C_FREE_MDS
.PURPOSE     free memory allocated by SCIA_LV1C_RD_MDS
.INPUT/OUTPUT
  call as   SCIA_LV1C_FREE_MDS( source, nr_mds, mds );
     input:  
            int          source    : source of MDS (Nadir, Limb, ... )
            unsigned int nr_mds    : number of level 1c MDS
    output:  
            struct mds1c_scia *mds : level 1c MDS records

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void SCIA_LV1C_FREE_MDS( int source, unsigned int nr_mds, 
			 struct mds1c_scia *mds )
{
     register unsigned int nm;

     for ( nm = 0; nm < nr_mds; nm++ ) {
	  if ( mds[nm].num_obs != 0 ) {
	       switch ( source ) {
	       case SCIA_NADIR:
		    if ( mds[nm].geoN != NULL ) free( mds[nm].geoN );
		    break;
	       case SCIA_LIMB:
	       case SCIA_OCCULT:
		    if ( mds[nm].geoL != NULL ) free( mds[nm].geoL );
		    break;
	       case SCIA_MONITOR:
		    if ( mds[nm].geoC != NULL ) free( mds[nm].geoC );
		    break;
	       }
	  }
	  if ( mds[nm].num_pixels != 0 ) {
	       if ( mds[nm].pixel_ids != NULL )
		    free( mds[nm].pixel_ids );
	       if ( mds[nm].pixel_wv != NULL )
		    free( mds[nm].pixel_wv );
	       if ( mds[nm].pixel_wv_err != NULL )
		    free( mds[nm].pixel_wv_err );
	       if ( mds[nm].num_obs != 0 ) {
		    if ( mds[nm].pixel_val != NULL )
			 free( mds[nm].pixel_val );
		    if ( mds[nm].pixel_err != NULL )
			 free( mds[nm].pixel_err );
	       }
	  }
     }
     free( mds );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1C_FREE_MDS_PMD
.PURPOSE     free memory allocated by SCIA_LV1C_RD_MDS_PMD
.INPUT/OUTPUT
  call as   SCIA_LV1C_FREE_MDS_PMD( source, pmd );
     input:  
            int          source   : source of MDS (Nadir, Limb, ... )
    output:  
            struct mds1c_pmd *pmd : level 1c PMD MDS records

.RETURNS     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1C_FREE_MDS_PMD( int source, struct mds1c_pmd *pmd )
{
     if ( pmd->num_geo != 0 ) {
	  switch ( source ) {
	  case SCIA_NADIR:
	       if ( pmd->geoN != NULL ) free( pmd->geoN );
	       break;
	  case SCIA_LIMB:
	  case SCIA_OCCULT:
	       if ( pmd->geoL != NULL ) free( pmd->geoL );
	       break;
	  }
     }
     if ( pmd->num_pmd != 0 && pmd->int_pmd != NULL ) free( pmd->int_pmd );

     free( pmd );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1C_FREE_MDS_POLV
.PURPOSE     free memory allocated by SCIA_LV1C_RD_MDS_POLV
.INPUT/OUTPUT
  call as   SCIA_LV1C_FREE_MDS_POLV( source, num_pol, polV );
     input:  
            int          source     : source of MDS (Nadir, Limb, ... )
    output:  
            struct mds1c_polV *polV : level 1c frac polarisation MDS records

.RETURNS     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1C_FREE_MDS_POLV( int source, struct mds1c_polV *polV )
{
     if ( polV->num_geo != 0 ) {
	  switch ( source ) {
	  case SCIA_NADIR:
	       if ( polV->geoN != NULL ) free( polV->geoN );
	       break;
	  case SCIA_LIMB:
	  case SCIA_OCCULT:
	       if ( polV->geoL != NULL ) free( polV->geoL );
	       break;
	  }
     }
     if ( polV->total_polV != 0 && polV->polV != NULL ) free( polV->polV );

     free( polV );
}
