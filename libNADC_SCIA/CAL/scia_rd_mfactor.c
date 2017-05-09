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

.IDENTifer   SCIA_RD_MFACTOR
.AUTHOR      K. Bramstedt
.KEYWORDS    SCIAMACHY level 1b/c product
.LANGUAGE    ANSI C
.PURPOSE     read in m-factors from external database 
.INPUT/OUTPUT
  call as    SCIA_RD_MFACTOR( mftype, sensing_start, calibFlag, mfactor );
     input:
            enum mf_type mftype      : which type is requested
	    char *sensing_start      : taken from MPH
	    unsigned int calibFlag   : bit-flag which defines how to calibrate
    output:
	    float *mfactor           : array holding mfactor

.RETURNS     nothing
.COMMENTS    nothing
.ENVIRONment None
.VERSION     1.2   30-Jul-2008  error message and minor code improvements, RvH
             1.1   07-Aug-2007  small documentation updates
                                include scia_lv1_mfactor_ascii, RvH
             1.0   07-Jun-2007	Initial release, Klaus Bramstedt (ife Bremen)
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE
#define  _BSD_SOURCE        /* needed voor scandir */

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

#include <hdf5.h>
#include <hdf5_hl.h>
 
/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
enum err_flag {FATAL, WARNING, INFO};

#define SCIA_ERROR( stop_flag, mess, arg ) \
   scia_err ( stop_flag, __FILE__, __LINE__, mess, arg );

typedef enum SCIA_err {
     OK,		/* No error */
     ERROR,	        /* unspecified error */
     MPH_ERROR,         /* Error in reading MPH (wrong product?) */
     SPH_ERROR,         /* Error in reading SPH (wrong product?) */
     DSD_ERROR,         /* Error in reading DSDs (Format change??) */
     FILE_NOT_FOUND,	/* File cannot be opened */
     FILE_NOT_CLOSED,	/* File nannot be closed (not opened?) */
} SCIA_err;

