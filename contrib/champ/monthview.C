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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/champ/RCS/monthview.C,v 1.8 1994/08/13 18:21:02 rr2b Stab74 $";
#endif

#include <andrewos.h>	/* time.h */

#include <month.H>
#include <monthview.H>
#include <fontdesc.H>
#include <graphic.H>
#include <champ.H>
#include <message.H>
#include <text.H>
#include <textview.H>
#include <im.H>
#include <style.H>
#include <util.h>

static class style *daystyle;


ATKdefineRegistry(monthview, view, monthview::InitializeClass);
#ifndef NORCSID
#endif
static int MonthLength(int  yr , int  mon);
static void LogEvent(struct eventnode  *en, class monthview  *self);
static void ClearAnnouncements(class monthview  *self);
static void AnnounceEvent(struct eventnode  *en, class monthview  *self);
static void ChangeMonth(class monthview  *self, int  change);


boolean monthview::InitializeClass()
{
    champ::ReadDatesFromChampPath(NULL);
    daystyle = new style;
    (daystyle)->AddNewFontFace( fontdesc_Bold);
    return(TRUE);
}

monthview::monthview()
{
	ATKinit;

    this->tv = NULL;
    this->skippedatstart = 0;
    this->mymonth = this->myyear = -1;
    this->t = new text;
    if (this->t == NULL) THROWONFAILURE( (FALSE));
    this->AnnounceArraySize = 0;
    THROWONFAILURE((TRUE));
}

monthview::~monthview()
{
	ATKinit;

    if (this->t) (this->t)->Destroy();
}

void monthview::SetTextview(class textview  *tv)
{
    this->tv = tv;
    if(this->tv && (this)->GetIM())
	(this->tv)->LinkTree( this);
}

