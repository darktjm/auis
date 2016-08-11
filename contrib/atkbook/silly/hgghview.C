static char *hgghview_rcsid = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/atkbook/silly/RCS/hgghview.C,v 1.2 1994/08/11 03:01:56 rr2b Stab74 $";

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
#include <hgghview.H>
#include <im.H>
#include <view.H>
#include <text.H>
#include <textview.H>
#include <lpair.H>
#include <butt.H>
#include <buttview.H>
#include <proctable.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <message.H>
#include <hggh.H>


     

static class keymap *hgghview_keymap = NULL;
static class menulist *hgghview_menulist = NULL;


ATKdefineRegistry(hgghview, lpair, hgghview::InitializeClass);
static void ChangeButton(class hgghview  *self, long  param);
static void ToggleProc(class hgghview  *self, long  param);
static void ToggleLpairViews(class hgghview  *self, long  ignored, class butt  *b, enum view_MouseAction  action);
static void ChangeFromShortList(class hgghview  *self, long  param);
static void ChangeFromLongList(class hgghview  *self, long  param);
static void HelpChoice(char  *partial, long  dummy , message_workfptr  helpTextFunction,  long  helpTextRock);
static enum message_CompletionCode CompleteChoice(char  *part , long  dummy , char  *buf, long  size);
static int matchct(char  *s1 , char  *s2);


boolean hgghview::InitializeClass()
{
    struct proctable_Entry *proc = NULL;

    hgghview_keymap = new keymap;
    hgghview_menulist = new menulist;

    proc = proctable::DefineProc("hgghview-toggle", (proctable_fptr)ToggleProc, &hgghview_ATKregistry_ , NULL, "toggles the two parts of the hgghview.");
    (hgghview_keymap)->BindToKey( "!", proc, 0);
    (hgghview_menulist)->AddToML( "Toggle Me!", proc, NULL, 0);

    proc = proctable::DefineProc("hgghview-change-button", (proctable_fptr)ChangeButton, &hgghview_ATKregistry_ , NULL, "Changes the text in the toggling button.");
    (hgghview_menulist)->AddToML("HGGH,Change Button Text", proc, NULL, 0);

    proc = proctable::DefineProc("hgghview-change-short-list", (proctable_fptr)ChangeFromShortList, &hgghview_ATKregistry_ , NULL, "Change toggling text from short list.");
    (hgghview_menulist)->AddToML("HGGH,Change From Short List", proc, NULL, 0);

    proc = proctable::DefineProc("hgghview-change-long-list", (proctable_fptr)ChangeFromLongList, &hgghview_ATKregistry_ , NULL, "Changes the toggling text using a long list.");
    (hgghview_menulist)->AddToML("HGGH,Change From Long List", proc, NULL, 0);

    return(TRUE);
}

hgghview::hgghview()
{
	ATKinit;

    class text *t1 = new text;
    class text *t2 = new text;
    class textview *tv1 = new textview;
    class textview *tv2 = new textview;

    this->bv = new buttview;
    this->b = new butt;
    this->lp = new lpair;
    (tv1)->SetDataObject( t1);
    (tv2)->SetDataObject( t2);
    (this->bv)->SetDataObject( this->b);
    (t1)->InsertCharacters( 0, "Hello, world!", 13);
    (t2)->InsertCharacters( 0, "Goodbye, world!", 15);
    (this->b)->SetText( "Toggle");
    (this->bv)->SetHitFunction( (buttview_hit_fptr)ToggleLpairViews);
    (this->bv)->SetRocks( this, NULL);
    (this->lp)->VSplit( tv1, tv2, 50, 1);
    (this)->VTFixed( this->bv, this->lp, 25, 1);
    (tv1)->WantInputFocus( tv1);
    this->ks = keystate::Create(this, hgghview_keymap);
    this->ml = (hgghview_menulist)->DuplicateML( this);
    THROWONFAILURE((TRUE));
}

hgghview::~hgghview()
{
	ATKinit;

    (this->lp)->Destroy();
    (this->b)->Destroy();
    (this->bv)->Destroy();
    delete this->ks;
    delete this->ml;
}

void
hgghview::PostKeyState(class keystate  *ks)
{
    this->ks->next = NULL;
    (this->ks)->AddBefore( ks);
    (this)->lpair::PostKeyState( this->ks);
}

void hgghview::PostMenus(class menulist  *ml)
{
    (this->ml)->ClearChain();
    if (ml) (this->ml)->ChainAfterML( ml, (long)ml);
    (this)->lpair::PostMenus( this->ml);
}

static void ChangeButton(class hgghview  *self, long  param)
{
    char Buf[250];

    if (message::AskForString(self, 50,
		"Enter new text for button: ",
		NULL, Buf, sizeof(Buf)) >= 0) {
	(self->b)->SetText( Buf);
	(self->b)->NotifyObservers(
		observable_OBJECTCHANGED);
	message::DisplayString(self, 10,
		"Changed the button text as requested.");
    }
}

