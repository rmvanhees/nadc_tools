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

.IDENTifer   SCIA_LV1_WR_H5_MDS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1b/1c - HDF5
.LANGUAGE    ANSI C
.PURPOSE     write SCIAMACHY level 1 Measurements Data Sets
.COMMENTS    Contains SCIA_LV1_RD_H5_MDS, SCIA_LV1_WR_H5_MDS, 
                       SCIA_LV1C_WR_H5_MDS, SCIA_LV1C_WR_H5_MDS_PMD, 
		       SCIA_LV1C_WR_H5_MDS_POLV
.ENVIRONment None
.VERSION      5.2   08-Oct-2008 added headers to modules, RvH
              5.1   13-Jun-2006 fixed several bugs, write Sig(c) with hdf5_hl
              5.0   07-Dec-2005 removed esig/esigc from MDS(1b)-struct,
				renamed pixel_val_err to pixel_err, RvH
              4.1   17-Oct-2005 write only one MDS_PMD or MDS_POLV record, RvH
              4.0   11-Oct-2005 added attribute "const" to MDS-parameter
                                pass state-record always by reference, RvH
              3.0   27-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              2.6   20-Mar-2003	Level 1c routines only require an unique 
                                ID (= index) of a state, RvH
              2.5   14-Nov-2002	removed redundant channed group, RvH
              2.4   09-Aug-2002	do not attempt to write level 1c 
                                PMD/polV for monitoring states, RvH
              2.3   02-Aug-2002	update for cluster selection, RvH 
              2.2   23-Apr-2002	groups now correctly reflect channel/cluster 
                                IDs, RvH
              2.1   28-Feb-2002	modified struct mds1_scia & HDF layout, RvH 
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              1.1   03-Aug-2001	Ahhhrgg SCIA_L01 format, RvH 
              1.0   09-Jan-2001 created by R. M. van Hees
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
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

/*
 * level 1b/c hdf5_hl table definitions
 */
#define NFIELD_GEOC   4
#define NFIELD_GEOL  14
#define NFIELD_GEON  12
#define NFIELD_L0HDR  6
#define NFIELD_POLV   7
#define NFIELD_SIG    3
#define NFIELD_SIGC   2
#define NFIELD_LV1    7
#define NFIELD_LV1C   7

