/* $Author: wjh $ */

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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/neos/RCS/eos.C,v 1.5 1996/06/11 01:26:13 wjh Exp $";
#endif


 
/*
 * eos.c
 *
 * Basic view for EOS application.
*/

/* ************************************************************
 *  Copyright (C) 1989, 1990, 1991
 *  by the Massachusetts Institute of Technology
 *   For full copyright information see:'mit-copyright.h'     *
 ************************************************************ */

#include <mit-copyright.h>

#include <stdio.h>



#include <atom.H>
#include <bind.H>
#include <buffer.H>
#include <completion.H>
#include <cursor.H>
#include <eos.h>
#include <environ.H>
#include <dataobject.H>
#include <fontdesc.H>
#include <frame.H>
#include <im.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <message.H>
#include <style.H>
#include <text.H>
#include <textview.H>

#define EOSUSETRANSIENTS

/* sys/types.h in AIX PS2 defines "struct label",  causing a type name clash.
  Avoid this by temporarily redefining "label" to be something else. */
#define label gezornenplatz
#include <andrewos.h>	/* andrewos.h includes sys/types.h */
#include <eosfx.H> /* eosfx.ih includes sys/types.h */
#undef label

#include <eos.H>   /* eos.eh uses struct label */
#include <pickup.H>	/* pickup.ih uses struct label */
#include <papers.H>	/* papers.ih uses struct label */
#include <proctable.H>
#include <turnin.H>	/* turnin.ih uses struct label */
#include <view.H>










/* Global Variables */
static stringlist_res *Courses;
static class cursor *clockcursor;
class menulist *eosMenu;
class keymap *eosKeys;

static struct {
    class buffer *data;
    class im *im;
    class frame *fr;
} StyleGuideWindow;

struct helpRock {
    procedure HelpWork;
    class text *text;
    long rock;
    char *partial;
};


ATKdefineRegistry(eos, view, eos::InitializeClass);
#ifndef NORCSID
#endif
int ReadCourseList(register class eos  *self);
enum message_CompletionCode CourseComplete(char  *partial, class eos  *self, char  *buffer, int  bufferSize);
static void CourseHelp(char  *partial, class eos  *self, procedure  HelpWork, long  rock);
static void SetCourse(class eos  *self, char  *rock);
static boolean CheckCourse(class eos  *self);
class papers *StartWindow(register class eos  *self, char  *string, char  *title, int  wdfactor , int  htfactor, boolean  wantframe);
class turnin *StartTurninWindow(register class eos  *self);
class pickup *StartPickupWindow(register class eos  *self);
struct windowlist *LookUpWindow(struct windowlist  *wlist, char  *course);
void AddWindow(struct windowlist  **wlist, union windows  *window, char  *course);
void eos_PickUp(class eos  *self, class observable  *triggerer, long  rock);
void eos_TurnIn(class eos  *self, class observable  *triggerer, long  rock);
void eos_Handouts(class eos  *self, class observable  *triggerer, long  rock);
void eos_Exchange(class eos  *self, class observable  *triggerer, long  rock);
static boolean FindBuffer(class frame  *f,class buffer  *b);
static void OpenGuide(class eos  *self);
void eos_StyleGuide(class eos  *self, class observable  *triggerer, long  rock);
void eos_Help(class eos  *self, class observable  *triggerer, long  rock);
void eos_Grade(class eos  *self, class observable  *triggerer, long  rock);
void eos_Return(class eos  *self, class observable  *triggerer, long  rock);
void eos_DeleteWindow(class eos  *self);
void eos_SetDefault(class eos  *self);
void eos_SetDisplay(class eos  *self);


int
ReadCourseList(register class eos  *self)
{
    /* Read the list of available courses */
    /* Returns non-zero if failure */
    FX *fxp;
    char *errm;

    if (errm = eosfx::OpenCourse(self->course, &fxp))
	if (!fxp) {
	    /* We cannot do anything with this course! */
	    message::DisplayString(self->frame, 100, errm);
	    return -1;
	}
    /* Else - we have an fxp with which we can read (at least) */
    if (errm = eosfx::Directory(fxp, &Courses)) {
	message::DisplayString(self->frame, 100, errm);
	eosfx::Close(&fxp);
	return -1;
    }
    eosfx::Close(&fxp);
    return 0;
}

