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

.IDENTifer   SCIA_LV1_WR_MDS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1b/1c data
.LANGUAGE    ANSI C
.PURPOSE     write SCIAMACHY level 1b/1c Measurement Data Sets 
.RETURNS     number of MDS records written
.COMMENTS    None
.ENVIRONment None
.VERSION      4.2   24-Nov-2008 fixed resently introduced "brown paper bag" 
                               bug in SCIA_LV1C_WR_MDS, RvH
              4.1   13-Jun-2006 fixed "brown paper bag" bug in 
                               SCIA_LV1_WR_ONE_MDS, RvH
              4.0   07-Dec-2005 removed esig/esigc from mds-struct
                               renamed pixel_val_err to pixel_err, RvH
              3.1   17-Oct-2005 write only one MDS_PMD or MDS_POLV
                               removed num_mds parameter, RvH
              3.0   11-Oct-2005 use direct write, add usage of SCIA_LV1_ADD_DSD
                               fixed several serieus bugs in L1c write-routines
              2.0   27-Apr-2005 added level 1c routines, RvH
              1.0   14-Jan-2004 created by R. M. van Hees
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

/*+++++ Local (static) variables +++++*/
static int source;

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

/*+++++ Macros +++++*/

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#ifdef _SWAP_TO_LITTLE_ENDIAN
#include "swap_lv1_mds.inc"
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_GeoN
.PURPOSE     General function to write level 1 Nadir geolocation records
.INPUT/OUTPUT
  call as    nr_byte = SCIA_LV1_WR_GeoN( num_geo, geoN, fd );
     input:
            unsigned short num_geo :
	    struct geoN_scia *geoN :
	    FILE *fd               :

