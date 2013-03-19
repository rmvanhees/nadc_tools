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

.IDENTifer   NADC_FILES_EQUAL
.AUTHOR      R.M. van Hees
.KEYWORDS    file access checking
.LANGUAGE    ANSI C
.PURPOSE     check if two files pointed to by fl_name_1 and fl_name_2, 
             respectively, are not the same on disk
.INPUT/OUTPUT
  call as   stat = NADC_FILES_EQUAL( fl_name_1, fl_name_2 );

     input:  
             char *fl_name_1 : name of the first file
	     char *fl_name_2 : name of the second file

.RETURNS     on success, zero
.COMMENTS    None
.ENVIRONment None
.VERSION     1.0     25-AUG-2003   Created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_SOURCE 2

/*+++++ System headers +++++*/
#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*+++++ Local Headers +++++*/
#include <nadc_common.h>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
int NADC_FILES_EQUAL( const char *fl_name_1, const char *fl_name_2 )
{
     struct stat buf1, buf2;
/*
 * get file status of the first file
 */
     if ( stat( fl_name_1, &buf1 ) != 0 ) return 0;
/*
 * get file status of the second file
 */
     if ( stat( fl_name_2, &buf2 ) == 0 ) {
	  if ( buf1.st_dev == buf2.st_dev && buf1.st_ino == buf2.st_ino ) {
	       return -1;
	  }
     }
     return 0;
}
