/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>		/* sys/time.h */
#include <util.h>
#include <stdio.h>

const char *NiceTime(long int Time)
{/* Like ctime, but do a more attractive job and don't end with a newline. */
    static char Res[50];
    static const char * const Mon[] = {	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    struct tm *This;

    This = localtime(&Time);
    osi_SetZone();
    sprintf(Res, "%d %s %04d at %d:%02d:%02d %s",
	     This->tm_mday,
	     Mon[This->tm_mon],
	     This->tm_year + 1900,
	     This->tm_hour,
	     This->tm_min,
	     This->tm_sec,
	     osi_ZoneNames[(osi_IsEverDaylight && This->tm_isdst ? 1 : 0)]);
    return Res;
}
