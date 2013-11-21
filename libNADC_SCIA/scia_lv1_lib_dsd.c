/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2005 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV1_LIB_DSD
.AUTHOR      R.M. van Hees
.KEYWORDS    Sciamachy DSD
.LANGUAGE    ANSI C
.PURPOSE     Sciamachy level 1 DSD book keeping routines
.COMMENTS    contains: SCIA_LV1_EXPORT_NUM_STATE, SCIA_LV1_SET_NUM_ATTACH
             SCIA_LV1_WR_DSD_INIT, SCIA_LV1_ADD_DSD, SCIA_LV1_WR_DSD_UPDATE
             SCIA_LV1_UPDATE_SQADS, SCIA_LV1_UPDATE_LADS, SCIA_LV1_UPDATE_STATE
.ENVIRONment None
.VERSION     1.2     22-Dec-2005   fixed serious bugs in SCIA_LV1_ADD_DSD
                                   selecting/writing non-MDS, Klaus Bramstedt
             1.1     17-Oct-2005   fixed serious bugs in SCIA_LV1_WR_DSD_INIT
                                   fixed serious bugs in SCIA_LV1_ADD_DSD
                                   added more error checks, RvH
             1.0     11-Oct-2005   initial release by R. M. van Hees
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
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

/*+++++ Macros +++++*/
#define MAX_ARRAY_DSD      128
#define MAX_ARRAY_STATE    128

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
static unsigned int num_dsd_wr = 0u;
static unsigned int num_dsd_out = 0u;
static struct dsd_envi dsd_out[MAX_ARRAY_DSD];

/* number of selected states with MDS attached and are of the selected types */
static unsigned short num_attach_states = 0u;
static unsigned int   indx_attach_states[MAX_ARRAY_STATE];

/* number of selected states returned by SCIA_LV1_SELECT_MDS */
static unsigned short num_nadir_states = 0;
static unsigned short num_limb_states = 0;
static unsigned short num_occult_states = 0;
static unsigned short num_monitor_states = 0;

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_INIT_DSD
.PURPOSE    determine number of DSD's in output file, 
            and reset all (static) global variables
.INPUT/OUTPUT
  call as   SCIA_LV1_INIT_DSD( write_lv1c, num_dsd, dsd );
     input:
            unsigned int num_dsd      :  number of DSD's (input)
            