static void ToggleProc(class hgghview  *self, long  param)
{
    class view *v1, *v2;

    v1 = (self->lp)->GetNth( 0);
    v2 = (self->lp)->GetNth( 1);
    (self->lp)->SetNth( 0, v2);
    (self->lp)->SetNth( 1, v1);
    (self->lp)->WantUpdate( self->lp);
    (v2)->WantInputFocus(v2);
}

static void ToggleLpairViews(class hgghview  *self, long  ignored, class butt  *b, enum view_MouseAction  action)
{
    if (action == view_LeftDown || action == view_RightDown) {
	ToggleProc(self, 0);
    }
}


static void ChangeFromShortList(class hgghview  *self, long  param)
{
    long result;
    static char *ShortList[] = {
	"hello",
	"goodbye",
	"kick me",
	"press here",
	"I am a button",
	"Impeach Nixon",
	NULL
    };

    if (message::MultipleChoiceQuestion(self, 50,
		"Choose new text for button: ",
		1, &result, ShortList, NULL) >= 0) {
	(self->b)->SetText( ShortList[result]);
	(self->b)->NotifyObservers(
		observable_OBJECTCHANGED);
	message::DisplayString(self, 10,
		"Changed the button text as requested.");
    }
}

static void ChangeFromLongList(class hgghview  *self, long  param)
{
    char Buf[500];

    if (message::AskForStringCompleted(self, 50, "Enter new text for button: ", NULL, Buf, sizeof(Buf), NULL, CompleteChoice, (message_helpfptr)HelpChoice, 0, message_MustMatch) >= 0) {
	(self->b)->SetText( Buf);
	(self->b)->NotifyObservers(observable_OBJECTCHANGED);
	message::DisplayString(self, 10, "Changed the button text as requested.");
    }
}

static char  *LongList[] = {
    "red",
    "yellow",
    "blue",
    "black",
    "brown",
    "beige",
    "mauve",
    "purple",
    "pink",
    "green",
    "silver",
    "orange",
    "gray",
    "white",
    "blue-green",
};
#define LONGLISTSIZE (sizeof(LongList) / sizeof(char *))

static void HelpChoice(char  *partial, long  dummy, message_workfptr  helpTextFunction, long  helpTextRock)
{
    int i, len;
    len = strlen(partial);
    for (i=0; i<LONGLISTSIZE;++i) {
	if (!strncmp(partial, LongList[i], len)) {
	    (*helpTextFunction)(helpTextRock,
		message_HelpListItem, LongList[i], NULL);
	}
    }
}
static enum message_CompletionCode CompleteChoice(char  *part , long  dummy , char  *buf, long  size)
{
    int matches = 0, i, plen, minmatch = -1, minmatchlen, newct;
    int DidMatch[LONGLISTSIZE];
    char partial[1000];

    strncpy(partial, part, sizeof(partial));
    /* previous line is in case part == buf */
    *buf = '\0';
    plen = strlen(partial);
    /* First, go through marking (in DidMatch) which ones
      match at all, keeping track of the shortest match
      in minmatch/minmatchlen. */
    for (i=0; i<LONGLISTSIZE; ++i) {
	DidMatch[i] = !strncmp(partial, LongList[i], plen);
	if (DidMatch[i]) {
	    ++matches;
	    if (minmatch < 0) {
		minmatch = i;
		minmatchlen = strlen(LongList[i]);
	    } else {
		newct = matchct(LongList[i],
				LongList[minmatch]);
		if (newct < minmatchlen) minmatchlen = newct;
		if (strlen(LongList[i]) <
		    strlen(LongList[minmatch])) {
		    minmatch = i;
		}
	    }
	}
    }
    if (matches < 1) {
	/* If there were no matches, we look for the
	    longest partial match and use it to set the
	    string before returning message_Invalid */
	int bestmatch = 0, dum;

	for (i=0; i<LONGLISTSIZE; ++i) {
	    dum = matchct(partial, LongList[i]);
	    if (dum > bestmatch) {
		bestmatch = dum;
		if (bestmatch>size) bestmatch = size;
		strncpy(buf, LongList[i], bestmatch);
		buf[bestmatch] = '\0';
	    }
	}
	return(message_Invalid);
    } else if (matches == 1) {
	/* If there is only one match, we copy it
	 and return message_Complete */
	strncpy(buf, LongList[minmatch], size);
	buf[size] = '\0';
	return(message_Complete);
    } else {
	/* If there are multiple matches, we want to
	 return the shortest matching string.  If it is
	 completely matched that string, we want to return
	 message_CompleteValid, otherwise message_Valid. */
	int lim = (minmatchlen > size) ? size : minmatchlen;
	strncpy(buf, LongList[minmatch], lim);
	buf[lim] = '\0';
	if (minmatchlen == strlen(LongList[minmatch])) {
	    return(message_CompleteValid);
	}
	return(message_Valid);
    }
}

static int matchct(char  *s1 , char  *s2)
{
    int ans = 0;
    while (*s1 && (*s1++ == *s2++)) ++ans;
    return(ans);
}


