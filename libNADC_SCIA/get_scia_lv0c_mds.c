/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2006 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   GET_SCIA_LV0C_MDS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIAMACHY level 0/1c
.LANGUAGE    ANSI C
.PURPOSE     convert L0 MDS_DET to L1C structure
.INPUT/OUTPUT
  call as    nr_mds = GET_SCIA_LV0C_MDS( nr_det, det, mds_1c );
     input:  
	     unsigned int nr_det       : number of MDS_DET structures
             struct mds0_det *det      : Detector MDS records (level 0)
    output:  
             struct mds1c_scia *mds_1c : level 1c MDS records

.RETURNS     number of level 1c MDS records
.COMMENTS    None
.ENVIRONment None
.VERSION     1.1.1   29-Sep-2011   moved clusID check to read module, RvH
             1.1     29-Sep-2011   added several checks, RvH
             1.0     07-Nov-2006   created by R. M. van Hees 
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
#define _SCIA_LEVEL_0
#include <nadc_scia.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static inline
unsigned short GET_INFO_CLUSDEF( unsigned short stateIndex, 
                                 unsigned int nr_info, 
                                 const struct mds0_info *info,
                                 /*@NULL@*/ /*@out@*/
                                 struct clusdef_rec *clusDef )
{
     register unsigned char ncl, nch;
     register unsigned int  ni;

     unsigned short numClus = 0;

     for ( nch = 1; nch <= SCIENCE_CHANNELS; nch++ ) {
          unsigned char clusIDmx = 0;
          for ( ni = 0; ni < nr_info; ni++ ) {
               if ( info[ni].stateIndex != stateIndex ) continue;

               for ( ncl = 0 ; ncl < info[ni].numClusters; ncl++ ) {
                    if ( info[ni].cluster[ncl].chanID == nch 
                         && info[ni].cluster[ncl].clusID == clusIDmx ) {
                         clusIDmx = info[ni].cluster[ncl].clusID;
                         if ( clusDef != NULL ) {
                              clusDef[numClus+clusIDmx].chanID = 
                                   info[ni].cluster[ncl].chanID;
                              clusDef[numClus+clusIDmx].clusID =
                                   info[ni].cluster[ncl].clusID;
                              clusDef[numClus+clusIDmx].start  =
                                   info[ni].cluster[ncl].start;
                              clusDef[numClus+clusIDmx].length =
                                   info[ni].cluster[ncl].length;
                         }
                         clusIDmx += UCHAR_ONE;
                    }
               }
          }
          numClus += clusIDmx;
     }
     return numClus;
}

static inline
unsigned short GET_DET_BCPS( const struct clusdef_rec *clusDef, 
			     unsigned int num_det, const struct mds0_det *det )
{
     register unsigned short nr, nch, ncl;

     for ( nr = 0; nr < num_det; nr++ ) {
	  for ( nch = 0; nch < det[nr].num_chan; nch++ ) {
	       if ( det[nr].data_src[nch].hdr.channel.field.id == clusDef->chanID ) {
		    for ( ncl = 0; ncl < det[nr].data_src[nch].hdr.channel.field.clusters; ncl++) {
			 if ( det[nr].data_src[nch].pixel[ncl].cluster_id == clusDef->clusID )
			      return det[nr].data_src[nch].hdr.bcps;
		    }
	       }
	  }
     }
     (void) fprintf( stderr, "*** WARNING *** [GET_DET_BCPS] no BCPS found\n" );
     return 0;
}

