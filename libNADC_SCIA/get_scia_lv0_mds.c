/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2007 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   GET_SCIA_LV0_MDS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 0
.LANGUAGE    ANSI C
.PURPOSE     read one state with Sciamachy level 0 MDS AUX/DET/PMD records
.RETURNS     number of records read
.COMMENTS    contains GET_SCIA_LV0_STATE_AUX, GET_SCIA_LV0_STATE_DET, 
                      GET_SCIA_LV0_STATE_PMD
.ENVIRONment None
.VERSION     1.0     15-Jan-2007   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_SOURCE 2

/*+++++ System headers +++++*/
#include <stdio.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_0
#include <nadc_scia.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
				/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   GET_SCIA_LV0_STATE_AUX
.PURPOSE     read all Auxiliary MDS records of one state
.INPUT/OUTPUT
  call as   nrec = GET_SCIA_LV0_STATE_AUX( fd, info, numInfo, &aux );
     input:
            FILE *fd               : file pointer
            struct mds0_info *info : structure holding info about MDS records
	    unsigned int numInfo   : number of info records
    output:
            struct mds0_aux **aux  : auxiliary MDS records
            
.RETURNS     number of MDS recods read (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int GET_SCIA_LV0_STATE_AUX( FILE *fd, const struct mds0_info *info,
				     unsigned int numInfo,
				     struct mds0_aux **aux_out )
{
     register unsigned int naux = 0u;

     const unsigned short stateIndex = (info == NULL ? 0 : info->stateIndex);

     if ( numInfo == 0u || info == NULL ) return 0u;

     while( ++naux < numInfo ) {
	  if ( info[naux].stateIndex != stateIndex ) break;
     }
     (void) SCIA_LV0_RD_AUX( fd, info, naux, aux_out );

     return naux;
}

/*+++++++++++++++++++++++++
.IDENTifer   GET_SCIA_LV0_STATE_DET
.PURPOSE     read all Detector MDS records of one state
.INPUT/OUTPUT
  call as   nrec = GET_SCIA_LV0_STATE_DET( fd, info, numInfo, &det );
     input:
            FILE *fd               : file pointer
            struct mds0_info *info : structure holding info about MDS records
	    unsigned int numInfo   : number of info records
    output:
            struct mds0_det **det  : detector MDS records
            
.RETURNS     number of MDS recods read (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int GET_SCIA_LV0_STATE_DET( unsigned char chan_mask,
				     FILE *fd, const struct mds0_info *info,
				     unsigned int numInfo, 
				     struct mds0_det **det_out )
{
     register unsigned int ndet = 0u;

     const unsigned short stateIndex = (info == NULL ? 0 : info->stateIndex);

     if ( numInfo == 0u || info == NULL ) return 0u;

     while( ++ndet < numInfo ) {
	  if ( info[ndet].stateIndex != stateIndex ) break;
     }
     (void) SCIA_LV0_RD_DET( fd, info, ndet, chan_mask, det_out );

     return ndet;
}

/*+++++++++++++++++++++++++
.IDENTifer   GET_SCIA_LV0_STATE_PMD
.PURPOSE     read all PMD MDS records of one state
.INPUT/OUTPUT
  call as   nrec = GET_SCIA_LV0_STATE_PMD( fd, info, numInfo, &pmd );
     input:
            FILE *fd               : file pointer
            struct mds0_info *info : structure holding info about MDS records
	    unsigned int numInfo   : number of info records
    output:
            struct mds0_pmd **pmd  : PMD MDS records
            
.RETURNS     number of MDS recods read (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int GET_SCIA_LV0_STATE_PMD( FILE *fd, const struct mds0_info *info,
				     unsigned int numInfo,
				     struct mds0_pmd **pmd_out )
{
     register unsigned int npmd = 0u;

     const unsigned short stateIndex = (info == NULL ? 0 : info->stateIndex);

     if ( numInfo == 0u || info == NULL ) return 0u;

     while( ++npmd < numInfo ) {
	  if ( info[npmd].stateIndex != stateIndex ) break;
     }
     (void) SCIA_LV0_RD_PMD( fd, info, npmd, pmd_out );

     return npmd;
}
