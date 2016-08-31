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
#ifdef AFS_ENV
#include <netinet/in.h>
#include <afs/param.h>
#include <sys/ioctl.h>
#include <afs/errors.h>
#include <afs/prs_fs.h>
#include <afs/venus.h>
#endif /* AFS_ENV */

MS_PrefetchMessage(DirName, id, GetNext) /* prefetch a vice file */
char *DirName, *id;
int GetNext;
{
#ifdef AFS_ENV
    struct MS_Directory *Dir;
    struct ViceIoctl blob;
    int msgnum;
    char SnapshotDum[AMS_SNAPSHOTSIZE+1], FileName[1+MAXPATHLEN];

    if (AMS_ViceIsRunning) {
    blob.out_size = 0;
    blob.in_size = 0;

    if (!id || !*id) {
	/* Just prefetch the folder */
	sprintf(FileName, "%s/%s", DirName, MS_DIRNAME);
    } else {
	if (ReadOrFindMSDir(DirName, &Dir, MD_READ) != 0) {
	    return(mserrcode);
	}
	if (GetSnapshotByID(Dir, id, &msgnum, SnapshotDum)) {
	    CloseMSDir(Dir, MD_READ);
	    return(mserrcode);
	}
	if (GetNext && GetSnapshotByNumber(Dir, ++msgnum, SnapshotDum)) {
	    CloseMSDir(Dir, MD_READ);
	    return(mserrcode);
	}
	CacheDirectoryForClosing(Dir, MD_READ);
	QuickGetBodyFileName(Dir->UNIXDir, AMS_ID(SnapshotDum), FileName);
    }
    if (pioctl(FileName, VIOCPREFETCH, &blob) &&
	(RetryBodyFileName(FileName) < 0
	 || pioctl(FileName, VIOCPREFETCH, &blob))) {
	AMS_RETURN_ERRCODE(errno, EIN_PIOCTL, EVIA_PREFETCHMSG);
    }
    }
#endif /* AFS_ENV */
    return(0);
}