enum message_CompletionCode
CourseComplete(char  *partial, class eos  *self, char  *buffer, int  bufferSize)
{
    struct result result;
    stringlist node;
    char textBuffer[100];

    if (Courses == NULL)
	if (ReadCourseList(self))
	    return message_Invalid;

    textBuffer[0] = '\0';
    result.partialLen = strlen(partial);
    result.bestLen = 0;
    result.code = message_Invalid;
    result.best = textBuffer;
    result.max = sizeof(textBuffer)-1;
    result.partial = partial;
    
    for (node = Courses->stringlist_res_u.list; node != NULL; node = node->next)
    {
	completion::CompletionWork(node->s, &result); 
    }
    strncpy(buffer, result.best, bufferSize);
    if (result.bestLen == bufferSize) /* Ensure it's a real string */
	buffer[result.bestLen] = '\0';
    return result.code;
}

static void
CourseHelp(char  *partial, class eos  *self, procedure  HelpWork, long  rock)
{
    stringlist node;
    int l = strlen(partial);
 
    if (Courses == NULL)
	if (ReadCourseList(self))
	    return;

   (*HelpWork)(rock, message_HelpGenericItem, "List of possible courses:\n\n");
	
    for (node = Courses->stringlist_res_u.list; node != NULL; node = node->next)
    {
	if (partial)
	    if (strncmp(partial, node->s, l) != 0)
		continue;
	(*HelpWork)(rock, message_HelpListItem, node->s, NULL);
    }
}

static void SetCourse(class eos  *self, char  *rock)
/* Requests user to type the name of a course.
   if rock != NULL, it points to string containing desired coursename
 */
{
    char newco[33];
    char string[64];
    char oldco[33];
    char *errormsg;
    boolean success = FALSE;
    FX *fxp;

    strcpy(oldco, self->course); /* Keep copy of old course */

    if (rock)
	strcpy(self->course, rock);
    else {
	if (message::Asking(self->frame))
	    return;
	if (message::AskForStringCompleted(self->frame,self->dialogpri, "Name of course to use: ", self->course, newco, 32, NULL, CourseComplete, CourseHelp, self, message_MustMatch) == -1) {
	    message::DisplayString(self->frame, 0, "Cancelled.");
	    return;
	}
	strcpy(self->course, newco);
    }

    im::SetProcessCursor(clockcursor);
    message::DisplayString(self->frame, 0, "Checking course...");
    im::ForceUpdate();
    if (errormsg = eosfx::OpenCourse(self->course, &fxp)) {
	message::DisplayString(self->frame, 100, errormsg);
	if (fxp) {
	    /* Well, we can at least read from this course */
	    message::DisplayString(self->frame, 0, "Warning: errors were found when opening the course");
	    success = TRUE;
	} else {
	    /* This course is broken */
	    message::DisplayString(self->frame, 0, "Course not changed because of errors");
	    strcpy(self->course, oldco); /* Back to whatever it was before */
	}
    } else {
	message::DisplayString(self->frame, 0, "OK.");
	success = TRUE;
    }
    eosfx::Close(&fxp);
    im::SetProcessCursor(NULL);

    if (success) {
	/* Update the title (in case the title names the course) */
	if (self->gradingflag)
	    (self)->SetTitle( "GRADE: Editor");
	else
	    (self)->SetTitle( "EOS: Editor");
    }
    (self)->WantInputFocus( self);
}

void
eos::SetCourse(char  *course)
{
    /* Since we cannot keybind this, we have to have two procedures... */
    /* (You cannot put class_routines into bind_Description) */
    ::SetCourse(this, course);
}

static boolean CheckCourse(class eos  *self)
/* If `course` is set to be "" i.e. no course is specified,
   then SetCourse is called to request the course in a Dialog.
   Post:  Returns TRUE if, on exit, the course has any value other
          than "". Else returns FALSE.
 */
{
    if (!strcmp(self->course, "")) {
        message::DisplayString(self->frame, MESSAGE, "You must specify a course for this function.");
        ::SetCourse(self, NULL);
        if (!strcmp(self->course, "")) {
            message::DisplayString(self->frame, MESSAGE, "Operation cancelled because no course was given");
            return FALSE;
        }
        message::DisplayString(self->frame, MESSAGE, "");
    }

    return TRUE;
}