static const size_t mds1_size = sizeof( struct mds1_scia );
static const size_t mds1_offs[NFIELD_LV1] = {
     HOFFSET( struct mds1_scia, mjd ),
     HOFFSET( struct mds1_scia, quality_flag ),
     HOFFSET( struct mds1_scia, n_aux ),
     HOFFSET( struct mds1_scia, n_pmd ),
     HOFFSET( struct mds1_scia, n_pol ),
     HOFFSET( struct mds1_scia, dsr_length ),
     HOFFSET( struct mds1_scia, scale_factor )
};

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_WR_H5_GEOC
.PURPOSE     write Calibration/Monitoring geolocation as HDF5 table
.INPUT/OUTPUT
  call as    SCIA_WR_H5_GEOC( file_id, grp_id, compress, nr_geo, geo );
     input:
            hid_t file_id         :
	    hid_t grp_id          :
	    hbool_t compress      :
	    unsigned int nr_geo   :
	    struct geoC_scia *geo :

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void SCIA_WR_H5_GEOC( hid_t file_id, hid_t grp_id, hbool_t compress, 
		      unsigned int nr_geo, const struct geoC_scia *geo )
{
     register unsigned int nr;

     hid_t geoC_type[NFIELD_GEOC];

     const size_t geoC_size = sizeof( struct geoC_scia );
     const size_t geoC_offs[NFIELD_GEOC] = {
	  HOFFSET( struct geoC_scia, pos_esm ),
	  HOFFSET( struct geoC_scia, pos_asm ),
	  HOFFSET( struct geoC_scia, sun_zen_ang ),
	  HOFFSET( struct geoC_scia, sub_sat_point )
     };
     const char *geoC_names[NFIELD_GEOC] = {
          "pos_esm", "pos_asm", "sun_zen_ang", "sub_sat_point"
     };
/*
 * check number of records
 */
     if ( nr_geo == 0 ) return;
/*
 * define user-defined data types of the Table-fields
 */
     geoC_type[0] = H5Tcopy( H5T_NATIVE_FLOAT );
     geoC_type[1] = H5Tcopy( H5T_NATIVE_FLOAT );
     geoC_type[2] = H5Tcopy( H5T_NATIVE_FLOAT );
     geoC_type[3] = H5Topen( file_id, "coord", H5P_DEFAULT );
/*
 * create table
 */
     (void) H5TBmake_table( "GeoCal", grp_id, "geoC_scia", NFIELD_GEOC,
			    nr_geo, geoC_size, geoC_names, geoC_offs,
			    geoC_type, nr_geo, NULL, compress, geo );
/*
 * close interface
 */
     for ( nr = 0; nr < NFIELD_GEOC; nr++ ) (void) H5Tclose( geoC_type[nr] );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_WR_H5_GEOL
.PURPOSE     write Limb/Occultation geolocation as HDF5 table
.INPUT/OUTPUT
  call as    SCIA_WR_H5_GEOL( file_id, grp_id, compress, nr_geo, geo );
     input:
            hid_t file_id         :
	    hid_t grp_id          :
	    hbool_t compress      :
	    unsigned int nr_geo   :
	    struct geoL_scia *geo :

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void SCIA_WR_H5_GEOL( hid_t file_id, hid_t grp_id, hbool_t compress, 
		      unsigned int nr_geo, const struct geoL_scia *geo )
{
     register unsigned int nr;

     hid_t   coord_id; 
     hid_t   geoL_type[NFIELD_GEOL];
     hsize_t adim;

     const size_t geoL_size = sizeof( struct geoL_scia );
     const size_t geoL_offs[NFIELD_GEOL] = {
	  HOFFSET( struct geoL_scia, pixel_type ),
	  HOFFSET( struct geoL_scia, glint_flag ),
	  HOFFSET( struct geoL_scia, pos_esm ),
	  HOFFSET( struct geoL_scia, pos_asm ),
	  HOFFSET( struct geoL_scia, sat_h ),
	  HOFFSET( struct geoL_scia, earth_rad ),
	  HOFFSET( struct geoL_scia, dopp_shift ),
	  HOFFSET( struct geoL_scia, sun_zen_ang ),
	  HOFFSET( struct geoL_scia, sun_azi_ang ),
	  HOFFSET( struct geoL_scia, los_zen_ang ),
	  HOFFSET( struct geoL_scia, los_azi_ang ),
	  HOFFSET( struct geoL_scia, tan_h ),
	  HOFFSET( struct geoL_scia, sub_sat_point ),
	  HOFFSET( struct geoL_scia, tang_ground_point )
     };
     const char *geoL_names[NFIELD_GEOL] = {
          "pixel_type", "glint_flag", "pos_esm", "pos_asm", "sat_h", 
	  "earth_rad", "dopp_shift", "sun_zen_ang", "sun_azi_ang", 
	  "los_zen_ang", "los_azi_ang", "tan_h", "sub_sat_point",
	  "tang_ground_point"
     };
/*
 * check number of records
 */
     if ( nr_geo == 0 ) return;
/*
 * define user-defined data types of the Table-fields
 */
     geoL_type[0] = H5Tcopy( H5T_NATIVE_UCHAR );
     geoL_type[1] = H5Tcopy( H5T_NATIVE_UCHAR );
     geoL_type[2] = H5Tcopy( H5T_NATIVE_FLOAT );
     geoL_type[3] = H5Tcopy( H5T_NATIVE_FLOAT );
     geoL_type[4] = H5Tcopy( H5T_NATIVE_FLOAT );
     geoL_type[5] = H5Tcopy( H5T_NATIVE_FLOAT );
     geoL_type[6] = H5Tcopy( H5T_NATIVE_FLOAT );
     adim = 3;
     geoL_type[7] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     geoL_type[8] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     geoL_type[9] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     geoL_type[10] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     geoL_type[11] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     coord_id = H5Topen( file_id, "coord", H5P_DEFAULT );
     geoL_type[12] = H5Tcopy( coord_id );
     geoL_type[13] = H5Tarray_create( coord_id, 1, &adim );
     (void) H5Tclose( coord_id );
/*
 * create table
 */
     (void) H5TBmake_table( "GeoCal", grp_id, "geoL_scia", NFIELD_GEOL,
			    nr_geo, geoL_size, geoL_names, geoL_offs,
			    geoL_type, nr_geo, NULL, compress, geo );
/*
 * close interface
 */
     for ( nr = 0; nr < NFIELD_GEOL; nr++ ) (void) H5Tclose( geoL_type[nr] );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_WR_H5_GEON
.PURPOSE     write Nadir geolocation as HDF5 table
.INPUT/OUTPUT
  call as    SCIA_WR_H5_GEON( file_id, grp_id, compress, nr_geo, geo );
     input:
            hid_t file_id         :
	    hid_t grp_id          :
	    hbool_t compress      :
	    unsigned int nr_geo   :
	    struct geoN_scia *geo :

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void SCIA_WR_H5_GEON( hid_t file_id, hid_t grp_id, hbool_t compress, 
		      unsigned int nr_geo, const struct geoN_scia *geo )
{
     register unsigned int nr;

     hid_t   coord_id; 
     hid_t   geoN_type[NFIELD_GEON];
     hsize_t adim;

     const size_t geoN_size = sizeof( struct geoN_scia );
     const size_t geoN_offs[NFIELD_GEON] = {
	  HOFFSET( struct geoN_scia, pixel_type ),
	  HOFFSET( struct geoN_scia, glint_flag ),
	  HOFFSET( struct geoN_scia, pos_esm ),
	  HOFFSET( struct geoN_scia, sat_h ),
	  HOFFSET( struct geoN_scia, earth_rad ),
	  HOFFSET( struct geoN_scia, sun_zen_ang ),
	  HOFFSET( struct geoN_scia, sun_azi_ang ),
	  HOFFSET( struct geoN_scia, los_zen_ang ),
	  HOFFSET( struct geoN_scia, los_azi_ang ),
	  HOFFSET( struct geoN_scia, sub_sat_point ),
	  HOFFSET( struct geoN_scia, corner ),
	  HOFFSET( struct geoN_scia, center )
     };
     const char *geoN_names[NFIELD_GEON] = {
          "pixel_type", "glint_flag", "pos_esm", "sat_h", "earth_rad", 
	  "sun_zen_ang", "sun_azi_ang", "los_zen_ang", "los_azi_ang", 
	  "sub_sat_point", "corner_coord", "center_coord"
     };
/*
 * check number of records
 */
     if ( nr_geo == 0 ) return;
/*
 * define user-defined data types of the Table-fields
 */
     geoN_type[0] = H5Tcopy( H5T_NATIVE_UCHAR );
     geoN_type[1] = H5Tcopy( H5T_NATIVE_UCHAR );
     geoN_type[2] = H5Tcopy( H5T_NATIVE_FLOAT );
     geoN_type[3] = H5Tcopy( H5T_NATIVE_FLOAT );
     geoN_type[4] = H5Tcopy( H5T_NATIVE_FLOAT );
     adim = 3;
     geoN_type[5] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     geoN_type[6] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     geoN_type[7] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     geoN_type[8] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     coord_id = H5Topen( file_id, "coord", H5P_DEFAULT );
     geoN_type[9] = H5Tcopy( coord_id );
     adim = NUM_CORNERS;
     geoN_type[10] = H5Tarray_create( coord_id, 1, &adim );
     geoN_type[11] = H5Tcopy( coord_id );
     (void) H5Tclose( coord_id );
/*
 * create table
 */
     (void) H5TBmake_table( "Geo Nadir", grp_id, "geoN_scia", NFIELD_GEON,
			    nr_geo, geoN_size, geoN_names, geoN_offs,
			    geoN_type, nr_geo, NULL, compress, geo );
/*
 * close interface
 */
     for ( nr = 0; nr < NFIELD_GEON; nr++ ) (void) H5Tclose( geoN_type[nr] );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_WR_H5_L0HDR
.PURPOSE     write level 0 Data headers as HDF5 table
.INPUT/OUTPUT
  call as    SCIA_WR_H5_L0HDR( file_id, grp_id, compress, nr_hdr, lv0 );
     input:
            hid_t file_id       :
	    hid_t grp_id        :
	    hbool_t compress    :
	    unsigned int nr_hdr :
	    struct lv0_hdr *lv0 :

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void SCIA_WR_H5_L0HDR( hid_t file_id, hid_t grp_id, hbool_t compress, 
		       unsigned int nr_hdr, const struct lv0_hdr *lv0 )
{
     register unsigned int nr;

     hid_t   type_id; 
     hid_t   lv0_type[NFIELD_L0HDR];
     hsize_t adim;

     const size_t lv0_size = sizeof( struct lv0_hdr );
     const size_t lv0_offs[NFIELD_L0HDR] = {
	  HOFFSET( struct lv0_hdr, bcps ),
	  HOFFSET( struct lv0_hdr, num_chan ),
	  HOFFSET( struct lv0_hdr, orbit_vector ),
	  HOFFSET( struct lv0_hdr, packet_hdr ),
	  HOFFSET( struct lv0_hdr, data_hdr ),
	  HOFFSET( struct lv0_hdr, pmtc_hdr )
     };
     const char *lv0_names[NFIELD_L0HDR] = {
          "bcps", "num_chan", "orbit_vector", "packet_hdr", "data_hdr", 
	  "pmtc_hdr"
     };
/*
 * check number of records
 */
     if ( nr_hdr == 0 ) return;
/*
 * define user-defined data types of the Table-fields
 */
     lv0_type[0] = H5Tcopy( H5T_NATIVE_USHORT );
     lv0_type[1] = H5Tcopy( H5T_NATIVE_USHORT );
     adim = 8;
     lv0_type[2] = H5Tarray_create( H5T_NATIVE_INT, 1, &adim );
     type_id = H5Topen( file_id, "packet_hdr", H5P_DEFAULT );
     lv0_type[3] = H5Tcopy( type_id );
     (void) H5Tclose( type_id );
     type_id = H5Topen( file_id, "data_hdr", H5P_DEFAULT );
     lv0_type[4] = H5Tcopy( type_id );
     (void) H5Tclose( type_id );
     type_id = H5Topen( file_id, "pmtc_hdr", H5P_DEFAULT );
     lv0_type[5] = H5Tcopy( type_id );
     (void) H5Tclose( type_id );
/*
 * create table
 */
     (void) H5TBmake_table( "Level 0 Data Headers", grp_id, "lv0_hdr", 
			    NFIELD_L0HDR, nr_hdr, lv0_size, lv0_names, 
			    lv0_offs, lv0_type, nr_hdr, NULL, compress, lv0 );
/*
 * close interface
 */
     for ( nr = 0; nr < NFIELD_L0HDR; nr++ ) (void) H5Tclose( lv0_type[nr] );
     
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_WR_H5_POLV
.PURPOSE     write Fractional Polarisation measurements as HDF5 table
.INPUT/OUTPUT
  call as    SCIA_WR_H5_POLV( grp_id, compress, nr_polV, polV );
     input:
            hid_t grp_id           :
	    hbool_t compress       :
	    unsigned int nr_polV   :
	    struct polV_scia *polV :

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void SCIA_WR_H5_POLV( hid_t grp_id, hbool_t compress, 
		      unsigned int nr_polV, const struct polV_scia *polV )
{
     register unsigned int nr;

     hid_t   tid_gdf, polV_type[NFIELD_POLV];
     hsize_t adim;

     const size_t polV_size = sizeof( struct polV_scia );
     const size_t polV_offs[NFIELD_POLV] = {
	  HOFFSET( struct polV_scia, Q ),
	  HOFFSET( struct polV_scia, error_Q ),
	  HOFFSET( struct polV_scia, U ),
	  HOFFSET( struct polV_scia, error_U ),
	  HOFFSET( struct polV_scia, rep_wv ),
	  HOFFSET( struct polV_scia, gdf ),
	  HOFFSET( struct polV_scia, intg_time )
     };
     const char *polV_names[NFIELD_POLV] = {
          "Q", "error_Q", "U", "error_U", "rep_wv", "gdf", "intg_time"
     };
/*
 * check number of records
 */
     if ( nr_polV == 0 ) return;
/*
 * create compound gdf_para
 */
     tid_gdf = H5Tcreate( H5T_COMPOUND, sizeof(struct gdf_para) );
     (void) H5Tinsert( tid_gdf, "p_bar", 
		       HOFFSET(struct gdf_para, p_bar), H5T_NATIVE_FLOAT );
     (void) H5Tinsert( tid_gdf, "beta", 
		       HOFFSET(struct gdf_para, beta), H5T_NATIVE_FLOAT );
     (void) H5Tinsert( tid_gdf, "w0", 
		       HOFFSET(struct gdf_para, w0), H5T_NATIVE_FLOAT );
/*
 * define user-defined data types of the Table-fields
 */
     adim = NUM_FRAC_POLV;
     polV_type[0] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     polV_type[1] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     polV_type[2] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     polV_type[3] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     adim = NUM_FRAC_POLV+1;
     polV_type[4] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     polV_type[5] = H5Tcopy( tid_gdf );
     polV_type[6] = H5Tcopy( H5T_NATIVE_USHORT );
     (void) H5Tclose( tid_gdf );
/*
 * create table
 */
     (void) H5TBmake_table( "PolVal", grp_id, "polV_scia", NFIELD_POLV,
			    nr_polV, polV_size, polV_names, polV_offs,
			    polV_type, nr_polV, NULL, compress, polV );
/*
 * close interface
 */
     for ( nr = 0; nr < NFIELD_POLV; nr++ ) (void) H5Tclose( polV_type[nr] );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_H5_MDS
.PURPOSE     read SCIAMACHY level 1 Measurements Data Sets
.INPUT/OUTPUT
  call as    num_mds = SCIA_LV1_RD_H5_MDS( scia_fl, state, mds_1b );
     input:  
             char *scia_fl             : name of the product
	     struct state1_scia *state : states in the product
	     struct mds1_scia  *mds_1b : MDS struct (level 1b)

.RETURNS     Number of MDS records read from the product (unsigned int), 
             error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
unsigned int SCIA_LV1_RD_H5_MDS( const char *scia_fl, 
				 const struct state1_scia *state,
				 struct mds1_scia **mds_out )
{
     const char prognm[] = "SCIA_LV1_RD_H5_MDS";

     char grp_name[10];

     struct mds1_scia *mds = NULL;
/*
 * initialise some variables
 */
     hid_t   file_id = -1; 
     hid_t   grp_id = -1; 
     hid_t   subgrp_id = -1;
/*
 * constants
 */
     const int source = (int) state->type_mds;
     const unsigned int num_mds = state->num_dsr;
     const size_t mds1_sizes[NFIELD_LV1] = {
	  sizeof( mds->mjd ),
	  sizeof( mds->quality_flag ),
	  sizeof( mds->n_aux ),
	  sizeof( mds->n_pmd ),
	  sizeof( mds->n_pol ),
	  sizeof( mds->dsr_length ),
	  sizeof( mds->scale_factor )
     };
/*
 * check MDS size and pointer for output
 */
     if ( num_mds == 0 || mds_out == NULL ) {
          if ( mds_out != NULL ) *mds_out = NULL;
          return 0u;
     }
/*
 * allocate memory to store output records
 */
     *mds_out = (struct mds1_scia *) 
          malloc( num_mds * sizeof(struct mds1_scia));
     if ( (mds = *mds_out) == NULL ) 
          NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "mds1_scia" );
/*
 * open output HDF5-file
 */
     file_id = H5Fopen( scia_fl, H5F_ACC_RDONLY, H5P_DEFAULT );
     if ( file_id < 0 )
          NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, scia_fl );
/*
 * open group /MDS
 */
     grp_id = NADC_OPEN_HDF5_Group( file_id, "/MDS" );
     if ( grp_id < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_GRP, "/MDS" );
     switch ( source ) {
     case SCIA_NADIR:
	  (void) snprintf( grp_name, 10, "nadir_%02u", state->indx );
	  break;
     case SCIA_LIMB:
	  (void) snprintf( grp_name, 10, "limb_%02u", state->indx );
	  break;
     case SCIA_MONITOR:
	  (void) snprintf( grp_name, 10, "moni_%02u", state->indx );
	  break;
     case SCIA_OCCULT:
	  (void) snprintf( grp_name, 10, "occult_%02u", state->indx );
	  break;
     default:
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "unknown MDS type" );
     }
     subgrp_id = NADC_OPEN_HDF5_Group( grp_id, grp_name );
     if ( subgrp_id < 0 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_GRP, grp_name );
/*
 * define data types of table-fields
 */
     (void) H5TBread_table( subgrp_id, "mds_hdr", mds1_size, mds1_offs, 
                            mds1_sizes, mds );
/*
 * set return values
 */
 done:
     if ( subgrp_id >= 0 ) (void) H5Gclose( subgrp_id );
     if ( grp_id >= 0 ) (void) H5Gclose( grp_id );
     if ( file_id >= 0 ) (void) H5Fclose( file_id );

     mds = NULL;
     return num_mds;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_H5_MDS
.PURPOSE     write SCIAMACHY level 1 Measurements Data Sets
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_H5_MDS( param, num_mds, mds_1b );
     input:  
             struct param_record param : struct holding user-defined settings
	     unsigned short  num_mds   : number of MDS
	     struct mds1_scia  *mds_1b : MDS struct (level 1b)

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1_WR_H5_MDS( const struct param_record param, 
			 unsigned int num_mds, const struct mds1_scia *mds )
{
     const char prognm[] = "SCIA_LV1_WR_H5_MDS";

     register unsigned short nc;
     register hsize_t        nr, ny;

     char           clus_name[11];
     char           grp_name[10];

     unsigned char  *ucbuff;
     float          *rbuff;

     struct lv0_hdr   *lv0_hdr;
     struct polV_scia  *polV;
     struct geoN_scia  *geoN;
     struct geoL_scia  *geoL;
     struct geoC_scia  *geoC;

     hid_t   grp_id = -1; 
     hid_t   subgrp_id = -1;
     hbool_t compress;
     size_t  nrpix;
     hsize_t dims[2];
     hid_t   mds1_type[NFIELD_LV1];

     const char *mds1_names[NFIELD_LV1] = {
          "mjd", "quality_flag", "n_aux", "n_pmd", "n_pol", "dsr_length", 
	  "scale_factor"
     };
     const size_t sig_size = sizeof( struct Sig_scia );
     const size_t sig_offs[NFIELD_SIG] = {
	  HOFFSET( struct Sig_scia, stray ),
	  HOFFSET( struct Sig_scia, corr ),
	  HOFFSET( struct Sig_scia, sign )
     };
     const char *sig_names[NFIELD_SIG] = {
	  "stray", "corr", "sign"
     };
     const size_t sigc_size = sizeof( struct Sigc_scia );
     const size_t sigc_offs[NFIELD_SIGC] = {
	  HOFFSET( struct Sigc_scia, stray ),
	  HOFFSET( struct Sigc_scia, det )
     };
     const char *sigc_names[NFIELD_SIGC] = {
	  "stray", "det"
     };
#if !defined(__mips) && !defined (__hpux)
     const hid_t sig_type[NFIELD_SIG] = {
	  H5T_NATIVE_UCHAR, H5T_NATIVE_SCHAR, H5T_NATIVE_USHORT
     };
     const hid_t sigc_type[NFIELD_SIGC] = {
	  H5T_NATIVE_UCHAR, H5T_NATIVE_UINT
     };
#else
     hid_t sig_type[NFIELD_SIG];
     hid_t sigc_type[NFIELD_SIGC];

     sig_type[0] = H5T_NATIVE_UCHAR;
     sig_type[1] = H5T_NATIVE_SCHAR;
     sig_type[2] = H5T_NATIVE_USHORT;

     sigc_type[0] = H5T_NATIVE_UCHAR;
     sigc_type[1] = H5T_NATIVE_UINT;
#endif
/*
 * check number of MDS records
 */
     if ( num_mds == 0 ) return;
/*
 * set HDF5 boolean variable for compression
 */
     if ( param.flag_deflate == PARAM_SET )
          compress = TRUE;
     else
          compress = FALSE;
/*
 * open/create group /MDS
 */
     grp_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/MDS" );
     if ( grp_id < 0 ) NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "/MDS" );
     switch ( (int) mds->type_mds ) {
     case SCIA_NADIR:
	  (void) snprintf( grp_name, 10, "nadir_%02u", mds->state_index );
	  break;
     case SCIA_LIMB:
	  (void) snprintf( grp_name, 10, "limb_%02u", mds->state_index );
	  break;
     case SCIA_MONITOR:
	  (void) snprintf( grp_name, 10, "moni_%02u", mds->state_index );
	  break;
     case SCIA_OCCULT:
	  (void) snprintf( grp_name, 10, "occult_%02u", mds->state_index );
	  break;
     default:
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, "unknown MDS type" );
     }
     subgrp_id = NADC_OPEN_HDF5_Group( grp_id, grp_name );
     if ( subgrp_id < 0 ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, grp_name );
/*
 * define data types of table-fields
 */
     mds1_type[0] = H5Topen( param.hdf_file_id, "mjd", H5P_DEFAULT );
     mds1_type[1] = H5Tcopy( H5T_NATIVE_CHAR );
     mds1_type[2] = H5Tcopy( H5T_NATIVE_USHORT );
     mds1_type[3] = H5Tcopy( H5T_NATIVE_USHORT );
     mds1_type[4] = H5Tcopy( H5T_NATIVE_USHORT );
     mds1_type[5] = H5Tcopy( H5T_NATIVE_UINT );
     dims[0] = SCIENCE_CHANNELS;
     mds1_type[6] = H5Tarray_create( H5T_NATIVE_UCHAR, 1, dims );
/*
 * create table
 */
     (void) H5TBmake_table( "Detector LV1b MDS", subgrp_id, "mds_hdr", 
			    NFIELD_LV1, num_mds, mds1_size, mds1_names, 
			    mds1_offs, mds1_type, num_mds, NULL, compress, 
			    mds );
/*
 * close interface
 */
     for ( nr = 0; nr < NFIELD_LV1; nr++ ) (void) H5Tclose( mds1_type[nr] );
/*
 * write saturation flags
 */
     dims[0] = (hsize_t) num_mds;
     dims[1] = (hsize_t) mds->n_aux;
     nrpix = num_mds * mds->n_aux;
     ucbuff = (unsigned char *) malloc( nrpix );
     if ( ucbuff == NULL ) 
          NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "ucbuff" );
     for ( ny = nr = 0; nr < num_mds; nr++, ny += mds->n_aux )
          (void) memcpy( ucbuff+ny, mds[nr].sat_flags, mds->n_aux );

     (void) H5LTmake_dataset( subgrp_id, "sat_flags", 2, dims, 
			      H5T_NATIVE_UCHAR, ucbuff );
     free( ucbuff );
/*
 * write Red Grass flags
 */
     if ( (dims[1] = (hsize_t) (mds->n_aux * mds->n_clus)) > 0 ) {
	  nrpix = mds->n_aux * mds->n_clus;
	  if ( (ucbuff = (unsigned char *) malloc( num_mds * nrpix )) == NULL ) 
	       NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "ucbuff" );
	  for ( ny = nr = 0; nr < num_mds; nr++, ny += nrpix )
	       (void) memcpy( ucbuff+ny, mds[nr].red_grass, nrpix );

	  (void) H5LTmake_dataset( subgrp_id, "red_grass", 2, dims, 
				   H5T_NATIVE_UCHAR, ucbuff );
	  free( ucbuff );
     }
