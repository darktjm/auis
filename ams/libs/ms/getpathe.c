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
#include <ms.h>
#include <mailconf.h>

extern char home[];
extern Boolean DidInit;

MS_GetSearchPathEntry(which, buf, lim)
int     which,
        lim;
char   *buf;
{
    int     i;

    if (!DidInit) {		/* First time-only -- initialization
				   section */
	if ((i = InitializeSearchPaths()) != 0)
	    return(i);
    }
    switch(which) {
	case AMS_MAILPATH:
	    /* Wants *mail* directory */
	    strncpy(buf, home, lim);
	    strncat(buf, MAILSEARCHPATHTEMPLATE, lim);
	    break;
	case AMS_OFFICIALPATH:
	    strncpy(buf, OFFICIALSEARCHPATHTEMPLATE, lim);
	    break;
	case AMS_LOCALPATH:
	    strncpy(buf, LOCALSEARCHPATHTEMPLATE, lim);
	    break;
	case AMS_EXTERNALPATH:
	    strncpy(buf, EXTERNALSEARCHPATHTEMPLATE, lim);
	    break;
	default:
	    if (which < 0 || which >= MS_NumDirsInSearchPath) {
		AMS_RETURN_ERRCODE(EINVAL, EIN_PARAMCHECK, EVIA_GETSEARCHPATHENTRY);
	    }
	    if (strlen(SearchPathElements[which].Path) > lim) {
		AMS_RETURN_ERRCODE(ERANGE, EIN_PARAMCHECK, EVIA_GETSEARCHPATHENTRY);
	    }
	    strncpy(buf, SearchPathElements[which].Path, lim);
	    break;
    }
    return(0);
}
