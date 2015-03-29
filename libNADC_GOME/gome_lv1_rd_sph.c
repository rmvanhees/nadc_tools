/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 1999 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   GOME_LV1_RD_SPH
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME level 1 data
.LANGUAGE    ANSI C
.PURPOSE     Read the Specific Product Header
.INPUT/OUTPUT
  call as   GOME_LV1_RD_SPH( infl, fsr, &sph );

     input:  
            FILE   *infl          : (open) file descriptor
	    struct fsr1_gome *fsr : structure for the FSR record
    output:  
            struct sph1_gome *sph : structure for the SPH

.RETURNS     nothing: modifies global error status
.COMMENTS    None
.ENVIRONment None
.VERSION      2.1   07-Apr-2003	get SPH-size from FSR-record, RvH
              2.0   11-Nov-2001	moved to the new Error handling routines, RvH
              1.2   17-Feb-1999	conform to ANSI C3.159-1989, RvH 
              1.1   05-Oct-1999 correct handling the number of 
                                   "Input Data References", RvH
              1.0   10-Feb-1999 created by R. M. van Hees
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
#define _GOME_COMMON
#include <nadc_gome.h>

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#ifdef _SWAP_TO_LITTLE_ENDIAN
#include <swap_bytes.h>

static
void Sun2Intel_SPH( struct sph1_gome *sph )
{
     register int ni;

     sph->prod_version = byte_swap_16( sph->prod_version );
     sph->time_orbit = byte_swap_u32( sph->time_orbit );
     sph->time_utc_day = byte_swap_u32( sph->time_utc_day );
     sph->time_utc_ms = byte_swap_u32( sph->time_utc_ms );
     sph->time_counter = byte_swap_u32( sph->time_counter );
     sph->time_period = byte_swap_u32( sph->time_period );
     sph->pmd_entry = byte_swap_16( sph->pmd_entry );
     sph->subset_entry = byte_swap_16( sph->subset_entry );
     sph->intgstat_entry = byte_swap_16( sph->intgstat_entry );
     sph->peltier_entry = byte_swap_16( sph->peltier_entry );
     sph->status2_entry = byte_swap_16( sph->status2_entry );
     for ( ni = 0; ni < (2 * PMD_NUMBER); ni++ )
	  IEEE_Swap__FLT( &sph->pmd_conv[ni] );
     sph->state_utc_day = byte_swap_u32( sph->state_utc_day );
     sph->state_utc_ms = byte_swap_u32( sph->state_utc_ms );
     sph->state_orbit = byte_swap_u32( sph->state_orbit );
     IEEE_Swap__FLT( &sph->state_x );
     IEEE_Swap__FLT( &sph->state_y );
     IEEE_Swap__FLT( &sph->state_z );
     IEEE_Swap__FLT( &sph->state_dx );
     IEEE_Swap__FLT( &sph->state_dy );
     IEEE_Swap__FLT( &sph->state_dz );
     IEEE_Swap__DBL( &sph->att_yaw );
     IEEE_Swap__DBL( &sph->att_pitch );
     IEEE_Swap__DBL( &sph->att_roll );
     IEEE_Swap__DBL( &sph->att_dyaw );
     IEEE_Swap__DBL( &sph->att_dpitch);
     IEEE_Swap__DBL( &sph->att_droll );
     sph->att_flag   = byte_swap_32( sph->att_flag );
     sph->att_stat   = byte_swap_32( sph->att_stat );
     IEEE_Swap__DBL( &sph->julian );
     IEEE_Swap__DBL( &sph->semi_major );
     IEEE_Swap__DBL( &sph->excen );
     IEEE_Swap__DBL( &sph->incl );
     IEEE_Swap__DBL( &sph->right_asc );
     IEEE_Swap__DBL( &sph->perigee );
     IEEE_Swap__DBL( &sph->mn_anom );
}
#endif

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void GOME_LV1_RD_SPH( FILE *infl, const struct fsr1_gome *fsr,
		      struct sph1_gome *sph )
{
     register int np;

     char   *sph_char;
     char   *sph_pntr;

     const long SPH_BYTE_OFFS = LVL1_PIR_LENGTH + LVL1_FSR_LENGTH;
/*
 * allocate memory to store data from SPH-record
 */
     if ( (sph_char = (char *) malloc( fsr->sz_sph )) == NULL ) 
	  NADC_RETURN_ERROR( NADC_ERR_ALLOC, "sph_char" );
/*
 * rewind/read input data file
 */
     (void) fseek( infl, SPH_BYTE_OFFS, SEEK_SET );
     if ( fread( sph_char, (size_t) fsr->sz_sph, 1, infl ) != 1 )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "" );

     (void) memcpy( &sph->nr_inref, sph_char, GOME_SHORT );
#ifdef _SWAP_TO_LITTLE_ENDIAN
     sph->nr_inref = byte_swap_16( sph->nr_inref );
