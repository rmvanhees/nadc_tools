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

.IDENTifer   SCIA_LV1_RD_MDS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1b/1c data
.LANGUAGE    ANSI C
.PURPOSE     read SCIAMACHY level 1b or 1c Measurement Data Sets 
.RETURNS     number of MDS records read
.COMMENTS    None
.ENVIRONment None
.VERSION      6.2   01-Jun-2006	bugfix determination pixeltype (L1c), RvH
	      6.1.1 27-Feb-2006	added check bytes-read & state definition, RvH
	      6.1   30-Jan-2006	bug-fix pixel-type of Limb-measurements, RvH
	      6.0   07-Dec-2005	removed esig/esigc from MDS(1b)-struct,
				renamed pixel_val_err to pixel_err, RvH
              5.1   17-Oct-2005	read only one MDS_PMD or MDS_POLV records
                                do propoer initialisation of MDS records
                                try to release all possible allocated memory
              5.0   11-Oct-2005	code clean-ups, adoption to modified function
                                calls, RvH
              4.13  19-Jan-2005	fixed several bugs in the LV1C read routines,
                                due to modifications in the struct-definitions
                                implemented mid-2004, RvH
              4.12  17-Jan-2004	byte-swap error in reading level 0 header, RvH
              4.11  17-Feb-2003	another bug: did not update state-records 
                                correctly for level 1C, RvH
              4.10  11-Dec-2002	bug: did not update state-record correctly, RvH
              4.9   06-Dec-2002	bug: crash while reading several clusters, RvH
              4.8   07-Nov-2002	added pixel_type to geoN, RvH
              4.7   09-Aug-2002	applied some splint suggestions, RvH
              4.6   02-Aug-2002	usage of CAL_OPTIONS (level 1c), RvH 
              4.5   02-Aug-2002	cluster instead of channel selection, RvH 
              4.4   08-Jul-2002	added selection on channels for level 1b, RvH 
              4.3   23-Apr-2002	added selection on channels for level 1c, RvH
              4.2   18-Apr-2002	fully implemented reading of level 1c datasets,
	                        RvH 
              4.1   27-Mar-2002	add reading of 1c specific DSD's, RvH 
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
#define INT_TIME_MIN  0.5                   /* 0.03125 / (1/16) sec */

#define SCIA_SIG     sizeof( struct Sig_scia )
#define SCIA_SIGC    sizeof( struct Sigc_scia )

/*+++++ Global Variables +++++*/
        /* NONE */

/*+++++ Static Variables +++++*/
static unsigned short sec_in_scan = 1;
static unsigned short indx_limb = 0;

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#ifdef _SWAP_TO_LITTLE_ENDIAN
#include "swap_lv1_mds.inc"
#include "swap_lv1c_mds.inc"
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_GeoN
.PURPOSE     General function to read level 1 Nadir geolocation records
.INPUT/OUTPUT
  call as    nr_byte = SCIA_LV1_RD_GeoN( mds_char, num_geo, geoN );
     input:
            char *mds_char         :  character buffer to read data from
	    unsigned short num_geo :  number of Nadir Gelocation records
    output:
	    struct geoN_scia *geoN :  Nadir Gelocation records

