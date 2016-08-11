/* Copyright 1992 Carnegie Mellon University All rights reserved.
  $Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/launchapp/RCS/launchapp.C,v 1.16 1995/11/07 20:17:10 robr Stab74 $";
#endif
#include <andrewos.h>
ATK_IMPL("launchapp.H")
#include <stdio.h>
#include <sys/param.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/file.h>

#include <im.H>
#include <frame.H>
#include <environ.H>
#include <text.H>
#include <textview.H>
#include <lpair.H>
#include <pushbutton.H>
#include <pshbttnv.H>
#include <message.H>
#include <fontdesc.H>
#include <style.H>

#ifdef USE_CLOCK
/*
 #include <clock.H>
#include <clockview.H>
 */
#endif /* USE_CLOCK */

#include <launchapp.H>

#define MAJORVERSION (1)
#define MINORVERSION (0)

#define TOURFILENAME "/doc/AtkTour/Tour"

#define WELCOMESTR ("Welcome to the Andrew Application Launcher")
#define SPIELTEXT ("We hope you enjoy this opportunity to try Andrew.  Please mail any comments or questions to ")
#define MAILADDR ("info-andrew-request+@andrew.cmu.edu")

enum lpair_side {
    lpair_TopLeft, lpair_BottomRight
};

enum lpair_dir {
    lpair_SideBySide, lpair_AboveAndBelow
};

enum lpair_type {
    lpair_Percent, lpair_Fixed
};

   
 
ATKdefineRegistry(launchapp, application, NULL);
#ifndef NORCSID
#endif
#ifdef USE_CLOCK
#endif /* USE_CLOCK */
static class lpair *lpair_AddSplit(class lpair  *lp, class view  *v, enum lpair_side  side, enum lpair_dir  splitdir, enum lpair_type  splittype, 				    long  splitval, boolean  movable);
static void ZombieHandler(int  pid, long  rock, int  *status);
static void Do(char  *name, class frame  *frame, long  index, char  **args);
static char *Bin(char  *str);
static void DoHelp(class launchapp  *self, class pushbutton  *b, long  rock);
static void DoTour(class launchapp  *self, class pushbutton  *b, long  rock);
static void DoEz(class launchapp  *self, class pushbutton  *b, long  rock);
static void DoConsole(class launchapp  *self, class pushbutton  *b, long  rock);
static void DoMessages(class launchapp  *self, class pushbutton  *b, long  rock);
static void DoTypescript(class launchapp  *self, class pushbutton  *b, long  rock);
static void DoBush(class launchapp  *self, class pushbutton  *b, long  rock);
static void DoExit(class launchapp  *self, class pushbutton  *b, long  rock);
static void YaGotMe(int  signal, class launchapp  *self);


typedef void (*progfptr)(class launchapp *, class pushbutton *, long);
static struct buttoninfo {
    char *label, *text;
    progfptr fn;
    int pid;
} Buttons[] = {
    {"tour", "Tour of Andrew", DoTour, 0},
    {"ez", "Multimedia editor", DoEz, 0},
    {"help", "On-line documentation reader", DoHelp, 0},
    {"messages", "Mail, bboard, and news reader", DoMessages, 0},
    {"typescript", "Unix shell interface", DoTypescript, 0},
    {"console", "Machine performance monitor", DoConsole, 0},
    {"bush", "Filesystem browser", DoBush, 0},
    {"Exit", "Leave this demo", DoExit, 0},
    {NULL, NULL,  0, 0}
};




