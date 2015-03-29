/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2001 - 2015 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV0_IDL
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIAMACHY level 0, IDL
.LANGUAGE    ANSI C
.PURPOSE     IDL wrapper for reading SCIAMACHY level 0 data
.COMMENTS    None
.ENVIRONment None
.VERSION      1.4   16-Mar-2015	fixed for nadc_tools v2.x, RvH
              1.3   12-Oct-2002	consistently return, in case of error, -1, RvH 
              1.2   02-Jul-2002	added more error checking, RvH
              1.1   19-Feb-2002	made program complied with libSCIA, RvH 
              1.0   04-Dec-2001	Created by R. M. van Hees 
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_SOURCE 2

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_0
#include <nadc_idl.h>

/*+++++ Global Variables +++++*/
extern FILE *fd_nadc;

/*+++++ Static Variables +++++*/
static const char err_msg[] = "invalid number of function arguments";

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/

/*+++++++++++++++++++++++++ Main Program or Functions +++++++++++++++*/
int IDL_STDCALL _SCIA_LV0_RD_SPH ( int argc, void *argv[] )
{
     struct mph_envi  mph;
     struct sph0_scia *sph;

     if ( argc != 2 ) NADC_GOTO_ERROR( NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( NADC_ERR_FILE, "No open stream" );

     mph = *(struct mph_envi *) argv[0];
     sph = (struct sph0_scia *) argv[1];

     SCIA_LV0_RD_SPH( fd_nadc, mph, sph );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return 1;
 done:
     return -1;
}

unsigned int IDL_STDCALL _GET_LV0_MDS_INFO ( int argc, void *argv[] )
{
     unsigned int num_info;

     struct dsd_envi  dsd;
     struct mds0_info *info;

     if ( argc != 2 ) NADC_GOTO_ERROR( NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( NADC_ERR_FILE, "No open stream" );

     dsd = *(struct dsd_envi *) argv[0];
     info = (struct mds0_info *) argv[1];

     num_info = GET_SCIA_LV0_MDS_INFO( fd_nadc, &dsd, info );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_FILE_RD, "GET_SCIA_LV0_MDS_INFO" );

     return num_info;
 done:
     return 0;
}


int IDL_STDCALL _SCIA_LV0_RD_MDS_INFO ( int argc, void *argv[] )
{
     unsigned int num_dsd;
     size_t       num_states;
     
     struct dsd_envi    *dsd;
     struct mds0_states *states;

     if ( argc != 3 ) NADC_GOTO_ERROR( NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( NADC_ERR_FILE, "No open stream" );

     num_dsd = *(unsigned int *) argv[0];
     dsd  = (struct dsd_envi *) argv[1];
     states = (struct mds0_states *) argv[2];

     num_states = SCIA_LV0_RD_MDS_INFO( fd_nadc, num_dsd, dsd, &states );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_FILE_RD, "GET_SCIA_LV0_MDS_INFO" );

     return (int) num_states;
 done:
     return -1;
}

static inline
void _UNPACK_LV0_PIXEL_DATA( const struct chan_src *pixel,
			     /*@out@*/ unsigned int *data )
{
     register unsigned short np = 0;

     register unsigned char *cpntr = pixel->data;

     if ( pixel->co_adding == (unsigned char) 1 ) {
          do {
               *data++ = (unsigned int) cpntr[1]
                    + ((unsigned int) cpntr[0] << 8);
               cpntr += 2;
          } while ( ++np < pixel->length );
     } else {
          do {
               *data++ = (unsigned int) cpntr[2]
                    + ((unsigned int) cpntr[1] << 8)
                    + ((unsigned int) cpntr[0] << 16);
               cpntr += 3;
          } while ( ++np < pixel->length );
     }
}

unsigned int IDL_STDCALL _SCIA_LV0_RD_DET ( int argc, void *argv[] )
{
     register unsigned short n_ch, n_cl;

     signed char cbuff;
     unsigned char  chan_mask;
     unsigned int   num_clus;

     register unsigned int nr = 0;
     register unsigned int offs = 0;

     unsigned int num_det;
     unsigned int *data;

     struct mds0_det  *C_det;
     struct mds0_info *info;

     struct IDL_chan_src
     {
	  unsigned char  cluster_id;
	  unsigned char  co_adding;
	  unsigned short sync;
	  unsigned short block_nr;
	  unsigned short start;
	  unsigned short length;
	  IDL_ULONG      pntr_data;            /* IDL uses 32-bit addresses */
     };

     struct IDL_det_src
     {
	  struct chan_hdr hdr;
	  struct IDL_chan_src pixel[LV0_MX_CLUSTERS];

     };

     struct IDL_mds0_det 
     { 
	  unsigned short      bcps;
	  unsigned short      num_chan;
	  int                 orbit_vector[8];
	  struct mjd_envi     isp;
	  struct fep_hdr      fep_hdr;
	  struct packet_hdr   packet_hdr;
	  struct data_hdr     data_hdr;
	  struct pmtc_hdr     pmtc_hdr;
	  struct IDL_det_src  data_src[SCIENCE_CHANNELS];
     } *det;

     if ( argc != 5 ) {
	  NADC_ERROR( NADC_ERR_PARAM, err_msg );
	  return -1;
     }
     if ( fileno( fd_nadc ) == -1 ) {
	  NADC_ERROR( NADC_ERR_FILE, "No open stream" );
	  return -1;
     }
     info  = (struct mds0_info *) argv[0];
     num_det   = *(unsigned int *) argv[1];
     cbuff = *(char *) argv[2];
     chan_mask = (unsigned char) cbuff;
     det   = (struct IDL_mds0_det *) argv[3];
     data  = (unsigned int *) argv[4];

     /* we need SCIA_LV0_RD_DET to do dynamic memory allocation */
     nadc_stat = NADC_STAT_SUCCESS;
     for ( nr = 0; nr < num_det; nr++ ) {
	  Use_Extern_Alloc = FALSE;

	  /* read one detector MDS into memory */
	  SCIA_LV0_RD_DET( fd_nadc, info+nr, 1, chan_mask, &C_det );
	  Use_Extern_Alloc = TRUE;
	  if ( IS_ERR_STAT_FATAL ) return -1;

	  /* copy C-struct to IDL-struct */
	  det[nr].bcps = C_det->bcps;
	  det[nr].num_chan = C_det->num_chan; 
	  (void) memcpy( det[nr].orbit_vector, C_det->orbit_vector, 
			 8 * sizeof( int  ) );
	  (void) memcpy( &det[nr].isp, &C_det->isp, 
			 sizeof( struct mjd_envi ) );
	  (void) memcpy( &det[nr].fep_hdr, &C_det->fep_hdr, 
			 sizeof( struct fep_hdr ) );
	  (void) memcpy( &det[nr].packet_hdr, &C_det->packet_hdr, 
			 sizeof( struct packet_hdr ) );
	  (void) memcpy( &det[nr].data_hdr, &C_det->data_hdr, 
			 sizeof( struct data_hdr ) );
	  (void) memcpy( &det[nr].pmtc_hdr, &C_det->pmtc_hdr, 
			 sizeof( struct pmtc_hdr ) );

	  for ( n_ch = 0; n_ch < C_det->num_chan; n_ch++ ) {
	       num_clus = C_det->data_src[n_ch].hdr.channel.field.clusters ;

	       (void) memcpy( &det[nr].data_src[n_ch].hdr,
                              &C_det->data_src[n_ch].hdr,
			      sizeof( struct chan_hdr ) );

	       for ( n_cl = 0; n_cl < num_clus; n_cl++ ) {
		    det[nr].data_src[n_ch].pixel[n_cl].cluster_id =
			 C_det->data_src[n_ch].pixel[n_cl].cluster_id;
		    det[nr].data_src[n_ch].pixel[n_cl].co_adding =
			 C_det->data_src[n_ch].pixel[n_cl].co_adding;
		    det[nr].data_src[n_ch].pixel[n_cl].sync =
			 C_det->data_src[n_ch].pixel[n_cl].sync;
		    det[nr].data_src[n_ch].pixel[n_cl].block_nr =
			 C_det->data_src[n_ch].pixel[n_cl].block_nr;
		    det[nr].data_src[n_ch].pixel[n_cl].start =
			 C_det->data_src[n_ch].pixel[n_cl].start;
		    det[nr].data_src[n_ch].pixel[n_cl].length =
			 C_det->data_src[n_ch].pixel[n_cl].length;

		    _UNPACK_LV0_PIXEL_DATA( &C_det->data_src[n_ch].pixel[n_cl],
					    data+offs );
		    offs += C_det->data_src[n_ch].pixel[n_cl].length;
	       }
	  }

	  /* release memory */
	  SCIA_LV0_FREE_MDS_DET( 1, C_det );
     }
     return num_det;
}

unsigned int IDL_STDCALL _SCIA_LV0_RD_AUX ( int argc, void *argv[] )
{
     unsigned int num_info, num_aux;

     struct mds0_aux  *aux;	
     struct mds0_info *info;

     if ( argc != 3 ) NADC_GOTO_ERROR( NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( NADC_ERR_FILE, "No open stream" );

     info  = (struct mds0_info *) argv[0];
     num_info  = *(unsigned int *) argv[1];
     aux   = (struct mds0_aux *) argv[2];

     num_aux = SCIA_LV0_RD_AUX( fd_nadc, info, num_info, &aux );
     if ( IS_ERR_STAT_FATAL ) return 0;

     return num_aux;
 done:
     return 0;
}

unsigned int IDL_STDCALL _SCIA_LV0_RD_PMD ( int argc, void *argv[] )
{
     unsigned int num_info, num_pmd;

     struct mds0_info *info;
     struct mds0_pmd  *pmd;

     if ( argc != 3 ) NADC_GOTO_ERROR( NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( NADC_ERR_FILE, "No open stream" );

     info  = (struct mds0_info *) argv[0];
     num_info  = *(unsigned int *) argv[1];
     pmd   = (struct mds0_pmd *) argv[2];

     num_pmd = SCIA_LV0_RD_PMD( fd_nadc, info, num_info, &pmd );
     if ( IS_ERR_STAT_FATAL ) return 0;

     return num_pmd;
 done:
     return 0;
}
