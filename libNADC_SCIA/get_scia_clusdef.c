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

.IDENTifer   GET_SCIA_CLUSDEF
.AUTHOR      R.M. van Hees
.LANGUAGE    ANSI C
.PURPOSE     determine characteristics of available clusters
.INPUT/OUTPUT
  call as   numClus = GET_SCIA_CLUSDEF( stateIndex, nr_info, info, &clusDef );
     input:  
             unsigned short stateIndex   :
	     unsigned int nr_info        :
	     struct mds0_info *info      :
    output:  
             struct clusdef_rec *clusDef :

.RETURNS     number of clusters in state
.COMMENTS    None
.VERSION     1.0     10-Apr-2008   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System headers +++++*/
	/* NONE */

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
unsigned short GET_SCIA_CLUSDEF( unsigned short stateIndex, 
				 unsigned int nr_info, 
				 const struct mds0_info *info,
                                 struct clusdef_rec *clusDef )
{
     register unsigned char ncl, nch;
     register unsigned int  ni;

     unsigned short numClus = 0;

     for ( nch = 1; nch <= SCIENCE_CHANNELS; nch++ ) {
	  unsigned char clusIDmx = 0;
	  for ( ni = 0; ni < nr_info; ni++ ) {
	       if ( info[ni].stateIndex != stateIndex ) continue;

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
