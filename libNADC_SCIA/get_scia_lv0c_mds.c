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
  call as    nr_mds = GET_SCIA_LV0C_MDS( num_det, det, mds_1c );
     input:  
	     unsigned int num_det      : number of MDS_DET structures
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
unsigned short _GET_LV0_CLUSDEF( unsigned int num_det, 
                                 const struct mds0_det *det,
				 struct clusdef_rec *clusDef )
{
     register unsigned char ncl, nch;
     register unsigned char chan_id = 1;
     register unsigned int  nd;

     unsigned short nr_clus;
     const struct chan_src *chan_src;

     /* initialize return value */
     unsigned short numClus = 0;

     (void) memset( clusDef, 0, MAX_CLUSTER * sizeof(struct clusdef_rec) );

     do {
          register unsigned char clusIDmx = 0;

          for ( nd = 0; nd < num_det; nd++ ) {
	       for ( nch = 0; nch < det[nd].num_chan; nch++ ) {
		    if ( det[nd].data_src[nch].hdr.channel.field.id != chan_id )
			 continue;

		    nr_clus = det[nd].data_src[nch].hdr.channel.field.clusters;
		    chan_src = det[nd].data_src[nch].pixel;
		    
		    for ( ncl = 0 ; ncl < nr_clus; ncl++ ) {
			 unsigned short indx = 
			      numClus + chan_src[ncl].cluster_id;
			 
			 if ( chan_src[ncl].cluster_id > clusIDmx )
			      clusIDmx = chan_src[ncl].cluster_id;
			 clusDef[indx].chanID = chan_id;
			 clusDef[indx].clusID = chan_src[ncl].cluster_id;
			 clusDef[indx].start  = chan_src[ncl].start;
			 clusDef[indx].length = chan_src[ncl].length;
		    }
	       }
          }
          numClus += clusIDmx + 1u;
     } while ( ++chan_id <= SCIENCE_CHANNELS );
     return numClus;
}

static inline
unsigned short _GET_LV0_BCPS( unsigned int num_det, 
			      const struct mds0_det *det,
			      unsigned char chanID,
			      unsigned char clusID )
{
     register unsigned char ncl, nch;
     register unsigned int  nd;

     unsigned short nr_clus;
     const struct chan_src *chan_src;

     for ( nd = 0; nd < num_det; nd++ ) {
	  for ( nch = 0; nch < det[nd].num_chan; nch++ ) {
	       if ( det[nd].data_src[nch].hdr.channel.field.id != chanID )
		    continue;

	       nr_clus = det[nd].data_src[nch].hdr.channel.field.clusters;
	       chan_src = det[nd].data_src[nch].pixel;
		    
	       for ( ncl = 0 ; ncl < nr_clus; ncl++ ) {
		    if ( chan_src[ncl].cluster_id == clusID ) 
			 return det[nd].data_src[nch].hdr.bcps;
	       }
          }
     }
     return 0;
}

static inline
unsigned char _GET_LV0_COADDF( unsigned int num_det, 
			       const struct mds0_det *det,
			       unsigned char chanID,
			       unsigned char clusID )
{
     register unsigned char ncl, nch;
     register unsigned int  nd;

     unsigned short nr_clus;
     const struct chan_src *chan_src;

     for ( nd = 0; nd < num_det; nd++ ) {
	  for ( nch = 0; nch < det[nd].num_chan; nch++ ) {
	       if ( det[nd].data_src[nch].hdr.channel.field.id != chanID )
		    continue;

	       nr_clus = det[nd].data_src[nch].hdr.channel.field.clusters;
	       chan_src = det[nd].data_src[nch].pixel;
		    
	       for ( ncl = 0 ; ncl < nr_clus; ncl++ ) {
		    if ( chan_src[ncl].cluster_id == clusID ) 
			 return chan_src[ncl].co_adding;
	       }
          }
     }
     return 0;
}

