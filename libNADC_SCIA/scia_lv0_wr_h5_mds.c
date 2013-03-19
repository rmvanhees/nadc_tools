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

.IDENTifer   SCIA_LV0_WR_H5_MDS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 0 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write Measurement Data Sets in HDF5 format
.INPUT/OUTPUT
  call as    SCIA_LV0_WR_H5_AUX( param, stateIndx, nr_mds, aux );
             SCIA_LV0_WR_H5_DET( param, stateIndx, nr_mds, det );
             SCIA_LV0_WR_H5_PMD( param, stateIndx, nr_mds, pmd );

     input:  
             struct param_record param :  struct holding user-defined settings
	     unsigned int    stateIndx :  index of State in product
	     unsigned int    nr_mds    :  number of MDS structures	
             struct mds0_aux *aux      :  Auxiliary MDS records
             struct mds0_det *det      :  Detector MDS records
             struct mds0_pmd *pmd      :  PMD MDS records

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      2.0   20-Oct-2003	complete rewrite using hdf5_hl, RvH
              1.2   21-Feb-2002	completed implementation, RvH
              1.1   13-Feb-2002	write level 0 structs for AUX and PMD MDS, RvH 
              1.0   06-Feb-2002	created by R. M. van Hees 
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <stdlib.h>
#include <string.h>

#include <hdf5.h>
#include <hdf5_hl.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_0
#include <nadc_scia.h>

#define CMP_MJD(a,b) memcmp( &(a), &(b), sizeof(struct mjd_envi)) 

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
/*
 * determine characteristics of available clusters
 */
static inline
unsigned short GET_SCIA_CLUSTERDEF( struct mjd_envi mjd, unsigned int nr_info,
				    const struct mds0_info *info,
				    /*@null@*/ /*@out@*/
				    struct clusdef_rec *clusDef )
{
     register unsigned char ncl, nch;
     register unsigned int  ni;

     unsigned short numClus = 0;

     for ( nch = 1; nch <= SCIENCE_CHANNELS; nch++ ) {
          unsigned char clusIDmx = 0;
          for ( ni = 0; ni < nr_info; ni++ ) {
               if ( CMP_MJD( info[ni].mjd, mjd ) != 0 ) continue;

               for ( ncl = 0 ; ncl < info[ni].numClusters; ncl++ ) {
                    if ( info[ni].cluster[ncl].chanID == nch
                         && info[ni].cluster[ncl].clusID == clusIDmx ) {
                         clusIDmx = info[ni].cluster[ncl].clusID;
                         if ( clusDef != NULL ) {
                              clusDef[numClus+clusIDmx].chanID =
                                   info[ni].cluster[ncl].chanID;
                              clusDef[numClus+clusIDmx].clusID =
                                   info[ni].cluster[ncl].clusID;
                              clusDef[numClus+clusIDmx].start  =
                                   info[ni].cluster[ncl].start;
                              clusDef[numClus+clusIDmx].length =
				   info[ni].cluster[ncl].length;
                         }
                         clusIDmx += UCHAR_ONE;
                    }
               }
          }
          numClus += clusIDmx;
     }
     return numClus;
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_WR_H5_AUX
.PURPOSE     define and write Auxiliary MDS records in HDF5 format
.INPUT/OUTPUT
  call as   SCIA_LV0_WR_H5_AUX( param, stateIndx, nr_mds, aux );
     input:  
             struct param_record param : struct holding user-defined settings
	     unsigned int stateIndx    : index of State in product
	     unsigned int nr_mds       : number of MDS structures	
             struct mds0_aux *aux      : Auxiliary MDS records

.RETURNS     nothing
.COMMENTS    nothing
-------------------------*/
void SCIA_LV0_WR_H5_AUX( struct param_record param, unsigned int stateIndx, 
                         unsigned int nr_aux, const struct mds0_aux *aux )
{
     const char prognm[] = "SCIA_LV0_WR_H5_AUX";

     hid_t   grpID = -1;
     hid_t   ptable;

     char    grpName[11];

     const char tblName[] = "mds0_aux";

     const int compress = (param.flag_deflate == PARAM_SET) ? 3 : -1;
/*
 * check number of MDS records
 */
     if ( nr_aux == 0 ) return;
/*
 * open/create group for MDS data
 */
     switch ( (int) GET_SCIA_MDS_TYPE( aux->data_hdr.state_id ) ) {
     case SCIA_NADIR:
	  (void) snprintf( grpName, 11, "nadir_%03u", stateIndx );
	  break;
     case SCIA_LIMB:
	  (void) snprintf( grpName, 11, "limb_%03u", stateIndx );
	  break;
     case SCIA_MONITOR:
	  (void) snprintf( grpName, 11, "moni_%03u", stateIndx );
	  break;
     case SCIA_OCCULT:
	  (void) snprintf( grpName, 11, "occult_%03u", stateIndx );
	  break;
     default:
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, "unknown MDS type" );
     }
     grpID = NADC_OPEN_HDF5_Group( param.hdf_file_id, grpName );
     if ( grpID < 0 ) NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, grpName );