class papers *StartWindow(register class eos  *self, char  *string, char  *title, int  wdfactor , int  htfactor, boolean  wantframe)
/* Creates a new 'paper' object on the screen.
   string = title box of window.
   the window is text + scroll + title [+ buttons] + frame
   If wantframe is true then a frame is wrapped around the window
 */
{
    long left, top, width, height;
    class im *im;
    class frame *fr;
    class papers *window;

    /* We want the new window to be half the height of the current
     * so change the preferred dimensions temporarily
     */
    im::GetPreferedDimensions(&left, &top, &width, &height);
    im::SetPreferedDimensions(left, top, width/wdfactor, height/htfactor);
    im = im::Create(NULL);
    im::SetPreferedDimensions(left, top, width, height);
    if (wantframe) {
        fr = new class frame;
        if (!fr) {
            (im)->Destroy();
            message::DisplayString(self->frame, DIALOG, "Could not create a new window!");
            return NULL;
        }
    }

    /* Create the window */
    window = new class papers;
    (window)->SetCourse( self->course);
    (window)->SetTitle( string);
    (window)->SetParent( self);
    if (self->gradingflag) (window)->SetGrading();

    /* Place window into display */
    if (wantframe) {
        (fr)->SetView( window);
        (im)->SetView( fr);
    } else
        (im)->SetView( window);

    (im)->SetTitle( title);
    return window;
}

class turnin *StartTurninWindow(register class eos  *self)
{
    long left, top, width, height;
    class im *im;
    class frame *fr;
    class turnin *window;
    class im *other;
    int	enlarge;

    /* We want the new window to be half the height of the current
      * so change the preferred dimensions temporarily
      */
    im::GetPreferedDimensions(&left, &top, &width, &height);

    /* enlarge if menubar on, otherwise labels are invisible */	
    if (environ::GetProfileSwitch("Menubar", FALSE)) enlarge = 20;
    else enlarge = 0;

#ifdef EOSUSETRANSIENTS
    if ((other = (self)->GetIM()) == NULL) {
	printf ("Attempt to create dialogue box for nonexistant view!\n Creating top level window.");
	im::SetPreferedDimensions(left, top, width, width/2 + enlarge);
	im = im::Create(NULL);
    } else {
	im::SetPreferedDimensions(0, 0, width, width/2 + enlarge);
	im = im::CreateTransient(other);
    }
#else /*EOSUSETRANSIENTS */
    im::SetPreferedDimensions(left, top, width, width/2 + enlarge);
    im = im::Create(NULL);
#endif /* EOSUSETRANSIENTS */
    im::SetPreferedDimensions(left, top, width, height);
    fr = new class frame;
    if (!fr) {
	(im)->Destroy();
	message::DisplayString(self->frame, DIALOG, "Could not create a new window!");
	return NULL;
    }

    /* Create the window */
    window = new class turnin;
    (window)->SetCourse( self->course);
    (window)->SetTitle( "Turn In Assignment");
    (window)->SetParent( self);

    /* Place window into display */
    (fr)->SetView( window);
    (fr)->SetCommandEnable( FALSE);
    (im)->SetView( fr);

    (im)->SetTitle( "turn in");
    return window;
}

class pickup *StartPickupWindow(register class eos  *self)
{
    long left, top, width, height;
    class im *im;
    class frame *fr;
    class pickup *window;
    class im *other;

