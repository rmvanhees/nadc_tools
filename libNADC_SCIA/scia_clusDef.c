/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_CLUSDEF
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 0 data
.LANGUAGE    ANSI C
.PURPOSE     Perform L0 detector DSR checks
.COMMENTS    contains CLUSDEF_DB_EXISTS
                      CLUSDEF_INVALID, CLUSDEF_DSR_SIZE, CLUSDEF_INTG_MIN, 
                      CLUSDEF_DURATION, CLUSDEF_NUM_DET, CLUSDEF_CLCON

             Uses state/cluster configuration database nadc_clusDef.h5
.ENVIRONment None
.VERSION     1.1     15-Nov-2013   added documentation, minor bug-fixes, RvH
             1.0     02-Nov-2013   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <string.h>

#include <hdf5.h>
#include <hdf5_hl.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_0
#include <nadc_scia.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
static const char name_clusDef_db[] = "nadc_clusDef.h5";

#define NFIELDS_MTBL  5
static struct scia_mtbl_rec {
     unsigned short orbit;
     unsigned char  num_clus;
     unsigned char  indx_Clcon;
     unsigned short duration;
     unsigned short num_info;
} metaTable;

#define NFIELDS_CLUS  9
static struct scia_clusDef_rec {
     unsigned char  clus_id;
     unsigned char  chan_id;
     unsigned short start;
     unsigned short length;
     float pet;
     unsigned short intg;
     unsigned short coaddf;
     unsigned short readouts;
     unsigned char  clus_type;
} clusDef[MAX_CLUSTER];

static char clusDef_file[MAX_STRING_LENGTH] = "";

static unsigned char  stateID_prev = 0;
static unsigned short absOrbit_prev = 0;

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   _SCIA_H5_RD_CLUSDEF
.PURPOSE     obtain cluster configuration
.INPUT/OUTPUT
  call as   stat = _SCIA_H5_RD_CLUSDEF( locID, metaIndex );
     input:
             hid_t locID               : hdf5 pointer to group
	     unsigned short metaIndex  : record to read from HDF5 dataset

.RETURNS     succesful read return 1 else -1
             error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
