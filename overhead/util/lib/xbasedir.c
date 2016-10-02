/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
	Although called xbasedir, this file implements only XLibDir()
	(It is hard to define XBaseDir bnecause not all platforms 
	use a common tree for all installed X facilities )	
*/

#include <util.h>
#include <stdio.h>
#include <andrdir.h>


/* Return a string with the current value for XLIBDIR imbedded in it. */
const char *XLibDir(const char *str)
{
    const char *p = NULL;
    int addLen;
    static int andyLen = 0;
    static int bufSize = -1;
    static char *buffer;

    if(str) {
	addLen=strlen(str);
    } else addLen = 0;
    if (bufSize == -1) {
	if (((p = getenv("ATKXLIBDIR")) == NULL || *p == '\0' ) 
		&& ((p = (char *) GetConfiguration("ATKXLibDir")) == NULL
			|| *p == '\0'))  {
	    p = QUOTED_DEFAULT_XLIBDIR_ENV;
	}
	andyLen = strlen(p);

	bufSize = addLen + andyLen + 1;
	if ((buffer = (char *) malloc(bufSize)) == NULL)  {
	    bufSize = -1;
	    return NULL;
	}
	strcpy(buffer, p);
    }

    if (bufSize < andyLen + addLen + 1)  {
	bufSize = addLen + andyLen + 1;
	if ((buffer = (char *) realloc(buffer, bufSize)) == NULL)  {
	    bufSize = -1;
	    return NULL;
	}
    }

    if (str != NULL)  {
	strcpy(&(buffer[andyLen]), str);
    }
    else  {
	buffer[andyLen] = '\0';
    }

    return buffer;
}
