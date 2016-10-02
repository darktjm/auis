/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>		/* sys/time.h */
#include <util.h>
#include <stdio.h>
#include <time.h>

const char *NiceTime(long int Time)
{/* Like ctime, but do a more attractive job and don't end with a newline. */
    static char Res[50];
    strftime(Res, sizeof(Res), "%d %b %Y at %T %Z", localtime(&Time));
    return Res;
}
