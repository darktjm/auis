/* **************************************************** *\
Copyright 1989 Nathaniel S. Borenstein
Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear in
supporting documentation, and that the name of 
Nathaniel S. Borenstein not be used in
advertising or publicity pertaining to distribution of the software
without specific, written prior permission. 

Nathaniel S. Borenstein DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
Nathaniel S. Borenstein BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY
DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
\* ***************************************************** */
#include <andrewos.h>
#include <time.h>
#include <clockob.H>
#include <observable.H>
#include <contimer.H>

static class contimer *clockob_contimer;
static const char * const Days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
static const char * const Months[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
static struct tm *LatestLocalTime;


ATKdefineRegistry(clockob, conob, clockob::InitializeClass);
static void CheckTime(class contimer * dummy);


static void CheckTime(class contimer * dummy)
{
    long clock;
    
    clock = time(0);
    LatestLocalTime = localtime(&clock);
}

boolean clockob::InitializeClass()
{
    clockob_contimer = new contimer;
    (clockob_contimer)->SetInterval( 1000); /* 1 second */
    (clockob_contimer)->SetDataCollectionProc( (collection_fptr)CheckTime);
    CheckTime(clockob_contimer); /* initialize time */
    return(TRUE);
}

clockob::clockob()
{
	ATKinit;

    (clockob_contimer)->AddObserver( this);
    (this)->SetClockPart( CP_MIN);
    THROWONFAILURE((TRUE));
}

clockob::~clockob()
{
	ATKinit;

    (clockob_contimer)->RemoveObserver( this);
}

void clockob::ObservedChanged(class observable  *ct, long  code)
{
    if (code == observable_OBJECTDESTROYED) {
	clockob_contimer = NULL; /* eliminate pointer to it */
	(this)->Destroy(); /* can't go on without a timer */
	return;
    }
    switch(this->clockpart) {
	case CP_SEC:
	    (this)->SetNumval( LatestLocalTime->tm_sec);
	    break;
	case CP_MIN:
	    (this)->SetNumval( LatestLocalTime->tm_min);
	    break;
	case CP_HR:
	    if (LatestLocalTime->tm_hour > 12) {
		(this)->SetNumval( LatestLocalTime->tm_hour
				  - 12);
	    } else {
		(this)->SetNumval( LatestLocalTime->tm_hour);
	    }
	    break;
	case CP_HRMIL:
	    (this)->SetNumval( LatestLocalTime->tm_hour);
	    break;
	case CP_MDAY:
	    (this)->SetNumval( LatestLocalTime->tm_mday);
	    break;
	case CP_WDAY:
	    (this)->SetNumval( LatestLocalTime->tm_wday);
	    (this)->SetStrval(
		Days[LatestLocalTime->tm_wday]);
	    break;
	case CP_MON:
	    (this)->SetNumval( LatestLocalTime->tm_mon);
	    (this)->SetStrval(
		Months[LatestLocalTime->tm_mon]);
	    break;
	case CP_YEAR:
	    (this)->SetNumval(
		LatestLocalTime->tm_year + 1900);
	    break;
	case CP_YDAY:
	    (this)->SetNumval( LatestLocalTime->tm_yday);
	    break;
    }
    (this)->NotifyObservers( observable_OBJECTCHANGED);
}

void clockob::SetClockPart(int  part)
{
    this->clockpart = part;
}

void
clockob::WriteState(FILE  *fp)
{
    fprintf(fp, "*a %d\n", this->clockpart);
    (this)->conob::WriteState( fp);
}

void
clockob::HandleDataLine(char  *line)
{
    if (*line == '*' && *(line+1) == 'a') {
	(this)->SetClockPart( atoi(line+3));
    } else {
	(this)->conob::HandleDataLine( line);
    }
}

void clockob::GetStringToDisplay(char  *buf, int  len, boolean  IsClick)
{
    const char *dt = (this)->GetDisplayTemplate();

    if (dt == NULL || *dt == '!' || IsClick) {
	int hr;
	if (LatestLocalTime->tm_hour > 12) {
	    hr = LatestLocalTime->tm_hour - 12;
	} else {
	    hr = LatestLocalTime->tm_hour;
	}
	sprintf(buf, "%d:%02d:%02d %cM, %s, %s %d, %d", hr,
		LatestLocalTime->tm_min,
		LatestLocalTime->tm_sec,
		(LatestLocalTime->tm_hour > 11) ? 'P' : 'A',
		Days[LatestLocalTime->tm_wday],
		Months[LatestLocalTime->tm_mon],
		LatestLocalTime->tm_mday,
		LatestLocalTime->tm_year + 1900);
    } else {
	(this)->conob::GetStringToDisplay( buf, len, IsClick);
    }
}
