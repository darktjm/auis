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
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/ams/libs/ms/RCS/crtdir.c,v 2.6 1992/12/15 21:18:15 rr2b Stab74 $";
#endif

#include <ms.h>

MS_CreateNewMessageDirectory(DirName, Overwrite, obsolete)
char *DirName;
int Overwrite;
char *obsolete;
{
    struct MS_Directory *Dir;

    debug(1, ("MS_CreateNewMessageDirectory %s %d\n", DirName, Overwrite));
    if (*DirName != '/') {
	AMS_RETURN_ERRCODE(EINVAL, EIN_PARAMCHECK, EVIA_CREATENEWMESSAGEDIRECTORY);
    }
    if (obsolete && strcmp(DirName, obsolete)) {
	AMS_RETURN_ERRCODE(EINVAL, EIN_PARAMCHECK, EVIA_CREATENEWMESSAGEDIRECTORY);
    }
    return(CreateNewMSDirectory(DirName, &Dir, Overwrite));
}