static inline
float _GET_LV0_PET( unsigned int num_det, 
		    const struct mds0_det *det,
		    unsigned char chanID,
		    unsigned char clusID )
{
     register unsigned char ncl, nch;
     register unsigned int  nd;

     unsigned short nr_clus;
     const struct chan_src *chan_src;

     for ( nd = 0; nd < num_det; nd++ ) {
	  for ( nch = 0; nch < det[nd].num_chan; nch++ ) {
	       if ( det[nd].data_src[nch].hdr.channel.field.id != chanID )
		    continue;

	       nr_clus = det[nd].data_src[nch].hdr.channel.field.clusters;
	       chan_src = det[nd].data_src[nch].pixel;
		    
	       for ( ncl = 0 ; ncl < nr_clus; ncl++ ) {
		    if ( chan_src[ncl].cluster_id == clusID ) {
			 unsigned short vir_chan_b, firstPixel;
			 float pet[2];			      

			 GET_SCIA_LV0_DET_PET( det[nd].data_src[nch].hdr, 
					       pet, &vir_chan_b );

			 if ( chanID != 2 ) {
			      firstPixel = chan_src[ncl].start % CHANNEL_SIZE;
			 } else {
			      firstPixel = 2 * CHANNEL_SIZE
				   - chan_src[ncl].start - chan_src[ncl].length;
			      vir_chan_b = CHANNEL_SIZE - vir_chan_b;
			 }
			 if ( vir_chan_b == 0 || firstPixel < vir_chan_b )
			      return pet[0];
			 else 
			      return pet[1];
		    }
	       }
          }
     }
     return -1.f;
}

static inline
unsigned short _GET_LV0_NUM_OBS( unsigned int num_det, 
				 const struct mds0_det *det,
				 unsigned char chanID,
				 unsigned char clusID )
{
     register unsigned char ncl, nch;
     register unsigned int  nd;

     unsigned short nr_clus;
     const struct chan_src *chan_src;

     /* initialize return value */
     unsigned short num_obs = 0;

     for ( nd = 0; nd < num_det; nd++ ) {
	  for ( nch = 0; nch < det[nd].num_chan; nch++ ) {
	       if ( det[nd].data_src[nch].hdr.channel.field.id != chanID )
		    continue;

	       nr_clus = det[nd].data_src[nch].hdr.channel.field.clusters;
	       chan_src = det[nd].data_src[nch].pixel;
		    
	       for ( ncl = 0 ; ncl < nr_clus; ncl++ ) {
		    if ( chan_src[ncl].cluster_id == clusID ) num_obs++;
	       }
	  }
     }
     return num_obs;
}

static inline
void _GET_LV0_PIXELVAL( unsigned int num_det, 
			const struct mds0_det *det,
			unsigned char chanID,
			unsigned char clusID,
			float *pixel_val )
{
     register unsigned char ncl, nch;
     register unsigned int  nd;

     unsigned short nr_clus;
     const struct chan_src *chan_src;

     for ( nd = 0; nd < num_det; nd++ ) {
	  for ( nch = 0; nch < det[nd].num_chan; nch++ ) {
	       if ( det[nd].data_src[nch].hdr.channel.field.id != chanID )
		    continue;

	       nr_clus = det[nd].data_src[nch].hdr.channel.field.clusters;
	       chan_src = det[nd].data_src[nch].pixel;
		    
	       for ( ncl = 0 ; ncl < nr_clus; ncl++ ) {
		    if ( chan_src[ncl].cluster_id == clusID ) {
			 register unsigned short np = 0;
			 register unsigned int data;
			 register unsigned char *cpntr = chan_src[ncl].data;

			 if ( chan_src[ncl].co_adding == (unsigned char) 1 ) {
			      do {
				   data = (unsigned int) cpntr[1]
					+ ((unsigned int) cpntr[0] << 8);
				   cpntr += 2;

				   *pixel_val++ = (float) data;
			      } while ( ++np < chan_src[ncl].length );
			 } else {
			      do {
				   data = (unsigned int) cpntr[2]
					+ ((unsigned int) cpntr[1] << 8)
					+ ((unsigned int) cpntr[0] << 16);
				   cpntr += 3;

				   *pixel_val++ = (float) data;
			      } while ( ++np < chan_src[ncl].length );
			 }
		    }
	       }
	  }
     }
}


/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned short GET_SCIA_LV0C_MDS( const unsigned int num_det, 
				  const struct mds0_det *det, 
				  struct mds1c_scia **mds_out )
{
     register unsigned short np;

     register unsigned int   nc;