/*
 * write integrated PMD values
 */
     if ( (int) mds->type_mds != SCIA_MONITOR ) {
	  dims[1] = (hsize_t) mds->n_pmd;
	  nrpix = num_mds * mds->n_pmd;
	  if ( (rbuff = (float *) malloc( nrpix * sizeof( float ))) == NULL ) 
	       NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "rbuff" );
	  for ( ny = nr = 0; nr < num_mds; nr++, ny += mds->n_pmd ) {
	       (void) memcpy( rbuff+ny, mds[nr].int_pmd,
			      mds->n_pmd * sizeof( float ) );
	  }
	  (void) H5LTmake_dataset( subgrp_id, "int_pmd", 2, dims, 
				   H5T_NATIVE_FLOAT, ucbuff );
	  free( rbuff );
     }
/*
 * write level 0 product headers
 */
     nrpix = num_mds * mds->n_aux;
     lv0_hdr = (struct lv0_hdr *)
	  malloc( nrpix * sizeof( struct lv0_hdr ) );
     if ( lv0_hdr == NULL )
          NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "lv0_hdr" );
     for ( ny = nr = 0; nr < num_mds; nr++, ny += mds->n_aux ) {
          (void) memcpy( lv0_hdr+ny, mds[nr].lv0,
			 mds->n_aux * sizeof( struct lv0_hdr ) );
     }
     SCIA_WR_H5_L0HDR( param.hdf_file_id, subgrp_id, compress, 
		       nrpix, lv0_hdr );
     free( lv0_hdr );
