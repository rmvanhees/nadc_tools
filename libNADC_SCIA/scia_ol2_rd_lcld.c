/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2011 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_OL2_RD_LCLD
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA offline level 2 data
.LANGUAGE    ANSI C
.PURPOSE     read Limb Cloud Data sets
.INPUT/OUTPUT
  call as   nbyte = SCIA_OL2_RD_LCLD( fd, num_dsd, dsd, &lcld );
     input:
            FILE *fd                :   stream pointer
	    unsigned int num_dsd    :   number of DSDs
	    struct dsd_envi *dsd    :   structure for the DSDs
    output:
            struct lcld_scia **lcld :   Limb Cloud Data sets

.RETURNS     number of data set records read (unsigned int)
.COMMENTS    None
.ENVIRONment None
.VERSION      1.0  10-Jan-2011	created by R. M. van Hees 
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
void Sun2Intel_LCLD( struct lcld_scia *lcld )
{
     register unsigned short ni;

     lcld->mjd.days = byte_swap_32( lcld->mjd.days );
     lcld->mjd.secnd = byte_swap_u32( lcld->mjd.secnd );
     lcld->mjd.musec = byte_swap_u32( lcld->mjd.musec );
     lcld->dsrlen = byte_swap_u32( lcld->dsrlen );
     lcld->intg_time = byte_swap_u16( lcld->intg_time );
     IEEE_Swap__FLT( &lcld->max_value_cir );
     IEEE_Swap__FLT( &lcld->hght_max_value_cir );
     IEEE_Swap__FLT( &lcld->max_value_cir_ice );
     IEEE_Swap__FLT( &lcld->hght_max_value_cir_ice );
     IEEE_Swap__FLT( &lcld->max_value_cir_strato );
     IEEE_Swap__FLT( &lcld->hght_max_value_cir_strato );
     IEEE_Swap__FLT( &lcld->hght_max_value_noctilucent );
     for ( ni = 0; ni < lcld->num_tangent_hghts; ni++ )
	  IEEE_Swap__FLT( &lcld->tangent_hghts[ni] );
     for ( ni = 0; ni < (lcld->num_tangent_hghts * lcld->num_cir); ni++ )
	  IEEE_Swap__FLT( &lcld->cir[ni] );
     for ( ni = 0; ni < lcld->num_limb_para; ni++ )
	  IEEE_Swap__FLT( &lcld->limb_para[ni] );
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned 
int SCIA_OL2_RD_LCLD( FILE *fd, unsigned int num_dsd, 
		      const struct dsd_envi *dsd, 
		      struct lcld_scia **lcld_out )
{
     char         *lcld_pntr, *lcld_char = NULL;
     size_t       adim, dsd_size;
     unsigned int indx_dsd;

     unsigned int nr_dsr = 0;

     struct lcld_scia *lcld;

     const char dsd_name[] = "LIM_CLOUDS";
/*
 * get index to data set descriptor
 */
     NADC_ERR_SAVE();
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL || IS_ERR_STAT_ABSENT )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, dsd_name );
     if ( IS_ERR_STAT_ABSENT || dsd[indx_dsd].num_dsr == 0 ) {
          NADC_ERR_RESTORE();
          lcld_out[0] = NULL;
          return 0u;
     }
     dsd_size = (size_t) dsd[indx_dsd].size;
