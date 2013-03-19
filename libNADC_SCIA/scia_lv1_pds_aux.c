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

.IDENTifer   SCIA_LV1_RD_AUX
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 data
.LANGUAGE    ANSI C
.PURPOSE     read/write auxiliary Data Packets
.COMMENTS    contains SCIA_LV1_RD_AUX and SCIA_LV1_WR_AUX
.ENVIRONment None
.VERSION      4.0   11-Oct-2005 use direct write, add usage of SCIA_LV1_ADD_DSD
              3.0   14-Apr-2005 added routine to write AUX-struct to file, RvH
              2.2   27-Nov-2002	bug fix: use variable Use_Extern_Alloc, RvH
              2.1   22-Mar-2002	test number of DSD; can be zero, RvH 
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
void Sun2Intel_AUX( struct aux_scia *aux )
{
     aux->mjd.days = byte_swap_32( aux->mjd.days );
     aux->mjd.secnd = byte_swap_u32( aux->mjd.secnd );
     aux->mjd.musec = byte_swap_u32( aux->mjd.musec );
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_AUX
.PURPOSE     read/write auxiliary Data Packets
.INPUT/OUTPUT
  call as   nr_dsr = SCIA_LV1_RD_AUX( fd, num_dsd, dsd, &aux );
     input:
            FILE *fd              :  stream pointer
	    unsigned int num_dsd  :  number of DSDs
	    struct dsd_envi *dsd  :  structure for the DSDs
    output:
            struct aux_scia **aux :  structure for auxiliary data packets

.RETURNS     number of data set records read (unsigned int)
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV1_RD_AUX( FILE *fd, unsigned int num_dsd, 
			      const struct dsd_envi *dsd,
			      struct aux_scia **aux_out )
{
     unsigned int indx_dsd;

     unsigned int nr_dsr = 0;

     struct aux_scia *aux;

     const char prognm[]   = "SCIA_LV1_RD_AUX";
     const char dsd_name[] = "AUXILIARY_PACKETS";
/*
 * get index to data set descriptor
 */
     NADC_ERR_SAVE();
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, dsd_name );
     if ( IS_ERR_STAT_ABSENT || dsd[indx_dsd].num_dsr == 0 ) {
          NADC_ERR_RESTORE();
          aux_out[0] = NULL;
          return 0u;
     }
     if ( ! Use_Extern_Alloc ) {
	  aux_out[0] = (struct aux_scia *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct aux_scia));
     }
     if ( (aux = aux_out[0]) == NULL )  
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "aux" );
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
/*
 * read data set records
 */
     do {
	  if ( fread( &aux->mjd, sizeof(struct mjd_envi), 1, fd ) != 1 )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
	  if ( fread( &aux->flag_mds, ENVI_UCHAR, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
	  SCIA_LV0_RD_LV1_AUX( fd, &aux->mds0 );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "MDS_AUX" );
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_AUX( aux );
#endif
	  aux++;
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
/*
 * set return values
 */
 done:
     return nr_dsr;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_AUX
.PURPOSE     write auxiliary Data Packets
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_AUX( fd, num_aux, aux );
     input:
            FILE *fd              :  stream pointer
	    unsigned int num_aux  :  number of Auxiliary records
            struct aux_scia *aux  :  structure for auxiliary data packets

.RETURNS     nothing
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1_WR_AUX( FILE *fd, unsigned int num_aux,
		      const struct aux_scia *aux_in )
{
     const char prognm[] = "SCIA_LV1_WR_AUX";

     struct aux_scia aux;

     struct dsd_envi dsd = {
          "AUXILIARY_PACKETS", "A",
          "                                                              ",
          0u, 0u, 0u, 0
     };

     if ( num_aux == 0u ) {
	  (void) strcpy( dsd.flname, "NOT USED" );
	  SCIA_LV1_ADD_DSD( &dsd );
	  return;
     }
/*
 * write data set records
 */
     do {
	  (void) memcpy( &aux, aux_in, sizeof( struct aux_scia ) );
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_AUX( &aux );
#endif

	  if ( fwrite( &aux.mjd, sizeof( struct mjd_envi ), 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += sizeof( struct mjd_envi );
	  if ( fwrite( &aux.flag_mds, ENVI_UCHAR, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UCHAR;
	  dsd.size += SCIA_LV0_WR_LV1_AUX( fd, aux.mds0 );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR(prognm,NADC_ERR_FATAL,"SCIA_LV0_WR_LV1_AUX");
	  aux_in++;
     } while ( ++dsd.num_dsr < num_aux );
/*
 * update list of written DSD records
 */
     dsd.dsr_size = (int) (dsd.size / dsd.num_dsr);
     SCIA_LV1_ADD_DSD( &dsd );
}
