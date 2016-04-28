/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2003 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   NADC_ERROR
.AUTHOR      R.M. van Hees
.KEYWORDS    header file
.LANGUAGE    ANSI C
.PURPOSE     definitions for error handling and macro's
.COMMENTS    None
.ENVIRONment None
.VERSION      1.2   31-Mar-2003	modified include for C++ code
              1.1   22-Mar-2002	added NADC_STAT_ABSENT, RvH 
              1.0   31-Oct-2001 created by R. M. van Hees
------------------------------------------------------------*/

#ifndef  __DEFS_NADC_ERR                       /* Avoid redefinitions */
#define  __DEFS_NADC_ERR

#ifdef __cplusplus
extern "C" {
#endif

/*+++++ Macros +++++*/
#define NADC_E_NSLOTS       ((unsigned short) 256)

#define NADC_STAT_SUCCESS   ((unsigned char) 0x0U)
#define NADC_STAT_FATAL     ((unsigned char) 0x1U)
#define NADC_STAT_WARN      ((unsigned char) 0x2U)
#define NADC_STAT_INFO      ((unsigned char) 0x4U)
#define NADC_STAT_ABSENT    ((unsigned char) 0x8U)

#define IS_ERR_STAT_INFO    ((nadc_stat & NADC_STAT_INFO) != UCHAR_ZERO)
#define IS_ERR_STAT_ABSENT  ((nadc_stat & NADC_STAT_ABSENT) != UCHAR_ZERO)
#define IS_ERR_STAT_WARN    ((nadc_stat & NADC_STAT_WARN) != UCHAR_ZERO)
#define IS_ERR_STAT_FATAL   ((nadc_stat & NADC_STAT_FATAL) != UCHAR_ZERO)

#define NADC_ERROR( num, str ) \
   NADC_Err_Push( num, __FILE__, __func__, __LINE__, str )

#define NADC_RETURN_ERROR( num, str ) \
   { NADC_ERROR( num, str ); return; }

#define NADC_GOTO_ERROR( num, str ) \
   { NADC_ERROR( num, str ); goto done; }

#define NADC_ERR_SAVE()      NADC_Err_Keep( TRUE )
#define NADC_ERR_RESTORE()   NADC_Err_Keep( FALSE )

typedef enum NADC_err_t {
     NADC_ERR_NONE = 0,
     NADC_ERR_WARN,
     NADC_ERR_FATAL,
     NADC_ERR_VERSION,
     NADC_ERR_HELP,
     NADC_ERR_ALLOC,
     NADC_ERR_MXDIMS,
     NADC_ERR_STRLEN,
     NADC_ERR_PARAM,
     NADC_ERR_CALIB,
     NADC_ERR_FILE,
     NADC_ERR_FILE_CRE,
     NADC_ERR_FILE_RD,
     NADC_ERR_FILE_WR,
     NADC_PDS_DSD_ABSENT,
     NADC_SDMF_ABSENT,
     NADC_ERR_SQL,
     NADC_ERR_SQL_TWICE,
     NADC_WARN_PDS_RD,
     NADC_ERR_PDS_RD,
     NADC_ERR_PDS_WR,
     NADC_ERR_PDS_KEY,
     NADC_ERR_PDS_DSD,
     NADC_ERR_PDS_SIZE,
     NADC_ERR_HDF_FILE,
     NADC_ERR_HDF_CRE,
     NADC_ERR_HDF_RD,
     NADC_ERR_HDF_WR,
     NADC_ERR_HDF_GRP,
     NADC_ERR_HDF_DATA,
     NADC_ERR_HDF_ATTR,
     NADC_ERR_HDF_SPACE,
     NADC_ERR_HDF_DTYPE,
     NADC_ERR_HDF_PLIST,
     NADC_ERR_DORIS
} NADC_err_t;

typedef struct NADC_mesg_t {
     NADC_err_t error_code;
     const char *str;
} NADC_mesg_t;

typedef struct NADC_error_t {
     NADC_err_t  mesg_num;
     int line;
     int count;
     char file_name[SHORT_STRING_LENGTH];
     char func_name[SHORT_STRING_LENGTH];
     char desc[MAX_STRING_LENGTH];
} NADC_error_t;

typedef struct NADC_E_t {
     unsigned short nused;
     NADC_error_t slots[NADC_E_NSLOTS];
} NADC_E_t;

/*+++++ Global Variables +++++*/
extern unsigned char nadc_stat;
extern NADC_E_t nadc_err_stack;

/* Prototypes of modules */
extern void NADC_Err_Push( NADC_err_t, const char *, const char *, int, 
			   const char * )
        /*@globals  nadc_stat, nadc_err_stack;@*/
        /*@modifies nadc_stat, nadc_err_stack@*/;

extern void NADC_Err_Clear( void )
        /*@globals  nadc_stat, nadc_err_stack;@*/
        /*@modifies nadc_stat, nadc_err_stack@*/;

extern void NADC_Err_Keep( bool )
        /*@globals  nadc_stat, nadc_err_stack;@*/
        /*@modifies nadc_stat, nadc_err_stack@*/;

#if defined _STDIO_H || defined _STDIO_H_
extern void NADC_Err_Trace( FILE *stream )
        /*@globals  nadc_stat, nadc_err_stack;@*/
        /*@modifies stream@*/;
#endif   /* ---- defined _STDIO_H || defined _STDIO_H_ ----- */

#ifdef __cplusplus
  }
#endif
#endif   /* __DEFS_NADC_ERR */
