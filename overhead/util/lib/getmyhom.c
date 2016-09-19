/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <stdio.h>
#include <util.h>

#ifndef NULL
#define NULL 0
#endif

static const char *myhome = NULL;

const char *getMyHome(void)
/* Like gethome(NULL): a less general version that gets only my own home directory, not anybody's.  Will fail if the environment variable isn't set. */
{
    const char *h;

    if (!myhome) {
	h = getenv("HOME");
	if (h != NULL) myhome = strdup(h);
	else myhome = "";
    }
    return (myhome);
}
#ifdef TESTINGONLYTESTING
int main(void)
{
    printf("getMyHome() returns ``%s''.\n", getMyHome());
}
#endif /* TESTINGONLYTESTING */