typedef struct MPH {
     /*  Product File name */
     char product[ENVI_FILENAME_SIZE];
     /*  Processing Stage FlagN = Near Real Time, T = test product, V= fully
	 validated (fully consolidated) product, S = special product.
	 Letters between N and V (with the exception of T and S) indicate
	 steps in the consolidation process, with letters closer to V */
     char proc_stage[2];
     /*  Reference Document Describing Product AA-BB-CCC-DD-EEEE_V/I&Oslash;&Oslash;
	 (23 characters, including blank space characters) where AA-BB-CCC-DD-EEEE
	 is the ESA standard document no. and V/I is the Version / Issue.
	 If the reference document is the Products Specifications PO-RS-MDA-GS-2009,
	 the version and revision have to refer to the volume 1 of the document,
	 where the status (version/revision) of all volumes can be found.
	 If not used, set to _______________________ */
     char ref_doc[24];
     /*  Spare */
     char spare_1[40];
     /*  Acquisition Station ID (up to 3 codes) If not used, set to ____________________ */
     char acquisition_station[21];
     /*  Processing Center ID which generated current product If not
	 used, set to ______ */
     char proc_center[7];
     /*  UTC Time of Processing (product generation time)UTC Time format.
	 If not used, set to ___________________________. */
     char proc_time[28];
     /*  Software Version number of processing softwareFormat: Name
	 of processor (up to 10 characters)/ version number (4 characters)
	 -- left justified (any blanks added at end). If not used, set to
	 ______________.e.g.
	 MIPAS/2.31____ */
     char software_ver[15];
     /*  Spare */
     char spare_2[40];
     /*  UTC start time of data sensing (first measurement in first data
	 record) UTC Time format. If not used, set to
	 ___________________________. */
     char sensing_start[28];
     /*  UTC stop time of data sensing (last measurements last data record)
	 UTC Time format. If not used, set to
	 ___________________________. */
     char sensing_stop[28];
     /*  Spare */
     char spare_3[40];
     /*  Phasephase letter. If not used, set to X. */
     char phase[2];
     /*  CycleCycle number. If not used, set to +000. */
     char cycle[5];
     /*  Start relative orbit number If not used, set to +00000 */
     char rel_orbit[7];
     /*  Start absolute orbit number.If not used, set to +00000. */
     char abs_orbit[7];
     /*  UTC of ENVISAT state vector. UTC time format. If not used, set
	 to ___________________________. */
     char state_vector_time[28];
     /*  DUT1=UT1-UTC. If not used, set to +.000000. */
     char delta_ut1[9];
     /*  X Position in Earth-Fixed reference. If not used, set to +0000000.000. */
     char x_position[13];
     /*  Y Position in Earth-Fixed reference. If not used, set to +0000000.000. */
     char y_position[13];
     /*  Z Position in Earth-Fixed reference. If not used, set to +0000000.000. */
     char z_position[13];
     /*  X velocity in Earth fixed reference. If not used, set to +0000.000000. */
     char x_velocity[13];
     /*  Y velocity in Earth fixed reference. If not used, set to +0000.000000. */
     char y_velocity[13];
     /*  Z velocity in Earth fixed reference. If not used, set to +0000.000000. */
     char z_velocity[13];
     /*  Source of Orbit Vectors */
     char vector_source[3];
     /*  Spare */
     char spare_4[40];
     /*  UTC time corresponding to SBT below(currently defined to be
	 given at the time of the ascending node state vector). If not used,
	 set to ___________________________. */
     char utc_sbt_time[28];
     /*  Satellite Binary Time (SBT) 32bit integer time of satellite
	 clock. Its value is unsigned (=&gt;0). If not used, set to +0000000000. */
     char sat_binary_time[12];
     /*  Clock Step Sizeclock step in picoseconds. Its value is unsigned
	 (=&gt;0). If not used, set to +0000000000. */
     char clock_step[12];
     /*  Spare */
     char spare_5[32];
     /*  UTC time of the occurrence of the Leap SecondSet to
	 ___________________________
	 if not used. */
     char leap_utc[28];
     /*  Leap second sign(+001 if positive Leap Second, -001 if negative)Set
	 to +000 if not used. */
     char leap_sign[5];
     /*  Leap second errorif leap second occurs within processing segment
	 = 1, otherwise = 0If not used, set to 0. */
     char leap_err[2];
     /*  Spare */
     char spare_6[40];
     /*  1 or 0. If 1, errors have been reported in the product. User should
	 then refer to the SPH or Summary Quality ADS of the product for
	 details of the error condition. If not used, set to 0. */
     char product_err[2];
     /*  Total Size Of Product (# bytes DSR + SPH+ MPH) */
     char tot_size[22];
     /*  Length Of SPH(# bytes in SPH) */
     char sph_size[12];
     /*  Number of DSDs(# DSDs) */
     char num_dsd[12];
     /*  Length of Each DSD(# bytes for each DSD, all DSDs shall have the
	 same length) */
     char dsd_size[12];
     /*  Number of DSs attached(not all DSDs have a DS attached) */
     char num_data_sets[12];
     /*  Spare */
     char spare_7[40];
} MPH;

/* Can be used for all : LK1 PE1 SP1 SU1 MF1 */
typedef struct SPH_SCI_AX {
     /*  SPH descriptor */
     char sph_descriptor[29];
     char spare[51];
} SPH_SCI_AX ;

/*****************************************/
/* DSD : Dataset Descriptors */
typedef struct DSD
{
     char name[29];
     char type;
     char filename[ENVI_FILENAME_SIZE];
     unsigned int offset;
     unsigned int size;
     unsigned int num_dsr;
     int dsr_size;
     char spare [32];
} DSD;

/*
 * compound data types; adapted from NADC lib
 */
typedef struct MJD
{
     int days;
     unsigned int secnd;
     unsigned int musec;
} MJD;

/* DSD MF1 is special */
typedef enum DSD_SCI_MF1_AX
{
     SCI_MF1_AX__M_FACTOR_CAL,
     SCI_MF1_AX__M_FACTOR_DL, 
     SCI_MF1_AX__M_FACTOR_PL, 
     SCI_MF1_AX__M_FACTOR_QL, 
     SCI_MF1_AX__M_FACTOR_DN, 
     SCI_MF1_AX__M_FACTOR_PN, 
     SCI_MF1_AX__M_FACTOR_QN, 
     SCI_MF1_AX__M_FACTOR_NDF,
     SCI_MF1_AX__M_FACTOR_DS, 
     SCI_MF1_AX__M_FACTOR_PS, 
     SCI_MF1_AX__M_FACTOR_QS,  
     MAX_DS_NAME_SCI_MF1_AX
} DSD_SCI_MF1_AX;