/*
 * allocate memory to store output structures
 */
     if ( ! Use_Extern_Alloc ) {
	  lcld_out[0] = (struct lcld_scia *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct lcld_scia));
     }
     if ( (lcld = lcld_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "lcld" );
/*
 * allocate memory to temporary store data for output structure
 */
     if ( (lcld_char = (char *) malloc( dsd_size )) == NULL ) 
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "lcld_char" );
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
     if ( fread( lcld_char, dsd_size, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "" );
/*
 * read data buffer to LCLD structure
 */
     lcld_pntr = lcld_char;
     do { 
	  (void) memcpy( &lcld->mjd.days, lcld_pntr, ENVI_INT );                     /* 1 */
	  lcld_pntr += ENVI_INT;
	  (void) memcpy( &lcld->mjd.secnd, lcld_pntr, ENVI_UINT );
	  lcld_pntr += ENVI_UINT;
	  (void) memcpy( &lcld->mjd.musec, lcld_pntr, ENVI_UINT );
	  lcld_pntr += ENVI_UINT;
	  (void) memcpy( &lcld->dsrlen, lcld_pntr, ENVI_UINT );                      /* 2 */
	  lcld_pntr += ENVI_UINT;
	  (void) memcpy( &lcld->quality, lcld_pntr, ENVI_CHAR );                     /* 3 */
	  lcld_pntr += ENVI_CHAR;
	  (void) memcpy( &lcld->intg_time, lcld_pntr, ENVI_USHRT );                  /* 4 */
	  lcld_pntr += ENVI_USHRT;

	  (void) memcpy( &lcld->diag_cloud_algo, lcld_pntr, ENVI_UCHAR );            /* 5 */
	  lcld_pntr += ENVI_UCHAR;
	  (void) memcpy( &lcld->flag_normal_water, lcld_pntr, ENVI_UCHAR );          /* 6 */
	  lcld_pntr += ENVI_UCHAR;
	  (void) memcpy( &lcld->max_value_cir, lcld_pntr, ENVI_FLOAT );              /* 7 */
	  lcld_pntr += ENVI_FLOAT;
	  (void) memcpy( &lcld->hght_max_value_cir, lcld_pntr, ENVI_FLOAT );         /* 8 */
	  lcld_pntr += ENVI_FLOAT;
	  (void) memcpy( &lcld->flag_water_clouds, lcld_pntr, ENVI_UCHAR );          /* 9 */
	  lcld_pntr += ENVI_UCHAR;
	  (void) memcpy( &lcld->flag_ice_clouds, lcld_pntr, ENVI_UCHAR );           /* 10 */
	  lcld_pntr += ENVI_UCHAR;
	  (void) memcpy( &lcld->max_value_cir_ice, lcld_pntr, ENVI_FLOAT );         /* 11 */
	  lcld_pntr += ENVI_FLOAT;
	  (void) memcpy( &lcld->hght_max_value_cir_ice, lcld_pntr, ENVI_FLOAT );    /* 12 */
	  lcld_pntr += ENVI_FLOAT;
	  (void) memcpy( &lcld->hght_index_max_value_ice, lcld_pntr, ENVI_UCHAR );  /* 13 */
	  lcld_pntr += ENVI_UCHAR;
	  (void) memcpy( &lcld->flag_polar_strato_clouds, lcld_pntr, ENVI_UCHAR );  /* 14 */
	  lcld_pntr += ENVI_UCHAR;
	  (void) memcpy( &lcld->max_value_cir_strato, lcld_pntr, ENVI_FLOAT );      /* 15 */
	  lcld_pntr += ENVI_FLOAT;
	  (void) memcpy( &lcld->hght_max_value_cir_strato, lcld_pntr, ENVI_FLOAT ); /* 16 */
	  lcld_pntr += ENVI_FLOAT;
	  (void) memcpy( &lcld->hght_index_max_value_strato, lcld_pntr, ENVI_UCHAR ); /*17*/
	  lcld_pntr += ENVI_UCHAR;
	  (void) memcpy( &lcld->flag_noctilucent_clouds, lcld_pntr, ENVI_UCHAR );     /*18*/
	  lcld_pntr += ENVI_UCHAR;
	  lcld_pntr += ENVI_FLOAT;                                                  /* 19 */
	  (void) memcpy( &lcld->hght_max_value_noctilucent, lcld_pntr, ENVI_FLOAT );  /*20*/
	  lcld_pntr += ENVI_FLOAT;
	  (void) memcpy( &lcld->hght_index_max_value_noctilucent, lcld_pntr, ENVI_UCHAR );
	  lcld_pntr += ENVI_UCHAR;
	  (void) memcpy( &lcld->num_tangent_hghts, lcld_pntr, ENVI_USHRT );         /* 23 */
	  lcld_pntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  lcld->num_tangent_hghts = byte_swap_u16( lcld->num_tangent_hghts );
#endif
	  if ( lcld->num_tangent_hghts > USHRT_ZERO ) {
	       lcld->tangent_hghts = (float *) 
		    malloc( (size_t) lcld->num_tangent_hghts * sizeof(float) );
	       if ( lcld->tangent_hghts == NULL ) 
		    NADC_GOTO_ERROR( NADC_ERR_ALLOC, "tangent_hghts" );
	       (void) memcpy( &lcld->tangent_hghts, lcld_pntr, 
			      (size_t) lcld->num_tangent_hghts * ENVI_FLOAT );
	       lcld_pntr += lcld->num_tangent_hghts * ENVI_FLOAT;
	  }
	  (void) memcpy( &lcld->num_cir, lcld_pntr, ENVI_USHRT );
	  lcld_pntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  lcld->num_cir = byte_swap_u16( lcld->num_cir );
#endif
	  adim = lcld->num_tangent_hghts * lcld->num_cir;
	  if ( adim > USHRT_ZERO ) {
	       lcld->cir = (float *) malloc( adim * sizeof(float) );
	       if ( lcld->cir == NULL ) 
		    NADC_GOTO_ERROR( NADC_ERR_ALLOC, "cir" );
	       (void) memcpy( &lcld->cir, lcld_pntr, adim * ENVI_FLOAT );
	       lcld_pntr += adim * ENVI_FLOAT;
	  }
	  (void) memcpy( &lcld->num_limb_para, lcld_pntr, ENVI_USHRT );
	  lcld_pntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  lcld->num_limb_para = byte_swap_u16( lcld->num_limb_para );
#endif
	  if ( lcld->num_limb_para > USHRT_ZERO ) {
	       lcld->limb_para = (float *) 
		    malloc( (size_t) lcld->num_limb_para * sizeof(float) );
	       if ( lcld->limb_para == NULL ) 
		    NADC_GOTO_ERROR( NADC_ERR_ALLOC, "limb_para" );
	       (void) memcpy( &lcld->limb_para, lcld_pntr, 
			      (size_t) lcld->num_limb_para * ENVI_FLOAT );
	       lcld_pntr += lcld->num_limb_para * ENVI_FLOAT;
	  }
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_LCLD( lcld );
#endif
	  lcld++;
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
/*
 * check if we read the whole DSR
 */
     if ( (unsigned int)(lcld_pntr - lcld_char) != dsd[indx_dsd].size )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_SIZE, dsd_name );
     lcld_pntr = NULL;
/*
 * set return values
 */
 done:
     if ( lcld_char != NULL ) free( lcld_char );

     return nr_dsr;
}