/*
 * write gelocation records (Nadir/Limb/Monitor)
 */
     nrpix = num_mds * mds->n_aux;
     if ( (int) mds->type_mds == SCIA_NADIR ) {
	  geoN = (struct geoN_scia *)
	       malloc( nrpix * sizeof( struct geoN_scia ) );
	  if ( geoN == NULL ) 
	       NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "geoN" );
	  for ( ny = nr = 0; nr < num_mds; nr++, ny += mds->n_aux ) {
	       (void) memcpy( geoN+ny, mds[nr].geoN, 
			      mds->n_aux * sizeof( struct geoN_scia ) );
	  }
	  SCIA_WR_H5_GEON( param.hdf_file_id, subgrp_id, compress, 
			   nrpix, geoN );
	  free( geoN );
     } else if ( (int) mds->type_mds == SCIA_MONITOR ) {
	  geoC = (struct geoC_scia *)
	       malloc( nrpix * sizeof( struct geoC_scia ) );
	  if ( geoC == NULL ) 
	       NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "geoC" );
	  for ( ny = nr = 0; nr < num_mds; nr++, ny += mds->n_aux ) {
	       (void) memcpy( geoC+ny, mds[nr].geoC, 
			      mds->n_aux * sizeof( struct geoC_scia ) );
	  }
	  SCIA_WR_H5_GEOC( param.hdf_file_id, subgrp_id, compress, 
			   nrpix, geoC );
	  free( geoC );
     } else {
	  geoL = (struct geoL_scia *)
	       malloc( nrpix * sizeof( struct geoL_scia ) );
	  if ( geoL == NULL ) 
	       NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "geoL" );
	  for ( ny = nr = 0; nr < num_mds; nr++, ny += mds->n_aux ) {
	       (void) memcpy( geoL+ny, mds[nr].geoL, 
			      mds->n_aux * sizeof( struct geoL_scia ) );
	  }
	  SCIA_WR_H5_GEOL( param.hdf_file_id, subgrp_id, compress, 
			   nrpix, geoL );
	  free( geoL );
     }
