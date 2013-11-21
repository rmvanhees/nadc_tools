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

.IDENTifer   SCIA_LV0_WR_MDS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 0 data
.LANGUAGE    ANSI C
.PURPOSE     write SCIAMACHY level 0 Measurement Data Sets 
.COMMENTS    contains SCIA_LV0_WR_AUX, SCIA_LV0_WR_DET, SCIA_LV0_WR_PMD,
		      SCIA_LV0_WR_LV1_AUX, SCIA_LV0_WR_LV1_PMD
             Documentation:
	      - Envisat-1 Product Specifications
	        Volume 6: Level 0 Product Specification
		Ref: PO-RS-MDA-GS-2009
	      - Measurement Data Definition and Format description
	        for SCIAMACHY
		Ref: PO-ID-DOR-SY-0032

.ENVIRONment none
.EXTERNALs   ENVI_GET_DSD_INDEX 
.VERSION      2.0   11-Oct-2005 modified several function declarations, every
                                module returns the number of bytes written, RvH
              1.0   18-Apr-2005 created by R. M. van Hees
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
#define _SCIA_LEVEL_0
#include <nadc_scia.h>

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#ifdef _SWAP_TO_LITTLE_ENDIAN
#include "swap_lv0_mds.inc"
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_WR_MDS_ANNOTATION
.PURPOSE     write level 0 MDS  annotations
.INPUT/OUTPUT
  call as   SCIA_LV0_WR_MDS_ANNOTATION( fd, isp, fep_hdr );
     input:  
            FILE *fd                : stream pointer
	    struct mjd_envi isp     : ISP sensing time
	    struct fep_hdr  fep_hdr : FEP annotations

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
void SCIA_LV0_WR_MDS_ANNOTATION( FILE *fd, 
                                 const struct mjd_envi isp_in,
				 const struct fep_hdr  fep_hdr_in )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd@*/
{
     const char prognm[] = "SCIA_LV0_WR_MDS_ANNOTATION";

     register char *hdr_pntr;

     char hdr_char[LV0_ANNOTATION_LENGTH];

     const char SpareFEP[3] = "  ";

     struct mjd_envi isp;
     struct fep_hdr  fep_hdr;

     (void) memcpy( &isp, &isp_in, sizeof( struct mjd_envi ));
     (void) memcpy( &fep_hdr, &fep_hdr_in, sizeof( struct fep_hdr ));
#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_MDS_ANNOTATION( &isp, &fep_hdr );
#endif
/*
 * write ISP sensing time
 */
     hdr_pntr = hdr_char;
     (void) memcpy( hdr_pntr, &isp.days, ENVI_INT );
     hdr_pntr += ENVI_INT;
     (void) memcpy( hdr_pntr, &isp.secnd, ENVI_UINT );
     hdr_pntr += ENVI_UINT;
     (void) memcpy( hdr_pntr, &isp.musec, ENVI_UINT );
     hdr_pntr += ENVI_UINT;
/*
 * write Annotation (FEP)
 */
     (void) memcpy( hdr_pntr, &fep_hdr.gsrt.days, ENVI_INT );
     hdr_pntr += ENVI_INT;
     (void) memcpy( hdr_pntr, &fep_hdr.gsrt.secnd, ENVI_UINT );
     hdr_pntr += ENVI_UINT;
     (void) memcpy( hdr_pntr, &fep_hdr.gsrt.musec, ENVI_UINT );
     hdr_pntr += ENVI_UINT;
     (void) memcpy( hdr_pntr, &fep_hdr.isp_length, ENVI_USHRT );
     hdr_pntr += ENVI_USHRT;
     (void) memcpy( hdr_pntr, &fep_hdr.crc_errs, ENVI_USHRT );
     hdr_pntr += ENVI_USHRT;
     (void) memcpy( hdr_pntr, &fep_hdr.rs_errs, ENVI_USHRT );
     hdr_pntr += ENVI_USHRT;
     (void) nadc_strlcpy( hdr_pntr, SpareFEP, 2 );
/*
 * write header data
 */
     if ( fwrite( hdr_char, LV0_ANNOTATION_LENGTH, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     return;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_WR_MDS_PACKET_HDR
.PURPOSE     write level 0 MDS ISP Packet header
.INPUT/OUTPUT
  call as   nr_byte = SCIA_LV0_WR_MDS_PACKET_HDR( fd, packet_hdr );
     input:  
            FILE *fd                     : stream pointer
	    struct packet_hdr packet_hdr : Packet header

.RETURNS     number of bytes written
.COMMENTS    static function
-------------------------*/
static
unsigned int SCIA_LV0_WR_MDS_PACKET_HDR( FILE *fd, 
				        const struct packet_hdr packet_hdr_in )
       /*@globals  errno;@*/
       /*@modifies errno, fd@*/
{
     const char prognm[] = "SCIA_LV0_WR_MDS_PACKET_HDR";

     struct packet_hdr packet_hdr;

     (void) memcpy( &packet_hdr, &packet_hdr_in, sizeof( struct packet_hdr ));
#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_MDS_PACKET_HDR( &packet_hdr );
#endif
     if ( fwrite( &packet_hdr.api.two_byte, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     if ( fwrite( &packet_hdr.seq_cntrl, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     if ( fwrite( &packet_hdr.length, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, "" );
 done:
     return (unsigned int) (3 * ENVI_USHRT);
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_WR_MDS_DATA_HDR
.PURPOSE     write Data Field Header of the Packet Data Field [ISP]
.INPUT/OUTPUT
  call as   nr_byte = SCIA_LV0_WR_MDS_DATA_HDR( fd, data_hdr );
     input:  
            FILE *fd                 : stream pointer
            struct data_hdr data_hdr : Data Field Header

.RETURNS     number of bytes written
.COMMENTS    static function
             The Data Field Header for each of the three packets (i.e.
	     Detector, Auxiliary and PMD) contains standard information.
-------------------------*/
static
unsigned int SCIA_LV0_WR_MDS_DATA_HDR( FILE *fd, 
				       const struct data_hdr data_hdr_in )
       /*@globals  errno;@*/
       /*@modifies errno, fd@*/
{
     const char prognm[] = "SCIA_LV0_WR_MDS_DATA_HDR";

     unsigned int nr_byte = 0u;

     struct data_hdr data_hdr;

     (void) memcpy( &data_hdr, &data_hdr_in, sizeof( struct data_hdr ));
#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_MDS_DATA_HDR( &data_hdr );
#endif
     if ( fwrite( &data_hdr.length, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     nr_byte += ENVI_USHRT;
     if ( fwrite( &data_hdr.category, ENVI_UCHAR, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     nr_byte += ENVI_UCHAR;
     if ( fwrite( &data_hdr.state_id, ENVI_UCHAR, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     nr_byte += ENVI_UCHAR;
     if ( fwrite( &data_hdr.on_board_time, ENVI_UINT, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     nr_byte += ENVI_UINT;
     if ( fwrite( &data_hdr.rdv.two_byte, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     nr_byte += ENVI_USHRT;
     if ( fwrite( &data_hdr.id.two_byte, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     nr_byte += ENVI_USHRT;
 done:
     return nr_byte;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_WR_MDS_PMTC_HDR
.PURPOSE     write PMTC settings from the ISP Data Field Header
.INPUT/OUTPUT
  call as   nr_byte = SCIA_LV0_WR_MDS_PMTC_HDR( fd, pmtc_hdr );
     input:  
            FILE *fd                 : stream pointer
            struct pmtc_hdr pmtc_hdr : PMTC settings

.RETURNS     number of bytes written (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
unsigned int SCIA_LV0_WR_MDS_PMTC_HDR( FILE *fd, 
				       const struct pmtc_hdr pmtc_hdr_in )
       /*@globals  errno;@*/
       /*@modifies errno, fd@*/
{
     const char prognm[] = "SCIA_LV0_WR_MDS_PMTC_HDR";

     unsigned int nr_byte = 0u;

     struct pmtc_hdr pmtc_hdr;

     (void) memcpy( &pmtc_hdr, &pmtc_hdr_in, sizeof( struct pmtc_hdr ));
#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_MDS_PMTC_HDR( &pmtc_hdr );
#endif
     if ( fwrite( &pmtc_hdr.pmtc_1.two_byte, ENVI_USHRT, 1, fd  ) != 1 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     nr_byte += ENVI_USHRT;
     if ( fwrite( &pmtc_hdr.scanner_mode, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     nr_byte += ENVI_USHRT;
     if ( fwrite( &pmtc_hdr.az_param.four_byte, ENVI_UINT, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     nr_byte += ENVI_UINT;
     if ( fwrite( &pmtc_hdr.elv_param.four_byte, ENVI_UINT, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     nr_byte += ENVI_UINT;
     if ( fwrite( &pmtc_hdr.factor, 6, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     nr_byte += 6;
 done:
     return nr_byte;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_WR_PMTC_FRAME
.PURPOSE     write source data of Auxilirary MDS
.INPUT/OUTPUT
  call as   nr_bye = SCIA_LV0_WR_PMTC_FRAME( fd, pmtc_frame );
     input:  
            FILE *fd                : stream pointer
    output:  
            struct pmtc_frame *pmtc_frame : PMTC frame

.RETURNS     number of bytes written (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
unsigned int SCIA_LV0_WR_MDS_PMTC_FRAME( FILE *fd, 
				      const struct pmtc_frame pmtc_frame_in )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd@*/
{
     register unsigned short nb = 0;

     char *src_pntr;
     char src_char[AUX_DATA_SRC_LENGTH];

     unsigned long long llbuff;

     struct pmtc_frame pmtc_frame;

     (void) memcpy( &pmtc_frame, &pmtc_frame_in, sizeof(struct pmtc_frame) );
#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_MDS_PMTC_FRAME( &pmtc_frame );
#endif
     src_pntr = src_char;
     do {
	  (void) memcpy( src_pntr, &pmtc_frame.bcp[nb].sync, ENVI_USHRT );
	  src_pntr += ENVI_USHRT;
	  (void) memcpy( src_pntr, &pmtc_frame.bcp[nb].bcps, ENVI_USHRT );
	  src_pntr += ENVI_USHRT;
	  (void) memcpy( src_pntr, &pmtc_frame.bcp[nb].flags.two_byte, 
			 ENVI_USHRT );
	  src_pntr += ENVI_USHRT;
/*
 * write azi_encode and ele_encode to a 64-bit integer, and write only 48-bits
 */
	  llbuff = pmtc_frame.bcp[nb].azi_encode_cntr;
	  llbuff <<= 20;
	  llbuff |= pmtc_frame.bcp[nb].ele_encode_cntr;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  llbuff = byte_swap_u64( llbuff );
#endif
	  llbuff >>= 8;
	  (void) memcpy( src_pntr, &llbuff, sizeof(unsigned long long) );
	  src_pntr += (ENVI_USHRT + ENVI_UINT);

	  (void) memcpy( src_pntr, &pmtc_frame.bcp[nb].azi_cntr_error, 
			 ENVI_USHRT );
	  src_pntr += ENVI_USHRT;
	  (void) memcpy( src_pntr, &pmtc_frame.bcp[nb].ele_cntr_error, 
			 ENVI_USHRT );
	  src_pntr += ENVI_USHRT;
	  (void) memcpy( src_pntr, &pmtc_frame.bcp[nb].azi_scan_error, 
			 ENVI_USHRT );
	  src_pntr += ENVI_USHRT;
	  (void) memcpy( src_pntr, &pmtc_frame.bcp[nb].ele_scan_error,
			 ENVI_USHRT );
	  src_pntr += ENVI_USHRT;
     } while ( ++nb < NUM_LV0_AUX_BCP );
	  
     (void) memcpy( src_pntr, &pmtc_frame.bench_rad.two_byte, ENVI_USHRT );
     src_pntr += ENVI_USHRT;
     (void) memcpy( src_pntr, &pmtc_frame.bench_elv.two_byte, ENVI_USHRT );
     src_pntr += ENVI_USHRT;
     (void) memcpy( src_pntr, &pmtc_frame.bench_az.two_byte, ENVI_USHRT );
     src_pntr += ENVI_USHRT;
/*
 * write packet data header data
 */
     return (unsigned int) fwrite(src_char, ENVI_CHAR, AUX_DATA_SRC_LENGTH, fd);
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_WR_MDS_DET_SRC
.PURPOSE     write Source Data of Detector Data Packet
.INPUT/OUTPUT
  call as   SCIA_LV0_WR_MDS_DET_SRC( fd, det_length, num_chan, data_src );
     input:  
            FILE *fd                  : stream pointer
            long det_length           : size of the Packet data field 
                                          (in bytes)
            unsigned short num_chan   : Number of channels
            struct det_src data_src   : Detector Source Packets

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
void SCIA_LV0_WR_MDS_DET_SRC( FILE *fd, long det_length, 
			      unsigned short num_chan,
			      const struct det_src *data_src_in )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd@*/
{
     const char prognm[]  = "SCIA_LV0_WR_MDS_DET_SRC";

     register size_t n_cl;
     register unsigned short n_ch;

     char *src_pntr;
     char *src_char;

     unsigned short nr_chan = 0;

     size_t num_byte, num_clus;

     struct det_src data_src;

     if ( (src_char = (char *) malloc( (size_t) det_length )) == NULL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "src_char" );
/*
 * write data of the different channels
 */
     src_pntr = src_char;
     for ( n_ch = 0; n_ch < num_chan; n_ch++ ) {
	  (void) memcpy( &data_src, data_src_in, sizeof( struct det_src ));

	  num_clus = (size_t) data_src.hdr.channel.field.clusters;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_MDS_CHAN_HDR( &data_src.hdr );
#endif
	  (void) memcpy( src_pntr, &data_src.hdr.sync, ENVI_USHRT );
	  src_pntr += ENVI_USHRT;
	  (void) memcpy( src_pntr, &data_src.hdr.channel.two_byte, 
			 ENVI_USHRT );
	  src_pntr += ENVI_USHRT;
	  (void) memcpy( src_pntr, &data_src.hdr.bcps, ENVI_USHRT );
	  src_pntr += ENVI_USHRT;
	  (void) memcpy( src_pntr, &data_src.hdr.command_vis.four_byte, 
			 ENVI_UINT );
	  (void) memcpy( src_pntr, &data_src.hdr.command_ir.four_byte,  
			 ENVI_UINT );
	  src_pntr += ENVI_UINT;
	  (void) memcpy( src_pntr, &data_src.hdr.ratio_hdr.two_byte, 
			 ENVI_USHRT );
	  src_pntr += ENVI_USHRT;
	  (void) memcpy( src_pntr, &data_src.hdr.bias, ENVI_USHRT );
	  src_pntr += ENVI_USHRT;
	  (void) memcpy( src_pntr, &data_src.hdr.temp, ENVI_USHRT );
	  src_pntr += ENVI_USHRT;

	  nr_chan++;
	  data_src.pixel = (struct chan_src *)
	       malloc ( num_clus * sizeof( struct chan_src ) );
	  if ( data_src.pixel == NULL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "pixel" );
	  (void) memcpy( data_src.pixel, data_src_in->pixel,
			 num_clus * sizeof( struct chan_src ) );
/*
 * write data of the clusters
 */
	  n_cl = (unsigned short) 0;
	  do {
	       (void) memcpy( src_pntr, &data_src.pixel[n_cl].sync, 
			      ENVI_USHRT );
	       src_pntr += ENVI_USHRT;
/* cluster block identifier */
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       data_src.pixel[n_cl].block_nr = 
		    byte_swap_u16( data_src.pixel[n_cl].block_nr );
#endif
	       (void) memcpy( src_pntr, &data_src.pixel[n_cl].block_nr, 
			      ENVI_USHRT );
	       src_pntr += ENVI_USHRT;
/* cluster identifier */
	       (void) memcpy( src_pntr, &data_src.pixel[n_cl].cluster_id, 
			      ENVI_UCHAR );
	       src_pntr += ENVI_UCHAR;
/* co-adding indicator */
	       (void) memcpy( src_pntr, &data_src.pixel[n_cl].co_adding, 
			      ENVI_UCHAR );
	       src_pntr += ENVI_UCHAR;
/* start pixel indicator */
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       data_src.pixel[n_cl].start = 
		    byte_swap_u16( data_src.pixel[n_cl].start );
#endif
	       (void) memcpy( src_pntr, &data_src.pixel[n_cl].start, 
			      ENVI_USHRT );
	       src_pntr += ENVI_USHRT;
/* cluster block length */
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       data_src.pixel[n_cl].length = 
		    byte_swap_u16( data_src.pixel[n_cl].length );
#endif
	       (void) memcpy( src_pntr, &data_src.pixel[n_cl].length, 
			      ENVI_USHRT );
	       src_pntr += ENVI_USHRT;
/* determine size of cluster pixel data block */
	       if ( data_src.pixel[n_cl].co_adding == UCHAR_ONE )
		    num_byte = (size_t) 
			 data_src.pixel[n_cl].length * ENVI_USHRT;
	       else {
		    num_byte = (size_t) 
			 data_src.pixel[n_cl].length * 3 * ENVI_UCHAR;
		    if ( (data_src.pixel[n_cl].length % 2) == 1 ) 
			 num_byte += ENVI_UCHAR;
	       }
/* pixel data */
	       (void) memcpy( src_pntr, data_src_in->pixel[n_cl].data, 
			      num_byte );
	       src_pntr += num_byte;
	  } while( ++n_cl < num_clus );
	  data_src_in++;

	  free( data_src.pixel );
     }
/*
 * check number of bytes write
 */
     if ( (long)(src_pntr - src_char) != det_length )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, "Detector MDS size" );
/*
 * write packet data packet
 */
     if ( fwrite( src_char, (size_t) det_length, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, "" );
/*
 * release allocated memory
 */
 done:
     free( src_char );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_WR_PMD_SRC
.PURPOSE     write source data of PMD MDS
.INPUT/OUTPUT
  call as   nr_byte = SCIA_LV0_WR_PMD_SRC( fd, pmd_src );
     input:  
            FILE *fd                : stream pointer
            struct pmd_src pmd_src : structure for PMD Source data

.RETURNS     number of bytes written (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
unsigned int SCIA_LV0_WR_MDS_PMD_SRC( FILE *fd, 
				      const struct pmd_src pmd_src_in )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd@*/
{
     register unsigned short np;

     char *src_pntr;
     char src_char[PMD_DATA_SRC_LENGTH];

     struct pmd_src pmd_src;

     const size_t nr_byte = 2 * PMD_NUMBER * ENVI_USHRT;

     (void) memcpy( &pmd_src, &pmd_src_in, sizeof( struct pmd_src ));
#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_MDS_PMD_SRC( &pmd_src );
#endif
     src_pntr = src_char;
     (void) memcpy( src_pntr, &pmd_src.temp, ENVI_USHRT );
     src_pntr += ENVI_USHRT;

     np = 0;
     do {
	  (void) memcpy( src_pntr, &pmd_src.packet[np].sync, ENVI_USHRT );
	  src_pntr += ENVI_USHRT;
	  (void) memcpy( src_pntr, pmd_src.packet[np].data, nr_byte );
	  src_pntr += nr_byte;
	  (void) memcpy( src_pntr, &pmd_src.packet[np].bcps, ENVI_USHRT );
	  src_pntr += ENVI_USHRT;
	  (void) memcpy( src_pntr, &pmd_src.packet[np].time.two_byte, 
			 ENVI_USHRT );
	  src_pntr += ENVI_USHRT;
     } while ( ++np < NUM_LV0_PMD_PACKET );
/*
 * write packet data header data
 */
     return (unsigned int) fwrite(src_char, ENVI_CHAR, PMD_DATA_SRC_LENGTH, fd);
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_WR_ONE_AUX
.PURPOSE     write one SCIAMACHY level 0 Auxiliary MDS
.INPUT/OUTPUT
  call as   SCIA_LV0_WR_ONE_AUX( fd, info, aux );
     input:  
            FILE   *fd             : (open) stream pointer
	    struct mds0_info *info : structure holding info about MDS records
            struct mds0_aux aux   : Auxiliary MDS records

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
void SCIA_LV0_WR_ONE_AUX( FILE *fd, const struct mds0_info *info,
			  const struct mds0_aux aux )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd@*/
{
     const char prognm[] = "SCIA_LV0_WR_ONE_AUX";

     register unsigned short nf = 0;
/*
 * rewind data file
 */
     (void) fseek( fd, (long) info->offset, SEEK_SET );
/*
 * write Annotation (ISP, FEP)
 */
     SCIA_LV0_WR_MDS_ANNOTATION( fd, aux.isp, aux.fep_hdr );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "Annotation" );
/*
 * write ISP Packet Header
 */
     (void) SCIA_LV0_WR_MDS_PACKET_HDR( fd, aux.packet_hdr);
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "PACKET_HDR" );
/*
 * write ISP Data Field Header
 */
     (void) SCIA_LV0_WR_MDS_DATA_HDR( fd, aux.data_hdr );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "MDS_DATA_HDR" );
/*
 * check packet ID
 */
     if ( (int) aux.data_hdr.id.field.packet != SCIA_AUX_PACKET )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, 
			     "not an Auxiliary data packet" );
     (void) SCIA_LV0_WR_MDS_PMTC_HDR( fd, aux.pmtc_hdr );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "MDS_PMTC_HDR" );
/*
 * write ISP Auxiliary data source packet
 */
     do {
	  (void) SCIA_LV0_WR_MDS_PMTC_FRAME( fd, aux.data_src[nf] );
	  if ( IS_ERR_STAT_FATAL ) 
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "MDS_PMTC_FRAME" );
     } while ( ++nf < NUM_LV0_AUX_PMTC_FRAME );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_WR_ONE_DET
.PURPOSE     write one SCIAMACHY level 0 Detector MDS
.INPUT/OUTPUT
  call as   SCIA_LV0_WR_ONE_DET( fd, info, det );
     input:  
            FILE   *fd              : (open) stream pointer
	    struct mds0_info *info  : structure holding info about MDS records
            struct mds0_det *det    : Detector MDS records

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
void SCIA_LV0_WR_ONE_DET( FILE *fd, const struct mds0_info *info,
			  const struct mds0_det det )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd@*/
{
     const char prognm[] = "SCIA_LV0_WR_ONE_DET";

     register unsigned short nr;

     unsigned short ubuff;
     long           nr_byte;
     int            ibuff[8];
/*
 * rewind data file
 */
     (void) fseek( fd, (long) info->offset, SEEK_SET );
/*
 * write Annotation (ISP, FEP)
 */
     SCIA_LV0_WR_MDS_ANNOTATION( fd, det.isp, det.fep_hdr );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "Annotation" );
/*
 * write Packet Header
 */
     (void) SCIA_LV0_WR_MDS_PACKET_HDR( fd, det.packet_hdr);
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "PACKET_HDR" );
/*
 * write ISP Data Field Header
 */
     (void) SCIA_LV0_WR_MDS_DATA_HDR( fd, det.data_hdr );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "MDS_DATA_HDR" );
/*
 * check packet ID
 */
     if ( (int) det.data_hdr.id.field.packet != SCIA_DET_PACKET )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, 
			     "not a detector data packet" );
/*
 * write Broadcast counter (MDI)
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
     ubuff = byte_swap_u16( det.bcps );
#else
     ubuff = det.bcps;
#endif
     if ( fwrite( &ubuff, ENVI_SHORT, 1, fd )!= 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
/*
 * write PMTC settings
 */
     (void) SCIA_LV0_WR_MDS_PMTC_HDR( fd, det.pmtc_hdr );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "MDS_PMTC_HDR" );
/*
 * write remaining variables from Data Field Header for Detector Data Packet
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
     nr = 0;
     do {
	  ibuff[nr] = byte_swap_32( det.orbit_vector[nr] );
     } while ( ++nr < 8 );

     ubuff = byte_swap_u16( det.num_chan );
#else
     nr = 0;
     do {
	  ibuff[nr] = det.orbit_vector[nr];
     } while ( ++nr < 8 );

     ubuff = det.num_chan;
#endif
     if ( fwrite( ibuff, ENVI_INT, 8, fd ) != 8 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
     if ( fwrite( &ubuff, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
/*
 * write ISP Detector Data Source Packet
 */
     nr_byte = (long) (det.packet_hdr.length - det.data_hdr.length) + 1l;
     SCIA_LV0_WR_MDS_DET_SRC( fd, nr_byte, det.num_chan, det.data_src );
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "MDS_DATA_SRC" );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_WR_ONE_PMD
.PURPOSE     write one SCIAMACHY level 0 PMD MDS
.INPUT/OUTPUT
  call as   SCIA_LV0_WR_ONE_PMD( fd, info, pmd );
     input:  
            FILE   *fd             : (open) stream pointer
	    struct mds0_info *info : structure holding info about MDS records
            struct mds0_pmd pmd    : PMD MDS records

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static inline
void SCIA_LV0_WR_ONE_PMD( FILE *fd, const struct mds0_info *info,
			  const struct mds0_pmd pmd )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd@*/
{
     const char prognm[] = "SCIA_LV0_WR_ONE_PMD";
/*
 * rewind data file
 */
     (void) fseek( fd, (long) info->offset, SEEK_SET );
/*
 * write Annotation (ISP, FEP)
 */
     SCIA_LV0_WR_MDS_ANNOTATION( fd, pmd.isp, pmd.fep_hdr );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "Annotation" );
/*
 * write Packet Header
 */
     (void) SCIA_LV0_WR_MDS_PACKET_HDR( fd, pmd.packet_hdr);
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "PACKET_HDR" );
/*
 * write ISP Data Field Header
 */
     (void) SCIA_LV0_WR_MDS_DATA_HDR( fd, pmd.data_hdr );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "MDS_DATA_HDR" );
/*
 * check packet ID
 */
     if ( (int) pmd.data_hdr.id.field.packet != SCIA_PMD_PACKET )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, 
			     "not a PMD data packet" );
/*
 * write ISP PMD data source packet
 */
     (void) SCIA_LV0_WR_MDS_PMD_SRC( fd, pmd.data_src );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "MDS_PMD_SRC" );
}

/*+++++++++++++++++++++++++ Main Program or Functions +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_WR_AUX
.PURPOSE     write selected SCIAMACHY level 0 Auxiliary MDS
.INPUT/OUTPUT
  call as   nr_aux = SCIA_LV0_WR_AUX( fd, info, num_info, aux );
     input:  
            FILE   *fd             : (open) stream pointer
	    struct mds0_info *info : structure holding info about MDS records
	    unsigned int num_info  : number of indices to struct info
            struct mds0_aux *aux   : Auxiliary MDS records

.RETURNS     number of Auxiliary MDS write (unsigned int), 
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV0_WR_AUX( FILE *fd, const struct mds0_info *info,
			      unsigned int num_info, 
			      const struct mds0_aux *aux )
{
     const char prognm[] = "SCIA_LV0_WR_AUX";

     register unsigned int nr_aux = 0;

     if ( num_info == 0 ) return 0u;
/*
 * write data buffer to MDS structure
 */
     do {
	  SCIA_LV0_WR_ONE_AUX( fd, info, *aux );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, "MDS_AUX" );
/*
 * go to next MDS info data set
 */
	  info++;
	  aux++;
     } while ( ++nr_aux < num_info );
/*
 * set return values
 */
 done:
     return nr_aux;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_WR_DET
.PURPOSE     write selected SCIAMACHY level 0 Detector MDS
.INPUT/OUTPUT
  call as   nr_det = SCIA_LV0_WR_DET( fd, info, num_info, det );
     input:  
            FILE   *fd              : (open) stream pointer
	    struct mds0_info *info  : structure holding info about MDS records
	    unsigned int num_info   : number of indices to struct info
            struct mds0_det *det    : Detector MDS records

.RETURNS     number of Detector MDS write (unsigned int), 
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV0_WR_DET( FILE *fd, const struct mds0_info *info,
			      unsigned int num_info, 
			      const struct mds0_det *det )
{
     const char prognm[] = "SCIA_LV0_WR_DET";

     register unsigned int nr_det = 0;


     if ( num_info == 0 ) return 0u;
/*
 * write data buffer to MDS structure
 */
     do {
	  SCIA_LV0_WR_ONE_DET( fd, info, *det );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, "MDS_DET" );
/*
 * go to next MDS info data set
 */
	  info++;
	  det++;
     } while ( ++nr_det < num_info );
/*
 * set return values
 */
 done:
     return nr_det;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_WR_PMD
.PURPOSE     write selected SCIAMACHY level 0 PMD MDS
.INPUT/OUTPUT
  call as   nr_pmd = SCIA_LV0_WR_PMD( fd, info, num_info, &pmd );
     input:  
            FILE   *fd             : (open) stream pointer
	    struct mds0_info *info : structure holding info about MDS records
	    unsigned int num_info  : number of indices to struct info
            struct mds0_pmd *pmd   : PMD MDS records

.RETURNS     number of PMD MDS write (unsigned int), 
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV0_WR_PMD( FILE *fd, const struct mds0_info *info,
			      unsigned int num_info, 
			      const struct mds0_pmd *pmd )
{
     const char prognm[] = "SCIA_LV0_WR_PMD";

     register unsigned int nr_pmd = 0;

     if ( num_info == 0 ) return 0u;
/*
 * write data buffer to MDS structure
 */
     do {
	  SCIA_LV0_WR_ONE_PMD( fd, info, *pmd );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, "MDS_PMD" );
/*
 * go to next MDS info data set
 */
	  info++;
	  pmd++;
     } while ( ++nr_pmd < num_info );
/*
 * set return values
 */
 done:
     return nr_pmd;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_WR_LV1_AUX
.PURPOSE     write SCIAMACHY level 0 Auxiliary MDS as stored in 
             a level 1b product
.INPUT/OUTPUT
  call as   nr_byte = SCIA_LV0_WR_LV1_AUX( fd, aux );
     input:  
            FILE   *fd             : (open) stream pointer
            struct mds1_aux aux    : Auxiliary MDS records

.RETURNS     number of bytes written (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV0_WR_LV1_AUX( FILE *fd, const struct mds1_aux aux )
{
     const char prognm[] = "SCIA_LV0_WR_LV1_AUX";

     register unsigned short nf = 0;

     unsigned int nr, nr_byte = 0u;

     if ( fwrite( &aux.mjd, sizeof( struct mjd_envi ), 1, fd ) != 1 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, "mjd" );
     nr_byte += sizeof( struct mjd_envi );
     if ( fwrite( &aux.flag_mds, ENVI_UCHAR, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, "flags_mds" );
     nr_byte += ENVI_UCHAR;
/*
 * write Packet Header
 */
     nr = SCIA_LV0_WR_MDS_PACKET_HDR( fd, aux.packet_hdr);
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, "PACKET_HDR" );
     nr_byte += nr;
/*
 * write ISP data field header
 */
     nr = SCIA_LV0_WR_MDS_DATA_HDR( fd, aux.data_hdr );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, "MDS_DATA_HDR" );
     nr_byte += nr;
/*
 * check packet ID
 */
     if ( (int) aux.data_hdr.id.field.packet != SCIA_AUX_PACKET )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, 
			   "not an Auxiliary data packet" );
/*
 * write PMTC settings
 */
     nr = SCIA_LV0_WR_MDS_PMTC_HDR( fd, aux.pmtc_hdr );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, "MDS_PMTC_HDR" );
     nr_byte += nr;
/*
 * write ISP AUX data source packet
 */
     do {
	  nr = SCIA_LV0_WR_MDS_PMTC_FRAME( fd, aux.data_src[nf] );
	  if ( IS_ERR_STAT_FATAL ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, "MDS_PMTC_FRAME" );
	  nr_byte += nr;
     } while ( ++nf < NUM_LV0_AUX_PMTC_FRAME );
 done:
     return nr_byte;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_WR_LV1_PMD
.PURPOSE     write SCIAMACHY level 0 PMD MDS as stored in a level 1b product
.INPUT/OUTPUT
  call as   nr_byte = SCIA_LV0_WR_LV1_PMD( fd, pmd );
     input:  
            FILE   *fd             : (open) stream pointer
            struct mds1_pmd pmd    : PMD MDS records

.RETURNS     number of bytes written (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV0_WR_LV1_PMD( FILE *fd, const struct mds1_pmd pmd )
{
     unsigned int nr, nr_byte = 0u;

     const char prognm[] = "SCIA_LV0_WR_LV1_PMD";

     if ( fwrite( &pmd.mjd, sizeof( struct mjd_envi ), 1, fd ) != 1 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, "mjd" );
     nr_byte += sizeof( struct mjd_envi );
     if ( fwrite( &pmd.flag_mds, ENVI_UCHAR, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, "flag_mds" );
     nr_byte += ENVI_UCHAR;
/*
 * write Packet Header
 */
     nr = SCIA_LV0_WR_MDS_PACKET_HDR( fd, pmd.packet_hdr);
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, "PACKET_HDR" );
     nr_byte += nr;
/*
 * write ISP data field header
 */
     nr = SCIA_LV0_WR_MDS_DATA_HDR( fd, pmd.data_hdr );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, "MDS_DATA_HDR" );
     nr_byte += nr;
/*
 * check packet ID
 */
     if ( (int) pmd.data_hdr.id.field.packet != SCIA_PMD_PACKET )
 	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "not a PMD data packet" );
/*
 * write ISP PMD data source packet
 */
     nr = SCIA_LV0_WR_MDS_PMD_SRC( fd, pmd.data_src );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, "MDS_PMD_SRC" );
     nr_byte += nr;
 done:
     return nr_byte;
}