typedef enum SCI_AX
{
     SCI_LK1_AX,
     SCI_PE1_AX,
     SCI_SP1_AX,
     SCI_SU1_AX,
     SCI_MF1_AX,
     SCI_AX_MAX,
} SCI_AX;

/* info struct for the m-factor file MF1 */
typedef struct info_MF1_AX
{
     FILE *FILE_l2N;		/* File handler for Lv1C - file */

     MPH mph;			/* Main product header */

     unsigned mph_tot_size;	/* tot_size of product */
    
     SPH_SCI_AX sph;		/* Specific Product header */

     DSD dsd [ MAX_DS_NAME_SCI_MF1_AX ];/* Array for DSDs  */
     /* here the (G)ADS will be stored */
//    m-factor key [MAX_DS_NAME_SCI_MF1_AX];
} info_MF1_AX;

/* Routines for handling m-factor files */
typedef struct M_Factor
{
     DSD_SCI_MF1_AX mftype;
     char creation_date[12];
     char source_of_data[20];
     const double *wl;
     const double *mfactor;
} M_Factor;

/* Array with DSD Names */
static char DS_NAME_SCI_MF1_AX [MAX_DS_NAME_SCI_MF1_AX][29] = {
     "_M_FACTOR_CAL",
     "_M_FACTOR_DL",
     "_M_FACTOR_PL",
     "_M_FACTOR_QL",
     "_M_FACTOR_DN",
     "_M_FACTOR_PN",
     "_M_FACTOR_QN",
     "_M_FACTOR_NDF",
     "_M_FACTOR_DS",
     "_M_FACTOR_PS",
     "_M_FACTOR_QS"
};

static char SCI_AX_ID[SCI_AX_MAX][4] =
{
     "LK1",
     "PE1",
     "SP1",
     "SU1",
     "MF1"
};

static char SPH_SCI_AX_descriptor[SCI_AX_MAX][28] =
{
     "LEAKAGE_CURRENT_PARAMETER",
     "PPG_ETALON_CORRECTION_PARAM",
     "SPECTRAL_CALIBRATION_PARAM",
     "SUN_REFERENCE_PARAMETER",
     "M_FACTOR_FILE"
};

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
/* Error message function, used by the macro SCIA_ERROR */
static void scia_err( enum err_flag err_flag, const char *source_file, 
		      int line, const char* message, const char* arg )
{
     const char *error_type[] = { "ERROR: ", "WARNING: ", " " };

     (void) fprintf( stderr, "%s:%d: %s%s %s\n", source_file, line, 
		     error_type[err_flag], message, arg );
    
     NADC_RETURN_ERROR( NADC_ERR_FATAL, "Error in external code" );
}