    /* We want the new window to be half the height of the current
      * so change the preferred dimensions temporarily
      */
    im::GetPreferedDimensions(&left, &top, &width, &height);
#ifdef EOSUSETRANSIENTS
    if ((other = (self)->GetIM()) == NULL) {
	printf ("Attempt to create dialogue box for nonexistant view!\n Creating top level window.");
	im::SetPreferedDimensions(left, top, width, width/2);
	im = im::Create(NULL);
    } else {
	im::SetPreferedDimensions(0, 0, width, width/2);
	im = im::CreateTransient(other);
    }
#else /*EOSUSETRANSIENTS */
    im::SetPreferedDimensions(left, top, width, width/2);
    im = im::Create(NULL);
#endif /* EOSUSETRANSIENTS */
    im::SetPreferedDimensions(left, top, width, height);
    fr = new class frame;
    if (!fr) {
	(im)->Destroy();
	message::DisplayString(self->frame, DIALOG, "Could not create a new window!");
	return NULL;
    }

    /* Create the window */
    window = new class pickup;
    (window)->SetCourse( self->course);
    (window)->SetTitle( "Pick Up");
    (window)->SetParent( self);

    /* Place window into display */
    (fr)->SetView( window);
    (fr)->SetCommandEnable( FALSE);
    (im)->SetView( fr);

    (im)->SetTitle( "pick up");
    return window;
}

struct windowlist *LookUpWindow(struct windowlist  *wlist, char  *course)
/* Searches for course in wlist. If found, returns the window
   associated with course, else returns NULL
 */
{
    if (wlist == NULL) return NULL;

    while (wlist != NULL && strcmp(wlist->course, course))
        wlist = wlist->next;
    return (wlist ? wlist : NULL);
}

void AddWindow(struct windowlist  **wlist, union windows  *window, char  *course)
/* Pre:  window is a window onto an application which has not yet been
             used for `course`
  Post: returns <window,course>::wlist
 */
{
    struct windowlist *node = (struct windowlist *) malloc(sizeof(struct windowlist));

    node->course = (char *) malloc(strlen(course) + 1);
    strcpy(node->course, course);
    node->window = window;
    node->next   = *wlist;
    *wlist = node;
}

void eos_PickUp(class eos  *self, class observable  *triggerer, long  rock)
/*
  Triggered from a button. It creates a new pickup object (if neccessary) and then returns
 */
{
    struct windowlist *win;
    class pickup *window;
    union windows *listentry = (union windows *) malloc(sizeof(union windows));

    StopWaitCursor();
    if (!CheckCourse(self)) return;
    StartWaitCursor();

    win = LookUpWindow(self->pickups, self->course);
    if (win == NULL) {
        /* This is the only window in this course - so create it and place into list */
        if ((window = StartPickupWindow(self)) == NULL) return;
	listentry->pickupwin = window;
	AddWindow(&self->pickups, listentry, self->course);
        (window)->DoPickUp();
    } else
        if (win->window->pickupwin->IDoNotExist) {
            /* There was a pickup at one point but it was 'cancelled'
             */
	    ((win->window->pickupwin)->GetIM())->ExposeWindow();
            (win->window->pickupwin)->DoPickUp();
        } else {
            message::DisplayString(self->frame, MESSAGE, "Re-trying pickup.");
            ((win->window->pickupwin)->GetIM())->ExposeWindow();
            (win->window->pickupwin)->DoPickUp();
        }

    StopWaitCursor();
    (self)->WantInputFocus( self);
}

void eos_TurnIn(class eos  *self, class observable  *triggerer, long  rock)
{
    struct windowlist *win;
    class turnin *window;
    union windows *listentry = (union windows *) malloc(sizeof(union windows));

    StopWaitCursor();
    if (!CheckCourse(self)) return;
    StartWaitCursor();

    win = LookUpWindow(self->turnins, self->course);
    if (win== NULL) {
        /* This is the only window in this course - so create it and place into list */
        if ((window = StartTurninWindow(self)) == NULL) return;
	listentry->turninwin = window;
        AddWindow(&self->turnins, listentry, self->course);
	(window)->GoForIt();
    } else
        if (win->window->turninwin->IDoNotExist) {
            /* There was a window at one point but it was 'cancelled'
             */
	    ((win->window->turninwin)->GetIM())->ExposeWindow();
	    (win->window->turninwin)->GoForIt();
        } else {
            message::DisplayString(self->frame, MESSAGE, "You already have a Turn In window for that course");
            ((win->window->turninwin)->GetIM())->ExposeWindow();
        }

    StopWaitCursor();
    (self)->WantInputFocus( self);
}