static class lpair *lpair_AddSplit(class lpair  *lp, class view  *v, enum lpair_side  side, enum lpair_dir  splitdir, enum lpair_type  splittype,
				    long  splitval, boolean  movable)
{
    class lpair *new_c = new lpair;

    if (!new_c)
	return ((class lpair *) 0);
    switch (splittype) {
      case lpair_Percent:
	switch (splitdir) {
	  case lpair_SideBySide:
	    if (side == lpair_TopLeft)
		(lp)->HSplit( v, new_c, 100 - splitval, movable);
	    else
		(lp)->HSplit( new_c, v, splitval, movable);
	    break;
	  case lpair_AboveAndBelow:
	    if (side == lpair_TopLeft)
		(lp)->VSplit( v, new_c, 100 - splitval, movable);
	    else
		(lp)->VSplit( new_c, v, splitval, movable);
	    break;
	}
	break;
      case lpair_Fixed:
	switch (splitdir) {
	  case lpair_SideBySide:
	    if (side == lpair_TopLeft)
		(lp)->HTFixed( v, new_c, splitval, movable);
	    else
		(lp)->HFixed( new_c, v, splitval, movable);
	    break;
	  case lpair_AboveAndBelow:
	    if (side == lpair_TopLeft)
		(lp)->VTFixed( v, new_c, splitval, movable);
	    else
		(lp)->VFixed( v, new_c, splitval, movable);
	    break;
	}
	break;
    }
    return (new_c);
}

static void ZombieHandler(int  pid, long  rock, WAIT_STATUS_TYPE  *status)
{
    Buttons[rock].pid = 0;
}

static void Do(char  *name, class frame  *frame, long  index, char  **args)
{
    int forkval;
    char msgbuf[100];

    if (Buttons[index].pid > 0) {
	sprintf(msgbuf, "%s is already running", name);
	message::DisplayString(frame, 100, msgbuf);
	return;
    }
    if ((forkval = osi_vfork()) == 0) {
	execvp(*args, args);
    }
    else {
	if (forkval > 0) {
	    sprintf(msgbuf, "Starting %s.  Window should appear soon.",
		    name);
	    message::DisplayString(frame, 0, msgbuf);
	    Buttons[index].pid = forkval;
	    im::AddZombieHandler(forkval, ZombieHandler, index);
	}
	else {
	    sprintf(msgbuf, "Error starting %s (%s)",
		    name, sys_errlist[errno]);
	    message::DisplayString(frame, 100, msgbuf);
	}
	(frame)->WantInputFocus( frame);
    }
}


static char *Bin(char  *str)
{
    static char buf[1024];
    char *bin=environ::AndrewDir("/bin/");
    if(strlen(bin)>sizeof(buf)-strlen(str)-1) return str;
    strcpy(buf, bin);
    strcat(buf, str);
    return buf;
}

static void DoHelp(class launchapp  *self, class pushbutton  *b, long  rock)
{
    static char *args[] = {"help", "-d", "-new", NULL};

    args[0]=Bin("help");
    
    Do("Help", self->framep, rock, args);
}

static void DoTour(class launchapp  *self, class pushbutton  *b, long  rock)
{
    char *args[4];
    args[0] = Bin("ez");
    args[1] = self->TourFile;
    args[2] = "-d";
    args[3] = NULL;
    Do("Tour", self->framep, rock, args);
}

static void DoEz(class launchapp  *self, class pushbutton  *b, long  rock)
{
    char *args[4];
    args[0] = Bin("ez");
    args[1] = environ::AndrewDir("/help/ez.help");
    args[2] = "-d";
    args[3] = NULL;
    Do("Ez", self->framep, rock, args);
}

static void DoConsole(class launchapp  *self, class pushbutton  *b, long  rock)
{
    static char *args[] = {"console", "-d", NULL};

    args[0]=Bin("console");
    
    Do("Console", self->framep, rock, args);
}

static void DoMessages(class launchapp  *self, class pushbutton  *b, long  rock)
{
    static char *args[] = {"messages", "-d", NULL};

    args[0]=Bin("messages");
    Do("Messages", self->framep, rock, args);
}

static void DoTypescript(class launchapp  *self, class pushbutton  *b, long  rock)
{
    static char *args[] = {"typescript", "-d", NULL};

    args[0]=Bin("typescript");
    Do("Typescript", self->framep, rock, args);
}