/* Reading the main product header MPH */
/* This is ASCII Format, therefore not bin... but fscanf is used */
static SCIA_err Read_MPH (FILE* unit, MPH *mph)
{
     int err;
     char nl[2];

     /* Zeros in the complete structure, so all strings have trailing \0 */
     (void) memset( mph, 0, sizeof(MPH) );
    
     err = fscanf( unit,
		   "PRODUCT=\"%62c\"%1c"
		   "PROC_STAGE=%1c%1c"
		   "REF_DOC=\"%23c\"%1c"
		   "%40c%1c"
		   "ACQUISITION_STATION=\"%20c\"%1c"
		   "PROC_CENTER=\"%6c\"%1c"
		   "PROC_TIME=\"%27c\"%1c"
		   "SOFTWARE_VER=\"%14c\"%1c"
		   "%40c%1c"
		   "SENSING_START=\"%27c\"%1c"
		   "SENSING_STOP=\"%27c\"%1c"
		   "%40c%1c"
		   "PHASE=%1c%1c"
		   "CYCLE=%4c%1c"
		   "REL_ORBIT=%6c%1c"
		   "ABS_ORBIT=%6c%1c"
		   "STATE_VECTOR_TIME=\"%27c\"%1c"
		   "DELTA_UT1=%8c<s>%1c"
		   "X_POSITION=%12c<m>%1c"
		   "Y_POSITION=%12c<m>%1c"
		   "Z_POSITION=%12c<m>%1c"
		   "X_VELOCITY=%12c<m/s>%1c"
		   "Y_VELOCITY=%12c<m/s>%1c"
		   "Z_VELOCITY=%12c<m/s>%1c"
		   "VECTOR_SOURCE=\"%2c\"%1c"
		   "%40c%1c"
		   "UTC_SBT_TIME=\"%27c\"%1c"
		   "SAT_BINARY_TIME=%11c%1c"
		   "CLOCK_STEP=%11c<ps>%1c"
		   "%32c%1c"
		   "LEAP_UTC=\"%27c\"%1c"
		   "LEAP_SIGN=%4c%1c"
		   "LEAP_ERR=%1c%1c"
		   "%40c%1c"
		   "PRODUCT_ERR=%1c%1c"
		   "TOT_SIZE=%21c<bytes>%1c"
		   "SPH_SIZE=%11c<bytes>%1c"
		   "NUM_DSD=%11c%1c"
		   "DSD_SIZE=%11c<bytes>%1c"
		   "NUM_DATA_SETS=%11c%1c"
		   "%40c%1c",
		   mph->product, nl,
		   mph->proc_stage, nl,
		   mph->ref_doc, nl,
		   mph->spare_1, nl,
		   mph->acquisition_station, nl,
		   mph->proc_center, nl,
		   mph->proc_time, nl,
		   mph->software_ver, nl,
		   mph->spare_2, nl,
		   mph->sensing_start, nl,
		   mph->sensing_stop, nl,
		   mph->spare_3, nl,
		   mph->phase, nl,
		   mph->cycle, nl,
		   mph->rel_orbit, nl,
		   mph->abs_orbit, nl,
		   mph->state_vector_time, nl,
		   mph->delta_ut1, nl,
		   mph->x_position, nl,
		   mph->y_position, nl,
		   mph->z_position, nl,
		   mph->x_velocity, nl,
		   mph->y_velocity, nl,
		   mph->z_velocity, nl,
		   mph->vector_source, nl,
		   mph->spare_4, nl,
		   mph->utc_sbt_time, nl,
		   mph->sat_binary_time, nl,
		   mph->clock_step, nl,
		   mph->spare_5, nl,
		   mph->leap_utc, nl,
		   mph->leap_sign, nl,
		   mph->leap_err, nl,
		   mph->spare_6, nl,
		   mph->product_err, nl,
		   mph->tot_size, nl,
		   mph->sph_size, nl,
		   mph->num_dsd, nl,
		   mph->dsd_size, nl,
		   mph->num_data_sets, nl,
		   mph->spare_7, nl );
     if (err != 82)
	  return MPH_ERROR;
     return OK;
}

/* read SPH */
static SCIA_err Read_SPH_SCI_AX( FILE* unit, SPH_SCI_AX *sph,  SCI_AX type )
{
     int err;
     char nl[2];

     /* Zeros in the complete structure, so all strings have trailing \0 */
     (void) memset( sph, 0, sizeof(SPH_SCI_AX) );

     err = fscanf( unit,
		   "SPH_DESCRIPTOR=\"%28c\"%1c"
		   "%51c%1c",
		   sph->sph_descriptor, nl,
		   sph->spare, nl );
     if ( strncmp (SPH_SCI_AX_descriptor[type], sph->sph_descriptor,
		   strlen(SPH_SCI_AX_descriptor[type])) != 0)
	  return SPH_ERROR;	
     if (err != 4)
	  return SPH_ERROR;
     return OK;
}

/* Read the Dataset Desc */
static SCIA_err Read_DSD( FILE* unit, DSD *dsd )
{
     int err;
     char nl[2];

     err = fscanf( unit,        
		   "DS_NAME=\"%28c\"%1c"
		   "DS_TYPE=%1c%1c"
		   "FILENAME=\"%62c\"%1c"
		   "DS_OFFSET=+%20u<bytes>%1c"
		   "DS_SIZE=+%20u<bytes>%1c"
		   "NUM_DSR=+%10u%1c"
		   "DSR_SIZE=%11d<bytes>%1c"
		   "%32c%1c",
		   dsd->name, nl,
		   &dsd->type, nl,
		   dsd->filename, nl,
		   &dsd->offset, nl,
		   &dsd->size, nl,
		   &dsd->num_dsr, nl,
		   &dsd->dsr_size, nl,
		   dsd->spare, nl);
/*
  dsd->name[28]='\0';
  if ( (str_ptr = strpbrk(dsd->name, " ") ) != NULL)
  *str_ptr='\0';

  dsd->filename[62]='\0';
  if ( (str_ptr = strpbrk(dsd->filename, " ") ) != NULL)
  *str_ptr='\0';
  */
     if ( err != 16 )
	  return DSD_ERROR;
     return OK;
}