     struct mds1c_scia  *mds_1c;

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
     const double dsec_const  = det->isp.secnd + det->isp.musec / 1e6
	  + ri[det->data_hdr.state_id - 1] / 256.;
/*
 * determine the required number of level 1c MDS
 */
     const unsigned short num_mds = _GET_LV0_CLUSDEF( num_det, det, clusDef );
/*
 * allocate memory to store output records
 */
     *mds_out = NULL;
     if ( num_mds == 0 ) return 0;

     mds_1c = (struct mds1c_scia *) 
	  malloc( (size_t) num_mds * sizeof( struct mds1c_scia ) );
     if ( mds_1c == NULL ) {
          NADC_ERROR( NADC_ERR_ALLOC, "mds1c_scia" );
	  return 0;
     }
/*
 * reorganise level 0 Detector MDS into level 1c structure
 */
     nc = 0;
     do {
	  double dsec = dsec_const 
	       + _GET_LV0_BCPS( num_det, det, clusDef[nc].chanID,
				clusDef[nc].clusID ) / 16.;

	  mds_1c[nc].mjd.days  = det->isp.days;
	  mds_1c[nc].mjd.secnd = (unsigned int) dsec;
	  mds_1c[nc].mjd.musec = 
	       (unsigned int)(1e6 * (dsec - mds_1c[nc].mjd.secnd));
	  mds_1c[nc].rad_units_flag = CHAR_ZERO;
	  mds_1c[nc].quality_flag = CHAR_ZERO;
	  mds_1c[nc].type_mds = GET_SCIA_MDS_TYPE( det->data_hdr.state_id );
	  mds_1c[nc].coaddf = _GET_LV0_COADDF( num_det, det, 
					       clusDef[nc].chanID,
					       clusDef[nc].clusID );
	  mds_1c[nc].category = det->data_hdr.category;
	  mds_1c[nc].state_id = det->data_hdr.state_id;
	  mds_1c[nc].chan_id = clusDef[nc].chanID;
	  mds_1c[nc].clus_id = clusDef[nc].clusID;
	  mds_1c[nc].dur_scan = 0;              /* FIX THIS */
	  mds_1c[nc].num_obs = _GET_LV0_NUM_OBS( num_det, det, 
						 clusDef[nc].chanID,
						 clusDef[nc].clusID );
	  mds_1c[nc].num_pixels = clusDef[nc].length;
	  mds_1c[nc].dsr_length = 0u;
	  mds_1c[nc].orbit_phase = -999.f;
	  mds_1c[nc].pet = _GET_LV0_PET( num_det, det, 
					 clusDef[nc].chanID,
					 clusDef[nc].clusID );

	  /* allocate memory for pixel ID and values */
	  mds_1c[nc].pixel_ids = (unsigned short *)
	       malloc( mds_1c[nc].num_pixels * sizeof(short) );
	  mds_1c[nc].pixel_val = (float *)
	       malloc( mds_1c[nc].num_obs * mds_1c[nc].num_pixels 
		       * sizeof(float) );
	  if ( mds_1c[nc].pixel_ids == NULL 
	       || mds_1c[nc].pixel_val == NULL ) {
	       NADC_ERROR( NADC_ERR_ALLOC, "pixel_ids/pixel_val" );
	       return 0; /* Oeps, we can't release (mds_1c) memory */
	  }
	  /* store pixel IDs */
	  for ( np = 0; np < clusDef[nc].length; np++ )
	       mds_1c[nc].pixel_ids[np] = clusDef[nc].start + np;
	  
	  /* store pixel values */
	  _GET_LV0_PIXELVAL( num_det, det, 
			     clusDef[nc].chanID, clusDef[nc].clusID,
			     mds_1c[nc].pixel_val );

	  /* make sure to initialize these pointers to NULL */
	  mds_1c[nc].pixel_wv     = NULL;
	  mds_1c[nc].pixel_wv_err = NULL;
	  mds_1c[nc].pixel_err    = NULL;
	  mds_1c[nc].geoC = NULL;
	  mds_1c[nc].geoL = NULL;
	  mds_1c[nc].geoN = NULL;
     } while( ++nc < num_mds );
/*
 * set return values
 */
     *mds_out = mds_1c;
     return num_mds;
}
