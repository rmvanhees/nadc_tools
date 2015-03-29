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

.IDENTifer   GET_SCIA_LV1C_MDS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIAMACHY level 1b/c
.LANGUAGE    ANSI C
.PURPOSE     convert MDS in level 1b format to level 1c format
.INPUT/OUTPUT
  call as    nr_mds = GET_SCIA_LV1C_MDS( clus_mask, state, mds_1b, mds_1c );
     input:
            ulong64 clus_mask         : mask for cluster selection
 in/output:
	    struct state1_scia *state : structure with States of the product
            struct mds1_scia *mds_1b  : structure holding level 1b MDS records
    output:
            struct mds1c_scia *mds_1c : level 1c MDS records

.RETURNS     number of level 1c MDS records (unsigned int), 
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
.ENVIRONment None
.VERSION      3.0   30-Aug-2013 added selection on clusters, RvH
              2.0   07-Dec-2005 removed esig/esigc from MDS(1b)-struct,
				renamed pixel_val_err to pixel_err, RvH
              1.3   17-Oct-2005 pass state-record by reference, RvH
              1.2   13-Oct-2005 obtain MJD from state definition, RvH
              1.1   06-Oct-2004 modified structure mds1c_scia, RvH
              1.0   09-Nov-2003 created by R. M. van Hees
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
#define absPixelID(Clcon) ((unsigned short) \
             ((Clcon.pixel_nr + CHANNEL_SIZE * ((int) Clcon.channel - 1))))

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned int GET_SCIA_LV1C_MDS( unsigned long long clus_mask,
				struct state1_scia *state,
				struct mds1_scia *mds_1b, 
				struct mds1c_scia *mds_1c )
{
     register unsigned int   nc, nd, ng;
     register unsigned short nb, np;

     register unsigned short nclus = 0;

     unsigned int nr_mds = 0u;

