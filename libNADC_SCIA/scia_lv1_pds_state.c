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

.IDENTifer   SCIA_LV1_PDS_STATE
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 data
.LANGUAGE    ANSI C
.PURPOSE     read/write States of the Product
.COMMENTS    contains SCIA_LV1_RD_STATE and SCIA_LV1_WR_STATE

             For level 1c data the returned state records reflect the actual
             number of clusters per state based on the CAL_OPTIONS DSD
.ENVIRONment None
.VERSION      4.2   20-Sep-2006 bugfixed calculation of offset to L1c MDS, RvH
                                (occurring when not all clusters are read)
              4.1   01-Jun-2006 bugfixed calculation of offset to L1c MDS, RvH
              4.0   11-Oct-2005 minor bugfixes, add usage of SCIA_LV1_ADD_DSD
              3.1   27-Jun-2005 check L1C files for presence of MDS's, RvH
              3.0   18-Apr-2005 added routine to write STATE-struct to file,RvH
              2.7   18-Sep-2002 wrong offsets when called more than once, RvH
              2.6   18-Sep-2002 use external function to test 1b/1c, RvH
              2.5   02-Aug-2002	new approach to get offsets, RvH
              2.4   18-Apr-2002	calculate dsr_size, instead of reading 
                                it from file (much faster), RvH
              2.3   05-Apr-2002	calculate offset to all level 1c DSD's, RvH 
              2.2   28-Feb-2002	added indx & offset to struct, RvH 
              2.1   16-Jan-2002	use of global Use_Extern_Alloc, RvH 
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              1.1   04-Oct-2001	changed input/output, RvH 
              1.0   10-Nov-1999 created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#ifdef _SWAP_TO_LITTLE_ENDIAN
#include <swap_bytes.h>

