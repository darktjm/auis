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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/ams/libs/ms/RCS/getsnap.c,v 2.7 1992/12/15 21:19:11 rr2b Stab74 $";
#endif

#include <ms.h>

MS_GetSnapshot(dirname, id, SnapshotBuf) 
char *dirname, *id; /* Passed in */
char *SnapshotBuf; /* Passed out */
{
    struct MS_Directory *Dir;
    int msgnum, errsave;

    debug(1, ("MS_GetSnapshot %s %s\n", dirname, id));
    if (ReadOrFindMSDir(dirname, &Dir, MD_READ) != 0) {
	return(mserrcode);
    }
    if (GetSnapshotByID(Dir, id, &msgnum, SnapshotBuf)) {
	errsave = mserrcode;
	CloseMSDir(Dir, MD_READ);
	return(errsave);
    }
    return(CacheDirectoryForClosing(Dir, MD_READ));
}