static char *DayAbbrevs[] = {"M", "Tu", "W", "Th", "F", "Sa", "Su"};
static int RawMonthLengths[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static char *MonthNames[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
static char *Weekdays[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
static char *DayStrs[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31"};

static int MonthLength(int  yr , int  mon)
{
    if (mon != 1) return(RawMonthLengths[mon]);
    if (yr%4 != 0 || (yr%100 == 0 && yr%400 != 0)) return(28);
    return(29);
}

static void LogEvent(struct eventnode  *en, class monthview  *self)
{
    int len, slen;

    len = (self->t)->GetLength();
    slen = strlen(en->event);
    (self->t)->AlwaysInsertCharacters( len, en->event, slen);
    len += slen;
    (self->t)->AlwaysInsertCharacters( len, "\n", 1);
}

void monthview::ResetMonth(boolean  ForceRedraw)
{
    struct tm MyTime, *FullTime;
    int dayct, i, len, slen, mlen;
    class month *mon;
    time_t clock;
    char MyString[500];

    bzero(&MyTime, sizeof(struct tm));
    mon = (class month *) (this)->GetDataObject();
    this->mymonth = MyTime.tm_mon = (mon)->GetMonth();
    this->myyear = MyTime.tm_year = (mon)->GetYear();
    MyTime.tm_mday = 1;
    for (dayct = 0, i=0; i<this->mymonth; ++i) {
	dayct += MonthLength(this->myyear, i);
    }
    MyTime.tm_yday = dayct;
    clock = gtime(&MyTime);
    FullTime = gmtime(&clock);
    this->skippedatstart = FullTime->tm_wday - 1;
    if (this->skippedatstart == -1) this->skippedatstart = 6;
    (this->t)->Clear();
    mlen = MonthLength(this->myyear, this->mymonth);
    for (i=0; i<mlen; ++i) {
	champ::ClearAllFlaggedEvents();
	this->EventCt[i] = champ::FlagEventsMatchingDate(FullTime);
	len = (this->t)->GetLength();
	this->textpositions[i] = len;
	if (this->EventCt[i] > 0) {
	    sprintf(MyString, "%s %s %d, %d\n",	Weekdays[FullTime->tm_wday], MonthNames[this->mymonth], FullTime->tm_mday, 1900+FullTime->tm_year);
	    slen = strlen(MyString);
	    (this->t)->AlwaysInsertCharacters( len, MyString, slen);
	    (this->t)->AlwaysAddStyle( len, slen-1, daystyle);
	    champ::IterateFlaggedEvents((champ_ifefptr)LogEvent, (long)this);
	}
	bcopy(FullTime, &this->FullTimes[i], sizeof(struct tm));
	champ::IncrementDate(FullTime);
    }
    if (this->tv) {
	int top;

	clock = time(0);
	FullTime = localtime(&clock);
	if (FullTime->tm_year == this->myyear && FullTime->tm_mon == this->mymonth) {
	    top = this->textpositions[FullTime->tm_mday - 1];
	} else {
	    top = 0;
	}
	(this)->GrabTextview( ForceRedraw);
	(this->tv)->FrameDot( -1);
	(this->tv)->SetTopPosition( top);
	(this->tv)->SetDotPosition( top);
    }
}

void monthview::FullUpdate(enum view_UpdateType  type, long  left, long  top, long  width, long  height)
{
    class month *mon;
    static class fontdesc *plainfont = NULL, *boldfont = NULL;
    int xcenter, x, y, i, j, startday, highlight;
    struct rectangle Rect;
    char MyString[150], *StrToUse;

    if((type == view_LastPartialRedraw) || (type == view_FullRedraw)) {
	mon = (class month *) (this)->GetDataObject();
	if (this->mymonth != (mon)->GetMonth() || this->myyear !=(mon)->GetYear()) {
	    (this)->ResetMonth( (this->mymonth != -1));
	}
	if (!plainfont) {
	    plainfont = fontdesc::Create("andy", fontdesc_Plain, 12);
	}
	if (!boldfont) {
	    boldfont = fontdesc::Create("andy", fontdesc_Bold, 12);
	}
	startday = -this->skippedatstart;
	(this)->SetTransferMode( graphic_COPY);
	(this)->GetLogicalBounds( &Rect);
	(this)->SetFont( plainfont); 
	xcenter = Rect.left + (Rect.width/2);
	y = Rect.top + (Rect.height/16);
	(this)->MoveTo( xcenter, y);
	sprintf(MyString, "%s %d", MonthNames[this->mymonth], this->FullTimes[1].tm_year+1900);
	(this)->DrawString( MyString, graphic_BETWEENLEFTANDRIGHT | graphic_BETWEENTOPANDBASELINE);
	(this)->FillTrapezoid( Rect.left+25, Rect.top, 0, Rect.left+5, Rect.top+8, 20, (this)->BlackPattern());
	(this)->FillTrapezoid( Rect.left+5, Rect.top+8, 20, Rect.left+25, Rect.top+16, 0, (this)->BlackPattern());
	(this)->FillTrapezoid( Rect.left+Rect.width - 25, Rect.top, 0, Rect.left+Rect.width-25, Rect.top+8, 20, (this)->BlackPattern());
	(this)->FillTrapezoid( Rect.left+Rect.width-25, Rect.top+8, 20, Rect.left+Rect.width-25, Rect.top+16, 0, (this)->BlackPattern());
	for (i = -1; i<6; ++i) {
	    y += Rect.height/8;
	    x = Rect.left + (Rect.width/14);
	    for (j=0; j<7; ++j, ++startday) {
		highlight = 0;
		if (i < 0) {
		    StrToUse = DayAbbrevs[j];
		    --startday;
		} else if (startday <0 || startday >= MonthLength(this->myyear, this->mymonth)) {
		    StrToUse =  "   ";
		} else {
		    StrToUse = DayStrs[startday];
		    if (this->EventCt[startday] > 0) {
			highlight = 1;
			(this)->SetFont( boldfont);
			if (this->EventCt[startday] > 2) {
			    (this)->FillRectSize( x-8, y-6, 18, 14, (this)->BlackPattern());
			    (this)->SetTransferMode( graphic_WHITE);
			} else if (this->EventCt[startday] > 1) {
			    (this)->FillRectSize( x-8, y-6, 18, 14, (this)->GrayPattern( 3, 10));
			}
		    }
		}
		(this)->MoveTo( x, y);
		(this)->DrawString( StrToUse, graphic_BETWEENLEFTANDRIGHT | graphic_BETWEENTOPANDBASELINE);
		if (highlight) {
		    (this)->SetTransferMode( graphic_COPY);
		    (this)->SetFont( plainfont);
		}
		x += Rect.width/7;
	    }
	}
    }	
}

void monthview::Update()
{
    struct rectangle Rect;

    (this)->GetLogicalBounds( &Rect);
    (this)->SetTransferMode( graphic_COPY);
    (this)->FillRect( &Rect, (this)->WhitePattern());
    (this)->FullUpdate( view_FullRedraw, Rect.left, Rect.top, Rect.width, Rect.height);
}

static void ClearAnnouncements(class monthview  *self)
{
    long result;

    self->AnnounceArray[self->AnnounceArraySize] = "Continue";
    self->AnnounceArray[self->AnnounceArraySize+1] = NULL;
    message::MultipleChoiceQuestion(self, 50, "Scheduled events:", self->AnnounceArraySize, &result, self->AnnounceArray, NULL);
    self->AnnounceArraySize = 0;
}

static void AnnounceEvent(struct eventnode  *en, class monthview  *self)
{
    if (self->AnnounceArraySize > 8) {
	ClearAnnouncements(self);
    }
    self->AnnounceArray[self->AnnounceArraySize++] = en->event;
/*    message_DisplayString(self, 90, en->event); */
}


class view *monthview::Hit(enum view_MouseAction  action, long  x, long  y, long  numberOfClicks)
{
    int row, column, mday, tpos = 0;
    struct rectangle Rect;
    char Msg[250];

    if (action != view_LeftDown && action != view_RightDown) return((class view *) this);
    (this)->GetLogicalBounds( &Rect);
    row = (y-Rect.top)/(Rect.height/8);
    column = (x-Rect.left)/(Rect.width/7);
    if (row == 0) {
	if (x< (Rect.left + 25)) {
	    ChangeMonth(this, -1);
	} else if (x > (Rect.left + Rect.width -25)) {
	    ChangeMonth(this, 1);
	} else {
	    int i, total = 0;

	    for (i = 0; i<MonthLength(this->myyear, this->mymonth); ++i) {
		total += this->EventCt[i];
	    }
	    sprintf(Msg, "There are %d events scheduled in %s, %d", total, MonthNames[this->mymonth], 1900+this->myyear);
	    message::DisplayString(this, 10, Msg);
	}
    } else if (row == 1) {
	message::DisplayString(this, 10, "These are the days of our lives.");
    } else {
	mday = ((row-2) * 7) + column - this->skippedatstart;
	if (mday < 0 || mday >= MonthLength(this->myyear, this->mymonth)) {
	    message::DisplayString(this, 10, "The days we are allotted are few.");
	} else {
	    tpos = this->textpositions[mday];
	    if (this->EventCt[mday] > 0) {
		if (!this->tv){
		    champ::ClearAllFlaggedEvents();
		    champ::FlagEventsMatchingDate(&this->FullTimes[mday]);
		    champ::IterateFlaggedEvents((champ_ifefptr)AnnounceEvent, (long)this);
		    ClearAnnouncements(this);
		}
	    } else {
		sprintf(Msg, "No events scheduled on %s %d, %d\n", MonthNames[this->mymonth], mday+1, 1900+this->myyear);
		message::DisplayString(this, 10, Msg);
	    }
	}
    }
    if (this->tv) {
	(this)->GrabTextview( FALSE);
	(this->tv)->FrameDot( -1);
	(this->tv)->SetTopPosition( tpos);
    }
    return((class view *) this);
}


view_DSattributes monthview::DesiredSize(long  width, long  height, enum view_DSpass  pass, long  *dWidth, long  *dheight)
{
    *dWidth = 168;
    *dheight = 120;
    return(view_WidthFlexible | view_HeightFlexible);
}

static void ChangeMonth(class monthview  *self, int  change)
{
    int m, y;
    class month *mon;

    mon = (class month *) (self)->GetDataObject();
    m = (mon)->GetMonth();
    y = (mon)->GetYear();
    m += change;
    while (m > 11) {
	m -= 12;
	y += 1;
    }
    while (m < 0) {
	m += 12;
	y -= 1;
    }
    (mon)->SetMonthAndYear( m, y);
}

void monthview::GrabTextview(boolean  ForceRedraw)
{
    if (this->tv && this->t){
	if ((class dataobject *) this->t != ((class view *) this->tv)->dataobject) {
	    (this->tv)->SetDataObject( this->t);
	    ForceRedraw = TRUE;
	}
	if (ForceRedraw) {
	    struct rectangle Rect;

	    (this->tv)->GetLogicalBounds( &Rect);
	    if (Rect.height > 0 || Rect.width > 0) {
		(this->tv)->FullUpdate( view_FullRedraw, Rect.left, Rect.top, Rect.width, Rect.height);
	    }
	}
    }
}

void monthview::LinkTree(class view  *parent)
{
    (this)->view::LinkTree( parent);
    if(this->tv && (this)->GetIM())
	(this->tv)->LinkTree( this);
}
