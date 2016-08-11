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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/util/lib/RCS/venusop.c,v 2.10 1992/12/15 21:11:36 rr2b Stab74 $";
#endif

/*

	venusop.c -- routines that are specific to Vice file system, doing Venus operations

	VenusFlush(pathname): take a pathname and do an ``fs flush'' op.

	VenusFetch(pathname): take a pathname and pre-fetch it.

	VenusCancelStore(fid): prevent the close(fid) from storing the file.

	VenusFlushCallback(pathname): flush the callback, but not the data, for the named file.

	All routines return the result of the pioctl/ioctl and leave error codes in errno.
*/

#include <andrewos.h>
#include <errno.h>
#include <stdio.h>
#include <sys/ioctl.h>
#ifdef AFS_ENV
#include <netinet/in.h>
#include <afs/param.h>
#include <afs/venus.h>
#endif /* AFS_ENV */

int VenusFlush(pname)
char *pname;
{
#ifdef AFS_ENV
    if (ViceIsRunning()) {
	struct ViceIoctl dummy;
	dummy.in_size = 0;
	dummy.out_size = 0;
	return (pioctl(pname, VIOCFLUSH, &dummy, 1));
    } else
#endif /* AFS_ENV */
	return 0;
}

int VenusFlushCallback(pname)
char *pname;
{
#ifdef AFS_ENV
    if (ViceIsRunning()) {
	struct ViceIoctl dummy;
	dummy.in_size = 0;
	dummy.out_size = 0;
	return(pioctl(pname, VIOCFLUSHCB, &dummy, 1));
    } else
#endif /* AFS_ENV */
	return 0;
}

int VenusFetch(pname)
char *pname;
{
#ifdef AFS_ENV
    if (ViceIsRunning()) {
	struct ViceIoctl dummy;
	dummy.in_size = 0;
	dummy.out_size = 0;
	return(pioctl(pname, VIOCPREFETCH, &dummy, 1));
    } else
#endif /* AFS_ENV */
	return 0;
}

int VenusCancelStore(fid)
int fid;
{
#ifdef AFS_ENV
    if (ViceIsRunning()) {
	struct ViceIoctl dummy;
	dummy.in_size = 0;
	dummy.out_size = 0;
	return(ioctl(fid, VIOCABORT, &dummy));
    } else
#endif /* AFS_ENV */
	return 0;
}
