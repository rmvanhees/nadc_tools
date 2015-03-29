/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2001 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV2_RD_STATE
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 2 data
.LANGUAGE    ANSI C
.PURPOSE     read States of the Product
.INPUT/OUTPUT
  call as   nbyte = SCIA_LV2_RD_STATE( fd, num_dsd, dsd, &state );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi *dsd  :   structure for the DSDs
    output:
            struct state2_scia **state :  states of the product

.RETURNS     number of data set records read (unsigned int)
.COMMENTS    None
.ENVIRONment None
.VERSION      2.1   21-Jan-2002	use of global Use_Extern_Alloc, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              1.1   09-Oct-2001	changed input/output, RvH 
              1.0   13-Sep-2001 created by R. M. van Hees
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
#define _SCIA_LEVEL_2
#include <nadc_scia.h>

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#ifdef _SWAP_TO_LITTLE_ENDIAN
#include <swap_bytes.h>

static
void Sun2Intel_STATE( struct state2_scia *state )
{
     state->mjd.days = byte_swap_32( state->mjd.days );
     state->mjd.secnd = byte_swap_u32( state->mjd.secnd );
     state->mjd.musec = byte_swap_u32( state->mjd.musec );
     state->state_id = byte_swap_u16( state->state_id );
     state->duration = byte_swap_u16( state->duration );
     state->longest_intg_time = byte_swap_u16( state->longest_intg_time );
     state->shortest_intg_time = byte_swap_u16( state->shortest_intg_time );
     state->num_obs_state = byte_swap_u16( state->num_obs_state );
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned 
int SCIA_LV2_RD_STATE( FILE *fd, unsigned int num_dsd, 
		       const struct dsd_envi *dsd,
		       struct state2_scia **state_out )
{
     char         *state_pntr, *state_char = NULL;
     size_t       dsr_size;
     unsigned int indx_dsd;

     unsigned int nr_dsr = 0;

     struct state2_scia *state;

     const char dsd_name[] = "STATES";
/*
 * get index to data set descriptor
 */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, dsd_name );
     if ( dsd[indx_dsd].num_dsr == 0 ) return 0u;
     if ( ! Use_Extern_Alloc ) {
	  state_out[0] = (struct state2_scia *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct state2_scia));
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
	  if ( fread( state_char, dsr_size, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "" );
/*
 * read data buffer to STATE structure
 */
	  (void) memcpy( &state->mjd.days, state_char, ENVI_INT );
	  state_pntr = state_char + ENVI_INT;
	  (void) memcpy( &state->mjd.secnd, state_pntr, ENVI_UINT );
	  state_pntr += ENVI_UINT;
	  (void) memcpy( &state->mjd.musec, state_pntr, ENVI_UINT );
	  state_pntr += ENVI_UINT;
	  (void) memcpy( &state->flag_mds, state_pntr, ENVI_UCHAR );
	  state_pntr += ENVI_UCHAR;
	  (void) memcpy( &state->state_id, state_pntr, ENVI_USHRT );
	  state_pntr += ENVI_USHRT;
	  (void) memcpy( &state->duration, state_pntr, ENVI_USHRT );
	  state_pntr += ENVI_USHRT;
	  (void) memcpy( &state->longest_intg_time, state_pntr, ENVI_USHRT );
	  state_pntr += ENVI_USHRT;
	  (void) memcpy( &state->shortest_intg_time, state_pntr, ENVI_USHRT);
	  state_pntr += ENVI_USHRT;

	  (void) memcpy( &state->num_obs_state, state_pntr, ENVI_USHRT );
	  state_pntr += ENVI_USHRT;
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
     state_pntr = NULL;
/*
 * set return values
 */
 done:
     if ( state_char != NULL ) free( state_char );
     
     return nr_dsr;
}
