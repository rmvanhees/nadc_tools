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

.IDENTifer   SCIA_LV0_H5_STRUCTS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIAMACHY level 0/1b/2, HDF5
.LANGUAGE    ANSI C
.PURPOSE     create data structures to store SCIAMACHY compound data types

.INPUT/OUTPUT
  call as   CRE_SCIA_LV0_H5_STRUCTS( param );
            CRE_SCIA_LV1_H5_STRUCTS( param );
            CRE_SCIA_LV2_H5_STRUCTS( param );
            CRE_SCIA_OL2_H5_STRUCTS( param );
     input:  
            struct param_record param : struct holding user-defined settings

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      4.0   07-Dec-2005	removed esig/esigc from MDS(1b)-struct,
				renamed pixel_val_err to pixel_err, RvH
              3.6   07-Nov-2002	added pixel_type to geoN, RvH
              3.5   19-Mar-2002	replaced confusing 
                                   bench_obm with bench_rad, RvH
              3.4   07-Mar-2002	gave MDS structs more logical names, RvH 
              3.3   28-Feb-2002	modified struct mds1_scia, RvH 
              3.2   21-Feb-2002	completed implementation & 
                       combined level 0/1b/2, RvH 
              3.1   13-Feb-2002	added level 0 structs for AUX and PMD MDS, RvH 
              3.0   11-Feb-2002	combined level 0 and 1b structs, RvH 
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH
              1.1   13-Sep-2001	separate module for level 1b and 2 data, RvH 
              1.0   08-Jun-2001 created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_SOURCE 2

/*+++++ System headers +++++*/
#include <hdf5.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#define _SCIA_LEVEL_2
#include <nadc_scia.h>

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   CRE_SCIA_H5_STRUCT_MJD
.PURPOSE     define HDF5 data type to store SCIAMACHY MJD compound data type
.INPUT/OUTPUT
  call as   CRE_SCIA_H5_MJD_STRUCT_MJD( file_id, &mjd_id );
            
