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

/* ************************************************************ *\
	getvuid.c
	getvuid() (``get Vice UID'') gets the VUID associated with the ThisDomain authentication.
\* ************************************************************ */

#include <andrewos.h>	/* types.h */
#include <andyenv.h>
#include <stdio.h>
#include <errno.h>
#include <svcconf.h>
#include <util.h>

#ifdef WHITEPAGES_ENV
int getvuid()
{/* Return the Vice UID of the current process, or -1 with errno set */
    struct CellAuth *ca;
    int rc;

    CheckServiceConfiguration();
    if (AMS_ViceIsRunning) {
	rc = FindCell(ThisDomain, &ca);
	if (rc == 0 && ca != NULL) {
	    if (ca->ViceID < 0) FillInCell(ca);
	    if (ca->ViceID >= 0) return ca->ViceID;	/* ViceId in the home cell */
	}
	errno = EINVAL;
	return -1;
    } else {
	return getuid();
    }
}
#endif /* WHITEPAGES_ENV */