/*********************************************************************
 * M factor part starts here!
 *********************************************************************/

/* the more or less generic open routine */
static SCIA_err open_SCIA_mfactor( char* FILE_name, info_MF1_AX *info )  
{
     int i,err;
     int num_dsd;
     FILE *L1B;			/* for shorter code :-) */
				/* open file */
     info->FILE_l2N = fopen(FILE_name, "rb");
    
     if (info->FILE_l2N == NULL) {
	  (void) fprintf( stderr, 
			  "Input file %s could not be opened! Abort!\n", 
			  FILE_name );
	  return FILE_NOT_FOUND;
     }
     L1B = info->FILE_l2N;
     /*  MPH, SPH, DSDs */
     if (Read_MPH (L1B, &info->mph) != OK)
	  return MPH_ERROR;
     err = sscanf (info->mph.num_dsd , "%d", &num_dsd);
     if (err != 1)
	  return MPH_ERROR;

     if (Read_SPH_SCI_AX (L1B, &info->sph, SCI_MF1_AX) != OK)
	  return SPH_ERROR;
     for ( err=0,i=0; i < num_dsd-1; i++)
	  err += Read_DSD ( L1B, &info->dsd[i] );
     if (err != 0)
	  return DSD_ERROR;
     return OK;
}

static SCIA_err close_SCIA_mfactor( info_MF1_AX *info )
{
     fclose (info->FILE_l2N);
     return OK;
}

/* Basic routine for reading an m-factor dataset */
static SCIA_err read_M_Factor( float *wl, float *mfactor, DSD_SCI_MF1_AX mft,
			       /*@out@*/ info_MF1_AX *info ) 
{
     int n, err;
     FILE *unit = info->FILE_l2N;
     char format_str[20];
     char id_string[200];
     char tmp_string[200];

     /* Jump to start of this Mfactor */
     (void) fseek( unit, info->dsd[mft].offset, SEEK_SET );

     switch( mft ) {
	  /* science detectors */
     case SCI_MF1_AX__M_FACTOR_CAL:
     case SCI_MF1_AX__M_FACTOR_DL: 
     case SCI_MF1_AX__M_FACTOR_DN: 
     case SCI_MF1_AX__M_FACTOR_NDF:
     case SCI_MF1_AX__M_FACTOR_DS: 
	  /* search wavelength */
	  (void) sprintf( id_string, "%s_DIM_1_LIST=", 
			   DS_NAME_SCI_MF1_AX[mft] );
	  (void) sprintf( format_str, "%%%zds", strlen( id_string ));
	  do {
	       err = fscanf( unit, format_str, tmp_string );
	       if (err != 1)
		    SCIA_ERROR( FATAL, "Reading error m-factor: ", 
				DS_NAME_SCI_MF1_AX[mft] );
	  } while( strcmp( id_string,tmp_string ) != 0 );
	  /* start of data found */
	  for ( n = 0; n < SCIENCE_PIXELS; n++) {
	       err = fscanf( unit, "%f%*c", wl+n );
	       if ( err != 1 )
		    SCIA_ERROR( FATAL, "Reading error m-factor: ", 
				DS_NAME_SCI_MF1_AX[mft] );
	  }
	  /* search data part  */
	  (void) sprintf( id_string, "%s=", DS_NAME_SCI_MF1_AX[mft] );
	  (void) sprintf( format_str, "%%%zds", strlen( id_string ));
	  do {
	       err = fscanf( unit, format_str, tmp_string );
	       if (err != 1)
		    SCIA_ERROR( FATAL, "Reading error m-factor: ", 
				DS_NAME_SCI_MF1_AX[mft] );
	  } while( strcmp( id_string,tmp_string ) != 0 );
	  /* start of data found */
	  for ( n = 0; n < SCIENCE_PIXELS; n++ ) {
	       err = fscanf( unit, "%f%*c", mfactor+n );
	       if ( err != 1 )
		    SCIA_ERROR( FATAL, "Reading error m-factor: ", 
				DS_NAME_SCI_MF1_AX[mft] );
	  }
	  break;
	  /* PMDs 1-6 */
     case SCI_MF1_AX__M_FACTOR_PL: 
     case SCI_MF1_AX__M_FACTOR_PN: 
     case SCI_MF1_AX__M_FACTOR_PS: 
	  /* search data part  */
	  (void) sprintf( id_string, "%s=", DS_NAME_SCI_MF1_AX[mft] );
	  (void) sprintf( format_str, "%%%zds", strlen( id_string ));
	  do {
	       err = fscanf( unit, format_str, tmp_string );
	       if ( err != 1 )
		    SCIA_ERROR( FATAL, "Reading error m-factor: ", 
				DS_NAME_SCI_MF1_AX[mft] );
	    } while( strcmp( id_string,tmp_string ) != 0 );
	  /* start of data found */
	  for ( n = 0; n < 6; n++ ) {
	       err = fscanf (unit, "%f%*c", mfactor+n);
	       if ( err != 1 )
		    SCIA_ERROR( FATAL, "Reading error m-factor: ", 
				DS_NAME_SCI_MF1_AX[mft] );
	  }
	  break;
	  /* PMD 7 Q */
     case SCI_MF1_AX__M_FACTOR_QL: 
     case SCI_MF1_AX__M_FACTOR_QN: 
     case SCI_MF1_AX__M_FACTOR_QS:  
	  /* search data part  */
	  (void) sprintf( id_string, "%s=", DS_NAME_SCI_MF1_AX[mft] );
	  (void) sprintf( format_str, "%%%zds", strlen( id_string ));
	  do {
	       err = fscanf( unit, format_str, tmp_string );
	       if ( err != 1 )
		    SCIA_ERROR( FATAL, "Reading error m-factor: ", 
				DS_NAME_SCI_MF1_AX[mft] );
	  } while( strcmp( id_string,tmp_string ) != 0 );
	  /* start of data found */
	  err = fscanf (unit, "%f%*c", mfactor);
	  if ( err != 1 )
	       SCIA_ERROR( FATAL, "Reading error m-factor: ", 
			   DS_NAME_SCI_MF1_AX[mft] );	   
	  break;
     default:
	  SCIA_ERROR( FATAL, "Error Writing M-factor: ", 
		      "Illegal type selected." );
     }
     /* That's it! */
     return OK;
}