.RETURNS     number of bytes written (unsigned int)
.COMMENTS    static function
-------------------------*/
static
unsigned int SCIA_LV1_WR_GeoN( unsigned short num_geo, 
			       const struct geoN_scia *geoN, FILE *fd )
     /*@globals  errno@*/
     /*@modifies errno, fd@*/
{
     unsigned int nr_byte = 0u;

     const size_t sz_coord_envi = sizeof(struct coord_envi);

     register unsigned short ng = 0;
     do {
	  if ( fwrite( &geoN->pos_esm, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += ENVI_FLOAT;
	  if ( fwrite( geoN->sun_zen_ang, ENVI_FLOAT, 3, fd ) != 3 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += 3 * ENVI_FLOAT;
	  if ( fwrite( geoN->sun_azi_ang, ENVI_FLOAT, 3, fd ) != 3 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += 3 * ENVI_FLOAT;
	  if ( fwrite( geoN->los_zen_ang, ENVI_FLOAT, 3, fd ) != 3 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += 3 * ENVI_FLOAT;
	  if ( fwrite( geoN->los_azi_ang, ENVI_FLOAT, 3, fd ) != 3 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += 3 * ENVI_FLOAT;
	  if ( fwrite( &geoN->sat_h, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += ENVI_FLOAT;
	  if ( fwrite( &geoN->earth_rad, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += ENVI_FLOAT;
	  if ( fwrite( &geoN->sub_sat_point, sz_coord_envi, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += sz_coord_envi;
	  if ( fwrite(geoN->corner, sz_coord_envi * NUM_CORNERS, 1, fd) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += NUM_CORNERS * sz_coord_envi;
	  if ( fwrite( &geoN->center, sz_coord_envi, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += sz_coord_envi;
     } while ( geoN++, ++ng < num_geo );
 done:
     return nr_byte;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_GeoL
.PURPOSE     General function to write level 1 Limb geolocation records
.INPUT/OUTPUT
  call as   nr_byte = SCIA_LV1_WR_GeoL( num_geo, geoL, fd );
     input:
            unsigned short num_geo :
	    struct geoL_scia *geoL :
	    FILE *fd               :

.RETURNS     number of bytes written (unsigned int)
.COMMENTS    static function
-------------------------*/
static
unsigned int SCIA_LV1_WR_GeoL( unsigned short num_geo, 
			       const struct geoL_scia *geoL, FILE *fd )
     /*@globals  errno@*/
     /*@modifies errno, fd@*/
{
     unsigned int nr_byte = 0u;

     const size_t sz_coord_envi = sizeof(struct coord_envi);

     register unsigned short ng = 0;
     do {
	  if ( fwrite( &geoL->pos_esm, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += ENVI_FLOAT;
	  if ( fwrite( &geoL->pos_asm, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += ENVI_FLOAT;
	  if ( fwrite( geoL->sun_zen_ang, ENVI_FLOAT, 3, fd ) != 3 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += 3 * ENVI_FLOAT;
	  if ( fwrite( geoL->sun_azi_ang, ENVI_FLOAT, 3, fd ) != 3 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += 3 * ENVI_FLOAT;
	  if ( fwrite( geoL->los_zen_ang, ENVI_FLOAT, 3, fd ) != 3 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += 3 * ENVI_FLOAT;
	  if ( fwrite( geoL->los_azi_ang, ENVI_FLOAT, 3, fd ) != 3 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += 3 * ENVI_FLOAT;
	  if ( fwrite( &geoL->sat_h, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += ENVI_FLOAT;
	  if ( fwrite( &geoL->earth_rad, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += ENVI_FLOAT;
	  if ( fwrite( &geoL->sub_sat_point, sz_coord_envi, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += sz_coord_envi;
	  if ( fwrite( geoL->tang_ground_point, sz_coord_envi, 3, fd ) != 3 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += 3 * sz_coord_envi;
	  if ( fwrite( geoL->tan_h, ENVI_FLOAT, 3, fd ) != 3 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += 3 * ENVI_FLOAT;
	  if ( fwrite( &geoL->dopp_shift, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += ENVI_FLOAT;
     } while ( geoL++, ++ng < num_geo );
 done:
     return nr_byte;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_GeoC
.PURPOSE     General function to write level 1 Monitoring geolocation records
.INPUT/OUTPUT
  call as   nr_byte = SCIA_LV1_WR_GeoC( num_geo, geoC, fd );
     input:
            unsigned short num_geo :
	    struct geoC_scia *geoC :
	    FILE *fd               :

.RETURNS     number of bytes written (unsigned int)
.COMMENTS    static function
-------------------------*/
static
unsigned int SCIA_LV1_WR_GeoC( unsigned short num_geo, 
			       const struct geoC_scia *geoC, FILE *fd )
     /*@globals  errno@*/
     /*@modifies errno, fd@*/
{
     unsigned int nr_byte = 0u;

     const size_t sz_coord_envi = sizeof(struct coord_envi);

     register unsigned short ng = 0;
     do {
	  if ( fwrite( &geoC->pos_esm, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += ENVI_FLOAT;
	  if ( fwrite( &geoC->pos_asm, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += ENVI_FLOAT;
	  if ( fwrite( &geoC->sun_zen_ang, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += ENVI_FLOAT;
	  if ( fwrite( &geoC->sub_sat_point, sz_coord_envi, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += sz_coord_envi;
     } while ( geoC++, ++ng < num_geo );
 done:
     return nr_byte;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_LV0Hdr
.PURPOSE     General function to write the level 0 headers of a level 1B record
.INPUT/OUTPUT
  call as   nr_byte = SCIA_LV1_WR_LV0Hdr( num_lv0hdr, lv0_hdr *lv0, fd );
     input:
            unsigned short num_lv0hdr :
	    struct lv0_hdr *lv0       :
	    FILE *fd                  :

.RETURNS     number of bytes written
.COMMENTS    static function
-------------------------*/
static
unsigned int SCIA_LV1_WR_LV0Hdr( unsigned short num_lv0hdr, 
				 const struct lv0_hdr *lv0, FILE *fd )
     /*@globals  errno@*/
     /*@modifies errno, fd@*/
{
     unsigned int nr_byte = 0u;

     unsigned short nl = 0;
     do {
	  if ( fwrite(&lv0->packet_hdr.api.two_byte, ENVI_USHRT, 1, fd) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += ENVI_USHRT;
	  if ( fwrite( &lv0->packet_hdr.seq_cntrl, ENVI_USHRT, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += ENVI_USHRT;
	  if ( fwrite( &lv0->packet_hdr.length, ENVI_USHRT, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += ENVI_USHRT;
	  if ( fwrite( &lv0->data_hdr.length, ENVI_USHRT, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += ENVI_USHRT;
	  if ( fwrite( &lv0->data_hdr.category, ENVI_UCHAR, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += ENVI_UCHAR;
	  if ( fwrite( &lv0->data_hdr.state_id, ENVI_UCHAR, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += ENVI_UCHAR;
	  if ( fwrite( &lv0->data_hdr.on_board_time, ENVI_UINT, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += ENVI_UINT;
	  if ( fwrite( &lv0->data_hdr.rdv.two_byte, ENVI_USHRT, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += ENVI_USHRT;
	  if ( fwrite( &lv0->data_hdr.id.two_byte, ENVI_USHRT, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += ENVI_USHRT;
	  if ( fwrite( &lv0->bcps, ENVI_USHRT, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += ENVI_USHRT;
	  if ( fwrite(&lv0->pmtc_hdr.pmtc_1.two_byte, ENVI_USHRT, 1, fd) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += ENVI_USHRT;
	  if ( fwrite( &lv0->pmtc_hdr.scanner_mode, ENVI_USHRT, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += ENVI_USHRT;
	  if ( fwrite(&lv0->pmtc_hdr.az_param.four_byte, ENVI_UINT, 1, fd) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += ENVI_UINT;
	  if ( fwrite(&lv0->pmtc_hdr.elv_param.four_byte, ENVI_UINT, 1, fd) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += ENVI_UINT;
	  if ( fwrite( lv0->pmtc_hdr.factor, 6 * ENVI_UCHAR, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += 6 * ENVI_UCHAR;
	  if ( fwrite( lv0->orbit_vector, 8 * ENVI_INT, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += 8 * ENVI_INT;
	  if ( fwrite( &lv0->num_chan, ENVI_USHRT, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += ENVI_USHRT;
     } while( lv0++, ++nl < num_lv0hdr );
 done:
     return nr_byte;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_PolV
.PURPOSE     General function to write level 1 fractional polarisation records
.INPUT/OUTPUT
  call as   nr_byte = SCIA_LV1_WR_PolV( num_polV, polV, fd );
     input:
            unsigned short num_polV :
	    struct polV_scia *polV  :
	    FILE *fd                :

.RETURNS     number of bytes written (unsigned int)
.COMMENTS    static function
-------------------------*/
static
unsigned int SCIA_LV1_WR_PolV( unsigned short num_polV, 
			       const struct polV_scia *polV, FILE *fd )
     /*@globals  errno@*/
     /*@modifies errno, fd@*/
{
     unsigned int nr_byte = 0u;

     register unsigned short np = 0;
     do {
	  if ( fwrite( polV->Q, ENVI_FLOAT * NUM_FRAC_POLV, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += NUM_FRAC_POLV * ENVI_FLOAT;
	  if ( fwrite( polV->error_Q, ENVI_FLOAT * NUM_FRAC_POLV, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += NUM_FRAC_POLV * ENVI_FLOAT;
	  if ( fwrite( polV->U, ENVI_FLOAT * NUM_FRAC_POLV, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += NUM_FRAC_POLV * ENVI_FLOAT;
	  if ( fwrite( polV->error_U, ENVI_FLOAT * NUM_FRAC_POLV, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += NUM_FRAC_POLV * ENVI_FLOAT;
	  if ( fwrite(polV->rep_wv, ENVI_FLOAT * (NUM_FRAC_POLV+1), 1, fd) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += (NUM_FRAC_POLV+1) * ENVI_FLOAT;
	  if ( fwrite( &polV->gdf.p_bar, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += ENVI_FLOAT;
	  if ( fwrite( &polV->gdf.beta, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += ENVI_FLOAT;
	  if ( fwrite( &polV->gdf.w0, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += ENVI_FLOAT;
     } while ( polV++, ++np < num_polV );
 done:
     return nr_byte;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_ONE_MDS
.PURPOSE     General function to write one level 1b Measurement Data Set
.INPUT/OUTPUT
  call as   nr_byte = SCIA_LV1_WR_ONE_MDS( fd, num_mds, mds ); 
     input:  
            FILE    *fd               : (open) stream pointer
            struct mds1_scia   *mds   : structure for level 1b MDS

.RETURNS     number of bytes written (unsigned int)
.COMMENTS    static function
-------------------------*/
static
unsigned int SCIA_LV1_WR_ONE_MDS( FILE *fd, const struct mds1_scia *mds )
       /*@globals  errno, nadc_stat, nadc_err_stack, source;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd@*/
{
     register unsigned short na, nc;
     register unsigned int   nr;

     size_t nmemb;

     unsigned int nr_byte = 0u;

     unsigned char *glint_flags;

#ifdef _SWAP_TO_LITTLE_ENDIAN
     register unsigned short ni;

     unsigned int   ubuff;
     size_t         byte_size;
     float          *rbuff;

     struct mjd_envi   mjd;
     struct geoN_scia *geoN;
     struct geoL_scia *geoL;
     struct geoC_scia *geoC;
     struct lv0_hdr   *lv0;
     struct polV_scia *polV;
#endif
/*
 * write all MDS parameters of this state
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
     mjd.days = byte_swap_32( mds->mjd.days );
     mjd.secnd = byte_swap_u32( mds->mjd.secnd );
     mjd.musec = byte_swap_u32( mds->mjd.musec );	  
     if ( fwrite( &mjd, sizeof( struct mjd_envi ), 1, fd ) != 1 )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
#else
     if ( fwrite( &mds->mjd, sizeof( struct mjd_envi ), 1, fd ) != 1 )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
#endif
     nr_byte += sizeof( struct mjd_envi );

#ifdef _SWAP_TO_LITTLE_ENDIAN
     ubuff = byte_swap_u32( mds->dsr_length );
     if ( fwrite( &ubuff, ENVI_UINT, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
#else
     if ( fwrite( &mds->dsr_length, ENVI_UINT, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
#endif
     nr_byte += ENVI_UINT;

     if ( fwrite( &mds->quality_flag, ENVI_CHAR, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
     nr_byte += ENVI_CHAR;
/*
 * write scale factor [SCIENCE_CHANNELS]
 */
     if ( fwrite( mds->scale_factor, SCIENCE_CHANNELS, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
     nr_byte += SCIENCE_CHANNELS;
/*
 * write satellite flags
 */
     if ( fwrite( mds->sat_flags, (size_t) mds->n_aux, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
     nr_byte += mds->n_aux;
/*
 * write red grass flags
 */
     nmemb = (size_t) mds->n_clus * mds->n_aux;
     if ( fwrite( mds->red_grass, nmemb, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
     nr_byte += nmemb;
/*
 * write Sun glint flags
 */
     if ( source != SCIA_MONITOR ) {
/*
 * allocate memory for the Sun glint/Rainbow flags
 */
	  glint_flags = (unsigned char *) malloc( (size_t) mds->n_aux );
	  if ( glint_flags == NULL ) 
	       NADC_GOTO_ERROR( NADC_ERR_ALLOC, "glint_flags" );

	  if ( source == SCIA_NADIR ) {
	       na = 0;
	       do {
		    glint_flags[na] = mds->geoN[na].glint_flag;
	       } while ( ++na < mds->n_aux );
	  } else {
	       na = 0;
	       do {
		    glint_flags[na] = mds->geoL[na].glint_flag;
	       } while ( ++na < mds->n_aux );
	  }
	  if ( fwrite( glint_flags, (size_t) mds->n_aux, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  nr_byte += mds->n_aux;

	  free( glint_flags );
     }
/*
 * write geolocation records
 */
     switch ( source ) {
     case SCIA_NADIR:
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  byte_size = (size_t) mds->n_aux * sizeof( struct geoN_scia );
	  if ( (geoN = (struct geoN_scia *) malloc( byte_size )) == NULL ) 
	       NADC_GOTO_ERROR( NADC_ERR_ALLOC, "geoN" );
	  (void) memcpy( geoN, mds->geoN, byte_size );
	  for ( ni = 0; ni < mds->n_aux; ni++ ) Sun2Intel_GeoN( &geoN[ni] );
	  nr_byte += SCIA_LV1_WR_GeoN( mds->n_aux, geoN, fd );
	  free( geoN );
#else
	  nr_byte += SCIA_LV1_WR_GeoN( mds->n_aux, mds->geoN, fd );
#endif
	  break;
     case SCIA_LIMB:
     case SCIA_OCCULT:
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  byte_size = (size_t) mds->n_aux * sizeof( struct geoL_scia );
	  if ( (geoL = (struct geoL_scia *) malloc( byte_size )) == NULL ) 
	       NADC_GOTO_ERROR( NADC_ERR_ALLOC, "geoL" );
	  (void) memcpy( geoL, mds->geoL, byte_size );
	  for ( ni = 0; ni < mds->n_aux; ni++ )
	       Sun2Intel_GeoL( &geoL[ni] );
	  nr_byte += SCIA_LV1_WR_GeoL( mds->n_aux, geoL, fd );
	  free( geoL );
#else
	  nr_byte += SCIA_LV1_WR_GeoL( mds->n_aux, mds->geoL, fd );
#endif
	  break;
     case SCIA_MONITOR:
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  byte_size = (size_t) mds->n_aux * sizeof( struct geoC_scia );
	  if ( (geoC = (struct geoC_scia *) malloc( byte_size )) == NULL ) 
	       NADC_GOTO_ERROR( NADC_ERR_ALLOC, "geoC" );
	  (void) memcpy( geoC, mds->geoC, byte_size );
	  for ( ni = 0; ni < mds->n_aux; ni++ ) Sun2Intel_GeoC( &geoC[ni] );
	  nr_byte += SCIA_LV1_WR_GeoC( mds->n_aux, geoC, fd );
	  free( geoC );
#else
	  nr_byte += SCIA_LV1_WR_GeoC( mds->n_aux, mds->geoC, fd );
#endif
	  break;
     }
/*
 * write level 0 header
 */
     if ( mds->n_aux > 0 ) {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  byte_size = (size_t) mds->n_aux * sizeof( struct lv0_hdr );
	  if ( (lv0 = (struct lv0_hdr *) malloc( byte_size )) == NULL ) 
	       NADC_GOTO_ERROR( NADC_ERR_ALLOC, "lv0" );
	  (void) memcpy( lv0, mds->lv0, byte_size );
	  for ( ni = 0; ni < mds->n_aux; ni++ ) Sun2Intel_LV0Hdr( &lv0[ni] );
	  nr_byte += SCIA_LV1_WR_LV0Hdr( mds->n_aux, lv0, fd );
	  free( lv0 );
#else
	  nr_byte += SCIA_LV1_WR_LV0Hdr( mds->n_aux, mds->lv0, fd );
#endif
     }
/*
 * PMD values
 */
     if ( source != SCIA_MONITOR ) {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  byte_size = mds->n_pmd * ENVI_FLOAT;
	  if ( (rbuff = (float *) malloc( byte_size )) == NULL ) 
	       NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rbuff" );
	  (void) memcpy( rbuff, mds->int_pmd, byte_size );
	  for ( ni = 0; ni < mds->n_pmd; ni++ ) IEEE_Swap__FLT( &rbuff[ni] );
	  if ( fwrite( rbuff, (size_t) mds->n_pmd * ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
	  free( rbuff );
#else
	  if ( fwrite( mds->int_pmd, ENVI_FLOAT * mds->n_pmd, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
#endif
	  nr_byte += mds->n_pmd * ENVI_FLOAT;
     }
/*
 * write fractional polarisation values
 */
     if ( source != SCIA_MONITOR ) {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  byte_size = mds->n_pol * sizeof( struct polV_scia );
	  if ( (polV = (struct polV_scia *) malloc( byte_size )) == NULL ) 
	       NADC_GOTO_ERROR( NADC_ERR_ALLOC, "polV" );
	  (void) memcpy( polV, mds->polV, byte_size );
	  for ( ni = 0; ni < mds->n_pol; ni++ ) 
	       Sun2Intel_polV( &polV[ni] );
	  nr_byte += SCIA_LV1_WR_PolV( mds->n_pol, polV, fd );
	  free( polV );
#else
	  nr_byte += SCIA_LV1_WR_PolV( mds->n_pol, mds->polV, fd );
#endif
     }
/*
 * write cluster data
 */
     nc = 0;
     do {
	  if ( mds->clus[nc].n_sig > 0u ) {
	       nr = 0;
	       do {
		    if ( fwrite( &mds->clus[nc].sig[nr].corr, ENVI_CHAR, 1, fd ) != 1 )
			 NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
		    nr_byte += ENVI_CHAR;
#ifdef _SWAP_TO_LITTLE_ENDIAN
		    mds->clus[nc].sig[nr].sign = 
			 byte_swap_u16( mds->clus[nc].sig[nr].sign );
#endif
		    if ( fwrite( &mds->clus[nc].sig[nr].sign, ENVI_USHRT, 1, fd ) != 1 )
			 NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
		    nr_byte += ENVI_USHRT;		    
		    if ( fwrite( &mds->clus[nc].sig[nr].stray, ENVI_CHAR, 1, fd ) != 1 )
			 NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
		    nr_byte += ENVI_CHAR;
	       } while ( ++nr < mds->clus[nc].n_sig );
	  } else if ( mds->clus[nc].n_sigc > 0u ) {
	       nr = 0;
	       do {
#ifdef _SWAP_TO_LITTLE_ENDIAN
		    mds->clus[nc].sigc[nr].det.four_byte = 
			 byte_swap_u32( 
			      mds->clus[nc].sigc[nr].det.four_byte );
#endif
		    if ( fwrite( &mds->clus[nc].sigc[nr].det.four_byte, ENVI_UINT, 1, fd ) != 1 )
			 NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
		    nr_byte += ENVI_UINT;
		    if ( fwrite( &mds->clus[nc].sigc[nr].stray, ENVI_CHAR, 1, fd ) != 1 )
			 NADC_GOTO_ERROR( NADC_ERR_PDS_WR, "" );
		    nr_byte += ENVI_CHAR;
	       } while ( ++nr < mds->clus[nc].n_sigc );
	  }
     } while ( ++nc < mds->n_clus );
/*
 * check written bytes against DSR length
 */
     if ( nr_byte != mds->dsr_length )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_SIZE, "MDS(1b)"  );
 done:
     return nr_byte;
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_MDS
.PURPOSE     read MDS of one state from a SCIAMACHY level 1b product
.INPUT/OUTPUT
  call as   nr_mds = SCIA_LV1_WR_MDS( fd, num_mds, mds );
     input:  
            FILE   *fd                : (open) stream pointer
	    unsigned int num_mds      : number of MDS records
            struct mds1_scia   *mds   : structure with level 1b MDS records

.RETURNS     number of written level 1b MDS records  (unsigned int), 
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1_WR_MDS( FILE *fd, unsigned int num_mds,
		      const struct mds1_scia *mds )
     /*@globals  errno, nadc_stat, nadc_err_stack, source;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fd, source@*/
{
     struct dsd_envi dsd = {
          "", "M",
          "                                                              ",
          0u, 0u, 0u, -1
     };
/*
 * set variable source (= type of MDS)
 */
     source = (int) mds->type_mds;
     switch ( source ) {
     case SCIA_NADIR:
	  (void) strcpy( dsd.name, "NADIR" );
	  break;
     case SCIA_LIMB:
	  (void) strcpy( dsd.name, "LIMB" );
	  break;
     case SCIA_OCCULT:
	  (void) strcpy( dsd.name, "OCCULTATION" );
	  break;
     case SCIA_MONITOR:
	  (void) strcpy( dsd.name, "MONITORING" );
	  break;
     }
/*
 * check number of MDS records
 */
     if ( num_mds == 0  ) {
	  SCIA_LV1_ADD_DSD( &dsd );
	  return;
     }
/*
 * read MDS data of the selected state
 */
     do {
	  dsd.size += SCIA_LV1_WR_ONE_MDS( fd, mds );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "ONE_MDS" );
	  mds++;
     } while ( ++dsd.num_dsr < num_mds );
/*
 * update list of written DSD records
 */
     SCIA_LV1_ADD_DSD( &dsd );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1C_WR_MDS
.PURPOSE     write SCIAMACHY level 1c MDS
.INPUT/OUTPUT
  call as    SCIA_LV1C_WR_MDS( fd, num_mds, mds );
     input:  
            FILE    *fd               : (open) stream pointer
            unsigned int num_mds      : number of MDS records
            struct mds1c_scia  *mds   : structure for level 1c MDS

.RETURNS     nothing,
             error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1C_WR_MDS( FILE *fd, unsigned int num_mds,
		       const struct mds1c_scia *mds )
     /*@globals  errno, nadc_stat, nadc_err_stack, source;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fd, source@*/
{
     register unsigned short ni;
     register unsigned int   bytes_written;

     size_t         nr_memb;
     size_t         nr_byte;

     unsigned short usbuff;
     float          rbuff;

     struct dsd_envi dsd = {
          "", "M",
          "                                                              ",
          0u, 0u, 0u, -1
     };

     unsigned short *utemp;

#ifdef _SWAP_TO_LITTLE_ENDIAN
     register size_t  nm;

     unsigned int   ubuff;
     float          *rtemp;

     struct mjd_envi   mjd;
     struct geoN_scia *geoN;
     struct geoL_scia *geoL;
     struct geoC_scia *geoC;
#endif
/*
 * set variable source (= type of MDS)
 */
     source = (int) mds->type_mds;
     switch ( source ) {
     case SCIA_NADIR:
	  (void) strcpy( dsd.name, "NADIR" );
	  break;
     case SCIA_LIMB:
	  (void) strcpy( dsd.name, "LIMB" );
	  break;
     case SCIA_OCCULT:
	  (void) strcpy( dsd.name, "OCCULTATION" );
	  break;
     case SCIA_MONITOR:
	  (void) strcpy( dsd.name, "MONITORING" );
	  break;
     }
/*
 * check number of MDS records
 */
     if ( num_mds == 0  ) {
	  SCIA_LV1_ADD_DSD( &dsd );
	  return;
     }
/*
 * write data set records
 */
     do {
	  bytes_written = 0u;
/* 1 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  mjd.days = byte_swap_32( mds->mjd.days );
	  mjd.secnd = byte_swap_u32( mds->mjd.secnd );
	  mjd.musec = byte_swap_u32( mds->mjd.musec );	  
	  if ( fwrite( &mjd, sizeof( struct mjd_envi ), 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
#else
	  if ( fwrite( &mds->mjd, sizeof( struct mjd_envi ), 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
#endif
	  bytes_written += sizeof( struct mjd_envi );
/* 2 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  ubuff = byte_swap_u32( mds->dsr_length );
	  if ( fwrite( &ubuff, ENVI_UINT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
#else
	  if ( fwrite( &mds->dsr_length, ENVI_UINT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
#endif
	  bytes_written += ENVI_UINT;
/* 3 */
	  if ( fwrite( &mds->quality_flag, ENVI_CHAR, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  bytes_written += ENVI_CHAR;
/* 4 */
	  rbuff = mds->orbit_phase; 
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  IEEE_Swap__FLT( &rbuff );
#endif
	  if ( fwrite( &rbuff, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  bytes_written += ENVI_FLOAT;
/* 5 */
	  usbuff = (unsigned short) mds->category;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  usbuff = byte_swap_u16( usbuff );
#endif
	  if ( fwrite( &usbuff, ENVI_USHRT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  bytes_written += ENVI_USHRT;
/* 6 */
	  usbuff = (unsigned short) mds->state_id;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  usbuff = byte_swap_u16( usbuff );
#endif
	  if ( fwrite( &usbuff, ENVI_USHRT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  bytes_written += ENVI_USHRT;
/* 7 */
	  usbuff = (unsigned short) mds->clus_id;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  usbuff = byte_swap_u16( usbuff );
#endif
	  if ( fwrite( &usbuff, ENVI_USHRT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  bytes_written += ENVI_USHRT;
/* 8 */
	  usbuff = (unsigned short) mds->num_obs;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  usbuff = byte_swap_u16( usbuff );
#endif
	  if ( fwrite( &usbuff, ENVI_USHRT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  bytes_written += ENVI_USHRT;
/* 9 */
	  usbuff = (unsigned short) mds->num_pixels;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  usbuff = byte_swap_u16( usbuff );
#endif
	  if ( fwrite( &usbuff, ENVI_USHRT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  bytes_written += ENVI_USHRT;
/* 10 */
	  if ( fwrite( &mds->rad_units_flag, ENVI_CHAR, 1 , fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );

	  bytes_written += ENVI_CHAR;
/* 11 */
	  nr_byte = (size_t) mds->num_pixels * ENVI_USHRT;
	  if ( (utemp = (unsigned short *) malloc( nr_byte )) == NULL ) 
	       NADC_RETURN_ERROR( NADC_ERR_ALLOC, "utemp" );
	  for ( ni = 0; ni < mds->num_pixels; ni++ ) {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       utemp[ni] = byte_swap_u16( mds->pixel_ids[ni] % CHANNEL_SIZE );
#else
	       utemp[ni] = mds->pixel_ids[ni] % CHANNEL_SIZE;
#endif
	  }
	  if ( fwrite( utemp, nr_byte, 1, fd ) != 1 ) {
	       free( utemp );
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  }
	  bytes_written += nr_byte;
	  free( utemp );
/* 12 & 13 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  nr_byte = (size_t) mds->num_pixels * ENVI_FLOAT;
	  if ( (rtemp = (float *) malloc( nr_byte )) == NULL ) 
	       NADC_RETURN_ERROR( NADC_ERR_ALLOC, "rtemp" );
	  for ( ni = 0; ni < mds->num_pixels; ni++ ) {
	       rtemp[ni] = mds->pixel_wv[ni];
	       IEEE_Swap__FLT( &rtemp[ni] );
	  }
	  if ( fwrite( rtemp, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  for ( ni = 0; ni < mds->num_pixels; ni++ ) {
	       rtemp[ni] = mds->pixel_wv_err[ni];
	       IEEE_Swap__FLT( &rtemp[ni] );
	  }
	  if ( fwrite( rtemp, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  free( rtemp );
#else
	  if ( fwrite( mds->pixel_wv, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" ) 
	  if ( fwrite( mds->pixel_wv_err, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" ) 
#endif
	  bytes_written += 2 * mds->num_pixels * ENVI_FLOAT;
/* 14 & 15 */
	  nr_memb = (size_t) mds->num_obs * (size_t) mds->num_pixels;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  nr_byte = nr_memb * ENVI_FLOAT;
	  if ( (rtemp = (float *) malloc( nr_byte )) == NULL ) 
	       NADC_RETURN_ERROR( NADC_ERR_ALLOC, "rtemp" );
	  nm = 0;
	  do {
	       rtemp[nm] = mds->pixel_val[nm];
	       IEEE_Swap__FLT( &rtemp[nm] );
	  } while ( ++nm < nr_memb );
	  if ( fwrite( rtemp, ENVI_FLOAT, nr_memb, fd ) != nr_memb )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );

	  nm = 0;
	  do { 
	       rtemp[nm] = mds->pixel_err[nm];
	       IEEE_Swap__FLT( &rtemp[nm] );
	  } while ( ++nm < nr_memb );
	  if ( fwrite( rtemp, ENVI_FLOAT, nr_memb, fd ) != nr_memb )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  free( rtemp );
#else
	  if ( fwrite( mds->pixel_val, nr_memb * ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  if ( fwrite( mds->pixel_err, nr_memb * ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
#endif
	  bytes_written += 2 * nr_memb * ENVI_FLOAT;
/* 16 */
	  switch ( source ) {
	  case SCIA_NADIR:
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       nr_byte = mds->num_obs * sizeof( struct geoN_scia );
	       if ( (geoN = (struct geoN_scia *) malloc( nr_byte )) == NULL ) 
		    NADC_RETURN_ERROR( NADC_ERR_ALLOC, "geoN" );
	       (void) memcpy( geoN, mds->geoN, nr_byte );
	       for ( ni = 0; ni < mds->num_obs; ni++ )
		    Sun2Intel_GeoN( &geoN[ni] );
	       bytes_written += SCIA_LV1_WR_GeoN( mds->num_obs, geoN, fd );
	       free( geoN );
#else
	       bytes_written += SCIA_LV1_WR_GeoN( mds->num_obs, mds->geoN, fd);
#endif
	       break;
	  case SCIA_LIMB:
	  case SCIA_OCCULT:
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       nr_byte = mds->num_obs * sizeof( struct geoL_scia );
	       if ( (geoL = (struct geoL_scia *) malloc( nr_byte )) == NULL ) 
		    NADC_RETURN_ERROR( NADC_ERR_ALLOC, "geoL" );
	       (void) memcpy( geoL, mds->geoL, nr_byte );
	       for ( ni = 0; ni < mds->num_obs; ni++ )
		    Sun2Intel_GeoL( &geoL[ni] );
	       bytes_written += SCIA_LV1_WR_GeoL( mds->num_obs, geoL, fd );
	       free( geoL );
#else
	       bytes_written += SCIA_LV1_WR_GeoL( mds->num_obs, mds->geoL, fd);
#endif
	       break;
	  case SCIA_MONITOR:
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       nr_byte = mds->num_obs * sizeof( struct geoC_scia );
	       if ( (geoC = (struct geoC_scia *) malloc( nr_byte )) == NULL ) 
		    NADC_RETURN_ERROR( NADC_ERR_ALLOC, "geoC" );
	       (void) memcpy( geoC, mds->geoC, nr_byte );
	       for ( ni = 0; ni < mds->num_obs; ni++ )
		    Sun2Intel_GeoC( &geoC[ni] );
	       bytes_written += SCIA_LV1_WR_GeoC( mds->num_obs, geoC, fd );
	       free( geoC );
#else
	       bytes_written += SCIA_LV1_WR_GeoC( mds->num_obs, mds->geoC, fd);
#endif
	       break;
	  }
/*
 * check written bytes against DSR length
 */
	  if ( bytes_written != mds->dsr_length )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_SIZE, "MDS(1c)"  );

	  dsd.size += bytes_written;
	  mds++;
     } while ( ++dsd.num_dsr < num_mds );
/*
 * update list of written DSD records
 */
     SCIA_LV1_ADD_DSD( &dsd );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1C_WR_MDS_PMD
.PURPOSE     write SCIAMACHY level 1c PMD MDS
.INPUT/OUTPUT
  call as    SCIA_LV1C_WR_MDS_PMD( fd, pmd );
     input:  
            FILE    *fd               : (open) stream pointer
            struct mds1c_scia  *pmd   : structure for level 1c PMD MDS

.RETURNS     nothing, 
             error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1C_WR_MDS_PMD( FILE *fd, const struct mds1c_pmd *pmd )
     /*@globals  errno, nadc_stat, nadc_err_stack, source;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fd, source@*/
{
     register unsigned int bytes_written;

     size_t         nr_byte;
     unsigned short usbuff;
     unsigned int   ubuff;
     float          rbuff;

     struct dsd_envi dsd = {
          "", "M",
          "                                                              ",
          0u, 0u, 0u, -1
     };

#ifdef _SWAP_TO_LITTLE_ENDIAN
     register unsigned short ni;

     float          *rtemp;

     struct mjd_envi   mjd;
     struct geoN_scia *geoN;
     struct geoL_scia *geoL;
#endif
/*
 * set variable source (= type of MDS)
 */
     source = (int) pmd->type_mds;
     switch ( source ) {
     case SCIA_NADIR:
	  (void) strcpy( dsd.name, "NADIR_PMD" );
	  break;
     case SCIA_LIMB:
	  (void) strcpy( dsd.name, "LIMB_PMD" );
	  break;
     case SCIA_OCCULT:
	  (void) strcpy( dsd.name, "OCCULTATION_PMD" );
	  break;
     case SCIA_MONITOR:
	  (void) strcpy( dsd.flname, "MISSING" );
	  SCIA_LV1_ADD_DSD( &dsd );
	  break;
     }
/*
 * check number of MDS records
 */
     if ( pmd == NULL ) {
	  SCIA_LV1_ADD_DSD( &dsd );
	  return;
     }
/*
 * write data set records
 */
     bytes_written = 0u;
/* 1 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
     mjd.days = byte_swap_32( pmd->mjd.days );
     mjd.secnd = byte_swap_u32( pmd->mjd.secnd );
     mjd.musec = byte_swap_u32( pmd->mjd.musec );	  
     if ( fwrite( &mjd, sizeof( struct mjd_envi ), 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
#else
     if ( fwrite( &pmd->mjd, sizeof( struct mjd_envi ), 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
#endif
     bytes_written += sizeof( struct mjd_envi );   
/* 2 */
     ubuff = pmd->dsr_length;
#ifdef _SWAP_TO_LITTLE_ENDIAN
     ubuff = byte_swap_u32( ubuff );
#endif
     if ( fwrite( &ubuff, ENVI_UINT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
     bytes_written += ENVI_UINT;   
/* 3 */
     if ( fwrite( &pmd->quality_flag, ENVI_CHAR, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
     bytes_written += ENVI_CHAR;   
/* 4 */
     rbuff = pmd->orbit_phase; 
#ifdef _SWAP_TO_LITTLE_ENDIAN
     IEEE_Swap__FLT( &rbuff );
#endif
     if ( fwrite( &rbuff, ENVI_FLOAT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
     bytes_written += ENVI_FLOAT;   
/* 5 */
     usbuff = (unsigned short) pmd->category;
#ifdef _SWAP_TO_LITTLE_ENDIAN
     usbuff = byte_swap_u16( usbuff );
#endif
     if ( fwrite( &usbuff, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
     bytes_written += ENVI_USHRT;   
/* 6 */
     usbuff = (unsigned short) pmd->state_id;
#ifdef _SWAP_TO_LITTLE_ENDIAN
     usbuff = byte_swap_u16( usbuff );
#endif
     if ( fwrite( &usbuff, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
     bytes_written += ENVI_USHRT;   
/* 7 */
     usbuff = (unsigned short) pmd->dur_scan;
#ifdef _SWAP_TO_LITTLE_ENDIAN
     usbuff = byte_swap_u16( usbuff );
#endif
     if ( fwrite( &usbuff, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
     bytes_written += ENVI_USHRT;   
/* 8 */
     usbuff = (unsigned short) pmd->num_pmd;
#ifdef _SWAP_TO_LITTLE_ENDIAN
     usbuff = byte_swap_u16( usbuff );
#endif
     if ( fwrite( &usbuff, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
     bytes_written += ENVI_USHRT;   
/* 9 */
     usbuff = (unsigned short) pmd->num_geo;
#ifdef _SWAP_TO_LITTLE_ENDIAN
     usbuff = byte_swap_u16( usbuff );
#endif
     if ( fwrite( &usbuff, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
     bytes_written += ENVI_USHRT;   
/* 10 */
     nr_byte = (size_t) pmd->num_pmd * ENVI_FLOAT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
     if ( (rtemp = (float *) malloc( nr_byte )) == NULL ) 
	  NADC_RETURN_ERROR( NADC_ERR_ALLOC, "rtemp" );
     for ( ni = 0; ni < pmd->num_pmd; ni++ ) {
	  rtemp[ni] = pmd->int_pmd[ni];
	  IEEE_Swap__FLT( &rtemp[ni] );
     }
     if ( fwrite( rtemp, nr_byte, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
     free( rtemp );
#else
     if ( fwrite( pmd->int_pmd, nr_byte, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
#endif
     bytes_written += pmd->num_pmd * ENVI_FLOAT;   
/* 11 */
     switch ( source ) {
     case SCIA_NADIR:
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  nr_byte = pmd->num_geo * sizeof( struct geoN_scia );
	  if ( (geoN = (struct geoN_scia *) malloc( nr_byte )) == NULL ) 
	       NADC_RETURN_ERROR( NADC_ERR_ALLOC, "geoN" );
	  (void) memcpy( geoN, pmd->geoN, nr_byte );
	  for ( ni = 0; ni < pmd->num_geo; ni++ )
	       Sun2Intel_GeoN( &geoN[ni] );
	  bytes_written += SCIA_LV1_WR_GeoN( pmd->num_geo, geoN, fd );
	  free( geoN );
#else
	  bytes_written += SCIA_LV1_WR_GeoN( pmd->num_geo, pmd->geoN, fd);
#endif
	  break;
     case SCIA_LIMB:
     case SCIA_OCCULT:
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  nr_byte = pmd->num_geo * sizeof( struct geoL_scia );
	  if ( (geoL = (struct geoL_scia *) malloc( nr_byte )) == NULL ) 
	       NADC_RETURN_ERROR( NADC_ERR_ALLOC, "geoL" );
	  (void) memcpy( geoL, pmd->geoL, nr_byte );
	  for ( ni = 0; ni < pmd->num_geo; ni++ )
	       Sun2Intel_GeoL( &geoL[ni] );
	  bytes_written += SCIA_LV1_WR_GeoL( pmd->num_geo, geoL, fd );
	  free( geoL );
#else
	  bytes_written += SCIA_LV1_WR_GeoL( pmd->num_geo, pmd->geoL, fd);
#endif
	  break;
     }
/*
 * check written bytes against DSR length
 */
     if ( bytes_written != pmd->dsr_length )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_SIZE, "MDS(1b)"  );
/*
 * update list of written DSD records
 */
     dsd.num_dsr = 1u;
     dsd.size += bytes_written;
     SCIA_LV1_ADD_DSD( &dsd );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1C_WR_MDS_POLV
.PURPOSE     write SCIAMACHY level 1c POLV MDS
.INPUT/OUTPUT
  call as    SCIA_LV1C_WR_MDS_POLV( fd, polV );
     input:  
            FILE    *fd               : (open) stream pointer
            struct mds1c_scia  *polV  : structure for level 1c POLV MDS

.RETURNS     nothing, 
             error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1C_WR_MDS_POLV( FILE *fd, const struct mds1c_polV *polV )
     /*@globals  errno, nadc_stat, nadc_err_stack, source;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fd, source@*/
{
     register unsigned int bytes_written;

     unsigned short usbuff;
     unsigned int   ubuff;
     float          rbuff;

     struct dsd_envi dsd = {
          "", "M",
          "                                                              ",
          0u, 0u, 0u, -1
     };

#ifdef _SWAP_TO_LITTLE_ENDIAN
     register unsigned short ni;

     unsigned short *utemp;
     size_t         nr_byte;

     struct mjd_envi   mjd;
     struct geoN_scia *geoN;
     struct geoL_scia *geoL;
     struct polV_scia *polval;
#endif
/*
 * set variable source (= type of MDS)
 */
     source = (int) polV->type_mds;
     switch ( source ) {
     case SCIA_NADIR:
	  (void) strcpy( dsd.name, "NADIR_FRAC_POL" );
	  break;
     case SCIA_LIMB:
	  (void) strcpy( dsd.name, "LIMB_FRAC_POL" );
	  break;
     case SCIA_OCCULT:
	  (void) strcpy( dsd.name, "OCCULTATION_FRAC_POL" );
	  break;
     case SCIA_MONITOR:
	  (void) strcpy( dsd.flname, "MISSING" );
	  SCIA_LV1_ADD_DSD( &dsd );
	  break;
     }
/*
 * check number of MDS records
 */
     if ( polV == NULL ) {
	  SCIA_LV1_ADD_DSD( &dsd );
	  return;
     }
/*
 * write data set records
 */
     bytes_written = 0u;
/* 1 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
     mjd.days = byte_swap_32( polV->mjd.days );
     mjd.secnd = byte_swap_u32( polV->mjd.secnd );
     mjd.musec = byte_swap_u32( polV->mjd.musec );	  
     if ( fwrite( &mjd, sizeof( struct mjd_envi ), 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
#else
     if ( fwrite( &polV->mjd, sizeof( struct mjd_envi ), 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
#endif
     bytes_written += sizeof( struct mjd_envi );
/* 2 */
     ubuff = polV->dsr_length;
#ifdef _SWAP_TO_LITTLE_ENDIAN
     ubuff = byte_swap_u32( ubuff );
#endif
     if ( fwrite( &ubuff, ENVI_UINT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
     bytes_written += ENVI_UINT;
/* 3 */
     if ( fwrite( &polV->quality_flag, ENVI_CHAR, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
     bytes_written += ENVI_CHAR;
/* 4 */
     rbuff = polV->orbit_phase; 
#ifdef _SWAP_TO_LITTLE_ENDIAN
     IEEE_Swap__FLT( &rbuff );
#endif
     if ( fwrite( &rbuff, ENVI_FLOAT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
     bytes_written += ENVI_FLOAT;
/* 5 */
     usbuff = (unsigned short) polV->category;
#ifdef _SWAP_TO_LITTLE_ENDIAN
     usbuff = byte_swap_u16( usbuff );
#endif
     if ( fwrite( &usbuff, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
     bytes_written += ENVI_USHRT;
/* 6 */
     usbuff = (unsigned short) polV->state_id;
#ifdef _SWAP_TO_LITTLE_ENDIAN
     usbuff = byte_swap_u16( usbuff );
#endif
     if ( fwrite( &usbuff, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
     bytes_written += ENVI_USHRT;
/* 7 */
     usbuff = (unsigned short) polV->dur_scan;
#ifdef _SWAP_TO_LITTLE_ENDIAN
     usbuff = byte_swap_u16( usbuff );
#endif
     if ( fwrite( &usbuff, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
     bytes_written += ENVI_USHRT;
/* 8 */
     usbuff = (unsigned short) polV->num_geo;
#ifdef _SWAP_TO_LITTLE_ENDIAN
     usbuff = byte_swap_u16( usbuff );
#endif
     if ( fwrite( &usbuff, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
     bytes_written += ENVI_USHRT;
/* 9 */
     usbuff = (unsigned short) polV->total_polV;
#ifdef _SWAP_TO_LITTLE_ENDIAN
     usbuff = byte_swap_u16( usbuff );
#endif
     if ( fwrite( &usbuff, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
     bytes_written += ENVI_USHRT;
/* 10 */
     usbuff = (unsigned short) polV->num_diff_intg;
#ifdef _SWAP_TO_LITTLE_ENDIAN
     usbuff = byte_swap_u16( usbuff );
#endif
     if ( fwrite( &usbuff, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
     bytes_written += ENVI_USHRT;
/* 11 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
     utemp = (unsigned short *) calloc( MAX_CLUSTER, ENVI_USHRT );
     if ( utemp == NULL ) 
	  NADC_RETURN_ERROR( NADC_ERR_ALLOC, "utemp" );
     for ( ni = 0; ni < polV->num_diff_intg; ni++ )
	  utemp[ni] = byte_swap_u16( polV->intg_times[ni] );
     if ( fwrite( utemp, (size_t) MAX_CLUSTER * ENVI_USHRT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
#else
     if ( fwrite( polV->intg_times, MAX_CLUSTER * ENVI_USHRT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
#endif
     bytes_written += MAX_CLUSTER * ENVI_USHRT;
/* 12 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
     for ( ni = 0; ni < polV->num_diff_intg; ni++ )
	  utemp[ni] = byte_swap_u16( polV->num_polar[ni] );
     if ( fwrite( utemp, (size_t) MAX_CLUSTER * ENVI_USHRT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
     free( utemp );
#else
     if ( fwrite( polV->num_polar, (size_t) MAX_CLUSTER * ENVI_USHRT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
#endif
     bytes_written += MAX_CLUSTER * ENVI_USHRT;
/* 13 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
     nr_byte = polV->total_polV * sizeof( struct polV_scia );
     if ( (polval = (struct polV_scia *) malloc( nr_byte )) == NULL ) 
	  NADC_RETURN_ERROR( NADC_ERR_ALLOC, "polval" );
     (void) memcpy( polval, polV->polV, nr_byte );
     for ( ni = 0; ni < polV->total_polV; ni++ )
	  Sun2Intel_polV( polval + ni );
     bytes_written += SCIA_LV1_WR_PolV( polV->total_polV, polval, fd );
     free( polval );
#else
     bytes_written += SCIA_LV1_WR_PolV( polV->total_polV, polV->polV, fd);
#endif
/* 14 */
     switch ( source ) {
     case SCIA_NADIR:
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  nr_byte = polV->num_geo * sizeof( struct geoN_scia );
	  if ( (geoN = (struct geoN_scia *) malloc( nr_byte )) == NULL ) 
	       NADC_RETURN_ERROR( NADC_ERR_ALLOC, "geoN" );
	  (void) memcpy( geoN, polV->geoN, nr_byte );
	  for ( ni = 0; ni < polV->num_geo; ni++ )
	       Sun2Intel_GeoN( &geoN[ni] );
	  bytes_written += SCIA_LV1_WR_GeoN( polV->num_geo, geoN, fd );
	  free( geoN );
#else
	  bytes_written += 
	       SCIA_LV1_WR_GeoN( polV->num_geo, polV->geoN, fd );
#endif
	  break;
     case SCIA_LIMB:
     case SCIA_OCCULT:
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  nr_byte = polV->num_geo * sizeof( struct geoL_scia );
	  if ( (geoL = (struct geoL_scia *) malloc( nr_byte )) == NULL ) 
	       NADC_RETURN_ERROR( NADC_ERR_ALLOC, "geoL" );
	  (void) memcpy( geoL, polV->geoL, nr_byte );
	  for ( ni = 0; ni < polV->num_geo; ni++ )
	       Sun2Intel_GeoL( &geoL[ni] );
	  bytes_written += SCIA_LV1_WR_GeoL( polV->num_geo, geoL, fd );
	  free( geoL );
#else
	  bytes_written += 
	       SCIA_LV1_WR_GeoL( polV->num_geo, polV->geoL, fd );
#endif
	  break;
     }
/*
 * check written bytes against DSR length
 */
     if ( bytes_written != polV->dsr_length )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_SIZE, "MDS(1b)"  );
/*
 * update list of written DSD records
 */
     dsd.num_dsr = 1u;
     dsd.size = bytes_written;
     SCIA_LV1_ADD_DSD( &dsd );
}
