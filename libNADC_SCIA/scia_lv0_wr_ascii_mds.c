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

.IDENTifer   SCIA_LV0_WR_ASCII_MDS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 0 - ASCII dump
.LANGUAGE    ANSI C
.PURPOSE     Dump Measurement Data Sets in ASCII
.COMMENTS    contains SCIA_WR_ASCII_LV0_AUX, SCIA_WR_ASCII_LV0_PMD, 
                SCIA_LV0_WR_ASCII_AUX, SCIA_LV0_WR_ASCII_PMD, 
		SCIA_LV0_WR_ASCII_DET
.ENVIRONment None
.VERSION      1.7   13-Oct-2003 no longer write empty Auxiliary and PMD MDS 
                                after "end of measurement", RvH
              1.6   12-Mar-2003	aggressive inline of static function, RvH
              1.5   19-Mar-2002	replaced confusing bench_obm 
                                with bench_rad, RvH
              1.4   01-Feb-2002	adapted to new MDS structures, RvH 
              1.3   04-Jan-2002	modified struct lv0_aux_bcp, RvH 
              1.2   07-Dec-2001	added SCIA_LV0_WR_ASCII_DET, RvH 
              1.1   07-Dec-2001	added SCIA_LV0_WR_ASCII_PMD, RvH 
              1.0   12-Aug-2001	Created by R. M. van Hees 
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