.RETURNS     number of DSD's in outputfile (unsigned int)
.COMMENTS    static function
-------------------------*/
static
void SCIA_LV1_INIT_DSD( unsigned char write_lv1c, unsigned int num_dsd_in, 
			const struct dsd_envi *dsd_in )
     /*@globals  num_nadir_states, num_limb_states, num_occult_states,
       num_monitor_states, num_dsd_wr, num_dsd_out, dsd_out,
       nadc_stat, nadc_err_stack;@*/
     /*@modifies num_nadir_states, num_limb_states, num_occult_states,
       num_monitor_states, num_dsd_wr, num_dsd_out, dsd_out,
       nadc_stat, nadc_err_stack@*/
{
     const char prognm[] = "SCIA_LV1_INIT_DSD";

     register unsigned int nr_in, nr_out;
/*
 * initialize number of DSD in output-file equal to input-file
 */
     num_nadir_states = 0;
     num_limb_states = 0;
     num_occult_states = 0;
     num_monitor_states = 0;

     num_dsd_wr = 0u;
     num_dsd_out = num_dsd_in;
/*
 * determine number of DSD's on selections given in param record
 *
 * do not write DSD records for ADS, except STATES
 */
     if ( write_lv1c == PARAM_SET ) {
	  num_dsd_out--;  /* no PMD */
	  num_dsd_out--;  /* no AUX */
	  num_dsd_out--;  /* no LCPN */
	  num_dsd_out--;  /* no DARK */
	  num_dsd_out--;  /* no PPGN */
	  num_dsd_out--;  /* no SCPN */
	  num_dsd_out--;  /* no SRSN */

	  if ( ! IS_SCIA_LV1C( num_dsd_in, dsd_in ) ) {
	       num_dsd_out++;  /* add CalOpt */
	       num_dsd_out++;  /* add PMD nadir */
	       num_dsd_out++;  /* add PolV nadir */
	       num_dsd_out++;  /* add PMD Limb */
	       num_dsd_out++;  /* add PolV Limb */
	       num_dsd_out++;  /* add PMD Occultation */
	       num_dsd_out++;  /* add PolV Occultation */
	  }
     }
     if ( num_dsd_out == 0u || num_dsd_out >= MAX_ARRAY_DSD ) {
	  char msg[SHORT_STRING_LENGTH];

	  (void) snprintf( msg, SHORT_STRING_LENGTH, "%s: %-u",
			   "invalid number of DSD records", num_dsd_out );
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, msg );
     }
     if ( write_lv1c == PARAM_UNSET ) {
	  (void) memcpy(dsd_out, dsd_in, num_dsd_in * sizeof(struct dsd_envi));
     } else {
	  nr_in = nr_out= 0;
	  do {
	       (void) nadc_strlcpy( dsd_out[nr_out].name, 
				    dsd_in[nr_in].name, 29 );
	       (void) nadc_strlcpy( dsd_out[nr_out].type, 
				    dsd_in[nr_in].type, 2 );
	       (void) nadc_strlcpy( dsd_out[nr_out].flname, 
				    dsd_in[nr_in].flname, ENVI_FILENAME_SIZE );
	       nr_out++;
	  } while ( strncmp( dsd_in[nr_in++].name, "STATES", 6 ) );
/*
 * skip all ADS records (except STATES)
 */
	  while ( dsd_in[nr_in].type[0] != 'M' ) nr_in++;

	  (void) nadc_strlcpy( dsd_out[nr_out].name, "CAL_OPTIONS", 29 );
	  (void) nadc_strlcpy( dsd_out[nr_out++].type, "G", 2 );

	  (void) nadc_strlcpy( dsd_out[nr_out].name, "NADIR", 29 );
	  (void) nadc_strlcpy( dsd_out[nr_out].type, "M", 2 );
	  (void) nadc_strlcpy( dsd_out[nr_out++].flname, 
			       "NOT USED", ENVI_FILENAME_SIZE );

	  (void) nadc_strlcpy( dsd_out[nr_out].name, "NADIR_PMD", 29 );
	  (void) nadc_strlcpy( dsd_out[nr_out].type, "M", 2 );
	  (void) nadc_strlcpy( dsd_out[nr_out++].flname, 
			       "NOT USED", ENVI_FILENAME_SIZE );

	  (void) nadc_strlcpy( dsd_out[nr_out].name, "NADIR_FRAC_POL", 29 );
	  (void) nadc_strlcpy( dsd_out[nr_out].type, "M", 2 );
	  (void) nadc_strlcpy( dsd_out[nr_out++].flname, 
			       "NOT USED", ENVI_FILENAME_SIZE );

	  (void) nadc_strlcpy( dsd_out[nr_out].name, "LIMB", 29 );
	  (void) nadc_strlcpy( dsd_out[nr_out].type, "M", 2 );
	  (void) nadc_strlcpy( dsd_out[nr_out++].flname, 
			       "NOT USED", ENVI_FILENAME_SIZE );

	  (void) nadc_strlcpy( dsd_out[nr_out].name, "LIMB_PMD", 29 );
	  (void) nadc_strlcpy( dsd_out[nr_out].type, "M", 2 );
	  (void) nadc_strlcpy( dsd_out[nr_out++].flname, 
			       "NOT USED", ENVI_FILENAME_SIZE );

	  (void) nadc_strlcpy( dsd_out[nr_out].name, "LIMB_FRAC_POL", 29 );
	  (void) nadc_strlcpy( dsd_out[nr_out].type, "M", 2 );
	  (void) nadc_strlcpy( dsd_out[nr_out++].flname, 
			       "NOT USED", ENVI_FILENAME_SIZE );

	  (void) nadc_strlcpy( dsd_out[nr_out].name, "OCCULTATION", 29 );
	  (void) nadc_strlcpy( dsd_out[nr_out].type, "M", 2 );
	  (void) nadc_strlcpy( dsd_out[nr_out++].flname, 
			       "NOT USED", ENVI_FILENAME_SIZE );

	  (void) nadc_strlcpy( dsd_out[nr_out].name, "OCCULTATION_PMD", 29 );
	  (void) nadc_strlcpy( dsd_out[nr_out].type, "M", 2 );
	  (void) nadc_strlcpy( dsd_out[nr_out++].flname, 
			       "NOT USED", ENVI_FILENAME_SIZE );

	  (void) nadc_strlcpy( dsd_out[nr_out].name, 
			       "OCCULTATION_FRAC_POL", 29 );
	  (void) nadc_strlcpy( dsd_out[nr_out].type, "M", 2 );
	  (void) nadc_strlcpy( dsd_out[nr_out++].flname, 
			       "NOT USED", ENVI_FILENAME_SIZE );

	  (void) nadc_strlcpy( dsd_out[nr_out].name, "MONITORING", 29 );
	  (void) nadc_strlcpy( dsd_out[nr_out].type, "M", 2 );
	  (void) nadc_strlcpy( dsd_out[nr_out++].flname, 
			       "NOT USED", ENVI_FILENAME_SIZE );
	  do {
	       if ( dsd_in[nr_in].type[0] == 'R' ) {
		    (void) memcpy( &dsd_out[nr_out++], &dsd_in[nr_in], 
				   sizeof(struct dsd_envi) );
	       }
	  } while( ++nr_in < num_dsd_in );
     }
