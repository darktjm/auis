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
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/util/lib/RCS/gethome.c,v 2.11 1996/05/13 17:47:12 robr Exp $";
#endif


 
#include <pwd.h>
#include <stdio.h>
#include <errno.h>
#include <util.h>

#ifndef NULL
#define NULL 0
#endif

static char *home = NULL;

const char *gethome(const char *name)
{
    struct passwd *pw=NULL;
    const char *h=NULL;

    if (name==NULL) {
	if(home)
	    return home;

	h=getenv("HOME");

	/* Do not trust env vbl HOME if it is "/" because we
	    may be spawned by the guardian -- nsb */
	if(h==NULL || strcmp(h,"/")==0) {
	    pw = getpwuid(getuid());
	    if (pw != NULL) h = pw->pw_dir;
	}
	if (h != NULL) {
	    home = strdup(h);
	}
    } else {
	errno = 0;
	pw=getpwnam(name);
	if (pw != NULL) h = pw->pw_dir;
    }

    return h;
}
#ifdef TESTINGONLYTESTING
main()
{
    char Buf[500], *Ques, *Ans;

    for (;;) {
	printf("home dir of: "); fflush(stdout);
	if (gets(Buf) == NULL) exit(0);
	Ques = Buf[0] == '\0' ? NULL : Buf;
	Ans = gethome(Ques);
	printf("gethome(\"%s\") returns ``%s''.\n",
	       Ques == NULL ? "NULL" : Ques,
	       Ans == NULL ? "NULL" : Ans);
    }
}
#endif /* TESTINGONLYTESTING */