void eos_Handouts(class eos  *self, class observable  *triggerer, long  rock)
{
    struct windowlist *win;
    class papers *hw;
    union windows *listentry = (union windows *) malloc(sizeof(union windows));

    StopWaitCursor();
    if (!CheckCourse(self)) return;
    StartWaitCursor();

    win = LookUpWindow(self->handouts, self->course);
    if (win== NULL) {
        /* This is the only window in this course - so create it and place into list */
        /* This should really be multi-threaded, so that handouts do not
         * interfere with the eos window, but alas...
         */
	if ((hw = StartWindow(self, "EOS: Handouts", "handouts", 1, 2, TRUE)) == NULL) return;
        if (self->gradingflag)
            (hw)->SetDisplay( papers_SIDESUBDISP, papersHandouts);
        else
            (hw)->SetDisplay( papers_SIDE, papersHandouts);
	listentry->paperswin = hw;
        AddWindow(&self->handouts, listentry, self->course);
	if (environ::GetProfileSwitch("PromptForDefaultHandouts", FALSE)) (hw)->SetDefault();
        (hw)->ListHandouts();
    } else
        if (win->window->paperswin->IDoNotExist) {
            /* There was a handouts at one point but it was 'cancelled'
	      * So, we need to expose the window and redo it's listing
             */
	    ((win->window->paperswin)->GetIM())->ExposeWindow();
            (win->window->paperswin)->ListHandouts();
        } else {
            message::DisplayString(self->frame, MESSAGE, "Updating pre-existing Handouts window.");
            ((win->window->paperswin)->GetIM())->ExposeWindow();
            (win->window->paperswin)->ListHandouts();
        }

    StopWaitCursor();
    (self)->WantInputFocus( self);
}

void eos_Exchange(class eos  *self, class observable  *triggerer, long  rock)
{
    struct windowlist *win;
    class papers *window;
    union windows *listentry = (union windows *) malloc(sizeof(union windows));

    StopWaitCursor();
    if (!CheckCourse(self)) return;
    StartWaitCursor();

    win = LookUpWindow(self->exchanges, self->course);
    if (win == NULL) {
        /* This is the only window in this course - so create it and place into list */
	if ((window = StartWindow(self, "EOS: Exchange", "exchange", 1, 2, TRUE)) == NULL) return;
	listentry->paperswin = window;
        AddWindow(&self->exchanges, listentry,  self->course);
        if (self->gradingflag)
            (window)->SetDisplay( papers_SIDESUBDISP, papersExchange);
        else
	    (window)->SetDisplay( papers_SIDESUBMIT,    papersExchange);
	if (environ::GetProfileSwitch("PromptForDefaultExchange", FALSE)) (window)->SetDefault();
        (window)->ListExchange();
    } else
	if (win->window->paperswin->IDoNotExist) {
            /* There was an exchanges at one point but it was 'cancelled'
             */
	    ((win->window->paperswin)->GetIM())->ExposeWindow();
            (win->window->paperswin)->ListExchange();
        } else {
            message::DisplayString(self->frame, MESSAGE, "Updating pre-existing Exchange window.");
            ((win->window->paperswin)->GetIM())->ExposeWindow();
            (win->window->paperswin)->ListExchange();
        }

    StopWaitCursor();
    (self)->WantInputFocus( self);
}

/* The next function is from cmu's hyplink/linkv.c
 * it is used in enumerating the frames to find a buffer
 */
static boolean FindBuffer(class frame  *f,class buffer  *b)
{
/*
  Little, dippy routine passed to frame_Enumerate to find the
  frame which contains the buffer we want.
*/

  return((f)->GetBuffer()==b);
}

