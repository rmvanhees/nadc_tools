/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2009 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   GET_SCIA_MAGIC_ID
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA 
.LANGUAGE    ANSI C
.PURPOSE     obtain unique product ID for SCIA level 0 products
.INPUT/OUTPUT
  call as   id = GET_SCIA_MAGIC_ID( productName );
     input:  
             char *productName  : name of the Sciamachy product

.RETURNS     unique number (unsigned int)
.COMMENTS    None
.ENVIRONment None
.VERSION     1.0     26-Jan-2009   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_SOURCE 2

/*+++++ System headers +++++*/
#include <stdlib.h>
#include <string.h>

/*+++++ Local Headers +++++*/
#define _SCIA_COMMON
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
unsigned int GET_SCIA_MAGIC_ID( const char prodName[] )
{
     const char *cpntr;

     unsigned int magicNumber;

     if ( (cpntr = strrchr( prodName, '/' )) != NULL )
          ++cpntr;
     else
          cpntr = prodName;

     switch ( cpntr[10] ) {
     case 'U': 
	  magicNumber = 3000000000u;
	  break;
     case 'R':
	  magicNumber = 2000000000u;
	  break;
     case 'P':
     case 'O':
	  magicNumber = 1000000000u;
	  break;
     default:
	  magicNumber = 0u;
	  break;
     }
     magicNumber += 10000U * strtol( cpntr+49, (char **) NULL, 10 );
     magicNumber += (unsigned int) strtoul( cpntr+55, (char **) NULL, 10 );

     return magicNumber;
}
