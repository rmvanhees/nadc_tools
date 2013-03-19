#include <swap_bytes.h>

static inline
void Sun2Intel_L1C_MDS( struct mds1c_scia *mds )
{
     register unsigned int nr;

     unsigned int nrval = (unsigned int) mds->num_obs * mds->num_pixels;

     mds->mjd.days = byte_swap_32( mds->mjd.days );
     mds->mjd.secnd = byte_swap_u32( mds->mjd.secnd );
     mds->mjd.musec = byte_swap_u32( mds->mjd.musec );
     IEEE_Swap__FLT( &mds->orbit_phase );
/*
 * convert pixel values
 */
     for ( nr = 0; nr < mds->num_pixels; nr++ ) {
	  mds->pixel_ids[nr] = byte_swap_u16( mds->pixel_ids[nr] );
	  IEEE_Swap__FLT( &mds->pixel_wv[nr] );
	  IEEE_Swap__FLT( &mds->pixel_wv_err[nr] );
     }
     for ( nr = 0; nr < nrval; nr++ ) {
	  IEEE_Swap__FLT( &mds->pixel_val[nr] );
	  IEEE_Swap__FLT( &mds->pixel_err[nr]);
     }
}

static inline
void Sun2Intel_L1C_MDS_PMD( struct mds1c_pmd *pmd )
{
     register unsigned int np;

     pmd->mjd.days = byte_swap_32( pmd->mjd.days );
     pmd->mjd.secnd = byte_swap_u32( pmd->mjd.secnd );
     pmd->mjd.musec = byte_swap_u32( pmd->mjd.musec );
     pmd->dur_scan = byte_swap_u16( pmd->dur_scan );
     IEEE_Swap__FLT( &pmd->orbit_phase );
/*
 * convert pmd values
 */
     for ( np = 0; np < pmd->num_pmd; np++ ) {
	  IEEE_Swap__FLT( &pmd->int_pmd[np] );
     }
}

static inline
void Sun2Intel_L1C_MDS_POLV( struct mds1c_polV *polV )
{
     register unsigned int np;

     polV->mjd.days = byte_swap_32( polV->mjd.days );
     polV->mjd.secnd = byte_swap_u32( polV->mjd.secnd );
     polV->mjd.musec = byte_swap_u32( polV->mjd.musec );
     polV->dur_scan = byte_swap_u16( polV->dur_scan );
     polV->num_diff_intg = byte_swap_u16( polV->num_diff_intg );
     IEEE_Swap__FLT( &polV->orbit_phase );
/*
 * convert fractional Polarisation data sets
 */
     for ( np = 0; np < polV->total_polV; np++ )
	  Sun2Intel_polV( &polV->polV[np] );

     for ( np = 0; np < MAX_CLUSTER; np++ ) {
	  polV->intg_times[np] = byte_swap_u16( polV->intg_times[np] );
	  polV->num_polar[np] = byte_swap_u16( polV->num_polar[np] );
     }
}
