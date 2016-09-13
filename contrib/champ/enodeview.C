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
#include "butter.H"
#include "butterview.H"
#include "enode.H"
#include "lpair.H"
#include "message.H"
#include "chimpview.H"
#include "chimp.H"
#include "chlistview.H"
#include "enodeview.H"

static const char * const WkDays[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Any Day", NULL};
static const char * const months[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December", "Any Month", NULL};
static const char * const HebrewMonths[] = {"Tishri", "Heshvan", "Kislev", "Tevet", "Shvat", "First Adar", "Second Adar", "Nisan", "Iyyar", "Sivan", "Tammuz", "Av", "Elul", "Any Month", NULL};
static const char * const DefaultString[] = {"calendar system", "year", "month", "date", "day of week", "week", "hour", "minute", "Add Event", "Delete Event"};
static const char * const WeekStrs[] = {"First Week in Month", "Second Week in Month", "Third Week in Month", "Fourth Week in Month", "Fifth Week in Month", "Last Week in Month", "Any Week", NULL};
static const char * const LandmarkStrs[] = {"First Sunday of Advent", "First Sunday after Christmas", "First Sunday after Epiphany", "Easter", NULL};
static const char * const SysChoices[] = {"Gregorian", "Hebrew", "Ecclesiastical", NULL};
static const char * const BooleanChoices[] = {"Yes", "No", NULL};


ATKdefineRegistry(enodeview, lpair, NULL);

static void ButtHit(class enodeview  *self, int  buttcode, class butter  *butter, enum view_MouseAction  action);
void ResetButterTexts(class enodeview  *self);


static void ButtHit(class enodeview  *self, int  buttcode, class butter  *butter, enum view_MouseAction  action)
{
    long result, pos, len;
    struct eventnode *en;
    class enode *enode;
    char buf[1500], *dbuf;

    if (action == view_LeftUp || action == view_RightUp) {
	enode = (class enode *) (self)->GetDataObject();
	if (!enode) {
	    message::DisplayString(self, 10, "No event is currently on display.");
	    return;
	}
	if (buttcode == BUTT_ADD) {
	    struct eventnode *adden;
	    
	    if (message::AskForString(self, 75, "Describe the event: ", NULL, buf, sizeof(buf)) < 0) return;
	    adden = (struct eventnode *) malloc(sizeof(struct eventnode));
	    if (!adden) {
		message::DisplayString(self, 10, "Out of memory!");
		return;
	    }
	    adden->event = strdup(buf);
	    if (!adden->event) {
		message::DisplayString(self, 10, "Out of memory!");
		return;
	    }
	    adden->flagged = 0;
	    adden->next = NULL;
	    adden->ds.calsys = CALSYS_GREGORIAN;
	    adden->ds.sys.gd.year = -1;
	    adden->ds.sys.gd.month = -1;
	    adden->ds.sys.gd.day = -1;
	    adden->ds.sys.gd.hour = -1;
	    adden->ds.sys.gd.min = -1;
	    adden->ds.sys.gd.wkday = -1;
	    adden->ds.sys.gd.wkdayselector = -1;
	    (enode->mychimp)->AddNew( adden);
	    (enode)->SetEvent( adden);
	    (self->mychimpview->lv)->ActivateItem( (enode->mychimp)->GetLength() - 1);
	    message::DisplayString(self, 10, "Added new event to end of list.");
	    pos = (enode->mychimp)->GetLength();
	    (self->mychimpview->lv)->FrameDot( pos);
	    return;
	}
	en = (enode)->GetEvent();
	if (!en) {
	    message::DisplayString(self, 10, "No event is currently on display.");
	    return;
	}
	if (buttcode == BUTT_DEL) {
	    pos = (self->mychimpview->lv)->GetDotPosition();
	    len = (self->mychimpview->lv)->GetDotLength();
	    dbuf = (char *)malloc(len);
	    if (!dbuf) {
		message::DisplayString(self, 10, "Out of memory!");
		return;
	    }
	    (enode->mychimp)->CopySubString( pos, len-1, dbuf, FALSE);
	    sprintf(buf, "Really delete event `%s'? ", dbuf);
	    if (message::MultipleChoiceQuestion(self, 50, buf, 1, &result, BooleanChoices, NULL) < 0) return;
	    if (result != 0) return;
	    (enode->mychimp)->DeleteItem( dbuf);
	    free(dbuf);
	    message::DisplayString(self, 10, "Deleted event as requested.");
	    return;
	}
	switch(en->ds.calsys) {
	    case CALSYS_HEBREW:
		switch(buttcode) {
		    case BUTT_SYS:
			if (message::MultipleChoiceQuestion(self, 50, "What calendar system do you want to use for this event? ", 1, &result, SysChoices, NULL) < 0) return;
			if (result == 0) en->ds.calsys = CALSYS_GREGORIAN;
			if (result == 1) en->ds.calsys = CALSYS_HEBREW;
			if (result == 2) en->ds.calsys = CALSYS_ECCLESIASTICAL;
			break;
		    case BUTT_YEAR:
			if (message::MultipleChoiceQuestion(self, 50, "Is this an event that happens every year? ", 0, &result, BooleanChoices, NULL) < 0) return;
			if (result == 0) {
			    en->ds.sys.hd.year = -1;
			} else {
			    if (message::AskForString(self, 50, "What year does it happen in? ", NULL, buf, sizeof(buf)) < 0) return;
			    result = atoi(buf);
			    if (result < 5730 || result > 5780) {
				message::DisplayString(self, 25, "Valid answers are 5730 to 5780.");
				return;
			    } else {
				en->ds.sys.hd.year = result;
			    }
			}
			break;
		    case BUTT_MON:
			result = en->ds.sys.hd.month;
			if (result < 0) result = 13;
			if (message::MultipleChoiceQuestion(self, 50, "In what month does this event happen? ", result, &result, HebrewMonths, NULL) < 0) return;
			if (result >= 14 || result < 0) result = -1;
			en->ds.sys.hd.month = result;
			break;
		    case BUTT_DAY:
			if (message::MultipleChoiceQuestion(self, 50, "Is this an event that happens every day? ", 0, &result, BooleanChoices, NULL) < 0) return;
			if (result == 0) {
			    en->ds.sys.hd.day = -1;
			} else {
			    if (message::AskForString(self, 50, "On what day does it happen? ", NULL, buf, sizeof(buf)) < 0) return;
			    result = atoi(buf);
			    if (result <= 0 || result > 38) {
				message::DisplayString(self, 25, "Valid answers are 1 to 38 (a high max, for date wrapping).");
				return;
			    } else {
				en->ds.sys.hd.day = result;
			    }
			}
			break;
		    default:
			message::DisplayString(self, 10, "This field is not used by the Hebrew calendar.");
		}
		break;
	    case CALSYS_GREGORIAN:
		switch(buttcode) {
		    case BUTT_SYS:
			if (message::MultipleChoiceQuestion(self, 50, "What calendar system do you want to use for this event? ", 0, &result, SysChoices, NULL) < 0) return;
			if (result == 0) en->ds.calsys = CALSYS_GREGORIAN;
			if (result == 1) en->ds.calsys = CALSYS_HEBREW;
			if (result == 2) en->ds.calsys = CALSYS_ECCLESIASTICAL;
			break;
		    case BUTT_YEAR:
			if (message::MultipleChoiceQuestion(self, 50, "Is this an event that happens every year? ", 0, &result, BooleanChoices, NULL) < 0) return;
			if (result == 0) {
			    en->ds.sys.gd.year = -1;
			} else {
			    if (message::AskForString(self, 50, "What year does it happen in? ", NULL, buf, sizeof(buf)) < 0) return;
			    result = atoi(buf);
			    if (result < 1900 || result > 2100) {
				message::DisplayString(self, 25, "Valid answers are 1900 to 2100.");
				return;
			    } else {
				en->ds.sys.gd.year = result - 1900;
			    }
			}
			break;
		    case BUTT_MON:
			result = en->ds.sys.gd.month;
			if (result < 0) result = 12;
			if (message::MultipleChoiceQuestion(self, 50, "In what month does this event happen? ", result, &result, months, NULL) < 0) return;
			if (result >= 12 || result < 0) result = -1;
			en->ds.sys.gd.month = result;
			break;
		    case BUTT_DAY:
			if (message::MultipleChoiceQuestion(self, 50, "Is this an event that happens every day? ", 0, &result, BooleanChoices, NULL) < 0) return;
			if (result == 0) {
			    en->ds.sys.gd.day = -1;
			} else {
			    if (message::AskForString(self, 50, "On what day does it happen? ", NULL, buf, sizeof(buf)) < 0) return;
			    result = atoi(buf);
			    if (result <= 0 || result > 31) {
				message::DisplayString(self, 25, "Valid answers are 1 to 31.");
				return;
			    } else {
				en->ds.sys.gd.day = result;
			    }
			}
			break;
		    case BUTT_WKDAY:
			result = en->ds.sys.gd.wkday;
			if (result < 0) result = 7;
			if (message::MultipleChoiceQuestion(self, 50, "On what day does this event happen? ", result, &result, WkDays, NULL) < 0) return;
			if (result >= 7 || result < 0) result = -1;
			en->ds.sys.gd.wkday = result;
			break;
		    case BUTT_WKDAYSELECT:
			result = en->ds.sys.gd.wkdayselector;
			if (result > 5) result = 5;
			if (result < 0) result = 6;
			if (message::MultipleChoiceQuestion(self, 50, "In what month does this event happen? ", result, &result, WeekStrs, NULL) < 0) return;
			if (result >= 6 || result < 0) result = -1;
			if (result == 5) result = 99;
			en->ds.sys.gd.wkdayselector = result;
			break;
		    case BUTT_HR:
			if (message::MultipleChoiceQuestion(self, 50, "Is this an event that happens during every hour? ", 0, &result, BooleanChoices, NULL) < 0) return;
			if (result == 0) {
			    en->ds.sys.gd.hour = -1;
			} else {
			    if (message::AskForString(self, 50, "In what hour does it happen? ", NULL, buf, sizeof(buf)) < 0) return;
			    result = atoi(buf);
			    if (result < 0 || result >= 24) {
				message::DisplayString(self, 25, "Valid answers are 0 to 23.");
				return;
			    } else {
				en->ds.sys.gd.hour = result;
			    }
			}
			break;
		    case BUTT_MIN:
			if (message::MultipleChoiceQuestion(self, 50, "Is this an event that happens during every minute? ", 0, &result, BooleanChoices, NULL) < 0) return;
			if (result == 0) {
			    en->ds.sys.gd.min = -1;
			} else {
			    if (message::AskForString(self, 50, "In what minute does it happen? ", NULL, buf, sizeof(buf)) < 0) return;
			    result = atoi(buf);
			    if (result < 0 || result >= 60) {
				message::DisplayString(self, 25, "Valid answers are 0 to 59.");
				return;
			    } else {
				en->ds.sys.gd.min = result;
			    }
			}
			break;
		    default:
			message::DisplayString(self, 10, "Unrecognized button code.");
			break;
		}
		break;
	    case CALSYS_ECCLESIASTICAL:
		switch(buttcode) {
		    case BUTT_SYS:
			if (message::MultipleChoiceQuestion(self, 50, "What calendar system do you want to use for this event? ", 0, &result, SysChoices, NULL) < 0) return;
			if (result == 0) en->ds.calsys = CALSYS_GREGORIAN;
			if (result == 1) en->ds.calsys = CALSYS_HEBREW;
			if (result == 2) en->ds.calsys = CALSYS_ECCLESIASTICAL;
			break;
		    case BUTT_YEAR:
			if (message::MultipleChoiceQuestion(self, 50, "Is this an event that happens every year? ", 0, &result, BooleanChoices, NULL) < 0) return;
			if (result == 0) {
			    en->ds.sys.ed.year = -1;
			} else {
			    if (message::AskForString(self, 50, "What year does it happen in? ", NULL, buf, sizeof(buf)) < 0) return;
			    result = atoi(buf);
			    if (result < 1900 || result > 2100) {
				message::DisplayString(self, 25, "Valid answers are 1900 to 2100.");
				return;
			    } else {
				en->ds.sys.ed.year = result - 1900;
			    }
			}
			break;
		    case BUTT_MON:/* landmark */
			result = en->ds.sys.ed.landmark;
			if (result < 0) result = 4;
			if (message::MultipleChoiceQuestion(self, 50, "With what landmark is this event associated? ", result, &result, LandmarkStrs, NULL) < 0) return;
			if (result >= 4 || result < 0) result = -2;
			en->ds.sys.ed.landmark = result + 1;
			break;
		    case BUTT_DAY:/* offset from landmark */
			if (message::AskForString(self, 50, "How many days is this event offset from the landmark? ", NULL, buf, sizeof(buf)) < 0) return;
			result = atoi(buf);
			if (result <-100 || result > 250) {
			    message::DisplayString(self, 25, "Valid answers are between -100 and 250.");
			    return;
			} else {
			    en->ds.sys.ed.offset = result;
			}
			break;
		    case BUTT_HR:
			if (message::MultipleChoiceQuestion(self, 50, "Is this an event that happens during every hour? ", 0, &result, BooleanChoices, NULL) < 0) return;
			if (result == 0) {
			    en->ds.sys.ed.hour = -1;
			} else {
			    if (message::AskForString(self, 50, "In what hour does it happen? ", NULL, buf, sizeof(buf)) < 0) return;
			    result = atoi(buf);
			    if (result < 0 || result >= 24) {
				message::DisplayString(self, 25, "Valid answers are 0 to 23.");
				return;
			    } else {
				en->ds.sys.ed.hour = result;
			    }
			}
			break;
		    case BUTT_MIN:
			if (message::MultipleChoiceQuestion(self, 50, "Is this an event that happens during every minute? ", 0, &result, BooleanChoices, NULL) < 0) return;
			if (result == 0) {
			    en->ds.sys.ed.min = -1;
			} else {
			    if (message::AskForString(self, 50, "In what minute does it happen? ", NULL, buf, sizeof(buf)) < 0) return;
			    result = atoi(buf);
			    if (result < 0 || result >= 60) {
				message::DisplayString(self, 25, "Valid answers are 0 to 59.");
				return;
			    } else {
				en->ds.sys.ed.min = result;
			    }
			}
			break;
		    default:
			message::DisplayString(self, 10, "Unrecognized button code.");
			break;
		}
		break;
	    default:
		message::DisplayString(self, 50, "Sorry; chimp so far only knows about Gregorian, Hebrew, and Christian liturgical dates.");
		break;
	}
	(enode)->NotifyObservers( 0);
	(enode->mychimp)->SetModified();
    }
}



enodeview::enodeview()
{
    class butterview *buttviews[NUMBUTTS];
    class lpair *lps[NUMBUTTS-1];
    int i;
    
    this->mychimpview = NULL;
    for (i=0; i<NUMBUTTS; ++i) {
	this->butts[i] = new butter;
	buttviews[i] = new butterview;
	(buttviews[i])->SetDataObject( this->butts[i]);
	(this->butts[i])->SetRocks((char *) this, i);
	(this->butts[i])->SetHitFunction((butter_hitfptr) ButtHit);
    }
    ResetButterTexts(this);
    lps[0] = new lpair;
    (lps[0])->SetUp( buttviews[0], buttviews[1], 50, lpair_PERCENTAGE, lpair_VERTICAL, FALSE);
    lps[1] = new lpair;
    (lps[1])->SetUp( buttviews[2], buttviews[3], 50, lpair_PERCENTAGE, lpair_VERTICAL, FALSE);
    lps[2] = new lpair;
    (lps[2])->SetUp( buttviews[4], buttviews[5], 50, lpair_PERCENTAGE, lpair_VERTICAL, FALSE);
    lps[3] = new lpair;
    (lps[3])->SetUp( buttviews[6], buttviews[7], 50, lpair_PERCENTAGE, lpair_VERTICAL, FALSE);
    lps[4] = new lpair;
    (lps[4])->SetUp( lps[0], lps[1], 50, lpair_PERCENTAGE, lpair_HORIZONTAL, FALSE);
    lps[5] = new lpair;
    (lps[5])->SetUp( lps[2], lps[3], 50, lpair_PERCENTAGE, lpair_HORIZONTAL, FALSE);

    lps[6] = new lpair;
    (lps[6])->SetUp( buttviews[8], buttviews[9], 50, lpair_PERCENTAGE, lpair_VERTICAL, FALSE);
    lps[7] = new lpair;
    (lps[7])->SetUp( lps[4], lps[5], 50, lpair_PERCENTAGE, lpair_HORIZONTAL, FALSE);

    (this)->SetUp( lps[6], lps[7], 40, lpair_TOPFIXED, lpair_HORIZONTAL, FALSE);

    THROWONFAILURE((TRUE));
}

void enodeview::ObservedChanged(class observable  *changed, long  value)
{
    ResetButterTexts(this);
}

void ResetButterTexts(class enodeview  *self)
{
    char Buf[50];
    const char *mystr;
    int i, j;
    struct eventnode *en = NULL;
    class enode *enode;

    enode = (class enode *) (self)->GetDataObject();
    if (enode) en = (enode)->GetEvent();
    if (!enode || !en) {
	for (i=0; i<NUMBUTTS; ++i) {
	    (self->butts[i])->SetText( DefaultString[i]);
	}
	return;
    }
    if (en->ds.calsys != CALSYS_GREGORIAN) {
	for (i=0; i<NUMBUTTS; ++i) {
	    (self->butts[i])->SetText( DefaultString[i]);
	}
	switch (en->ds.calsys) {
	    case CALSYS_HEBREW:
		(self->butts[BUTT_SYS])->SetText( "Hebrew date");
		break;
	    case CALSYS_ECCLESIASTICAL:
		(self->butts[BUTT_SYS])->SetText( "Ecclesiastical date");
		break;
	    default:
		(self->butts[BUTT_SYS])->SetText( "Non-Gregorian date");
		return;	/* OTHER CASES FALL THROUGH */
	}
    } else {
	(self->butts[BUTT_SYS])->SetText( "Gregorian date");
    }

    switch (en->ds.calsys) {
	case CALSYS_GREGORIAN:
	    i = en->ds.sys.gd.year;
	    j = i + 1900;
	    break;
	case CALSYS_HEBREW:
	    i = en->ds.sys.hd.year;
	    j = i;
	    break;
	case CALSYS_ECCLESIASTICAL:
	    i = en->ds.sys.ed.year;
	    j = i + 1900;
	    break;
    }
    if (i < 0) {
	strcpy(Buf, "Any year");
    } else {
	sprintf(Buf, "Year: %d", j);
    }
    (self->butts[BUTT_YEAR])->SetText( Buf);

    switch (en->ds.calsys) {
	case CALSYS_GREGORIAN:
	    (self->butts[BUTT_MON])->SetText( (en->ds.sys.gd.month < 0) ? months[12] : months[en->ds.sys.gd.month]);
	    break;
	case CALSYS_HEBREW:
	    (self->butts[BUTT_MON])->SetText( (en->ds.sys.hd.month < 0) ? HebrewMonths[13] : HebrewMonths[en->ds.sys.hd.month]);
	    break;
	case CALSYS_ECCLESIASTICAL:
	    (self->butts[BUTT_MON])->SetText( (en->ds.sys.ed.landmark <= 0) ? "No Landmark" : LandmarkStrs[en->ds.sys.ed.landmark - 1]);
	    break;
    }

    if (en->ds.calsys != CALSYS_ECCLESIASTICAL) {
	switch (en->ds.calsys) {
	    case CALSYS_GREGORIAN:
		i = en->ds.sys.gd.day;
		break;
	    case CALSYS_HEBREW:
		i = en->ds.sys.hd.day;
		break;
	}
	if (i < 0) {
	    strcpy(Buf, "Any date");
	} else {
	    sprintf(Buf, "Date: %d", i);
	}
    } else {
	sprintf(Buf, "Day offset: %d", en->ds.sys.ed.offset);
    }
    (self->butts[BUTT_DAY])->SetText( Buf);

    if (en->ds.calsys == CALSYS_GREGORIAN) {
	(self->butts[BUTT_WKDAY])->SetText( (en->ds.sys.gd.wkday < 0) ? "Any day of week" : WkDays[en->ds.sys.gd.wkday]);
	if (en->ds.sys.gd.wkdayselector < 0) {
	    mystr = "Any week";
	} else if (en->ds.sys.gd.wkdayselector > 5) {
	    mystr = WeekStrs[5];
	} else {
	    mystr = WeekStrs[en->ds.sys.gd.wkdayselector];
	}
	(self->butts[BUTT_WKDAYSELECT])->SetText( mystr);
    } else {
	(self->butts[BUTT_WKDAY])->SetText( "Unused field");
	(self->butts[BUTT_WKDAYSELECT])->SetText( "Unused field");
    }

    if (en->ds.calsys == CALSYS_GREGORIAN || en->ds.calsys == CALSYS_ECCLESIASTICAL) {
	i = (en->ds.calsys == CALSYS_GREGORIAN ? en->ds.sys.gd.hour : en->ds.sys.ed.hour);
	if (i < 0) {
	    strcpy(Buf, "Any hour");
	} else {
	    sprintf(Buf, "Hour of day: %d", i);
	}
	(self->butts[BUTT_HR])->SetText( Buf);

	i = (en->ds.calsys == CALSYS_GREGORIAN ? en->ds.sys.gd.min : en->ds.sys.ed.min);
	if (i < 0) {
	    strcpy(Buf, "Any minute");
	} else {
	    sprintf(Buf, "minute of hour: %d", i);
	}
	(self->butts[BUTT_MIN])->SetText( Buf);
    } else {
	(self->butts[BUTT_HR])->SetText( "Unused field");
	(self->butts[BUTT_MIN])->SetText( "Unused field");
    }

    (self->butts[BUTT_ADD])->SetText( DefaultString[BUTT_ADD]);
    (self->butts[BUTT_DEL])->SetText( DefaultString[BUTT_DEL]);
}

void enodeview::SetChimpview(class chimpview  *cv)
{
    this->mychimpview = cv;
}
