#include <swap_bytes.h>

static
void Sun2Intel_GeoN( struct geoN_scia *geoN )
{
     register unsigned short ni;

     IEEE_Swap__FLT( &geoN->pos_esm );
     IEEE_Swap__FLT( &geoN->sat_h );
     IEEE_Swap__FLT( &geoN->earth_rad );
     for ( ni = 0; ni < 3; ni++ ) {
	  IEEE_Swap__FLT( &geoN->sun_zen_ang[ni] );
	  IEEE_Swap__FLT( &geoN->sun_azi_ang[ni] );
	  IEEE_Swap__FLT( &geoN->los_zen_ang[ni] );
	  IEEE_Swap__FLT( &geoN->los_azi_ang[ni] );
     }
     geoN->sub_sat_point.lon = byte_swap_32( geoN->sub_sat_point.lon );
     geoN->sub_sat_point.lat = byte_swap_32( geoN->sub_sat_point.lat );
     for ( ni = 0; ni < 4; ni++ ) {
	  geoN->corner[ni].lon = byte_swap_32( geoN->corner[ni].lon );
	  geoN->corner[ni].lat = byte_swap_32( geoN->corner[ni].lat );
     }
     geoN->center.lon = byte_swap_32( geoN->center.lon );
     geoN->center.lat = byte_swap_32( geoN->center.lat );
}

static
void Sun2Intel_GeoL( struct geoL_scia *geoL )
{
     register unsigned short ni;

     IEEE_Swap__FLT( &geoL->pos_esm );
     IEEE_Swap__FLT( &geoL->pos_asm );
     IEEE_Swap__FLT( &geoL->sat_h );
     IEEE_Swap__FLT( &geoL->earth_rad );
     IEEE_Swap__FLT( &geoL->dopp_shift );
     geoL->sub_sat_point.lon = byte_swap_32( geoL->sub_sat_point.lon );
     geoL->sub_sat_point.lat = byte_swap_32( geoL->sub_sat_point.lat );
     for ( ni = 0; ni < 3; ni++ ) {
	  IEEE_Swap__FLT( &geoL->sun_zen_ang[ni] );
	  IEEE_Swap__FLT( &geoL->sun_azi_ang[ni] );
	  IEEE_Swap__FLT( &geoL->los_zen_ang[ni] );
	  IEEE_Swap__FLT( &geoL->los_azi_ang[ni] );
	  IEEE_Swap__FLT( &geoL->tan_h[ni] );
	  geoL->tang_ground_point[ni].lon = 
	       byte_swap_32(geoL->tang_ground_point[ni].lon);
	  geoL->tang_ground_point[ni].lat = 
	       byte_swap_32(geoL->tang_ground_point[ni].lat);
     }
}

static
void Sun2Intel_GeoC( struct geoC_scia *geoC )
{
     IEEE_Swap__FLT( &geoC->pos_esm );
     IEEE_Swap__FLT( &geoC->pos_asm );
     IEEE_Swap__FLT( &geoC->sun_zen_ang );
     geoC->sub_sat_point.lon = byte_swap_32( geoC->sub_sat_point.lon );
     geoC->sub_sat_point.lat = byte_swap_32( geoC->sub_sat_point.lat );
}

static inline
void Sun2Intel_LV0Hdr( struct lv0_hdr *lv0 )
{
     register unsigned short ni;

     lv0->packet_hdr.api.two_byte = 
	  byte_swap_u16( lv0->packet_hdr.api.two_byte );
     lv0->packet_hdr.seq_cntrl = byte_swap_u16( lv0->packet_hdr.seq_cntrl );
     lv0->packet_hdr.length = byte_swap_u16( lv0->packet_hdr.length );

     lv0->data_hdr.length = byte_swap_u16( lv0->data_hdr.length );
     lv0->data_hdr.on_board_time = byte_swap_u32( lv0->data_hdr.on_board_time);

     lv0->bcps = byte_swap_u16( lv0->bcps );
     lv0->pmtc_hdr.scanner_mode = byte_swap_u16( lv0->pmtc_hdr.scanner_mode );
     lv0->pmtc_hdr.az_param.four_byte = 
	  byte_swap_u32( lv0->pmtc_hdr.az_param.four_byte );
     lv0->pmtc_hdr.elv_param.four_byte = 
	  byte_swap_u32( lv0->pmtc_hdr.elv_param.four_byte );
     for ( ni = 0; ni < 8; ni++ )
	  lv0->orbit_vector[ni] = byte_swap_32( lv0->orbit_vector[ni] );
     lv0->num_chan = byte_swap_u16( lv0->num_chan );
}

static
void Sun2Intel_polV ( struct polV_scia *polV )
{
     register unsigned short ni;

     IEEE_Swap__FLT( &polV->gdf.beta );
     IEEE_Swap__FLT( &polV->gdf.p_bar );
     IEEE_Swap__FLT( &polV->gdf.w0 );
     for ( ni = 0; ni < 12; ni++ ) {
	  IEEE_Swap__FLT( &polV->Q[ni] );
	  IEEE_Swap__FLT( &polV->error_Q[ni] );
	  IEEE_Swap__FLT( &polV->U[ni] );
	  IEEE_Swap__FLT( &polV->error_U[ni] );
     }
     for ( ni = 0; ni < 13; ni++ )
	  IEEE_Swap__FLT( &polV->rep_wv[ni] );
}

static inline
void Sun2Intel_L1B_MDS( struct mds1_scia *mds )
     /*@globals  source@*/
{
     register int na, np;

     mds->mjd.days = byte_swap_32( mds->mjd.days );
     mds->mjd.secnd = byte_swap_u32( mds->mjd.secnd );
     mds->mjd.musec = byte_swap_u32( mds->mjd.musec );
/*
 * convert level 0 data packets
 */
     for ( na = 0; na < mds->n_aux; na++ )
	  Sun2Intel_LV0Hdr ( &mds->lv0[na] );
/*
 * convert PMD and fractional Polarisation data sets
 */
     if ( source != SCIA_MONITOR ) {
	  for ( np = 0; np < mds->n_pmd; np++ )
	       IEEE_Swap__FLT( &mds->int_pmd[np] );

	  for ( np = 0; np < mds->n_pol; np++ )
	       Sun2Intel_polV( &mds->polV[np] );
     }
}
