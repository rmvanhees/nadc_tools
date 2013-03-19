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

.IDENTifer   GET_SCIA_LV1C_PMD
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIAMACHY level 1b/c
.LANGUAGE    ANSI C
.PURPOSE     extract PMD data from the level 1b MDS
.INPUT/OUTPUT
  call as    nr_mds = GET_SCIA_LV1C_PMD( state, mds_1b, mds_pmd );

     input:
	    struct state1_scia *state : structure with States of the product
            struct mds1_scia *mds_1b  : structure holding level 1b MDS records
    output:
	    struct mds1c_pmd *mds_pmd : level 1c PMD MDS record

.RETURNS     number of PMD records (=1)
.COMMENTS    none
.ENVIRONment None
.VERSION      1.4   17-Oct-2005 pass state-record by reference, RvH
              1.3   13-Oct-2005 obtain MJD from state definition, RvH
              1.2   11-Oct-2005 fixed serious bug 
                                while writing PMD records, RvH
              1.1   20-Oct-2004 modified structure mds1c_pmd, RvH
              1.0   07-Nov-2003 created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <string.h>
#include <stdlib.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned int GET_SCIA_LV1C_PMD( const struct state1_scia *state,
				const struct mds1_scia *mds_1b, 
				struct mds1c_pmd *mds_pmd )
{
     const char prognm[] = "GET_SCIA_LV1C_PMD";

     register unsigned short ng, nr;
     register unsigned short n_pmd_geo;

/* constants */
     const unsigned int geoL_size = 112u;
     const unsigned int geoN_size = 108u;
/*
 * initialize MDS-structure
 */
     mds_pmd->num_geo = 0u;
     mds_pmd->num_pmd = 0u;
     mds_pmd->int_pmd = NULL;
     mds_pmd->geoN = NULL;
     mds_pmd->geoL = NULL;
/*
 * reorganise level 1b MDS struct into level 1c MDS-PMD struct
 */
     (void) memcpy( &mds_pmd->mjd, &state->mjd, sizeof( struct mjd_envi ));
     mds_pmd->quality_flag = mds_1b->quality_flag;
     mds_pmd->type_mds = mds_1b->type_mds;
     mds_pmd->category = (unsigned char) state->category;
     mds_pmd->state_id = mds_1b->state_id;
     mds_pmd->state_index = mds_1b->state_index;
     mds_pmd->dur_scan = state->dur_scan;
     mds_pmd->num_pmd = state->num_pmd * PMD_NUMBER;
     mds_pmd->num_geo = state->num_pmd;
     mds_pmd->orbit_phase = state->orbit_phase;

     switch ( (int) mds_1b->type_mds ) {
     case SCIA_NADIR:
	  mds_pmd->dsr_length = 31u + sizeof( float ) * mds_pmd->num_pmd 
	       + geoN_size * mds_pmd->num_geo;

	  mds_pmd->geoN = (struct geoN_scia *)
	       malloc(mds_pmd->num_geo * sizeof(struct geoN_scia));
	  if ( mds_pmd->geoN == NULL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "geoN" );

	  for ( ng = nr = 0; nr < state->num_dsr; nr++ ) {
	       n_pmd_geo = mds_1b[nr].n_pmd / PMD_NUMBER;
	       GET_SCIA_LV1C_GEON( mds_1b[nr].n_aux, mds_1b[nr].geoN,
				   n_pmd_geo, mds_pmd->geoN+ng );
	       ng += n_pmd_geo;
	  }
	  break;
     case SCIA_LIMB:
     case SCIA_OCCULT:
	  mds_pmd->dsr_length = 31u + sizeof( float ) * mds_pmd->num_pmd 
	       + geoL_size * mds_pmd->num_geo;
	  mds_pmd->geoL = (struct geoL_scia *)
	       malloc(mds_pmd->num_geo * sizeof(struct geoL_scia));
	  if ( mds_pmd->geoL == NULL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "geoL" );
	  for ( ng = nr = 0; nr < state->num_dsr; nr++ ) {
	       n_pmd_geo = mds_1b[nr].n_pmd / PMD_NUMBER;

	       GET_SCIA_LV1C_GEOL( mds_1b[nr].n_aux, mds_1b[nr].geoL,
				   n_pmd_geo, mds_pmd->geoL+ng );
	       ng += n_pmd_geo;
	  }
	  break;
     }
     mds_pmd->int_pmd = (float *)
	  malloc( mds_pmd->num_pmd * sizeof( float ));
     if ( mds_pmd->int_pmd == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "int_pmd" );
     for ( ng = nr = 0; nr < state->num_dsr; nr++ ) {
	  (void) memcpy( mds_pmd->int_pmd+ng, mds_1b[nr].int_pmd,
			 mds_1b[nr].n_pmd * sizeof( float ) );
	  ng += mds_1b[nr].n_pmd;
     }
     return 1u;
 done:
     return 0u;
}
