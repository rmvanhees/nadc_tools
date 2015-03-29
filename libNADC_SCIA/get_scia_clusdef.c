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
  call as   numClus = GET_SCIA_CLUSDEF( stateID, clusDef );
     input:  
             unsigned char stateID       : state ID [1..70]
    output:  
             struct clusdef_rec *clusDef : cluster definition

.RETURNS     number of clusters in state
.COMMENTS    None
.VERSION     2.0     09-Oct-2013   re-write without using info-records, RvH
             1.0     10-Apr-2008   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System headers +++++*/
#include <string.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_0
#include <nadc_scia.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
#define NCLUSDEF_ONE     40
static
const struct clusdef_rec clusDefOne[NCLUSDEF_ONE] = {
     {1, 0, 0, 5}, {1, 1, 5, 192}, {1, 2, 197, 355}, {1, 3, 552, 290},
     {1, 4, 842, 177}, {1, 5, 1019, 5}, {2, 0, 1024, 5}, {2, 1, 1029, 71},
     {2, 2, 1100, 778}, {2, 3, 1878, 94}, {2, 4, 1972, 71}, {2, 5, 2043, 5},
     {3, 0, 2048, 10}, {3, 1, 2058, 23}, {3, 2, 2081, 897}, {3, 3, 2978, 89},
     {3, 4, 3067, 5}, {4, 0, 3072, 5}, {4, 1, 3077, 5}, {4, 2, 3082, 909},
     {4, 3, 3991, 100}, {4, 4, 4091, 5}, {5, 0, 4096, 5}, {5, 1, 4101, 5},
     {5, 2, 4106, 991}, {5, 3, 5097, 18}, {5, 4, 5115, 5}, {6, 0, 5120, 10},
     {6, 1, 5130, 14}, {6, 2, 5144, 973}, {6, 3, 6117, 17}, {6, 4, 6134, 10},
     {7, 0, 6144, 10}, {7, 1, 6154, 38}, {7, 2, 6192, 940}, {7, 3, 7132,26 },
     {7, 4, 7158, 10}, {8, 0, 7168, 10}, {8, 1, 7178, 1004}, {8, 2, 8182, 10}
};

#define NCLUSDEF_THREE   56
static
const struct clusdef_rec clusDefThree[NCLUSDEF_THREE] = {
     {1, 0, 0, 5}, {1, 1, 5, 192}, {1, 2, 197, 355}, {1, 3, 552, 196},
     {1, 4, 748, 94}, {1, 5, 1019, 5}, {2, 0, 1024, 5}, {2, 1, 1100, 114},
     {2, 2, 1214, 664}, {2, 3, 1878, 94}, {2, 4, 2043, 5}, {3, 0, 2048, 10},
     {3, 1, 2081, 50}, {3, 2, 2131, 80}, {3, 3, 2211, 436}, {3, 4, 2647, 75},
     {3, 5, 2722, 87}, {3, 6, 2809, 135}, {3, 7, 2944, 34}, {3, 8, 3067, 5},
     {4, 0, 3072, 5}, {4, 1, 3082, 36}, {4, 2, 3118, 32}, {4, 3, 3150, 535},
     {4, 4, 3685, 134}, {4, 5, 3819, 106}, {4, 6, 3925, 66}, {4, 7, 4091, 5},
     {5, 0, 4096, 5}, {5, 1, 4106, 46}, {5, 2, 4152, 28}, {5, 3, 4180, 525},
     {5, 4, 4705, 158}, {5, 5, 4863, 234}, {5, 6, 5115, 5}, {6, 0, 5120, 10},
     {6, 1, 5144, 83}, {6, 2, 5227, 228}, {6, 3, 5455, 26}, {6, 4, 5481, 178},
     {6, 5, 5659, 28}, {6, 6, 5687, 179}, {6, 7, 5866, 154}, {6, 8, 6020, 31},
     {6, 9, 6051, 14}, {6, 10, 6065, 52}, {6, 11, 6134, 10}, {7, 0, 6144, 10},
     {7, 1, 6192, 245}, {7, 2, 6437, 148}, {7, 3, 6585, 442}, {7, 4, 7027,105},
     {7, 5, 7158, 10}, {8, 0, 7168, 10}, {8, 1, 7178, 1004}, {8, 2, 8182, 10}
};

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
unsigned short GET_SCIA_CLUSDEF( unsigned char stateID, 
                                 /*@out@*/ struct clusdef_rec *clusDef )
       /*@globals clusDefOne, clusDefThree;@*/
       /*@modifies clusDef@*/
{
     unsigned short numClus = 0;

     const int ClusDefIndx[MAX_NUM_STATE+1] = {
          0, 3, 3, 3, 3, 3, 3, 3, 1, 3, 3,
          3, 3, 3, 3, 3, 1, 1, 1, 1, 1,
          1, 1, 3, 3, 3, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1, 1, 3, 1, 1,
          1, 3, 3, 3, 3, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1, 1, 1, 1, 1
     };

     if ( stateID < UCHAR_ONE || stateID >= (unsigned char) MAX_NUM_STATE ) {
          NADC_ERROR( NADC_ERR_FATAL, "state ID out-of-range" );
	  return 0;
     }

     switch ( ClusDefIndx[stateID] ) {
     case ( 1 ):
	  (void) memcpy( clusDef, clusDefOne, 
			 NCLUSDEF_ONE * sizeof( struct clusdef_rec ) );
          numClus = NCLUSDEF_ONE;
	  break;
     case ( 3 ):
	  (void) memcpy( clusDef, clusDefThree, 
			 NCLUSDEF_THREE * sizeof( struct clusdef_rec ) );
          numClus = NCLUSDEF_THREE;
     }
     return numClus;
}