     size_t nrpix;

/* constants */
     const unsigned int geoC_size = 20u;
     const unsigned int geoL_size = 112u;
     const unsigned int geoN_size = 108u;
/*
 * reorganise level 1b struct into level 1c struct
 */	  
     do {
	  unsigned char indx_mask = state->Clcon[nclus].id - 1;

	  if ( Get_Bit_LL( clus_mask, indx_mask ) == 0ULL ) {
	       nd = 0;
	       do {
		    if ( mds_1b[nd].clus[nclus].n_sig != 0 ) {
			 free( mds_1b[nd].clus[nclus].sig );
			 mds_1b[nd].clus[nclus].n_sig = 0;
		    }
		    if ( mds_1b[nd].clus[nclus].n_sigc != 0 ) {
			 free( mds_1b[nd].clus[nclus].sigc );
			 mds_1b[nd].clus[nclus].n_sigc = 0;
		    }
	       } while ( ++nd < state->num_dsr );  
	       continue;
	  }
	  (void) memcpy( &mds_1c->mjd, &state->mjd, sizeof( struct mjd_envi ));
	  mds_1c->rad_units_flag = CHAR_ZERO;
	  mds_1c->quality_flag   = mds_1b->quality_flag;
	  mds_1c->type_mds       = state->type_mds;
	  mds_1c->category       = (unsigned char) state->category;
	  mds_1c->state_id       = state->state_id;
	  mds_1c->state_index    = state->indx;
	  mds_1c->chan_id        = state->Clcon[nclus].channel;
	  mds_1c->clus_id        = state->Clcon[nclus].id;
	  mds_1c->coaddf         = (unsigned char) state->Clcon[nclus].coaddf;
	  mds_1c->pet            = state->Clcon[nclus].pet;
	  mds_1c->num_obs        = state->num_dsr * state->Clcon[nclus].n_read;
	  mds_1c->num_pixels     = state->Clcon[nclus].length;
	  mds_1c->dur_scan       = state->dur_scan;
	  mds_1c->orbit_phase    = state->orbit_phase;
/*
 * allocate memory for pointers in the level 1c MDS structure
 */
	  switch ( (int) state->type_mds ) {
	  case SCIA_NADIR:
	       mds_1c->dsr_length = 32u + 10u * mds_1c->num_pixels
		    + 8u * mds_1c->num_obs * mds_1c->num_pixels
		    + geoN_size * mds_1c->num_obs;

	       mds_1c->geoN = (struct geoN_scia *)
		    malloc(mds_1c->num_obs * sizeof(struct geoN_scia));
	       if ( mds_1c->geoN == NULL )
		    NADC_GOTO_ERROR( NADC_ERR_ALLOC, "geoN" );
	       mds_1c->geoL = NULL;
	       mds_1c->geoC = NULL;
	       break;
	  case SCIA_LIMB:
	       mds_1c->dsr_length = 32u + 10u * mds_1c->num_pixels
		    + 8u * mds_1c->num_obs * mds_1c->num_pixels 
		    + geoL_size * mds_1c->num_obs;

	       mds_1c->geoL = (struct geoL_scia *)
		    malloc(mds_1c->num_obs * sizeof(struct geoL_scia));
	       if ( mds_1c->geoL == NULL )
		    NADC_GOTO_ERROR( NADC_ERR_ALLOC, "geoL" );
	       mds_1c->geoN = NULL;
	       mds_1c->geoC = NULL;
	       break;
	  case SCIA_OCCULT:
	       mds_1c->dsr_length = 32u + 10u * mds_1c->num_pixels
		    + 8u * mds_1c->num_obs * mds_1c->num_pixels 
		    + geoL_size * mds_1c->num_obs;

	       mds_1c->geoL = (struct geoL_scia *)
		    malloc(mds_1c->num_obs * sizeof(struct geoL_scia));
	       if ( mds_1c->geoL == NULL )
		    NADC_GOTO_ERROR( NADC_ERR_ALLOC, "geoL" );
	       mds_1c->geoN = NULL;
	       mds_1c->geoC = NULL;
	       break;
	  case SCIA_MONITOR:
	       mds_1c->dsr_length = 32u + 10u * mds_1c->num_pixels
		    + 8u * mds_1c->num_obs * mds_1c->num_pixels 
		    + geoC_size * mds_1c->num_obs;

	       mds_1c->geoC = (struct geoC_scia *)
		    malloc(mds_1c->num_obs * sizeof(struct geoC_scia));
	       if ( mds_1c->geoC == NULL )
		    NADC_GOTO_ERROR( NADC_ERR_ALLOC, "geoC" );
	       mds_1c->geoN = NULL;
	       mds_1c->geoL = NULL;
	       break;
	  }
	  nrpix = (size_t) mds_1c->num_pixels;
	  mds_1c->pixel_ids = (unsigned short *) malloc( nrpix * sizeof(short));
	  if ( mds_1c->pixel_ids == NULL )
	       NADC_GOTO_ERROR( NADC_ERR_ALLOC, "pixel_ids" );
	  mds_1c->pixel_wv = (float *) calloc( nrpix, sizeof( float ));
	  if ( mds_1c->pixel_wv == NULL )
	       NADC_GOTO_ERROR( NADC_ERR_ALLOC, "pixel_wv" );
	  mds_1c->pixel_wv_err = (float *) calloc( nrpix, sizeof( float ));
	  if ( mds_1c->pixel_wv_err == NULL )
	       NADC_GOTO_ERROR( NADC_ERR_ALLOC, "pixel_wv_err" );

	  nrpix = (size_t) (mds_1c->num_obs * mds_1c->num_pixels);
	  mds_1c->pixel_val = (float *) malloc( nrpix * sizeof( float ));
	  if ( mds_1c->pixel_val == NULL )
	       NADC_GOTO_ERROR( NADC_ERR_ALLOC, "pixel_val" );
	  mds_1c->pixel_err = (float *) calloc( nrpix, sizeof( float ));
	  if ( mds_1c->pixel_err == NULL )
	       NADC_GOTO_ERROR( NADC_ERR_ALLOC, "pixel_err" );
/*
 * set pixel ID and initialize wavelength arrays
 */
	  np = 0;
	  do {
	       mds_1c->pixel_ids[np] = absPixelID(state->Clcon[nclus]) + np;
	  } while( ++np < mds_1c->num_pixels );
/*
 * go through all the cluster DataSet Records
 */
	  nd = 0;        /* counter for the number of DSR's */
	  nc = 0;        /* counter for the number of level 1b Clusters */
	  ng = 0;        /* counter for the number of Geolocation records */
	  do {
/*
 * calculate geolocation for each cluster
 */
	       switch( (int) state->type_mds ) {
	       case SCIA_NADIR:
		    GET_SCIA_LV1C_GEON( mds_1b[nd].n_aux, mds_1b[nd].geoN,
					state->Clcon[nclus].n_read,
					mds_1c->geoN+ng );
		    break;
	       case SCIA_LIMB:
	       case SCIA_OCCULT:
		    GET_SCIA_LV1C_GEOL( mds_1b[nd].n_aux, mds_1b[nd].geoL,
					state->Clcon[nclus].n_read, 
					mds_1c->geoL+ng );
		    break;
	       case SCIA_MONITOR:
		    GET_SCIA_LV1C_GEOC( mds_1b[nd].n_aux, mds_1b[nd].geoC,
					state->Clcon[nclus].n_read, 
					mds_1c->geoC+ng );
		    break;
	       }
	       ng += state->Clcon[nclus].n_read;
/*
 * copy raw detector counts from 1b-struct to 1c-struct
 */
	       for ( nb = 0; nb < mds_1b[nd].clus[nclus].n_sig; nb++ ) {
		    mds_1c->pixel_val[nc++] = (float) 
			 mds_1b[nd].clus[nclus].sig[nb].sign;
	       }
	       for ( nb = 0; nb < mds_1b[nd].clus[nclus].n_sigc; nb++ ) {
		    mds_1c->pixel_val[nc++] = (float) 
			 mds_1b[nd].clus[nclus].sigc[nb].det.field.sign;
	       }
	  } while ( ++nd < state->num_dsr );

	  /* update State & MDS(1B) records*/
	  if ( nr_mds < nclus ) {
	       (void) memcpy( &state->Clcon[nr_mds], &state->Clcon[nclus],
			      sizeof( struct Clcon_scia ) );

	       nd = 0;
	       do {
		    (void) memcpy( &mds_1b[nd].clus[nr_mds],
				   &mds_1b[nd].clus[nclus],
				   sizeof( struct Clus_scia ) );
		    /* no double free */
		    mds_1b[nd].clus[nclus].n_sig = 0;
		    mds_1b[nd].clus[nclus].n_sigc = 0;
	       } while ( ++nd < state->num_dsr );  
	  }

	  /* succesfully filled a MDS record */
	  nr_mds++;
	  mds_1c++;

     } while ( ++nclus < state->num_clus );
 
     /* update number of clusters in State-record */
     state->num_clus = (unsigned short) nr_mds;
/*
 * return number of MDS records
 */
     return nr_mds;
done:
     return 0u;
}