static inline
void UNPACK_LV0_PIXEL_VAL( const struct chan_src *pixel,
                           /*@out@*/ unsigned int *data )
{
     register unsigned short np = 0;

     register unsigned char *cpntr = pixel->data;

     if ( pixel->co_adding == (unsigned char) 1 ) {
          do {
               *data++ = (unsigned int) cpntr[1]
                    + ((unsigned int) cpntr[0] << 8);
               cpntr += 2;
          } while ( ++np < pixel->length );
     } else {
          do {
               *data++ = (unsigned int) cpntr[2]
                    + ((unsigned int) cpntr[1] << 8)
                    + ((unsigned int) cpntr[0] << 16);
               cpntr += 3;
          } while ( ++np < pixel->length );
     }
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned short GET_SCIA_LV0C_MDS( unsigned int nr_det, 
				  const struct mds0_info *info,
				  const struct mds0_det *det, 
				  struct mds1c_scia **mds_out )
{
     const char prognm[] = "GET_SCIA_LV0C_MDS";

     register unsigned short nch, ncl, np;
     register unsigned short numClus = 0;

     register unsigned int   nc, nd = 0;
     register unsigned int   offs, nrpix;

     register struct mds1c_scia *mds_pntr;

     unsigned int   ubuff[CHANNEL_SIZE];

     unsigned short num_mds = 0;

     struct mds1c_scia *mds_1c;

     struct clusdef_rec clusDef[MAX_CLUSTER];

     const unsigned short ri[MAX_NUM_STATE] = {
	  86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 
	  86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 
	  86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 
	  86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 
	  86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 
	  86, 86, 86, 86, 86, 86, 86, 86, 111, 86, 
	  303, 86, 86, 86, 86, 86, 86, 86, 111, 303
     };
/*
 * determine the required number of level 1c MDS
 */
     do {
	  if ( nd == 0 || info[nd-1].stateIndex != info[nd].stateIndex ) {
	       num_mds += GET_INFO_CLUSDEF( info[nd].stateIndex, 
					    nr_det, info, NULL );
	  }
     } while ( ++nd < nr_det );
     if ( num_mds == 0 ) return 0;
/*
 * allocate memory to store output records
 */
     *mds_out = NULL;
     mds_1c = (struct mds1c_scia *) 
	  malloc( num_mds * sizeof( struct mds1c_scia ) );
     if ( (mds_pntr = mds_1c) == NULL ) {
          NADC_ERROR( prognm, NADC_ERR_ALLOC, "mds1c_scia" );
	  return 0;
     }
/*
 * reorganise level 0 Detector MDS into level 1c structure
 */
     nd = 0;
     do {
	  if ( nd == 0 || info[nd-1].stateIndex != info[nd].stateIndex ) {
	       const unsigned char stateID = info[nd].stateID;
	       const double dsec_const  = det[nd].isp.secnd 
		    + det[nd].isp.musec / 1e6 + ri[stateID-1] / 256.;

	       numClus = GET_INFO_CLUSDEF( info[nd].stateIndex, 
					   nr_det, info, clusDef );

	       /* set pointer to mds-records of next state */
	       if ( nd > 0 ) mds_pntr += numClus;

	       /* initialise mds-records of this state */
	       nc = 0;
	       do {
		    double dsec = dsec_const 
			 + GET_DET_BCPS( clusDef+nc, nr_det-nd, det+nd ) / 16.;

		    mds_pntr[nc].mjd.days  = det[nd].isp.days;
		    mds_pntr[nc].mjd.secnd = (unsigned int) dsec;
		    mds_pntr[nc].mjd.musec = 
			 (unsigned int)(1e6 * (dsec - mds_pntr[nc].mjd.secnd));
		    mds_pntr[nc].rad_units_flag = CHAR_ZERO;
		    mds_pntr[nc].quality_flag = CHAR_ZERO;
		    mds_pntr[nc].type_mds = GET_SCIA_MDS_TYPE( stateID );
		    mds_pntr[nc].coaddf = UCHAR_ZERO;
		    mds_pntr[nc].category = det[nd].data_hdr.category;
		    mds_pntr[nc].state_id = stateID;
		    mds_pntr[nc].chan_id = clusDef[nc].chanID;
		    mds_pntr[nc].clus_id = clusDef[nc].clusID;
		    mds_pntr[nc].dur_scan = 0;              /* FIX THIS */
		    mds_pntr[nc].num_obs = 0;
		    mds_pntr[nc].num_pixels = clusDef[nc].length;
		    mds_pntr[nc].dsr_length = 0u;
		    mds_pntr[nc].orbit_phase = -999.f;
		    mds_pntr[nc].pet = -1.f;

                    /* allocate memory for pixel ID and values */
		    mds_pntr[nc].pixel_ids = (unsigned short *)
			 malloc( clusDef[nc].length * sizeof( short ));
		    if ( mds_pntr[nc].pixel_ids == NULL ) {
			 NADC_ERROR(prognm, NADC_ERR_ALLOC, "pixel_ids");
			 return 0; /* Oeps, we can't release (mds_1c) memory */
		    }
                    /* store pixel IDs */
		    for ( np = 0; np < clusDef[nc].length; np++ )
			 mds_pntr[nc].pixel_ids[np] = clusDef[nc].start + np;

                    /* make sure to initialize these pointers to NULL */
		    mds_pntr[nc].pixel_wv     = NULL;
		    mds_pntr[nc].pixel_wv_err = NULL;
		    mds_pntr[nc].pixel_val    = NULL;
		    mds_pntr[nc].pixel_err    = NULL;
		    mds_pntr[nc].geoC = NULL;
		    mds_pntr[nc].geoL = NULL;
		    mds_pntr[nc].geoN = NULL;
	       } while( ++nc < numClus );
	  }
          /* store detector readouts in L1c MDS */
	  nch = 0;
	  do {
	       const unsigned char chanID = 
		    det[nd].data_src[nch].hdr.channel.field.id;
	       const unsigned short clusters =
                    det[nd].data_src[nch].hdr.channel.field.clusters;

	       for ( ncl = 0; ncl < clusters; ncl++ ) {
		    unsigned char clusID = 
			 det[nd].data_src[nch].pixel[ncl].cluster_id;

		    nc = 0;
		    do {
			 if ( clusDef[nc].chanID == chanID
			      && clusDef[nc].clusID == clusID ) break;
		    } while( ++nc < numClus );
		    if ( nc == numClus ) {
			 char   msg[80];

			 (void) snprintf( msg, 80, 
					  "impossible combination of chanID [%-hhu] and clusID [%-hhu] for cluster %-hu", 
					  chanID, clusID, ncl );
			 NADC_ERROR( prognm, NADC_ERR_FATAL, msg );
			 return 0;              /* Oeps, another memory leak */
		    }
		    if ( mds_pntr[nc].coaddf == UCHAR_ZERO ) {
			 mds_pntr[nc].coaddf =
			      det[nd].data_src[nch].pixel[ncl].co_adding;
		    }
		    if ( mds_pntr[nc].pet < 0.f ) {
			 unsigned short vir_chan_b, firstPixel;
			 float pet[2];			      

			 GET_SCIA_LV0_DET_PET( det[nd].data_src[nch].hdr, pet,
					       &vir_chan_b );

			 if ( (int) mds_pntr[nc].chan_id != 2 ) {
			      firstPixel = clusDef[nc].start % CHANNEL_SIZE;
			 } else {
			      firstPixel = 2 * CHANNEL_SIZE
				   - clusDef[nc].start - clusDef[nc].length;
			      vir_chan_b = CHANNEL_SIZE - vir_chan_b;
			 }
			 if ( vir_chan_b == 0 || firstPixel < vir_chan_b )
			      mds_pntr[nc].pet = pet[0];
			 else {
			      mds_pntr[nc].pet = pet[1];
			 }
		    }
		    offs = mds_pntr[nc].num_obs * mds_pntr[nc].num_pixels;
		    nrpix = ++(mds_pntr[nc].num_obs) * mds_pntr[nc].num_pixels;
                    /* allocate memory for pixel values */
		    mds_pntr[nc].pixel_val = (float *)
			 realloc( mds_pntr[nc].pixel_val, 
				  nrpix * sizeof(float) );
		    if ( mds_pntr[nc].pixel_val == NULL ) {
			 NADC_ERROR( prognm, NADC_ERR_ALLOC, "pixel_val" );
			 return 0;              /* Oeps, another memory leak */
		    }
                    /* store pixel values */
		    UNPACK_LV0_PIXEL_VAL( det[nd].data_src[nch].pixel+ncl,
					  ubuff );
		    for ( np = 0; np < mds_pntr[nc].num_pixels; np++ )
			 mds_pntr[nc].pixel_val[offs+np] = (float) ubuff[np];
	       }
	  } while( ++nch < det[nd].num_chan );
     } while ( ++nd < nr_det );
/*
 * set return values
 */
     *mds_out = mds_1c;
     return num_mds;
}