/*********************************************************************
 * Auxiliary file directory handler 
 *********************************************************************/
/* routine for selecting an auxillary file */

static SCI_AX cur_aux_type;

/* selector for scandir */
static int aux_dir_selector( const struct dirent *entry )
{
     if (strncmp (entry->d_name+4, SCI_AX_ID[cur_aux_type], 3) == 0 &&
	 strncmp (entry->d_name, "SCI_", 4) == 0 )
/* && */
/* 	entry->d_namlen  d_namlen == 61) */
	  return 1;

     return 0;
}

/* sorting for scandir */
#if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ > 3) || __clang__
static int aux_dir_date_sort( const struct dirent **A, 
			      const struct dirent **B )
#else
static int aux_dir_date_sort( const void *A, const void *B )
#endif
{
     int cmp;
#if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ > 3) || __clang__
     const struct dirent *dirA = *A;
     const struct dirent *dirB = *B;
#else
     const struct dirent *dirA = *(const struct dirent **)A;
     const struct dirent *dirB = *(const struct dirent **)B;
#endif
     /* first sort according to validity time */
     cmp = strncmp (dirA->d_name+30, dirB->d_name+30, 15);

     /* if equal, use sort to creation time, but vice versa */
     if (cmp == 0)
	  cmp = strncmp (dirA->d_name+14, dirB->d_name+14, 15);

     return cmp;
}

/* overall routine */
static 
void file_aux( /*@out@*/ char *file_name, const char* dir_name, 
	       SCI_AX aux_type, const char* start_date )
{
     register int n;
     int n_entries;
     struct dirent **entries;

     cur_aux_type = aux_type;
     /* read in dir content */
     n_entries = scandir( dir_name, &entries, aux_dir_selector,
			  aux_dir_date_sort );
     /* no aux_files found */
     if ( n_entries <= 0 )
	  SCIA_ERROR( FATAL, "No auxiliary files found in directory",
		      dir_name );

     /* search for right position */
     for ( n = 0; n < n_entries; n++ )
	  if ( strncmp (start_date, entries[n]->d_name+30, 15) < 0 ) break; 

     /* measurement time before first AUX file ...*/
     if ( n == 0 )
	  SCIA_ERROR( FATAL, "Auxiliary files begin after this date.", 
		      start_date );

     /* not within validity time of found AUX file*/
     if ( strncmp( start_date, entries[--n]->d_name+46, 15 ) > 0 ) {
	  char msg[2 * MAX_STRING_LENGTH];

	  (void) snprintf( msg, sizeof(msg),
			   "Auxiliary file %s not within validity time %s.", 
			   entries[n]->d_name, start_date );
	  SCIA_ERROR( FATAL, msg, "" );
     }
     /*  write file_name including path */
     (void) snprintf (file_name, MAX_STRING_LENGTH, "%s/%s",
		      dir_name, entries[n]->d_name);
     /* free memories */
     for ( n = 0; n < n_entries; n++ ) free( entries[n] );
     free( entries );
}