.RETURNS     Nothing
.COMMENT     static function
-------------------------*/
static
void CRE_SCIA_H5_STRUCT_MJD( hid_t file_id, /*@out@*/ hid_t *mjd_id )
{
     *mjd_id = H5Tcreate( H5T_COMPOUND, sizeof( struct mjd_envi ));
     (void) H5Tinsert( *mjd_id, "days", 
		       HOFFSET(struct mjd_envi, days),
		       H5T_NATIVE_INT );
     (void) H5Tinsert( *mjd_id, "secnd", 
		       HOFFSET(struct mjd_envi, secnd),
		       H5T_NATIVE_UINT );
     (void) H5Tinsert( *mjd_id, "musec", 
		       HOFFSET(struct mjd_envi, musec),
		       H5T_NATIVE_UINT );
     (void) H5Tcommit( file_id, "mjd", *mjd_id, 
		       H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
}

/*+++++++++++++++++++++++++
.IDENTifer   CRE_SCIA_H5_STRUCT_COORD
.PURPOSE     define HDF5 data type to store SCIAMACHY Coord compound data type
.INPUT/OUTPUT
  call as   CRE_SCIA_H5_MJD_STRUCT_COORD( file_id, &coord_id );
            
.RETURNS     Nothing
.COMMENT     static function
-------------------------*/
static
void CRE_SCIA_H5_STRUCT_COORD( hid_t file_id, /*@out@*/ hid_t *coord_id )
{
     *coord_id = H5Tcreate( H5T_COMPOUND, sizeof( struct coord_envi ));
     (void) H5Tinsert( *coord_id, "lat", 
		       HOFFSET(struct coord_envi, lat), 
		       H5T_NATIVE_INT );
     (void) H5Tinsert( *coord_id, "lon", 
		       HOFFSET(struct coord_envi, lon), 
		       H5T_NATIVE_INT );
     (void) H5Tcommit( file_id, "coord", *coord_id,
		       H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
}

/*+++++++++++++++++++++++++
.IDENTifer   CRE_SCIA_H5_STRUCTS_LV01
.PURPOSE     define HDF5 data types to store SCIAMACHY compound data types
             (only those common in level 0 and 1b data products)
.INPUT/OUTPUT
  call as   CRE_SCIA_H5_STRUCTS_LV01( file_id );

.RETURNS     Nothing
.COMMENT     static function
-------------------------*/
static
void CRE_SCIA_H5_STRUCTS_LV01( hid_t file_id )
{
     hid_t   bcp_id, pmd_id, pmtc_id;
     hid_t   arr_id, type_id;
     hsize_t adim, dims[2];
/*
 * Auxiliary BCP
 */
     bcp_id = H5Tcreate( H5T_COMPOUND, sizeof( struct aux_bcp ));
     (void) H5Tinsert( bcp_id, "sync",
		       HOFFSET(struct aux_bcp, sync),
		       H5T_NATIVE_USHORT );
     (void) H5Tinsert( bcp_id, "counter",
		       HOFFSET(struct aux_bcp, bcps),
		       H5T_NATIVE_USHORT );
     (void) H5Tinsert( bcp_id, "flags",
		       HOFFSET(struct aux_bcp, flags),
		       H5T_NATIVE_USHORT );
     (void) H5Tinsert( bcp_id, "azi_encode_cntr",
		       HOFFSET(struct aux_bcp, azi_encode_cntr),
		       H5T_NATIVE_UINT );
     (void) H5Tinsert( bcp_id, "ele_encode_cntr",
		       HOFFSET(struct aux_bcp, ele_encode_cntr),
		       H5T_NATIVE_UINT );
     (void) H5Tinsert( bcp_id, "azi_cntr_error",
		       HOFFSET(struct aux_bcp, azi_cntr_error),
		       H5T_NATIVE_USHORT );
     (void) H5Tinsert( bcp_id, "ele_cntr_error",
		       HOFFSET(struct aux_bcp, ele_cntr_error),
		       H5T_NATIVE_USHORT );
     (void) H5Tinsert( bcp_id, "azi_scan_error",
		       HOFFSET(struct aux_bcp, azi_scan_error),
		       H5T_NATIVE_USHORT );
     (void) H5Tinsert( bcp_id, "ele_scan_error",
		       HOFFSET(struct aux_bcp, ele_scan_error),
		       H5T_NATIVE_USHORT );
     (void) H5Tcommit( file_id, "aux_bcp", bcp_id,
		       H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
/*
 * PMTC frame
 */
     pmtc_id = H5Tcreate( H5T_COMPOUND, sizeof( struct pmtc_frame ));
     adim = NUM_LV0_AUX_BCP;
     arr_id = H5Tarray_create( bcp_id, 1, &adim );
     (void) H5Tinsert( pmtc_id, "bcp",
		       HOFFSET(struct pmtc_frame, bcp), arr_id );
     (void) H5Tclose( arr_id );
     (void) H5Tinsert( pmtc_id, "bench_rad",
		       HOFFSET(struct pmtc_frame, bench_rad), 
		       H5T_NATIVE_USHORT );
     (void) H5Tinsert( pmtc_id, "bench_elv",
		       HOFFSET(struct pmtc_frame, bench_elv), 
		       H5T_NATIVE_USHORT );
     (void) H5Tinsert( pmtc_id, "bench_az",
		       HOFFSET(struct pmtc_frame, bench_az), 
		       H5T_NATIVE_USHORT );
     (void) H5Tcommit( file_id, "pmtc_frame", pmtc_id,
		       H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
/*
 * PMD data packets
 */
     pmd_id = H5Tcreate( H5T_COMPOUND, sizeof( struct pmd_data ));
     (void) H5Tinsert( pmd_id, "sync",
		       HOFFSET(struct pmd_data, sync), 
		       H5T_NATIVE_USHORT );
     dims[0] = 2;
     dims[1] = 7;
     arr_id = H5Tarray_create( H5T_NATIVE_USHORT, 2, dims );
     (void) H5Tinsert( pmd_id, "data",
		       HOFFSET(struct pmd_data, data), arr_id );
     (void) H5Tclose( arr_id );
     (void) H5Tinsert( pmd_id, "mdi",
		       HOFFSET(struct pmd_data, bcps), 
		       H5T_NATIVE_USHORT );
     (void) H5Tinsert( pmd_id, "time",
		       HOFFSET(struct pmd_data, time), 
		       H5T_NATIVE_USHORT );
     (void) H5Tcommit( file_id, "pmd_data", pmd_id,
		       H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
/*
 * PMD data source packets
 */
     type_id = H5Tcreate( H5T_COMPOUND, sizeof( struct pmd_src ));
     (void) H5Tinsert( type_id, "temp",
		       HOFFSET(struct pmd_src, temp), 
		       H5T_NATIVE_USHORT );
     adim = NUM_LV0_PMD_PACKET;
     arr_id = H5Tarray_create( pmd_id, 1, &adim );
     (void) H5Tinsert( type_id, "packet",
		       HOFFSET(struct pmd_src, packet), arr_id );
     (void) H5Tclose( arr_id );
     (void) H5Tclose( pmd_id );
     (void) H5Tcommit( file_id, "pmd_src", type_id,
		       H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
     (void) H5Tclose( type_id );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   CRE_SCIA_LV1_H5_STRUCTS
.PURPOSE     create data structures to store SCIAMACHY compound data types
.INPUT/OUTPUT
  call as   CRE_SCIA_LV1_H5_STRUCTS( param );
     input:  
            struct param_record param : struct holding user-defined settings

.RETURNS     Nothing
.COMMENT     None
-------------------------*/
void CRE_SCIA_LV1_H5_STRUCTS( struct param_record param )
{
     hid_t   coord_id, mjd_id, type_id;
     hid_t   arr_id;
     hsize_t adim;
	  
/*
 * create structs common to level 0 and level 1b
 */
     CRE_SCIA_H5_STRUCT_MJD( param.hdf_file_id, &mjd_id );
     (void) H5Tclose( mjd_id );
     CRE_SCIA_H5_STRUCT_COORD( param.hdf_file_id, &coord_id );
     (void) H5Tclose( coord_id );
     CRE_SCIA_H5_STRUCTS_LV01( param.hdf_file_id );
/*
 * Packet header
 */
     type_id = H5Tcreate( H5T_COMPOUND, sizeof( struct packet_hdr ));
     (void) H5Tinsert( type_id, "api", 
                       HOFFSET(struct packet_hdr, api), H5T_NATIVE_USHORT );
     (void) H5Tinsert( type_id, "seq_cntrl", 
                       HOFFSET(struct packet_hdr, seq_cntrl), 
		       H5T_NATIVE_USHORT );
     (void) H5Tinsert( type_id, "length", 
                       HOFFSET(struct packet_hdr, length), H5T_NATIVE_USHORT );
     (void) H5Tcommit( param.hdf_file_id, "packet_hdr", type_id,
		       H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
     (void) H5Tclose( type_id );

/*
 * Data header
 */
     type_id = H5Tcreate( H5T_COMPOUND, sizeof( struct data_hdr ));
     (void) H5Tinsert( type_id, "length", 
                       HOFFSET(struct data_hdr, length), H5T_NATIVE_USHORT );
     (void) H5Tinsert( type_id, "category", 
                       HOFFSET(struct data_hdr, category), H5T_NATIVE_UCHAR );
     (void) H5Tinsert( type_id, "state_id", 
                       HOFFSET(struct data_hdr, state_id), H5T_NATIVE_UCHAR );
     (void) H5Tinsert( type_id, "on_board_time", 
                       HOFFSET(struct data_hdr, on_board_time), 
		       H5T_NATIVE_UINT );
     (void) H5Tinsert( type_id, "rdv", 
                       HOFFSET(struct data_hdr, rdv), H5T_NATIVE_USHORT );
     (void) H5Tinsert( type_id, "id", 
                       HOFFSET(struct data_hdr, id), H5T_NATIVE_USHORT );
     (void) H5Tcommit( param.hdf_file_id, "data_hdr", type_id,
		       H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
     (void) H5Tclose( type_id );
/*
 * PMTC header
 */
     type_id = H5Tcreate( H5T_COMPOUND, sizeof( struct pmtc_hdr ));
     (void) H5Tinsert( type_id, "pmtc_1", 
                       HOFFSET(struct pmtc_hdr, pmtc_1), H5T_NATIVE_USHORT );
     (void) H5Tinsert( type_id, "scanner_mode", 
                       HOFFSET(struct pmtc_hdr, scanner_mode), 
		       H5T_NATIVE_USHORT );
     (void) H5Tinsert( type_id, "az_param", 
                       HOFFSET(struct pmtc_hdr, az_param), 
		       H5T_NATIVE_USHORT );
     (void) H5Tinsert( type_id, "elv_param", 
                       HOFFSET(struct pmtc_hdr, elv_param), 
		       H5T_NATIVE_USHORT );
     adim = 6;
     arr_id = H5Tarray_create( H5T_NATIVE_UCHAR, 1, &adim );
     (void) H5Tinsert( type_id, "factor", 
                       HOFFSET(struct pmtc_hdr, factor), arr_id );
     (void) H5Tclose( arr_id );
     (void) H5Tcommit( param.hdf_file_id, "pmtc_hdr", type_id,
		       H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
     (void) H5Tclose( type_id );
/*
 * Clcon structure
 */
     type_id = H5Tcreate( H5T_COMPOUND, sizeof( struct Clcon_scia ));
     (void) H5Tinsert( type_id, "cluster_id", 
		       HOFFSET(struct Clcon_scia, id), 
		       H5T_NATIVE_UCHAR );
     (void) H5Tinsert( type_id, "chan_num", 
		       HOFFSET(struct Clcon_scia, channel), 
		       H5T_NATIVE_UCHAR );
     (void) H5Tinsert( type_id, "clus_data_type", 
		       HOFFSET(struct Clcon_scia, type), 
		       H5T_NATIVE_UCHAR );
     (void) H5Tinsert( type_id, "start_pix", 
		       HOFFSET(struct Clcon_scia, pixel_nr), 
		       H5T_NATIVE_USHORT );
     (void) H5Tinsert( type_id, "clus_len", 
		       HOFFSET(struct Clcon_scia, length), 
		       H5T_NATIVE_USHORT );
     (void) H5Tinsert( type_id, "intg_time", 
		       HOFFSET(struct Clcon_scia, intg_time), 
		       H5T_NATIVE_USHORT );
     (void) H5Tinsert( type_id, "coadd_factor", 
		       HOFFSET(struct Clcon_scia, coaddf), 
		       H5T_NATIVE_USHORT );
     (void) H5Tinsert( type_id, "num_readouts", 
		       HOFFSET(struct Clcon_scia, n_read), 
		       H5T_NATIVE_USHORT );
     (void) H5Tinsert( type_id, "pet", 
		       HOFFSET(struct Clcon_scia, pet), 
		       H5T_NATIVE_FLOAT );
     (void) H5Tcommit( param.hdf_file_id, "Clcon", type_id,
		       H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );    
     (void) H5Tclose( type_id );
/*
 * create the "Sig" structure
 */
     type_id = H5Tcreate( H5T_COMPOUND, sizeof( struct Sig_scia ));
     (void) H5Tinsert( type_id, "stray", 
		       HOFFSET(struct Sig_scia, stray), 
		       H5T_NATIVE_UCHAR );
     (void) H5Tinsert( type_id, "corr", 
		       HOFFSET(struct Sig_scia, corr), 
		       H5T_NATIVE_SCHAR );
     (void) H5Tinsert( type_id, "sign", 
		       HOFFSET(struct Sig_scia, sign), 
		       H5T_NATIVE_USHORT );
     (void) H5Tpack( type_id );
     (void) H5Tcommit( param.hdf_file_id, "Sig", type_id,
		       H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );    
     (void) H5Tclose( type_id );
/*
 * create the "Sigc" structure
 */
     type_id = H5Tcreate( H5T_COMPOUND, sizeof( struct Sigc_scia ));
     (void) H5Tinsert( type_id, "stray", 
		       HOFFSET(struct Sigc_scia, stray), 
		       H5T_NATIVE_UCHAR );
     (void) H5Tinsert( type_id, "det", 
		       HOFFSET(struct Sigc_scia, det), 
		       H5T_NATIVE_UINT );
     (void) H5Tpack( type_id );
     (void) H5Tcommit( param.hdf_file_id, "Sigc", type_id,
		       H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );    
     (void) H5Tclose( type_id );
}

/*+++++++++++++++++++++++++
.IDENTifer   CRE_SCIA_LV2_H5_STRUCTS
.PURPOSE     create data structures to store SCIAMACHY compound data types
.INPUT/OUTPUT
  call as   CRE_SCIA_LV2_H5_STRUCTS( param );
     input:  
            struct param_record param : struct holding user-defined settings

.RETURNS     nothing
.COMMENTS    nothing
-------------------------*/
void CRE_SCIA_LV2_H5_STRUCTS( struct param_record param )
{
     hid_t   type_id;
     hid_t   arr_id;
     hid_t   mjd_id, coord_id;
     hsize_t adim;
/*
 * Create "mjd" and "coord" structure
 */
     CRE_SCIA_H5_STRUCT_MJD( param.hdf_file_id, &mjd_id );
     (void) H5Tclose( mjd_id );
     CRE_SCIA_H5_STRUCT_COORD( param.hdf_file_id, &coord_id );
     (void) H5Tclose( coord_id );
/*
 * create the "bias_win" structure
 */
     type_id = H5Tcreate( H5T_COMPOUND, sizeof( struct bias_record ));
     (void) H5Tinsert( type_id, "wv_min",
		       HOFFSET(struct bias_record, wv_min),
		       H5T_NATIVE_USHORT );
     (void) H5Tinsert( type_id, "wv_max",
		       HOFFSET(struct bias_record, wv_max),
		       H5T_NATIVE_USHORT );
     (void) H5Tinsert( type_id, "nr_micro",
		       HOFFSET(struct bias_record, nr_micro),
		       H5T_NATIVE_USHORT );
     adim = (hsize_t) MAX_BIAS_MICRO_WIN;
     arr_id = H5Tarray_create( H5T_NATIVE_USHORT, 1, &adim );
     (void) H5Tinsert( type_id, "micro_wv_min",
		       HOFFSET(struct bias_record, micro_min), arr_id );
     (void) H5Tinsert( type_id, "micro_wv_max",
		       HOFFSET(struct bias_record, micro_max), arr_id );
     (void) H5Tclose( arr_id );
     (void) H5Tcommit( param.hdf_file_id, "bias_win", type_id,
		       H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
     (void) H5Tclose( type_id );
/*
 * create the "doas_win" structure
 */
     type_id = H5Tcreate( H5T_COMPOUND, sizeof( struct doas_record ));
     (void) H5Tinsert( type_id, "wv_min",
		       HOFFSET(struct doas_record, wv_min),
		       H5T_NATIVE_USHORT );
     (void) H5Tinsert( type_id, "wv_max",
		       HOFFSET(struct doas_record, wv_max),
		       H5T_NATIVE_USHORT );
     (void) H5Tcommit( param.hdf_file_id, "doas_win", type_id,
		       H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
     (void) H5Tclose( type_id );
}

/*+++++++++++++++++++++++++
.IDENTifer   CRE_SCIA_OL2_H5_STRUCTS
.PURPOSE     create data structures to store SCIAMACHY compound data types
.INPUT/OUTPUT
  call as   CRE_SCIA_OL2_H5_STRUCTS( param );
     input:  
            struct param_record param : struct holding user-defined settings

.RETURNS     nothing
.COMMENTS    nothing
-------------------------*/
void CRE_SCIA_OL2_H5_STRUCTS( struct param_record param )
{
     hid_t   type_id;
     hid_t   arr_id;
     hid_t   mjd_id, coord_id;

     hsize_t num;
/*
 * Create "mjd" and "coord" structure
 */
     CRE_SCIA_H5_STRUCT_MJD( param.hdf_file_id, &mjd_id );
     CRE_SCIA_H5_STRUCT_COORD( param.hdf_file_id, &coord_id );
     (void) H5Tclose( coord_id );
/*
 * Create struct to hold Limb Profile Layer Record data
 */
     type_id = H5Tcreate( H5T_COMPOUND, sizeof( struct layer_rec ));
     (void) H5Tinsert( type_id, "tangvmr",
		       HOFFSET( struct layer_rec, tangvmr ),
		       H5T_NATIVE_FLOAT );
     (void) H5Tinsert( type_id, "errtangvmr",
		       HOFFSET( struct layer_rec, errtangvmr ),
		       H5T_NATIVE_FLOAT );
     (void) H5Tinsert( type_id, "vertcol",
		       HOFFSET( struct layer_rec, vertcol ),
		       H5T_NATIVE_FLOAT );
     (void) H5Tinsert( type_id, "errvertcol",
		       HOFFSET( struct layer_rec, errvertcol ),
		       H5T_NATIVE_FLOAT );
     (void) H5Tcommit( param.hdf_file_id, "layer_rec", type_id,
		       H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
     (void) H5Tclose( type_id );     
/*
 * Create struct to hold Measurement Grid Record data
 */
     type_id = H5Tcreate( H5T_COMPOUND, sizeof( struct meas_grid ));
     (void) H5Tinsert( type_id, "starttime",
		       HOFFSET( struct meas_grid, mjd ),
		       mjd_id );
     (void) H5Tinsert( type_id, "tangh",
		       HOFFSET( struct meas_grid, tangh ),
		       H5T_NATIVE_FLOAT );
     (void) H5Tinsert( type_id, "tangp",
		       HOFFSET( struct meas_grid, tangp ),
		       H5T_NATIVE_FLOAT );
     (void) H5Tinsert( type_id, "tangt",
		       HOFFSET( struct meas_grid, tangt ),
		       H5T_NATIVE_FLOAT );
     (void) H5Tinsert( type_id, "num_win",
		       HOFFSET( struct meas_grid, num_win ),
		       H5T_NATIVE_UCHAR );
     (void) H5Tinsert( type_id, "winmin",
		       HOFFSET( struct meas_grid, win_limits ),
		       H5T_NATIVE_FLOAT );
     (void) H5Tinsert( type_id, "winmax",
		       HOFFSET( struct meas_grid, win_limits ) + sizeof(float),
		       H5T_NATIVE_FLOAT );
     (void) H5Tcommit( param.hdf_file_id, "meas_grid", type_id,
		       H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
     (void) H5Tclose( type_id );     
/*
 * Create struct to hold State Vector Record data
 */
     type_id = H5Tcreate( H5T_COMPOUND, sizeof( struct state_vec ));
     (void) H5Tinsert( type_id, "value",
		       HOFFSET( struct state_vec, value ),
		       H5T_NATIVE_FLOAT );
     (void) H5Tinsert( type_id, "error",
		       HOFFSET( struct state_vec, error ),
		       H5T_NATIVE_FLOAT );
     num = 4;
     arr_id = H5Tarray_create( H5T_NATIVE_UCHAR, 1, &num );
     (void) H5Tinsert( type_id, "type",
		       HOFFSET( struct state_vec, type ), arr_id );
     (void) H5Tcommit( param.hdf_file_id, "state_vec", type_id,
		       H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
     (void) H5Tclose( type_id );     
     (void) H5Tclose( arr_id );
/*
 * close file interface
 */
     (void) H5Tclose( mjd_id );
}
