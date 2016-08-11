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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/console/lib/RCS/timemon.C,v 1.3 1994/08/12 18:53:39 rr2b Stab74 $";
#endif


 

#include <andrewos.h> /* sys/time.h sys/file.h */

#include <consoleClass.H>
#include <console.h>
#include <environ.H>
#include <environment.H>
#include <sitevars.h>

extern boolean WasUDPAction;

#ifndef NORCSID
#endif
void InitClock();
void CheckClock(class consoleClass  *self);


void InitClock() {
    mydbg(("entering: InitClocks\n"));
    Numbers[CLOCKSECONDS].RawText = (char *)malloc(20);
    Numbers[CLOCKMINUTES].RawText = (char *)malloc(10);
    Numbers[CLOCKALL].RawText = (char *)malloc(40);
    Numbers[DATE].RawText = (char *)malloc(30);
}

void CheckClock(class consoleClass  *self)
     {
    time_t    now;
    struct tm  *currentTime;
    boolean IsPM = FALSE;
    int     hours;
    static char *DayOfWeek[] = {
	"Sunday", "Monday", "Tuesday", "Wednesday", 
	"Thursday", "Friday", "Saturday", "Sunday", 0};
    static int OldHours = 0,
	OldMinutes = 0,
	OldDate = 0,
    TFHC = -1;/* Twenty-Four Hour Clock */
    static long LastUDPAction = 0;

    mydbg(("entering: CheckClock\n"));
    now = time(0);
    if (Numbers[UDPIDLE].IsDisplaying) {
	if (WasUDPAction) {
	    LastUDPAction = now;
	    WasUDPAction = FALSE;
	}
	NewValue(self, &Numbers[UDPIDLE], now - LastUDPAction, NULL, FALSE);
    }
    if (TFHC == -1){
	TFHC = environ::GetProfileSwitch("console.tfhc", FALSE);
    }
    currentTime = localtime(&now);
    hours = currentTime->tm_hour;
    if (!TFHC && hours >= 12) {
        IsPM = TRUE;
	hours -= 12;
    }
    if (!TFHC && hours == 0) {
	hours = 12;
    }
    if (Numbers[CLOCKSECONDS].IsDisplaying) {
	OldMinutes = -1; /* Hack to avoid overwriting min hand */
	OldHours = -1; /* Hack to avoid overwriting hour hand */
	if (!TFHC){
	    sprintf(Numbers[CLOCKSECONDS].RawText, "%2d:%02d:%02d %cM",
	    hours, currentTime->tm_min, currentTime->tm_sec, IsPM ? 'P' : 'A');
	}
	else{
	    sprintf(Numbers[CLOCKSECONDS].RawText, "%02d:%02d:%02d",
	    hours, currentTime->tm_min, currentTime->tm_sec);
	}
	NewValue(self, &Numbers[CLOCKSECONDS], currentTime->tm_sec, NULL, TRUE);
    }
    if (Numbers[CLOCKMINUTES].IsDisplaying && OldMinutes != currentTime->tm_min) {
	OldMinutes = currentTime->tm_min;
	OldHours = -1; /* Hack to avoid overwriting hour hand */
	if (!TFHC){
	    sprintf(Numbers[CLOCKMINUTES].RawText, "%2d:%02d %cM",
	    hours, currentTime->tm_min, IsPM ? 'P' : 'A');
	}
	else{
	    sprintf(Numbers[CLOCKMINUTES].RawText, "%02d:%02d",
	    hours, currentTime->tm_min);
	}
	NewValue(self, &Numbers[CLOCKMINUTES], currentTime->tm_min, NULL, TRUE);
    }
    if (Numbers[CLOCKHOURS].IsDisplaying && OldHours != hours) {
	OldHours = hours;
	NewValue(self, &Numbers[CLOCKHOURS], hours, NULL, TRUE);
    }
    if (Numbers[CLOCKHOURFIFTHS].IsDisplaying) {
        NewValue(self, &Numbers[CLOCKHOURFIFTHS], (hours >= 12 ? hours -12 : hours) * 5+ currentTime->tm_min/12, NULL, TRUE);
    }
    if (!TFHC){
	sprintf(Numbers[CLOCKALL].RawText, "%2d:%02d:%02d %cM, %s, %d/%d/%d",
		hours, currentTime->tm_min, currentTime->tm_sec,
		IsPM ? 'P' : 'A',
		DayOfWeek[currentTime->tm_wday], currentTime->tm_mon + 1, 
		currentTime->tm_mday, currentTime->tm_year);
    }
    else{
	sprintf(Numbers[CLOCKALL].RawText, "%02d:%02d:%02d, %s, %d/%d/%d",
		hours, currentTime->tm_min, currentTime->tm_sec,
		DayOfWeek[currentTime->tm_wday], currentTime->tm_mon + 1, 
		currentTime->tm_mday, currentTime->tm_year);
    }
	NewValue(self, &Numbers[CLOCKALL], currentTime->tm_yday *86400 + currentTime->tm_min *60 + currentTime->tm_hour *3600 + currentTime->tm_sec, NULL, TRUE);
    if (Numbers[DATE].IsDisplaying && currentTime->tm_yday != OldDate) {
	OldDate = currentTime->tm_yday;
	sprintf(Numbers[DATE].RawText, "%s %d/%d/%d",
		DayOfWeek[currentTime->tm_wday], currentTime->tm_mon + 1, currentTime->tm_mday, currentTime->tm_year);
	NewValue(self, &Numbers[DATE], 0, NULL, TRUE);
    }
}