static
void Sun2Intel_STATE( struct state1_scia *state )
{
     register int ni;

     state->mjd.days = byte_swap_32( state->mjd.days );
     state->mjd.secnd = byte_swap_u32( state->mjd.secnd );
     state->mjd.musec = byte_swap_u32( state->mjd.musec );
     state->category = byte_swap_u16( state->category );
     state->state_id = byte_swap_u16( state->state_id );
     state->dur_scan = byte_swap_u16( state->dur_scan );
     state->longest_intg_time = byte_swap_u16( state->longest_intg_time );
     state->num_aux = byte_swap_u16( state->num_aux );
     state->num_pmd = byte_swap_u16( state->num_pmd );
     state->total_polar = byte_swap_u16( state->total_polar );
     state->num_dsr = byte_swap_u16( state->num_dsr );
     state->length_dsr = byte_swap_u32( state->length_dsr );
     IEEE_Swap__FLT( &state->orbit_phase );
     for ( ni = 0; ni < state->num_clus; ni++ ) {
	  state->intg_times[ni] = byte_swap_u16( state->intg_times[ni] );
	  state->num_polar[ni] = byte_swap_u16( state->num_polar[ni] );
	  state->Clcon[ni].pixel_nr = 
	       byte_swap_u16( state->Clcon[ni].pixel_nr );
	  state->Clcon[ni].length = 
	       byte_swap_u16( state->Clcon[ni].length );
	  state->Clcon[ni].intg_time = 
	       byte_swap_u16( state->Clcon[ni].intg_time );
	  state->Clcon[ni].coaddf = 
	       byte_swap_u16( state->Clcon[ni].coaddf );
	  state->Clcon[ni].n_read = 
	       byte_swap_u16( state->Clcon[ni].n_read );
	  IEEE_Swap__FLT( &state->Clcon[ni].pet );
     }
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++
.IDENTifer   SET_LV1_STATE_OFFS
.PURPOSE     calculate correct offset to MDS in the level 1b product
.INPUT/OUTPUT
  call as    SET_LV1_STATE_OFFS( num_dsd, dsd, &do_init, state );
     input:
            unsigned int num_dsd :       number of DSDs
	    struct dsd_envi *dsd :       structure for the DSDs
 in/output:
            bool *do_init        :       initialize offsets according to DSD
	    struct state1_scia *state :  structure with States of the product
	    
.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void SET_LV1_STATE_OFFS( unsigned int num_dsd, const struct dsd_envi *dsd,
			 bool *do_init, struct state1_scia *state )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack, do_init, 
	           state->offset, state->offs_pmd, state->offs_polV@*/
{
     unsigned char nadc_stat_save = nadc_stat;
     unsigned int  indx_dsd;
/*
 * following variables are initialised when do_init = TRUE
 */
     static unsigned int nadir_offs, limb_offs, moni_offs, occul_offs;

     if ( *do_init ) {
	  *do_init = FALSE;
	  if ( IS_ERR_STAT_ABSENT )
	       nadc_stat &= ~NADC_STAT_ABSENT;

	  indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, "NADIR" );
	  if ( (! IS_ERR_STAT_ABSENT) && dsd[indx_dsd].num_dsr > 0 ) {
	       nadir_offs = dsd[indx_dsd].offset;
	  } else if ( IS_ERR_STAT_ABSENT ) {
	       if ( nadc_err_stack.nused > 0 ) nadc_err_stack.nused--;
	       nadc_stat = nadc_stat_save;
	  }
	  indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, "LIMB" );
	  if ( (! IS_ERR_STAT_ABSENT) && dsd[indx_dsd].num_dsr > 0 ) {
	       limb_offs = dsd[indx_dsd].offset;
	  } else if ( IS_ERR_STAT_ABSENT ) {
	       if ( nadc_err_stack.nused > 0 ) nadc_err_stack.nused--;
	       nadc_stat = nadc_stat_save;
	  }
	  indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, "OCCULTATION" );
	  if ( (! IS_ERR_STAT_ABSENT) && dsd[indx_dsd].num_dsr > 0 ) {
	       occul_offs = dsd[indx_dsd].offset;
	  } else if ( IS_ERR_STAT_ABSENT ) {
	       if ( nadc_err_stack.nused > 0 ) nadc_err_stack.nused--;
	       nadc_stat = nadc_stat_save;
	  }
	  indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, "MONITORING" );
	  if ( (! IS_ERR_STAT_ABSENT) && dsd[indx_dsd].num_dsr > 0 ) {
	       moni_offs = dsd[indx_dsd].offset;
	  } else if ( IS_ERR_STAT_ABSENT ) {
	       if ( nadc_err_stack.nused > 0 ) nadc_err_stack.nused--;
	       nadc_stat = nadc_stat_save;
	  }
     }
     state->offset    = 0u;
     state->offs_pmd  = 0u;
     state->offs_polV = 0u;
     if ( state->flag_mds == MDS_ATTACHED  ) {
	  switch ( state->type_mds ) {
	  case SCIA_NADIR:
	       state->offset = nadir_offs;
	       nadir_offs += (state->num_dsr * state->length_dsr);
	       break;
	  case SCIA_LIMB:
	       state->offset = limb_offs;
	       limb_offs += (state->num_dsr * state->length_dsr);
	       break;
	  case SCIA_OCCULT:
	       state->offset = occul_offs;
	       occul_offs += (state->num_dsr * state->length_dsr);
	       break;
	  case SCIA_MONITOR:
	       state->offset = moni_offs;
	       moni_offs += (state->num_dsr * state->length_dsr);
	       break;
	  }
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   SET_LV1C_STATE_OFFS
.PURPOSE     calculate correct offset to MDS in the level 1c product
.INPUT/OUTPUT
  call as    SET_LV1C_STATE_OFFS( fd, num_dsd, dsd, calopt, &do_init, state );
     input:
            FILE *fd             :       stream pointer
            unsigned int num_dsd :       number of DSDs
	    struct dsd_envi *dsd :       structure for the DSDs
	    struct cal_options calopt :  structure for L1c calibration options
 in/output:
            bool *do_init        :       initialize offsets according to DSD
	    struct state1_scia *state :  structure with States of the product
	    
.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void SET_LV1C_STATE_OFFS( FILE *fd, unsigned int num_dsd, 
			  const struct dsd_envi *dsd,
			  const struct cal_options calopt,
			  bool *do_init, struct state1_scia *state )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, do_init, fd, 
                   state->flag_mds, state->num_clus, state->Clcon, 
                   state->offset@*/
{
     register unsigned int nc;

     unsigned char nadc_stat_save = nadc_stat;

     unsigned int  indx_dsd;
     long  num_pix, num_obs;

     struct mjd_envi mjd;

     const long  geoC_size = 20l;
     const long  geoL_size = 112l;
     const long  geoN_size = 108l;
/*
 * following variables are initialised when do_init = TRUE
 */
     static unsigned int nadir_offs, limb_offs, moni_offs, occul_offs;
     static unsigned int nadir_max, limb_max, moni_max, occul_max;

     if ( *do_init ) {
	  *do_init = FALSE;
	  if ( IS_ERR_STAT_ABSENT )
	       nadc_stat &= ~NADC_STAT_ABSENT;

	  indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, "NADIR" );
	  if ( (! IS_ERR_STAT_ABSENT) && dsd[indx_dsd].num_dsr > 0 ) {
	       nadir_offs = dsd[indx_dsd].offset;
	       nadir_max = nadir_offs + dsd[indx_dsd].size;
	  } else if ( IS_ERR_STAT_ABSENT ) {
	       if ( nadc_err_stack.nused > 0 ) nadc_err_stack.nused--;
	       nadc_stat = nadc_stat_save;
	  }
	  indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, "LIMB" );
	  if ( (! IS_ERR_STAT_ABSENT) && dsd[indx_dsd].num_dsr > 0 ) {
	       limb_offs = dsd[indx_dsd].offset;
	       limb_max = limb_offs + dsd[indx_dsd].size;
	  } else if ( IS_ERR_STAT_ABSENT ) {
	       if ( nadc_err_stack.nused > 0 ) nadc_err_stack.nused--;
	       nadc_stat = nadc_stat_save;
	  }
	  indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, "OCCULTATION" );
	  if ( (! IS_ERR_STAT_ABSENT) && dsd[indx_dsd].num_dsr > 0 ) {
	       occul_offs = dsd[indx_dsd].offset;
	       occul_max = occul_offs + dsd[indx_dsd].size;
	  } else if ( IS_ERR_STAT_ABSENT ) {
	       if ( nadc_err_stack.nused > 0 ) nadc_err_stack.nused--;
	       nadc_stat = nadc_stat_save;
	  }
	  indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, "MONITORING" );
	  if ( (! IS_ERR_STAT_ABSENT) && dsd[indx_dsd].num_dsr > 0 ) {
	       moni_offs = dsd[indx_dsd].offset;
	       moni_max = moni_offs + dsd[indx_dsd].size;
	  } else if ( IS_ERR_STAT_ABSENT ) {
	       if ( nadc_err_stack.nused > 0 ) nadc_err_stack.nused--;
	       nadc_stat = nadc_stat_save;
	  }
     }
