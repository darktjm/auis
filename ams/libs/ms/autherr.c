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
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/ams/libs/ms/RCS/autherr.c,v 2.10 1992/12/15 21:17:22 rr2b Stab74 $";
#endif


 

#include <ms.h>
#ifdef AFS_ENV
#include <netinet/in.h>
#include <afs/param.h>
#include <sys/ioctl.h>
#include <afs/errors.h>
#include <afs/prs_fs.h>
#include <afs/venus.h>
#endif /* AFS_ENV */

extern char home[];

MS_CheckAuthentication(Authenticated)
int *Authenticated;
{
#ifdef AFS_ENV
    struct ViceIoctl    blob;
    long authcode;
#endif /* AFS_ENV */

    *Authenticated = 1;
#ifdef AFS_ENV
    if (AMS_ViceIsRunning) {
    authcode = 0;
    blob.out = (char *) &authcode;
    blob.out_size = sizeof authcode;
    blob.in_size = 0;
    if (pioctl(home, VIOCCKCONN, &blob, 1)) {
	AMS_RETURN_ERRCODE(errno, EIN_PIOCTL, EVIA_CHECKAUTH);
    } else {
	if (authcode != 0) {
	    *Authenticated = 0;
	}
    }
    }
#endif /* AFS_ENV */
    return(0);
}

