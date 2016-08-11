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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/ams/libs/shr/RCS/findroot.c,v 2.8 1992/12/15 21:21:37 rr2b Stab74 $";
#endif

#include <andrewos.h>
#include <ams.h>

FindTreeRoot(DirName, RootName, ReallyWantParent)
char *DirName, *RootName;
short ReallyWantParent;
{
    char *s, *t;

    strcpy(RootName, DirName);
    for (s = RootName; *s; ++s) {
	if (*s == '/' && !strncmp(s+1, MS_TREEROOT, sizeof(MS_TREEROOT) -1)) {
	    break;
	}
    }
    if (!*s) {
	return(-1);
    }
    if (ReallyWantParent) {
	*s = '\0';
    } else {
	t = strchr(++s, '/');
	if (t) *t = '\0';
    }
    return(0);
}
