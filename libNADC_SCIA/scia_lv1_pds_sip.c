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

.IDENTifer   SCIA_LV1_PDS_SIP
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 data
.LANGUAGE    ANSI C
.PURPOSE     read/write Static Instrument Parameters
.COMMENTS    contains SCIA_LV1_RD_SIP and SCIA_LV1_WR_SIP
.ENVIRONment None
.VERSION      4.0   11-Oct-2005 use direct write, add usage of SCIA_LV1_ADD_DSD
              3.0   14-Apr-2005 added routine to write SIP-struct to file, RvH
              2.2   22-Mar-2002	test number of DSD; can be zero, RvH
              2.1   06-Mar-2002	added parameter: level_2_smr, RvH 
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              1.1   04-Oct-2001	changed input/output, RvH 
              1.0   18-Dec-2000   Created by R. M. van Hees
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
void Sun2Intel_SIP( struct sip_scia *sip )
{
     register int ni;

     sip->pmd_sat_limit = byte_swap_u16( sip->pmd_sat_limit );

     sip->startpix_6 = byte_swap_16( sip->startpix_6 );
     sip->startpix_8 = byte_swap_16( sip->startpix_8 );

     IEEE_Swap__FLT( &sip->alpha0_asm );
     IEEE_Swap__FLT( &sip->alpha0_esm );
     IEEE_Swap__FLT( &sip->ppg_error );
     IEEE_Swap__FLT( &sip->stray_error );
     IEEE_Swap__FLT( &sip->h_toa );
     IEEE_Swap__FLT( &sip->lambda_end_gdf );
     for ( ni = 0; ni < SCIENCE_CHANNELS; ni++ ) {
	  sip->sat_level[ni] = byte_swap_u16( sip->sat_level[ni] );
	  IEEE_Swap__FLT( &sip->electrons_bu[ni] );
     }
     IEEE_Swap__FLT( &sip->lc_stray_indx[0] );
     IEEE_Swap__FLT( &sip->lc_stray_indx[1] );
     for ( ni = 0; ni < sip->ds_n_phases; ni++ ) 
	  IEEE_Swap__FLT( &sip->ds_phase_boundaries[ni] );
     for ( ni = 0; ni < sip->sp_n_phases; ni++ ) 
	  IEEE_Swap__FLT( &sip->sp_phase_boundaries[ni] );
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_SIP
.PURPOSE     read Static Instrument Parameters
.INPUT/OUTPUT
  call as   nr_dsr = SCIA_LV1_RD_SIP( fd, num_dsd, dsd, &sip );
     input:  
            FILE   *fd            :  (open) stream pointer
	    unsigned int num_dsd  :  number of DSDs
	    struct dsd_envi dsd   :  structure for the DSDs
    output:  
            struct sip_scia *sip  :  structure for the SIP

.RETURNS     number of data set records read (unsigned int)
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV1_RD_SIP( FILE *fd, unsigned int num_dsd, 
			      const struct dsd_envi *dsd,
			      struct sip_scia *sip )
{
     const char prognm[]   = "SCIA_LV1_RD_SIP";

     char         *sip_pntr, *sip_char = NULL;
     size_t       dsr_size, nr_byte;
     unsigned int indx_dsd;

     const char dsd_name[] = "INSTRUMENT_PARAMS";
/*
 * get index to data set descriptor
 */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL ) {
	  NADC_ERROR( prognm, NADC_ERR_PDS_RD, dsd_name );
	  return 0;
     }
     if ( dsd[indx_dsd].num_dsr == 0 ) return 0;
/*
 * allocate memory to temporary store data for output structure
 */
     dsr_size = (size_t) dsd[indx_dsd].dsr_size;
     if ( (sip_char = (char *) malloc( dsr_size )) == NULL ) {
	  NADC_ERROR( prognm, NADC_ERR_ALLOC, "sip_char" );
	  return 0;
     }
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
     if ( fread( sip_char, dsr_size, 1, fd ) != 1 ) {
	  free( sip_char );
	  NADC_ERROR( prognm, NADC_ERR_PDS_RD, "" );
	  return 0;
     }
     sip_pntr = sip_char;
/*
 * read data buffer to SIP structure
 */
/* field 01 */ 
     (void) memcpy( &sip->n_lc_min, sip_pntr, ENVI_UCHAR );
     sip_pntr += ENVI_UCHAR;
/* field 02 */ 
     (void) memcpy( &sip->ds_n_phases, sip_pntr, ENVI_UCHAR );
     sip_pntr += ENVI_UCHAR;
/* field 03 */
     if ( (int)(sip->ds_n_phases)+1 > MaxBoundariesSIP ) {
	  NADC_ERROR( prognm, NADC_ERR_WARN, 
		      "array ds_phase_boundaries too small" );
     }
     nr_byte = MaxBoundariesSIP * ENVI_FLOAT;
     (void) memcpy( sip->ds_phase_boundaries, sip_pntr, nr_byte );
     sip_pntr += nr_byte;
/* field 04 */ 
     (void) memcpy( sip->lc_stray_indx, sip_pntr, 2 * ENVI_FLOAT );
     sip_pntr += 2 * ENVI_FLOAT;
/* field 05 */ 
     (void) memcpy( &sip->lc_harm_order, sip_pntr, ENVI_UCHAR );
     sip_pntr += ENVI_UCHAR;
/* field 06 */ 
     (void) memcpy( &sip->ds_poly_order, sip_pntr, ENVI_UCHAR );
     sip_pntr += ENVI_UCHAR;
/* field 07 */ 
     nr_byte = 4 * IR_CHANNELS * ENVI_CHAR;
     (void) memcpy( sip->do_var_lc_cha, sip_pntr, nr_byte );
     sip->do_var_lc_cha[nr_byte] = '\0';
     sip_pntr += nr_byte;
/* field 08 */ 
     nr_byte = 4 * SCIENCE_CHANNELS * ENVI_CHAR;
     (void) memcpy( sip->do_stray_lc_cha, sip_pntr, nr_byte );
     sip->do_stray_lc_cha[nr_byte] = '\0';
     sip_pntr += nr_byte;
/* field 09 */ 
     nr_byte = 4 * IR_PMD_NUMBER * ENVI_CHAR;
     (void) memcpy( sip->do_var_lc_pmd, sip_pntr, nr_byte );
     sip->do_var_lc_pmd[nr_byte] = '\0';
     sip_pntr += nr_byte;
/* field 10 */ 
     nr_byte = 4 * PMD_NUMBER * ENVI_CHAR;
     (void) memcpy( sip->do_stray_lc_pmd, sip_pntr, nr_byte );
     sip->do_stray_lc_pmd[nr_byte] = '\0';
     sip_pntr += nr_byte;
/* field 11 */ 
     nr_byte = (size_t) SCIENCE_CHANNELS * ENVI_FLOAT;
     (void) memcpy( sip->electrons_bu, sip_pntr, nr_byte );
     sip_pntr += nr_byte;
/* field 12 */ 
     (void) memcpy( &sip->ppg_error, sip_pntr, ENVI_FLOAT );
     sip_pntr += ENVI_FLOAT;
/* field 13 */ 
     (void) memcpy( &sip->stray_error, sip_pntr, ENVI_FLOAT );
     sip_pntr += ENVI_FLOAT;
/* field 14 */ 
     (void) memcpy( &sip->sp_n_phases, sip_pntr, ENVI_UCHAR );
     sip_pntr += ENVI_UCHAR;
/* field 15 */ 
     if ( (int)(sip->sp_n_phases)+1 > MaxBoundariesSIP ) {
	  NADC_ERROR( prognm, NADC_ERR_WARN, 
		      "array sp_phase_boundaries too small" );
     }
     nr_byte = MaxBoundariesSIP * ENVI_FLOAT;
     (void) memcpy( sip->sp_phase_boundaries, sip_pntr, nr_byte );
     sip_pntr += nr_byte;
/* field 16 */ 
     (void) memcpy( &sip->startpix_6, sip_pntr, ENVI_SHORT );
     sip_pntr += ENVI_SHORT;
/* field 17 */ 
     (void) memcpy( &sip->startpix_8, sip_pntr, ENVI_SHORT );
     sip_pntr += ENVI_SHORT;
/* field 18 */ 
     (void) memcpy( &sip->h_toa, sip_pntr, ENVI_FLOAT );
     sip_pntr += ENVI_FLOAT;
/* field 19 */ 
     (void) memcpy( &sip->lambda_end_gdf, sip_pntr, ENVI_FLOAT );
     sip_pntr += ENVI_FLOAT;
/* field 20 */ 
     nr_byte = (size_t) sip->sp_n_phases * ENVI_CHAR;
     (void) memcpy( sip->do_pol_point, sip_pntr, nr_byte );
     sip->do_pol_point[nr_byte] = '\0';
     sip_pntr += nr_byte;
/* field 21 */ 
     nr_byte = (size_t) SCIENCE_CHANNELS * ENVI_USHRT;
     (void) memcpy( sip->sat_level, sip_pntr, nr_byte );
     sip_pntr += nr_byte;
/* field 22 */ 
     (void) memcpy( &sip->pmd_sat_limit, sip_pntr, ENVI_USHRT );
     sip_pntr += ENVI_USHRT;
/* field 23 */ 
     (void) memcpy( sip->do_use_limb_dark, sip_pntr, ENVI_CHAR );
     sip->do_use_limb_dark[1] = '\0';
     sip_pntr += ENVI_CHAR;
/* field 24 */ 
     nr_byte = (size_t) SCIENCE_CHANNELS * ENVI_CHAR;
     (void) memcpy( sip->do_pixelwise, sip_pntr, nr_byte );
     sip->do_pixelwise[nr_byte] = '\0';
     sip_pntr += nr_byte;
/* field 25 */ 
     (void) memcpy( &sip->alpha0_asm, sip_pntr, ENVI_FLOAT );
     sip_pntr += ENVI_FLOAT;
/* field 26 */ 
     (void) memcpy( &sip->alpha0_esm, sip_pntr, ENVI_FLOAT );
     sip_pntr += ENVI_FLOAT;
/* field 27 */ 
     nr_byte = 5 * SCIENCE_CHANNELS * ENVI_CHAR;
     (void) memcpy( sip->do_fraunhofer, sip_pntr, nr_byte );
     sip->do_fraunhofer[nr_byte] = '\0';
     sip_pntr += nr_byte;
/* field 28 */ 
     nr_byte = 3 * SCIENCE_CHANNELS * ENVI_CHAR;
     (void) memcpy( sip->do_etalon, sip_pntr, nr_byte );
     sip->do_etalon[nr_byte] = '\0';
     sip_pntr += nr_byte;
/* field 29 */ 
     nr_byte = PMD_NUMBER * ENVI_CHAR;
     (void) memcpy( sip->do_ib_sd_etn, sip_pntr, nr_byte );
     sip->do_ib_sd_etn[nr_byte] = '\0';
     sip_pntr += nr_byte;
/* field 30 */ 
     (void) memcpy( sip->do_ib_oc_etn, sip_pntr, nr_byte );
     sip->do_ib_oc_etn[nr_byte] = '\0';
     sip_pntr += nr_byte;
/* field 31 */
     nr_byte = (size_t) SCIENCE_CHANNELS * ENVI_UCHAR;
     (void) memcpy( sip->level_2_smr, sip_pntr, nr_byte );
     sip_pntr += nr_byte;
/*
 * check if we read the whole DSR
 */
     if ( (size_t)(sip_pntr - sip_char) != dsr_size ) {
	  free( sip_char );
	  NADC_ERROR( prognm, NADC_ERR_PDS_SIZE, dsd_name );
	  return 0;
     }
     sip_pntr = NULL;
     free( sip_char );
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_SIP( sip );
#endif
/*
 * set return values
 */
     return 1;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_SIP
.PURPOSE     write Static Instrument Parameters
.INPUT/OUTPUT
  call as   SCIA_LV1_RD_SIP( fd, num_sip, sip );
     input:  
            FILE   *fd            :  (open) stream pointer
	    unsigned int num_sip  :  number of SIP records
            struct sip_scia sip   :  structure for the SIP

.RETURNS     nothing
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1_WR_SIP( FILE *fd, unsigned int num_sip, 
		      const struct sip_scia sip_in )
{
     const char prognm[] = "SCIA_LV1_WR_SIP";

     size_t nr_byte;

     struct sip_scia sip;

     struct dsd_envi dsd = {
          "INSTRUMENT_PARAMS", "G",
          "                                                              ",
          0u, 0u, 0u, 0
     };

     if ( num_sip == 0u ) {
	  (void) strcpy( dsd.flname, "NOT USED" );
	  SCIA_LV1_ADD_DSD( &dsd );
	  return;
     }
/*
 * write SIP structure to data buffer
 */
     (void) memcpy( &sip, &sip_in, sizeof( struct sip_scia ) );
#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_SIP( &sip );
#endif
/*
 * write SIP structure to data buffer
 */
/* field 01 */
     if ( fwrite(&sip.n_lc_min, ENVI_UCHAR, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_UCHAR;
/* field 02 */ 
     if ( fwrite(&sip.ds_n_phases, ENVI_UCHAR, 1, fd  ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_UCHAR;
/* field 03 */ 
     nr_byte = ((size_t) (sip.ds_n_phases)+1) * ENVI_FLOAT;
     if ( fwrite(sip.ds_phase_boundaries, nr_byte, 1, fd  ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
/* field 04 */ 
     if ( fwrite(sip.lc_stray_indx, 2 * ENVI_FLOAT, 1, fd  ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += 2 * ENVI_FLOAT;
/* field 05 */ 
     if ( fwrite(&sip.lc_harm_order, ENVI_UCHAR, 1, fd  ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_UCHAR;
/* field 06 */ 
     if ( fwrite(&sip.ds_poly_order, ENVI_UCHAR, 1, fd  ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_UCHAR;
/* field 07 */ 
     nr_byte = 4 * IR_CHANNELS * ENVI_CHAR;
     if ( fwrite(sip.do_var_lc_cha, nr_byte, 1, fd  ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
/* field 08 */ 
     nr_byte = 4 * SCIENCE_CHANNELS * ENVI_CHAR;
     if ( fwrite(sip.do_stray_lc_cha, nr_byte, 1, fd  ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
/* field 09 */ 
     nr_byte = 4 * IR_PMD_NUMBER * ENVI_CHAR;
     if ( fwrite(sip.do_var_lc_pmd, nr_byte, 1, fd  ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
/* field 10 */ 
     nr_byte = 4 * PMD_NUMBER * ENVI_CHAR;
     if ( fwrite(sip.do_stray_lc_pmd, nr_byte, 1, fd  ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
/* field 11 */ 
     nr_byte = (size_t) SCIENCE_CHANNELS * ENVI_FLOAT;
     if ( fwrite(sip.electrons_bu, nr_byte, 1, fd  ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
/* field 12 */ 
     if ( fwrite(&sip.ppg_error, ENVI_FLOAT, 1, fd  ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_FLOAT;
/* field 13 */ 
     if ( fwrite(&sip.stray_error, ENVI_FLOAT, 1, fd  ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_FLOAT;
/* field 14 */ 
     if ( fwrite(&sip.sp_n_phases, ENVI_UCHAR, 1, fd  ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_UCHAR;
/* field 15 */ 
     nr_byte = ((size_t) (sip.sp_n_phases)+1) * ENVI_FLOAT;
     if ( fwrite(sip.sp_phase_boundaries, nr_byte, 1, fd  ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
/* field 16 */ 
     if ( fwrite(&sip.startpix_6, ENVI_SHORT, 1, fd  ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_SHORT;
/* field 17 */ 
     if ( fwrite(&sip.startpix_8, ENVI_SHORT, 1, fd  ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_SHORT;
/* field 18 */ 
     if ( fwrite(&sip.h_toa, ENVI_FLOAT, 1, fd  ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_FLOAT;
/* field 19 */ 
     if ( fwrite(&sip.lambda_end_gdf, ENVI_FLOAT, 1, fd  ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_FLOAT;
/* field 20 */ 
     nr_byte = (size_t) sip.sp_n_phases * ENVI_CHAR;
     if ( fwrite(sip.do_pol_point, nr_byte, 1, fd  ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
/* field 21 */ 
     nr_byte = (size_t) SCIENCE_CHANNELS * ENVI_USHRT;
     if ( fwrite(sip.sat_level, nr_byte, 1, fd  ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
/* field 22 */ 
     if ( fwrite(&sip.pmd_sat_limit, ENVI_USHRT, 1, fd  ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_USHRT;
/* field 23 */ 
     if ( fwrite(sip.do_use_limb_dark, ENVI_CHAR, 1, fd  ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_CHAR;
/* field 24 */ 
     nr_byte = (size_t) SCIENCE_CHANNELS * ENVI_CHAR;
     if ( fwrite(sip.do_pixelwise, nr_byte, 1, fd  ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
/* field 25 */ 
     if ( fwrite(&sip.alpha0_asm, ENVI_FLOAT, 1, fd  ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_FLOAT;
/* field 26 */ 
     if ( fwrite(&sip.alpha0_esm, ENVI_FLOAT, 1, fd  ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += ENVI_FLOAT;
/* field 27 */ 
     nr_byte = 5 * SCIENCE_CHANNELS * ENVI_CHAR;
     if ( fwrite(sip.do_fraunhofer, nr_byte, 1, fd  ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
/* field 28 */ 
     nr_byte = 3 * SCIENCE_CHANNELS * ENVI_CHAR;
     if ( fwrite(sip.do_etalon, nr_byte, 1, fd  ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
/* field 29 */ 
     nr_byte = PMD_NUMBER * ENVI_CHAR;
     if ( fwrite(sip.do_ib_sd_etn, nr_byte, 1, fd  ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
/* field 30 */ 
     if ( fwrite(sip.do_ib_oc_etn, nr_byte, 1, fd  ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
/* field 31 */
     nr_byte = (size_t) SCIENCE_CHANNELS * ENVI_UCHAR;
     if ( fwrite(sip.level_2_smr, nr_byte, 1, fd  ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     dsd.size += nr_byte;
/*
 * update list of written DSD records
 */
     dsd.num_dsr = 1u;
     dsd.dsr_size = (int) dsd.size;
     SCIA_LV1_ADD_DSD( &dsd );
}