/*
 * write fractional polarisation records
 */
     if ( (int) mds->type_mds != SCIA_MONITOR ) {
	  nrpix = num_mds * mds->n_pol;
	  polV = (struct polV_scia *) malloc(nrpix * sizeof(struct polV_scia));
          if ( polV == NULL ) 
               NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "polV" );
	  for ( ny = nr = 0; nr < num_mds; nr++, ny += mds->n_pol ) {
	       (void) memcpy( polV+ny, mds[nr].polV,
			      mds->n_pol * sizeof( struct polV_scia ) );
	  }
	  SCIA_WR_H5_POLV( subgrp_id, compress, nrpix, polV );
	  free( polV );
     }
/*
 * +++++ create/write datasets in the /MDS group
 *
 * write cluster data
 */
     for ( nc = 0; nc < mds->n_clus; nc++ ) {
/*
 * set name of the cluster
 */
	  (void) snprintf( clus_name, 11, "cluster_%02d", ((int) nc+1) );
/*
 * write reticon data
 */
	  if ( mds->clus[nc].n_sig > 0 ) {
	       unsigned int num_sig = num_mds * mds->clus[nc].n_sig;
	       size_t nr_byte = mds->clus[nc].n_sig * sizeof(struct Sig_scia);
	       size_t nr_byte_sig = num_mds * nr_byte;
	       struct Sig_scia *sig;

	       sig = (struct Sig_scia *) malloc( nr_byte_sig );
	       ny = nr = 0;
	       do {
		    (void) memcpy( sig+ny, mds[nr].clus[nc].sig, nr_byte );
		    ny += mds->clus[nc].n_sig;
	       } while ( ++nr < num_mds );
	       (void) H5TBmake_table( "Reticon/Epitax detector (not co-added)",
				      subgrp_id, clus_name, NFIELD_SIG, 
				      num_sig, sig_size, sig_names, 
				      sig_offs, sig_type, num_sig, 
				      NULL, FALSE, sig );
	       free( sig );
	  } else if ( mds->clus[nc].n_sigc > 0 ) {
	       unsigned int num_sigc = mds->clus[nc].n_sigc;
	       size_t nr_byte = 
		    mds->clus[nc].n_sigc * sizeof(struct Sigc_scia);
	       size_t nr_byte_sigc = num_mds * nr_byte;
	       struct Sigc_scia *sigc;

	       sigc = (struct Sigc_scia *) malloc( nr_byte_sigc );
	       ny = nr = 0;
	       do {
		    (void) memcpy( sigc+ny, mds[nr].clus[nc].sigc, nr_byte );
		    ny += mds->clus[nc].n_sigc;
	       } while ( ++nr < num_mds );
	       (void) H5TBmake_table( "Reticon/Epitax detector (co-added)", 
				      subgrp_id, clus_name, NFIELD_SIGC, 
				      num_sigc, sigc_size, sigc_names, 
				      sigc_offs, sigc_type, num_sigc, 
				      NULL, FALSE, sigc );
	       free( sigc );
	  }
     }