.RETURNS     size of the geolocation records (bytes)
.COMMENTS    static function
-------------------------*/
static
size_t SCIA_LV1_RD_GeoN( const char *mds_char, unsigned short num_geo, 
                       /*@out@*/ struct geoN_scia *geoN )
{
     register const char *mds_pntr = mds_char;

     register unsigned short na = 0;
     do {
	  (void) memcpy( &geoN->pos_esm, mds_pntr, ENVI_FLOAT );
	  mds_pntr += ENVI_FLOAT;
	  (void) memcpy( geoN->sun_zen_ang, mds_pntr, 3 * ENVI_FLOAT );
	  mds_pntr += 3 * ENVI_FLOAT;
	  (void) memcpy( geoN->sun_azi_ang, mds_pntr, 3 * ENVI_FLOAT );
	  mds_pntr += 3 * ENVI_FLOAT;
	  (void) memcpy( geoN->los_zen_ang, mds_pntr, 3 * ENVI_FLOAT );
	  mds_pntr += 3 * ENVI_FLOAT;
	  (void) memcpy( geoN->los_azi_ang, mds_pntr, 3 * ENVI_FLOAT );
	  mds_pntr += 3 * ENVI_FLOAT;
	  (void) memcpy( &geoN->sat_h, mds_pntr, ENVI_FLOAT );
	  mds_pntr += ENVI_FLOAT;
	  (void) memcpy( &geoN->earth_rad, mds_pntr, ENVI_FLOAT );
	  mds_pntr += ENVI_FLOAT;
	  (void) memcpy( &geoN->sub_sat_point, mds_pntr, 
			 sizeof(struct coord_envi) );
	  mds_pntr += sizeof(struct coord_envi);
	  (void) memcpy( geoN->corner, mds_pntr, 
			 NUM_CORNERS * sizeof(struct coord_envi) );
	  mds_pntr += NUM_CORNERS * sizeof(struct coord_envi);
	  (void) memcpy( &geoN->center, mds_pntr, sizeof(struct coord_envi) );
	  mds_pntr += sizeof(struct coord_envi);
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_GeoN( geoN );
#endif
     } while ( ++geoN, ++na < num_geo );

     return (size_t) (mds_pntr - mds_char);
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_GeoL
.PURPOSE     General function to read level 1 Limb geolocation records
.INPUT/OUTPUT
  call as    nr_byte = SCIA_LV1_RD_GeoL( mds_char, num_geo, geoL );
     input:
            char *mds_char         :  character buffer to read data from
	    unsigned short num_geo :  number of Limb Gelocation records
    output:
	    struct geoL_scia *geoL :  Limb Gelocation records
            
.RETURNS     size of the geolocation records (bytes)
.COMMENTS    static function
-------------------------*/
static
size_t SCIA_LV1_RD_GeoL( const char *mds_char, unsigned short num_geo, 
                       /*@out@*/ struct geoL_scia *geoL )
{
     register const char *mds_pntr = mds_char;
     register unsigned short na = 0;

     do {
	  (void) memcpy( &geoL->pos_esm, mds_pntr, ENVI_FLOAT );
	  mds_pntr += ENVI_FLOAT;
	  (void) memcpy( &geoL->pos_asm, mds_pntr, ENVI_FLOAT );
	  mds_pntr += ENVI_FLOAT;
	  (void) memcpy( geoL->sun_zen_ang, mds_pntr, 3 * ENVI_FLOAT );
	  mds_pntr += 3 * ENVI_FLOAT;
	  (void) memcpy( geoL->sun_azi_ang, mds_pntr, 3 * ENVI_FLOAT );
	  mds_pntr += 3 * ENVI_FLOAT;
	  (void) memcpy( geoL->los_zen_ang, mds_pntr, 3 * ENVI_FLOAT );
	  mds_pntr += 3 * ENVI_FLOAT;
	  (void) memcpy( geoL->los_azi_ang, mds_pntr, 3 * ENVI_FLOAT );
	  mds_pntr += 3 * ENVI_FLOAT;
	  (void) memcpy( &geoL->sat_h, mds_pntr, ENVI_FLOAT );
	  mds_pntr += ENVI_FLOAT;
	  (void) memcpy( &geoL->earth_rad, mds_pntr, ENVI_FLOAT );
	  mds_pntr += ENVI_FLOAT;
	  (void) memcpy( &geoL->sub_sat_point, mds_pntr, 
			   sizeof(struct coord_envi) );
	  mds_pntr += sizeof(struct coord_envi);
	  (void) memcpy( geoL->tang_ground_point, mds_pntr, 
			   3 * sizeof(struct coord_envi) );
	  mds_pntr += 3 * sizeof(struct coord_envi);
	  (void) memcpy( &geoL->tan_h, mds_pntr, 3 * ENVI_FLOAT );
	  mds_pntr += 3 * ENVI_FLOAT;
	  (void) memcpy( &geoL->dopp_shift, mds_pntr, ENVI_FLOAT );
	  mds_pntr += ENVI_FLOAT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_GeoL( geoL );
#endif
     } while ( ++geoL, ++na < num_geo );

     return (size_t) (mds_pntr - mds_char);
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_GeoC
.PURPOSE     General function to read level 1 Monitoring geolocation records
.INPUT/OUTPUT
  call as    nr_byte = SCIA_LV1_RD_GeoC( mds_char, num_geo, geoC );
     input:
            char *mds_char         :  character buffer to read data from
	    unsigned short num_geo :  number of Monitoring Gelocation records
    output:
	    struct geoC_scia *geoC :  Monitoring Gelocation records

.RETURNS     size of the geolocation records (bytes)
.COMMENTS    static function
-------------------------*/
static
size_t SCIA_LV1_RD_GeoC( const char *mds_char, unsigned short num_geo, 
                       /*@out@*/ struct geoC_scia *geoC )
{
     register const char *mds_pntr = mds_char;
     register unsigned short na = 0;

     do {
	  (void) memcpy( &geoC->pos_esm, mds_pntr, ENVI_FLOAT );
	  mds_pntr += ENVI_FLOAT;
	  (void) memcpy( &geoC->pos_asm, mds_pntr, ENVI_FLOAT );
	  mds_pntr += ENVI_FLOAT;
	  (void) memcpy( &geoC->sun_zen_ang, mds_pntr, ENVI_FLOAT );
	  mds_pntr += ENVI_FLOAT;
	  (void) memcpy( &geoC->sub_sat_point, mds_pntr, 
			   sizeof(struct coord_envi) );
	  mds_pntr += sizeof(struct coord_envi);
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_GeoC( geoC );
#endif
     } while ( ++geoC, ++na < num_geo );

     return (size_t) (mds_pntr - mds_char);
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_L0Hdr
.PURPOSE     General function to read level 0 MDS headers
.INPUT/OUTPUT
  call as    nr_byte = SCIA_LV1_RD_LV0Hdr( mds_char, num_lv0hdr, lv0 );
     input:
            char *mds_char            :   character buffer to read data from
	    unsigned short num_lv0hdr :   number of level 0 MDS headers
	    struct lv0_hdr *lv0       :   level 0 MDS headers

.RETURNS     total size of the level 0 MDS headers records (bytes)
.COMMENTS    static function
-------------------------*/
static inline
size_t SCIA_LV1_RD_LV0Hdr( const char *mds_char, unsigned short num_lv0hdr, 
			   /*@out@*/ struct lv0_hdr *lv0 )
{
     register const char *mds_pntr = mds_char;
     register unsigned short nl = 0;

     do {
	  (void) memcpy( &lv0->packet_hdr.api.two_byte, mds_pntr, ENVI_USHRT );
	  mds_pntr += ENVI_USHRT;
	  (void) memcpy( &lv0->packet_hdr.seq_cntrl, mds_pntr, ENVI_USHRT );
	  mds_pntr += ENVI_USHRT;
	  (void) memcpy( &lv0->packet_hdr.length, mds_pntr, ENVI_USHRT );
	  mds_pntr += ENVI_USHRT;

	  (void) memcpy( &lv0->data_hdr.length, mds_pntr, ENVI_USHRT );
	  mds_pntr += ENVI_USHRT;
	  (void) memcpy( &lv0->data_hdr.category, mds_pntr, ENVI_UCHAR );
	  mds_pntr += ENVI_UCHAR;
	  (void) memcpy( &lv0->data_hdr.state_id, mds_pntr, ENVI_UCHAR );
	  mds_pntr += ENVI_UCHAR;
	  (void) memcpy( &lv0->data_hdr.on_board_time, mds_pntr, ENVI_UINT );
	  mds_pntr += ENVI_UINT;
	  (void) memcpy( &lv0->data_hdr.rdv.two_byte, mds_pntr, ENVI_USHRT );
	  mds_pntr += ENVI_USHRT;
	  (void) memcpy( &lv0->data_hdr.id.two_byte, mds_pntr, ENVI_USHRT );
	  mds_pntr += ENVI_USHRT;
	  (void) memcpy( &lv0->bcps, mds_pntr, ENVI_USHRT );
	  mds_pntr += ENVI_USHRT;
	  (void) memcpy( &lv0->pmtc_hdr.pmtc_1.two_byte, 
			 mds_pntr, ENVI_USHRT );
	  mds_pntr += ENVI_USHRT;
	  (void) memcpy( &lv0->pmtc_hdr.scanner_mode, mds_pntr, ENVI_USHRT );
	  mds_pntr += ENVI_USHRT;
	  (void) memcpy( &lv0->pmtc_hdr.az_param.four_byte, 
			 mds_pntr, ENVI_UINT );
	  mds_pntr += ENVI_UINT;
	  (void) memcpy( &lv0->pmtc_hdr.elv_param.four_byte, 
			 mds_pntr, ENVI_UINT );
	  mds_pntr += ENVI_UINT;
	  (void) memcpy( &lv0->pmtc_hdr.factor, mds_pntr, 6 * ENVI_UCHAR );
	  mds_pntr += 6 * ENVI_UCHAR;
	  (void) memcpy( &lv0->orbit_vector, mds_pntr, 8 * ENVI_INT );
	  mds_pntr += 8 * ENVI_INT;
	  (void) memcpy( &lv0->num_chan, mds_pntr, ENVI_USHRT );
	  mds_pntr += ENVI_USHRT;
     } while ( lv0++, ++nl < num_lv0hdr );

     return (size_t) (mds_pntr - mds_char);
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_PolV
.PURPOSE     General function to read level 1 fractional polarisation records
.INPUT/OUTPUT
  call as    nr_byte = SCIA_LV1_RD_PolV( mds_char, num_polV, polV );
     input:
            char *mds_char          :   character buffer to read data from
            unsigned short num_polV :   number of polarisation records
    output:
	    struct polV_scia *polV  :   fractional polarisation records

.RETURNS     size of the fractional polarisation records (bytes)
.COMMENTS    static function
-------------------------*/
static
size_t SCIA_LV1_RD_PolV( const char *mds_char, unsigned short num_polV, 
			 /*@out@*/ struct polV_scia *polV )
{
     register const char *mds_pntr = mds_char;
     register unsigned short np = 0;
     register size_t nr_byte;

     do {
	  nr_byte = NUM_FRAC_POLV * ENVI_FLOAT;
	  (void) memcpy( polV->Q, mds_pntr, nr_byte );
	  mds_pntr += nr_byte;
	  (void) memcpy( polV->error_Q, mds_pntr, nr_byte );
	  mds_pntr += nr_byte;
	  (void) memcpy( polV->U, mds_pntr, nr_byte );
	  mds_pntr += nr_byte;
	  (void) memcpy( polV->error_U, mds_pntr, nr_byte );
	  mds_pntr += nr_byte;
	  nr_byte = (NUM_FRAC_POLV+1) * ENVI_FLOAT;
	  (void) memcpy( polV->rep_wv, mds_pntr, nr_byte );
	  mds_pntr += nr_byte;
	  (void) memcpy( &polV->gdf.p_bar, mds_pntr, ENVI_FLOAT );
	  mds_pntr += ENVI_FLOAT;
	  (void) memcpy( &polV->gdf.beta, mds_pntr, ENVI_FLOAT );
	  mds_pntr += ENVI_FLOAT;
	  (void) memcpy( &polV->gdf.w0, mds_pntr, ENVI_FLOAT );
	  mds_pntr += ENVI_FLOAT;
     } while ( polV++, ++np < num_polV );

     return (size_t) (mds_pntr - mds_char);
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_ONE_MDS
.PURPOSE     General function to read one Measurement Data Set
.INPUT/OUTPUT
  call as   SCIA_LV1_RD_ONE_MDS( fd, clus_mask, state, mds ); 
     input:  
            FILE    *fd               : (open) stream pointer
	    ulong64 clus_mask         : mask for cluster selection
 in/output:  
	    struct state1_scia *state : structure with States of the product
    output:  
            struct mds1_scia   **mds  : structure for level 1b MDS

.RETURNS     exits on failure
.COMMENTS    static function
-------------------------*/
static
void SCIA_LV1_RD_ONE_MDS( FILE *fd, unsigned long long clus_mask,
			  const struct state1_scia *state,
			  /*@partial@*/ struct mds1_scia *mds )
       /*@globals  errno, nadc_stat, nadc_err_stack, source;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, mds@*/
{
     register char           *mds_pntr;
     register unsigned short na, nc, ncc, ni, nr;

     char   *mds_char;
     char   msg[64];
     size_t nr_byte;

     unsigned char *glint_flags;

     const int intg_per_sec = 16 / state->intg_times[state->num_intg-1];
     const unsigned short indx_deep_space = state->num_aux 
	  - (state->intg_times[0] / state->intg_times[state->num_intg-1]);
/*
 * allocate memory for the Sun glint/Rainbow flags
 */
     glint_flags = (unsigned char *) malloc( (size_t) mds->n_aux );
     if ( glint_flags == NULL ) 
	  NADC_RETURN_ERROR( NADC_ERR_ALLOC, "glint_flags" );
/*
 * allocate memory to temporary store data for output structure
 */
     mds_char = (char *) malloc( (size_t) state->length_dsr );
     if ( mds_char == NULL ) {
	  free( glint_flags );
          NADC_RETURN_ERROR( NADC_ERR_ALLOC, "mds_char" );
     }
/*
 * read all MDS parameters of this state
 */
     if ( fread( mds_char, (size_t) state->length_dsr, 1, fd ) != 1 ) {
	  (void) snprintf( msg, 64, "MDS[%-u]: read failed", state->indx );
	  NADC_GOTO_ERROR( NADC_ERR_FILE_RD, msg );
     }
/*
 * read data buffer to MDS structure
 */
     mds_pntr = mds_char;
     (void) memcpy( &mds->mjd, mds_pntr, sizeof( struct mjd_envi ) );
     mds_pntr += sizeof( struct mjd_envi );
     (void) memcpy( &mds->dsr_length, mds_pntr, ENVI_UINT );
     mds_pntr += ENVI_UINT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
     mds->dsr_length = byte_swap_u32( mds->dsr_length );
#endif
     if ( mds->dsr_length != state->length_dsr ) {
	  (void) snprintf( msg, 64, 
			   "MDS[%-u]: Size according to State/DSR = %-u/%-u",
			   state->indx, state->length_dsr, mds->dsr_length );
	  NADC_GOTO_ERROR( NADC_ERR_PDS_SIZE, msg );
     }
     (void) memcpy( &mds->quality_flag, mds_pntr, ENVI_CHAR );
     mds_pntr += ENVI_CHAR;
/*
 * read scale factor [SCIENCE_CHANNELS]
 */
     nr_byte = (size_t) SCIENCE_CHANNELS * ENVI_UCHAR;
     (void) memcpy( mds->scale_factor, mds_pntr, nr_byte );
     mds_pntr += nr_byte;
/*
 * read satellite flags
 */
     nr_byte = mds->n_aux * ENVI_UCHAR;
     if ( (mds->sat_flags = (unsigned char *) malloc( nr_byte )) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "sat_flags" );
     (void) memcpy( mds->sat_flags, mds_pntr, nr_byte );
     mds_pntr += nr_byte;
/*
 * read red grass flags
 */
     nr_byte = (size_t) state->num_clus * mds->n_aux;
     if ( (mds->red_grass = (unsigned char *) malloc( nr_byte )) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "red_grass" );
     (void) memcpy( mds->red_grass, mds_pntr, nr_byte );
     mds_pntr += nr_byte;
/*
 * read Sun glint flags
 */
     if ( source != SCIA_MONITOR ) {
	  (void) memcpy( glint_flags, mds_pntr, (size_t) mds->n_aux );
	  mds_pntr += (size_t) mds->n_aux;
     }
/*
 * read geolocation
 */
     switch ( source ) {
     case SCIA_NADIR:
	  mds->geoN = (struct geoN_scia *) 
	       malloc( mds->n_aux * sizeof( struct geoN_scia ) );
	  if ( mds->geoN == NULL )
	       NADC_GOTO_ERROR( NADC_ERR_ALLOC, "geoN" );
	  mds_pntr += SCIA_LV1_RD_GeoN( mds_pntr, mds->n_aux, mds->geoN );

          /* set Rainbow/Sun glint flag and pixel type: 0 (= backscan) or 1 */
	  for ( na = 0; na < mds->n_aux; na++ ) {
	       mds->geoN[na].glint_flag = glint_flags[na];

	       if ( sec_in_scan == 5 ) sec_in_scan = 0;

	       if ( sec_in_scan == 0 )
		    mds->geoN[na].pixel_type = BACK_SCAN;
	       else
		    mds->geoN[na].pixel_type = FORWARD_SCAN;

	       if ( (na+1) % intg_per_sec == 0 ) sec_in_scan++;
	  }
	  break;
     case SCIA_LIMB:
	  mds->geoL = (struct geoL_scia *) 
	       malloc( mds->n_aux * sizeof( struct geoL_scia ) );
	  if ( mds->geoL == NULL )
	       NADC_GOTO_ERROR( NADC_ERR_ALLOC, "geoL" );
	  mds_pntr += SCIA_LV1_RD_GeoL( mds_pntr, mds->n_aux, mds->geoL );

          /* set Rainbow/Sun glint flags and pixel type */
	  for ( na = 0; na < mds->n_aux; na++ ) {
	       mds->geoL[na].glint_flag = glint_flags[na];
	       mds->geoL[na].pixel_type = ALONG_TANG_HGHT;

	       if ( indx_limb >= indx_deep_space )
		    mds->geoL[na].pixel_type |= DEEP_SPACE;
	       if ( ++indx_limb >= state->num_aux ) indx_limb = 0;
	  }
	  mds->geoL[0].pixel_type |= NEW_TANG_HGHT;
	  break;
     case SCIA_OCCULT:
	  mds->geoL = (struct geoL_scia *) 
	       malloc( mds->n_aux * sizeof( struct geoL_scia ) );
	  if ( mds->geoL == NULL )
	       NADC_GOTO_ERROR( NADC_ERR_ALLOC, "geoL" );
	  mds_pntr += SCIA_LV1_RD_GeoL( mds_pntr, mds->n_aux, mds->geoL );

          /* set Rainbow/Sun glint flags and pixel type */
	  for ( na = 0; na < mds->n_aux; na++ ) {
	       mds->geoL[na].glint_flag = glint_flags[na];
	       mds->geoL[na].pixel_type = ALONG_TANG_HGHT;
	  }
	  break;
     case SCIA_MONITOR:
	  mds->geoC = (struct geoC_scia *) 
	       malloc( mds->n_aux * sizeof( struct geoC_scia ) );
	  if ( mds->geoC == NULL )
	       NADC_GOTO_ERROR( NADC_ERR_ALLOC, "geoC" );
	  mds_pntr += SCIA_LV1_RD_GeoC( mds_pntr, mds->n_aux, mds->geoC );
	  break;
     }
/*
 * level 0 header
 */
     mds->lv0 = (struct lv0_hdr *) 
	  malloc( mds->n_aux * sizeof( struct lv0_hdr ) );
     if ( mds->lv0 == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "lv0" );
     mds_pntr += SCIA_LV1_RD_LV0Hdr( mds_pntr, mds->n_aux, mds->lv0 );
/*
 * PMD values
 */
     if ( source != SCIA_MONITOR ) {
	  nr_byte = mds->n_pmd * ENVI_FLOAT;
	  mds->int_pmd = (float *) malloc( nr_byte );
	  if ( mds->int_pmd == NULL )
	       NADC_GOTO_ERROR( NADC_ERR_ALLOC, "int_pmd" );
	  (void) memcpy( mds->int_pmd, mds_pntr, nr_byte );
	  mds_pntr += nr_byte;
/*
 * Fractional polarisation values
 */
	  mds->polV = (struct polV_scia *) 
	       malloc( mds->n_pol * sizeof( struct polV_scia ) );
	  if ( mds->polV == NULL )
	       NADC_GOTO_ERROR( NADC_ERR_ALLOC, "mds->polV" );
	  mds_pntr += SCIA_LV1_RD_PolV( mds_pntr, mds->n_pol, mds->polV );
/*
 * add integration times
 */
	  for ( ncc = ni = 0; ni < state->num_intg; ni++ ) {
	       unsigned short n_pol_intg = 
		    state->num_polar[ni] / state->num_dsr;

	       for ( nr = 0; nr < n_pol_intg; nr++ )
		    mds->polV[ncc + nr].intg_time = state->intg_times[ni];
	       ncc += n_pol_intg;
	  }
     }
/*
 * cluster data
 */
     (void) strcpy( msg, "" );
     nc = ncc = 0;
     do {
	  unsigned short num = 
	       state->Clcon[nc].length * state->Clcon[nc].n_read;

	  switch ( state->Clcon[nc].type ) {
	  case RSIG:
	  case ESIG:
	       if ( Get_Bit_LL( clus_mask, (unsigned char) nc ) == 0ULL ){
		    size_t byte_dest = ncc * mds->n_aux;
		    size_t byte_src  = (ncc + 1) * mds->n_aux;
		    size_t bytes_to_move = 
			 (state->num_clus - ncc - 1) * mds->n_aux;

		    (void) memmove( mds->red_grass + byte_dest, 
				    mds->red_grass + byte_src, bytes_to_move );
		    mds_pntr += num * (2 * ENVI_CHAR + ENVI_USHRT);
	       } else {
		    /* mds->clus[ncc].id = state->Clcon[nc].id; */
		    mds->clus[ncc].sig = (struct Sig_scia *)
			 malloc( (size_t) num * SCIA_SIG );
		    if ( mds->clus[ncc].sig == NULL ) {
			 (void) snprintf( msg, 25, "clus[%-hu].sig", nc );
			 NADC_GOTO_ERROR( NADC_ERR_ALLOC, msg );
		    }
		    nr = 0;
		    do {
			 (void) memcpy( &mds->clus[ncc].sig[nr].corr,
					mds_pntr, ENVI_CHAR );
			 mds_pntr += ENVI_CHAR;
			 (void) memcpy( &mds->clus[ncc].sig[nr].sign,
					mds_pntr, ENVI_USHRT );
#ifdef _SWAP_TO_LITTLE_ENDIAN
			 mds->clus[ncc].sig[nr].sign = 
			      byte_swap_u16( mds->clus[ncc].sig[nr].sign );
#endif
			 mds_pntr += ENVI_USHRT;
			 (void) memcpy( &mds->clus[ncc].sig[nr].stray,
					mds_pntr, ENVI_CHAR );
			 mds_pntr += ENVI_CHAR;
		    } while ( ++nr < num );
		    mds->clus[ncc++].n_sig = num;
	       }
	       break;
	  case RSIGC:
	  case ESIGC:
	       if ( Get_Bit_LL( clus_mask, (unsigned char) nc ) == 0ULL ){
		    size_t byte_dest = ncc * mds->n_aux;
		    size_t byte_src  = (ncc + 1) * mds->n_aux;
		    size_t bytes_to_move = 
			 (state->num_clus - ncc - 1) * mds->n_aux;

		    (void) memmove( mds->red_grass + byte_dest, 
				    mds->red_grass + byte_src, bytes_to_move );
		    mds_pntr += num * (ENVI_CHAR + ENVI_UINT);
	       } else {
		    /* mds->clus[ncc].id = state->Clcon[nc].id; */
		    mds->clus[ncc].sigc = (struct Sigc_scia *)
			 malloc( (size_t) num * SCIA_SIGC );
		    if ( mds->clus[ncc].sigc == NULL ) {
			 (void) snprintf( msg, 25, "clus[%-hu].sigc", nc );
			 NADC_GOTO_ERROR( NADC_ERR_ALLOC, msg );
		    }
		    nr = 0;
		    do {
			 (void) memcpy( 
			      &mds->clus[ncc].sigc[nr].det.four_byte,
			      mds_pntr, ENVI_UINT );
#ifdef _SWAP_TO_LITTLE_ENDIAN
			 mds->clus[ncc].sigc[nr].det.four_byte = 
			      byte_swap_u32( 
				   mds->clus[ncc].sigc[nr].det.four_byte );
#endif
			 mds_pntr += ENVI_UINT;
			 (void) memcpy( &mds->clus[ncc].sigc[nr].stray,
					mds_pntr, ENVI_CHAR );
			 mds_pntr += ENVI_CHAR;
		    } while ( ++nr < num );
		    mds->clus[ncc++].n_sigc = num;
	       }
	       break;
	  default:
	       (void) snprintf( msg, 25, "unknown reticon type: %02d",
				((int) state->Clcon[nc].type) % 100 );
	       NADC_GOTO_ERROR( NADC_ERR_FATAL, msg );
	  }
     } while ( ++nc < state->num_clus );
     mds->n_clus = ncc;
/*
 * check if we read the whole DSR
 */
     if ( (nr_byte = mds_pntr - mds_char) != (size_t) state->length_dsr ) {
	  (void) snprintf( msg, 64, "MDS[%-u]: expected: %6u - read: %6zd",
			   state->indx, state->length_dsr, nr_byte );
	  NADC_GOTO_ERROR( NADC_ERR_PDS_SIZE, msg );
     }
/*
 * deallocate memory
 */
 done:
     free( glint_flags );
     free( mds_char );
     return;
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_MDS
.PURPOSE     read MDS of one state from a SCIAMACHY level 1b product
.INPUT/OUTPUT
  call as   nr_mds = SCIA_LV1_RD_MDS( fd, clus_mask, state, &mds );
     input:  
            FILE    *fd               : (open) stream pointer
	    ulong64 clus_mask         : mask for cluster selection
 in/output:  
	    struct state1_scia *state : structure with States of the product
    output:  
            struct mds1_scia **mds    : structure for level 1b MDS records

.RETURNS     number of level 1b MDS read (unsigned int), 
             error status passed by global variable ``nadc_stat''
.COMMENTS    This routine allocates memory for the following variables:
               sat_flags, red_grass, glint_flags, geoN/geoL/geoC, lvl0_header,
	       int_pmd, polV
	     and depending up on the cluster configuration:
	       clus[].sig, clus[].sigc
-------------------------*/
unsigned int SCIA_LV1_RD_MDS( FILE *fd, unsigned long long clus_mask,
			      struct state1_scia *state,
			      struct mds1_scia **mds_out )
     /*@globals  source;@*/
     /*@modifies source@*/
{
     register unsigned short nc, ncc;
     register unsigned int   nr_mds = 0;

     unsigned int num_mds = state->num_dsr;

     struct mds1_scia *mds = NULL;

     if ( num_mds == 0 || mds_out == NULL ) {
	  if ( mds_out != NULL ) *mds_out = NULL;
	  return 0u;
     }
/*
 * set variable source (= type of MDS)
 */
     source = (int) state->type_mds;
/*
 * allocate memory to store output records
 */
     mds = (struct mds1_scia *) malloc( num_mds * sizeof(struct mds1_scia));
     if ( mds == NULL ) NADC_GOTO_ERROR( NADC_ERR_ALLOC, "mds1_scia" );
     *mds_out = mds;
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) state->offset, SEEK_SET );
/*
 * initialize the MDS record and read MDS data of the selected state
 */
     sec_in_scan = 1;
     indx_limb = 0;
     do {
	  mds->type_mds = state->type_mds;
	  mds->state_id = (unsigned char) state->state_id;
	  mds->state_index = (unsigned char) state->indx;
	  mds->n_clus = 0;
	  mds->n_aux = state->num_aux / state->num_dsr;
	  if ( source != SCIA_MONITOR ) {
	       mds->n_pmd = PMD_NUMBER * state->num_pmd / state->num_dsr;
	       mds->n_pol = state->total_polar / state->num_dsr;
	  } else {
	       mds->n_pmd = 0u;
	       mds->n_pol = 0u;
	  }
	  mds->sat_flags = NULL;
	  mds->red_grass = NULL;
	  mds->lv0 = NULL;
	  mds->geoC = NULL;
	  mds->geoL = NULL;
	  mds->geoN = NULL;
	  mds->int_pmd = NULL;
	  mds->polV = NULL;
	  for ( nc = 0; nc < MAX_CLUSTER; nc++ ) {
	       mds->clus[nc].n_sig  = 0u;
	       mds->clus[nc].sig = NULL;
	       mds->clus[nc].n_sigc = 0u;
	       mds->clus[nc].sigc = NULL;
	  }
	  SCIA_LV1_RD_ONE_MDS( fd, clus_mask, state, mds );
	  if ( IS_ERR_STAT_FATAL ) 
	       NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "ONE_MDS" );
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_L1B_MDS( mds );
#endif
     } while ( mds++, ++nr_mds < num_mds );
/*
 * update state-record to reflect the actual cluster stored in the MDS record
 */
     for ( nc = ncc = 0; nc < state->num_clus; nc++ ) {
	  if ( Get_Bit_LL( clus_mask, (unsigned char) nc ) == 1ULL ) {
	       if ( ncc < nc )
		    (void) memmove( &state->Clcon[ncc], &state->Clcon[nc],
				    sizeof( struct Clcon_scia ) );
	       ncc++;
	  }
     }
     state->num_clus = ncc;
/*
 * set return values
 */
     return nr_mds;
 done:
     *mds_out = NULL;
     if ( (mds -= nr_mds) != NULL ) SCIA_LV1_FREE_MDS( source, nr_mds, mds );
     return 0u;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1C_RD_MDS
.PURPOSE     read SCIAMACHY level 1c MDS
.INPUT/OUTPUT
  call as   nr_mds = SCIA_LV1C_RD_MDS( fd, clus_mask, state, &mds );
     input:  
            FILE    *fd               : (open) stream pointer
	    ulong64 clus_mask         : mask for cluster selection
 in/output:  
	    struct state1_scia *state : structure with States of the product
    output:  
            struct mds1c_scia  **mds  : structure for level 1c MDS

.RETURNS     number of level 1c MDS read (unsigned int), 
             error status passed by global variable ``nadc_stat''
.COMMENTS    This routine allocates memory for the following variables:
              pixel_ids, pixel_wv, pixel_wv_err, pixel_val, pixel_err, 
              geoN/geoL/geoC
-------------------------*/
unsigned int SCIA_LV1C_RD_MDS( FILE *fd, unsigned long long clus_mask,
			       struct state1_scia *state,
			       struct mds1c_scia **mds_out )
     /*@globals  source;@*/
     /*@modifies source@*/
{
     register unsigned short nc, ncc, ni, nobs;
     register unsigned int   nr_mds = 0u;

     register unsigned short bcp_in_limb, bcp_deep_space;

     char    *mds_char, *mds_pntr;
     size_t  dsr_length_left, nr_byte;

     unsigned short ubuff;
     unsigned short num_clus_file, num_clus_out;

     struct mds1c_scia *mds = NULL;

     static unsigned short bcp_in_nadir = 0;

     const unsigned char uchar_one = 1;
     const size_t        DSR_Read  = sizeof(struct mjd_envi) + ENVI_UINT;
/*
 * set variable source (= type of MDS)
 */
     source = (int) state->type_mds;
/*
 * count number of cluster to be read
 */
     num_clus_out = 0;
     num_clus_file = state->num_clus;
     for ( nc = 0; nc < num_clus_file; nc++ ) {
	  if ( Get_Bit_LL( clus_mask, 
			   state->Clcon[nc].id - uchar_one  ) == 1ULL )
	       num_clus_out++;
     }
/*
 * check dimension of the output array mds_out
 */
     if ( num_clus_out == 0u || mds_out == NULL ) {
          if ( mds_out != NULL ) *mds_out = NULL;
          return 0u;
     }
/*
 * allocate memory to store output records
 */
     mds = (struct mds1c_scia *) 
	  malloc( num_clus_out * sizeof(struct mds1c_scia));
     if ( mds == NULL ) NADC_GOTO_ERROR( NADC_ERR_ALLOC, "mds1_scia" );
     *mds_out = mds;
/*
 * initialize MDS-structure
 */
     nc = 0;
     do {
	  mds[nc].num_obs = 0;
	  mds[nc].num_pixels = 0;
	  mds[nc].pixel_ids = NULL;
	  mds[nc].pixel_wv = NULL;
	  mds[nc].pixel_wv_err = NULL;
	  mds[nc].pixel_val = NULL;
	  mds[nc].pixel_err = NULL;
	  mds[nc].geoN = NULL;
	  mds[nc].geoL = NULL;
	  mds[nc].geoC = NULL;
     } while ( ++nc < num_clus_out );
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) state->offset, SEEK_SET );
/*
 * read data set records
 */
     nc = 0;
     do {
	  mds->type_mds    = state->type_mds;
	  mds->state_index = (unsigned char) state->indx;
	  mds->dur_scan    = state->dur_scan;
/* 1 */
	  if ( fread( &mds->mjd, sizeof( struct mjd_envi ), 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "" );
/* 2 */
	  if ( fread( &mds->dsr_length, ENVI_UINT, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "" );
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  mds->dsr_length = byte_swap_u32( mds->dsr_length );
#endif
/*
 * now we know the size of this dsr, so read it in memory!
 */
	  dsr_length_left = mds->dsr_length - DSR_Read;
	  mds_char = (char *) malloc( dsr_length_left );
	  if ( mds_char == NULL )
	       NADC_GOTO_ERROR( NADC_ERR_ALLOC, "mds_char" );
	  if ( fread( mds_char, dsr_length_left, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "" );
/* 3 */
	  (void) memcpy( &mds->quality_flag, mds_char, ENVI_CHAR );
	  mds_pntr = mds_char + ENVI_CHAR;
/* 4 */
	  (void) memcpy( &mds->orbit_phase, mds_pntr, ENVI_FLOAT );
	  mds_pntr += ENVI_FLOAT;
/* 5 */
	  (void) memcpy( &ubuff, mds_pntr, ENVI_USHRT );
	  mds_pntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  ubuff = byte_swap_u16( ubuff );
#endif
	  mds->category = (unsigned char) ubuff;
/* 6 */
	  (void) memcpy( &ubuff, mds_pntr, ENVI_USHRT );
	  mds_pntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  ubuff = byte_swap_u16( ubuff );
#endif
	  mds->state_id = (unsigned char) ubuff;
/* 7 */
	  (void) memcpy( &ubuff, mds_pntr, ENVI_USHRT );
	  mds_pntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  ubuff = byte_swap_u16( ubuff );
#endif
	  mds->clus_id = (unsigned char) ubuff;
	  for ( ncc = 0; ncc < state->num_clus; ncc++ )
	       if ( state->Clcon[ncc].id == mds->clus_id ) break;
	  
	  mds->chan_id = state->Clcon[ncc].channel;
	  mds->coaddf  = (unsigned char) state->Clcon[ncc].coaddf;
	  mds->pet     = state->Clcon[ncc].pet;

	  if ( Get_Bit_LL(clus_mask,(unsigned char)(mds->clus_id-1)) != 0ULL ){
/* 8 */
	       (void) memcpy( &mds->num_obs, mds_pntr, ENVI_USHRT );
	       mds_pntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       mds->num_obs = byte_swap_u16( mds->num_obs );
#endif
/* 9 */
	       (void) memcpy( &mds->num_pixels, mds_pntr, ENVI_USHRT );
	       mds_pntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       mds->num_pixels = byte_swap_u16( mds->num_pixels );
#endif
/* 10 */
	       (void) memcpy( &mds->rad_units_flag, mds_pntr, ENVI_CHAR );
	       mds_pntr += ENVI_CHAR;
/* 11 */
	       nr_byte = mds->num_pixels * ENVI_USHRT;
	       mds->pixel_ids = (unsigned short *) malloc( nr_byte );
	       if ( mds->pixel_ids == NULL ) {
		    free( mds_char );
		    NADC_GOTO_ERROR( NADC_ERR_ALLOC, "pixel_ids" );
	       }
	       (void) memcpy( mds->pixel_ids, mds_pntr, nr_byte );
	       mds_pntr += nr_byte;
/* 12 */
	       nr_byte = mds->num_pixels * ENVI_FLOAT;
	       mds->pixel_wv = (float *) malloc( nr_byte );
	       if ( mds->pixel_wv == NULL ) {
		    free( mds_char );
		    NADC_GOTO_ERROR(NADC_ERR_ALLOC, "pixel_wv");
	       }
	       (void) memcpy( mds->pixel_wv, mds_pntr, nr_byte );
	       mds_pntr += nr_byte;
/* 13 */
	       mds->pixel_wv_err = (float *) malloc( nr_byte );
	       if ( mds->pixel_wv_err == NULL ) {
		    free( mds_char );
		    NADC_GOTO_ERROR( NADC_ERR_ALLOC, "pixel_wv_err" );
	       }
	       (void) memcpy( mds->pixel_wv_err, mds_pntr, nr_byte );
	       mds_pntr += nr_byte;
/* 14 */
	       nr_byte = mds->num_obs * mds->num_pixels * ENVI_FLOAT;
	       mds->pixel_val = (float *) malloc( nr_byte );
	       if ( mds->pixel_val == NULL ) {
		    free( mds_char );
		    NADC_GOTO_ERROR( NADC_ERR_ALLOC, "pixel_val" );
	       }
	       (void) memcpy( mds->pixel_val, mds_pntr, nr_byte );
	       mds_pntr += nr_byte;
/* 15 */
	       mds->pixel_err = (float *) malloc( nr_byte );
	       if ( mds->pixel_err == NULL ) {
		    free( mds_char );
		    NADC_GOTO_ERROR( NADC_ERR_ALLOC, "pixel_err" );
	       }
	       (void) memcpy( mds->pixel_err, mds_pntr, nr_byte );
	       mds_pntr += nr_byte;
/* 16 */
	       switch ( source ) {
	       case SCIA_NADIR:
		    mds->geoN = (struct geoN_scia *) 
			 malloc( mds->num_obs * sizeof( struct geoN_scia ));
		    if ( mds->geoN == NULL ) {
			 free( mds_char );
			 NADC_GOTO_ERROR( NADC_ERR_ALLOC, "geoN" );
		    }
		    mds_pntr += SCIA_LV1_RD_GeoN( mds_pntr, mds->num_obs, 
						  mds->geoN );
/*
 * set Rainbow/Sun glint flags and pixel type: 0 (= backscan) else 1
 */
		    for ( nobs = 0; nobs < mds->num_obs; nobs++ ) {
			 mds->geoN[nobs].glint_flag = 0;

			 bcp_in_nadir += state->Clcon[ncc].intg_time;
			 if ( bcp_in_nadir > 64 ) {
			      mds->geoN[nobs].pixel_type = BACK_SCAN;
			      if ( bcp_in_nadir == 80 ) bcp_in_nadir = 0;
			 } else
			      mds->geoN[nobs].pixel_type = FORWARD_SCAN;
		    }
		    break;
	       case SCIA_LIMB:
		    mds->geoL = (struct geoL_scia *) 
			 malloc( mds->num_obs * sizeof( struct geoL_scia ));
		    if ( mds->geoL == NULL ) {
			 free( mds_char );
			 NADC_GOTO_ERROR( NADC_ERR_ALLOC, "geoL" );
		    }
		    mds_pntr += SCIA_LV1_RD_GeoL( mds_pntr, mds->num_obs, 
						  mds->geoL );
/*
 * set Rainbow/Sun glint flags and pixel type
 */
		    bcp_in_limb = 0;
		    bcp_deep_space =
			 mds->num_obs * state->Clcon[ncc].intg_time - 24;
		    for ( nobs = 0; nobs < mds->num_obs; nobs++ ) {
			 mds->geoL[nobs].glint_flag = 0;
			 mds->geoL[nobs].pixel_type = ALONG_TANG_HGHT;

			 if ( nobs == mds->num_obs-1
			      || bcp_in_limb > bcp_deep_space )
			      mds->geoL[nobs].pixel_type = DEEP_SPACE;
			 else if ( (bcp_in_limb % 24) == 0 )
			      mds->geoL[nobs].pixel_type = NEW_TANG_HGHT;

			 bcp_in_limb += state->Clcon[ncc].intg_time;
		    }
		    break;
	       case SCIA_OCCULT:
		    mds->geoL = (struct geoL_scia *) 
			 malloc( mds->num_obs * sizeof( struct geoL_scia ));
		    if ( mds->geoL == NULL ) {
			 free( mds_char );
			 NADC_GOTO_ERROR( NADC_ERR_ALLOC, "geoL" );
		    }
		    mds_pntr += SCIA_LV1_RD_GeoL( mds_pntr, mds->num_obs, 
						  mds->geoL );
/*
 * set Rainbow/Sun glint flags and pixel type
 */
		    for ( nobs = 0; nobs < mds->num_obs; nobs++ ) {
			 mds->geoL[nobs].glint_flag = 0;
			 mds->geoL[nobs].pixel_type = ALONG_TANG_HGHT;
		    }
		    break;
	       case SCIA_MONITOR:
		    mds->geoC = (struct geoC_scia *) 
			 malloc( mds->num_obs * sizeof( struct geoC_scia ));
		    if ( mds->geoC == NULL ) {
			 free( mds_char );
			 NADC_GOTO_ERROR( NADC_ERR_ALLOC, "geoC" );
		    }
		    mds_pntr += SCIA_LV1_RD_GeoC( mds_pntr, mds->num_obs, 
						  mds->geoC );
		    break;
	       }
/*
 * check if we read the whole DSR
 */
	       if ( (size_t)(mds_pntr - mds_char) != dsr_length_left ) {
		    const char *dsd_names[] = { "UNKNOWN", "NADIR", "LIMB",
						"OCCULTATION", "MONITORING" };
		    free( mds_char );
		    NADC_GOTO_ERROR( NADC_ERR_PDS_SIZE, 
				     dsd_names[source] );
	       }
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       Sun2Intel_L1C_MDS( mds );
#endif
	       if ( ++nr_mds == num_clus_out ) {
		    free( mds_char );
		    break;
	       }
/*
 * pixel_ids has a value in the range [0..8191]
 */
	       for ( ni = 0; ni < mds->num_pixels; ni++ )
		    mds->pixel_ids[ni] += (mds->chan_id - 1) * CHANNEL_SIZE;
	       mds++;
	  }
	  free( mds_char );
     } while ( ++nc < num_clus_file );
/*
 * update state-record to reflect the actual clusters to be read
 * note that the dimension of 'mds_out' equals 'num_clus_out'
 */
     num_clus_out = 0;
     for ( nc = 0; nc < num_clus_file; nc++ ) {
	  if ( Get_Bit_LL( clus_mask, 
			   state->Clcon[nc].id - uchar_one ) == 1ULL ) {
	       if ( num_clus_out < nc )
		    (void) memmove( &state->Clcon[num_clus_out], 
				    &state->Clcon[nc],
				    sizeof( struct Clcon_scia ) );
	       num_clus_out++;
	  }
     }
     state->num_clus = num_clus_out;
/*
 * set return values
 */
     return nr_mds;
 done:
     *mds_out = NULL;
     if ( (mds -= nr_mds) != NULL ) SCIA_LV1C_FREE_MDS( source, nr_mds, mds );
     return 0u;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1C_RD_MDS_PMD
.PURPOSE     read SCIAMACHY level 1c PMD MDS
.INPUT/OUTPUT
  call as   nr_mds = SCIA_LV1C_RD_MDS_PMD( fd, state, &pmd );
     input:  
            FILE   *fd                : (open) stream pointer
	    struct state1_scia *state : structure with States of the product
    output:  
            struct mds1c_pmd **pmd    : structure for level 1c PMD MDS

.RETURNS     number of level 1c PMD MDS read (unsigned int), 
             error status passed by global variable ``nadc_stat''
.COMMENTS    This routine allocates memory for the following variables:
              int_pmd, geoN/geoL/geoC
-------------------------*/
unsigned int SCIA_LV1C_RD_MDS_PMD( FILE *fd, const struct state1_scia *state,
				   struct mds1c_pmd **pmd_out )
     /*@globals  source;@*/
     /*@modifies source@*/
{
     char         *mds_char, *mds_pntr;
     size_t       dsr_length_left, nr_byte;

     unsigned short ubuff;

     struct mds1c_pmd *pmd;

     const size_t DSR_Read = sizeof(struct mjd_envi) + ENVI_UINT;
/*
 * set variable source (= type of MDS)
 */
     source = (int) state->type_mds;
/*
 * check type of MDS
 */
     if ( source == SCIA_MONITOR || state->offs_pmd == 0L ) {
	  *pmd_out = NULL;
	  return 0u;
     }
/*
 * allocate memory to store output records
 */
     *pmd_out = (struct mds1c_pmd *) malloc( sizeof(struct mds1c_pmd));
     if ( (pmd = *pmd_out) == NULL ) {
          NADC_GOTO_ERROR( NADC_ERR_ALLOC, "mds1c_pmd" );
     }
/*
 * initialize MDS-structure
 */
     pmd->num_geo = 0u;
     pmd->num_pmd = 0u;
     pmd->int_pmd = NULL;
     pmd->geoN = NULL;
     pmd->geoL = NULL;
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) state->offs_pmd, SEEK_SET );
/*
 * read data set records
 */
     pmd->type_mds    = state->type_mds;
     pmd->state_index = (unsigned char) state->indx;
/* 1 */
     if ( fread( &pmd->mjd, sizeof( struct mjd_envi ), 1 , fd ) != 1 )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "" );
/* 2 */
     if ( fread( &pmd->dsr_length, ENVI_UINT, 1 , fd ) != 1 )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "" );
#ifdef _SWAP_TO_LITTLE_ENDIAN
     pmd->dsr_length = byte_swap_u32( pmd->dsr_length );
#endif
/*
 * now we know the size of this dsr, so read it in memory!
 */
     dsr_length_left = pmd->dsr_length - DSR_Read;
     mds_char = (char *) malloc( dsr_length_left );
     if ( mds_char == NULL ) 
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "mds_char" );
     if ( fread( mds_char, dsr_length_left, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "" );
/* 3 */
     (void) memcpy( &pmd->quality_flag, mds_char, ENVI_CHAR );
     mds_pntr = mds_char + ENVI_CHAR;
/* 4 */
     (void) memcpy( &pmd->orbit_phase, mds_pntr, ENVI_FLOAT );
     mds_pntr += ENVI_FLOAT;
/* 5 */
     (void) memcpy( &ubuff, mds_pntr, ENVI_USHRT );
     mds_pntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
     ubuff = byte_swap_u16( ubuff );
#endif
     pmd->category = (unsigned char) ubuff;
/* 6 */
     (void) memcpy( &ubuff, mds_pntr, ENVI_USHRT );
     mds_pntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
     ubuff = byte_swap_u16( ubuff );
#endif
     pmd->state_id = (unsigned char) ubuff;
/* 7 */
     (void) memcpy( &pmd->dur_scan, mds_pntr, ENVI_USHRT );
     mds_pntr += ENVI_USHRT;
/* 8 */
     (void) memcpy( &pmd->num_pmd, mds_pntr, ENVI_USHRT );
     mds_pntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
     pmd->num_pmd = byte_swap_u16( pmd->num_pmd );
#endif
/* 9 */
     (void) memcpy( &pmd->num_geo, mds_pntr, ENVI_USHRT );
     mds_pntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
     pmd->num_geo = byte_swap_u16( pmd->num_geo );
#endif
/* 10 */
     nr_byte = pmd->num_pmd * ENVI_FLOAT;
     pmd->int_pmd = (float *) malloc( nr_byte );
     if ( pmd->int_pmd == NULL ) {
	  free( mds_char );
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "int_pmd" );
     }
     (void) memcpy( pmd->int_pmd, mds_pntr, nr_byte );
     mds_pntr += nr_byte;
/* 11 */	  
     switch ( source ) {
     case SCIA_NADIR:
	  pmd->geoN = (struct geoN_scia *) 
	       malloc( pmd->num_geo * sizeof( struct geoN_scia ) );
	  if ( pmd->geoN == NULL ) {
	       free( mds_char );
	       NADC_GOTO_ERROR( NADC_ERR_ALLOC, "geoN" );
	  }
	  mds_pntr += 
	       SCIA_LV1_RD_GeoN(mds_pntr, pmd->num_geo, pmd->geoN);
	  break;
     case SCIA_LIMB:
     case SCIA_OCCULT:
	  pmd->geoL = (struct geoL_scia *) 
	       malloc( pmd->num_geo * sizeof( struct geoL_scia ) );
	  if ( pmd->geoL == NULL ) {
	       free( mds_char );
	       NADC_GOTO_ERROR( NADC_ERR_ALLOC, "geoL" );
	  }
	  mds_pntr += 
	       SCIA_LV1_RD_GeoL(mds_pntr, pmd->num_geo, pmd->geoL);
	  break;
     }
/*
 * check if we read the whole DSR
 */
     if ( (size_t)(mds_pntr - mds_char) != dsr_length_left ) {
	  const char *dsd_names[] = { "UNKNOWN", "NADIR_PMD", "LIMB_PMD",
				      "OCCULTATION_PMD" };

	  free( mds_char );
	  NADC_GOTO_ERROR( NADC_ERR_PDS_SIZE, dsd_names[source] );
     }
     free( mds_char );
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_L1C_MDS_PMD( pmd );
#endif
/*
 * set return values
 */
     pmd = NULL;
     return 1;
 done:
     if ( pmd != NULL ) SCIA_LV1C_FREE_MDS_PMD( source, pmd );
     return 0;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1C_RD_MDS_POLV
.PURPOSE     read SCIAMACHY level 1c POLV MDS
.INPUT/OUTPUT
  call as   nr_mds = SCIA_LV1C_RD_MDS_POLV( fd, state, &polV );
     input:  
            FILE   *fd                : (open) stream pointer
	    struct state1_scia *state : structure with States of the product
    output:  
            struct mds1c_polV **polV  : structure for level 1c POLV MDS

.RETURNS     number of level 1c POLV MDS read (unsigned int), 
             error status passed by global variable ``nadc_stat''
.COMMENTS    This routine allocates memory for the following variables:
              polV, geoN/geoL/geoC
-------------------------*/
unsigned int SCIA_LV1C_RD_MDS_POLV( FILE *fd, const struct state1_scia *state,
				    struct mds1c_polV **polV_out )
     /*@globals  source;@*/
     /*@modifies source@*/
{
     char         *mds_char, *mds_pntr;
     size_t       dsr_length_left, nr_byte;

     unsigned short ubuff;

     struct mds1c_polV *polV;

     const size_t DSR_Read = sizeof(struct mjd_envi) + ENVI_UINT;
/*
 * set variable source (= type of MDS)
 */
     source = (int) state->type_mds;
/*
 * check type of MDS
 */
     if ( (source != SCIA_NADIR && source != SCIA_LIMB 
	   && source != SCIA_OCCULT) || state->offs_polV == 0L ) {
	  *polV_out = NULL;
	  return 0u;
     }
/*
 * allocate memory to store output records
 */
     *polV_out = (struct mds1c_polV *) malloc( sizeof(struct mds1c_polV) );
     if ( (polV = *polV_out) == NULL ) {
          NADC_GOTO_ERROR( NADC_ERR_ALLOC, "mds1c_polV" );
     }
/*
 * initialize MDS-structure
 */
     polV->total_polV = 0u;
     polV->num_geo = 0u;
     polV->polV = NULL;
     polV->geoN = NULL;
     polV->geoL = NULL;
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) state->offs_polV, SEEK_SET );
/*
 * read data set records
 */
     polV->type_mds   = state->type_mds;
     polV->state_index = (unsigned char) state->indx;
/* 1 */
     if ( fread( &polV->mjd, sizeof( struct mjd_envi ), 1 , fd ) != 1 )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "" );
