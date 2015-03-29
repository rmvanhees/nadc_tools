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

.IDENTifer   GET_SCIA_LV1C_POLV
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIAMACHY level 1b/c
.LANGUAGE    ANSI C
.PURPOSE     extract polarisation datasets from the level 1b MDS
.INPUT/OUTPUT
  call as    nr_mds = GET_SCIA_LV1C_POLV( state, mds_1b, mds_polV );

     input:
	    struct state1_scia *state : structure with States of the product
            struct mds1_scia *mds_1b  : structure holding level 1b MDS records
    output:
	    struct mds1c_polV *mds_polV : level 1c Polarisation MDS record

.RETURNS     number of polarisation records (=1)
.COMMENTS    none
.ENVIRONment None
.VERSION      1.4   17-Oct-2005 pass state-record by reference, RvH
              1.3   13-Oct-2005 obtain MJD from state definition, RvH
              1.2   11-Oct-2005 fixed serious bug 
                                while writing PolV records, RvH
              1.1   20-Oct-2004 modified structure mds1c_polV, RvH
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
unsigned int GET_SCIA_LV1C_POLV( const struct state1_scia *state,
				 const struct mds1_scia *mds_1b, 
				 struct mds1c_polV *mds_polV )
{
     register unsigned short nr;
     register unsigned short n_pol_intg, indx_pol_1b, indx_pol_1c;
     register unsigned int   ni;

/* constants */
     const unsigned int geoL_size = 112u;
     const unsigned int geoN_size = 108u;
     const unsigned int polV_size = 256u;
/*
 * initialize MDS-structure
 */
     mds_polV->total_polV = 0u;
     mds_polV->num_geo = 0u;
     mds_polV->polV = NULL;
     mds_polV->geoN = NULL;
     mds_polV->geoL = NULL;
/*
 * reorganise level 1b MDS struct into level 1c MDS-POLV struct
 */
     (void) memcpy( &mds_polV->mjd, &state->mjd, sizeof( struct mjd_envi ));
     mds_polV->quality_flag = mds_1b->quality_flag;
     mds_polV->type_mds = state->type_mds;
     mds_polV->category = (unsigned char) state->category;
     mds_polV->state_id = state->state_id;
     mds_polV->state_index = state->indx;
     mds_polV->dur_scan = state->dur_scan;
     mds_polV->orbit_phase = state->orbit_phase;
     mds_polV->num_diff_intg = state->num_intg;

     ni = 0;
     mds_polV->total_polV = 0;
     do {
	  mds_polV->total_polV += state->num_polar[ni]; 
	  mds_polV->intg_times[ni] = state->intg_times[ni];
	  mds_polV->num_polar[ni] = state->num_polar[ni];
     } while ( ++ni < state->num_intg );
     mds_polV->num_geo = mds_polV->total_polV;
/*
 * allocate memory for polarisation values and geolocations
 */
     mds_polV->polV = (struct polV_scia *)
	  malloc(mds_polV->total_polV * sizeof(struct polV_scia));
     if ( mds_polV->polV == NULL ) NADC_GOTO_ERROR( NADC_ERR_ALLOC, "polV" );
     switch ( (int) mds_1b->type_mds ) {
     case SCIA_NADIR:
	  mds_polV->dsr_length = 289u +
	       (polV_size + geoN_size) * mds_polV->total_polV ;

	  mds_polV->geoN = (struct geoN_scia *)
	       malloc(mds_polV->total_polV * sizeof(struct geoN_scia));
	  if ( mds_polV->geoN == NULL )
	       NADC_GOTO_ERROR( NADC_ERR_ALLOC, "geoN" );
	  break;
     case SCIA_LIMB:
     case SCIA_OCCULT:
	  mds_polV->dsr_length = 289u +
	       (polV_size + geoL_size) * mds_polV->total_polV ;

	  mds_polV->geoL = (struct geoL_scia *)
	       malloc(mds_polV->total_polV * sizeof(struct geoL_scia));
	  if ( mds_polV->geoL == NULL )
	       NADC_GOTO_ERROR( NADC_ERR_ALLOC, "geoL" );
	  break;
     }
/*
 * loop over the integration times
 */
     ni = 0;
     indx_pol_1b = 0;
     indx_pol_1c = 0;
     do {
	  n_pol_intg = state->num_polar[ni] / state->num_dsr;

	  nr = 0;
	  do {
	       (void) memcpy( &mds_polV->polV[indx_pol_1c],
			      &mds_1b[nr].polV[indx_pol_1b],
			      n_pol_intg * sizeof( struct polV_scia ));
	       switch ( (int) mds_1b->type_mds ) {
	       case SCIA_NADIR:
		    GET_SCIA_LV1C_GEON( mds_1b[nr].n_aux, mds_1b[nr].geoN,
					n_pol_intg, 
					&mds_polV->geoN[indx_pol_1c] );
		    break;
	       case SCIA_LIMB:
		    GET_SCIA_LV1C_GEOL( mds_1b[nr].n_aux, mds_1b[nr].geoL,
					n_pol_intg, 
					&mds_polV->geoL[indx_pol_1c] );
		    break;
	       case SCIA_OCCULT:
		    GET_SCIA_LV1C_GEOL( mds_1b[nr].n_aux, mds_1b[nr].geoL,
					n_pol_intg, 
					&mds_polV->geoL[indx_pol_1c] );
		    break;
	       }
	       indx_pol_1c += n_pol_intg;
	  } while( ++nr < state->num_dsr );
	  indx_pol_1b += n_pol_intg;
     } while ( ++ni < state->num_intg );

     return 1u;
 done:
     return 0u;
}
