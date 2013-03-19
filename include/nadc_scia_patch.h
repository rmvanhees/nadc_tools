/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2004 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   NADC_SCIA_PATCH
.AUTHOR      R.M. van Hees 
.KEYWORDS    header file
.LANGUAGE    ANSI-C
.PURPOSE     prototypes of the specific SRON modules
.ENVIRONment none
.VERSION     1.1     07-Mar-2005   renamed module, added SCIA_LV1_WR_MDS, RvH
             1.0     08-Apr-2004   Creation by R.M. van Hees
------------------------------------------------------------*/

#ifndef  NADC_SCIA_PATCH                       /* Avoid redefinitions */
#define  NADC_SCIA_PATCH

#include <nadc_scia.h>

#if defined _STDIO_INCLUDED || defined _STDIO_H || defined __STDIO_H__
extern void SCIA_LV1_PATCH_HDR( FILE *fp_in, FILE *fp_out )
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp_in, fp_out@*/;

extern void SCIA_LV1_PATCH_BASE( unsigned short, unsigned int, 
				 const struct dsd_envi *, 
				 FILE *fp_in, FILE *fp_out )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp_in, fp_out@*/;
extern void SCIA_LV1_PATCH_DARK( unsigned short, int, unsigned int, 
				 const struct dsd_envi *, 
				 FILE *fp_in, FILE *fp_out )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp_in, fp_out@*/;
extern void SCIA_LV1_PATCH_PPG( unsigned short, int, unsigned int, 
				 const struct dsd_envi *, 
				 FILE *fp_in, FILE *fp_out )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp_in, fp_out@*/;
extern void SCIA_H5_RD_KEYDATA( const char *, /*@out@*/ struct keydata_rec * );
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1_PATCH_RSPN( /*@null@*/ const struct keydata_rec *,
				 unsigned int, const struct dsd_envi *, 
				 FILE *fp_in, FILE *fp_out )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp_in, fp_out@*/;
extern void SCIA_LV1_PATCH_RSPL( /*@null@*/ const struct keydata_rec *,
				 unsigned int, const struct dsd_envi *, 
				 FILE *fp_in, FILE *fp_out )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp_in, fp_out@*/;
extern void SCIA_LV1_PATCH_PSPN( /*@null@*/ const struct keydata_rec *,
				 unsigned int, const struct dsd_envi *, 
				 FILE *fp_in, FILE *fp_out )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp_in, fp_out@*/;
extern void SCIA_LV1_PATCH_PSPL( /*@null@*/ const struct keydata_rec *,
				 unsigned int, const struct dsd_envi *, 
				 FILE *fp_in, FILE *fp_out )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp_in, fp_out@*/;
extern void SCIA_LV1_PATCH_SRS( /*@null@*/ const struct keydata_rec *,
				unsigned int, const struct dsd_envi *, 
				FILE *fp_in, FILE *fp_out )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp_in, fp_out@*/;
extern void SCIA_LV1_PATCH_ADS( unsigned short, const struct dsd_envi, 
				FILE *fp_in, FILE *fp_out )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp_in, fp_out@*/;
#endif

#endif /* NADC_SCIA_PATCH */