#define PACKET_SSC     ((unsigned short) 49152)

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static inline
void UNPACK_LV0_PIXEL_VAL( const struct chan_src *pixel,
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

/*
 * -------------------------
 * Level 0 MDS: Annotiations and [ISP] Packet Header
 */
static
void WRITE_ANNOTIATION( FILE *outfl, unsigned int nr, 
			const struct mjd_envi isp, 
			const struct fep_hdr fep_hdr )
{
     char  date_str[UTC_STRING_LENGTH];

     (void) MJD_2_ASCII( isp.days, isp.secnd, isp.musec, date_str );
     nadc_write_text( outfl, nr, "ISP date", date_str );
/*
 * FEP header
 */
     (void) MJD_2_ASCII( fep_hdr.gsrt.days, fep_hdr.gsrt.secnd,
			 fep_hdr.gsrt.musec, date_str );
     nadc_write_text( outfl, nr, "gsrt", date_str );
     nadc_write_ushort( outfl, nr, "isp_length", fep_hdr.isp_length );
     nadc_write_ushort( outfl, nr, "crc_errs", fep_hdr.crc_errs );
     nadc_write_ushort( outfl, nr, "rs_errs", fep_hdr.rs_errs );
}

/*
 * -------------------------
 * Level 0 MDS: [ISP] Packet Header
 */
static
void WRITE_PACKET_HDR( FILE *outfl, unsigned int nr, 
		       const struct packet_hdr packet_hdr )
{
     nadc_write_ushort( outfl, nr, "packet_id-apid-op_mode", 
			packet_hdr.api.field.op_mode );
     nadc_write_ushort( outfl, nr, "packet_id-apid-vcid", 
			packet_hdr.api.field.vcid );
     nadc_write_ushort( outfl, nr, "packet_seq_cntrl-ssc", 
			packet_hdr.seq_cntrl - PACKET_SSC );
     nadc_write_ushort( outfl, nr, "packet_data_length", 
			packet_hdr.length );
}

/*
 * -------------------------
 * Data Field Header of the [ISP] Packet Data Field
 */
static
void WRITE_DATA_HDR( FILE *outfl, unsigned int nr,
		     const struct data_hdr data_hdr )
{
     nadc_write_ushort( outfl, nr, "field header length", 
			data_hdr.length );
     nadc_write_uchar( outfl, nr, "measurement category", 
		       data_hdr.category );
     nadc_write_uchar( outfl, nr, "state id", 
		       data_hdr.state_id );
     nadc_write_uint( outfl, nr, "on_board_time", 
		      data_hdr.on_board_time );
     nadc_write_uchar( outfl, nr, "hsm", 
		       data_hdr.rdv.field.hsm );
     nadc_write_uchar( outfl, nr, "atc table id", 
		       data_hdr.rdv.field.atc_id );
     nadc_write_uchar( outfl, nr, "configuration id", 
		       data_hdr.rdv.field.config_id );
     nadc_write_uchar( outfl, nr, "packet identifier", 
		       data_hdr.id.field.packet );
     nadc_write_uchar( outfl, nr, "buffer overflow indicator", 
		       data_hdr.id.field.overflow );
}

/*
 * -------------------------
 * PMTC settings
 */
static
void WRITE_PMTC_HDR( FILE *outfl, unsigned int nr,
		     const struct pmtc_hdr *pmtc_hdr )
{
     const unsigned int num_factor = 6;

     nadc_write_uchar( outfl, nr, "PHASE", 
		       pmtc_hdr->pmtc_1.field.phase );
     nadc_write_uchar( outfl, nr, "NDFM", 
		       pmtc_hdr->pmtc_1.field.ndfm );
     nadc_write_uchar( outfl, nr, "NCWM", 
		       pmtc_hdr->pmtc_1.field.ncwm );
     nadc_write_uchar( outfl, nr, "APSM", 
		       pmtc_hdr->pmtc_1.field.apsm );
     nadc_write_uchar( outfl, nr, "WLS", 
		       pmtc_hdr->pmtc_1.field.wls );
     nadc_write_uchar( outfl, nr, "SLS", 
		       pmtc_hdr->pmtc_1.field.sls );
     nadc_write_uchar( outfl, nr, "Type (azimuth)", 
		       pmtc_hdr->az_param.field.type );
     nadc_write_uchar( outfl, nr, "Centre (azimuth)", 
		       pmtc_hdr->az_param.field.centre );
     nadc_write_uchar( outfl, nr, "Filter (azimuth)", 
		       pmtc_hdr->az_param.field.filter );
     nadc_write_uchar( outfl, nr, "Invert (azimuth)", 
		       pmtc_hdr->az_param.field.invert );
     nadc_write_uchar( outfl, nr, "Correctrion (azimuth)", 
		       pmtc_hdr->az_param.field.corr );
     nadc_write_uchar( outfl, nr, "Rel. profile (azimuth)", 
		       pmtc_hdr->az_param.field.rel );
     nadc_write_uchar( outfl, nr, "H/W constellation (azimuth)", 
		       pmtc_hdr->az_param.field.h_w );
     nadc_write_uchar( outfl, nr, "Basic profile (azimuth)", 
		       pmtc_hdr->az_param.field.basic );
     nadc_write_ushort( outfl, nr, "Repetitions (azimuth)", 
			pmtc_hdr->az_param.field.repeat );
     nadc_write_uchar( outfl, nr, "Centre (elevation)", 
		       pmtc_hdr->elv_param.field.centre );
     nadc_write_uchar( outfl, nr, "Filter (elevation)", 
		       pmtc_hdr->elv_param.field.filter );
     nadc_write_uchar( outfl, nr, "Invert (elevation)", 
		       pmtc_hdr->elv_param.field.invert );
     nadc_write_uchar( outfl, nr, "Correctrion (elevation)", 
		       pmtc_hdr->elv_param.field.corr );
     nadc_write_uchar( outfl, nr, "Rel. profile (elevation)", 
		       pmtc_hdr->elv_param.field.rel );
     nadc_write_uchar( outfl, nr, "Basic profile (elevation)", 
		       pmtc_hdr->elv_param.field.basic );
     nadc_write_ushort( outfl, nr, "Repetitions (elevation)", 
			pmtc_hdr->elv_param.field.repeat );

     nadc_write_arr_uchar( outfl, nr, "Factor", 
			   1, &num_factor, pmtc_hdr->factor );
}

static inline
void WRITE_CHANNEL_HDR( FILE *outfl, unsigned int nr, 
			const struct chan_hdr chan_hdr )
{
     nadc_write_ushort( outfl, nr, "Channel Sync", chan_hdr.sync );

     nadc_write_uchar( outfl, nr, "Channel ID", chan_hdr.channel.field.id );
     nadc_write_uchar( outfl, nr, "DME Interface Status (IS)", 
		       chan_hdr.channel.field.is );
     nadc_write_uchar( outfl, nr, "Latch Up Indicator (LU)", 
		       chan_hdr.channel.field.lu );
     nadc_write_uchar( outfl, nr, "Cluster Indicator", 
		       chan_hdr.channel.field.clusters );

     nadc_write_ushort( outfl, nr, "Broadcast counter", chan_hdr.bcps );

     if ( chan_hdr.channel.field.id < (unsigned char) 6 ) {
	  nadc_write_ushort( outfl, nr, "Exposure Time Factor", 
			     chan_hdr.command_vis.field.etf );
	  nadc_write_uchar( outfl, nr, "Detector Mode", 
			    chan_hdr.command_vis.field.mode );
	  nadc_write_ushort( outfl, nr, "Section Address", 
			     chan_hdr.command_vis.field.sec );
	  nadc_write_uchar( outfl, nr, "Ratio", 
			    chan_hdr.command_vis.field.ratio );
	  nadc_write_uchar( outfl, nr, "Control", 
			    chan_hdr.command_vis.field.cntrl );
     } else {
	  nadc_write_ushort( outfl, nr, "Exposure Time Factor", 
			     chan_hdr.command_ir.field.etf );
	  nadc_write_uchar( outfl, nr, "Detector Mode", 
			    chan_hdr.command_ir.field.mode );
	  nadc_write_uchar( outfl, nr, "Offset Compensation", 
			    chan_hdr.command_ir.field.comp );
	  nadc_write_uchar( outfl, nr, "Fine Bias Settings", 
			    chan_hdr.command_ir.field.bias );
	  nadc_write_uchar( outfl, nr, "Short Pixel Exposure Time", 
			    chan_hdr.command_ir.field.pet );
	  nadc_write_uchar( outfl, nr, "Control", 
			    chan_hdr.command_ir.field.cntrl );
     }
     nadc_write_uchar( outfl, nr, "Ratio Counter", 
		       chan_hdr.ratio_hdr.field.ratio );
     nadc_write_uchar( outfl, nr, "ADC Status Bits", 
		       chan_hdr.ratio_hdr.field.status );
     nadc_write_uchar( outfl, nr, "Frame Counter", 
		       chan_hdr.ratio_hdr.field.frame );
     nadc_write_ushort( outfl, nr, "Bias Voltage", chan_hdr.bias );
     nadc_write_ushort( outfl, nr, "Detector Temperature", chan_hdr.temp );
}

static inline
void WRITE_PIXEL_BLOCK( FILE *outfl, unsigned int nr, 
			const struct chan_src pixel )
{
     const char prognm[] = "WRITE_PIXEL_BLOCK";

     unsigned int *data;

     const unsigned int adim = pixel.length;

     nadc_write_ushort( outfl, nr, "Synchronisation Pattern", pixel.sync );
     nadc_write_ushort( outfl, nr, "Block Identifier", pixel.block_nr );
     nadc_write_uchar( outfl, nr, "Cluster ID", pixel.cluster_id );
     nadc_write_uchar( outfl, nr, "Co-adding Indicator", pixel.co_adding );
     nadc_write_ushort( outfl, nr, "Start Pixel Indicator", pixel.start );
     nadc_write_ushort( outfl, nr, "Cluster Block Length", pixel.length );
     data = (unsigned int *) malloc( (size_t) pixel.length * sizeof( int ));
     if ( data == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "data" );
     UNPACK_LV0_PIXEL_VAL( &pixel, data );
     nadc_write_arr_uint( outfl, nr, "Pixel Data", 1, &adim, data );
     free( data );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_WR_ASCII_LV0_AUX
.PURPOSE     dump content of Auxiliary data packets to file
.INPUT/OUTPUT
  call as   SCIA_WR_ASCII_LV0_AUX( outfl, nr, aux );
     input:
            FILE  *outfl         : (open) stream pointer
	    unsigned int nr      : number of Auxiliary data packets
	    struct mds0_aux *aux : pointer to Auxiliary data packets

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void SCIA_WR_ASCII_LV0_AUX( FILE  *outfl, unsigned int nr,
			    const struct mds0_aux *aux )
{
     register unsigned int  nb, np;
/*
 * Auxiliary: [ISP] packet header and data header
 */
     WRITE_PACKET_HDR( outfl, nr, aux->packet_hdr );
     WRITE_DATA_HDR( outfl, nr, aux->data_hdr );
     WRITE_PMTC_HDR( outfl, nr, &aux->pmtc_hdr );
/*
 * Auxiliary: [ISP] Packet Data Field (Source Data)
 */
     for ( np = 0; np < NUM_LV0_AUX_PMTC_FRAME; np++ ) {
	  if ( aux->data_src.pmtc[np].bcp[0].sync == USHRT_ZERO ) break;

	  for ( nb = 0; nb < NUM_LV0_AUX_BCP; nb++ ) {
	       nadc_write_ushort( outfl, ++nr, "SYNC", 
				  aux->data_src.pmtc[np].bcp[nb].sync );
	       nadc_write_ushort( outfl, nr, "Broadcast counter", 
				  aux->data_src.pmtc[np].bcp[nb].bcps );

	       nadc_write_uchar( outfl, nr, "AU flag", 
			 aux->data_src.pmtc[np].bcp[nb].flags.field.au );
	       nadc_write_uchar( outfl, nr, "EU flag", 
			 aux->data_src.pmtc[np].bcp[nb].flags.field.eu );
	       nadc_write_uchar( outfl, nr, "D flag", 
			 aux->data_src.pmtc[np].bcp[nb].flags.field.d );
	       nadc_write_uchar( outfl, nr, "M flag", 
			 aux->data_src.pmtc[np].bcp[nb].flags.field.m );
	       nadc_write_uchar( outfl, nr, "PHASE", 
			 aux->data_src.pmtc[np].bcp[nb].flags.field.phase );
	       nadc_write_uchar( outfl, nr, "Pointing counter", 
			 aux->data_src.pmtc[np].bcp[nb].flags.field.pointing);

	       nadc_write_uint( outfl, nr, 
				"Azimuth encoder counter", 
			 aux->data_src.pmtc[np].bcp[nb].azi_encode_cntr );
	       nadc_write_uint( outfl, nr, 
				"Elevation encoder counter", 
		         aux->data_src.pmtc[np].bcp[nb].ele_encode_cntr );

	       nadc_write_ushort( outfl, nr, 
				  "Azimuth counter zero error", 
			 aux->data_src.pmtc[np].bcp[nb].azi_cntr_error );
	       nadc_write_ushort( outfl, nr,  
				  "Elevation counter zero error", 
			 aux->data_src.pmtc[np].bcp[nb].ele_cntr_error );
	       nadc_write_ushort( outfl, nr,  
				  "Azimuth scanner control error", 
			 aux->data_src.pmtc[np].bcp[nb].azi_scan_error );
	       nadc_write_ushort( outfl, nr,   
				  "Elevation scanner control error", 
			 aux->data_src.pmtc[np].bcp[nb].ele_scan_error );
	  }
	  nadc_write_ushort( outfl, nr, "Bench temp_1 (Radiator OBM)",
			     aux->data_src.pmtc[np].bench_rad.field.temp );
	  nadc_write_uchar( outfl, nr, "Optical bench control status 1",
			    aux->data_src.pmtc[np].bench_rad.field.stat );
	  nadc_write_ushort( outfl, nr, "Bench temp_2 (ELV scanner, Nadir)",
			     aux->data_src.pmtc[np].bench_elv.field.temp );
	  nadc_write_uchar( outfl, nr, "Optical bench control status 2",
			    aux->data_src.pmtc[np].bench_elv.field.stat );
	  nadc_write_ushort( outfl, nr, "Bench temp_3 (AZ scanner, Limb)",
			     aux->data_src.pmtc[np].bench_az.field.temp );
	  nadc_write_uchar( outfl, nr, "Optical bench control status 3",
			    aux->data_src.pmtc[np].bench_az.field.stat );
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_WR_ASCII_LV0_PMD
.PURPOSE     dump content of PMD data packets to file
.INPUT/OUTPUT
  call as   SCIA_WR_ASCII_LV0_PMD( outfl, nr, pmd );
     input:
            FILE  *outfl         : (open) stream pointer
	    unsigned int nr      : number of PMD data packets
            struct mds0_pmd *pmd : pointer to PMD records

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void SCIA_WR_ASCII_LV0_PMD( FILE  *outfl, unsigned int nr,
			    const struct mds0_pmd *pmd )
{
     register unsigned int nd;

     const unsigned int dims[2] = {2, PMD_NUMBER};
/*
 * Auxiliary: [ISP] packet header and data header
 */
     WRITE_PACKET_HDR( outfl, nr, pmd->packet_hdr );
     WRITE_DATA_HDR( outfl, nr, pmd->data_hdr );
/*
 * PMD [ISP] Packet Data Field (Source Data)
 */
     nadc_write_ushort( outfl, nr, "PMD temperature (HK)", 
			 pmd->data_src.temp );
     for ( nd = 0; nd < NUM_LV0_PMD_PACKET; nd++ ) {
	  if ( pmd->data_src.packet[nd].sync == USHRT_ZERO ) break;

	  nadc_write_ushort( outfl, ++nr, "SYNC", 
			     pmd->data_src.packet[nd].sync );
	  nadc_write_arr_ushort( outfl, nr, "PMD data", 2, dims, 
				 pmd->data_src.packet[nd].data[0] );
	  nadc_write_ushort( outfl, nr, "Broadcast Counter (MDI)", 
			     pmd->data_src.packet[nd].bcps );
	  nadc_write_ushort( outfl, nr, "Interface Status (IS)", 
			     pmd->data_src.packet[nd].time.field.is );
	  nadc_write_ushort( outfl, nr, "Delta Time", 
			     pmd->data_src.packet[nd].time.field.delta );
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_WR_ASCII_AUX
.PURPOSE     dump content of Auxiliary data packets
.INPUT/OUTPUT
  call as   SCIA_LV0_WR_ASCII_AUX( param, stateIndx, nr_aux, aux );
     input:
             struct param_record param : struct holding user-defined settings
	     unsigned int stateIndx    : Index of State in product
	     unsigned int nr_aux       : number of Auxiliary MDS structures
             struct mds0_aux *aux      : Level 0 Auxiliary MDS records

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void SCIA_LV0_WR_ASCII_AUX( struct param_record param, unsigned int stateIndx,
			    unsigned int nr_aux, const struct mds0_aux *aux )
{
     const char prognm[] = "SCIA_LV0_WR_ASCII_AUX";

     register unsigned int na;

     char  ext_str[10];

     FILE  *outfl;

     (void) snprintf( ext_str, 10, "aux_%03u", stateIndx );
     outfl = CRE_ASCII_File( param.outfile, ext_str );
     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of MDS record
 */
     nadc_write_header( outfl, 999, param.infile, 
			 "Level 0 Measurement Data Sets" );
     for ( na = 0; na < nr_aux; na++, aux++ ) {
	  WRITE_ANNOTIATION( outfl, 0, aux->isp, aux->fep_hdr );
	  SCIA_WR_ASCII_LV0_AUX( outfl, 0, aux );
     }
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_WR_ASCII_PMD
.PURPOSE     dump content of PMD data packets
.INPUT/OUTPUT
  call as   SCIA_LV0_WR_ASCII_PMD( param, stateIndx, nr_pmd, pmd );
     input:
             struct param_record param : struct holding user-defined settings
	     unsigned int stateIndx    : Index of State in product
	     unsigned int nr_pmd       : number of PMD MDS structures
             struct mds0_pmd *pmd      : Level 0 PMD MDS records

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void SCIA_LV0_WR_ASCII_PMD( struct param_record param, unsigned int stateIndx,
			    unsigned int nr_pmd, const struct mds0_pmd *pmd )
{
     const char prognm[] = "SCIA_LV0_WR_ASCII_PMD";

     register unsigned int np;

     char  ext_str[10];

     FILE  *outfl;

     (void) snprintf( ext_str, 10, "pmd_%03u", stateIndx );
     outfl = CRE_ASCII_File( param.outfile, ext_str );
     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of MDS record
 */
     nadc_write_header( outfl, 999, param.infile, 
			 "Level 0 Measurement Data Sets" );
     for ( np = 0; np < nr_pmd; np++, pmd++ ) {
	  WRITE_ANNOTIATION( outfl, 0, pmd->isp, pmd->fep_hdr );
	  SCIA_WR_ASCII_LV0_PMD( outfl, 0, pmd );
     }
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_WR_ASCII_DET
.PURPOSE     dump content of Detector data packets
.INPUT/OUTPUT
  call as   SCIA_LV0_WR_ASCII_DET( param, stateIndx, nr_det, det );
     input:
             struct param_record param : struct holding user-defined settings
	     unsigned int stateIndx    : Index of State in product
	     unsigned int nr_det       : number of Detector MDS structures
             struct mds0_det *det      : Level 0 Detector MDS records

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void SCIA_LV0_WR_ASCII_DET( struct param_record param, unsigned int stateIndx,
			    unsigned int nr_det, const struct mds0_det *det )
{
     const char prognm[] = "SCIA_LV0_WR_ASCII_DET";

     register unsigned short n_chan, n_clus;
     register unsigned int   nd, nr;

     const unsigned int num_orbit = 8;

     char  ext_str[10];

     FILE  *outfl;

     (void) snprintf( ext_str, 10, "det_%03u", stateIndx );
     outfl = CRE_ASCII_File( param.outfile, ext_str );
     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of MDS record
 */
     nadc_write_header( outfl, 0, param.infile, 
			 "Level 0 Measurement Data Sets" );
     for ( nd = 0; nd < nr_det; nd++, det++ ) {
	  nr = 0;
	  WRITE_ANNOTIATION( outfl, ++nr, det->isp, det->fep_hdr );
	  WRITE_PACKET_HDR( outfl, ++nr, det->packet_hdr );
	  WRITE_DATA_HDR( outfl, ++nr, det->data_hdr );
	  nadc_write_ushort( outfl, nr, "Broadcast Counter (MDI)", 
			      det->bcps );
	  WRITE_PMTC_HDR( outfl, ++nr, &det->pmtc_hdr );
	  nadc_write_arr_int( outfl, ++nr, "Orbit State Vector", 
			       1, &num_orbit, det->orbit_vector );
	  nadc_write_ushort( outfl, ++nr, "Channels", det->num_chan );
/*
 * -------------------------
 * Detector [ISP] Packet Data Field (Source Data)
 *
 * Channel Data Header
 */
	  for ( n_chan = 0; n_chan < det->num_chan; n_chan++ ) {
	       struct det_src data_src = det->data_src[n_chan];

	       unsigned short num_clus = (unsigned short)
		    det->data_src[n_chan].hdr.channel.field.clusters;

	       WRITE_CHANNEL_HDR( outfl, ++nr, data_src.hdr );

	       for ( n_clus = 0; n_clus < num_clus; n_clus++ ) {
		    WRITE_PIXEL_BLOCK( outfl, nr, 
				       data_src.pixel[n_clus] );
	       }
	  }
     }
     (void) fclose( outfl );
}