/*
 * check if the MDS of this state is attached
 */
     if ( state->flag_mds == MDS_ATTACHED ) {
	  switch ( state->type_mds ) {
	  case SCIA_NADIR:
	       if ( calopt.nadir_mds == SCHAR_ZERO )
		    state->flag_mds = UCHAR_ONE;
	       else if ( nadir_offs >= nadir_max ) 
		    state->flag_mds = UCHAR_ONE;
	       else {
		    (void) fseek( fd, (long) nadir_offs, SEEK_SET );
		    if ( fread( &mjd, sizeof(struct mjd_envi), 1, fd ) != 1 )
			 NADC_RETURN_ERROR( NADC_ERR_PDS_RD, "" );
#ifdef _SWAP_TO_LITTLE_ENDIAN
		    mjd.days = byte_swap_32( mjd.days );
		    mjd.secnd = byte_swap_u32( mjd.secnd );
		    mjd.musec = byte_swap_u32( mjd.musec );
#endif
		    if ( state->mjd.days != mjd.days 
			 || state->mjd.secnd != mjd.secnd
			 || state->mjd.musec != mjd.musec ) {
			 state->flag_mds = UCHAR_ONE;
		    }
	       }
	       break;
	  case SCIA_LIMB:
	       if ( calopt.limb_mds == SCHAR_ZERO )
		    state->flag_mds = UCHAR_ONE;
	       else if ( limb_offs >= limb_max )
		    state->flag_mds = UCHAR_ONE;
	       else {
		    (void) fseek( fd, (long) limb_offs, SEEK_SET );
		    if ( fread( &mjd, sizeof(struct mjd_envi), 1, fd ) != 1 )
			 NADC_RETURN_ERROR( NADC_ERR_PDS_RD, "" );
#ifdef _SWAP_TO_LITTLE_ENDIAN
		    mjd.days = byte_swap_32( mjd.days );
		    mjd.secnd = byte_swap_u32( mjd.secnd );
		    mjd.musec = byte_swap_u32( mjd.musec );
#endif
		    if ( state->mjd.days != mjd.days 
			 || state->mjd.secnd != mjd.secnd
			 || state->mjd.musec != mjd.musec ) {
			 state->flag_mds = UCHAR_ONE;
		    }
	       }
	       break;
	  case SCIA_OCCULT:
	       if ( calopt.occ_mds == SCHAR_ZERO )
		    state->flag_mds = UCHAR_ONE;
	       else if ( occul_offs >= occul_max ) 
		    state->flag_mds = UCHAR_ONE;
	       else {
		    (void) fseek( fd, (long) occul_offs, SEEK_SET );
		    if ( fread( &mjd, sizeof(struct mjd_envi), 1, fd ) != 1 )
			 NADC_RETURN_ERROR( NADC_ERR_PDS_RD, "" );
#ifdef _SWAP_TO_LITTLE_ENDIAN
		    mjd.days = byte_swap_32( mjd.days );
		    mjd.secnd = byte_swap_u32( mjd.secnd );
		    mjd.musec = byte_swap_u32( mjd.musec );
#endif
		    if ( state->mjd.days != mjd.days 
			 || state->mjd.secnd != mjd.secnd
			 || state->mjd.musec != mjd.musec ) {
			 state->flag_mds = UCHAR_ONE;
		    }
	       }
	       break;
	  case SCIA_MONITOR:
	       if ( calopt.moni_mds == SCHAR_ZERO )
		    state->flag_mds = UCHAR_ONE;
	       else if ( moni_offs >= moni_max ) 
		    state->flag_mds = UCHAR_ONE;
	       else {
		    (void) fseek( fd, (long) moni_offs, SEEK_SET );
		    if ( fread( &mjd, sizeof(struct mjd_envi), 1, fd ) != 1 )
			 NADC_RETURN_ERROR( NADC_ERR_PDS_RD, "" );
#ifdef _SWAP_TO_LITTLE_ENDIAN
		    mjd.days = byte_swap_32( mjd.days );
		    mjd.secnd = byte_swap_u32( mjd.secnd );
		    mjd.musec = byte_swap_u32( mjd.musec );
#endif
		    if ( state->mjd.days != mjd.days 
			 || state->mjd.secnd != mjd.secnd
			 || state->mjd.musec != mjd.musec ) {
			 state->flag_mds = UCHAR_ONE;
		    }
	       }
	       break;
	  }
     }
