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
#include <trouble.H>
#include <conob.H>
#include <observable.H>
#include <contimer.H>
#include <statsob.H>
#include <getstats.h>

#define MAXWORRIES 25
struct worry {
    class conob *co;
    int TooMuch;
};
struct worry Worries[MAXWORRIES];
static int NumWorries = 0, NumProblems = 0;
static class contimer *trouble_contimer = NULL;




ATKdefineRegistry(trouble, conob, trouble::InitializeClass);
static void CheckTrouble(long  dummy  /* should just be trouble_contimer, ignored here */);


boolean trouble::InitializeClass()
{
    class statsob *go;

    go = new statsob;
    (go)->SetStatPart( VM);
    Worries[0].co = (class conob *) go;
    Worries[0].TooMuch = 95;

    go = new statsob;
    (go)->SetStatPart( DISK1);
    Worries[1].co = (class conob *) go;
    Worries[1].TooMuch = 95;

    NumWorries = 2;

    trouble_contimer = new contimer;
    (trouble_contimer)->SetInterval( 1000); /* 1 second */
    (trouble_contimer)->SetDataCollectionProc((collection_fptr)CheckTrouble);
    return(TRUE);
}

static void CheckTrouble(long  dummy  /* should just be trouble_contimer, ignored here */)
{
    int i;

    NumProblems = 0;
    for (i=0; i<NumWorries; ++i) {
	if (Worries[i].co->numval >= Worries[i].TooMuch) {
	    ++NumProblems;
	}
    }
}

trouble::trouble()
{
	ATKinit;

    (trouble_contimer)->AddObserver( this);
    THROWONFAILURE((TRUE));
}

trouble::~trouble()
{
	ATKinit;

    (trouble_contimer)->RemoveObserver( this);
}

void trouble::ObservedChanged(class contimer  *ct, long  code)
{
    if (code == observable::OBJECTDESTROYED) {
	(this)->Destroy(); /* can't go on without a timer */
	return;
    }
    (this)->SetNumval( NumProblems);
    (this)->NotifyObservers( observable::OBJECTCHANGED);
}

void trouble::GetStringToDisplay(char  *buf, int  buflen, boolean  IsClick)
{
    int i, buffilled;
    boolean SawTrouble = FALSE;
    const char *dt = (this)->GetDisplayTemplate();

    if (!IsClick && dt != NULL && *dt != '!') {
	(this)->conob::GetStringToDisplay( buf, buflen, IsClick);
	return;
    }
    if (NumProblems <= 0) {
	strncpy(buf, "This workstation is just fine.", buflen);
	return;
    }
    if (NumProblems == 1) {
	strncpy(buf, "Trouble: ", buflen);
    } else {
	sprintf(buf, "%d troubles: ", NumProblems);
    }
    for (i=0; i<NumWorries; ++i) {
	if (Worries[i].co->numval >= Worries[i].TooMuch) {
	    buffilled = strlen(buf);
	    if (SawTrouble && (buflen-buffilled) > 2) {
		strcat(buf, "  ");
		buffilled += 2;
	    } else {
		SawTrouble = TRUE;
	    }
	    (Worries[i].co)->GetStringToDisplay(
		buf+buffilled, buflen-buffilled, IsClick);
	}
    }
}