/* 2 */
     if ( fread( &polV->dsr_length, ENVI_UINT, 1 , fd ) != 1 )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "" );
#ifdef _SWAP_TO_LITTLE_ENDIAN
     polV->dsr_length = byte_swap_u32( polV->dsr_length );
#endif
/*
 * now we know the size of this dsr, so read it in memory!
 */
     dsr_length_left = polV->dsr_length - DSR_Read;
     mds_char = (char *) malloc( dsr_length_left );
     if ( mds_char == NULL ) 
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "mds_char" );
     if ( fread( mds_char, dsr_length_left, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "" );
/* 3 */
     (void) memcpy( &polV->quality_flag, mds_char, ENVI_CHAR );
     mds_pntr = mds_char + ENVI_CHAR;
/* 4 */
     (void) memcpy( &polV->orbit_phase, mds_pntr, ENVI_FLOAT );
     mds_pntr += ENVI_FLOAT;
/* 5 */
     (void) memcpy( &ubuff, mds_pntr, ENVI_USHRT );
     mds_pntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
     ubuff = byte_swap_u16( ubuff );
#endif
     polV->category = (unsigned char) ubuff;
/* 6 */
     (void) memcpy( &ubuff, mds_pntr, ENVI_USHRT );
     mds_pntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
     ubuff = byte_swap_u16( ubuff );
#endif
     polV->state_id = (unsigned char) ubuff;
/* 7 */
     (void) memcpy( &polV->dur_scan, mds_pntr, ENVI_USHRT );
     mds_pntr += ENVI_USHRT;
/* 8 */
     (void) memcpy( &polV->num_geo, mds_pntr, ENVI_USHRT );
     mds_pntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
     polV->num_geo = byte_swap_u16( polV->num_geo );
#endif
/* 9 */
     (void) memcpy( &polV->total_polV, mds_pntr, ENVI_USHRT );
     mds_pntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
     polV->total_polV = byte_swap_u16( polV->total_polV );
#endif
/* 10 */
     (void) memcpy( &polV->num_diff_intg, mds_pntr, ENVI_USHRT );
     mds_pntr += ENVI_USHRT;
/* 11 */
     nr_byte = MAX_CLUSTER * ENVI_USHRT;
     (void) memcpy( polV->intg_times, mds_pntr, nr_byte );
     mds_pntr += nr_byte;
/* 12 */
     (void) memcpy( polV->num_polar, mds_pntr, nr_byte );
     mds_pntr += nr_byte;
/* 13 */
     nr_byte = polV->total_polV * sizeof( struct polV_scia );
     polV->polV = (struct polV_scia *) malloc( nr_byte );
     if ( polV->polV == NULL ) {
	  free( mds_char );
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "polV" );
     }
     mds_pntr += 
	  SCIA_LV1_RD_PolV( mds_pntr, polV->total_polV, polV->polV );
