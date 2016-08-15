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
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/util/lib/RCS/hname.c,v 1.13 1995/03/18 17:30:57 rr2b Stab74 $";
#endif

/*
	hname.c -- Get host name, including domain name if necessary.
*/

 

#include <stdio.h>
#include <errno.h>
#include <andyenv.h>
#include <system.h>
#include <util.h>

int GetHostDomainName(char *buf, int buflen)
{/* Returns like gethostname(); optionally appends the result of getdomainname(). */
    int Code;
    int hlen, dlen;
    const char *DomSfx;
#ifdef GETDOMAIN_ENV
    char Dom[400];
#endif /* GETDOMAIN_ENV */

    buf[buflen-1] = '\0';
    Code = gethostname(buf, buflen);
    if (Code != 0) return Code;
    if (buf[buflen-1] != '\0') {errno = EDOM; return -1;}
    /* Heuristically append ThisDomainSuffix, or the result from getdomainname(). */
    /* Do none of this if the host name already seems to be a fully-qualified domain name. */
    if (strchr(buf, '.') == 0) {
	DomSfx = GetConfiguration("ThisDomainSuffix");
	Code = 0;
#ifdef GETDOMAIN_ENV
	if (DomSfx == NULL) {	/* if there wasn't any such AndrewSetup value */
	    Dom[sizeof(Dom)-1] = '\0';
	    Code = getdomainname(Dom, sizeof(Dom));
	    if (Dom[sizeof(Dom)-1] != '\0') {errno = EDOM; Code = -1;}
	    if (Code == 0 && Dom[0] != '\0') DomSfx = Dom;
	}
#endif /* #ifdef GETDOMAIN_ENV */
	if (DomSfx != NULL && DomSfx[0] != '\0' && strcmp(DomSfx, ".") != 0) {
	    hlen = strlen(buf);	/* append only if non-null */
	    dlen = strlen(DomSfx);
	    /* Is the domain name the same as this host name? */
	    if (hlen == dlen && ULstrcmp(buf, DomSfx) == 0) return 0;
	    /* Is the domain name already appended to the host name?  (Great!) */
	    if (hlen > dlen && ULstrcmp(buf+(hlen-dlen), DomSfx) == 0 && buf[hlen-dlen-1] == '.')
		return 0;
	    /* Have to append the two.  Make sure there's room. */
	    strncpy(buf+hlen, ".", buflen-hlen);
	    strncpy(buf+hlen+1, DomSfx, buflen-hlen-1);
	    if (buf[buflen-1] != '\0') {errno = EDOM; Code = -1;}
	}
	return Code;
    }
    return 0;
}

#ifdef TESTINGONLYTESTING
#include <stdio.h>
main()
{
    char Dogs[1000];
    int RC;

    RC = GetHostDomainName(Dogs, sizeof(Dogs));
    if (RC != 0) {
	printf("GetHostDomainName() failed with code %d\n");
    } else {
	printf("GetHostDomainName() returns ``%s''.\n", Dogs);
    }
    exit(0);
}
#endif /* TESTINGONLYTESTING */