/*
 * create table: AUX MDS
 */
     if ( H5LTfind_dataset( grpID, tblName ) == 0 ) {
	  hsize_t chunk_sz = 1;
	  hsize_t adim;
	  hid_t   dtype_id;
	  hid_t   tid, tid_bcp, tid_frame;

	  /* create compound aux_bcp */
	  tid_bcp = H5Tcreate( H5T_COMPOUND, sizeof(struct aux_bcp) );
	  (void) H5Tinsert( tid_bcp, "sync",
			    HOFFSET( struct aux_bcp, sync ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid_bcp, "bcps",
			    HOFFSET( struct aux_bcp, bcps ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid_bcp, "flags",
			    HOFFSET( struct aux_bcp, flags.two_byte ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid_bcp, "azi_encode_cntr",
			    HOFFSET( struct aux_bcp, azi_encode_cntr ),
			    H5T_NATIVE_UINT );
	  (void) H5Tinsert( tid_bcp, "ele_encode_cntr",
			    HOFFSET( struct aux_bcp, ele_encode_cntr ),
			    H5T_NATIVE_UINT );
	  (void) H5Tinsert( tid_bcp, "azi_cntr_error",
			    HOFFSET( struct aux_bcp, azi_cntr_error ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid_bcp, "ele_cntr_error",
			    HOFFSET( struct aux_bcp, ele_cntr_error ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid_bcp, "azi_scan_error",
			    HOFFSET( struct aux_bcp, azi_scan_error ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid_bcp, "ele_scan_error",
			    HOFFSET( struct aux_bcp, ele_scan_error ),
			    H5T_NATIVE_USHORT );

	  /* create compound pmtc_frame */
	  tid_frame = H5Tcreate( H5T_COMPOUND, sizeof(struct pmtc_frame) );
	  adim = NUM_LV0_AUX_BCP;
	  dtype_id = H5Tarray_create( tid_bcp, 1, &adim );
	  (void) H5Tinsert( tid_frame, "bcp",
			    HOFFSET( struct pmtc_frame, bcp ),
			    dtype_id );
	  (void) H5Tclose( dtype_id );
	  (void) H5Tinsert( tid_frame, "bench_rad",
			    HOFFSET( struct pmtc_frame, bench_rad ),
			    H5T_NATIVE_USHORT);
	  (void) H5Tinsert( tid_frame, "bench_elv",
			    HOFFSET( struct pmtc_frame, bench_elv ),
			    H5T_NATIVE_USHORT);
	  (void) H5Tinsert( tid_frame, "bench_az",
			    HOFFSET( struct pmtc_frame, bench_az ),
			    H5T_NATIVE_USHORT);

	  /* create compound mds0_aux */
	  tid = H5Tcreate( H5T_COMPOUND, sizeof(struct mds0_aux) );
	  (void) H5Tinsert( tid, "isp.days",
			    HOFFSET( struct mds0_aux, isp.days ),
			    H5T_NATIVE_INT );
	  (void) H5Tinsert( tid, "isp.secnd",
			    HOFFSET( struct mds0_aux, isp.secnd ),
			    H5T_NATIVE_UINT );
	  (void) H5Tinsert( tid, "isp.musec",
			    HOFFSET( struct mds0_aux, isp.musec ),
			    H5T_NATIVE_UINT );
	  (void) H5Tinsert( tid, "fep_hdr.gsrt.days",
			    HOFFSET( struct mds0_aux, fep_hdr.gsrt.days ),
			    H5T_NATIVE_INT );
	  (void) H5Tinsert( tid, "fep_hdr.gsrt.secnd",
			    HOFFSET( struct mds0_aux, fep_hdr.gsrt.secnd ),
			    H5T_NATIVE_UINT );
	  (void) H5Tinsert( tid, "fep_hdr.gsrt.musec",
			    HOFFSET( struct mds0_aux, fep_hdr.gsrt.musec ),
			    H5T_NATIVE_UINT );
	  (void) H5Tinsert( tid, "fep_hdr.isp_length",
			    HOFFSET( struct mds0_aux, fep_hdr.isp_length ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "fep_hdr.crc_errs",
			    HOFFSET( struct mds0_aux, fep_hdr.crc_errs ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "fep_hdr.rs_errs",
			    HOFFSET( struct mds0_aux, fep_hdr.rs_errs ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "packet_hdr.api",
			    HOFFSET( struct mds0_aux, packet_hdr.api ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "packet_hdr.seq_cntrl",
			    HOFFSET( struct mds0_aux, packet_hdr.seq_cntrl ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "packet_hdr.length",
			    HOFFSET( struct mds0_aux, packet_hdr.length ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "data_hdr.category",
			    HOFFSET( struct mds0_aux, data_hdr.category ),
			    H5T_NATIVE_UCHAR );
	  (void) H5Tinsert( tid, "data_hdr.state_id",
			    HOFFSET( struct mds0_aux, data_hdr.state_id ),
			    H5T_NATIVE_UCHAR );
	  (void) H5Tinsert( tid, "data_hdr.length",
			    HOFFSET( struct mds0_aux, data_hdr.length ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "data_hdr.rdv",
			    HOFFSET( struct mds0_aux, data_hdr.rdv ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "data_hdr.id",
			    HOFFSET( struct mds0_aux, data_hdr.id ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "data_hdr.on_board_time",
			    HOFFSET( struct mds0_aux, data_hdr.on_board_time ),
			    H5T_NATIVE_UINT );
	  (void) H5Tinsert( tid, "pmtc_hdr.pmtc_1",
			    HOFFSET( struct mds0_aux, pmtc_hdr.pmtc_1 ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "pmtc_hdr.scanner_mode",
			    HOFFSET( struct mds0_aux, pmtc_hdr.scanner_mode ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "pmtc_hdr.az_param",
			    HOFFSET( struct mds0_aux, pmtc_hdr.az_param ),
			    H5T_NATIVE_UINT );
 	  (void) H5Tinsert( tid, "pmtc_hdr.elv_param",
			    HOFFSET( struct mds0_aux, pmtc_hdr.elv_param ),
			    H5T_NATIVE_UINT );
	  adim = 6;
	  dtype_id = H5Tarray_create( H5T_NATIVE_CHAR, 1, &adim );
	  (void) H5Tinsert( tid, "pmtc_hdr.factor",
			    HOFFSET( struct mds0_aux, pmtc_hdr.factor ),
			    dtype_id );
	  (void) H5Tclose( dtype_id );
	  adim = NUM_LV0_AUX_PMTC_FRAME;
	  dtype_id = H5Tarray_create( tid_frame, 1, &adim );
	  (void) H5Tinsert( tid, "data_src.pmtc",
			    HOFFSET( struct mds0_aux, data_src.pmtc ),
			    dtype_id );
	  (void) H5Tclose( dtype_id );

	  ptable = H5PTcreate_fl( grpID, tblName, tid, chunk_sz, compress );
	  (void) H5Tclose( tid );
	  (void) H5Tclose( tid_bcp );
	  (void) H5Tclose( tid_frame );
	  if ( ptable == H5I_BADID )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DATA, tblName );
     } else {
	  ptable = H5PTopen( grpID, tblName );
 	  if ( ptable == H5I_BADID )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DATA, tblName );
     }
     if ( H5PTappend( ptable, nr_aux, aux ) < 0 )
          NADC_ERROR( prognm, NADC_ERR_HDF_WR, tblName );
     (void) H5PTclose( ptable );
/*
 * close interface
 */
done:
     if ( H5Gclose( grpID ) < 0 ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, grpName );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_WR_H5_PMD
.PURPOSE     define and write PMD MDS records in HDF5 format
.INPUT/OUTPUT
  call as   SCIA_LV0_WR_H5_PMD( param, stateIndx, nr_mds, pmd );
     input:  
             struct param_record param : struct holding user-defined settings
	     unsigned int stateIndx    : index of State in product
	     unsigned int nr_mds       : number of MDS structures	
             struct mds0_pmd *pmd      : PMD MDS records

.RETURNS     nothing
.COMMENTS    nothing
-------------------------*/
void SCIA_LV0_WR_H5_PMD( struct param_record param, unsigned int stateIndx, 
                         unsigned int nr_pmd, const struct mds0_pmd *pmd )
{
     const char prognm[] = "SCIA_LV0_WR_H5_PMD";

     char    grpName[11];

     hid_t   grpID = -1;
     hid_t   ptable;

     const char tblName[] = "mds0_pmd";

     const int compress = (param.flag_deflate == PARAM_SET) ? 3 : -1;
/*
 * check number of MDS records
 */
     if ( nr_pmd == 0 ) return;
/*
 * open/create group for MDS data
 */
     switch ( (int) GET_SCIA_MDS_TYPE( pmd->data_hdr.state_id ) ) {
     case SCIA_NADIR:
	  (void) snprintf( grpName, 11, "nadir_%03u", stateIndx );
	  break;
     case SCIA_LIMB:
	  (void) snprintf( grpName, 11, "limb_%03u", stateIndx );
	  break;
     case SCIA_MONITOR:
	  (void) snprintf( grpName, 11, "moni_%03u", stateIndx );
	  break;
     case SCIA_OCCULT:
	  (void) snprintf( grpName, 11, "occult_%03u", stateIndx );
	  break;
     default:
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, "unknown MDS type" );
     }
     grpID = NADC_OPEN_HDF5_Group( param.hdf_file_id, grpName );
     if ( grpID < 0 ) NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, grpName );
/*
 * create table: PMD MDS
 */
     if ( H5LTfind_dataset( grpID, tblName ) == 0 ) {
          hsize_t chunk_sz = 1;
          hsize_t dims[2];
          hid_t   dtype_id;
          hid_t   tid, tid_data, tid_src;

	  /* create compound pmd_data */
	  tid_data = H5Tcreate( H5T_COMPOUND, sizeof(struct pmd_data) );
	  (void) H5Tinsert( tid_data, "sync",
			    HOFFSET( struct pmd_data, sync ),
			    H5T_NATIVE_USHORT );
	  dims[0] = 2;
	  dims[1] = PMD_NUMBER;
	  dtype_id = H5Tarray_create( H5T_NATIVE_USHORT, 2, dims );
	  (void) H5Tinsert( tid_data, "data",
			    HOFFSET( struct pmd_data, data ),
			    dtype_id );
	  (void) H5Tclose( dtype_id );
	  (void) H5Tinsert( tid_data, "bcps",
			    HOFFSET( struct pmd_data, bcps ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid_data, "time.two_byte",
			    HOFFSET( struct pmd_data, time.two_byte ),
			    H5T_NATIVE_USHORT );

	  /* create compound pmd_src */
	  tid_src = H5Tcreate( H5T_COMPOUND, sizeof(struct pmd_src) );
	  (void) H5Tinsert( tid_src, "temp",
			    HOFFSET( struct pmd_src, temp ),
			    H5T_NATIVE_USHORT );
	  dims[0] = NUM_LV0_PMD_PACKET;
	  dtype_id = H5Tarray_create( tid_data, 1, dims );
	  (void) H5Tinsert( tid_src, "packet",
			    HOFFSET( struct pmd_src, packet ),
			    dtype_id );	
	  (void) H5Tclose( dtype_id );
	  (void) H5Tclose( tid_data );
  
	  /* create compound mds0_pmd */
	  tid = H5Tcreate( H5T_COMPOUND, sizeof(struct mds0_pmd) );
	  (void) H5Tinsert( tid, "isp.days",
			    HOFFSET( struct mds0_pmd, isp.days ),
			    H5T_NATIVE_INT );
	  (void) H5Tinsert( tid, "isp.secnd",
			    HOFFSET( struct mds0_pmd, isp.secnd ),
			    H5T_NATIVE_UINT );
	  (void) H5Tinsert( tid, "isp.musec",
			    HOFFSET( struct mds0_pmd, isp.musec ),
			    H5T_NATIVE_UINT );
	  (void) H5Tinsert( tid, "fep_hdr.gsrt.days",
			    HOFFSET( struct mds0_pmd, fep_hdr.gsrt.days ),
			    H5T_NATIVE_INT );
	  (void) H5Tinsert( tid, "fep_hdr.gsrt.secnd",
			    HOFFSET( struct mds0_pmd, fep_hdr.gsrt.secnd ),
			    H5T_NATIVE_UINT );
	  (void) H5Tinsert( tid, "fep_hdr.gsrt.musec",
			    HOFFSET( struct mds0_pmd, fep_hdr.gsrt.musec ),
			    H5T_NATIVE_UINT );
	  (void) H5Tinsert( tid, "fep_hdr.isp_length",
			    HOFFSET( struct mds0_pmd, fep_hdr.isp_length ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "fep_hdr.crc_errs",
			    HOFFSET( struct mds0_pmd, fep_hdr.crc_errs ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "fep_hdr.rs_errs",
			    HOFFSET( struct mds0_pmd, fep_hdr.rs_errs ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "packet_hdr.api",
			    HOFFSET( struct mds0_pmd, packet_hdr.api ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "packet_hdr.seq_cntrl",
			    HOFFSET( struct mds0_pmd, packet_hdr.seq_cntrl ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "packet_hdr.length",
			    HOFFSET( struct mds0_pmd, packet_hdr.length ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "data_hdr.category",
			    HOFFSET( struct mds0_pmd, data_hdr.category ),
			    H5T_NATIVE_UCHAR );
	  (void) H5Tinsert( tid, "data_hdr.state_id",
			    HOFFSET( struct mds0_pmd, data_hdr.state_id ),
			    H5T_NATIVE_UCHAR );
	  (void) H5Tinsert( tid, "data_hdr.length",
			    HOFFSET( struct mds0_pmd, data_hdr.length ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "data_hdr.rdv",
			    HOFFSET( struct mds0_pmd, data_hdr.rdv ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "data_hdr.id",
			    HOFFSET( struct mds0_pmd, data_hdr.id ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "data_hdr.on_board_time",
			    HOFFSET( struct mds0_pmd, data_hdr.on_board_time ),
			    H5T_NATIVE_UINT );
	  (void) H5Tinsert( tid, "packet",
			    HOFFSET( struct mds0_pmd, data_src ),
			    tid_src );	

	  ptable = H5PTcreate_fl( grpID, tblName, tid, chunk_sz, compress );
	  (void) H5Tclose( tid );
	  (void) H5Tclose( tid_src );
	  if ( ptable == H5I_BADID )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DATA, tblName );
     } else {
	  ptable = H5PTopen( grpID, tblName );
 	  if ( ptable == H5I_BADID )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DATA, tblName );
     }
     if ( H5PTappend( ptable, nr_pmd, pmd ) < 0 )
          NADC_ERROR( prognm, NADC_ERR_HDF_WR, tblName );
     (void) H5PTclose( ptable );
/*
 * close interface
 */
done:
     if ( H5Gclose( grpID ) < 0 ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, grpName );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_WR_H5_DET
.PURPOSE     define and write Detector MDS records in HDF5 format
.INPUT/OUTPUT
  call as   SCIA_LV0_WR_H5_DET( param, info, nr_mds, det );
     input:  
             struct param_record param : struct holding user-defined settings
	     struct mds0_scia *info    : 
	     unsigned int nr_mds       : number of MDS structures	
             struct mds0_det *det      : Detector MDS records

.RETURNS     nothing
.COMMENTS    nothing
-------------------------*/
void SCIA_LV0_WR_H5_DET( struct param_record param, 
			 const struct mds0_info *info,
                         unsigned int nr_det, const struct mds0_det *det )
{
     const char prognm[] = "SCIA_LV0_WR_H5_DET";

     register unsigned short nc;

     char    grpName[11];
     hsize_t chunk_sz;

     hid_t   grpID = -1;
     hid_t   subgrpID;
     hid_t   ptable;

     register unsigned int   nd;

     struct clusdef_rec clusDef[MAX_CLUSTER];

     struct chan_hdr *chan_hdr = NULL;

     const char tblName[] = "mds0_det";

     const unsigned short stateIndex = (info != NULL) ? info->stateIndex : 0;

     const unsigned short CLUSTER_SYNC = 0xBBBB;
     const unsigned short numClus = 
	  GET_SCIA_CLUSTERDEF( det->isp, nr_det, info, clusDef );

     const int compress = (param.flag_deflate == PARAM_SET) ? 3 : -1;
/*
 * check number of MDS records
 */
     if ( nr_det == 0 ) return;
/*
 * open/create group for MDS data
 */
     switch ( (int) GET_SCIA_MDS_TYPE( det->data_hdr.state_id ) ) {
     case SCIA_NADIR:
	  (void) snprintf( grpName, 11, "nadir_%03hu", stateIndex );
	  break;
     case SCIA_LIMB:
	  (void) snprintf( grpName, 11, "limb_%03hu", stateIndex );
	  break;
     case SCIA_MONITOR:
	  (void) snprintf( grpName, 11, "moni_%03hu", stateIndex );
	  break;
     case SCIA_OCCULT:
	  (void) snprintf( grpName, 11, "occult_%03hu", stateIndex );
	  break;
     default:
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, "unknown MDS type" );
     }
     grpID = NADC_OPEN_HDF5_Group( param.hdf_file_id, grpName );
     if ( grpID < 0 ) NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, grpName );
/*
 * create table: Detector MDS
 */
     if ( H5LTfind_dataset( grpID, "mds0_det" ) == 0 ) {
	  hsize_t adim;
	  hid_t   dtype_id;
	  hid_t   tid;

	  /* create compound mds0_det */
	  tid = H5Tcreate( H5T_COMPOUND, sizeof(struct mds0_det) );
	  (void) H5Tinsert( tid, "bcps",
			    HOFFSET( struct mds0_det, bcps ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "num_chan",
			    HOFFSET( struct mds0_det, num_chan ),
			    H5T_NATIVE_USHORT );
	  adim = 8;
	  dtype_id = H5Tarray_create( H5T_NATIVE_INT, 1, &adim );
	  (void) H5Tinsert( tid, "orbit_vector",
			    HOFFSET( struct mds0_det, orbit_vector ),
			    dtype_id );
	  (void) H5Tclose( dtype_id );
	  (void) H5Tinsert( tid, "isp.days",
			    HOFFSET( struct mds0_det, isp.days ),
			    H5T_NATIVE_INT );
	  (void) H5Tinsert( tid, "isp.secnd",
			    HOFFSET( struct mds0_det, isp.secnd ),
			    H5T_NATIVE_UINT );
	  (void) H5Tinsert( tid, "isp.musec",
			    HOFFSET( struct mds0_det, isp.musec ),
			    H5T_NATIVE_UINT );
	  (void) H5Tinsert( tid, "fep_hdr.gsrt.days",
			    HOFFSET( struct mds0_det, fep_hdr.gsrt.days ),
			    H5T_NATIVE_INT );
	  (void) H5Tinsert( tid, "fep_hdr.gsrt.secnd",
			    HOFFSET( struct mds0_det, fep_hdr.gsrt.secnd ),
			    H5T_NATIVE_UINT );
	  (void) H5Tinsert( tid, "fep_hdr.gsrt.musec",
			    HOFFSET( struct mds0_det, fep_hdr.gsrt.musec ),
			    H5T_NATIVE_UINT );
	  (void) H5Tinsert( tid, "fep_hdr.isp_length",
			    HOFFSET( struct mds0_det, fep_hdr.isp_length ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "fep_hdr.crc_errs",
			    HOFFSET( struct mds0_det, fep_hdr.crc_errs ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "fep_hdr.rs_errs",
			    HOFFSET( struct mds0_det, fep_hdr.rs_errs ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "packet_hdr.api",
			    HOFFSET( struct mds0_det, packet_hdr.api ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "packet_hdr.seq_cntrl",
			    HOFFSET( struct mds0_det, packet_hdr.seq_cntrl ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "packet_hdr.length",
			    HOFFSET( struct mds0_det, packet_hdr.length ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "data_hdr.category",
			    HOFFSET( struct mds0_det, data_hdr.category ),
			    H5T_NATIVE_UCHAR );
	  (void) H5Tinsert( tid, "data_hdr.state_id",
			    HOFFSET( struct mds0_det, data_hdr.state_id ),
			    H5T_NATIVE_UCHAR );
	  (void) H5Tinsert( tid, "data_hdr.length",
			    HOFFSET( struct mds0_det, data_hdr.length ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "data_hdr.rdv",
			    HOFFSET( struct mds0_det, data_hdr.rdv ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "data_hdr.id",
			    HOFFSET( struct mds0_det, data_hdr.id ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "data_hdr.on_board_time",
			    HOFFSET( struct mds0_det, data_hdr.on_board_time ),
			    H5T_NATIVE_UINT );
	  (void) H5Tinsert( tid, "pmtc_hdr.pmtc_1",
			    HOFFSET( struct mds0_det, pmtc_hdr.pmtc_1 ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "pmtc_hdr.scanner_mode",
			    HOFFSET( struct mds0_det, pmtc_hdr.scanner_mode ),
			    H5T_NATIVE_USHORT );
	  (void) H5Tinsert( tid, "pmtc_hdr.az_param",
			    HOFFSET( struct mds0_det, pmtc_hdr.az_param ),
			    H5T_NATIVE_UINT );
 	  (void) H5Tinsert( tid, "pmtc_hdr.elv_param",
			    HOFFSET( struct mds0_det, pmtc_hdr.elv_param ),
			    H5T_NATIVE_UINT );
	  adim = 6;
	  dtype_id = H5Tarray_create( H5T_NATIVE_CHAR, 1, &adim );
	  (void) H5Tinsert( tid, "pmtc_hdr.factor",
			    HOFFSET( struct mds0_det, pmtc_hdr.factor ),
			    dtype_id );
	  (void) H5Tclose( dtype_id );

	  chunk_sz = nr_det;
	  ptable = H5PTcreate_fl( grpID, tblName, tid, chunk_sz, compress );
	  (void) H5Tclose( tid );
	  if ( ptable == H5I_BADID )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DATA, tblName );
     } else {
	  ptable = H5PTopen( grpID, tblName );
 	  if ( ptable == H5I_BADID )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DATA, tblName );
     }
     if ( H5PTappend( ptable, nr_det, det ) < 0 )
          NADC_ERROR( prognm, NADC_ERR_HDF_WR, tblName );
     (void) H5PTclose( ptable );
/*
 * allocate memory for channel/cluster header
 */
     chan_hdr = (struct chan_hdr *) malloc( nr_det * sizeof(struct chan_hdr));
     if ( chan_hdr == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "chan_hdr" );
/*
 * process detector source packets
 */
     for ( nc = 0; nc < numClus; nc++ ) {
	  register unsigned short numHDR = 0;

	  char            *data;
	  unsigned short  *ptr_short;
	  unsigned int    *ptr_int;
	  struct chan_src *ptr_chan_src = NULL;

	  (void) snprintf( grpName, 11, "cluster_%02u", (nc + 1u) );
/*
 * allocate memory and intitialize pointers
 */
	  data = (char *) malloc(nr_det * clusDef[nc].length * sizeof(int));
	  if ( data == NULL ) NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "data" );
	  ptr_short = (unsigned short *) data;
	  ptr_int = (unsigned int *) data;

	  for ( nd = 0; nd < nr_det; nd++ ) {
	       register unsigned char  nclus = UCHAR_ZERO;
	       register unsigned short nchan = 0;
	       unsigned char clusters;

	       do {
		    unsigned char id =
			 det[nd].data_src[nchan].hdr.channel.field.id;

		    if ( clusDef[nc].chanID == id ) break;
	       } while( ++nchan < det[nd].num_chan );
	       if ( nchan == det[nd].num_chan ) continue;

	       clusters = det[nd].data_src[nchan].hdr.channel.field.clusters;
	       do {
		    unsigned char cluster_id =
			 det[nd].data_src[nchan].pixel[nclus].cluster_id;

		    if ( clusDef[nc].clusID == cluster_id ) break;
	       } while( ++nclus < clusters );
	       if ( nclus == clusters ) continue;

	       if ( det[nd].data_src[nchan].pixel[nclus].sync != CLUSTER_SYNC
		    || det[nd].data_src[nchan].pixel[nclus].length != clusDef[nc].length )
		    continue;
	       ptr_chan_src = &det[nd].data_src[nchan].pixel[nclus];

	       (void) memcpy( &chan_hdr[numHDR], 
			      &det[nd].data_src[nchan].hdr, 
			      sizeof( struct chan_hdr ) );

	       if ( ptr_chan_src->co_adding == UCHAR_ONE ) {
		    register unsigned short np = 0;
		    unsigned char *cpntr = ptr_chan_src->data;

		    do {
			 *ptr_short++ = (unsigned short) cpntr[1]
			      + ((unsigned short) cpntr[0] << 8);
			 cpntr += 2;
		    } while ( ++np < ptr_chan_src->length );
	       } else {
		    register unsigned short np = 0;
		    unsigned char *cpntr = ptr_chan_src->data;

		    do {
			 *ptr_int++ = (unsigned int) cpntr[2]
			      + ((unsigned int) cpntr[1] << 8)
			      + ((unsigned int) cpntr[0] << 16);
			 cpntr += 3;
		    } while ( ++np < ptr_chan_src->length );
	       }
	       numHDR++;
	  }
/*
 * create group to store cluster data
 */
	  if ( numHDR > 0 ) {
	       hid_t   pt_clus, pt_data;
	       hsize_t adim;

	       const char clusName[] = "mds0_cluster";

	       subgrpID = NADC_OPEN_HDF5_Group( grpID, grpName );
	       if ( subgrpID < 0 ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, grpName );
	       (void) H5LTset_attribute_uchar( grpID, grpName, "channel",
					       &clusDef[nc].chanID, 1 );
	       (void) H5LTset_attribute_uchar( grpID, grpName, "cluster_id",
					       &clusDef[nc].clusID, 1 );
	       (void) H5LTset_attribute_uchar( grpID, grpName, "coadd-factor",
					       &ptr_chan_src->co_adding, 1 );
	       (void) H5LTset_attribute_ushort( grpID, grpName, "sync",
						&ptr_chan_src->sync, 1 );
	       (void) H5LTset_attribute_ushort( grpID, grpName, "block_nr",
						&ptr_chan_src->block_nr, 1 );
	       (void) H5LTset_attribute_ushort( grpID, grpName, "start",
						&clusDef[nc].start, 1 );
	       (void) H5LTset_attribute_ushort( grpID, grpName, "length",
						&clusDef[nc].length, 1 );

	       if ( H5LTfind_dataset( subgrpID, clusName ) == 0 ) {
		    hid_t   tid;

		    tid = H5Tcreate( H5T_COMPOUND, sizeof(struct chan_hdr) );
		    (void) H5Tinsert( tid, "sync",
				      HOFFSET( struct chan_hdr, sync ),
				      H5T_NATIVE_USHORT );
		    (void) H5Tinsert( tid, "bcps",
				      HOFFSET( struct chan_hdr, bcps ),
				      H5T_NATIVE_USHORT );
		    (void) H5Tinsert( tid, "bias",
				      HOFFSET( struct chan_hdr, bias ),
				      H5T_NATIVE_USHORT );
		    (void) H5Tinsert( tid, "temp",
				      HOFFSET( struct chan_hdr, temp ),
				      H5T_NATIVE_USHORT );
		    (void) H5Tinsert( tid, "channel",
				      HOFFSET( struct chan_hdr, channel ),
				      H5T_NATIVE_USHORT );
		    (void) H5Tinsert( tid, "ratio_hdr",
				      HOFFSET( struct chan_hdr, ratio_hdr ),
				      H5T_NATIVE_USHORT );
		    (void) H5Tinsert( tid, "command_vis",
				      HOFFSET( struct chan_hdr, command_vis ),
				      H5T_NATIVE_UINT );
		    (void) H5Tinsert( tid, "command_ir",
				      HOFFSET( struct chan_hdr, command_ir ), 
				      H5T_NATIVE_UINT );

		    chunk_sz = numHDR;
		    pt_clus = H5PTcreate_fl( subgrpID, clusName, tid, 
					     chunk_sz, compress );
		    (void) H5Tclose( tid );
		    if ( pt_clus == H5I_BADID )
			 NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DATA, clusName );
		    
		    adim = ptr_chan_src->length;
		    if ( ptr_chan_src->co_adding == UCHAR_ONE )	
			 tid = H5Tarray_create( H5T_NATIVE_USHORT, 1, &adim );
		    else
			 tid = H5Tarray_create( H5T_NATIVE_UINT, 1, &adim );
		    pt_data = H5PTcreate_fl( subgrpID, "mds0_data", tid,
					     chunk_sz, compress );
		    (void) H5Tclose( tid );
		    if ( pt_data == H5I_BADID )
			 NADC_GOTO_ERROR(prognm,NADC_ERR_HDF_DATA,"mds0_data");
	       } else {
		    pt_clus = H5PTopen( subgrpID, clusName );
		    if ( pt_clus == H5I_BADID )
			 NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DATA, clusName );
		    pt_data = H5PTopen( subgrpID, "mds0_data" );
		    if ( pt_data == H5I_BADID )
			 NADC_GOTO_ERROR(prognm,NADC_ERR_HDF_DATA,"mds0_data");
	       }
	       if ( H5PTappend( pt_clus, numHDR, chan_hdr ) < 0 )
		    NADC_ERROR( prognm, NADC_ERR_HDF_WR, clusName );
	       if ( H5PTappend( pt_data, numHDR, data ) < 0 )
		    NADC_ERROR( prognm, NADC_ERR_HDF_WR, "mds0_data" );
	       (void) H5PTclose( pt_clus );
	       (void) H5PTclose( pt_data );
	       if ( H5Gclose( subgrpID ) < 0 )
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, grpName );
	  }
	  free( data );
     }
/*
 * close interface
 */
 done:
     if ( chan_hdr != NULL ) free( chan_hdr );
     if ( H5Gclose( grpID ) < 0 ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, grpName );
}
