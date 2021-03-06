/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2008 - 2019 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   meris_fr1
.AUTHOR      R.M. van Hees
.KEYWORDS    MERIS full resolution (level 1)
.LANGUAGE    ANSI C
.PURPOSE     read Envisat MERIS full resolution level 1 products, extract data,
             and write in a flexible binary format (HDF5) or dump, in 
	     human readable form, the contents of each PDS data set to a 
	     separate ASCII file
.INPUT/OUTPUT
  call as
            meris_fr1 [options] <input-file>

.RETURNS     non-negative on success, negative on failure
.COMMENTS    None
.ENVIRONment None
.VERSION      1.0   22-Sep-2008	created by R. M. van Hees 
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <hdf5.h>

/*+++++ Local Headers +++++*/
#define _MERIS_COMMON
#include <nadc_meris.h>

/*+++++ Global Variables +++++*/
/* 
 * Most routines to read SCIAMACHY data can allocate memory internally
 * However IDL requires the use of their own memory allocation routines
 */
bool Use_Extern_Alloc = FALSE;

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
int main(int argc, char *argv[])
     /*@globals  errno, stderr, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, stderr, nadc_stat, nadc_err_stack@*/
{
     unsigned int num_dsd;

     char  *cpntr;
     FILE  *fp = NULL;

     struct mph_envi  mph;
     struct sph_meris sph;
     struct dsd_envi  *dsd = NULL;
/*
 * initialization of command-line parameters
 */
     MERIS_SET_PARAM(argc, argv, MERIS_LEVEL_1);
     if (IS_ERR_STAT_FATAL) 
          NADC_GOTO_ERROR(NADC_ERR_PARAM, "MERIS_SET_PARAM");
/*
 * check if we have to display version and exit
 */
     if (nadc_get_param_uint8("flag_version") == PARAM_SET) {
	  MERIS_SHOW_VERSION(stdout, "meris_fr1");
	  exit(EXIT_SUCCESS);
     }
/*
 * dump command-line parameters
 */
     if (nadc_get_param_uint8("flag_show") == PARAM_SET) {
          MERIS_SHOW_PARAM(MERIS_LEVEL_1);
          exit(EXIT_SUCCESS);
     }
/*
 * open input-file
 */
     cpntr = nadc_get_param_string("infile");
     if ((fp = fopen(cpntr, "r")) == NULL)
	  NADC_GOTO_ERROR(NADC_ERR_FILE, cpntr);
     free(cpntr);
/*
 * create output file
 */
     if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
          MERIS_CRE_H5_FILE(MERIS_LEVEL_1);
          if (IS_ERR_STAT_FATAL)
               NADC_GOTO_ERROR(NADC_ERR_HDF_CRE, "HDF5 base");
          MERIS_WR_H5_VERSION();
     }
/*
 * -------------------------
 * read Main Product Header
 */
     ENVI_RD_MPH(fp, &mph);
     if (IS_ERR_STAT_FATAL) 
	  NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "MPH");
     cpntr = nadc_get_param_string("infile");
     if (mph.tot_size != nadc_file_size(cpntr))
          NADC_GOTO_ERROR(NADC_ERR_FATAL, "file size check failed");
     free(cpntr);
     if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	  ENVI_WR_ASCII_MPH(&mph);
	  if (IS_ERR_STAT_FATAL)
	       NADC_GOTO_ERROR(NADC_ERR_FILE_WR, "MPH");
     }
/*      if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) { */
/* 	  SCIA_WR_H5_MPH(&mph); */
/* 	  if (IS_ERR_STAT_FATAL) */
/* 	       NADC_GOTO_ERROR(NADC_ERR_HDF_WR, "MPH"); */
/*      } */
/*
 * -------------------------
 * read Specific Product Header
 */
     MERIS_RD_SPH(fp, mph, &sph);
     if (IS_ERR_STAT_FATAL) 
	  NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "SPH");
     if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	  MERIS_WR_ASCII_SPH(&sph);
	  if (IS_ERR_STAT_FATAL)
	       NADC_GOTO_ERROR(NADC_ERR_FILE_WR, "SPH");
     }
/*      if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) { */
/* 	  SCIA_OL2_WR_H5_SPH(&sph); */
/* 	  if (IS_ERR_STAT_FATAL) */
/* 	       NADC_GOTO_ERROR(NADC_ERR_HDF_WR, "SPH"); */
/*      } */
/*
 * -------------------------
 * read Data Set Descriptor records
 */
     dsd = (struct dsd_envi *)
	  malloc((mph.num_dsd-1) * sizeof(struct dsd_envi));
     if (dsd == NULL) NADC_GOTO_ERROR(NADC_ERR_ALLOC, "dsd");
     num_dsd = ENVI_RD_DSD(fp, mph, dsd);
     if (IS_ERR_STAT_FATAL) 
	  NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "DSD");
     if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	  ENVI_WR_ASCII_DSD(num_dsd, dsd);
	  if (IS_ERR_STAT_FATAL)
	       NADC_GOTO_ERROR(NADC_ERR_FILE_WR, "DSD");
     }
/*
 * when an error has occurred we jump to here:
 */
 done:
/*
 * close input file
 */
     if (fp != NULL) (void) fclose(fp);
/*
 * close HDF5 output file
 */
     if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	  hid_t fid = nadc_get_param_hid("hdf_file_id");
          
          if (fid >= 0 && H5Fclose(fid) < 0) {
               cpntr = nadc_get_param_string("outfile");
               NADC_ERROR(NADC_ERR_HDF_FILE, cpntr);
               free(cpntr);
          }
     }
/*
 * free allocated memory
 */
     if (dsd != NULL) free(dsd);
/*
 * display error messages?
 */
     if (nadc_get_param_uint8("flag_silent") == PARAM_UNSET)
	  NADC_Err_Trace(stderr);
     if (IS_ERR_STAT_FATAL) 
	  return NADC_ERR_FATAL;
     else
	  return NADC_ERR_NONE;
}