/*+++++++++++++++++++++++++
.IDENTifer   Scia_rd_aux_mfactor
.PURPOSE     read m-factor for science channels from database 
             with auxiliary files.
.INPUT/OUTPUT
  call as    Scia_rd_H5_mfactor( mftype, sensing_start, mfactor );
     input:
            enum mf_type mftype   : which type is requested
	    char* sensing_start   : sensing start time yyyymmdd_hhmmss
    output:
	    float *mfactor        : array to write mfactor
.RETURNS     nothing
.COMMENTS    none
-------------------------*/
static
void Scia_rd_aux_mfactor( enum mf_type mftype, char *sensing_start, 
			  /*@out@*/ float *mfactor )
{
     char default_dir[] = "m-factor_07.01";

     DSD_SCI_MF1_AX dsd;
     info_MF1_AX info;

     char mf_file_name[MAX_STRING_LENGTH];
     char mf_dir_name[MAX_STRING_LENGTH];

     char *dir_name = default_dir;
     char *env_dir = getenv( "SCIA_MFACTOR_DIR" );

     float wl[SCIENCE_PIXELS];

     (void) memset( mfactor, 0, SCIENCE_PIXELS * sizeof(float) );

     switch (mftype) {
     case M_CAL:
	  dsd = SCI_MF1_AX__M_FACTOR_CAL;
	  break;
     case M_DL:
	  dsd = SCI_MF1_AX__M_FACTOR_DL;
	  break;
     case M_DN:
	  dsd = SCI_MF1_AX__M_FACTOR_DN;
	  break;
     default:
	  NADC_RETURN_ERROR( NADC_ERR_FATAL, "unknown mftype" );
	  break;
     }

     /* check if Environment variable is set */
     if ( env_dir ) {
	  dir_name = env_dir;
     } else {    /* search for standard dirs */
	  DIR *dir;
	  dir = opendir( default_dir );
	  if ( dir ) {
	       closedir( dir );
	  } else {
	       (void) snprintf( mf_dir_name, MAX_STRING_LENGTH, 
				"%s/%s", DATA_DIR, default_dir );
	       dir = opendir( mf_dir_name );
	       if ( dir ) {
		    dir_name = mf_dir_name;
		    closedir( dir );
	       } else {
		    char msg[MAX_STRING_LENGTH];

		    (void) snprintf( msg, MAX_STRING_LENGTH,
				     "can not open m-factor directory: %s", 
				     mf_dir_name );
		    NADC_RETURN_ERROR( NADC_ERR_FATAL, msg );
	       }
	  }
     }
     /* Here we have a standard directory or an environment value */
     file_aux( mf_file_name, dir_name, SCI_MF1_AX, sensing_start );
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FATAL, 
			     "failed to select appropriate m-factor file" );

     if ( open_SCIA_mfactor( mf_file_name, &info ) != OK )
	  NADC_RETURN_ERROR( NADC_ERR_FILE, mf_file_name );
     read_M_Factor( wl, mfactor, dsd, &info ) ;
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FILE_RD, mf_file_name );
     close_SCIA_mfactor( &info );
}

/*+++++++++++++++++++++++++
.IDENTifer   Scia_rd_H5_mfactor
.PURPOSE     read m-factor for science channels from H5 database.
.INPUT/OUTPUT
  call as    Scia_rd_H5_mfactor( mftype, sensing_start, mfactor );
     input:
            enum mf_type mftype    : which type is requested
	    char* sensing_start    : sensing start time yyyymmdd_hhmmss
    output:
	    float *mfactor         : array to write mfactor
.RETURNS     nothing
.COMMENTS    none
-------------------------*/
static const char *mf_type_str[3] = {
     "SCIA_M_CAL", "SCIA_M_DL", "SCIA_M_DN" 
};

