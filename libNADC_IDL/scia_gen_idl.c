/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2001 - 2019 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_GEN_IDL
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIAMACHY, IDL
.LANGUAGE    ANSI C
.PURPOSE     IDL wrapper for reading SCIAMACHY data (general)
.COMMENTS    None
.ENVIRONment None
.VERSION      1.3   25-Sep-2009	added get_scia_quality, RvH
              1.2   12-Oct-2002	consistently return, in case of error, -1, RvH 
              1.1   02-Jul-2002	added more error checking, RvH
              1.0   15-Jan-2002	Created by R. M. van Hees 
------------------------------------------------------------*/
/*
 * Define POSIX to indicate
 * that this is a POSIX program
 */
#define  _POSIX_SOURCE 2

/*+++++ System headers +++++*/
#include <stdio.h>
//#include <stdlib.h>
//#include <unistd.h>

/*+++++ Local Headers +++++*/
#define _SCIA_COMMON
#include <nadc_idl.h>

/*+++++ Global Variables +++++*/
FILE *fd_nadc = NULL;

/* 
 * Most routines to read SCIAMACHY data can allocate memory internally
 * However IDL requires the use of their own memory allocation routines
 */
bool Use_Extern_Alloc = TRUE;

/*+++++ Static Variables +++++*/
static bool File_Is_Open = FALSE;
static const char err_msg[] = "invalid number of function arguments";

/*+++++++++++++++++++++++++ Main Program or Functions +++++++++++++++*/
int IDL_STDCALL OpenFile(int argc, void *argv[])
{
     IDL_STRING *str_descr;

     if (argc != 1) NADC_GOTO_ERROR(NADC_ERR_PARAM, err_msg);
     str_descr = (IDL_STRING *) argv[0];

     if (File_Is_Open) (void) fclose(fd_nadc);

     if ((fd_nadc = fopen(str_descr[0].s, "r")) == NULL) {
	  NADC_GOTO_ERROR(NADC_ERR_FILE, strerror(errno));
     } else
	  File_Is_Open = TRUE;

     return 0;
 done:
     return -1;
}

int IDL_STDCALL CloseFile(void)
{
     int stat = 0;

     if (File_Is_Open) {
	  stat = fclose(fd_nadc);
	  File_Is_Open = FALSE;
     }
     return stat;
}

int IDL_STDCALL Err_Clear(void)
{
     NADC_Err_Clear();
     return 0;
}

int IDL_STDCALL Err_Trace(int argc, void *argv[])
{
     FILE *fd_log;

     IDL_STRING *log_name;

     if (argc != 1) NADC_GOTO_ERROR(NADC_ERR_PARAM, err_msg);
     log_name = (IDL_STRING *) argv[0];
/*
 * create temporary file
 */
     fd_log = fopen(log_name[0].s, "a");
     NADC_Err_Trace(fd_log);
     (void) fclose(fd_log);

     NADC_Err_Clear();
     return 0;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_SET_CALIB(int argc, void *argv[])
{
     IDL_STRING *calib_str;

     if (argc != 1) {
	  NADC_ERROR(NADC_ERR_PARAM, err_msg);
	  return -1;
     }
     calib_str = (IDL_STRING *) argv[0];

     if (calib_str[0].slen == 0)
	  return -1;
     
     scia_set_calib(calib_str[0].s);
     if (IS_ERR_STAT_FATAL)
	  return -1;
     
     return 0;
}

int IDL_STDCALL _ENVI_RD_MPH(int argc, void *argv[])
{
     struct mph_envi *mph;

     if (argc != 1) NADC_GOTO_ERROR(NADC_ERR_PARAM, err_msg);
     if (! File_Is_Open) 
	  NADC_GOTO_ERROR(NADC_ERR_FILE, "No open stream");

     mph = (struct mph_envi *) argv[0];
     ENVI_RD_MPH(fd_nadc, mph);
     if (IS_ERR_STAT_FATAL) return -1;

     return 1;
 done:
     return -1;
}

int IDL_STDCALL _ENVI_RD_DSD(int argc, void *argv[])
{
     int nr_dsd = 0;

     struct mph_envi  mph;
     struct dsd_envi  *dsd;

     if (argc != 2) NADC_GOTO_ERROR(NADC_ERR_PARAM, err_msg);
     if (! File_Is_Open) 
	  NADC_GOTO_ERROR(NADC_ERR_FILE, "No open stream");

     mph = *(struct mph_envi *) argv[0];
     dsd = (struct dsd_envi *) argv[1];

     nr_dsd = (int) ENVI_RD_DSD(fd_nadc, mph, dsd);
     if (IS_ERR_STAT_FATAL) return -1;

     return nr_dsd;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_RD_LADS(int argc, void *argv[])
{
     int nr_lads;

     unsigned int num_dsd;

     struct dsd_envi  *dsd;
     struct lads_scia *lads;

     if (argc != 3) NADC_GOTO_ERROR(NADC_ERR_PARAM, err_msg);
     if (! File_Is_Open) 
	  NADC_GOTO_ERROR(NADC_ERR_FILE, "No open stream");

     num_dsd = *(unsigned int *) argv[0];
     dsd  = (struct dsd_envi *) argv[1];
     lads = (struct lads_scia *) argv[2];

     nr_lads = (int) SCIA_RD_LADS(fd_nadc, num_dsd, dsd, &lads);
     if (IS_ERR_STAT_FATAL) return -1;

     return nr_lads;
 done:
     return -1;
}

int IDL_STDCALL _GET_SCIA_ROE_ORBITPHASE(int argc, void *argv[])
{
     register int nm;

     bool   eclipseMode, saaFlag;
     int    num, orbit;
     float  *orbitPhase;
     double *julianDay;

     if (argc != 4) NADC_GOTO_ERROR(NADC_ERR_PARAM, err_msg);
     
     eclipseMode = *(bool *) argv[0];
     num         = *(int *) argv[1];
     julianDay   = (double *) argv[2];
     orbitPhase  = (float *) argv[3];

     for (nm = 0; nm < num; nm++)
	  GET_SCIA_ROE_INFO(eclipseMode, julianDay[nm],
			     &orbit, &saaFlag, orbitPhase+nm);

     return 0;
done:
     return -1;
}

int IDL_STDCALL _GET_SCIA_ROE_INFO(int argc, void *argv[])
{
     bool   eclipseMode;
     double julianDay;

     bool   *saaFlag;
     int    *absOrbit;
     float  *orbitPhase;

     if (argc != 5) NADC_GOTO_ERROR(NADC_ERR_PARAM, err_msg);
     
     eclipseMode = *(bool *) argv[0];
     julianDay   = *(double *) argv[1];
     absOrbit    = (int *) argv[2];
     saaFlag     = (bool *) argv[3];
     orbitPhase  = (float *) argv[4];

     GET_SCIA_ROE_INFO(eclipseMode, julianDay,
			absOrbit, saaFlag, orbitPhase);
     return 0;
done:
     return -1;
}

unsigned short IDL_STDCALL _GET_SCIA_QUALITY(int argc, void *argv[])
{
     int orbit, *range;

     if (argc != 2) NADC_GOTO_ERROR(NADC_ERR_PARAM, err_msg);
     orbit  = *(int *) argv[0];
     range  = (int *) argv[1];

     return GET_SCIA_QUALITY(orbit, range);
done:
     return 0xFFFFU;
}
