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
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/ams/libs/ms/RCS/altsnap.c,v 2.7 1992/12/15 21:17:22 rr2b Stab74 $";
#endif

#include <stdio.h>
#include <ms.h>

MS_AlterSnapshot(dirname, id, NewSnapshot, Code) 
char *dirname, *id, *NewSnapshot;
int Code;
{
    struct MS_Directory *Dir;
    int msgnum, errsave, i;
    char SnapshotDum[AMS_SNAPSHOTSIZE], *s, *t, DateBuf[AMS_DATESIZE];

    if (MSDebugging & 1) { /* Debugging SHOULD go to stdout -- nsb 5/16/86 */
	printf("MS_AlterSnapshot %s %s code %d\n", dirname, id, Code);
	fputs("Snapshot: 0x", stdout);
	for (i=0; i<AMS_SNAPSHOTSIZE; i++) fprintf(stdout, "%02x", (unsigned char) NewSnapshot[i]);
	fputs(")\n", stdout);
    }

    if (ReadOrFindMSDir(dirname, &Dir, MD_WRITE) != 0) {
	return(mserrcode);
    }
    if (GetSnapshotByID(Dir, id, &msgnum, SnapshotDum)) {
	errsave = mserrcode; 
	CloseMSDir(Dir, MD_WRITE);
	return(errsave);
    }
    switch(Code) {
	case ASS_REPLACE_ALL:
	    strcpy(DateBuf, AMS_DATE(SnapshotDum));
	    bcopy(NewSnapshot, SnapshotDum, AMS_SNAPSHOTSIZE);
	    strcpy(AMS_DATE(SnapshotDum), DateBuf);
	    break;
	case ASS_REPLACE_ATT_CAPT:
	    bcopy(AMS_CAPTION(NewSnapshot), AMS_CAPTION(SnapshotDum), AMS_CAPTIONSIZE);
	    /* DROP THROUGH TO NEXT CLAUSE */
	case ASS_REPLACE_ATTRIBUTES:
	    bcopy(AMS_ATTRIBUTES(NewSnapshot), AMS_ATTRIBUTES(SnapshotDum), AMS_ATTRIBUTESIZE);
	    break;
	case ASS_OR_ATTRIBUTES:
	    for (i=0, s=AMS_ATTRIBUTES(NewSnapshot), t = AMS_ATTRIBUTES(SnapshotDum); i<AMS_ATTRIBUTESIZE; ++i, ++s, ++t) {
		*t |= *s;
	    }
	    break;
	case ASS_AND_ATTRIBUTES:
	    for (i=0, s=AMS_ATTRIBUTES(NewSnapshot), t = AMS_ATTRIBUTES(SnapshotDum); i<AMS_ATTRIBUTESIZE; ++i, ++s, ++t) {
		*t &= *s;
	    }
	    break;
    }
    debug(4, ("Altering snapshot of  message %d %s\n", msgnum, AMS_CAPTION(SnapshotDum)));
    if (RewriteSnapshotInDirectory(Dir, msgnum, SnapshotDum)) {
	errsave = mserrcode;
	CloseMSDir(Dir, MD_WRITE);
	return(errsave);
    }
    if (CacheDirectoryForClosing(Dir, MD_WRITE)) {
	return(mserrcode);
    }
    return(0);
}
