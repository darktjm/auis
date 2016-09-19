/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
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