static 
void Scia_rd_H5_mfactor( enum mf_type mftype, char *sensing_start,
			 /*@out@*/ float *mfactor )
{
     char  mf_file[MAX_STRING_LENGTH];
     char  mf_software_version[MAX_STRING_LENGTH];

     char  *val_time;
     float *mf_array;

     hid_t  file_id = -1;
     hsize_t dims[2];

     int n_days;
     int day;
/*
 * open output HDF5-file
 */
     (void) strcpy( mf_file, "./m-factor.h5" );
     H5E_BEGIN_TRY {
	  file_id = H5Fopen( mf_file, H5F_ACC_RDONLY, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( file_id < 0 ) {
	  (void) snprintf( mf_file, MAX_STRING_LENGTH, 
			   "%s/m-factor.h5", DATA_DIR );
	  file_id = H5Fopen( mf_file, H5F_ACC_RDONLY, H5P_DEFAULT );
	  if ( file_id < 0 )
	       NADC_GOTO_ERROR( NADC_ERR_HDF_FILE, mf_file );
     }
/*
 * read software version
 */
     H5LTread_dataset_string(file_id, "SOFTWARE_VERSION", mf_software_version);
 /*
  *  read array with validity strings
  */
     (void) H5LTget_dataset_info( file_id,  "VALIDITY_TIME", 
				  dims, NULL, NULL );
     val_time = (char *) malloc( (size_t) dims[0] * sizeof(char));
     if ( val_time == NULL ) 
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "val_time" );
     (void) H5LTread_dataset_char( file_id, "VALIDITY_TIME", val_time );
     n_days = (int) dims[0] / 32;
/*
 *  read array with mfactors
 */
     (void) H5LTget_dataset_info( file_id,  mf_type_str[mftype], 
				  dims, NULL, NULL );
     mf_array = (float *) malloc( (size_t)(dims[0] * dims[1]) * sizeof(float));
     if ( mf_array == NULL ) 
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "mf_array" );
     (void) H5LTread_dataset_float( file_id, mf_type_str[mftype], mf_array );
/* 
 * (simple) search for correct data
 * yyyymmdd_hhmmss_yyyymmdd_hhmmss
 * start valid.    end validi.
 */
     for ( day=n_days-1; day>=0; day-- ) {
	  if ( strncmp (val_time+32*day, sensing_start, 15) <= 0 ) {
	       if (strncmp (val_time+32*day+16, sensing_start, 15) > 0 )
		    break;
	  }
     }
     if ( day == -1 )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, "No valid m-factor found" );
/* 
 * copy m-factor  
 */
     (void) memcpy( mfactor, 
		    mf_array+ SCIENCE_PIXELS * day, 
		    SCIENCE_PIXELS * sizeof (float) );
     free (mf_array);
     free (val_time);
 done:
     if ( file_id >= 0 ) (void) H5Fclose( file_id );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_RD_MFACTOR( enum mf_type mftype, const char *sensing_start,
		      unsigned int calibFlag, float *mfactor )
{
     char sensing_start_ymd[SHORT_STRING_LENGTH];

     int mjd2000;
     unsigned int second, mu_sec ;
/* 
 * convert sensing start 
 */
     ASCII_2_MJD( sensing_start, &mjd2000, &second, &mu_sec );
     MJD_2_YMD( mjd2000, second, sensing_start_ymd );

                                        /* read m-factor from auxiliary file */
     if ( (calibFlag & DO_MFAC_H5_RAD) == UINT_ZERO ) {
	  Scia_rd_aux_mfactor( mftype, sensing_start_ymd, mfactor );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( NADC_ERR_FATAL, "Scia_rd_aux_mfactor" );
     } else {                                /* read m-factor from HDF5 file */
	  char msg[SHORT_STRING_LENGTH];

	  Scia_rd_H5_mfactor( mftype, sensing_start_ymd, mfactor );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR(NADC_ERR_FATAL, "Scia_rd_H5_mfactor");
	  (void) snprintf( msg, SHORT_STRING_LENGTH,
			   "read m-factors from HDF5 file - %s",
			  sensing_start_ymd );
	  NADC_ERROR( NADC_ERR_NONE, msg );
     }
}