/*
 * make sure that all integer DSD parameters are set to zero
 */
     nr_out = 0;
     do {
	  if ( dsd_out[nr_out].type[0] != 'R' ) {
	       if ( dsd_out[nr_out].type[0] == 'M' ) {
		    (void) nadc_strlcpy( dsd_out[nr_out].flname, 
					 "NOT USED", ENVI_FILENAME_SIZE );
	       }
	       dsd_out[nr_out].offset = 0u;
	       dsd_out[nr_out].size = 0u;
	       dsd_out[nr_out].num_dsr = 0u;
	       dsd_out[nr_out].dsr_size = 0;
	  }
     } while( ++nr_out < num_dsd_out );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_EXPORT_NUM_STATE
.PURPOSE    allow external program to update number of NADIR, LIMB, ... states
.INPUT/OUTPUT
  call as   SCIA_LV1_EXPORT_NUM_STATE( source, number );
     input:
            int source            : data source (Nadir, Limb, ...)
	    unsigned short number : number of selected states
            
.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void SCIA_LV1_EXPORT_NUM_STATE( int source, unsigned short number )
{
     switch ( source ) {
     case SCIA_NADIR:
	  num_nadir_states = number;
          break;
     case SCIA_LIMB:
	  num_limb_states = number;
          break;
     case SCIA_OCCULT:
	  num_occult_states = number;
          break;
     case SCIA_MONITOR:
	  num_monitor_states = number;
          break;
     }
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_ADD_DSD
.PURPOSE    add DSD records to list of DSD written in output file
.INPUT/OUTPUT
  call as   SCIA_LV1_ADD_DSD( dsd_to_add );
     input:
            struct dsd_envi *dsd_to_add
            
.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void SCIA_LV1_ADD_DSD( const struct dsd_envi *dsd_to_add )
{
     unsigned int indx =
	  ENVI_GET_DSD_INDEX( num_dsd_out, dsd_out, dsd_to_add->name );

     if ( dsd_to_add->type[0] == 'M' ) {
	  if ( dsd_to_add->num_dsr > 0u ) {
	       if ( dsd_out[indx].num_dsr == 0u ) num_dsd_wr++;

	       dsd_out[indx].size += dsd_to_add->size;
	       dsd_out[indx].num_dsr += dsd_to_add->num_dsr;
	       (void) strcpy( dsd_out[indx].flname, "" );
	       if ( dsd_to_add->num_dsr > 1u ) {
		    dsd_out[indx].dsr_size = -1;
	       } else {                     /* dsd_to_add->num_dsr == 1u */
		    int dsr_size_new = (int) dsd_to_add->size;

		    if ( dsd_out[indx].dsr_size == 0 )
			 dsd_out[indx].dsr_size = dsr_size_new;
		    else if ( dsd_out[indx].dsr_size != -1
			      && (dsd_out[indx].dsr_size != dsr_size_new) )
			 dsd_out[indx].dsr_size = -1;
	       }
	  } else if ( dsd_out[indx].num_dsr == 0u ) {
	       (void) strcpy( dsd_out[indx].flname, "NOT USED" );
	  }
     } else {
	  dsd_out[indx].size = dsd_to_add->size;
	  dsd_out[indx].num_dsr = dsd_to_add->num_dsr;
	  dsd_out[indx].dsr_size = dsd_to_add->dsr_size;
	  if ( dsd_out[indx].num_dsr > 0u ) {
	       (void) strcpy( dsd_out[indx].flname, "" );
	       num_dsd_wr++;
	  } else {
	       (void) strcpy( dsd_out[indx].flname, "NOT USED" );
	  }
     }
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_WR_DSD_INIT
.PURPOSE    initialise DSD records and write them to file
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_DSD_INIT( param, fp_out, num_dsd_in, dsd_in );
     input:
            struct param_record param :  struct holding user-defined settings
	    FILE *fp_in               :  (open) stream pointer (input)
	    FILE *fp_out              :  (open) stream pointer (output)
	    unsigned int num_dsd_in   :  number of DSD's (input)
	    struct dsd_envi *dsd_in   :  DSD's records (input)
            
.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void SCIA_LV1_WR_DSD_INIT( const struct param_record param, FILE *fp_out, 
			   unsigned int num_dsd_in, 
			   const struct dsd_envi *dsd_in )
       /*@globals  num_nadir_states, num_limb_states, num_occult_states,
                   num_monitor_states;@*/
       /*@modifies num_nadir_states, num_limb_states, num_occult_states,
                   num_monitor_states, num_dsd_wr, num_dsd_out, dsd_out, 
		   fp_out@*/
{
/*
 * obtain number of DSD-records, allocated empty DSD-records for initial write
 */
     SCIA_LV1_INIT_DSD( param.write_lv1c, num_dsd_in, dsd_in );
/*
 * write empty DSD-records, 
 * the correct values are written through a call of SCIA_LV1_WR_DSD_UPDATE
 */
     ENVI_WR_DSD( fp_out, num_dsd_out, dsd_out );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_SET_NUM_ATTACH
.PURPOSE    update global variables: num_attach_states, indx_attach_states
.INPUT/OUTPUT
  call as   SCIA_LV1_SET_NUM_ATTACH( param, fp_in, num_dsd_in, dsd_in );
     input:
            struct param_record param :  struct holding user-defined settings
	    FILE *fp_in               :  (open) stream pointer (input)
	    unsigned int num_dsd_in   :  number of DSD's (input)
	    struct dsd_envi *dsd_in   :  DSD's records (input)
            
.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void SCIA_LV1_SET_NUM_ATTACH( const struct param_record param, FILE *fp_in, 
			      unsigned int num_dsd_in, 
			      const struct dsd_envi *dsd_in )
       /*@globals errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc,
                  num_attach_states, indx_attach_states;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, 
                  num_attach_states, indx_attach_states, fp_in@*/
{
     const char prognm[] = "SCIA_LV1_SET_NUM_ATTACH";

     register unsigned int ni;

     unsigned int num_dsr;

     struct state1_scia *state;
/*
 * get number of selected states with a MDS attached 
 *                                    and are of one of the selected types
 */
     num_dsr = SCIA_LV1_RD_STATE( fp_in, num_dsd_in, dsd_in, &state );
     if ( IS_ERR_STAT_FATAL || num_dsr  ==  0 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "STATE" );
     
     num_attach_states = 0u;
     for ( ni = 0; ni < num_dsr; ni++ ) {
	  bool found = TRUE;

	  if ( state[ni].flag_mds != UCHAR_ZERO ) {
	       found = FALSE;
	  } else if ( param.write_lv1c == PARAM_UNSET ) {
	       switch ( (int) state[ni].type_mds ) {
	       case SCIA_NADIR:
		    if ( param.write_nadir != PARAM_SET ) found = FALSE;
		    break;
	       case SCIA_LIMB:
		    if ( param.write_limb != PARAM_SET ) found = FALSE;
		    break;
	       case SCIA_OCCULT:
		    if ( param.write_occ != PARAM_SET ) found = FALSE;
		    break;
	       case SCIA_MONITOR:
		    if ( param.write_moni != PARAM_SET ) found = FALSE;
		    break;
	       }
	  }
	  if ( found ) indx_attach_states[num_attach_states++] = ni;
     }
     free( state );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_WR_DSD_UPDATE
.PURPOSE    update DSD records in output file
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_DSD_UPDATE( fp_in, fp_out );
     input:
            FILE *fp_in   :  (open) stream pointer (input)
	    FILE *fp_out  :  (open) stream pointer (output)
            
.RETURNS     nothing
.COMMENTS    IMPORTANT: call this routine just before closing the PDS file
             Update MPH: PRODUCT, NUM_DSD, NUM_DATA_SETS
             Update SPH: NO_OF_NADIR_STATES, NO_OF_LIMB_STATES, 
                         NO_OF_OCCULTATION_STATES, NO_OF_MONI_STATES
             Update DSD records in file 
                         (recalculate all dsd records "ds_offset")
-------------------------*/
void SCIA_LV1_WR_DSD_UPDATE( FILE *fp_in, FILE *fp_out )
       /*@globals  errno, stderr, nadc_stat, nadc_err_stack, 
                   num_nadir_states, num_limb_states, num_occult_states,
                  num_monitor_states, num_dsd_out, dsd_out;@*/
       /*@modifies errno, stderr, nadc_stat, nadc_err_stack, fp_in, fp_out,
                   dsd_out[].offset@*/
{
     const char prognm[] = "SCIA_LV1_WR_DSD_UPDATE";

     register unsigned int nr;
     register unsigned int offset;

     unsigned int sph_only_size;
     unsigned int num_data_sets;

     struct mph_envi    mph;
     struct sph1_scia   sph;

/* Note: MPH maybe modified, therefore, read MPH from output file */
     ENVI_RD_MPH( fp_out, &mph );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "MPH" );
     SCIA_LV1_RD_SPH( fp_in, mph, &sph );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "SPH" );
/*
 * update DSD records (recalculate all dsd records "ds_offset")
 */
     sph_only_size = mph.sph_size - mph.num_dsd * mph.dsd_size;
     num_data_sets = 2;          /* MPH & SPH */
     offset = PDS_MPH_LENGTH + sph_only_size 
	  + (num_dsd_out+1) * mph.dsd_size;
     for ( nr = 0; nr < num_dsd_out; nr++ ) {
	  if ( dsd_out[nr].type[0] == 'R' ) break;

	  if ( dsd_out[nr].size > 0u ) num_data_sets++;
	  if ( strncmp( dsd_out[nr].flname, "NOT USED", 8 ) != 0 )
	       dsd_out[nr].offset = offset;
	  offset += dsd_out[nr].size;
     }
/*
 * update MPH records
 */
     if ( fseek( fp_out, 0L, SEEK_SET ) != 0 ) 
	  perror( "SCIA_LV1_WR_DSD_UPDATE" );
     if ( IS_SCIA_LV1C( num_dsd_out, dsd_out )
	  && strncmp( mph.product, "SCI_NL__1P", 10 ) == 0 ) {
	  mph.product[6] = 'C';
	  mph.product[9] = 'C';
     }
     mph.num_dsd = num_dsd_out + 1;
     mph.sph_size = sph_only_size + mph.num_dsd * mph.dsd_size;
     mph.tot_size = offset;
     mph.num_data_sets = num_data_sets;
     ENVI_WR_MPH( fp_out, mph );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "MPH" );
/*
 * update SPH records
 */
     sph.no_nadir = num_nadir_states;
     sph.no_limb = num_limb_states;
     sph.no_occult = num_occult_states;
     sph.no_monitor = num_monitor_states;
     SCIA_LV1_WR_SPH( fp_out, mph, sph );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "SPH" );
/*
 * update DSD records
 */
     ENVI_WR_DSD( fp_out, num_dsd_out, dsd_out );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "DSD" );
