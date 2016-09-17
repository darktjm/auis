/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */

/*
	Although called xbasedir, this file implements only XLibDir()
	(It is hard to define XBaseDir bnecause not all platforms 
	use a common tree for all installed X facilities )	

	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

/* andydir.c */

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