static void OpenGuide(class eos  *self)
/* 
  creates a new window (im[frame[buffer]]) on the file containing the style guide 
 */
{
    char string[128];

    if ((StyleGuideWindow.data = buffer::GetBufferOnFile(STYLEGUIDE, buffer_MustExist)) == NULL) {
	message::DisplayString(self->frame, DIALOG, "Could not find the Style Guide");
        strcpy(string, "The Style Guide is supposed to be at ");
        strcat(string, STYLEGUIDE);
        message::DisplayString(self->frame, MESSAGE, string);
        return;
    }

    StyleGuideWindow.fr = new class frame;
    (StyleGuideWindow.fr)->SetCommandEnable( TRUE);
    (StyleGuideWindow.fr)->SetBuffer( StyleGuideWindow.data, TRUE);
    StyleGuideWindow.im = im::Create(NULL);
    (StyleGuideWindow.im)->SetView( StyleGuideWindow.fr);
    (self)->WantInputFocus( (StyleGuideWindow.fr)->GetView());
    im::ForceUpdate();
}

void eos_StyleGuide(class eos  *self, class observable  *triggerer, long  rock)
/* 
  Button triggered procedure which checks to see if there already is a style guide;
  If there is, then it merely makes sure it is visible, else it calls OpenGuide
 */
{
    class frame *f;

    if (StyleGuideWindow.im == NULL || StyleGuideWindow.data == NULL) {
        /* First time style guide */
        OpenGuide(self);
    } else {
        if (!(StyleGuideWindow.data)->Visible())
            /* Yep - we got a buffer, but no window */
            OpenGuide(self);
        else {
            /* We have buffer AND window - so expose it */
            f = frame::Enumerate(FindBuffer, StyleGuideWindow.data);
            ((f)->GetIM())->ExposeWindow();
        }
    }
    return;
}

void eos_Help(class eos  *self, class observable  *triggerer, long  rock)
/* 
  Button triggered procedure. Fork()s an andrew help process, using the
   program name as the parameter to help
 */
{
    char *helpname = environ::AndrewDir("/bin/help");
    int fd;

    switch (osi_vfork()) {
	case 0:
	    for (fd = getdtablesize(); fd > 2; --fd) close(fd);
	    execl(helpname, helpname, im::GetProgramName(), 0);
	    printf ("Exec of %s failed.\n", helpname);
	    fflush (stdout);
	    _exit(-1);
	case -1:
	    message::DisplayString(self->frame, DIALOG, "Could not start the help!");
	    break;
	default:
	    message::DisplayString(self->frame, MESSAGE, "A Help window will appear shortly.");
	    break;
    }
    (self)->WantInputFocus( self);
    return;
}

/*-------------------------------------------
 * The instructor's interface rountines 
 *-------------------------------------------
 */

void eos_Grade(class eos  *self, class observable  *triggerer, long  rock)
/* Same as handouts, pickup, turnin and exchange */
{
    struct windowlist *win;
    class papers *gw;
    union windows *listentry = (union windows *) malloc(sizeof(union windows));
    enum papers_Toggles type;

    StopWaitCursor();
    if (!CheckCourse(self)) return;
    StartWaitCursor();

    type = papers_NEW;
    if (environ::GetProfileSwitch("StartWithOldPapers", FALSE)) type = papers_OLD;

    win = LookUpWindow(self->grades, self->course);
    if (win == NULL) {
        /* This is the only window in this c             ourse - so create it and place into list */
	if ((gw = StartWindow(self, "GRADE: Papers to Grade", "grading", 1, 1, TRUE)) == NULL) return;
	listentry->paperswin = gw;
        AddWindow(&self->grades, listentry, self->course);
        (gw)->SetDisplay( papers_ALTSIDE, papersGrading);
	if (environ::GetProfileSwitch("PromptForDefaultGrade", FALSE)) (gw)->SetDefault();
	(gw)->GradingListType( type);
    } else
	if (win->window->paperswin->IDoNotExist) {
            /* There was a window at one point but it was 'cancelled'
             */
	    ((win->window->paperswin)->GetIM())->ExposeWindow();
	    (win->window->paperswin)->GradingListType( type);
        } else {
            message::DisplayString(self->frame, MESSAGE, "If you really want an update,hide first, or use the menu command.");
            ((win->window->paperswin)->GetIM())->ExposeWindow();
        }

    StopWaitCursor();
    (self)->WantInputFocus( self);
    return;
}


