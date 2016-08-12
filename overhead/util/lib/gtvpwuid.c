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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/util/lib/RCS/gtvpwuid.c,v 2.9 1992/12/15 21:10:10 rr2b Stab74 $";
#endif

/* ************************************************************ *\
	gtvpwuid.c
	getvpwuid(vuid) is like getpwuid(uid), but for Vice IDs
\* ************************************************************ */
#include <andyenv.h>
#include <andrewos.h> /* syslog.h */
#include <stdio.h>
#include <errno.h>
#include <util.h>
#ifdef WHITEPAGES_ENV
#include <pwd.h>
#include <wp.h>
#endif /* WHITEPAGES_ENV */
#include <svcconf.h>
#ifndef NULL
#define NULL 0
#endif
#ifdef WHITEPAGES_ENV
struct passwd *getvpwuid(vuid)
int vuid;
{/* Return a struct passwd for vuid, a Vice UID */
    wp_ErrorCode Res;
    wp_PrimeKey KVal;
    struct passwd *RV;
    extern struct passwd *_pw_getvpwkey();

    CheckServiceConfiguration();
    if (AMS_UseWP) {
	if (_pw_OpenWP() == 0) {
		errno = ENXIO;
		return NULL;
	}
	Res = wp_GetNIDOnly(vuid, &KVal);
	if (Res != wperr_NoError) {_pw_CloseWP(); errno = ETIMEDOUT; return NULL;}
	RV = _pw_getvpwkey(KVal);
	free(KVal);
	_pw_CloseWP();
	return RV;
    } else {
	return(getpwuid(vuid));
    }
}
#endif /* WHITEPAGES_ENV */
