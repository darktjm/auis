/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
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

#include <andrewos.h>

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/util/lib/RCS/localdir.c,v 2.8 1992/12/15 21:10:10 rr2b Stab74 $";
#endif

#include <stdio.h>
#include <andrdir.h>
#include <util.h>

 

/* Return a string with the current value for LOCALDIR imbedded in it. */
const char *LocalDir(const char *str)
{
    const char *p = NULL;
    int addLen;
    static int locyLen = 0;
    static int bufSize = -1;
    static char *buffer;

    if (str != NULL)  {
	addLen = strlen(str);
    }
    else  {
	addLen = 0;
    }

    if (bufSize == -1) {
	if (((p = getenv("LOCALDIR")) == NULL || *p == '\0' ) && ((p = (char *) GetConfiguration("LocalDir")) == NULL || *p == '\0'))  {
/*	    p = "/usr/local"; */
	    p = QUOTED_DEFAULT_LOCALDIR_ENV;
	}
	locyLen = strlen(p);

	bufSize = addLen + locyLen + 1;
	if ((buffer = (char *) malloc(bufSize)) == NULL)  {
	    bufSize = -1;
	    return NULL;
	}

	strcpy(buffer, p);
    }

    if (bufSize < locyLen + addLen + 1)  {
	bufSize = addLen + locyLen + 1;
	if ((buffer = (char *) realloc(buffer, bufSize)) == NULL)  {
	    bufSize = -1;
	    return NULL;
	}
    }

    if (str != NULL)  {
	strcpy(&(buffer[locyLen]), str);
    }
    else  {
	buffer[locyLen] = '\0';
    }

    return buffer;
}