void eos_Return(class eos  *self, class observable  *triggerer, long  rock)
{
    Paper paper;
    Paper *ppaper;
    long retrock;
    char answer[128], uname[33], strnum[6], *errormsg;   
    class buffer *buf;
    class dataobject *buf_data;
    int can_delete = 0;
    int version;
    FX *fxp;

    eosfx::PaperClear(&paper);

    buf = (self->frame)->GetBuffer ();
    buf_data = (buf)->GetData ();

    if (!(buf_data)->Get ( self->paperatom, &(self->paperatom), &retrock)) {
	if (message::AskForString(self->frame, MESSAGE, "Please give the username of the recipient of this paper:", "", uname, 32) == -1) {
	    message::DisplayString(self->frame, MESSAGE, "Cancelled.");
	    (self)->WantInputFocus( self);
	    return;
	}
	paper.author = uname;
	if (message::AskForString(self->frame, MESSAGE, "What is the assignment number?", "", strnum, 5) == -1) {
	    message::DisplayString(self->frame, MESSAGE, "Cancelled.");
	    (self)->WantInputFocus( self);
	    return;
	}
	paper.assignment = atoi(strnum);
	paper.filename = (self)->NameBuffer();
    } else {
	ppaper = (Paper *)retrock;
	eosfx::PaperCopy (ppaper, &paper);
	can_delete = 1;
    }

    paper.type = GRADED;
    message::DisplayString(self->frame, MESSAGE, "Please wait: Sending paper...");
    StartWaitCursor();
    im::ForceUpdate();
    strcpy(answer, (char *) mktemp(".eosXXXXXX"));
    (buf)->WriteToFile( answer, buffer_ReliableWrite | buffer_MakeBackup);
    if (errormsg = eosfx::SendFile(self->course, answer, &paper, TRUE)) {
	message::DisplayString(self->frame, MESSAGE, "Cancelled");
	message::DisplayString(self->frame, DIALOG, errormsg);
	(self)->WantInputFocus( self);
	StopWaitCursor();
	im::ForceUpdate();
	return;
    } else message::DisplayString(self, MESSAGE, "Done.");

    /* We want to inform the buffer that it no longer needs saving */
    version = (buf_data)->GetModified();
    (buf)->SetCkpClock( 0);
    (buf)->SetCkpVersion( version);
    (buf)->SetWriteVersion( version);
    unlink((buf)->GetCkpFilename());

    /* Should we blow away original? */
    if (can_delete && environ::GetProfileSwitch ("OverwriteOriginalPaper", TRUE)) {
	/* Recover paper identity for delete. */
	if (errormsg = eosfx::OpenCourse(self->course, &fxp))
	    message::DisplayString(self->frame, DIALOG, errormsg);
	else {
	    if (errormsg = eosfx::Delete(fxp, ppaper))
		message::DisplayString(self->frame, DIALOG, errormsg);
	    else eosfx::Close(&fxp);
	}
	/* clear the old paper away (dataobj_UnPut would be nice) */
	(buf_data->properties)->Unbind ( self->paperatom);
	eosfx::PaperFreeContents (ppaper);
	if (errormsg) message::DisplayString(self->frame, MESSAGE, "Paper has been returned.");
	else message::DisplayString(self->frame, MESSAGE, "Paper has been returned; original deleted.");
    } else message::DisplayString(self->frame, MESSAGE, "Paper has been returned.");
    (self)->WantInputFocus( self);
    StopWaitCursor();
    im::ForceUpdate();
    return;
}

static char delwindowWarning[] =
  "Deleting this window kills the program.";
static char *delwindowChoices[] = {
	"Continue Running",
	"Quit Application",
	NULL};

#define delwindow_CANCEL 0
#define delwindow_QUIT   1

void eos_DeleteWindow(class eos  *self)
    {
    long answer;
    if (message::MultipleChoiceQuestion(self->frame, 0,
					delwindowWarning, delwindow_CANCEL,
					&answer, delwindowChoices, NULL)
	 == -1)
	return;
    switch(answer){
	case delwindow_CANCEL:
	    return;

	case delwindow_QUIT :
	    frame_Exit(self->frame);
    }
}