static void DoBush(class launchapp  *self, class pushbutton  *b, long  rock)
{
    static char *args[] = {"bush", "-d", NULL};

    args[0]=Bin("bush");
    Do("Bush", self->framep, rock, args);
}

static void DoExit(class launchapp  *self, class pushbutton  *b, long  rock)
{
    im::KeyboardExit();
}

launchapp::launchapp()
{
    char *tourFile = (char *) environ::AndrewDir(TOURFILENAME);

    this->numbuttons = ((sizeof (Buttons)) / (sizeof (struct buttoninfo))) - 1;
    if (!(this->buttons = (class pushbutton **)
	  malloc(this->numbuttons * (sizeof (class pushbutton *)))))
	THROWONFAILURE( (FALSE));
    if (!(this->buttontext = (class text **)
	  malloc(this->numbuttons * (sizeof (class text *)))))
	THROWONFAILURE( (FALSE));
    if(tourFile) {
	if((this->TourFile = (char *) malloc(strlen(tourFile) + 1)) != NULL)
	    strcpy(this->TourFile, tourFile);
	else {
	    printf("Can't allocate enough memory; exitting.");
	    THROWONFAILURE((FALSE));
	}
    }
    else {
	printf("Can't find tour file; exiting.");
	THROWONFAILURE((FALSE));
    }

    (this)->SetMajorVersion( MAJORVERSION);
    (this)->SetMinorVersion( MINORVERSION);
    (this)->SetName( "Launcher");

    THROWONFAILURE((TRUE));
}

launchapp::~launchapp()
{
    if(this->buttontext) {
	int i;
	for (i = 0; i < this->numbuttons; i++) {
	    (this->buttontext[i])->Destroy();
	    (this->buttons[i])->Destroy();
	}
	free(this->buttontext);
	free(this->buttons);
    }
    if(this->TourFile) free(this->TourFile);
}

/* Futile attempt at maintaining consistency in the face of adversity */
static void YaGotMe(int  signal, class launchapp  *self)
{
    struct buttoninfo *ptr;

    for (ptr = Buttons; ptr->label; ++ptr) {
	if (ptr->pid) {
	    kill(ptr->pid, SIGTERM);
	    ptr->pid = 0;
	}
    }
}

int launchapp::Run()
{
    int result;
    struct buttoninfo *ptr;

    if ((result = (this)->application::Run()) != 0) {
	fprintf(stderr, "Could not run application!\n");
    }
    for (ptr = Buttons; ptr->label; ++ptr) {
	if (ptr->pid) {
	    kill(ptr->pid, SIGTERM);
	    ptr->pid = 0;
	}
    }
    return (result);
}

