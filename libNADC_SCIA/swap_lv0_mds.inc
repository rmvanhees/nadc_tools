#include <swap_bytes.h>

static inline
void Sun2Intel_MDS_ANNOTATION( struct mjd_envi *isp, struct fep_hdr *fep_hdr )
{
     isp->days = byte_swap_32( isp->days );
     isp->secnd = byte_swap_u32( isp->secnd );
     isp->musec = byte_swap_u32( isp->musec );
     fep_hdr->gsrt.days = byte_swap_32( fep_hdr->gsrt.days );
     fep_hdr->gsrt.secnd = byte_swap_u32( fep_hdr->gsrt.secnd );
     fep_hdr->gsrt.musec = byte_swap_u32( fep_hdr->gsrt.musec );

     fep_hdr->isp_length = byte_swap_u16( fep_hdr->isp_length );
     fep_hdr->crc_errs = byte_swap_u16( fep_hdr->crc_errs );
     fep_hdr->rs_errs = byte_swap_u16( fep_hdr->rs_errs );
}

static inline
void Sun2Intel_MDS_PACKET_HDR( struct packet_hdr *packet_hdr )
{
     packet_hdr->api.two_byte = byte_swap_u16( packet_hdr->api.two_byte );
     packet_hdr->seq_cntrl = byte_swap_u16( packet_hdr->seq_cntrl );
     packet_hdr->length = byte_swap_u16( packet_hdr->length );
}


static inline
void Sun2Intel_MDS_DATA_HDR( struct data_hdr *data_hdr )
{
     data_hdr->length = byte_swap_u16( data_hdr->length );
     data_hdr->on_board_time = byte_swap_u32( data_hdr->on_board_time);
}

static inline
void Sun2Intel_MDS_PMTC_HDR( struct pmtc_hdr *pmtc_hdr )
{
     pmtc_hdr->scanner_mode = 
	  byte_swap_u16( pmtc_hdr->scanner_mode );
     pmtc_hdr->az_param.four_byte = 
	  byte_swap_u32( pmtc_hdr->az_param.four_byte );
     pmtc_hdr->elv_param.four_byte = 
	  byte_swap_u32( pmtc_hdr->elv_param.four_byte );
}

static inline
void Sun2Intel_MDS_CHAN_HDR( struct chan_hdr *chan_hdr )
{
     chan_hdr->bcps = byte_swap_u16( chan_hdr->bcps );
     chan_hdr->bias = byte_swap_u16( chan_hdr->bias );
     chan_hdr->temp = byte_swap_u16( chan_hdr->temp );
     chan_hdr->command_vis.four_byte = 
	  byte_swap_u32( chan_hdr->command_vis.four_byte );
     chan_hdr->command_ir.four_byte = 
	  byte_swap_u32( chan_hdr->command_ir.four_byte );
}

static inline
void Sun2Intel_MDS_PMTC_FRAME( struct pmtc_frame *pmtc_frame )
{
     register unsigned short nb = 0;

     do {
	  pmtc_frame->bcp[nb].bcps =
	       byte_swap_u16( pmtc_frame->bcp[nb].bcps );

	  pmtc_frame->bcp[nb].azi_cntr_error =
	       byte_swap_u16( pmtc_frame->bcp[nb].azi_cntr_error );
	  pmtc_frame->bcp[nb].ele_cntr_error =
	       byte_swap_u16( pmtc_frame->bcp[nb].ele_cntr_error );
	  pmtc_frame->bcp[nb].azi_scan_error =
	       byte_swap_u16( pmtc_frame->bcp[nb].azi_scan_error );
	  pmtc_frame->bcp[nb].ele_scan_error =
	       byte_swap_u16( pmtc_frame->bcp[nb].ele_scan_error );
     } while ( ++nb < NUM_LV0_AUX_BCP );
	  
     pmtc_frame->bench_rad.two_byte =
	  byte_swap_u16( pmtc_frame->bench_rad.two_byte );
     pmtc_frame->bench_elv.two_byte =
	  byte_swap_u16( pmtc_frame->bench_elv.two_byte );
     pmtc_frame->bench_az.two_byte = 
	  byte_swap_u16( pmtc_frame->bench_az.two_byte );
}

static inline
void Sun2Intel_MDS_PMD_SRC( struct pmd_src *pmd_src )
{
     register unsigned short np = 0;
     register unsigned short npp;

     pmd_src->temp = byte_swap_u16( pmd_src->temp );
     do {
	  for ( npp = 0; npp < PMD_NUMBER; npp++ ) {
	       pmd_src->packet[np].data[npp][0] = 
		    byte_swap_u16( pmd_src->packet[np].data[npp][0] );
	       pmd_src->packet[np].data[npp][1] = 
		    byte_swap_u16( pmd_src->packet[np].data[npp][1] );
	  }
	  pmd_src->packet[np].bcps = 
	       byte_swap_u16( pmd_src->packet[np].bcps );
	  pmd_src->packet[np].time.two_byte = 
	       byte_swap_u16( pmd_src->packet[np].time.two_byte );
     } while ( ++np < NUM_LV0_PMD_PACKET );
}