void eos_SetDefault(class eos  *self)
    {
    class textview *tv;
    long dsize, usize, fontStyle;
    enum style_FontSize basis;
    char *font, fontname[100];
    class buffer *buffer;

    /* tickle the font size if it's different from the default */
    tv = (class textview *) (self->frame)->GetView();
    if (strcmp ((tv)->GetTypeName(), "textview") == 0) {
	class style *ds = (tv)->GetDefaultStyle();

	if ((font = environ::GetProfile("bodyfont")) == NULL || ! fontdesc::ExplodeFontName(font, fontname, sizeof(fontname), &fontStyle, &dsize)) {
	    dsize = 12;
	}

	usize = environ::GetProfileInt("DisplayFontsize", 20);
	(ds)->GetFontSize( &basis, &usize);
	if (basis == style_ConstantFontSize && dsize != usize)
	    (ds)->SetFontSize( style_ConstantFontSize, dsize);
	(self)->WantUpdate( self);
    }
}

void eos_SetDisplay(class eos  *self)
    {
    class textview *tv;
    long dsize, usize;
    enum style_FontSize basis;
    class buffer *buffer;

    /* tickle the font size if it's different from the default */
    tv = (class textview *) (self->frame)->GetView();
    if (strcmp ((tv)->GetTypeName(), "textview") == 0) {
	class style *ds = (tv)->GetDefaultStyle();
	usize = environ::GetProfileInt("DisplayFontsize", 20);
	(ds)->GetFontSize( &basis, &dsize);
	if (basis == style_ConstantFontSize && dsize != usize)
	    (ds)->SetFontSize( style_ConstantFontSize, usize);
	(self)->WantUpdate( self);
    }
}

void eos::SetFontDisplay()
    {
    eos_SetDisplay(this);
}

void eos::SetFontDefault()
    {
    eos_SetDefault(this);
}

static struct bind_Description eosBindings[]={
/*  { proc-name,     keybinding,rock, Menu name,        Menu rock, menuflag,
      function, documentation [, module-name]}
 */
    {"eos-change-course",    NULL, 0, "Change Course",         0, MENUS_general,   SetCourse,      "Set New Course Number"},
    {"eos-new-window",    "\0302", 0, NULL,         0, MENUS_general,   eos_NewWindow,      "Creates a new window"},
    {"eos-set-display-font",    NULL, 0, "View in BIG Font",         0, MENUS_general,   eos_SetDisplay,      "View buffer in Display Font"},
    {"eos-set-default-font",    NULL, 0, "View in Default Font",         0, MENUS_general,   eos_SetDefault,      "View buffer in Default Font"},
    {"eos-delete-window", "\030\004", 0, "Delete Window~89", 0, MENUS_delwindow, eos_DeleteWindow, "Same as to Quit"},
    NULL
};

boolean eos::InitializeClass()
{
    eosMenu = new class menulist;
    eosKeys = new class keymap;
    bind::BindList(eosBindings, eosKeys, eosMenu, &eos_ATKregistry_ );

    clockcursor = cursor::Create(NULL);
    (clockcursor)->SetStandard( Cursor_Wait);
    StyleGuideWindow.im = NULL;
    return TRUE;
}

void eos::Update()
{
/*    eos_EraseVisualRect(self); */
    (this)->FullUpdate( view_FullRedraw, (this)->GetLogicalTop(), (this)->GetLogicalLeft(), (this)->GetLogicalWidth(), (this)->GetLogicalHeight());
}

void eos::PostMenus(class menulist  *menulist)
{
    /* Received the children's menus. Want to add our own options,
           then pass menulist up to superclass */
    (this->menus)->ClearChain();
    (this->menus)->SetMask( this->menuflags);
    if (menulist) (this->menus)->ChainAfterML( menulist, menulist);

    (this)->view::PostMenus(this->menus); 
}


void eos::ReceiveInputFocus()
/* 
  We want to ensure that when we receive the focus, the focus is passed on
  into the editing window
 */

{
    (this)->WantInputFocus( (this->frame)->GetView());
}


void eos::SetBuffer(char  *filename, long  flags)
{
    this->editregion = buffer::GetBufferOnFile(filename, flags);
    (this->frame)->SetBuffer( this->editregion, TRUE);
}

char *eos::NameBuffer()
{
    return ((this->frame)->GetBuffer())->GetName();
}

