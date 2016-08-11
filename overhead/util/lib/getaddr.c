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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/util/lib/RCS/getaddr.c,v 2.9 1995/11/07 20:17:10 robr Stab74 $";
#endif


 

/* getaddr -- get our internet address */

#include <andrewos.h>		/* types.h */
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

#if SY_AIXx && !defined(AF_LINK)
/* For binary compatibility with AIX 3.2... */
#define	AF_LINK		18		/* Link layer interface */
#endif

#define NIFS		6

/* Return our internet address as a long in network byte order.  Returns zero if it can't find one. */
unsigned long getaddr ()
{
    int     s;
    int     i, len;
    struct ifconf   ifc;
    struct ifreq    ifs[NIFS];
    struct ifreq   *ifp;
    struct sockaddr_in *a;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0)
	return 0;
    ifc.ifc_len = sizeof(ifs);
    ifc.ifc_buf = (caddr_t) ifs;
    memset(ifs, 0, sizeof(ifs));
    i = ioctl(s, SIOCGIFCONF, &ifc);
    close(s);
    if (i < 0)
	return 0;
    len = ifc.ifc_len / sizeof(struct ifreq);
    if (len > NIFS)
	len = NIFS;
#if SY_AIX4
    /* AIX 4 packs these structs tightly.  We must use the length of the address
     * to advance to the next struct.
     */
    ;
    for (ifp = ifs; ifp - ifs < ifc.ifc_len; ++i, ifp = (struct ifreq *)((char *)ifp + IFNAMSIZ + a->sin_len)) {
	a = (struct sockaddr_in *) &(ifp->ifr_addr);
	if (a->sin_family != AF_INET)
	    continue;
	if (a->sin_addr.s_addr != 0 && strcmp(ifp->ifr_name, "lo0") != 0)
	    return a->sin_addr.s_addr;
    }
#else
    for (i = 0; i < len; ++i) {
	a = (struct sockaddr_in *) &ifs[i].ifr_addr;
#if SY_AIXx
	if (a->sin_family == AF_LINK)
	    continue;	/* Skip <link> addresses */
#endif
	if (a->sin_addr.s_addr != 0 && strcmp(ifs[i].ifr_name, "lo0") != 0)
	    return a->sin_addr.s_addr;
    }
#endif
    return 0;
}

#ifdef TESTINGONLYTESTING
#include <stdio.h>
#include <arpa/inet.h>
main(argc,argv)
int argc;
char *argv[];
{
  struct in_addr a;

  a.s_addr = getaddr();
  printf("%ul == %s\n", a.s_addr, inet_ntoa(a));
}
#endif /* TESTINGONLYTESTING */
