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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/ams/libs/ms/RCS/getbody.c,v 2.9 1992/12/15 21:19:11 rr2b Stab74 $";
#endif

#include <andrewos.h>
#include <ms.h>

MS_GetPartialBody(DirName, id, Buf, BufLim, offset, remaining, ct)
char *DirName, *id, *Buf;
int BufLim, offset, *remaining, *ct;
{
    struct MS_Directory *Dir;
    char FileName[MAXPATHLEN+1];

    debug(1, ("MS_GetPartialBody %s %s\n", DirName, id));
    *ct = *remaining = 0;
    if (ReadOrFindMSDir(DirName, &Dir, MD_OK) != 0) {
	return(mserrcode);
    }
    QuickGetBodyFileName(Dir->UNIXDir, id, FileName);
    if (!offset) {
	ConsiderLoggingRead(FileName);
    }
    return(MS_GetPartialFile(FileName, Buf, BufLim, offset, remaining, ct));
}