#endif
     sph_pntr = sph_char + GOME_SHORT;
     for ( np = 0; np < sph->nr_inref; np++ ) {
	  (void) memcpy( sph->inref[np], sph_pntr, LVL1_PIR_LENGTH );
	  sph->inref[np][LVL1_PIR_LENGTH] = '\0';
	  sph_pntr += LVL1_PIR_LENGTH;
     }
     if ( sph->nr_inref == 1 ) (void) strcpy( sph->inref[1], "" );
     (void) memcpy( sph->soft_version, sph_pntr, 5 );
     sph->soft_version[5] = '\0';
     sph_pntr += 5;
     (void) memcpy( sph->calib_version, sph_pntr, 5 );
     sph->calib_version[5] = '\0';
     sph_pntr += 5;
     (void) memcpy( &sph->prod_version, sph_pntr, GOME_SHORT );
     sph_pntr += GOME_SHORT;

     (void) memcpy( &sph->time_orbit, sph_pntr, GOME_UINT );
     sph_pntr += GOME_UINT;
     (void) memcpy( &sph->time_utc_day, sph_pntr, GOME_UINT );
     sph_pntr += GOME_UINT;
     (void) memcpy( &sph->time_utc_ms, sph_pntr, GOME_UINT );
     sph_pntr += GOME_UINT;
     (void) memcpy( &sph->time_counter, sph_pntr, GOME_UINT );
     sph_pntr += GOME_UINT;
     (void) memcpy( &sph->time_period, sph_pntr, GOME_UINT );
     sph_pntr += GOME_UINT;

     (void) memcpy( &sph->pmd_entry, sph_pntr, GOME_SHORT );
     sph_pntr += GOME_SHORT;
     (void) memcpy( &sph->subset_entry, sph_pntr, GOME_SHORT );
     sph_pntr += GOME_SHORT;
     (void) memcpy( &sph->intgstat_entry, sph_pntr, GOME_SHORT );
     sph_pntr += GOME_SHORT;
     (void) memcpy( &sph->peltier_entry, sph_pntr, GOME_SHORT );
     sph_pntr += GOME_SHORT;
     (void) memcpy( &sph->status2_entry, sph_pntr, GOME_SHORT );
     sph_pntr += GOME_SHORT;
     (void) memcpy( sph->pmd_conv, sph_pntr, 6 * GOME_FLOAT );
     sph_pntr += 6 * GOME_FLOAT;
     (void) memcpy( &sph->state_utc_day, sph_pntr, GOME_UINT );
     sph_pntr += GOME_UINT;
     (void) memcpy( &sph->state_utc_ms, sph_pntr, GOME_UINT );
     sph_pntr += GOME_UINT;
     (void) memcpy( &sph->state_orbit, sph_pntr, GOME_UINT );
     sph_pntr += GOME_UINT;

     (void) memcpy( &sph->state_x, sph_pntr, GOME_FLOAT );
     sph_pntr += GOME_FLOAT;
     (void) memcpy( &sph->state_y, sph_pntr, GOME_FLOAT );
     sph_pntr += GOME_FLOAT;
     (void) memcpy( &sph->state_z, sph_pntr, GOME_FLOAT );
     sph_pntr += GOME_FLOAT;
     (void) memcpy( &sph->state_dx, sph_pntr, GOME_FLOAT );
     sph_pntr += GOME_FLOAT;
     (void) memcpy( &sph->state_dy, sph_pntr, GOME_FLOAT );
     sph_pntr += GOME_FLOAT;
     (void) memcpy( &sph->state_dz, sph_pntr, GOME_FLOAT );
     sph_pntr += GOME_FLOAT;

     (void) memcpy( &sph->att_yaw, sph_pntr, GOME_DBLE );
     sph_pntr += GOME_DBLE;
     (void) memcpy( &sph->att_pitch, sph_pntr, GOME_DBLE );
     sph_pntr += GOME_DBLE;
     (void) memcpy( &sph->att_roll, sph_pntr, GOME_DBLE );
     sph_pntr += GOME_DBLE;
     (void) memcpy( &sph->att_dyaw, sph_pntr, GOME_DBLE );
     sph_pntr += GOME_DBLE;
     (void) memcpy( &sph->att_dpitch, sph_pntr, GOME_DBLE );
     sph_pntr += GOME_DBLE;
     (void) memcpy( &sph->att_droll, sph_pntr, GOME_DBLE );
     sph_pntr += GOME_DBLE;
     (void) memcpy( &sph->att_flag, sph_pntr, GOME_INT );
     sph_pntr += GOME_INT;
     (void) memcpy( &sph->att_stat, sph_pntr, GOME_INT );
     sph_pntr += GOME_INT;

     (void) memcpy( &sph->julian, sph_pntr, GOME_DBLE );
     sph_pntr += GOME_DBLE;
     (void) memcpy( &sph->semi_major, sph_pntr, GOME_DBLE );
     sph_pntr += GOME_DBLE;
     (void) memcpy( &sph->excen, sph_pntr, GOME_DBLE );
     sph_pntr += GOME_DBLE;
     (void) memcpy( &sph->incl, sph_pntr, GOME_DBLE );
     sph_pntr += GOME_DBLE;
     (void) memcpy( &sph->right_asc, sph_pntr, GOME_DBLE );
     sph_pntr += GOME_DBLE;
     (void) memcpy( &sph->perigee, sph_pntr, GOME_DBLE );
     sph_pntr += GOME_DBLE;
     (void) memcpy( &sph->mn_anom, sph_pntr, GOME_DBLE );
     sph_pntr += GOME_DBLE;
/*
 * check if we read the whole DSR
 */
     if ( sph_pntr - sph_char !=  fsr->sz_sph )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_SIZE, "SPH size" );
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_SPH( sph );
#endif
done:
     free( sph_char );
     return;
}