herr_t _SCIA_H5_RD_CLUSDEF( hid_t locID, unsigned short metaIndex )
       /*@globals  clusDef;@*/
       /*@modifies clusDef@*/
{
     const char tableName[] = "clusDef";

     hid_t  dataID = -1;
     hid_t  typeID = -1, mem_typeID = -1;
     hid_t  spaceID = -1, mem_spaceID = -1;
     herr_t stat;

     const int     rank = 2;

     const hsize_t start[] = {metaIndex, 0};
     const hsize_t count[] = {1, MAX_CLUSTER};

     if ( (dataID = H5Dopen( locID, tableName, H5P_DEFAULT )) < 0 )
	  NADC_GOTO_ERROR( NADC_ERR_HDF_RD, tableName );

    /* Get the data type ID */
     if ( (typeID = H5Dget_type( dataID )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_DTYPE, tableName );

     /* Use the equivalent native datatypes */
     mem_typeID = H5Tget_native_type( typeID, H5T_DIR_ASCEND );

     /* Get the dataspace handle */
     if ( (spaceID = H5Dget_space( dataID )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_SPACE, tableName );

     /* Create a memory dataspace handle */
     if ( (mem_spaceID = H5Screate_simple( rank, count, NULL )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_SPACE, tableName );

     /* Select hyperslap */
     stat = H5Sselect_hyperslab( spaceID, H5S_SELECT_SET, 
				 start, NULL, count, NULL );
     if ( stat < 0 ) NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, tableName );

     stat = H5Dread( dataID, mem_typeID, mem_spaceID, spaceID,
		     H5P_DEFAULT, clusDef );
     if ( stat < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_RD, tableName );

     (void) H5Sclose( mem_spaceID );
     (void) H5Sclose( spaceID );
     (void) H5Tclose( mem_typeID );
     (void) H5Tclose( typeID );
     (void) H5Dclose( dataID );
     return 0;
done:
     if ( mem_spaceID >= 0 ) (void) H5Sclose( mem_spaceID );
     if ( spaceID >= 0 ) (void) H5Sclose( spaceID );
     if ( mem_typeID >= 0 ) (void) H5Tclose( mem_typeID );
     if ( typeID >= 0 ) (void) H5Tclose( typeID );
     if ( dataID >= 0 ) (void) H5Dclose( dataID );
     return -1;
}

/*+++++++++++++++++++++++++
.IDENTifer   _SET_SCIA_CLUSDEF
.PURPOSE     set global variables for given state and orbit number
.INPUT/OUTPUT
  call as   stat = _SET_SCIA_CLUSDEF( stateID, absOrbit );
     input:
             unsigned char stateID       :  State ID
	     unsigned short absOrbit     :  orbit number

.RETURNS     succesful read return 0; empty mtbl-entry return 1;
             unknown Clcon-entry return 2; else negative
.COMMENTS    static function
-------------------------*/
static
int _SET_SCIA_CLUSDEF( unsigned char stateID, unsigned short absOrbit )
       /*@globals  metaTable, clusDef, stateID_prev, absOrbit_prev;@*/
       /*@modifies metaTable, stateID_prev, absOrbit_prev@*/
{
     const size_t mtbl_size = sizeof( struct scia_mtbl_rec );
     const size_t mtbl_offs[NFIELDS_MTBL] = {
	  HOFFSET(struct scia_mtbl_rec, orbit),
	  HOFFSET(struct scia_mtbl_rec, num_clus),
	  HOFFSET(struct scia_mtbl_rec, indx_Clcon),
	  HOFFSET(struct scia_mtbl_rec, duration),
	  HOFFSET(struct scia_mtbl_rec, num_info)
     };
     const size_t mtbl_sizes[NFIELDS_MTBL] = {
	  sizeof( metaTable.orbit ),
	  sizeof( metaTable.num_clus ),
	  sizeof( metaTable.indx_Clcon ),
	  sizeof( metaTable.duration ),
	  sizeof( metaTable.num_info )
     };

     int     res = 0;

     char    grpName[9];

     hid_t   fid = -1;
     hid_t   gid = -1;
     herr_t  stat;

     char msg[SHORT_STRING_LENGTH];
/*
 * set global variables
 */
     stateID_prev  = stateID;
     absOrbit_prev = absOrbit;

     /* open output HDF5-file */
     if ( ! CLUSDEF_DB_EXISTS() ) {
	  (void) snprintf( msg, SHORT_STRING_LENGTH, 
			   "can not open file: %s", clusDef_file );
	  NADC_GOTO_ERROR( NADC_ERR_NONE, msg );
     }
     fid = H5Fopen( clusDef_file, H5F_ACC_RDONLY, H5P_DEFAULT );
     if ( fid < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_FILE, clusDef_file );

     /* open group with data of requested state */
     (void) snprintf( grpName, 9, "State_%02hhu", stateID );
     H5E_BEGIN_TRY {
          gid = H5Gopen( fid, grpName, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( gid < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_GRP, grpName );

     /* read metaTable record */
     stat = H5TBread_records( gid, "metaTable", absOrbit, 1,
			      mtbl_size, mtbl_offs, mtbl_sizes, 
			      &metaTable );
     if ( stat < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "metaTable" );

     /* read Clcon record */
     if ( metaTable.duration == 0 ) {
	  res = 1;
     } else {
	  if ( metaTable.indx_Clcon == 255 ) {
	       res = 2;
	  } else {
	       if ( _SCIA_H5_RD_CLUSDEF( gid, metaTable.indx_Clcon ) < 0 )
		    NADC_GOTO_ERROR( NADC_ERR_HDF_RD, "clusDef" );
	  }
     }

     (void) H5Gclose( gid );
     (void) H5Fclose( fid );
     return res;
done:
     if ( gid > 0 ) (void) H5Gclose( gid );
     if ( fid > 0 ) (void) H5Fclose( fid );
     return -1;
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   CLUSDEF_DB_EXISTS
.PURPOSE     check if state-cluster configuration database exists on the system
.INPUT/OUTPUT
  call as   stat = CUSDEF_INVALID();

.RETURNS     return TRUE if database exists else FALSE
.COMMENTS    none
-------------------------*/
bool CLUSDEF_DB_EXISTS( void )
{
     if ( *clusDef_file != '\0' ) return TRUE;

     (void) snprintf( clusDef_file, MAX_STRING_LENGTH, "./%s", 
		      name_clusDef_db );
     if ( nadc_file_exists( clusDef_file ) ) return TRUE;

     (void) snprintf( clusDef_file, MAX_STRING_LENGTH, "%s/%s", 
		      DATA_DIR, name_clusDef_db );
     if ( nadc_file_exists( clusDef_file ) ) return TRUE;

     return FALSE;
}

/*+++++++++++++++++++++++++
.IDENTifer   CLUSDEF_MTBL_VALID
.PURPOSE     check if entry in nadc_clusDef database is valid
.INPUT/OUTPUT
  call as   stat = CUSDEF_MTBL_VALID( stateID, absOrbit );
     input:
             unsigned char stateID       :  State ID
	     unsigned short absOrbit     :  orbit number

.RETURNS     return TRUE if entry is valid else FALSE
.COMMENTS    none
-------------------------*/
bool CLUSDEF_MTBL_VALID( unsigned char stateID, unsigned short absOrbit )
       /*@globals  metaTable, stateID_prev, absOrbit_prev;@*/
{
     static bool res = TRUE;

     if ( stateID != stateID_prev || absOrbit != absOrbit_prev ) {
	  char msg[SHORT_STRING_LENGTH];

	  const int stat = _SET_SCIA_CLUSDEF( stateID, absOrbit );

	  if ( stat == 2 ) {
	       (void) snprintf( msg, SHORT_STRING_LENGTH, 
				"no valid Clcon for orbit/state: %05hu/%02hhu", 
				absOrbit, stateID );
	       NADC_ERROR( NADC_ERR_NONE, msg );
	       res = TRUE;
	  } else if ( stat == 1 ) {
	       (void) snprintf( msg, SHORT_STRING_LENGTH, 
				"empty entry for orbit/state: %05hu/%02hhu", 
				absOrbit, stateID );
	       NADC_ERROR( NADC_ERR_NONE, msg );
	       res = FALSE;
	  } else if ( stat < 0 ) {
	       (void) snprintf( msg, SHORT_STRING_LENGTH, 
				"failed to read from %s: %05hu/%02hhu", 
				name_clusDef_db, absOrbit, stateID );
	       NADC_ERROR( NADC_ERR_FATAL, msg );
	       res = FALSE;
	  } else {                 /* successful read */
	       res = TRUE;
	  }
     }
     return res;
}

/*+++++++++++++++++++++++++
.IDENTifer   CLUSDEF_DSR_SIZE
.PURPOSE     calculate size of DSR state ID and time in state
.INPUT/OUTPUT
  call as   dsr_size = CLUSDEF_DSR_SIZE( stateID, absOrbit, bcps_in_scan );
     input:
             unsigned char stateID       :  State ID
	     unsigned short absOrbit     :  orbit number
	     unsigned short bcps_in_scan :  Broad Cast Pulse Signal (16 Hz)
                                            since start of state execusion 
                                               or new tangent height

.RETURNS     size of DSR in bytes (unsigned short)
.COMMENTS    none
-------------------------*/
unsigned short CLUSDEF_DSR_SIZE( unsigned char stateID, 
				 unsigned short absOrbit,
				 unsigned short bcps_in_scan )
       /*@globals  metaTable, clusDef, stateID_prev, absOrbit_prev;@*/
{
     register unsigned char nch = 1;

     unsigned short sz = 65;

     if ( stateID != stateID_prev || absOrbit != absOrbit_prev ) {
	  if ( _SET_SCIA_CLUSDEF( stateID, absOrbit ) != 0 ) {
	       metaTable.num_clus = 0;
	       return 0;
	  }
     }
     if ( metaTable.indx_Clcon == 255 ) return 0;

     do {
	  register unsigned short ncl = 0;
	  register unsigned short intg;

	  bool add_hdr = TRUE;

	  do {
	       if ( clusDef[ncl].pet <= 0.03125f && clusDef[ncl].coaddf > 1 ) 
		    intg = clusDef[ncl].coaddf;
	       else
		    intg = clusDef[ncl].intg;

	       if ( clusDef[ncl].readouts != 0 
		    && clusDef[ncl].chan_id == nch
		    && (bcps_in_scan % intg) == 0 ) {
		    if ( add_hdr ) {
			 sz += 16;
			 add_hdr = FALSE;
		    }
		    if ( clusDef[ncl].coaddf == 1 ) {
			 sz += 10 + clusDef[ncl].length * 2;
		    } else {
			 sz += 10 + clusDef[ncl].length * 3;
			 if ( (clusDef[ncl].length % 2) == 1 ) sz += 1;
		    }
	       }
	  } while ( ++ncl < MAX_CLUSTER );
     } while ( ++nch <= SCIENCE_CHANNELS );

     return sz;
}

/*+++++++++++++++++++++++++
.IDENTifer   CLUSDEF_INTG_MIN
.PURPOSE     obtain shortest integration time (1/16 sec)
.INPUT/OUTPUT
  call as   intg_min = CLUSDEF_INTG_MIN( stateID, absOrbit );
     input:
             unsigned char stateID       :  State ID
	     unsigned short absOrbit     :  orbit number

.RETURNS     shortest integration time (unsigned short)
.COMMENTS    none
-------------------------*/
unsigned short CLUSDEF_INTG_MIN( unsigned char stateID, 
				 unsigned short absOrbit )
       /*@globals  metaTable, clusDef, stateID_prev, absOrbit_prev;@*/
{
     register unsigned short ncl = 0;

     unsigned short intg_min = USHRT_MAX;

     if ( stateID != stateID_prev || absOrbit != absOrbit_prev ) {
	  if ( _SET_SCIA_CLUSDEF( stateID, absOrbit ) != 0 ) {
	       metaTable.num_clus = 0;
	       return 0;
	  }
     }
     do {
	  if ( clusDef[ncl].readouts > 0  && clusDef[ncl].intg < intg_min )
	       intg_min = clusDef[ncl].intg;
     } while ( ++ncl < MAX_CLUSTER );

     return intg_min;
}

/*+++++++++++++++++++++++++
.IDENTifer   CLUSDEF_DURATION
.PURPOSE     obtain state duration (BCPS)
.INPUT/OUTPUT
  call as   bcps = CUSDEF_DURATION( stateID, absOrbit );
     input:
             unsigned char stateID       :  State ID
	     unsigned short absOrbit     :  orbit number

.RETURNS     state duration (BCPS) (unsigned short)
.COMMENTS    none
-------------------------*/
unsigned short CLUSDEF_DURATION( unsigned char stateID, 
				 unsigned short absOrbit )
       /*@globals  metaTable, stateID_prev, absOrbit_prev;@*/
{
     if ( stateID != stateID_prev || absOrbit != absOrbit_prev ) {
	  if ( _SET_SCIA_CLUSDEF( stateID, absOrbit ) < 0 ) return 0;
     }
     return metaTable.duration;
}

/*+++++++++++++++++++++++++
.IDENTifer   CLUSDEF_NUM_DET
.PURPOSE     obtain number of DSR in state
.INPUT/OUTPUT
  call as   num_dsr = CLUSDEF_NUM_DET( stateID, absOrbit );
     input:
             unsigned char stateID       :  State ID
	     unsigned short absOrbit     :  orbit number

.RETURNS     number of DSR in state (unsigned short)
.COMMENTS    none
-------------------------*/
unsigned short CLUSDEF_NUM_DET( unsigned char stateID, 
				unsigned short absOrbit )
       /*@globals  metaTable, stateID_prev, absOrbit_prev;@*/
{
     if ( stateID != stateID_prev || absOrbit != absOrbit_prev ) {
	  if ( _SET_SCIA_CLUSDEF( stateID, absOrbit ) < 0 ) return 0;
     }
     return metaTable.num_info;
}

/*+++++++++++++++++++++++++
.IDENTifer   CLUSDEF_CLCON
.PURPOSE     obtain cluster configuration: chan_id, clus_id, start and length
.INPUT/OUTPUT
  call as   num_clus = CLUSDEF_CLCON( stateID, absOrbit, clusdef );
     input:
             unsigned char stateID       :  State ID
	     unsigned short absOrbit     :  orbit number
    output:
	     struct clusdef_rec *clusdef : cluster configuration (brief)

.RETURNS     number of clusters (unsigned short)
.COMMENTS    none
-------------------------*/
unsigned short CLUSDEF_CLCON( unsigned char stateID, 
			      unsigned short absOrbit,
			      struct clusdef_rec *clusdef )
       /*@globals  metaTable, clusDef, stateID_prev, absOrbit_prev;@*/
{
     register unsigned short ncl, offs = 0;
     register unsigned char chan_id = 0;
     register unsigned char clus_id = 0;

     if ( stateID != stateID_prev || absOrbit != absOrbit_prev ) {
	  if ( _SET_SCIA_CLUSDEF( stateID, absOrbit ) != 0 ) {
	       metaTable.num_clus = 0;
	       return 0;
	  }
     }
     (void) memset( clusdef, 0, MAX_CLUSTER * sizeof(struct clusdef_rec) );

     for ( ncl = 0; ncl < metaTable.num_clus; ncl++ ) {
	  if ( chan_id < clusDef[ncl].chan_id ) {
	       chan_id = clusDef[ncl].chan_id;
	       clus_id = 0;
	       if ( chan_id == 2 )
		    offs = chan_id * CHANNEL_SIZE;
	       else
		    offs = (chan_id - 1) * CHANNEL_SIZE;
	  } else {
	       clus_id++;
	  }
	  clusdef[ncl].chanID = clusDef[ncl].chan_id;
	  clusdef[ncl].clusID = clus_id;
	  if ( chan_id == 2 )
	       clusdef[ncl].start  =
		    offs - (clusDef[ncl].start + clusDef[ncl].length);
	  else
	       clusdef[ncl].start  = offs + clusDef[ncl].start;
	  clusdef[ncl].length = clusDef[ncl].length;
     }     
     return metaTable.num_clus;
}