/*
 * close interface
 */
     (void) H5Gclose( subgrp_id );
     (void) H5Gclose( grp_id );
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1C_WR_H5_MDS
.PURPOSE     write SCIAMACHY level 1c Measurements Data Sets
.INPUT/OUTPUT
  call as    SCIA_LV1C_WR_H5_MDS( param, num_mds, mds_1c );
     input:  
             struct param_record param : struct holding user-defined settings
	     unsigned short  num_mds   : number of MDS
	     struct mds1c_scia *mds_1c : MDS struct (level 1c)

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1C_WR_H5_MDS( const struct param_record param, 
			  unsigned int num_mds, 
			  const struct mds1c_scia *mds_1c )
{
     const char prognm[] = "SCIA_LV1C_WR_H5_MDS";

     register unsigned int nf, nr;

     char  clus_name[12], grp_name[15];

     hid_t   grp_id = -1; 
     hid_t   subgrp_id = -1;
     hbool_t compress;
     hsize_t dims[3];
     hid_t   mds1c_type[NFIELD_LV1C];

     const size_t mds1c_size = sizeof( struct mds1c_scia );
     const size_t mds1c_offs[NFIELD_LV1C] = {
	  HOFFSET( struct mds1c_scia, mjd ),
	  HOFFSET( struct mds1c_scia, quality_flag ),
	  HOFFSET( struct mds1c_scia, type_mds ),
	  HOFFSET( struct mds1c_scia, category ),
	  HOFFSET( struct mds1c_scia, state_id ),
	  HOFFSET( struct mds1c_scia, dur_scan ),
	  HOFFSET( struct mds1c_scia, orbit_phase )
     };
     const char *mds1c_names[NFIELD_LV1C] = {
          "mjd", "quality_flag", "type_mds", "category", "state_id", 
	  "dur_scan", "orbit_phase"
     };

     static bool init = TRUE;
/*
 * check number of MDS records
 */
     if ( num_mds == 0 ) return;
/*
 * set HDF5 boolean variable for compression
 */
     if ( param.flag_deflate == PARAM_SET )
          compress = TRUE;
     else
          compress = FALSE;
/*
 * open /MDS-group
 */
     if ( init ) {
	  grp_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/MDS" );
	  if ( grp_id < 0 ) 
	       NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "/MDS" );
	  (void) H5Gclose( grp_id );
	  init = FALSE;
     }
/*
 * open/create group /MDS/<source>_<state_id>
 */
     switch ( (int) mds_1c->type_mds ) {
     case SCIA_NADIR:
	  (void) snprintf( grp_name, 15, "/MDS/nadir_%02u", 
			   mds_1c->state_index );
	  break;
     case SCIA_LIMB:
	  (void) snprintf( grp_name, 15, "/MDS/limb_%02u", 
			   mds_1c->state_index );
	  break;
     case SCIA_MONITOR:
	  (void) snprintf( grp_name, 15, "/MDS/moni_%02u", 
			   mds_1c->state_index );
	  break;
     case SCIA_OCCULT:
	  (void) snprintf( grp_name, 15, "/MDS/occult_%02u", 
			   mds_1c->state_index );
	  break;
     default:
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, "unknown MDS type" );
     }
     grp_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, grp_name );
     if ( grp_id < 0 ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, grp_name );
/*
 * write table with MDS-header info
 */
     if ( H5LTfind_dataset( grp_id, "mds1c_hdr" ) == 0 ) {
/*
 * define user-defined data types of the Table-fields
 */
	  mds1c_type[0] = H5Topen( param.hdf_file_id, "mjd", H5P_DEFAULT );
	  mds1c_type[1] = H5Tcopy( H5T_NATIVE_SCHAR );
	  mds1c_type[2] = H5Tcopy( H5T_NATIVE_UCHAR );
	  mds1c_type[3] = H5Tcopy( H5T_NATIVE_UCHAR );
	  mds1c_type[4] = H5Tcopy( H5T_NATIVE_UCHAR );
	  mds1c_type[5] = H5Tcopy( H5T_NATIVE_USHORT );
	  mds1c_type[6] = H5Tcopy( H5T_NATIVE_FLOAT );
/*
 * create table
 */
	  (void) H5TBmake_table( "LV1C MDS Header", grp_id, "mds1c_hdr", 
				 NFIELD_LV1C, 1, mds1c_size, mds1c_names, 
				 mds1c_offs, mds1c_type, 1, NULL, compress, 
				 mds_1c );
/*
 * close interface
 */
	  for ( nf = 0; nf < NFIELD_LV1C; nf++ ) 
	       (void) H5Tclose( mds1c_type[nf] );
     }