/*
 * check if state id within the dateTime selection
 */
     if ( state->flag_mds == MDS_ATTACHED 
	  && calopt.time_filter != SCHAR_ZERO ) {
	  double SecPerDay = 60. * 60. * 24.;
	  double bgn_jdate = calopt.start_time.days 
	       + calopt.start_time.secnd / SecPerDay;
	  double end_jdate = calopt.stop_time.days 
	       + calopt.stop_time.secnd / SecPerDay;
	  double state_jdate = state->mjd.days 
	       + (state->mjd.secnd + state->dur_scan / 32.) / SecPerDay;
	  if ( (state_jdate < bgn_jdate) || (state_jdate > end_jdate) )
	       state->flag_mds = UCHAR_ONE;
     }
/*
 * check and correct state-description for the presence of clusters
 */
     state->offset = 0u;
     if ( state->flag_mds == MDS_ATTACHED  ) {
	  unsigned int num_clus = state->num_clus;

	  switch ( state->type_mds ) {
	  case SCIA_NADIR:
	       state->offset = nadir_offs;

	       for ( nc = 0; nc < num_clus; nc++ ) {
		    if ( calopt.nadir_cluster[nc] != CHAR_ZERO ) {
			 num_pix = (long) state->Clcon[nc].length;
			 num_obs = (long) 
			      state->num_dsr * state->Clcon[nc].n_read;
			 nadir_offs += 32 
			      + (ENVI_USHRT + 2 * ENVI_FLOAT) * num_pix
			      + (2 * ENVI_FLOAT) * num_obs * num_pix
			      + geoN_size * num_obs;
		    }
	       }
	       break;
	  case SCIA_LIMB:
	       state->offset = limb_offs;
	       for ( nc = 0; nc < num_clus; nc++ ) {
		    if ( calopt.limb_cluster[nc] != CHAR_ZERO ) {
			 num_pix = (long) state->Clcon[nc].length;
			 num_obs = (long) 
			      state->num_dsr * state->Clcon[nc].n_read;
			 limb_offs += 32 
			      + (ENVI_USHRT + 2 * ENVI_FLOAT) * num_pix
			      + (2 * ENVI_FLOAT) * num_obs * num_pix
			      + geoL_size * num_obs;
		    }
	       }
	       break;
	  case SCIA_OCCULT:
	       state->offset = occul_offs;
	       for ( nc = 0; nc < num_clus; nc++ ) {
		    if ( calopt.occ_cluster[nc] != CHAR_ZERO ) {
			 num_pix = (long) state->Clcon[nc].length;
			 num_obs = (long) 
			      state->num_dsr * state->Clcon[nc].n_read;
			 occul_offs += 32 
			      + (ENVI_USHRT + 2 * ENVI_FLOAT) * num_pix
			      + (2 * ENVI_FLOAT) * num_obs * num_pix
			      + geoL_size * num_obs;
		    }
	       }
	       break;
	  case SCIA_MONITOR:
	       state->offset = moni_offs;
	       for ( nc = 0; nc < num_clus; nc++ ) {
		    if ( calopt.moni_cluster[nc] != CHAR_ZERO ) {
			 num_pix = (long) state->Clcon[nc].length;
			 num_obs = (long) 
			      state->num_dsr * state->Clcon[nc].n_read;
			 moni_offs += 32 
			      + (ENVI_USHRT + 2 * ENVI_FLOAT) * num_pix
			      + (2 * ENVI_FLOAT) * num_obs * num_pix
			      + geoC_size * num_obs;
		    }
	       }
	       break;
	  }
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   SET_LV1C_STATE_PMD_OFFS
.PURPOSE     calculate correct offset to MDS in the level 1c product
.INPUT/OUTPUT
  call as    SET_LV1C_STATE_PMD_OFFS( num_dsd, dsd, &do_init, state );
     input:
            unsigned int num_dsd :       number of DSDs
	    struct dsd_envi *dsd :       structure for the DSDs
 in/output:
            bool *do_init        :       initialize offsets according to DSD
	    struct state1_scia *state :  structure with States of the product
            
.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void SET_LV1C_STATE_PMD_OFFS( FILE *fd, unsigned int num_dsd, 
			      const struct dsd_envi *dsd,
			      bool *do_init, struct state1_scia *state )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack, do_init, fd, state->offs_pmd@*/
{
     unsigned char nadc_stat_save = nadc_stat;
     unsigned int  indx_dsd;
/*
 * following variables are initialised when do_init = TRUE
 */
     static bool is_nadc_product;
     static bool have_nadir_pmd, have_limb_pmd, have_occul_pmd;
     static unsigned int nadir_pmd_offs, limb_pmd_offs, occul_pmd_offs;

     if ( *do_init ) {
	  *do_init = FALSE;
	  if ( IS_ERR_STAT_ABSENT )
	       nadc_stat &= ~NADC_STAT_ABSENT;

	  indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, "NADIR_PMD" );
	  if ( (! IS_ERR_STAT_ABSENT) && dsd[indx_dsd].num_dsr > 0 ) {
	       nadir_pmd_offs = dsd[indx_dsd].offset;
	       have_nadir_pmd = TRUE;
	  } else {
	       if ( IS_ERR_STAT_ABSENT ) {
		    if ( nadc_err_stack.nused > 0 ) nadc_err_stack.nused--;
		    nadc_stat = nadc_stat_save;
	       }
	       have_nadir_pmd = FALSE;
	  }
	  indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, "LIMB_PMD" );
	  if ( (! IS_ERR_STAT_ABSENT) && dsd[indx_dsd].num_dsr > 0 ) {
	       limb_pmd_offs = dsd[indx_dsd].offset;
	       have_limb_pmd  = TRUE;
	  } else {
	       if ( IS_ERR_STAT_ABSENT ) {
		    if ( nadc_err_stack.nused > 0 ) nadc_err_stack.nused--;
		    nadc_stat = nadc_stat_save;
	       }
	       have_limb_pmd = FALSE;
	  }
	  indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, "OCCULTATION_PMD" );
	  if ( (! IS_ERR_STAT_ABSENT) && dsd[indx_dsd].num_dsr > 0 ) {
	       occul_pmd_offs = dsd[indx_dsd].offset;
	       have_occul_pmd = TRUE;
	  } else {
	       if ( IS_ERR_STAT_ABSENT ) {
		    if ( nadc_err_stack.nused > 0 ) nadc_err_stack.nused--;
		    nadc_stat = nadc_stat_save;
	       }
	       have_occul_pmd = FALSE;
	  }
	  if ( have_nadir_pmd || have_limb_pmd || have_occul_pmd ) {
	       unsigned short num_geo;

	       (void) fseek( fd, (long) dsd[indx_dsd].offset + 29l, SEEK_SET );
	       if ( fread( &num_geo, ENVI_USHRT, 1, fd ) != 1 )
		    NADC_RETURN_ERROR( NADC_ERR_PDS_RD, "" );
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       num_geo = byte_swap_u16( num_geo );
#endif
	       if ( num_geo == state->num_pmd )
		    is_nadc_product = TRUE;
	       else
		    is_nadc_product = FALSE;	       
	  }
     }

     if ( state->flag_mds == MDS_ATTACHED  ) {
	  const long SizeHdr  =  31l;
	  const long SizeGeoL = 112l;
	  const long SizeGeoN = 108l;
	  const long SizePMD  = (long) (PMD_NUMBER * ENVI_FLOAT);

	  switch ( state->type_mds ) {
	  case SCIA_NADIR:
	       if ( have_nadir_pmd ) {
		    state->offs_pmd = nadir_pmd_offs;
		    if ( is_nadc_product )
			 nadir_pmd_offs += 
			      SizeHdr + state->num_pmd * (SizePMD + SizeGeoN);
		    else
			 nadir_pmd_offs += SizeHdr
			      + state->num_aux * SizeGeoN
			      + state->num_pmd * SizePMD;
	       } else
		    state->offs_pmd = 0u;
	       break;
	  case SCIA_LIMB:
	       if ( have_limb_pmd ) {
		    state->offs_pmd = limb_pmd_offs;
		    if ( is_nadc_product )
			 limb_pmd_offs += 
			      SizeHdr + state->num_pmd * (SizePMD + SizeGeoL);
		    else
			 limb_pmd_offs += SizeHdr
			      + state->num_aux * SizeGeoL
			      + state->num_pmd * SizePMD;
	       } else
		    state->offs_pmd = 0u;
	       break;
	  case SCIA_OCCULT:
	       if ( have_occul_pmd ) {
		    state->offs_pmd  = occul_pmd_offs;
		    if ( is_nadc_product )
			 occul_pmd_offs += 
			      SizeHdr + state->num_pmd * (SizePMD + SizeGeoL);
		    else
			 occul_pmd_offs += SizeHdr
			      + state->num_aux * SizeGeoL
			      + state->num_pmd * SizePMD;
	       } else
		    state->offs_pmd = 0u;
	       break;
	  }
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   SET_LV1C_STATE_POLV_OFFS
.PURPOSE     calculate correct offset to MDS in the level 1c product
.INPUT/OUTPUT
  call as    SET_LV1C_STATE_POLV_OFFS( num_dsd, dsd, &do_init, state );
     input:
            unsigned int num_dsd :       number of DSDs
	    struct dsd_envi *dsd :       structure for the DSDs
 in/output:
            bool *do_init        :       initialize offsets according to DSD
	    struct state1_scia *state :  structure with States of the product
            
.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void SET_LV1C_STATE_POLV_OFFS( FILE *fd, unsigned int num_dsd, 
			       const struct dsd_envi *dsd,
			       bool *do_init, struct state1_scia *state )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack, do_init, fd, state->offs_polV@*/
{
     unsigned char nadc_stat_save = nadc_stat;
     unsigned int  indx_dsd;
/*
 * following variables are initialised when do_init = TRUE
 */
     static bool is_nadc_product;
     static bool have_nadir_polV, have_limb_polV, have_occul_polV;
     static unsigned int nadir_polV_offs, limb_polV_offs, occul_polV_offs;

     if ( *do_init ) {
	  *do_init = FALSE;
	  if ( IS_ERR_STAT_ABSENT )
	       nadc_stat &= ~NADC_STAT_ABSENT;

	  indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, "NADIR_FRAC_POL" );
	  if ( (! IS_ERR_STAT_ABSENT) && dsd[indx_dsd].num_dsr > 0 ) {
	       nadir_polV_offs = dsd[indx_dsd].offset;
	       have_nadir_polV = TRUE;
	  } else {
	       if ( IS_ERR_STAT_ABSENT ) {
		    if ( nadc_err_stack.nused > 0 ) nadc_err_stack.nused--;
		    nadc_stat = nadc_stat_save;
	       }
	       have_nadir_polV = FALSE;
	  }
	  indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, "LIMB_FRAC_POL" );
	  if ( (! IS_ERR_STAT_ABSENT) && dsd[indx_dsd].num_dsr > 0 ) {
	       limb_polV_offs = dsd[indx_dsd].offset;
	       have_limb_polV  = TRUE;
	  } else {
	       if ( IS_ERR_STAT_ABSENT ) {
		    if ( nadc_err_stack.nused > 0 ) nadc_err_stack.nused--;
		    nadc_stat = nadc_stat_save;
	       }
	       have_limb_polV = FALSE;
	  }
	  indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, "OCCULTATION_FRAC_POL" );
	  if ( (! IS_ERR_STAT_ABSENT) && dsd[indx_dsd].num_dsr > 0 ) {
	       occul_polV_offs = dsd[indx_dsd].offset;
	       have_occul_polV = TRUE;
	  } else {
	       if ( IS_ERR_STAT_ABSENT ) {
		    if ( nadc_err_stack.nused > 0 ) nadc_err_stack.nused--;
		    nadc_stat = nadc_stat_save;
	       }
	       have_occul_polV = FALSE;
	  }
	  if ( have_nadir_polV || have_limb_polV || have_occul_polV ) {
	       unsigned short num_geo;

	       (void) fseek( fd, (long) dsd[indx_dsd].offset + 27l, SEEK_SET );
	       if ( fread( &num_geo, ENVI_USHRT, 1, fd ) != 1 )
		    NADC_RETURN_ERROR( NADC_ERR_PDS_RD, "" );
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       num_geo = byte_swap_u16( num_geo );
#endif
	       if ( num_geo == state->total_polar )
		    is_nadc_product = TRUE;
	       else
		    is_nadc_product = FALSE;	       
	  }
     }

     if ( state->flag_mds == MDS_ATTACHED  ) {
	  const long SizeHdr  = 289l;
	  const long SizeGeoL = 112l;
	  const long SizeGeoN = 108l;
	  const long SizePolV = 256l;

	  switch ( state->type_mds ) {
	  case SCIA_NADIR:
	       if ( have_nadir_polV ) {
		    state->offs_polV = nadir_polV_offs;
		    if ( is_nadc_product ) 
			 nadir_polV_offs += SizeHdr 
			      + state->total_polar * (SizePolV + SizeGeoN);
		    else
			 nadir_polV_offs += SizeHdr 
			      + state->num_aux * SizeGeoN
			      + state->total_polar * SizePolV;
	       } else
		    state->offs_polV = 0u;
	       break;
	  case SCIA_LIMB:
	       if ( have_limb_polV ) {
		    state->offs_polV = limb_polV_offs;
		    if ( is_nadc_product )
			 limb_polV_offs += SizeHdr 
			      + state->total_polar * (SizePolV + SizeGeoL);
		    else
			 limb_polV_offs += SizeHdr 
			      + state->num_aux * SizeGeoL
			      + state->total_polar * SizePolV;
	       } else
		    state->offs_polV = 0u;
	       break;
	  case SCIA_OCCULT:
	       if ( have_occul_polV ) {
		    state->offs_polV = occul_polV_offs;
		    if ( is_nadc_product )
			 occul_polV_offs += SizeHdr 
			      + state->total_polar * (SizePolV + SizeGeoL);
		    else
			 occul_polV_offs += SizeHdr 
			      + state->num_aux * SizeGeoL
			      + state->total_polar * SizePolV;
	       } else
		    state->offs_polV = 0u;
	       break;
	  }
     }
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_STATE
.PURPOSE     read States of the Product
.INPUT/OUTPUT
  call as   nr_dsr = SCIA_LV1_RD_STATE( fd, num_dsd, dsd, &state );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi *dsd  :   structure for the DSDs
    output:
            struct state1_scia **state :  States of the product