boolean launchapp::Start()
{
    class lpair *tmp, *top2, *top, *buttonlp;
    class pushbuttonview *pbv;
    class textview *tv, *tvtmp;
    class im *Im;
    class view *applayer;
    class text *t;
    class style *st;
#ifdef USE_CLOCK
    class clock *cl;
    class clockview *clv;
    struct clock_options *clopts;
#endif /* USE_CLOCK */
    int i;

    environ::Put("ANDREWDIR", environ::AndrewDir(""));
    if (!(this)->application::Start()) {
	fprintf(stderr, "Could not start application!\n");
	return (FALSE);
    }

    this->framep = new frame;
    Im = im::Create(NULL);

    if (Im == NULL) {
	fprintf(stderr, "Could not find host!\n");
	return (FALSE);
    }

    t = new text;
    (t)->AlwaysInsertCharacters( (long) 0, WELCOMESTR,
				(sizeof (WELCOMESTR) - 1));
    (t)->SetReadOnly( TRUE);
    st = new style;
    (st)->SetFontSize( style_ConstantFontSize, (long) 12);
    (st)->SetJustification( style_Centered);
    (st)->AddNewFontFace( fontdesc_Bold);
    (st)->AddNewFontFace( fontdesc_Italic);
    (t)->SetGlobalStyle( st);

    tv = new textview;
    (tv)->SetDataObject( t);

    top2 = tmp = new lpair;
    tmp = lpair_AddSplit(tmp, tv, lpair_TopLeft, lpair_AboveAndBelow,
			 lpair_Percent, 20, FALSE);
    
    t = new text;
    (t)->AlwaysInsertCharacters( (long) 0, SPIELTEXT,
				(sizeof (SPIELTEXT) - 1));
    (t)->AlwaysInsertCharacters( (sizeof (SPIELTEXT) - 1),
				MAILADDR, (sizeof (MAILADDR) - 1));
    st = new style;
    (st)->AddNewFontFace( fontdesc_Bold);
    (t)->AddStyle( (sizeof (SPIELTEXT) - 1), (sizeof (MAILADDR) - 1), st);
    st = new style;
    (st)->SetJustification( style_LeftJustified);
    (t)->SetGlobalStyle( st);
    (t)->SetReadOnly( TRUE);

    tv = new textview;
    (tv)->SetDataObject( t);
    applayer = (tv)->GetApplicationLayer();
#ifdef USE_CLOCK
    cl = new clock;
    clopts = (cl)->GetOptions();
    clopts->border_shape = circle;
    clopts->border_width = 1;
    clopts->major_ticks = 4;
    clopts->minor_ticks = 12;
    clopts->seconds_length = 90;
    (cl)->SetOptions( &clopts);

    clv = new clockview;
    (clv)->SetDataObject( cl);
    (tmp)->HSplit( applayer, clv, 20, FALSE);
#else
    (tmp)->HSplit( applayer, NULL, 0, FALSE);
#endif /* USE_CLOCK */

    top = tmp = new lpair;
    tmp = lpair_AddSplit(tmp, top2, lpair_TopLeft, lpair_AboveAndBelow,
			 lpair_Percent, 40, FALSE);
    for (i = 0; i < this->numbuttons; ++i) {
	this->buttons[i] = new pushbutton;
	this->buttontext[i] = new text;
	pbv = new pushbuttonview;
	tvtmp = new textview;
	(pbv)->SetDataObject( this->buttons[i]);
	(tvtmp)->SetDataObject( this->buttontext[i]);
	(this->buttons[i])->SetText( Buttons[i].label);
	(this->buttons[i])->SetStyle( pushbutton_MOTIF);
	(this->buttontext[i])->AlwaysInsertCharacters( 0,
				    Buttons[i].text,
				    strlen(Buttons[i].text));
	(this->buttontext[i])->SetReadOnly( TRUE);
	(pbv)->AddRecipient( atom::Intern("buttonpushed"),
				    this, (observable_fptr) Buttons[i].fn, (long) i);
	buttonlp = new lpair;
	(buttonlp)->HSplit( pbv, tvtmp, 70, FALSE);
	if (i < (this->numbuttons - 1)) {
	    if (i < (this->numbuttons - 2)) {
		tmp = lpair_AddSplit(tmp, (class view *) buttonlp,
				     lpair_TopLeft,
				     lpair_AboveAndBelow, lpair_Percent,
				     100 / (this->numbuttons - i), FALSE);
	    }
	    else {
		(tmp)->VSplit( buttonlp, NULL, 50, FALSE);
	    }
	}
	else {
	    (tmp)->SetNth( 1, buttonlp);
	}
    }
    (this->framep)->SetView( top);
    (Im)->SetView( this->framep);
    (this->framep)->WantInputFocus( this->framep);

    im::SignalHandler(SIGTERM, (im_signalfptr)YaGotMe, (char *) this);
    im::SignalHandler(SIGPIPE, (im_signalfptr)YaGotMe, (char *) this);
    return (TRUE);
}