/*
 * write datasets specific for a cluster
 */
     for ( nr = 0; nr < num_mds; nr++, mds_1c++ ) {
/*
 * create group with name of the cluster ID
 */
	  (void) snprintf( clus_name, 11, "cluster_%02hhu", mds_1c->clus_id );
	  if ( subgrp_id >= 0 ) (void) H5Gclose( subgrp_id );
	  if ( (subgrp_id = NADC_OPEN_HDF5_Group( grp_id, clus_name )) < 0 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, clus_name );
/*
 * radiance units flag
 */
	  (void) H5LTset_attribute_char( grp_id, clus_name, "rad_units_flag", 
					 (const char *) &mds_1c->rad_units_flag, 1 );
/*
 * channel ID
 */
	  (void) H5LTset_attribute_uchar( grp_id, clus_name, "channelID", 
					  &mds_1c->chan_id, 1 );
/*
 * co-add factor
 */
	  (void) H5LTset_attribute_uchar( grp_id, clus_name, "co-addfactor", 
					  &mds_1c->coaddf, 1 );
/*
 * pet
 */
	  (void) H5LTset_attribute_float( grp_id, clus_name, "pet", 
					  &mds_1c->pet, 1 );
/*
 * DSR length
 */
	  (void) H5LTset_attribute_uint( grp_id, clus_name, "dsr_length", 
					 &mds_1c->dsr_length, 1 );
/*
 * pixel IDs
 */
	  dims[0] = (hsize_t) mds_1c->num_pixels;
	  (void) H5LTmake_dataset( subgrp_id, "pixel_ID", 1, dims, 
				   H5T_NATIVE_USHORT, mds_1c->pixel_ids );
/*
 * pixel wavelength and error estimate
 */
	  (void) H5LTmake_dataset_float( subgrp_id, "pixel_wavelength", 
					 1, dims, mds_1c->pixel_wv );
	  (void) H5LTmake_dataset_float( subgrp_id, "pixel_wavelength_error", 
					 1, dims, mds_1c->pixel_wv_err );
/*
 * signal values and error estimate
 */
	  dims[1] = (hsize_t) mds_1c->num_pixels;
	  dims[0] = (hsize_t) mds_1c->num_obs;
	  (void) H5LTmake_dataset_float( subgrp_id, "pixel_signal", 
					 2, dims, mds_1c->pixel_val );
	  (void) H5LTmake_dataset_float( subgrp_id, "pixel_signal_error", 
					 2, dims, mds_1c->pixel_err );
/*
 * Geolocation
 */
	  switch ( (int) mds_1c->type_mds ) {
	  case SCIA_NADIR:
	       SCIA_WR_H5_GEON( param.hdf_file_id, subgrp_id, compress, 
				mds_1c->num_obs, mds_1c->geoN );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "geoN" );
	       break;
	  case SCIA_OCCULT:
	  case SCIA_LIMB:
	       SCIA_WR_H5_GEOL( param.hdf_file_id, subgrp_id, compress, 
				mds_1c->num_obs, mds_1c->geoL );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "geoL" );
	       break;
	  case SCIA_MONITOR:
	       SCIA_WR_H5_GEOC( param.hdf_file_id, subgrp_id, compress, 
				mds_1c->num_obs, mds_1c->geoC );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "geoC" );
	       break;
	  }
     }
/*
 * close interface
 */
 done:
     if ( subgrp_id >= 0 ) (void) H5Gclose( subgrp_id );
     (void) H5Gclose( grp_id );
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1C_WR_H5_MDS_PMD
.PURPOSE     write SCIAMACHY level 1 Measurements Data Sets
.INPUT/OUTPUT
  call as    SCIA_LV1C_WR_H5_MDS_PMD( param, pmd );
     input:  
             struct param_record param : struct holding user-defined settings
	     unsigned short  num_mds   : number of MDS
             struct mds1c_pmd  *pmd    : PMD MDS struct (level 1c)

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1C_WR_H5_MDS_PMD( const struct param_record param, 
			      const struct mds1c_pmd *pmd )
{
     const char prognm[] = "SCIA_LV1C_WR_H5_MDS_PMD";

     register unsigned int nf;

     char    grp_name[20];

     hid_t   grp_id = -1; 
     hbool_t compress;
     hsize_t dims[3];
     hid_t   mds1c_type[NFIELD_LV1C];

     const size_t mds1c_size = sizeof( struct mds1c_pmd );
     const size_t mds1c_offs[NFIELD_LV1C] = {
	  HOFFSET( struct mds1c_pmd, mjd ),
	  HOFFSET( struct mds1c_pmd, quality_flag ),
	  HOFFSET( struct mds1c_pmd, type_mds ),
	  HOFFSET( struct mds1c_pmd, category ),
	  HOFFSET( struct mds1c_pmd, state_id ),
	  HOFFSET( struct mds1c_pmd, dur_scan ),
	  HOFFSET( struct mds1c_pmd, orbit_phase )
     };
     const char *mds1c_names[NFIELD_LV1C] = {
          "mjd", "quality_flag", "type_mds", "category", "state_id", 
	  "dur_scan", "orbit_phase"
     };

     static bool init = TRUE;
/*
 * check number of MDS records
 */
     if ( (int) pmd->type_mds == SCIA_MONITOR ) return;
/*
 * set HDF5 boolean variable for compression
 */
     if ( param.flag_deflate == PARAM_SET )
          compress = TRUE;
     else
          compress = FALSE;
/*
 * open /MDS-group
 */
     if ( init ) {
	  grp_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/MDS" );
	  if ( grp_id < 0 ) 
	       NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "/MDS" );
	  (void) H5Gclose( grp_id );
	  init = FALSE;
     }
/*
 * open/create group /MDS/<source>_<state_id>/PMD
 */
     switch ( (int) pmd->type_mds ) {
     case SCIA_NADIR:
	  (void) snprintf( grp_name, 19, "/MDS/nadir_%02u/PMD", 
			   pmd->state_index );
	  break;
     case SCIA_LIMB:
	  (void) snprintf( grp_name, 19, "/MDS/limb_%02u/PMD",
			   pmd->state_index );
	  break;
     case SCIA_MONITOR:
	  (void) snprintf( grp_name, 19, "/MDS/moni_%02u/PMD", 
			   pmd->state_index );
	  break;
     case SCIA_OCCULT:
	  (void) snprintf( grp_name, 19, "/MDS/occult_%02u/PMD", 
			   pmd->state_index );
	  break;
     default:
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, "unknown MDS type" );
     }
     grp_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, grp_name );
     if ( grp_id < 0 ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, grp_name );
/*
 * +++++ create/write datasets in the /MDS/<source>_<state_id>/PMD group
 *
 * write table with MDS-header info
 */
     if ( H5LTfind_dataset( grp_id, "mds1c_hdr" ) == 0 ) {
/*
 * define user-defined data types of the Table-fields
 */
	  mds1c_type[0] = H5Topen( param.hdf_file_id, "mjd", H5P_DEFAULT );
	  mds1c_type[1] = H5Tcopy( H5T_NATIVE_SCHAR );
	  mds1c_type[2] = H5Tcopy( H5T_NATIVE_UCHAR );
	  mds1c_type[3] = H5Tcopy( H5T_NATIVE_UCHAR );
	  mds1c_type[4] = H5Tcopy( H5T_NATIVE_UCHAR );
	  mds1c_type[5] = H5Tcopy( H5T_NATIVE_USHORT );
	  mds1c_type[6] = H5Tcopy( H5T_NATIVE_FLOAT );
/*
 * create table
 */
	  (void) H5TBmake_table( "LV1C MDS Header", grp_id, "mds1c_hdr", 
				 NFIELD_LV1C, 1, mds1c_size, mds1c_names, 
				 mds1c_offs, mds1c_type, 1, NULL, compress, 
				 pmd );
/*
 * close interface
 */
	  for ( nf = 0; nf < NFIELD_LV1C; nf++ ) 
	       (void) H5Tclose( mds1c_type[nf] );
     }
/*
 * DSR length
 */
     (void) H5LTset_attribute_uint( param.hdf_file_id, grp_name, "dsr_length", 
				    &pmd->dsr_length, 1 );
/*
 * integrated PMD values
 */
     dims[0] = (hsize_t) pmd->num_pmd / PMD_NUMBER;
     dims[1] = PMD_NUMBER;
     (void) H5LTmake_dataset_float( grp_id, "int_pmd", 2, dims, pmd->int_pmd );