/*
 * close output file
 */
     (void) fclose( fp_out );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_UPDATE_SQADS
.PURPOSE     return selected SQADS records with MDS attach
.INPUT/OUTPUT
  call as   num_sqads_out = SCIA_LV1_UPDATE_SQADS( sqads );
 in/output:
            struct sqads1_scia *sqads :  summary of quality flags per state

.RETURNS     number of selected states
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV1_UPDATE_SQADS( struct sqads1_scia *sqads )
{
     register unsigned int ni;

     for ( ni = 0; ni < num_attach_states; ni++ ) {
	  (void) memmove( &sqads[ni], &sqads[indx_attach_states[ni]],
			  sizeof( struct sqads1_scia ) );
     }
     return num_attach_states;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_UPDATE_LADS
.PURPOSE     return selected LADS records with MDS attach
.INPUT/OUTPUT
  call as   num_lads_out = SCIA_LV1_UPDATE_LADS( lads );
 in/output:
            struct lads_scia *lads    :  summary of quality flags per state

.RETURNS     number of selected states
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV1_UPDATE_LADS( struct lads_scia *lads )
{
     register unsigned int ni;

     for ( ni = 0; ni < num_attach_states; ni++ ) {
	  (void) memmove( &lads[ni], &lads[indx_attach_states[ni]],
			  sizeof( struct lads_scia ) );
     }
     return num_attach_states;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_UPDATE_STATE
.PURPOSE     return selected STATE records with MDS attach
.INPUT/OUTPUT
  call as   num_state_out = SCIA_LV1_WR_STATE( state );
 in/output:
            struct state1_scia *state :  summary of quality flags per state

.RETURNS     number of selected states
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV1_UPDATE_STATE( struct state1_scia *state )
{
     register unsigned int ni;

     for ( ni = 0; ni < num_attach_states; ni++ ) {
	  (void) memmove( &state[ni], &state[indx_attach_states[ni]],
			  sizeof( struct state1_scia ) );
     }
     return num_attach_states;
}