/* 14 */	  
     switch ( source ) {
     case SCIA_NADIR:
	  polV->geoN = (struct geoN_scia *) 
	       malloc( polV->num_geo * sizeof( struct geoN_scia ) );
	  if ( polV->geoN == NULL ) {
	       free( mds_char );
	       NADC_GOTO_ERROR( NADC_ERR_ALLOC, "geoN" );
	  }
	  mds_pntr += 
	       SCIA_LV1_RD_GeoN(mds_pntr, polV->num_geo, polV->geoN);
	  break;
     case SCIA_LIMB:
     case SCIA_OCCULT:
	  polV->geoL = (struct geoL_scia *) 
	       malloc( polV->num_geo * sizeof( struct geoL_scia ) );
	  if ( polV->geoL == NULL ) {
	       free( mds_char );
	       NADC_GOTO_ERROR( NADC_ERR_ALLOC, "geoL" );
	  }
	  mds_pntr += 
	       SCIA_LV1_RD_GeoL(mds_pntr, polV->num_geo, polV->geoL);
	  break;
     }
/*
 * check if we read the whole DSR
 */
     if ( (size_t)(mds_pntr - mds_char) != dsr_length_left ) {
	  const char *dsd_names[] = { "UNKNOWN", "NADIR_FRAC_POL", 
				      "LIMB_FRAC_POL", "OCCULTATION_FRAC_POL" };

	  free( mds_char );
	  NADC_GOTO_ERROR( NADC_ERR_PDS_SIZE, dsd_names[source] );
     }
     free( mds_char );
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_L1C_MDS_POLV( polV );
#endif
/*
 * set return values
 */
     polV = NULL;
     return 1;
 done:
     if ( polV != NULL ) SCIA_LV1C_FREE_MDS_POLV( source, polV );
     return 0;
}