/*
 * Geolocation
 */
     switch ( (int) pmd->type_mds ) {
     case SCIA_NADIR:
	  if ( H5LTfind_dataset( grp_id, "geoN" ) == 0 ) {
	       SCIA_WR_H5_GEON( param.hdf_file_id, grp_id, compress, 
				pmd->num_geo, pmd->geoN );
	  }
	  break;
     case SCIA_OCCULT:
     case SCIA_LIMB:
	  if ( H5LTfind_dataset( grp_id, "geoL" ) == 0 ) {
	       SCIA_WR_H5_GEOL( param.hdf_file_id, grp_id, compress, 
				pmd->num_geo, pmd->geoL );
	  }
	  break;
     }
/*
 * close interface
 */
     (void) H5Gclose( grp_id );
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1C_WR_H5_MDS_POLV
.PURPOSE     write SCIAMACHY level 1 Measurements Data Sets
.INPUT/OUTPUT
  call as    SCIA_LV1C_WR_H5_MDS_POLV( param, polV );
     input:  
             struct param_record param : struct holding user-defined settings
	     unsigned short  num_mds   : number of MDS
	     struct mds1c_polV *polV   : fractional polarisation MDS (level 1c)

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1C_WR_H5_MDS_POLV( const struct param_record param, 
			       const struct mds1c_polV *polV )
{
     const char prognm[] = "SCIA_LV1C_WR_H5_MDS_POLV";

     register unsigned int nf;

     char    grp_name[21];

     hid_t   grp_id = -1; 
     hbool_t compress;
     hsize_t dims[3];
     hid_t   mds1c_type[NFIELD_LV1C];

     const size_t mds1c_size = sizeof( struct mds1c_pmd );
     const size_t mds1c_offs[NFIELD_LV1C] = {
	  HOFFSET( struct mds1c_polV, mjd ),
	  HOFFSET( struct mds1c_polV, quality_flag ),
	  HOFFSET( struct mds1c_polV, type_mds ),
	  HOFFSET( struct mds1c_polV, category ),
	  HOFFSET( struct mds1c_polV, state_id ),
	  HOFFSET( struct mds1c_polV, dur_scan ),
	  HOFFSET( struct mds1c_polV, orbit_phase )
     };
      const char *mds1c_names[NFIELD_LV1C] = {
          "mjd", "quality_flag", "type_mds", "category", "state_id", 
	  "dur_scan", "orbit_phase"
     };

     static bool init = TRUE;
/*
 * check number of MDS records
 */
     if ( (int) polV->type_mds == SCIA_MONITOR ) return;
/*
 * set HDF5 boolean variable for compression
 */
     if ( param.flag_deflate == PARAM_SET )
          compress = TRUE;
     else
          compress = FALSE;
/*
 * open /MDS-output
 */
     if ( init ) {
	  grp_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/MDS" );
	  if ( grp_id < 0 ) 
	       NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "/MDS" );
	  (void) H5Gclose( grp_id );
	  init = FALSE;
     }
/*
 * open/create group /MDS/<source>_<state_id>/polV
 */
     switch ( (int) polV->type_mds ) {
     case SCIA_NADIR:
	  (void) snprintf( grp_name, 20, "/MDS/nadir_%02u/polV", 
			   polV->state_index );
	  break;
     case SCIA_LIMB:
	  (void) snprintf( grp_name, 20, "/MDS/limb_%02u/polV", 
			   polV->state_index );
	  break;
     case SCIA_MONITOR:
	  (void) snprintf( grp_name, 20, "/MDS/moni_%02u/polV", 
			   polV->state_index );
	  break;
     case SCIA_OCCULT:
	  (void) snprintf( grp_name, 20, "/MDS/occult_%02u/polV", 
			   polV->state_index );
	  break;
     default:
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, "unknown MDS type" );
     }
     grp_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, grp_name );
     if ( grp_id < 0 ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, grp_name );
/*
 * +++++ create/write datasets in the /MDS/<source>_<state_id>/polV group
 *
 * write table with MDS-header info
 */
     if ( H5LTfind_dataset( grp_id, "mds1c_hdr" ) == 0 ) {
/*
 * define user-defined data types of the Table-fields
 */
	  mds1c_type[0] = H5Topen( param.hdf_file_id, "mjd", H5P_DEFAULT );
	  mds1c_type[1] = H5Tcopy( H5T_NATIVE_SCHAR );
	  mds1c_type[2] = H5Tcopy( H5T_NATIVE_UCHAR );
	  mds1c_type[3] = H5Tcopy( H5T_NATIVE_UCHAR );
	  mds1c_type[4] = H5Tcopy( H5T_NATIVE_UCHAR );
	  mds1c_type[5] = H5Tcopy( H5T_NATIVE_USHORT );
	  mds1c_type[6] = H5Tcopy( H5T_NATIVE_FLOAT );
/*
 * create table
 */
	  (void) H5TBmake_table( "LV1C MDS Header", grp_id, "mds1c_hdr", 
				 NFIELD_LV1C, 1, mds1c_size, mds1c_names, 
				 mds1c_offs, mds1c_type, 1, NULL, compress, 
				 polV );
/*
 * close interface
 */
	  for ( nf = 0; nf < NFIELD_LV1C; nf++ ) 
	       (void) H5Tclose( mds1c_type[nf] );
     }
/*
 * DSR length
 */
     (void) H5LTset_attribute_uint( param.hdf_file_id, grp_name, "dsr_length", 
				    &polV->dsr_length, 1 );
/*
 * integration times and repetition factors
 */
     dims[0] = (hsize_t) polV->num_diff_intg;
     (void) H5LTmake_dataset( grp_id, "intg_times", 1, dims, 
			      H5T_NATIVE_USHORT, polV->intg_times );
     (void) H5LTmake_dataset( grp_id, "repetition_factors", 1, dims, 
			      H5T_NATIVE_USHORT, polV->num_polar );
/*
 * fractional polarisation
 */
     SCIA_WR_H5_POLV( grp_id, compress, polV->total_polV, polV->polV );
/*
 * Geolocation
 */
     switch ( (int) polV->type_mds ) {
     case SCIA_NADIR:
	  if ( H5LTfind_dataset( grp_id, "geoN" ) == 0 ) {
	       SCIA_WR_H5_GEON( param.hdf_file_id, grp_id, compress, 
				polV->num_geo, polV->geoN );
	  }
	  break;
     case SCIA_OCCULT:
     case SCIA_LIMB:
	  if ( H5LTfind_dataset( grp_id, "geoL" ) == 0 ) {
	       SCIA_WR_H5_GEOL( param.hdf_file_id, grp_id, compress, 
				polV->num_geo, polV->geoL );
	  }
	  break;
     }
/*
 * close interface
 */
     (void) H5Gclose( grp_id );
}