.RETURNS     number of data set records read (unsigned int)
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV1_RD_STATE( FILE *fd, unsigned int num_dsd, 
				const struct dsd_envi *dsd,
				struct state1_scia **state_out )
{
     register unsigned short ni;
     register unsigned int   nr_dsr = 0;

     char         *state_pntr, *state_char = NULL;
     size_t       dsr_size, nr_byte;
     unsigned int indx_dsd;

     struct state1_scia *state;

     bool init_offs      = TRUE;
     bool init_offs_pmd  = TRUE;
     bool init_offs_polv = TRUE;

     const char dsd_name[] = "STATES";
/*
 * get index to data set descriptor
 */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, dsd_name );
     if ( dsd[indx_dsd].num_dsr == 0 ) {
          state_out[0] = NULL;
          return 0u;
     }
     if ( ! Use_Extern_Alloc ) {
	  state_out[0] = (struct state1_scia *) 
	       calloc((size_t)dsd[indx_dsd].num_dsr,sizeof(struct state1_scia));
     }
     if ( (state = state_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "state" );
/*
 * allocate memory to temporary store data for output structure
 */
     dsr_size = (size_t) dsd[indx_dsd].dsr_size;
     if ( (state_char = (char *) malloc( dsr_size )) == NULL ) 
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "state_char" );
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
/*
 * read data set records
 */
     do {
	  state->indx = nr_dsr;
	  if ( fread( state_char, dsr_size, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "" );
/*
 * read data buffer to STATE structure
 */
	  state_pntr = state_char;
	  (void) memcpy( &state->mjd, state_pntr, sizeof( struct mjd_envi ) );
	  state_pntr += sizeof( struct mjd_envi );
	  (void) memcpy( &state->flag_mds, state_pntr, ENVI_UCHAR );
	  state_pntr += ENVI_UCHAR;
	  (void) memcpy( &state->flag_reason, state_pntr, ENVI_UCHAR );
	  state_pntr += ENVI_UCHAR;
	  (void) memcpy( &state->orbit_phase, state_pntr, ENVI_FLOAT );
	  state_pntr += ENVI_FLOAT;
	  (void) memcpy( &state->category, state_pntr, ENVI_USHRT );
	  state_pntr += ENVI_USHRT;
	  (void) memcpy( &state->state_id, state_pntr, ENVI_USHRT );
	  state_pntr += ENVI_USHRT;
	  (void) memcpy( &state->dur_scan, state_pntr, ENVI_USHRT );
	  state_pntr += ENVI_USHRT;
	  (void) memcpy( &state->longest_intg_time, state_pntr, ENVI_USHRT );
	  state_pntr += ENVI_USHRT;
	  (void) memcpy( &state->num_clus, state_pntr, ENVI_USHRT );
	  state_pntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  state->num_clus = byte_swap_u16( state->num_clus );
#endif
	  for ( ni = 0; ni < state->num_clus; ni++ ) {
	       (void) memcpy( &state->Clcon[ni].id, state_pntr, ENVI_UCHAR );
	       state_pntr += ENVI_UCHAR;
	       (void) memcpy( &state->Clcon[ni].channel, state_pntr, 
				ENVI_UCHAR );
	       state_pntr += ENVI_UCHAR;
	       (void) memcpy( &state->Clcon[ni].pixel_nr, state_pntr, 
				ENVI_USHRT);
	       state_pntr += ENVI_USHRT;
	       (void) memcpy( &state->Clcon[ni].length, state_pntr, 
				ENVI_USHRT );
	       state_pntr += ENVI_USHRT;
	       (void) memcpy( &state->Clcon[ni].pet, state_pntr, 
				ENVI_FLOAT );
	       state_pntr += ENVI_FLOAT;
	       (void) memcpy( &state->Clcon[ni].intg_time, state_pntr, 
				ENVI_USHRT);
	       state_pntr += ENVI_USHRT;
	       (void) memcpy( &state->Clcon[ni].coaddf, state_pntr, 
				ENVI_USHRT );
	       state_pntr += ENVI_USHRT;
	       (void) memcpy( &state->Clcon[ni].n_read, state_pntr, 
				ENVI_USHRT );
	       state_pntr += ENVI_USHRT;
	       (void) memcpy( &state->Clcon[ni].type, state_pntr, 
				ENVI_UCHAR );
	       state_pntr += ENVI_UCHAR;
	  }
	  state_pntr += LV1_Clcon_LENGTH * (MAX_CLUSTER - state->num_clus);
	  (void) memcpy( &state->type_mds, state_pntr, ENVI_UCHAR );
	  state_pntr += ENVI_UCHAR;
	  (void) memcpy( &state->num_aux, state_pntr, ENVI_USHRT );
	  state_pntr += ENVI_USHRT;
	  (void) memcpy( &state->num_pmd, state_pntr, ENVI_USHRT );
	  state_pntr += ENVI_USHRT;
	  (void) memcpy( &state->num_intg, state_pntr, ENVI_USHRT );
	  state_pntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  state->num_intg = byte_swap_u16( state->num_intg );
#endif
	  nr_byte = (size_t) state->num_intg * ENVI_USHRT;
	  (void) memcpy( state->intg_times, state_pntr, nr_byte );
	  state_pntr += MAX_CLUSTER * ENVI_USHRT;

	  (void) memcpy( state->num_polar, state_pntr, nr_byte );
	  state_pntr += MAX_CLUSTER * ENVI_USHRT;

	  (void) memcpy( &state->total_polar, state_pntr, ENVI_USHRT );
	  state_pntr += ENVI_USHRT;
	  (void) memcpy( &state->num_dsr, state_pntr, ENVI_USHRT );
	  state_pntr += ENVI_USHRT;
	  (void) memcpy( &state->length_dsr, state_pntr, ENVI_UINT );
	  state_pntr += ENVI_UINT;
/*
 * check if we read the whole DSR
 */
	  if ( (size_t)(state_pntr - state_char) != dsr_size )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_SIZE, dsd_name );
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_STATE( state );
#endif
	  state++;
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
/*
 * calculate offset in bytes to different MDS DSD's in the file
 */
     nr_dsr = 0;
     state -= dsd[indx_dsd].num_dsr;
     if ( ! IS_SCIA_LV1C( num_dsd, dsd ) ) {
	  do {
	       SET_LV1_STATE_OFFS( num_dsd, dsd, &init_offs, state );
	       state++;
	  } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
     } else {
	  struct cal_options calopt;

	  (void) SCIA_LV1C_RD_CALOPT( fd, num_dsd, dsd, &calopt );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "CALOPT" );
	  
	  do {
	       SET_LV1C_STATE_OFFS( fd, num_dsd, dsd, calopt, 
				    &init_offs, state );
	       SET_LV1C_STATE_PMD_OFFS( fd, num_dsd, dsd, 
					&init_offs_pmd, state );
	       SET_LV1C_STATE_POLV_OFFS( fd, num_dsd, dsd, 
					 &init_offs_polv, state );
	       state++;
	  } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
     }
