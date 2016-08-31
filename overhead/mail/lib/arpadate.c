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

#include <andrewos.h> /* sys/time.h */

/*
		arpadate.c -- return current time in ARPAnet format
*/


char *arpadate()
{
    static char time_out[] = "Mon, 03 Feb 1986 10:54:23 +HHMM (MEZ AUSSS FMT 664)\n";
    static const char * const days[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
    static const char * const months[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    long int the_time;
    struct tm *now;
    static char zone[] = "GMT+HH:MM EXTRA SPACE";
    static char ZoneComment[] = " (GMT+HH:MM EXTRA SPACE)";
    static int gmtYear, gmtDay, gmtHour, gmtMin;
    static long int OffsetHours, OffsetMinutes;
    static char OffsetSign;
    static long int GMTDiff;	/* what to add to GMT (in minutes) to get local time (in minutes), or ``minutesEast'' */

/* The zone returned by gettimeofday is static for the life of the kernel; it is localtime() that judges whether the local time is daylight or not.  Get the GMT info and compare it to the localtime result to determine the current offset from GMT. */

    the_time = osi_GetSecs();
    osi_SetZone();
    now = gmtime(&the_time);
    gmtYear = now->tm_year;
    gmtDay = now->tm_yday;
    gmtHour = now->tm_hour;
    gmtMin = now->tm_min;
    now = localtime(&the_time);
    if (gmtYear > now->tm_year) GMTDiff = - (60*24);
    else if (gmtYear < now->tm_year) GMTDiff = (60*24);
    else if (gmtDay > now->tm_yday) GMTDiff = - (60*24);
    else if (gmtDay < now->tm_yday) GMTDiff = (60*24);
    else GMTDiff = 0;
    GMTDiff += (now->tm_hour - gmtHour) * 60;
    GMTDiff += (now->tm_min - gmtMin);

    if (GMTDiff < 0) {
	OffsetMinutes = - GMTDiff; OffsetSign = '-';
    } else {
	OffsetMinutes = GMTDiff; OffsetSign = '+';
    }
    OffsetHours = OffsetMinutes / 60;
    OffsetMinutes = OffsetMinutes % 60;	/* convert to hours and minutes */

/* Make primary zone designator numeric, with a comment indicating local zone name, if available. */
    strcpy(zone, osi_ZoneNames[(osi_IsEverDaylight && now->tm_isdst ? 1 : 0)]);
    if (zone[0] == 'G' && zone[1] == 'M' && zone[2] == 'T' && zone[3] != '\0')
	ZoneComment[0] = '\0';	/* don't use one */
    else
	sprintf(ZoneComment, " (%s)", zone);

    sprintf(time_out, "%s, %2d %s %4d %02d:%02d:%02d %c%02ld%02ld%s\n",
	     days[now->tm_wday],
	     now->tm_mday,
	     months[now->tm_mon],
	     now->tm_year + 1900,
	     now->tm_hour, now->tm_min, now->tm_sec,
	     OffsetSign, OffsetHours, OffsetMinutes,
	     ZoneComment);
    return time_out;
}

#ifdef TESTINGONLYTESTING
#include <stdio.h>
main()
{
    fputs(arpadate(), stdout);
}
#endif /* TESTINGONLYTESTING */