/*
 * set return values
 */
     state_pntr = NULL;
 done:
     if ( state_char != NULL ) free( state_char );
     return nr_dsr;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_STATE
.PURPOSE     write States of the Product
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_STATE( fd, num_dsd, dsd, state );
     input:
            FILE *fd               :   stream pointer
	    unsigned int num_state :   number of STATE records
            struct state1_scia *state :  States of the product

.RETURNS     nothing
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1_WR_STATE( FILE *fd, unsigned int num_state,
			const struct state1_scia *state_in )
{
     register unsigned short ni;

     const size_t nr_byte = MAX_CLUSTER * ENVI_USHRT;
    
     struct state1_scia state;

     struct dsd_envi dsd = {
          "STATES", "A",
          "                                                              ",
          0u, 0u, 0u, 0
     };

     if ( num_state == 0u ) {
	  (void) strcpy( dsd.flname, "MISSING" );
	  SCIA_LV1_ADD_DSD( &dsd );
	  return;
     }
/*
 * write data set records
 */
     do {
	  (void) memcpy(  &state, state_in, sizeof( struct state1_scia ));
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_STATE( &state );
	  state.num_clus = byte_swap_u16( state.num_clus );
	  state.num_intg = byte_swap_u16( state.num_intg );
#endif
/*
 * write STATE structure file
 */
	  if ( fwrite( &state.mjd, sizeof(struct mjd_envi), 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += sizeof( struct mjd_envi );
	  if ( fwrite( &state.flag_mds, ENVI_UCHAR, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UCHAR;
	  if ( fwrite( &state.flag_reason, ENVI_UCHAR, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UCHAR;
	  if ( fwrite( &state.orbit_phase, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_FLOAT;
	  if ( fwrite( &state.category, ENVI_USHRT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_USHRT;
	  if ( fwrite( &state.state_id, ENVI_USHRT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_USHRT;
	  if ( fwrite( &state.dur_scan, ENVI_USHRT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_USHRT;
	  if ( fwrite( &state.longest_intg_time, ENVI_USHRT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_USHRT;
	  if ( fwrite( &state.num_clus, ENVI_USHRT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_USHRT;
	  for ( ni = 0; ni < MAX_CLUSTER; ni++ ) {
	       if ( fwrite( &state.Clcon[ni].id, ENVI_UCHAR, 1, fd ) != 1 )
		    NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	       dsd.size += ENVI_UCHAR;
	       if ( fwrite( &state.Clcon[ni].channel, ENVI_UCHAR, 1, fd ) != 1 )
		    NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	       dsd.size += ENVI_UCHAR;
	       if ( fwrite( &state.Clcon[ni].pixel_nr, ENVI_USHRT, 1, fd) != 1 )
		    NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	       dsd.size += ENVI_USHRT;
	       if ( fwrite( &state.Clcon[ni].length, ENVI_USHRT, 1, fd ) != 1 )
		    NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	       dsd.size += ENVI_USHRT;
	       if ( fwrite( &state.Clcon[ni].pet, ENVI_FLOAT, 1, fd ) != 1 )
		    NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	       dsd.size += ENVI_FLOAT;
	       if ( fwrite(&state.Clcon[ni].intg_time, ENVI_USHRT, 1, fd) != 1 )
		    NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	       dsd.size += ENVI_USHRT;
	       if ( fwrite( &state.Clcon[ni].coaddf, ENVI_USHRT, 1, fd ) != 1 )
		    NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	       dsd.size += ENVI_USHRT;
	       if ( fwrite( &state.Clcon[ni].n_read, ENVI_USHRT, 1, fd ) != 1 )
		    NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	       dsd.size += ENVI_USHRT;
	       if ( fwrite( &state.Clcon[ni].type, ENVI_UCHAR, 1, fd ) != 1 )
		    NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	       dsd.size += ENVI_UCHAR;
	  }
	  if ( fwrite( &state.type_mds, ENVI_UCHAR, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UCHAR;
	  if ( fwrite( &state.num_aux, ENVI_USHRT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_USHRT;
	  if ( fwrite( &state.num_pmd, ENVI_USHRT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_USHRT;
	  if ( fwrite( &state.num_intg, ENVI_USHRT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_USHRT;
	  if ( fwrite( state.intg_times, nr_byte ,1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( state.num_polar, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( &state.total_polar, ENVI_USHRT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_USHRT;
	  if ( fwrite( &state.num_dsr, ENVI_USHRT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_USHRT;
	  if ( fwrite( &state.length_dsr, ENVI_UINT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UINT;

	  state_in++;
     } while ( ++dsd.num_dsr < num_state );
/*
 * update list of written DSD records
 */
     dsd.dsr_size = (int) (dsd.size / dsd.num_dsr);
     SCIA_LV1_ADD_DSD( &dsd );
}
