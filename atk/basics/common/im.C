/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
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

#include <andrewos.h> /* sys/time.h sys/file.h sys/types.h */

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/basics/common/RCS/im.C,v 3.43 1996/09/03 19:03:55 robr Exp $";
#endif


 

/* Put in error messages for handling of keystrokes.
Figure out how to handle handlers and information requests.
Figure out some way to handle levels of user.  Macros should probably not be an novice level facility. */

ATK_IMPL("im.H")
#include <signal.h>
#include <ctype.h>
#ifndef MAXPATHLEN
#include <sys/param.h>
#endif
#include <setjmp.h>
#include <andyenv.h>

#ifdef ANDREW_MALLOC_ENV
#include <amalloc.h>
#endif

#define INTERACTION_MANAGER
#include <im.H>
#undef INTERACTION_MANAGER
#include <atomlist.H>
#include <rm.H>
#include <atom.H>
#include <graphic.H>
#include <dataobject.H>
#include <view.H>
#include <event.H>
#include <keymap.H>
#include <keystate.H>
#include <environ.H>
#include <proctable.H>
#include <menulist.H>
#include <updatelist.H>
#include <cursor.H>
#include <physical.h>
#include <message.H>
#include <init.H>
#include <bind.H>
#include <windowsystem.H>
#include <viewholderv.H>

#define rm rmgrn
#include <netinet/in.h>	/* for byte ordering in logs */
#undef rm


#ifdef RCH_ENV
#include <application.H>
#include <errno.h>
#if SY_AIXx
extern "C" {
#include "/usr/local/include/traceback.h"
};
static char *tracedir;
static time_t trace_starttime;
static char app_version_string[50];
#endif
#endif


#define HITPIXELS 1

#define RECORDING (doRecord && !dontRecord)

/* For use with PostDefaultHandler and WantHandler */
struct handler {
    char *name;
    ATK  *handler;
    struct handler *next;
};

static class windowsystem *currentWS = NULL; /* Used to distiguish window systems. */

static class atom * A_application;
static class atom * ProgramNameAtom;

/* Handlers for im_AddZombieHandler function. */
struct zombiehandler {
    struct zombiehandler *next;
    int pid;
    im_zombiefptr function;
    long functionData;
};

static boolean childDied;
static boolean cleanUpZombies;
static long eventCount;
static boolean doRecord;
static boolean dontRecord=FALSE;
static int recordingSuspensionLevel;
static boolean playingRecord;
static long thisCmd;		/* Used for deciding the last command that was executed */
static boolean keyboardExitFlag;
static long keyboardLevel;
static struct zombiehandler * allZombieHandlers; /* Handlers for im_AddZombieHandler function. */
static im_signalfptr sigProcs[NSIG];
static char *sigData[NSIG];
static boolean sigDelivered[NSIG];
static class keymap *globalKeymap;
static class menulist *imMenus;
static long destroycount;
static boolean allowCtrlUCmds;

  




/* Everyone uniformly references this data through a pointer, 
		declared below and statically allocated in im */

static struct im_GlobalDataType im_GlobalData;
static struct im_GlobalDataType * gData = &im_GlobalData;


ATKdefineRegistry(im, view, im::InitializeClass);
static void InitGlobalStructure();
static char *charToPrintable(long  c);
static void WriteLogEntry (class im  *self, unsigned char code, const char  *str);
static void WriteLogXY (class im  *self, unsigned char code, long  x , long  y);
static struct action * newAction();
static struct action * cloneAction(register struct action  *a);
static void stackAction(register struct action  **Q , register struct action  *a);
static void enqAction(register struct action  **Q , register struct action  *a);
static void freeQlist (struct action  *Q);
static void freeQelt(struct action  *Q);
static void pruneActions(class im  *im);
static struct action * keyAction(class im  *im, register long  k);
static struct action * mouseAction(class im  *im, enum view_MouseAction  act, long  x, long  y, long  newButtonState);
static struct action * menuAction(class im  *im, struct proctable_Entry  *procTableEntry, ATK   *object, long  rock);
static struct action * macroAction(class im  *im, struct action  *macro, struct action  *nextaction, long  remainingrepetitions);
static void userKey(register class im  *self, long  key);
static void userMouse(register class im  *self, enum view_MouseAction  act, long  x, long  y, long  newButtonState);
static void userMenu(register class im  *self, struct proctable_Entry  *procTableEntry, ATK   *object, long  rock);
static struct action *ConsumeMacroEvent(struct action  *a);
static struct action * GetNextInputEvent();
static struct action * PeekInputEvent();
static struct vfile *GetUnUsedVfile();
static struct vfile *GetCorrespondingVFile(FILE  *f);
#ifdef LWP
static int WakeUpIM(char  *dummy);
#endif /* LWP */
static SIGNAL_RETURN_TYPE DeathInTheFamily(int sig);
static void startKeyEchoing(class im  *self,long  time);
static void echoKey(class im  *self,long  key,int  pending);
static void resetKeyEcho(class im  *self);
static boolean stillexists(class im  *self);
static void HandleArgumentProcessing(class im  *self, long  key);
static void RecordProc(class im  *im, struct proctable_Entry  *procTableEntry, long  rock, ATK   *object, struct action  *keys);
static class im *HandleProc(class im  *self, struct proctable_Entry  *procTableEntry, ATK   *object, long  rock, struct action  *keys);
static boolean getMenuEntry(class menulist  *ml, char  *cname , char  *name, struct proctable_Entry  **pPE, ATK   **pObj, long  *pRock);
static const char * getMenuEntryName(class menulist  *ml, struct proctable_Entry  *procTableEntry, ATK   *object, long  rock);
static void GenericConfig(class im  *self, long  rock , long  customrock, class im  *parent, int  *x , int  *y, unsigned int  *w , unsigned int  *h);
static void  set_logical_wd(char  *dir ,	const char  *newdir);
static char *get_logical_wd(char  *dir);
static SIGNAL_RETURN_TYPE InternalSignal (int asigno);
static void ProcessInputQueue();
#if defined(hp9000s300) && HP_OS < 70
xim_sigAlrm();
#endif /* hp9000s300 */
static void InteractionEventWork(struct im_InteractionEvent  *interactionEvent);
static void FreeInteractionEvents(class im  *self);
static void RedrawWindow(class im  *self, long  key);
void im__PlayActions(class im  *self, struct action  *a);
static void StartKeyboardMacro(class im  *self, long  key);
static void EditRecording();
static void DumpActions(struct action  *a);
static void StopKeyboardMacro(class im  *self, long  key);
static void PlayKeyboardMacro(class im  *self, long  key);
static void SetArgProvided(class im  *self, boolean  value);
static void RecordCharacter(long  key);
void static PrintMallocStats(class im  *self, int  c);
void ResetMallocStats(class im  *self, long  c);
void PrintMallocTable(class im  *self, long  c);
static void StartProfiling(class im  *self,long  c);
static void StopProfiling(class im  *self, long  c);
static SIGNAL_RETURN_TYPE SigHandler(int sig);
static boolean isString(char  *arg);
static boolean VerifyBinding(class im  *self, char  *keys, int  keyslen, ATK   *obj, struct proctable_Entry  *pe, long  rock, boolean  rstring);
static char *GetKeyBinding(class im  *self, class keymap  *km, ATK   *obj, struct proctable_Entry  *pe, long  rock, boolean  rstring);


static void InitGlobalStructure() {

    initialProgramName = NULL;
    setDimensions = FALSE;
    geometrySpec = NULL;
    preferedTop = 0;
    preferedLeft = 0;
    preferedWidth = 500;
    preferedHeight = 500;

    imList = NULL;
    globalDoRedraw = FALSE;
    globalUpdateList = NULL;
    ProcessCursor = NULL;
    imPid = NULL;
    globalInit = NULL;
    anyDelivered = FALSE;
    NFILEHandlers = 0;  
    NCanOutHandlers = 0;  
    allZombieHandlers = NULL;
    readCutBuffer.string = NULL;
    readCutBuffer.pos = readCutBuffer.length = readCutBuffer.size = 0;
    writeCutBuffer.string = NULL;
    writeCutBuffer.pos = writeCutBuffer.length = writeCutBuffer.size = 0;
    defaultServerHost = NULL;
    enQuserKey = userKey;
    enQuserMouse = userMouse;
    enQuserMenu = userMenu;
}

/* Everyone gets the global data pointer through the following im class procedure */

struct im_GlobalDataType * im::GetGlobalData()
{
	ATKinit;

    return gData;
}



static char *charToPrintable(long  c)
{
    static char s[8];

    if (c==9)
	strcpy(s,"Tab");
    else if(c == 27)
        strcpy(s, "Esc");
    else if (c < 32)
        s[0] = '^', s[1] = c + '@', s[2] = '\0';
    else if(c==32)
	strcpy(s,"Spc");
    else if (c < 127)
        s[0] = c, s[1] = '\0';
    else if (c == 127)
        strcpy(s, "Del ");
    else {
        s[0] = '\\';
        s[1] = '0' + (c > 127);
        s[2] = '0' + ((c & 56) >> 3);
        s[3] = '0' + (c & 3);
        s[4] = '\0';
    }

    return s;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 *   log file - set environment variable  LOGDIRECTORY  to the name of a directory
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* Write an entry to the log file.  This is for use in studying the
   use of the editor.  Log entries are ASCII
	time	seconds since start of program
	code
	value
	newline

   The codes and their related values are:
	space	a single key that was typed
	@	host name
	:	title line (usually the file name)
	-	nothing - a menu selection was made
	! 	mouse down,  followed by coordinates
	^	mouse up, followed by coordinates
   To save space, mouse coordinates are divided by 16 and the remainders discarded
 */
#define log_KEY		' '
#define log_HOST  	'@'
#define log_TITLE  	':'
#define log_MENU  	'-'
#define log_MOUSEDOWN	'!'
#define log_MOUSEUP	'^'

static time_t LogStart;	/* time log started */

	void
WriteLogEntry (class im  *self, unsigned char code, const char  *str)
			{
	time_t now = time(0);
	if (now - LogStart > 600) {
		LogStart = now;
		fprintf(self->LogFile, "%s", ctime(&now));
	}
	if (self->LogFile == NULL) return;
	if(str == NULL) str="NULL host";
	else {
	    if (str[0]=='\0' || str[1] == '\0') 
		str = charToPrintable(*str);
	}
	fprintf(self->LogFile, "%d%c%s\n", now - LogStart, code, str);
}

	void
WriteLogXY (class im  *self, unsigned char code, long  x , long  y)
			{
	if (self->LogFile == NULL) return;
	fprintf(self->LogFile, "%d%c%d,%d\n", time(0) - LogStart, code, x>>4, y>>4);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 *	action package - input event queue routines and definitions
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* struct action definition has moved to im.ch */

	
/* NOTE - at present there is ONE input queue for the entire process 
	It is in global data as g_InQ  and accessible via macro InQ
*/
/* If there is  to be one queue per window, then enqAction and stackAction 
	need to have 'self' as a first argument */

static struct action *InQ = NULL;
static struct action *Record = NULL;
static struct action *FreeQ = NULL;
static struct action *PendingRelease = NULL;

/* newAction()
	allocate an empty action block 
*/
	struct action *
newAction()
{
	register struct action *a;
	if (FreeQ == NULL) 
		return (struct action *)malloc(sizeof(struct action));
	a = FreeQ;
	FreeQ = a->next;
	return a;
}

/* cloneAction(a)
	allocate an empty action block 
*/
	struct action *
cloneAction(register struct action  *a)
	{
	register struct action *new_c = newAction();
	if (a == NULL) return NULL;
	*new_c = *a;
	if(a->type==im_ProcEvent && a->v.proc.keys!=NULL) {
	    struct action *first=NULL, *last=NULL, *b;
	    a=a->v.proc.keys;
	    while(a) {
		b=cloneAction(a);
		if(first==NULL) first=b;
		if(last) last->next=b;
		last=b;
		a=a->next;
	    }
	    a->v.proc.keys=first;
	    a->v.proc.keypos=first;
	}
	new_c->next = NULL;
	return new_c;
}

/* stackAction(Q, a)
	put an action at the front of a queue
*/
	void
stackAction(register struct action  **Q , register struct action  *a)
	{
	if (a == NULL) return;	/* the malloc failed */
	a->next = *Q;
	*Q = a;
}

/* enqAction(Q, a)
	put an action at the rear of a queue
*/
	void
enqAction(register struct action  **Q , register struct action  *a)
	{
	if (a == NULL) return;	/* the malloc failed */
	a->next = NULL;
	for ( ; *Q != NULL; Q = &((*Q)->next)) {}
	*Q = a;
}

/* freeQlist(Q)
	return list of action elements to FreeQ
*/
	void
freeQlist (struct action  *Q)
	{
	struct action *a;
	if (Q == NULL) return;
	for (a = Q; a->next != NULL; a = a->next) {}
	a->next = FreeQ;
	FreeQ = Q;
}

/* freeQelt(Q)
	return action element to FreeQ
*/
void freeQelt(struct action  *Q)
{
    switch(Q->type) {
	case im_ProcEvent:
	    if(Q->v.proc.keys) {
		freeQlist(Q->v.proc.keys);
		Q->v.proc.keys=NULL;
	    }
	    break;
	case im_AnswerEvent:
	    if(Q->v.answer) {
		free(Q->v.answer);
		Q->v.answer=NULL;
	    }
	    break;
	default: ;
    }
    Q->next = FreeQ;
    FreeQ = Q;
}

/* pruneActions(im)
	remove all interactions for the given im from the queue InQ
	(no need to prune PendingRelease because its im field is not used)
*/
static	void
pruneActions(class im  *im)
	{
	register struct action *a, *p, *n;
	for (p = NULL, a = InQ; a != NULL; a = n) {
		n = a->next;
		if (a->im == im) {
			/* dequeue this element */
			if (p == NULL) InQ = n;
			else p->next = n;
			freeQelt(a);
		}
		else {
		    p = a;
		}
	}
}

/* keyAction(im, k)
	allocate an action block for a keystroke
*/
static	struct action *
keyAction(class im  *im, register long  k)
		{
	register struct action *a = newAction();
	if (a == NULL) return NULL;
	a->type = im_KeyboardEvent;
	a->im = im;
	a->v.key = k;
	return a;
}

/* mouseAction(im, act, x, y, newButtonState)
	allocate an action block for a mouse hit
*/
static	struct action *
mouseAction(class im  *im, enum view_MouseAction  act, long  x, long  y, long  newButtonState)
					{
	register struct action *a = newAction();
	if (a == NULL) return NULL;
	a->type = im_MouseEvent;
	a->im = im;
	a->v.mouse.action = act;
	a->v.mouse.x = x;
	a->v.mouse.y = y;
	a->v.mouse.newButtonState = newButtonState;
	return a;
}

/* menuAction(im, procTableEntry, object, rock)
	allocate an action block for a menu selection
*/
static	struct action *
menuAction(class im  *im, struct proctable_Entry  *procTableEntry, ATK   *object, long  rock)
	 		   	{
	register struct action *a = newAction();
	if (a == NULL) return NULL;
	a->type = im_MenuEvent;
	a->im = im;
	a->v.proc.procTableEntry = procTableEntry;
	a->v.proc.object = object;
	a->v.proc.rock = rock;
	a->v.proc.keys = NULL;
	a->v.proc.keypos = NULL;
	return a;
}
	

/* macroAction(im, macro, nextaction, remainingrepetitions)
	allocate an action block for a macro playback action
*/
static	struct action *
macroAction(class im  *im, struct action  *macro, struct action  *nextaction, long  remainingrepetitions)
				{
	register struct action *a = newAction();
	if (a == NULL) return NULL;
	a->type = im_MacroEvent;
	a->im = im;
	a->v.macro.macro = macro;
	a->v.macro.nextaction = nextaction;
	a->v.macro.remainingrepetitions = remainingrepetitions;
	return a;
}

	static void
userKey(register class im  *self, long  key)
		{
	struct action *a;
	if (self->LogFile != NULL) {
		char buf[2];
		buf[0] = ((key == IM_METAESC) ? '\033' : key);
		buf[1] = '\0';
		WriteLogEntry(self, log_KEY, buf);
	}

	a = keyAction(self, key);
	enqAction(&InQ, a);
}

	static void
userMouse(register class im  *self, enum view_MouseAction  act, long  x, long  y, long  newButtonState)
	{
	struct action *a;
	if(self==NULL) return;
	if (self->LogFile != NULL) switch (act) {
		case view_RightDown:
		case view_LeftDown:
			WriteLogXY(self, log_MOUSEDOWN, x, y);    break;
		case view_RightUp:
		case view_LeftUp:
			WriteLogXY(self, log_MOUSEUP, x, y);    break;
	}

	a = mouseAction(self, act, x, y, newButtonState);
	enqAction(&InQ, a);
}

	static void
userMenu(register class im  *self, struct proctable_Entry  *procTableEntry, ATK   *object, long  rock)
	 		   	{
	struct action *a;
	if (self->LogFile != NULL) 
		WriteLogEntry(self, log_MENU,
			 getMenuEntryName(self->menus, procTableEntry,
				object, rock));

	a = menuAction(self, procTableEntry, object, rock);
	enqAction(&InQ, a);
}

/* ConsumeMacroEvent consumes an event off the macro at the head of the input queue, returns the event consumed, or NULL if an interrupt was detected. */
static struct action *ConsumeMacroEvent(struct action  *a)
{
    if (a->next == NULL) {
	/* reduce repetitions or remove macro elt */
	if (--InQ->v.macro.remainingrepetitions <= 0) {
	    PendingRelease = InQ;
	    InQ = InQ->next;
	}
	else {
	    /* do another iteration.  First check for ^G */
	    if (im::CheckForInterrupt()) {
		/* (the macro has been
		    flushed from the Q 
		    by im_CheckForInterrupt()) */
		return NULL;
	    }
	    InQ->v.macro.nextaction = InQ->v.macro.macro;
	}
    }
    else
	InQ->v.macro.nextaction = a->next;
    return a;
}

/* GetNextInputEvent()
	get the next event off the input queue
	This routine knows about finding macros on the queue.
	Returns NULL if there are no events.
	Returns a struct action if there is an event.

	The event remains the property of the event queue
	and is only guaranteed to exist until the next call to this routine.

	The caller of this routine must use the event IMMEDIATELY.

*/
static	struct action *
GetNextInputEvent()
{
	if (PendingRelease != NULL) {
		if (PendingRelease->type == im_MacroEvent) {
			class im *im;
			playingRecord = FALSE;

			/* cursors have been disabled,  update them all */
			for (im = imList; im != NULL; im = im->next) {
				(im)->UpdateCursors();
			}
		}
		freeQelt(PendingRelease);
		PendingRelease = NULL;
	}
	if (InQ == NULL) return NULL;
	if (InQ->type == im_MacroEvent) {
		/* peel off one element of the macro */
		struct action *a;
		a = InQ->v.macro.nextaction;
		if(a->type == im_ProcEvent) {
		    if(a->v.proc.keypos!=NULL) {
			struct action *b=a->v.proc.keypos;
			a->v.proc.keypos=b->next;
			return b;
		    } else {
			 a->v.proc.keypos=a->v.proc.keys;
		    }
		}
		return ConsumeMacroEvent(a);
	} else {
		PendingRelease = InQ;
		InQ = InQ->next;
		return PendingRelease;
	}
}

/* PeekInputEvent()
	get a reference to the next event which will come off the input queue
	This routine knows about finding macros on the queue.
	Returns NULL if there are no events.
	Returns a struct action if there is an event.

	The event remains the property of the event queue
	and is only guaranteed to exist until 
		the second subsequent call to GetNextInputEvent()

	The caller of this routine must use the event IMMEDIATELY.

*/
static	struct action *
PeekInputEvent()
{
	if (InQ == NULL) return NULL;
	if (InQ->type == im_MacroEvent)
		return InQ->v.macro.nextaction;
	else 
		return InQ;
}

/* im__DoKeySequence(self, keys)
	queues the sequence of keys to be executed as if a macro
*/
	void
im::DoKeySequence(char *keys)
		{
	struct action *old;
	boolean olddontRecord=dontRecord;
	struct action *lastkey=NULL;
	struct action *KeyQ=NULL;
	
	old = InQ;
	/* Accumulate the keys on an action Queue */
	for ( ; *keys; keys++)
		enqAction(&KeyQ, lastkey=keyAction(this, *keys));

	if(lastkey) {
	    /* tack the keys action queue on the front of the input queue XXX the result of a previous call to peekinputevent will be invalid after a call to DoKeySequence */
	    lastkey->next=InQ;
	    InQ=KeyQ;

	    dontRecord=TRUE; /* make sure we don't record these keys in a macro */
	    while (old != InQ)
		im::Interact(FALSE); /* interact until all the keys have been done */
	    dontRecord=olddontRecord;
	}
}



/* Special stubs so that everyone can use the one shared version of the LWP and vfile package */

void im::IOMGRCancel(char  * localImPid /* actually of type PROCESS which is a point to a lwp_pcb struct*/)
{
	ATKinit;

#ifdef LWP
    IOMGR_Cancel(localImPid);
#endif /* LWP */
}

void im::IOMGRSoftSig(procedure  aproc,char  * arock)
{
	ATKinit;

#ifdef LWP
    IOMGR_SoftSig(aproc,arock);
#endif /* LWP */
}

boolean im::IOMGRSelect(long  maxnum,long  *rmask,long  *wmask,long  *emask,struct timeval  * timeOut)
{
	ATKinit;

    long ret = 0;
#ifdef LWP
    ret = IOMGR_Select(maxnum,rmask,wmask,emask,timeOut);
#endif /* LWP */
    return ret;
}

long im::LWPCurrentProcess(char  * curProcessID /* really type *PROCESS */ )
{
	ATKinit;

    long retValue = 0;
#ifdef LWP
    retValue = LWP_CurrentProcess(curProcessID);
#endif /* LWP */
    return retValue;
}

#define NUMBEROFVFILES 4
#define INITIALSIZE 200

struct vfile  {
    FILE *file;
    char name[30];
    long size;
    char mode;
    char used;
};

static struct vfile vFiles[NUMBEROFVFILES];
static struct vfile *lastVFile;

static struct vfile *GetUnUsedVfile()
{
    register int i;
    for (i = 0; i < NUMBEROFVFILES; i++)  {
	if (! vFiles[i].used)  {
	    if (vFiles[i].file == NULL)  {
		register FILE *f;

		f = tmpfile();
		if(f==NULL) continue;

		vFiles[i].file=f;
		vFiles[i].size = 0;
		vFiles[i].mode = ' ';
	    }
	    vFiles[i].used = 1;
	    lastVFile = &(vFiles[i]);
	    return lastVFile;
	}
    }
    return NULL;
}

static struct vfile *GetCorrespondingVFile(FILE  *f)
{
    register int i;

    for (i = 0; i < NUMBEROFVFILES; i++)  {
	if (vFiles[i].used && vFiles[i].file == f)
	    return (&(vFiles[i]));
    }

    return NULL;
}


FILE *im::vfileopen(char  *mode, struct expandstring  *buffer)
{
	ATKinit;

    FILE *f;
    struct vfile *vf;

    vf = GetUnUsedVfile();
    if (vf == NULL)
        return NULL;
    f = vf->file;

    if (*mode == 'r')  {
	rewind(f);
	fwrite(buffer->string, sizeof (char), buffer->length, f);
	rewind(f);
	if (buffer->length < vf->size)  {
	    ftruncate(fileno(f), buffer->length);
	}
	vf->size = buffer->length;
    }
    else if (*mode == 'w')  {
	rewind(f);
    }
    else  {
	vf->used = 0;
        return NULL;
    }

    vf->mode = *mode;

    return f;
}

void im::vfileclose(FILE  *f, struct expandstring  *buffer)
{
	ATKinit;

    struct vfile *vf;
    
    vf = (f == lastVFile->file) ? lastVFile : GetCorrespondingVFile(f);

    if (vf == NULL)
        return;

    if (vf->mode == 'r')  {
	rewind(f);
    }
    else {
	if (buffer != NULL)  {
	    buffer->length = ftell(f);
	    if (buffer->size < buffer->length + 1)  {
		if (buffer->string == NULL)  {
		    buffer->size = (INITIALSIZE > buffer->length + 1) ? INITIALSIZE : (buffer->length + 1);
		    buffer->string = (char *) malloc(buffer->size);
		}
		else {
		    buffer->size = buffer->length + 1;
		    buffer->string = (char *) realloc(buffer->string, buffer->size);
		}
	    }
	    if (buffer->length > vf->size)
	        vf->size = buffer->length;
	    rewind(f);
	    fread(buffer->string, sizeof (char), buffer->length, f);
	    buffer->string[buffer->length] = '\0';
	}
    }
    vf->used = 0;
}


void im::vfilecleanup()
{
	ATKinit;

    register int i;

    for (i = 0; i < NUMBEROFVFILES; i++)  {
	if (vFiles[i].file != NULL)  {
	    fclose(vFiles[i].file);
	}
    }
}


void im::plumber(FILE  * reportFile )
{
	ATKinit;

#if defined(ANDREW_MALLOC_ENV) && defined(DEBUG_MALLOC_ENV)
    ::plumber(reportFile);
#endif /* #if defined(ANDREW_MALLOC_ENV) && defined(DEBUG_MALLOC_ENV) */
}

void im::SetDefaultServerHost(const char  *value  )
{
	ATKinit;

    char *buffer;

    buffer = (char *)malloc(strlen(value) + 1);
    strcpy(buffer, value);
    if (defaultServerHost != NULL)  {
	free(defaultServerHost);
    }
    defaultServerHost = buffer;
}

#ifdef LWP
/* called on iomgr lwp's stack at a safe time*/
static int WakeUpIM(char  *dummy)
    {
    if (imPid != NULL)
        IOMGR_Cancel(imPid);
}
#endif /* LWP */

static SIGNAL_RETURN_TYPE DeathInTheFamily(int sig) {
    childDied = TRUE;
}

static void startKeyEchoing(class im  *self,long  time)
{
    if(self->keyEchoState==im_KeyEchoPending){
	self->keyEchoState=im_KeyEchoDisplayed;
	message::DisplayString(self,0,self->keyEcho);
    }

    self->keyEchoEvent=NULL;
}

#define KEYECHODELAY 750 /* msec */

static void echoKey(class im  *self,long  key,int  pending)
{
    if(self->keyEchoState==im_KeyEchoOff){
	if(self->argState.argProvided) {
	    sprintf(self->keyEcho,"%d ",self->argState.argument);
	}
	else if (self->argState.argPending) {
	    sprintf(self->keyEcho,"%d ",self->argState.tmpArgument);
	}
	else {
	    self->keyEcho[0]='\0';
	}
	self->keyEchoState=im_KeyEchoPending;
	self->keyEchoEvent=NULL;
    }

    if(self->keyEchoState!=im_KeyEchoDisplayed){
	if(self->keyEchoEvent!=NULL){
	    (self->keyEchoEvent)->Cancel();
	    self->keyEchoEvent=NULL;
	}

	if(pending)
	    self->keyEchoEvent=
	      event::Enqueue(event::Now()+
				event_MSECtoTU(KEYECHODELAY),
			     (event_fptr) startKeyEchoing,
			    (char *) self);
    }

    strcat(self->keyEcho,charToPrintable(key));
    if(pending)
	strcat(self->keyEcho,"-");

    if(self->keyEchoState==im_KeyEchoDisplayed)
	message::DisplayString(self,0,self->keyEcho);
}	

static void resetKeyEcho(class im  *self)
{
    if(self->keyEchoState!=im_KeyEchoOff){
	if(self->keyEchoEvent!=NULL){
	    (self->keyEchoEvent)->Cancel();
	    self->keyEchoEvent=NULL;
	}
	self->keyEchoState=im_KeyEchoOff;
    }
}
static boolean stillexists(class im  *self)
	{
    register class im *im = imList;
    while (im != NULL)  {
	if(im == self) return TRUE;
	im = im->next;
    }
    return FALSE;
}

static char argbuf[30];

static void HandleArgumentProcessing(class im  *self, long  key)
{
    long newArg;

    argbuf[0]='\0';
    if  (key == 025 && self->argState.cmdpos == 0) {
	self->argState.argDigit = FALSE;
	newArg = 4;
	if (self->argState.argPending) {
	    newArg = self->argState.tmpArgument * 4;
	    if (newArg > 65536)
		newArg = 65536;
	}
	self->argState.tmpArgument = newArg;
	self->argState.argPending = TRUE;
	self->argState.processCmd = allowCtrlUCmds;

	sprintf(argbuf, "Arg: %d", newArg);
	/* message_DisplayString(self, 0, buf); */
    }
    else if (self->argState.argPending && self->argState.cmdpos == 0 && key >= '0' && key <= '9') {
	if(RECORDING) enqAction(&Record, keyAction(self, key));
	if (! self->argState.argDigit) {
	    self->argState.argDigit = TRUE;
	    self->argState.tmpArgument = 0;
	}
	newArg = self->argState.tmpArgument * 10 + (key - '0');
	if (newArg > 65536) {
	    newArg = 65536;
	}
	self->argState.tmpArgument = newArg;

	sprintf(argbuf, "Arg: %d", newArg);
	message::DisplayString(self, 0, argbuf);
    }
    else if (self->argState.argPending) {
	if (self->argState.processCmd) {
	    self->argState.argcmd[self->argState.cmdpos++] = key;
	}
	else {
	    (self)->ProvideArg( self->argState.tmpArgument);
	    SetArgProvided(self, TRUE);
	}
    }
}


static void RecordProc(class im  *im, struct proctable_Entry  *procTableEntry, long  rock, ATK   *object, struct action  *keys)
{
    register struct action *a = newAction();
    if (a == NULL) return;
    if(keys!=NULL) a->type = im_ProcEvent;
    else a->type = im_MenuEvent;
    a->im = im;
    a->v.proc.procTableEntry = procTableEntry;
    a->v.proc.object = object;
    a->v.proc.rock = rock;
    a->v.proc.keys = keys;
    a->v.proc.keypos = keys;
    enqAction(&Record, a);
}
    
static struct action *lastkeys=NULL;

/* this will be filled in (in InitializeClass) with the proctable_Entry for im-stop-keyboard-macro */
static struct proctable_Entry *stopmacroproc=NULL;

static class im *HandleProc(class im  *self, struct proctable_Entry  *procTableEntry, ATK   *object, long  rock, struct action  *keys)
{
    register long dest = destroycount;
    char errmsg[256];

    self->argState.argNext = FALSE;
    thisCmd = 0;
    
    if(RECORDING && procTableEntry!=stopmacroproc)  {
	RecordProc(self, procTableEntry, rock, object, keys);
	im::SuspendRecording();    
	if(keys) lastkeys=NULL;
    }
    im::ForceUpdate();
    switch ((self->keystate)->DoProc( procTableEntry, rock, object))  {
	case keystate_NoProc:
	    sprintf(errmsg, "Could not load procedure: %s", procTableEntry->name);
	    message::DisplayString(self, 0, errmsg);
	    break;
	case keystate_TypeMismatch:
	    sprintf(errmsg, "Bad command: %s", procTableEntry->name);
	    message::DisplayString(self, 0, errmsg);
	    break;
    }

    if(RECORDING && procTableEntry!=stopmacroproc)  {
	im::ResumeRecording();
    }
    
    if(dest == destroycount || stillexists(self))/* make sure self still exists */
    {
	SetArgProvided(self, self->argState.argNext);
	self->lastCommand = thisCmd;
	return self;
    }
    else {
	/* if we're here self has been destroyed so we can't do this... -rr2b 
	      SetArgProvided(self, FALSE); */
	return NULL;
    }
}

	class im * 
im::DoKey(long  key)
		{
	struct proctable_Entry *procTableEntry;
	ATK  *object;
	long rock = 0L;
	long dest = destroycount;
	class im *self=this;
	if (this->keystate == NULL) return this;

	HandleArgumentProcessing(this, key);

	if ((! this->argState.argPending) || this->argState.processCmd) {
	    switch ((this->keystate)->ApplyKey( key, &procTableEntry, &rock, &object))  {
		case keystate_ProcFound:
		    if(RECORDING) enqAction(&lastkeys, keyAction(this, key));
		   if(this->keyEchoState==im_KeyEchoDisplayed){
			echoKey(this, key, FALSE);
			im::ForceUpdate();
			message::DisplayString(this, 0, "");
		    }
		    resetKeyEcho(this);
		    self=HandleProc(this, procTableEntry, object, rock, lastkeys);
		    if(self) self->argState.argPending = FALSE;
		    break;
		case keystate_NoBinding:
		    if(argbuf[0]) message::DisplayString(this, 0, argbuf);
		    if(RECORDING) {
			enqAction(&Record, keyAction(this, key));
			freeQlist(lastkeys);
			lastkeys=NULL;
		    }
		    if (key == 'G' - 64) {
			message::DisplayString(this,0,"Cancelled.");
			SetArgProvided(this, FALSE);
		    }
		    else {
			if (this->argState.argPending) {
			    if (this->argState.cmdpos != 0) {
				(this)->ProvideArg( this->argState.tmpArgument);
				this->argState.argcmd[this->argState.cmdpos] = '\0';

				SetArgProvided(this, TRUE);
				this->DoKeySequence(this->argState.argcmd);
			    }
			    else {
				this->argState.processCmd = FALSE;
			    }
			}
			else {
			    echoKey(this,key,FALSE);
			    strcat(this->keyEcho, ": Unknown command");
			    message::DisplayString(this,0,this->keyEcho);

			    SetArgProvided(this, FALSE);
			    this->lastCommand = 0;
			}
		    }
		    resetKeyEcho(this);
		    break;
		case keystate_Pending:
		    if(RECORDING) enqAction(&lastkeys, keyAction(this, key));
		    echoKey(this, key, TRUE);
		    break;
	    }
	}
	return self;
}

/* used for DoMenu */
	static boolean
getMenuEntry(class menulist  *ml, char  *cname , char  *name, struct proctable_Entry  **pPE, ATK   **pObj, long  *pRock)
		 		   	{
	class menulist *tml;
	struct proctable_Entry *tpe;
	long trock;
	char *entryname;

	menulist_RewindBeforeMC(ml);
	while ((tml=(ml)->NextBeforeMC()) != NULL)
		if (getMenuEntry(tml, cname, name, pPE, pObj, pRock)) return TRUE;

	menulist_RewindML(ml);
	while ((ml)->NextME( &entryname, &trock, &tpe)) {
		char cbuf[256], ibuf[256];
		char *itemnm, *nmend;
		if(entryname==NULL || strlen(entryname)>sizeof(cbuf)-1) continue;
		while(isspace(*entryname)) entryname++;
		itemnm = strchr(entryname, ',');
		if (itemnm == NULL) {
		    cbuf[0]='\0';
		    strcpy(ibuf, entryname);
		} else {
		    strncpy(cbuf, entryname, itemnm-entryname);
		    cbuf[itemnm-entryname]='\0';
		    itemnm++;
		    while(isspace(*itemnm)) itemnm++;
		    strcpy(ibuf, itemnm);
		}

		nmend=strchr(cbuf, '~');
		if(nmend) *nmend='\0';
		for(nmend=cbuf;*nmend;nmend++) if(isupper(*nmend)) *nmend=tolower(*nmend);
		nmend=strchr(ibuf, '~');
		if(nmend) *nmend='\0';
		for(nmend=ibuf;*nmend;nmend++) if(isupper(*nmend)) *nmend=tolower(*nmend);

		if ((cname[0]=='\0' || strcmp(cbuf, cname)==0) && strcmp(ibuf, name)==0) {
			*pPE = tpe;
			*pObj = ml->object;
			*pRock = trock;
			return TRUE;
		}
	}

	menulist_RewindAfterMC(ml);
	while ((tml=(ml)->NextAfterMC()) != NULL)
		if (getMenuEntry(tml, cname, name, pPE, pObj, pRock))
			return TRUE;

	return FALSE;
}


/* used for logging menu hits */
	static const char *
getMenuEntryName(class menulist  *ml, struct proctable_Entry  *procTableEntry, ATK   *object, long  rock)
	 		   	{
	class menulist *tml;
	const char *entryname;
	struct proctable_Entry *tpe;
	long trock;

	entryname = NULL;

	menulist_RewindBeforeMC(ml);
	while (entryname == NULL  
			&&  (tml=(ml)->NextBeforeMC()) != NULL)
		entryname = getMenuEntryName(tml, procTableEntry, object, rock);

	menulist_RewindML(ml);
	if (ml->object == object)
		while (entryname == NULL 
			&& (ml)->NextME( (char **)&entryname, 
				&trock, &tpe)) 
			if (trock != rock  ||  tpe != procTableEntry)
				entryname = NULL;

	menulist_RewindAfterMC(ml);
	while (entryname == NULL  
			&&  (tml=(ml)->NextAfterMC()) != NULL)
		entryname = getMenuEntryName(tml, procTableEntry, object, rock);

	return ((entryname == NULL) ? "" : entryname);
}

    
class im * 
im::HandleMenu(struct proctable_Entry  *procTableEntry, ATK   *object, long  rock)
                {
    static struct ATKregistryEntry  *viewinfo=NULL;
    this->argState.argProvided = FALSE;
    
    if(viewinfo==NULL) viewinfo=ATK::LoadClass("view");

    /* trying to make domenu useful... */
    if(object==NULL) object=(ATK  *)this->inputFocus;

    if((object)->IsType( viewinfo) && procTableEntry->type) {
	class view *v=(class view *)object;
	while(v && !(v)->IsType( procTableEntry->type)) v=v->parent;
	if(v) object=(ATK  *)v;
    }
    return HandleProc(this, procTableEntry, object, rock, NULL);
}

/* We have a hit method here so that it can be subclassed, if necessary
  for such things as override windows */

class view *
im::Hit (enum view_MouseAction  action, long  x , long  y , long  clicks)
{
    return (this->topLevel)->Hit( action, x, y, clicks);
}

class im *
im::HandleMouse(enum view_MouseAction  action, long  x, long  y, long  newButtonState)
					{
	register long dest = destroycount;

	if(RECORDING) {
	    enqAction(&Record, mouseAction(this, action, x, y, newButtonState));
	    im::SuspendRecording();
	}
	
	if (this->keystate) {
	    (this->keystate)->Reset();
	}

	if (this->topLevel != NULL && action != view_NoMouseEvent) {
		if ((this->buttonState == im_AllUp 
					&& newButtonState != im_AllUp)
				|| this->mouseFocus == NULL)  {
			register boolean closeHit;

			closeHit = (this->lastEvent == im_MouseEvent) 
					&& (this->lastMouseDown == action) 
					&& (((this->lastX - HITPIXELS) <= x) 
					&& ((this->lastX + HITPIXELS) >= x) 
					&& ((this->lastY - HITPIXELS) <= y) 
					&& ((this->lastY + HITPIXELS) >= y));
			if (closeHit)
				this->clickCount += 1;
			else if (action == view_LeftDown 
						|| action == view_RightDown)  {
				this->lastMouseDown = action;
				this->clickCount = 1;
				this->lastX = x;
				this->lastY = y;
			}
			this->buttonState = newButtonState;

			// call the client
			this->mouseFocus = (this)->Hit( action, x, y, 
					this->clickCount);

			if (action == view_UpMovement)
				this->mouseFocus = NULL;
			else if (mouseFocus==NULL) {
			    fprintf(stderr, "WARNING: mouseFocus==NULL.\n");
			    fprintf(stderr, "A view just clicked in is broken.\n");
			} else if (mouseFocus->GetIM() != this) {
			    im *other=mouseFocus->GetIM();
			    other->buttonState=newButtonState;
			    other->mouseFocus=mouseFocus;
			    mouseFocus=NULL;
			    buttonState=im_AllUp;
			}
		}
		else {
			class view *hitee = this->mouseFocus;

			if (newButtonState == im_AllUp)
				this->mouseFocus = NULL;

			this->buttonState = newButtonState;
			(hitee)->Hit( action,
			 	physical_GlobalXToLogicalX((hitee)->GetDrawable(),x),
				physical_GlobalYToLogicalY((hitee)->GetDrawable(),y),
				this->clickCount);
		}
	}
	if(RECORDING) im::ResumeRecording();
	
	if(dest == destroycount || stillexists(this)) /* make sure self still exists */
	{
	    /* moved from in front of this if since it only makes sense if self still exists */
	    SetArgProvided(this, FALSE);
	    this->lastCommand = 0;
	    return this;
	}
	return NULL;
}

void im::NormalConfiguration(long  rock , long  customrock, class im  *parent, int  *x , int  *y, unsigned int  *w , unsigned int  *h)
{
    if(rock&im_AtTop) {
	long py=(parent)->GetVisualTop();
	long ph=(parent)->GetVisualHeight();
	*y=py+ph/10;
    } else if(rock&im_InMiddle) {
	long ph=(parent)->GetVisualHeight();
	*y=((long)ph/2)-((long)*h/2);
    } else if(rock&im_AtBottom) {
	long py=(parent)->GetVisualBottom();
	long ph=(parent)->GetVisualHeight();
	*y=py-ph/10-*h;
    }
    if(rock&im_Centered) {
	long pw=(parent)->GetVisualWidth();
	*x=((long)pw/2)-((long)*w/2);
    }
}

/* so that im's can easily override the "normal" configuration function */
static void GenericConfig(class im  *self, long  rock , long  customrock, class im  *parent, int  *x , int  *y, unsigned int  *w , unsigned int  *h)
{
    (self)->NormalConfiguration( rock, customrock, parent, x, y, w, h);
}

static im_configurefptr configfunc=GenericConfig;

im_configurefptr im::DefaultConfigureFunction(im_configurefptr  func)
{
	ATKinit;

    im_configurefptr result=::configfunc;
    ::configfunc=func;
    return result;
}

static long configrock=0;
long im::DefaultConfigureRock(long  rock)
{
	ATKinit;

    long result=::configrock;
    ::configrock=rock;
    return result;
}

static long configcustomrock=0;
long im::DefaultConfigureCustomRock(long  rock)
{
	ATKinit;

    long result=::configcustomrock;
    ::configcustomrock=rock;
    return result;
}

static boolean defaulticonic=FALSE;

void im::SetDefaultIconic(boolean  val)
{
	ATKinit;

    defaulticonic=val;
}

boolean im::GetDefaultIconic()
{
	ATKinit;

    return defaulticonic;
}

#if defined(RCH_ENV) && SY_AIXx
void TraceSignal(int sig, int code, struct sigcontext *context)
{
    struct im *im = im::GetLastUsed();
    char *appname;
    time_t curtime = time(NULL);
    FILE *f;
    char **env;
    struct configurelist *cl;
    char buffer[1024];

    if (tracedir == NULL || tracedir[0] == '\0')
	return;
    appname = im::GetProgramName();
    if (appname == NULL)
	appname = "unknown";
    if (strcmp(tracedir, "stderr") == 0)
	f = stderr;
    else if (strcmp(tracedir, "stdout") == 0)
	f = stdout;
    else {
	sprintf(buffer, "%s/%s.%d", tracedir, appname, trace_starttime);
	f = fopen(buffer, "w");
	if (f != NULL) {
	    fprintf(stderr, "%s:  Abnormal termination!  A problem report was logged.\n", appname);
	    setbuf(f, NULL);
	    fprintf(f, "Application: %s\n", appname);
	    fprintf(f, "Start-time:  %s", ctime(&trace_starttime));
	    fprintf(f, "Crash-time:  %s", ctime(&curtime));
	    fprintf(f, "User-UID:    %d\n", getuid());
	    fprintf(f, "System-Type: %s\n", SYS_NAME);
	    fprintf(f, "Version:     %s\n", app_version_string);
	    if (gethostname(buffer, sizeof(buffer)) == -1)
		fprintf(f, "hostname:    (gethostname failed, errno=%d)\n", errno);
	    else
		fprintf(f, "Hostname:    %s\n", buffer);
	    if (readlink("/usr/andrew", buffer, sizeof(buffer)) == -1)
		fprintf(f, "AndrewDir:   (readlink failed, errno=%d)\n", errno);
	    else
		fprintf(f, "AndrewDir:   %s\n", buffer);
	    fprintf(f, "----------   Traceback   ----------\n");
	    traceback_signal(f, sig, code, context);
	    fclose(f);
	}
	exit(100+sig);
    }
    /* Only get here if stdout or stderr. */
    fprintf(f, "\n----------   Traceback of %s  ----------\n", appname);
    traceback_signal(f, sig, code, context);
    exit(100+sig);
}
#endif


im::im()
        {
	ATKinit;

#if defined(RCH_ENV) && SY_AIXx
	{
	    static boolean first_time = TRUE;
	    int siglist[] = {SIGILL, SIGFPE, SIGBUS, SIGSEGV, 0};
	    int i = 0;
	    struct sigaction sa;
	    sigset_t empty;
	    application *app;

	    if (first_time) {
		tracedir = environ::GetProfile("TraceLogDirectory");
		if (tracedir != NULL && strcmp(tracedir, "none") != 0) {
		    while (siglist[i] != 0) {
			sigemptyset(&empty);
			sa.sa_handler = (void (*)(int))TraceSignal;
			sa.sa_mask = empty;
			sa.sa_flags = 0;
			sigaction(siglist[i], &sa, NULL);
			i++;
		    }
		}
		trace_starttime = time(0);
		// Get app version string for tracebacks.
		if (app = application::GetStartupApplication()) {
		    sprintf(app_version_string, "%d.%d ATK %s", app->GetMajorVersion(), app->GetMinorVersion(), application::GetATKVersion());
		}
		first_time = FALSE;
	    }
	}
#endif

    class atom * atom;

    doRedraw=TRUE;
    this->lastCommand=0;
    this->starticonic=FALSE;
    this->automap=TRUE;
    
    this->postedml=NULL;
    
    this->initversion=0;
    
    this->configfunc=::configfunc;
    this->configrock=::configrock;
    this->configcustomrock=::configcustomrock;
    this->delete_window_cb = NULL;
    this->delete_window_rock = 0;
    this->topLevel = NULL;
    this->inputFocus = NULL;
    this->mouseFocus = NULL;
    this->buttonState = im_AllUp;
    this->imKeystate = keystate::Create(this, globalKeymap);
    this->keystate = this->imKeystate;
    this->menus = (imMenus)->DuplicateML( this);
    this->lastEvent = im_NoEvent;
    this->lastX = -1;
    this->lastY = -1;
    this->lastMouseDown = view_NoMouseEvent;
    this->clickCount = 1;
    this->next = imList;
    imList = this;
    this->imPtr = this;
    this->argState.argument = 1;
    this->argState.argProvided = FALSE;
    this->argState.argNext = FALSE;
    this->argState.argDigit = FALSE;
    this->argState.argPending = FALSE;
    this->argState.cmdpos = 0;
    this->cursorlist = NULL;
    this->WindowCursor = NULL;
    this->CursorsAreActive = 0;
    this->handlers = NULL;
    this->programName = NULL;
    this->title = NULL;
    if (globalInit != NULL)
        this->init = (globalInit)->Duplicate();
    else
	this->init = NULL;
    if(this->init) this->initversion=this->init->version;
    this->interactionEvents = NULL;
    this->pendingInteractionEvents = NULL;
    this->cursorPostsPending = FALSE;
    this->LogFile = NULL;
#if 0
    /* really the im name should be unique, but this
     way is a core leak... */
    atom = atom::InternRock((long) this);
    this->name = new atomlist; /* can't use setname here because of a class `feature' */    
    (this->name)->Prepend(atom);
#else
    this->name = atomlist::StringToAtomlist("im");
#endif    
    this->className = atomlist::StringToAtomlist("im");

    this->keyEchoState=im_KeyEchoOff;

    this->moreRecentlyUsed = this->lessRecentlyUsed = (class im *) 0;

    this->installedColormap = NULL;

    im_kind = im_NORMAL;	// good default

    bbarv = NULL;
    // bbar_size and bbar_location are no longer used.

    THROWONFAILURE( TRUE);
}

static class im *lastUsed = NULL;

class im *im::GetLastUsed()
    {
	ATKinit;

    return lastUsed;
}

void im::SetLastUsed(class im  *used)
        {
	ATKinit;

    if (lastUsed == used)
	return;
    if (used->moreRecentlyUsed)
	(used->moreRecentlyUsed)->lessRecentlyUsed =
	    used->lessRecentlyUsed;
    if (used->lessRecentlyUsed)
	(used->lessRecentlyUsed)->moreRecentlyUsed =
	    used->moreRecentlyUsed;
    used->lessRecentlyUsed = lastUsed;
    if (lastUsed)
	lastUsed->moreRecentlyUsed = used;
    used->moreRecentlyUsed = (class im *) 0;
    lastUsed = used;
}

static class view *selectionOwner=NULL;
static class im *ownerIM=NULL;

void im::ObservedChanged(class observable  *changedo, long  value)
{
    class view *changed=(class view *)changedo;
    if(value!=observable_OBJECTDESTROYED) return;
    if(changed!=selectionOwner) return;
    selectionOwner=NULL;
    ownerIM=NULL;
}

im::~im()
{
    register class im *im = imList;
    register class im *prevIM = NULL;
    struct handler *next_handler;

    if(ownerIM==this && selectionOwner) {
	(selectionOwner)->RemoveObserver( this);
	selectionOwner=NULL;
	ownerIM=NULL;
    }
    resetKeyEcho(this); 
   

    if (this->bbarv) {
	this->bbarv->UnlinkTree();
	this->bbarv->Destroy();
    } else if (this->topLevel)
	(this->topLevel)->UnlinkTree();

    while (im != NULL && im != this)  {
	prevIM = im;
	im = im->next;
    }
    if (im != NULL)  {
	if (prevIM != NULL)
	    prevIM->next = im->next;
	else
	    imList = im->next;
    }
    delete this->imKeystate;
    delete this->menus;
    if (this->init != NULL)
        delete this->init;
    pruneActions(this);

    FreeInteractionEvents(this);

    if (this->moreRecentlyUsed || this->lessRecentlyUsed) {
	if (this->moreRecentlyUsed)
	    (this->moreRecentlyUsed)->lessRecentlyUsed =
		this->lessRecentlyUsed;
	if (this->lessRecentlyUsed)
	    (this->lessRecentlyUsed)->moreRecentlyUsed =
		this->moreRecentlyUsed;
    }
    if (lastUsed == this) {
	lastUsed = this->lessRecentlyUsed;
    }

    if (this->LogFile) 
	fclose(this->LogFile);
    destroycount++;

    while(this->handlers) {
	if(this->handlers->name) free(this->handlers->name);
	next_handler = this->handlers->next;
	free(this->handlers);
	this->handlers=next_handler;
    }
    if(this->title) {
	free(this->title);
	this->title=NULL;
    }
}

void im::WantUpdate(class view  *requestor)
        {
    (globalUpdateList)->AddTo( requestor);
}

void im::WantInputFocus(class view  *requestor)
        {
    class colormap **current = NULL, **new_c = NULL;
    class view *ancestor;
    if (this->inputFocus != NULL) {
	ancestor=this->inputFocus->parent;
	while(ancestor) {
	    ancestor->ChildLosingInputFocus();
	    ancestor=ancestor->parent;
	}
	(this->inputFocus)->LoseInputFocus();
	if(requestor)
	    current = (this->inputFocus)->CurrentColormap();
    }
    if(requestor) {
	ancestor=requestor->parent;
	while(ancestor) {
	    ancestor->ChildReceivingInputFocus();
	    ancestor=ancestor->parent;
	}
    }
    this->inputFocus = requestor;
    SetArgProvided(this, FALSE);
    this->lastCommand = 0;
    if (this->inputFocus != NULL) {
	new_c = (this->inputFocus)->CurrentColormap();
	if(current != new_c)
	    (this)->InstallColormap( *new_c);
	(this->inputFocus)->ReceiveInputFocus();
    }
}

void
im::WantColormap( class view  *requestor, class colormap  **cmap )
            {
    class colormap **cMap = NULL;
    class view *v;
    if(requestor) {
	if(cmap) {
	    (requestor)->SetColormap( cmap);
	    (this)->InstallColormap( *cmap);
	    (requestor)->ReceiveColormap( *cmap);
	}
	else {
	    class colormap **inherited = (requestor)->GetInheritedColormap();
	    if((cMap = (requestor)->GetColormap()) && *cMap)
		(requestor)->LoseColormap( *cMap);
	    (requestor)->SetColormap( NULL);
	    (this)->InstallColormap( *inherited);
	}
    }
}

void im::WantNewSize(class view  *requestor)
{
}

ATK  *im::WantHandler(char  *handlerName)
        {
    struct handler *ptr;

    for (ptr = this->handlers; ptr != NULL; ptr = ptr->next)
        if (strcmp(ptr->name, handlerName) == 0)
            return ptr->handler;

    return NULL;
}

char *im::WantInformation(char  *key)
        {
    return NULL;
    }
    
void im::PostKeyState(class keystate  *keystate)
{
    this->keystate = (this->imKeystate)->AddAfter( keystate);

    if (this->init != NULL)
	this->keystate = (this->init)->ModifyKeystate( this->keystate);
    if (this->keystate != NULL)  {
	(this->keystate)->Reset();
    }
    SetArgProvided(this, FALSE);
    this->lastCommand = 0;
}

/* Menu stuff... */


void im::PostMenus(class menulist  *menulist)
        {
    printf("im_PostMenus: missing method\n");
}

void im::PostDefaultHandler(char  *handlerName, ATK   *handler)
            {
    struct handler **ptr;
    struct handler *next_handler;

    for (ptr = &(this->handlers); *ptr != NULL; ptr = &((*ptr)->next))
        if (strcmp((*ptr)->name, handlerName) == 0)
            break;

    if (handler == NULL) {
        if (*ptr != NULL) {
	    next_handler = (*ptr)->next;
            free((*ptr)->name);
            free(*ptr);
            *ptr = next_handler;
        }
    }
    else {
        struct handler *this_c = *ptr;

        if (this_c == NULL) {
            this_c = (struct handler *)malloc(sizeof(struct handler));
            this_c->next = this->handlers;
            this->handlers = this_c;
        }
        else
            free(this_c->name);

        this_c->name = (char *)malloc(strlen(handlerName)+1);
        strcpy(this_c->name, handlerName);
        this_c->handler = handler;
    }
}
    
void im::SetView(class view  *topLevel)
        {
    if (this->bbarv)
        this->bbarv->UnlinkTree();
    else if (this->topLevel)
        this->topLevel->UnlinkTree();

/* In theory, all of these lines of code will be handled by the UnlinkNotification procedure,
 * Maybe later, I will remove them.
 */
    this->inputFocus = NULL;
    this->mouseFocus = NULL;
    this->buttonState = im_AllUp;
    this->keystate = this->imKeystate;
    this->lastX = -1;
    this->lastY = -1;
    this->lastMouseDown = view_NoMouseEvent;
    this->clickCount = 1;

    this->topLevel = topLevel;
    this->doRedraw = TRUE;
    this->argState.argument = 1;
    SetArgProvided(this, FALSE);
    this->argState.argNext = FALSE;
    this->argState.argDigit = FALSE;
    this->lastCommand = 0;
    if(topLevel == NULL){
	if (this->bbarv)
	    ((viewholderv *)this->bbarv)->setChild(NULL);
    } else {
	if (!this->bbarv && this->im_kind == im_NORMAL &&
	    environ::GetProfileSwitch("ButtonBar", FALSE)) {
	    // Init the buttonbar now.
	    const char *bbar_name = environ::GetProfile("ButtonBarContainer");
	    viewholderv *vh = NULL;

	    if (bbar_name)
		vh = (viewholderv *)ATK::NewObject(bbar_name);
	    else
		fprintf(stderr, "ButtonBarContainer preference not specified.  Ignoring ButtonBar preference.\n");
	    if (vh && vh->IsType("viewholderv")) {
		// Got a valid viewholder subclass.
		// XXX stick it in bbarv, which is currently declared as a view *.
		// XXX fix bbarv later.
		this->bbarv = vh;
	    }
	}
	if (this->bbarv) {
	    // Have a buttonbar.  Insert the buttonbar view into the window,
	    // and give the toplevel view to the bbar.
	    ((viewholderv *)this->bbarv)->setChild(topLevel);
	    this->bbarv->LinkTree(this); /* Sets up parent and imPtr fields. */
	    this->bbarv->InsertView(this, &this->drawable->localBounds);
	} else {
	    // No buttonbar.  Be lightweight.  Insert the toplevel directly.
	    // Alternatively, we could implement a dummy viewholderv.
	    topLevel->LinkTree(this); /* Sets up parent and imPtr fields. */
	    topLevel->InsertView(this, &this->drawable->localBounds);
	}
    }
    globalDoRedraw = TRUE;
}

	boolean
im::CreateWindow(char  *host)
        {
    printf("im_CreateWindow: missing method\n");
    return FALSE;
}

void im::SetBorderWidth(int  n)
{
    printf("im_SetBorderWidth: missing method\n");
}

/* these should be overridden by any ims which support transients or overrides, they need to be methods so that if you have a generic im pointer you can discover whether the specific kind of im underlying it supports transients or overrides, as opposed to the previous macros which would only actually tell you if the class used in the *im_SupportsTransient call supports transients */
boolean im::SupportsTransient()
{
    return FALSE;
}

boolean im::SupportsOverride()
{
    return FALSE;
}

/* It is expected that CreateTransientWindow
 will be overridden by our window-server specific 
 subclass.
 If our window server does not support Transient
 Windows we will create a top level one instead. */

	boolean
im::CreateTransientWindow(class im  *other, int flags, im_configurefptr cfp, long crock)
    {
    return (this)->CreateWindow( NULL);
}

/* It is expected that CreateOverrideWindow
 will be overridden by our window-server specific 
 subclass.
 If our window server does not support Override
 Windows we will create a top level one instead. */

	boolean
im::CreateOverrideWindow(class im  *other, im_configurefptr cfp, long crock)
    {
    return (this)->CreateWindow( NULL);
}

static class im *
DoCreate(char  *host, class im  *other, int  flag, int  width , int  height, int flags=im_TRANSIENTMENUS, im_configurefptr cfp=NULL, long crock=0)
{
    class im *newIM;
    unsigned char *logdir;

    newIM = (currentWS)->CreateIM();

    switch (flag) {
	case im_NORMAL:
	    if (newIM == NULL  ||  ! (newIM)->CreateWindow( host))
		return NULL;
	    break;
	case im_TRANSIENT:
	    if (newIM == NULL  || ! (newIM)->CreateTransientWindow( other, flags, cfp, crock))
		return NULL;
	    break; 
	case im_OVERRIDE:
	    if (newIM == NULL  || ! (newIM)->CreateOverrideWindow( other, cfp, crock))
		return NULL;
	    break;
	case im_OFFSCREEN:
	    if (newIM == NULL  || ! (newIM)->CreateOffscreenWindow( other, width, height))
		return NULL;
	    break;
	default: return NULL;
    }

    logdir = (unsigned char *)environ::Get("LOGDIRECTORY");
    if (logdir) {
	/* initialize logging of user actions */
	char  name[200];
	register class tm *tm;

	LogStart = time (0) - 10000;
	tm = localtime(&LogStart);
	sprintf(name, "%s/Log.%d.%02d.%02d.%02d.%02d", logdir, getpid(),
		tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min);
	newIM->LogFile = fopen(name, "w");
	WriteLogEntry(newIM, log_HOST, host);
    }
    newIM->im_kind = flag;
    return newIM;
}

class im *im::Create(char  *host)
{
	ATKinit;

	return (DoCreate(host, NULL, im_NORMAL,0, 0));
}

/* Just the same as im__Create except we call im_CreateTransientWindow. */

class im *im::CreateTransient(class im  *other, int flags, im_configurefptr cfp, long crock)
        {
	ATKinit;

	return (DoCreate(NULL, other, im_TRANSIENT, 0, 0, flags, cfp, crock));
}

class im *im::CreateOverride(class im  *other, im_configurefptr cfp, long crock)
        {
	ATKinit;

	return (DoCreate(NULL, other, im_OVERRIDE, 0, 0, 0, cfp, crock));
}

class im *im::CreateOffscreen(class im  *other, int  width , int  height)
{
	ATKinit;

    return (DoCreate(NULL, other, im_OFFSCREEN, width, height));
}

/* im__WhichWS()
	returns a string for the current window system:  "X" or "wm"
	(Overriden in the subclasses)
*/
	unsigned char *
im::WhichWS()
	{
	return (unsigned char *)"none";
}


void im::ForceUpdate()
    {
	ATKinit;

    im::RedrawChangedWindows();
    (globalUpdateList)->Clear();
    (currentWS)->FlushAllWindows();
}

void im::RedrawChangedWindows()
{
	ATKinit;

    class im *im;

    globalDoRedraw=FALSE;

    for(im=imList; im!=NULL; im=im->next)
	if(im->doRedraw){
	    (globalUpdateList)->DeleteTree(im);

	    im->doRedraw=FALSE;
	    (im)->RedrawWindow();
	}
}

void im::RedrawWindow()
{
}

static char workingDirectory[MAXPATHLEN];
static boolean initializeWorkingDirectory = TRUE;
#if !defined(hpux) && !defined(M_UNIX)
	
#endif /* hpux */


/* the next two routines are complements of CCH */
/*  Change to use PWD if defined instead of getcwd();
 *  This shows what the user expects to see, instead of the
 *  physical path, which maybe confusing.
 */
static void 
set_logical_wd(char  *dir ,	const char  *newdir)
{
#ifdef DOS_STYLE_PATHNAMES
    if (*newdir != '/' && *(newdir+1) != ':')
#else
    if( *newdir != '/')
#endif
    {
	strcat(dir, newdir);
		/* This really needs further work to take care of "../" etc */
		/* Fortunately this leg does not seem to get used at all! */
		/* im__ChangeDirectory seems to always get full paths to change to */
		/* but I will leave this in here just in case */
    }
    else {
	char *p;
		/* get rid of the last '/' */
	strcpy(dir, newdir);
	p = strrchr(dir, '/');
#ifdef DOS_STYLE_PATHNAMES
	if (p == NULL) p=strrchr(dir, '\\');
#endif
	if (p != NULL && *(p+1) == '\0') *p = '\0';
    }
}

static char *get_logical_wd(char  *dir)
	 {
#ifdef LOGICAL_WD_ENV
/* this code is ifdef'ed out because Zalman feels it can give incorrect results */
	char *wdir = environ::Get("PWD");
	if( wdir != NULL && *wdir != '\0') {
		strcpy(dir, wdir);
		return(dir);
	} 
	else 
#endif /* LOGICAL_WD_ENV */
#ifdef M_UNIX
		/* this may not be needed on all systems -
		 * it's to work around a problem with popen()
		 */
		(void)getwd(dir);
#endif /* M_UNIX */
		return(getwd(dir));
}


	long 
im::ChangeDirectory(const char  *dirName)
		{
	ATKinit;

	register long code;

	if ((code = chdir(dirName)) >= 0)  /* Successful */
	{
		set_logical_wd(workingDirectory, dirName);
	 	initializeWorkingDirectory = FALSE;
	}
	return code;
}

	char *
im::GetDirectory(char  *outputString)
		{
	ATKinit;

	boolean returnFail = FALSE;

	if (initializeWorkingDirectory)
		returnFail = (get_logical_wd(workingDirectory) == NULL);
	strcpy(outputString, workingDirectory);
	if (returnFail)
		return NULL;
	initializeWorkingDirectory = FALSE;
	return outputString;
}


void im::DeliverSignals()
    {
	ATKinit;

    register int i;
    anyDelivered = 0;
    for(i=1;i<NSIG;i++) {
	if(sigDelivered[i]) {
	    sigDelivered[i] = 0;
	    (*sigProcs[i])(i, sigData[i]);
	}
    }
}

static SIGNAL_RETURN_TYPE InternalSignal (int asigno)
{
    anyDelivered = 1;
    sigDelivered[asigno] = 1;
    PollTime.tv_sec = 0;
    PollTime.tv_usec = 0;
}


void im::SignalHandler(long  signalNumber, im_signalfptr proc, char  *procdata)
                {
	ATKinit;

    sigProcs[signalNumber] = proc;
    sigData[signalNumber] = procdata;
#ifdef HAVE_POSIX_SIGNALS
    {
        struct sigaction sa;
	sigset_t empty;

	sigemptyset(&empty);
	sa.sa_handler = SIGACTIONHANDLERFUNC(InternalSignal);
	sa.sa_mask = empty;
	sa.sa_flags = 0;
	sigaction(signalNumber, &sa, NULL);
    }
#else
    signal(signalNumber, InternalSignal);
#endif
}


boolean im::AddFileHandler (FILE  *file, im_filefptr  proc, char  *procdata, long  priority)
                    {
	ATKinit;

    register long i;
    register struct FILEHandlers  *p = globalFILEHandlers;

    for (i = 0; i < NFILEHandlers; i++, p++)  {
	if (p->file == file) {
	    if (p->priority != priority)  {
		im::RemoveFileHandler(file);
		break;
	    }
	    p->proc = proc;
	    p->procdata= procdata;
	    return TRUE;
        }
    }

    if (i >= NUMFILE)
	return FALSE;

    for (p = globalFILEHandlers+NFILEHandlers-1; p>=globalFILEHandlers && p->priority > priority; p--)  {
	*(p+1) = *p;
    }

    NFILEHandlers++;
    p++;
    p->file = file;
    p->proc = proc;
    p->procdata = procdata;
    p->priority = priority;
    return TRUE;
}

void im::RemoveFileHandler (FILE  *file)
        {
	ATKinit;

    register struct FILEHandlers *p = &globalFILEHandlers[NFILEHandlers];

    while (--p >= globalFILEHandlers)  {
	if (p->file == file) {
	    for (NFILEHandlers--; p < &globalFILEHandlers[NFILEHandlers]; p++)
		*p = *(p+1);
	    return;
	}
    }
}

boolean im::AddCanOutHandler (FILE  *file, im_filefptr  proc, char  *procdata, long  priority)
                    {
	ATKinit;

    register long i;
    register struct FILEHandlers  *p = CanOutHandlers;

    for (i = 0; i < NCanOutHandlers; i++, p++)  {
	if (p->file == file) {
	    if (p->priority != priority)  {
		im::RemoveCanOutHandler(file);
		break;
	    }
	    p->proc = proc;
	    p->procdata= procdata;
	    return TRUE;
        }
    }

    if (i >= NUMFILE)
	return FALSE;

    for (p = CanOutHandlers+NCanOutHandlers-1; p>=CanOutHandlers && p->priority > priority; p--)  {
	*(p+1) = *p;
    }

    NCanOutHandlers++;
    p++;
    p->file = file;
    p->proc = proc;
    p->procdata = procdata;
    p->priority = priority;
    return TRUE;
}

void im::RemoveCanOutHandler (FILE  *file)
        {
	ATKinit;

    register struct FILEHandlers *p = &CanOutHandlers[NCanOutHandlers];

    while (--p >= CanOutHandlers)  {
	if (p->file == file) {
	    for (NCanOutHandlers--; p < &CanOutHandlers[NCanOutHandlers]; p++)
		*p = *(p+1);
	    return;
	}
    }
}
	static void
ProcessInputQueue() 
{
	struct action *a;
	class im *self;
	a = GetNextInputEvent();
	if (a == NULL) return;
	self = a->im;
	if(self) im::SetLastUsed(self);

	switch (a->type) {
	    case im_KeyboardEvent:
	    if (a->v.key == IM_METAESC)
		self = (a->im)->DoKey( '\033');
	    else
		self = (a->im)->DoKey( a->v.key);
	    break;
	case im_MenuEvent:
	    self = (a->im)->HandleMenu( a->v.proc.procTableEntry, 
				     a->v.proc.object, a->v.proc.rock);
	    self=NULL;
	    break;
	case im_ProcEvent:
	    /* these will only occur when playing a macro,
	     they come at the end of a sequence of keys
	     getnextinput expanded out. */
	    self= NULL;
	    break;
	case im_MouseEvent:
	    self = (a->im)->HandleMouse( a->v.mouse.action, 
				a->v.mouse.x, a->v.mouse.y, a->v.mouse.newButtonState);
	    break;
	}
	if(self)
	    self->lastEvent = a->type;
}



 void im::HandleRedraw ()
    {
    printf("im_HandleRedraw: missing method\n");
}


#if defined(hp9000s300) && HP_OS < 70
xim_sigAlrm()
{ }
#endif /* hp9000s300 */

boolean im::Interact(boolean  mayBlock)
{
    ATKinit;

    class im *trav=imList;

    while(trav) {
	if(trav->init && trav->init->version!=trav->initversion) {
	    if(trav->inputFocus!=NULL) {
		(trav->inputFocus)->ReceiveInputFocus();
	    }
	    trav->initversion=trav->init->version;
	}
	trav=trav->next;
    }

    if (InQ != NULL) 
	ProcessInputQueue();
    else if (globalDoRedraw) 
	im::RedrawChangedWindows();
    else if ((currentWS)->HandleFiles( 0, TRUE))  {
		/* if HandleFiles called a handler, don't do the updatelist clear,
		  or anything else, for that matter. */
    }
    else { /* check timer and then other possibilities */
	long when, WaitTime;

	if (eventCount > 0) {
			/* do not rely on the timeout in fselect;  recompute the time 
			to see if first event should now occur */

	    when = event::Now();
	    WaitTime = event::FirstTime(when);
	    if (WaitTime == event_ENDOFTIME)
		eventCount = 0;
	}
	else WaitTime = event_ENDOFTIME;

	if (WaitTime < event_MSECtoTU(10))  {
	    eventCount -= 1;
	    event::HandleTimer(when);
	}
	else if ((globalUpdateList)->UpdatesPending()) 
	    (globalUpdateList)->Clear();
	else if (childDied) {
	    childDied = FALSE;
	    if (cleanUpZombies) {
#if defined(hpux) && HP_OS < 70
		struct sigvec vecAlrm;
		struct itimerval timer;
		int pid;
		WAIT_STATUS_TYPE status;

				/** enable an interval timer to abort the wait(); **/
		vecAlrm.sv_handler = xim_sigAlrm;
		vecAlrm.sv_mask = 0;
		vecAlrm.sv_flags = 0;
		sigvector(SIGALRM, &vecAlrm, 0);
		timer.it_value.tv_sec = 0;
		timer.it_value.tv_usec = 100000;
		timer.it_interval.tv_sec = 0;
		timer.it_interval.tv_usec = 100000;
		setitimer(ITIMER_REAL, &timer, 0);

				/** God only knows if the status this wait returns looks
				    anything like what wait3 will return;  I hope it's good
				    enough to pass to the zombie handler; 
				 **/
		while ((pid = wait(&status)) > 0) {
		    struct zombiehandler *thisHandler;
		    for (	thisHandler = allZombieHandlers;
			 thisHandler != NULL 
			   && thisHandler->pid != pid;
			 thisHandler = thisHandler->next)
		    {}
		    if (thisHandler != NULL) { thisHandler->function(pid, thisHandler->functionData, &status);
		    im::RemoveZombieHandler(pid);
		    }
		}

				/** disable the timer **/
		timer.it_value.tv_sec = 0;
		timer.it_value.tv_usec = 0;
		setitimer(ITIMER_REAL, &timer, 0);
#else /* any system except HP_OS < 70 */
		int pid;

#ifdef HAVE_WAITPID
		    WAIT_STATUS_TYPE status;
		while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
#else
		    WAIT_STATUS_TYPE status;
		while ((pid = wait3 (&status, WNOHANG, 0)) > 0)
#endif
		{
		    struct zombiehandler *thisHandler;
		    for (	thisHandler = allZombieHandlers;
			 thisHandler != NULL 
			   && thisHandler->pid != pid; 
			 thisHandler = thisHandler->next)
		    {}
		    if (thisHandler != NULL) {
			(*thisHandler->function)
			  (pid, 
			   thisHandler->functionData,
			   &status);
			im::RemoveZombieHandler(pid);
		    }
		}
#endif /* defined(hpux) && HP_OS < 70 */
	    } /* cleanUpZombies */
#ifdef M_UNIX
	    signal(SIGCHLD, DeathInTheFamily);
#endif
	} /* childDied */

	else {
	    if (mayBlock) 
		(currentWS)->HandleFiles( WaitTime, FALSE);
	    else 
				/* interact with mayBlock FALSE 
				    returns whether we should call it again */
		return (currentWS)->HandleFiles( 0, FALSE);
	}

    } /* check timer and then other possibilities */

    return TRUE;
}




/* This function has largely been replaced by im_AddZombieHandler.
 * It is still here because it is very convenient for the messages system
 * model of the world and possibly other programs which were not originally
 * native to BE 2. Don't remove it without contacting the current messages
 * maintainer.
 */
void im::SetCleanUpZombies(boolean  value)
        {
	ATKinit;

#ifdef HAVE_POSIX_SIGNALS
    {
        struct sigaction sa;
	sigset_t empty;

	sigemptyset(&empty);
        if ((cleanUpZombies = value)) /* Insure proper state of signal handler. */
            sa.sa_handler = SIGACTIONHANDLERFUNC(DeathInTheFamily);
        else
	    sa.sa_handler = SIGACTIONHANDLERFUNC(SIG_DFL);
	sa.sa_mask = empty;
	sa.sa_flags = 0;
	sigaction(SIGCHLD, &sa, NULL);
    }
#else
    if ((cleanUpZombies = value)) /* Insure proper state of signal handler. */
        signal(SIGCHLD, DeathInTheFamily);
    else
        signal(SIGCHLD, SIG_DFL);
#endif
}

void im::AddZombieHandler(int  pid, im_zombiefptr  function, long  functionData /* Actually any 32 bit crufty. */)
                {
	ATKinit;


    struct zombiehandler *thisHandler;

    for (thisHandler = allZombieHandlers; thisHandler != NULL && thisHandler->pid != pid; thisHandler = thisHandler->next)
        ;

    if (thisHandler != NULL) {
        thisHandler->pid = pid;
        thisHandler->function = function;
        thisHandler->functionData = functionData;
    }
    else {
        thisHandler = (struct zombiehandler *) malloc(sizeof(struct zombiehandler));
        thisHandler->pid = pid;
        thisHandler->function = function;
        thisHandler->functionData = functionData;
        thisHandler->next = allZombieHandlers;
        allZombieHandlers = thisHandler;
    }
}

void im::RemoveZombieHandler(int  pid)
        {
	ATKinit;


    struct zombiehandler *thisHandler, **previous = &allZombieHandlers;

    for (thisHandler = allZombieHandlers; thisHandler != NULL && thisHandler->pid != pid; thisHandler = thisHandler->next)
        previous = &thisHandler->next;

    if (thisHandler != NULL) {
        *previous = thisHandler->next;
        free(thisHandler);
    }
}


void im::KeyboardExit()
    {
	ATKinit;

    keyboardExitFlag = TRUE;
}

long im::KeyboardLevel()
    {
	ATKinit;

    return keyboardLevel;
}

void im::KeyboardProcessor()
    {
	ATKinit;

    if(defaulticonic) im::SetDefaultIconic(FALSE);
    keyboardLevel += 1;
    while (! keyboardExitFlag)
	im::Interact(TRUE);
    keyboardLevel -= 1;
    keyboardExitFlag = FALSE;
}


class event *im::EnqueueEvent(event_fptr proc, char  *procdata, long  timeIncrement)
                {
	ATKinit;

    class event *event;

    event = event::Enqueue(event::Now() + timeIncrement, proc, procdata);
    eventCount += 1;
    return event;
}

	boolean
im::IsPlaying()
	{
	ATKinit;

	return playingRecord;
}


static void InteractionEventWork(struct im_InteractionEvent  *interactionEvent)
    {

    class im *im = interactionEvent->im;
    struct im_InteractionEvent *event;
    struct im_InteractionEvent **previous = &im->interactionEvents;

    for (event = im->interactionEvents; event != NULL && event != interactionEvent; event = event->next)
        previous = &event->next;

    if (event == NULL)
        return; /* If this happens, some invariant has been violated. */
    
    *previous = event->next;
    event->next = im->pendingInteractionEvents;
    im->pendingInteractionEvents = event;
    event->event=NULL;
}

struct im_InteractionEvent *im::SetInteractionEvent(im_interactionfptr  interactionFunction, long  interactionData, long  timeIncrement)
                {

    struct im_InteractionEvent *newEvent;

    newEvent = (struct im_InteractionEvent *) malloc(sizeof(struct im_InteractionEvent));
    if (newEvent == NULL)
        return NULL;
    newEvent->function = interactionFunction;
    newEvent->data = interactionData;
    newEvent->im = this;
    newEvent->event = im::EnqueueEvent((event_fptr) InteractionEventWork, (char *) newEvent, timeIncrement);
    newEvent->next = this->interactionEvents;
    this->interactionEvents = newEvent;
    return newEvent;
}

void im::CancelInteractionEvent(struct im_InteractionEvent  *event)
        {

    struct im_InteractionEvent **previous = &this->interactionEvents;
    struct im_InteractionEvent *interactionEvent;

    for (interactionEvent = this->interactionEvents; interactionEvent != NULL && interactionEvent != event; interactionEvent = interactionEvent->next)
        previous = &interactionEvent->next;

    if (interactionEvent == NULL) {
        previous = &this->pendingInteractionEvents;
        for (interactionEvent = this->pendingInteractionEvents; interactionEvent != NULL && interactionEvent != event; interactionEvent = interactionEvent->next)
            previous = &interactionEvent->next;
    }

    if (interactionEvent != NULL) {
        *previous = interactionEvent->next;
        if(interactionEvent->event) (interactionEvent->event)->Cancel();
        free(interactionEvent);
    }
}

static void FreeInteractionEvents(class im  *self)
    {

    struct im_InteractionEvent *interactionEvent;
    struct im_InteractionEvent *next;

    for (interactionEvent = self->interactionEvents;
         interactionEvent != NULL;
         interactionEvent = next) {
        if(interactionEvent->event) (interactionEvent->event)->Cancel();
        next = interactionEvent->next;
        free(interactionEvent);
    }
    self->interactionEvents = NULL;

    for (interactionEvent = self->pendingInteractionEvents;
         interactionEvent != NULL;
         interactionEvent = next) {
        next = interactionEvent->next;
        free(interactionEvent);
    }
    self->pendingInteractionEvents = NULL;
}

void im::DispatchPendingInteractionEvents()
    {

    struct im_InteractionEvent *interactionEvent;
    struct im_InteractionEvent *nextInteractionEvent;

    for (interactionEvent = this->pendingInteractionEvents;
         interactionEvent != NULL; interactionEvent = nextInteractionEvent) {
        (*interactionEvent->function)(interactionEvent->data);
        nextInteractionEvent = interactionEvent->next;
        free(interactionEvent);
    }
    this->pendingInteractionEvents = NULL;
}

static void RedrawWindow(class im  *self, long  key)
        {
    self->doRedraw = TRUE;
    globalDoRedraw = TRUE;
}

struct action *im::GetMacro()
{
	ATKinit;

    return Record;
}

void im::SetMacro(struct action  *NewRecord)
{
	ATKinit;

    freeQlist(Record);
    Record=NewRecord;
}

/* Place all the actions in the list starting at a at the front of the queue and interact until they are gone. */
void im__PlayActions(class im  *self, struct action  *a)
{
    /* REMOVE THIS BEFORE FINAL CHECK-IN */
   /* struct action *newa, *last=NULL, *first=NULL;
    struct action *head=InQ;
    while(a) {
	newa=cloneAction(a);
	if(newa==NULL) return;
	newa->im=self;
	if(first==NULL) first=newa;
	if(last) last->next=newa;
	last=newa;
	a=a->next;
    }
    if(first && last) {
	last->next=head;
	InQ=first;
	while(InQ!=head && im_Interact(FALSE));
    }
    */
}    

static void StartKeyboardMacro(class im  *self, long  key)
        {
    if (playingRecord) 
	return;
    if (doRecord)  {
	message::DisplayString(self, 0, "Already recording events");
	return;
    }
    lastkeys=NULL;
    doRecord = TRUE;
    freeQlist(Record);
    Record = NULL;
    message::DisplayString(self, 0, "Recording on");
}

/* EditRecording edits out any non im_{Suspend,Answer,Resume}Events from within im_{Suspend,Resume}Event pairs, so that im_GetAnswer doesn't need to cope with ignoring them. */    
static void EditRecording()
{
    int susplevel=0;
    struct action *a=Record;
    struct action *last=NULL;
    while(a) {
	struct action *next=a->next;
	if(last) last->next=a;
	else Record=a;
	if(a->type==im_SuspendEvent) susplevel++;
	if(a->type==im_ResumeEvent && susplevel>0) susplevel--;
	if(susplevel>0 && a->type!=im_ResumeEvent && a->type!=im_SuspendEvent && a->type!=im_AnswerEvent) {
		freeQelt(a);
	} else {
	    last=a;
	}
	a=next;
    }
}

static void DumpActions(struct action  *a)
{
    while(a) {
	switch(a->type) {
	    case im_KeyboardEvent:
		printf("key:'%c'\n",a->v.key);
		break;
	    case im_ProcEvent:
		printf("proc %s\n",proctable::GetName(a->v.proc.procTableEntry));
		printf("rock %d '%c'\n",a->v.proc.rock, a->v.proc.rock);
		break;
	    case im_MenuEvent:
		printf("menu proc %s\n",proctable::GetName(a->v.proc.procTableEntry));
		printf("rock %d '%c'\n",a->v.proc.rock, a->v.proc.rock);
		break;
	    case im_MouseEvent:
		printf("mouse event!!\n");
		break;
	    case im_AnswerEvent:
		printf("answer:%x\n",a->v.answer);
		printf("answer:'%s'\n",a->v.answer);
		break;
	    case im_SuspendEvent:
		printf("suspend\n");
		break;
	    case im_ResumeEvent:
		printf("resume\n");
		break;
	    default:
		printf("unhandled type %d in im!\n",a->type);
	}
	a=a->next;
    }
}

static void StopKeyboardMacro(class im  *self, long  key)
{
    int i;
    struct action *look=Record;

    if (playingRecord) 
	return;

    if (doRecord)  {
	doRecord = FALSE;
	message::DisplayString(self, 0, "Recording off");
    }
    else
	message::DisplayString(self, 0, "You weren't recording events");
}

static void PlayKeyboardMacro(class im  *self, long  key)
        {
    register long count;
    
    count = (self)->Argument();
    (self)->ClearArg();
    if (doRecord)  {
	message::DisplayString(self, 0, "Can not execute event macro while recording events");
    }
    else if (playingRecord)  {
	message::DisplayString(self, 0, "Recursive call to playing event macro");
    }
    else if (Record == NULL)  {
	message::DisplayString(self, 0, "You haven't defined a keyboard macro yet");
    }
    else  {
	playingRecord = TRUE;
	enqAction(&InQ, macroAction(self, Record, Record, count));
	if (count > 0)
		message::DisplayString(self, 0, "Playing macro.  Control-G to quit early.");
    }
}

	void
im::CancelMacro()
	{
	ATKinit;

	/* if PendingRelease is not NULL, the macro has completed */
	if ( ! playingRecord || PendingRelease != NULL) return;

	/* remove macro from front of InQ */
	PendingRelease = InQ;
	InQ = InQ->next;

	playingRecord = FALSE;
}

/* Code for handling key events for im object (Used to be in keystate).
This section deals with the global command argument, usually set by the ^U command.
 */

static void SetArgProvided(class im  *self, boolean  value)
{
    if (self->argState.argProvided != value) {
	(self->keystate)->Reset();
	self->argState.argProvided = value;
    }
    self->argState.argPending = FALSE;
    self->argState.cmdpos = 0;
}

struct im_ArgState *im::GetArgState()
    {
    return &(this->argState);
}

void im::ClearArg()
    {
    this->argState.argument = 1;
}

boolean im::ArgProvided()
    {
    return this->argState.argProvided;
}

long im::Argument()
    {
    if (this->argState.argProvided)
	return this->argState.argument;
    else
	return 1;
}

void im::ProvideArg(long  arg)
        {
    this->argState.argNext = TRUE;
    this->argState.argument = arg;
}

void im::DisplayArg()
{
    char buf[30];

    struct im_ArgState *as = (this)->GetArgState();

    sprintf(buf, "Arg: %d", as->argument);
    message::DisplayString(this, 0, buf);
}

long im::BumpArg(long  val)
{
    struct im_ArgState *as = (this)->GetArgState();
    long newArg;

    if (! as->argDigit) {
	as->argDigit = TRUE;
	as->argument = 0;
    }
    newArg = as->argument * 10 + val;
    if (newArg > 65536)
	newArg = 65536;
    (this)->ProvideArg( newArg);
    return newArg;
}
    
/* These routines deal with the last command variable. */

static long nextCmdValue = 1;	/* next value to alloc */

long im::AllocLastCmd()
    {
	ATKinit;

    return nextCmdValue++;
}

long im::GetLastCmd()
    {
    return this->lastCommand;
}

void im::SetLastCmd(long  cmd)
        {
    thisCmd = cmd;
}

	void 
im::DoMacro()
	{
	if (doRecord)  {
		message::DisplayString(this, 0, 
			"Cannot execute event macro while recording events");
	}
	else if (playingRecord)  {
		message::DisplayString(this, 0, "Recursive call to playing event macro");
	}
	else  {
		playingRecord = TRUE;
		enqAction(&InQ, macroAction(this, Record, Record, 1));
		while (playingRecord) 
			im::Interact(FALSE);
	}
}

/* im__DoMenu(self, itemname)
	executes the function associated with the menuitem
	the itemname may be just the element name or the name preceded
		by the cardname and a comma.
	comparisons are case insensitive
*/
	void
im::DoMenu(char  *itemname)
		{
 	struct proctable_Entry *pe;
	ATK  *obj;
	long rock;
	char cbuf[256];
	char ibuf[256];
	char *p;
	
	if(itemname==NULL || strlen(itemname)>sizeof(cbuf)-1) return;
	
	while(isspace(*itemname)) itemname++;
	p=strchr(itemname, ',');
	if(p) {
	    strncpy(cbuf, itemname, p-itemname);
	    cbuf[p-itemname]='\0';
	    itemname=p+1;
	    for(p=cbuf;*p;p++) if(isupper(*p)) *p=tolower(*p);
	} else cbuf[0]='\0';
	while(isspace(*itemname)) itemname++;
	strcpy(ibuf, itemname);
	for(p=ibuf;*p;p++) if(isupper(*p)) *p=tolower(*p);
	if (getMenuEntry(this->postedml ? this->postedml : this->menus, cbuf, ibuf, &pe, &obj, &rock)) {
	    boolean olddontRecord=dontRecord;
	    dontRecord=TRUE;
	    (this)->HandleMenu( pe, obj, rock);
	    dontRecord=olddontRecord;
	}
}

/* im__CheckForInterrupt()
	scans ahead in user input looking for control-G and queuing the inputs
	if finds control-G, discards the input queue and returns TRUE
	otherwise returns FALSE
*/
	boolean
im::CheckForInterrupt()
	{
	ATKinit;

	struct action *tQ, *tx;
	class im *im;

	/* go get pending events from the windowsystem 
		actually do the select to get new input */
	(currentWS)->HandleFiles( 0, FALSE);

	/* Check queue for control-G */
	for (tQ = InQ;  tQ != NULL;  tQ = tQ->next) 
		if (tQ->type == im_KeyboardEvent && tQ->v.key == '\007') {
			/* BINGO! discard everything 
					up to and including the control-G */
			tx = InQ;
			InQ = tQ->next;	/* retain stuff after the ^G */
			tQ->next = NULL;

			/* XXX need to traverse the discarded stuff to update mouse state */

			freeQlist(tx);
			playingRecord = FALSE;  /* if it was TRUE, it isn't now */
			for (im = imList; im != NULL; im = im->next)
				/* discard any partial keystates */
				if (im->keystate != NULL)  
					(im->keystate)->Reset();
			return TRUE;
		}
	return FALSE;
}
	
boolean im::WasMeta()
{
    return (this->WasMetaVal);
}

static char charbuf[16];
static void RecordCharacter(long  key)
{
    switch(key) {
	case EOF:
	    strcpy(charbuf, "EOF");
	    break;
	case 0:
	    strcpy(charbuf, "NUL");
	    break;
	default:
	    charbuf[0]=(char)key;
	    charbuf[1]='\0';
	    break;
    }
    im::RecordAnswer(charbuf);
}

int im::GetCharacter()
{
    struct action * a;
    class view *curview = this->topLevel;
    class view *curfocus = this->inputFocus;
    char *answer=im::GetAnswer();
    if(answer) {
	if(answer[1]=='\0') return answer[0];
	else if(answer[0]=='E') return EOF;
	else if(answer[0]=='N') return 0;
    }
    while (TRUE) {
	if (InQ != NULL) {
	    while((a=PeekInputEvent()) && (a->type==im_ProcEvent || a->type==im_ResumeEvent || a->type==im_SuspendEvent || a->type==im_AnswerEvent)) a=GetNextInputEvent();
	    if(a!=NULL) {
		if ((a->type == im_KeyboardEvent || a->type == im_NoEvent) && a->im == this) {
		    a = GetNextInputEvent();
		    if(RECORDING) enqAction(&Record, keyAction(this, a->v.key));
		    if (a->v.key == IM_METAESC) {
			this->WasMetaVal = TRUE;
			if(RECORDING) {
			    RecordCharacter('\033');
			}
			return '\033';
		    }
		    else {
			this->WasMetaVal = FALSE;
			if(RECORDING) {
			    RecordCharacter(a->v.key);
			}
			return a->v.key;
		    }
		}
		else {
		    if(RECORDING) RecordCharacter(EOF);
		    return EOF;
		}
	    }
	}

		/* in the following call, InQ is NULL, so im_Interact will NOT
		 *	call ProcessInputQueue()
		*/
	im::Interact(TRUE);

	if (keyboardExitFlag  ||  curview != this->topLevel  
	    || curfocus != this->inputFocus) {

	    if(RECORDING) RecordCharacter(EOF);
	    return EOF;
	}
    }
}


static long WriteID = 1;

long im::GetWriteID()
    {
	ATKinit;

    return WriteID++;
}

void static PrintMallocStats(class im  *self, int  c)
        {
#if defined(ANDREW_MALLOC_ENV) && defined(DEBUG_MALLOC_ENV)
    FILE *outFile;
    char filename[500];
    char outstring[1000];
    
    sprintf(filename, "/tmp/mallocstats.%d", getpid());
    if ((outFile = fopen(filename, "a")) != NULL)  {
	MallocStats("be2 stats", outFile);
	fclose(outFile);
	sprintf(outstring, "Malloc statistics appended to %s", filename);
	message::DisplayString(self, 0, outstring);
    }
    else 
#endif /* #if defined(ANDREW_MALLOC_ENV) && defined(DEBUG_MALLOC_ENV) */
	message::DisplayString(self, 0, "could not write out malloc statistics");
}

void ResetMallocStats(class im  *self, long  c)
        {
#if defined(ANDREW_MALLOC_ENV) && defined(DEBUG_MALLOC_ENV)
    resetmstats();
    message::DisplayString(self, 0, "Reset malloc statistics");
#endif /* #if defined(ANDREW_MALLOC_ENV) && defined(DEBUG_MALLOC_ENV) */
}

void PrintMallocTable(class im  *self, long  c)
        {
#if defined(ANDREW_MALLOC_ENV) && defined(DEBUG_MALLOC_ENV)
    FILE *outFile;
    char filename[500];
    char outstring[1000];
    static long count = 0;
    
    sprintf(filename, "/tmp/malloctable.%d.%d", getpid(), ++count);
    if ((outFile = fopen(filename, "w")) != NULL)  {
	::plumber(outFile);
	fclose(outFile);
	sprintf(outstring, "Malloc table appended to %s", filename);
	message::DisplayString(self, 0, outstring);
    }
    else 
#endif /* #if defined(ANDREW_MALLOC_ENV) && defined(DEBUG_MALLOC_ENV) */
	message::DisplayString(self, 0, "Could not write out malloc table");
}

static void StartProfiling(class im  *self,long  c)
{
    message::DisplayString(self,0,"Couldn't start profiling!");
}

static void StopProfiling(class im  *self, long  c)
	{
	message::DisplayString(self,0,"Error stopping profiling!");
}

static long imkeyboardexit(ATK *i, long c)
{
    im::KeyboardExit();
    return 0;
}

static struct bind_Description imBindings[]={
	{"im-exit-program", "\030\003", 0, "Quit~99", 0, 0, imkeyboardexit, 
			"Exit program."},
			{"im-start-keyboard-macro", "\030(", 0, NULL, 0, 0, (proctable_fptr)StartKeyboardMacro , 
			"Start recording input events."},
	{"im-stop-keyboard-macro", "\030)", 0, NULL, 0, 0, (proctable_fptr)StopKeyboardMacro, 
			"Stop recording input events."},
	{"im-play-keyboard-macro", "\030e", 0, NULL, 0, 0, (proctable_fptr)PlayKeyboardMacro, 
			"Play current keyboard macro."},
	{"im-redraw-window", "\014", 0, NULL, 0, 0, (proctable_fptr)::RedrawWindow, 
			"Redraw the window."},

	{"exit", 0, 0, 0, 0, 0, imkeyboardexit, "Obsolete"},
	{"start-keyboard-macro", 0, 0, 0, 0, 0, (proctable_fptr)StartKeyboardMacro, "Obsolete"},
	{"stop-keyboard-macro", 0, 0, 0, 0, 0, (proctable_fptr)StopKeyboardMacro, "Obsolete"},
	{"play-keyboard-macro", 0, 0, 0, 0, 0, (proctable_fptr)PlayKeyboardMacro, "Obsolete"},
	{"redraw-window", 0, 0, 0, 0, 0, (proctable_fptr)::RedrawWindow, "Obsolete"},

	{"im-print-malloc-statistics", "\030\200m", 0, NULL, 0, 0, (proctable_fptr)PrintMallocStats,
			"Print malloc statisticsp to a file."},
	{"im-reset-malloc-statistics", "\030\200r", 0, NULL, 0, 0, (proctable_fptr)ResetMallocStats,
			"Reset malloc statistics."},
	{"im-print-malloc-table", "\030\200t", 0, NULL, 0, 0, (proctable_fptr)PrintMallocTable,
			"Print malloc table to a file."},
	{"im-start-profile", "\030\200p", 0, NULL, 0, 0, (proctable_fptr)StartProfiling,
			"Start profiling program."},
	{"im-stop-profile", "\030\200q", 0, NULL, 0, 0, (proctable_fptr)StopProfiling,
			"Stop profiling program."},
	NULL
};

/* This array is magic in that its first entry is the default. */
static struct wsinfo {
    char *keyName;
    char *windowSystemName;
} knownWindowSystems[] = {
#ifdef WM_ENV
    {"andrewwm", "wmws"},
#endif /* WM_ENV */
    {"x11", "xws"},
    {"pm", "pmws"}
};

static boolean grokSelections = FALSE;
static boolean xSelectionLossage = FALSE;
static boolean copyOnSelect = FALSE;

boolean im::InitializeClass()
    {

    const char *envString;
    boolean NotFound = TRUE;
    struct wsinfo *windowsys;
    char *wsName;

    InitGlobalStructure();
    childDied = FALSE;
    cleanUpZombies = TRUE;
    doRecord = FALSE;
    dontRecord = FALSE;
    recordingSuspensionLevel = 0;
    playingRecord = FALSE;
    eventCount = 0;
    thisCmd = 0;
    keyboardExitFlag = FALSE;
    keyboardLevel = 0;
    destroycount = 0;
    allZombieHandlers = NULL;
    allowCtrlUCmds = environ::GetProfileSwitch("CtrlUCmds", TRUE); 
    grokSelections = environ::GetProfileSwitch("XStyleSelections", FALSE);
    xSelectionLossage = environ::GetProfileSwitch("StrictXStyleSelections", grokSelections);
    copyOnSelect = environ::GetProfileSwitch("CopyOnSelect", grokSelections);
    
#ifdef HAVE_POSIX_SIGNALS
    {
	struct sigaction sa;
	sigset_t empty;

	sigemptyset(&empty);
	sa.sa_handler = SIGACTIONHANDLERFUNC(DeathInTheFamily);
	sa.sa_mask = empty;
	sa.sa_flags = 0;
	sigaction(SIGCHLD, &sa, NULL);
    }
#else
    signal(SIGCHLD, DeathInTheFamily);
#endif

    if ((envString = environ::Get("BE2WM")) != NULL) {
	for (windowsys = knownWindowSystems; (windowsys < knownWindowSystems + (sizeof(knownWindowSystems)/sizeof(knownWindowSystems[0]))) && (NotFound = (strcmp(envString, windowsys->keyName) != 0)); windowsys++);
	if (NotFound) {
	    windowsys = knownWindowSystems;
	    if (envString != NULL)
		printf("im_InitializeClass: Unknown window manager %s; assuming %s\n", envString, windowsys->keyName);
	}
	wsName = windowsys->windowSystemName;
    }
#ifdef WM_ENV
    else if ((envString = environ::Get("WMHOST")) != NULL) 
	wsName = "wmws";
#endif /* WM_ENV */
#ifdef X11_ENV
    else if ((envString = environ::Get("DISPLAY")) != NULL) 
	wsName = "xws";
#endif /* X11_ENV */
    else 
        wsName = knownWindowSystems[0].windowSystemName;

    currentWS = (class windowsystem *) ATK::NewObject(wsName);


    A_application = atom::Intern("application");
    ProgramNameAtom = A_application;

    globalUpdateList = new updatelist;

    globalKeymap = new keymap;

    imMenus = new menulist;
    
    bind::BindList(imBindings, globalKeymap, imMenus, &im_ATKregistry_ );

    stopmacroproc=proctable::Lookup("im-stop-keyboard-macro");
    
    return TRUE;
}

/*     The following supply cursor support.   */
/*
 NOTE The ClearCursors() Macro does different things in WM and X

In WM it delete the subrectangles associated with a cursor, but they will be rebuilt in im_UpdateCursor if the item remains in the im's cursorlist. Routines that remove cursors have to call this macro on any cursor that may be affected by the removal of a cursor that had been placed over it.

Under X it unmaps the inputonly window associated the cursor, since X handles these windows properly, only cursors that are really being removed are cleared.

*/

void im::ClearCursors(class cursor  * C )
{
    printf("im_ClearCursors: missing method\n");
}

void im::PostCursor(struct rectangle  *rec,class cursor  *cursor)
{
    printf("im_PostCursor: missing method\n");

}

void im::RetractCursor(register class cursor  *cursor)
{
    register class cursor *cp,*lastcp;
    if(cursor == NULL || cursor->posted == NULL) return;
    for(cp= this->cursorlist,lastcp = NULL; cp != NULL;cp = cp->next){
	if(cp == cursor){
	    cp->posted = NULL;
	    (this)->ClearCursors( cp);
	    if(lastcp)
		lastcp->next = cp->next;
	    else this->cursorlist= cp->next;
	}
	else lastcp = cp;
    }
    this->cursorPostsPending = TRUE;
    if(ProcessCursor == NULL && this->WindowCursor == NULL && this->inRedraw == FALSE)
	(this)->UpdateCursors();
}

void im::RetractViewCursors(class view  *requestor)
{
    /* clears cursors belonging to a view */
    register class cursor *cp,*lastcp;
    register int found = 0;
    for(cp= this->cursorlist,lastcp = NULL; cp != NULL;cp = cp->next){
	if(cp->view == requestor) {
	    found++;
	    cp->posted = NULL;
	    (this)->ClearCursors( cp);
	    if(lastcp)
		lastcp->next = cp->next;
	    else this->cursorlist= cp->next;
	}
	else  {
	    lastcp = cp;
	}
    }
    this->cursorPostsPending = TRUE;
    if(found && ProcessCursor == NULL && this->WindowCursor == NULL && this->inRedraw == FALSE)
	(this)->UpdateCursors();
}


void im::SetProcessCursor(class cursor  *cursor) /* set cursor to NULL to deactivate */
    {
	ATKinit;

    register class im *im;
    if(ProcessCursor == cursor){
	if(cursor == NULL || !cursor->changed)  return;
	cursor->changed = FALSE;
    }
    else{
	if(ProcessCursor)ProcessCursor->processC = FALSE;
	if(cursor) cursor->processC = TRUE;
        ProcessCursor = cursor;
    }
    for (im = imList; im != NULL ; im = im->next)
        (im)->UpdateCursors();
}

class cursor *im::GetProcessCursor()
    {
	ATKinit;

	return ProcessCursor;
}

void im::SetWindowCursor(class cursor  *cursor) /* set cursor to NULL to deactivate */
{
    if(this->WindowCursor == cursor){
	if(cursor == NULL || !cursor->changed)  return;
	cursor->changed = FALSE;
    }
    else{
	if(this->WindowCursor)this->WindowCursor->windowim = NULL;
	if(cursor) cursor->windowim = this;
        this->WindowCursor = cursor;
    }
    (this)->UpdateCursors();
}

void im::ClearCursorList()
    {
    register class cursor *cp;

    if (im::IsPlaying()) return;

    for(cp= this->cursorlist; cp != NULL;cp = cp->next){
        cp->posted = NULL;
	(this)->ClearCursors( cp);
        }
    this->cursorlist = NULL;
    this->cursorPostsPending = TRUE;
    }

void im::UpdateCursors()
{
    printf("im_UpdateCursors: missing method\n");
}




void im::SetTitle(const char  *title)
        {
    char *slash;
/* Hack to display OS/2 file names with backslashes */
#ifdef DOS_STYLE_PATHNAMES
    while ((slash = strchr(title, '/')) != NULL)
	*slash = '\\';
#endif

    if (this->title != NULL)  {
	free(this->title);
	this->title = NULL;
    }
    if (title != NULL)  {
	this->title = (char *) malloc(strlen(title) + 1);
	strcpy(this->title, title);
    }
    if (this->LogFile != NULL) WriteLogEntry(this, log_TITLE, title);
}

char *im::GetTitle()
    {
    return this->title;
}

void im::SetProgramName(const char  *name)
        {
	ATKinit;

    const char *shudder;
    if (initialProgramName != NULL)  {
	free(initialProgramName);
	initialProgramName = NULL;
    }
    if (name != NULL)  {
	initialProgramName = (char *) malloc(strlen(name) + 1);
	strcpy(initialProgramName, name);
	/* Used for reading the preferences file */
	environ::SetProgramName(name);

	/* the following 3 lines are gross and awful because app.c also checks this
		preference option and because this may override a SetGeometrySpec
		deliberatly set by the program.  */
	shudder = environ::GetProfile("Geometry");
	if (shudder != NULL)
	    im::SetGeometrySpec(shudder);
    }

    ProgramNameAtom = atom::Intern(initialProgramName);
}

char *im::GetProgramName()
    {
	ATKinit;

    return initialProgramName;
}

void im::SetGlobalInit(class init  *init)
        {
	ATKinit;


    globalInit = init;
}

class init *im::GetGlobalInit()
    {
	ATKinit;


    return globalInit;
}

/* We really ought to tell the window manager about this change in preferences.
    This should be a method instead of a class procedure. */
void im::SetPreferedDimensions(long  top , long  left , long  width , long  height)
        {
	ATKinit;


    preferedTop = top;
    preferedLeft = left;
    preferedWidth = width;
    preferedHeight = height;
    setDimensions = TRUE;
}

void im::GetPreferedDimensions(long  *top , long  *left , long  *width , long  *height)
        {
	ATKinit;


    *top = preferedTop;
    *left = preferedLeft;
    *width = preferedWidth;
    *height = preferedHeight;
}

/* We really ought to tell the window manager about this change in preferences.
    This should be a method instead of a class procedure. */
	void 
im::SetGeometrySpec(const char  *value)
		{
	ATKinit;

	char *buffer;
	buffer = (char *)malloc(strlen(value) + 2);
	if (*value != '=') {
		*buffer = '=';
		strcpy(buffer+1, value);
	}
	else
		strcpy(buffer, value);
	if (geometrySpec != NULL)  
		free(geometrySpec);
	geometrySpec = buffer;
}


FILE *im::FromCutBuffer()
    {
    printf("im_FromCutBuffer: missing method\n");
    return NULL;
}

FILE *im::OnlyFromCutBuffer()
{
    printf("im::OnlyFromCutBuffer: missing method\n");
    return NULL;
}

FILE *im::OnlyFromSelection()
{
    printf("im::OnlyFromSelection: missing method\n");
    return NULL;
}

FILE *im::ToCutBuffer()
    {
    FILE *cutFile;

    writeCutBuffer.pos = 0;
    writeCutBuffer.length = 0;
    cutFile = (FILE *) im::vfileopen("w",0);
    return cutFile;
}

void im::CloseFromCutBuffer(FILE  *readFile)
        {
    im::vfileclose(readFile, 0);
}

void im::CloseToCutBuffer(FILE  *writeFile)
        {
    printf("im_CloseToCutBuffer: missing method\n");

}

void im::RotateCutBuffers(long  count)
        {
    printf("im_RotateCutBuffers: missing method\n");

}

void im::AppendToCutBuffer(FILE  *writeFile)
        {
    printf("im_AppendToCutBuffer: missing method\n");

}

void im::SetWMFocus()
    {
    printf("im_SetWMFocus: missing method\n");

}

void im::ExposeWindow()
    {
    printf("im_ExposeWindow: missing method\n");
}

	void
im::HideWindow()
	{
	printf("im_HideWindow: missing method\n");
}

	void
im::VanishWindow()
	{
	printf("im_VanishWindow: missing method\n"); 
}

class windowsystem *im::GetWindowSystem()
    {
	ATKinit;


    return currentWS;
}

class cursor * im::GetCursor()
    {
	ATKinit;

    return (im::GetWindowSystem())->CreateCursor();
}

class fontdesc * im::GetFontdesc()
    {
	ATKinit;

    return (im::GetWindowSystem())->CreateFontdesc();
}

class graphic * im::GetGraphic()
    {
	ATKinit;

    return (im::GetWindowSystem())->CreateGraphic();
}

short im::GetResource( class atomlist  * name, class atomlist  * class_c, class atom  * type, long  * data )
                         {
  struct atoms * nameMark = (name)->Mark();
  struct atoms * classMark = (name)->Mark();
  short found;

  (name)->JoinToBeginning(  this->name );
  (class_c)->JoinToBeginning(  this->className );
  (name)->Prepend(  ProgramNameAtom );
  (class_c)->Prepend(  A_application );
  found = rm::GetResource( name, class_c, type, data );
  (name)->Cut(  nameMark );
  (class_c)->Cut(  classMark );
  return found;
}


void im::PostResource( class atomlist  * path, class atom  * type, long  data )
                    {
  struct atoms * pathMark = (path)->Mark();

  (path)->JoinToBeginning(  this->name );
  (path)->Prepend(  ProgramNameAtom );
  rm::PostResource( path, data, type );
  (path)->Cut(  pathMark );
}


void im::GetManyParameters(struct resourceList  * resources, class atomlist  * name, class atomlist  * class_c)
                    {
  struct atoms * nameMark = NULL;
  struct atoms * classMark = NULL;
  class atomlist * passname;
  class atomlist * passclass;

  if (name == NULL)
    passname = this->name;
  else
    {
      nameMark = (name)->Mark();
      (name)->JoinToBeginning(this->name);
      passname = name;
    }

  if (class_c == NULL)
    passclass = this->className;
  else
    {
      classMark = (class_c)->Mark();
      (class_c)->JoinToBeginning(this->className);
      passclass = class_c;
    }

  (passname)->Prepend(  ProgramNameAtom );
  (passclass)->Prepend(  A_application );
  rm::GetManyResources( resources, passname, passclass );
  (passname)->DropFirst();
  (passclass)->DropFirst();
  
  if (name != NULL)
    (name)->Cut(nameMark);
  if (class_c != NULL)
    (class_c)->Cut(classMark);
}

void im::UnlinkNotification(class view  *unlinkedTree)
        {

    class cursor *thisCursor;

/* Input focus. */
    if (this->inputFocus != NULL && (this->inputFocus)->IsAncestor( unlinkedTree)) {
        (this->inputFocus)->LoseInputFocus();
	this->inputFocus = NULL;
	if(unlinkedTree->parent && unlinkedTree->parent!=(class view *)this) (unlinkedTree->parent)->WantInputFocus( unlinkedTree->parent);
       /* im_PostKeyState(self, NULL);
        im_PostMenus(self, NULL); */
    }

/* Mouse focus */
    if (this->mouseFocus != NULL && (this->mouseFocus)->IsAncestor( unlinkedTree)) {
        this->mouseFocus = NULL;
        this->buttonState = im_AllUp;
        this->lastX = -1;
        this->lastY = -1;
        this->lastMouseDown = view_NoMouseEvent;
        this->clickCount = 1;
    }

 /* Cursors */
    for (thisCursor = this->cursorlist; thisCursor != NULL; thisCursor = thisCursor->next)
        if ((thisCursor->view)->IsAncestor( unlinkedTree))
            (this)->RetractCursor( thisCursor);

/* Pending updates. */
    (globalUpdateList)->DeleteTree( unlinkedTree);

    (this)->view::UnlinkNotification( unlinkedTree);
}

     boolean
im::CreateOffscreenWindow(class im  *other, int  width , int  height)
{
    printf("im_CreateOffscreenWindow: missing method\n");
    return FALSE;
}

boolean im::SupportsOffscreen()
{
    return FALSE;
}

struct rectangle *im::GetLoc(class view  *view, struct rectangle  *rect)
{
    printf("im_GetLoc: missing method\n");
    return NULL;
}

char **im::GetDroppedFiles()
	{
    return NULL;
}

void im::DropFile(char  *pathname, class cursor  *cursor)
			{
}

void im::DropFiles(char  **pathnames, class cursor  *cursor)
			{
}


void im::SuspendRecording()
{
	ATKinit;

    struct action *a;
    if(doRecord) {
	a=newAction();
	a->im=NULL;
	a->type=im_SuspendEvent;
	enqAction(&Record, a);
    }
    recordingSuspensionLevel++;
}

void im::RecordAnswer(const char  *answer)
{
	ATKinit;

    struct action *a;
    if(!doRecord) return;
    a=newAction();
    if(a!=NULL) {
	a->im=NULL;
	a->type=im_AnswerEvent;
	if(answer) a->v.answer=(char *)malloc(strlen(answer)+1);
	else a->v.answer=NULL;
	if(a->v.answer!=NULL) strcpy(a->v.answer, answer);
	enqAction(&Record, a);
    }
}

void im::RecordCancellation()
{
	ATKinit;

    im::RecordAnswer(NULL);
}

static struct action *AnswerQueue=NULL;
static struct action *pendingAnswerFree=NULL;

static boolean wasCancel;

char *im::GetAnswer()
{
	ATKinit;

    wasCancel=FALSE;
    if(pendingAnswerFree!=NULL) {
	freeQelt(pendingAnswerFree);
	pendingAnswerFree=NULL;
    }
   /* We want playing keyboard macros to be just like
     before.
    if(playingRecord && InQ->type==im_MacroEvent) {
	struct action *a, *b;
	a = InQ->v.macro.nextaction;
	while(a) {
	    if(a->type==im_AnswerEvent || a->type==im_ResumeEvent) break;
	    a=a->next;
	}
	if(a) {
	    while(a!=PeekInputEvent()) {
		GetNextInputEvent();
	    }
	    a=GetNextInputEvent();
	    if((b=PeekInputEvent()) && b->type==im_ResumeEvent) GetNextInputEvent();
	    if(a->type==im_AnswerEvent) {
		if(a->v.answer==NULL) wasCancel=TRUE;
		return a->v.answer;
	    }
	}
    } */
    if(AnswerQueue!=NULL) {
	pendingAnswerFree=AnswerQueue;
	AnswerQueue=AnswerQueue->next;
	if(pendingAnswerFree->v.answer==NULL) wasCancel=TRUE;
	return pendingAnswerFree->v.answer;
    }
    return NULL;
}

boolean im::AnswerWasCancel()
{
	ATKinit;

    return wasCancel;
}

void im::ResumeRecording()
{
	ATKinit;

    if(doRecord && recordingSuspensionLevel>0) {
	struct action *a;
	a=newAction();
	if(a!=NULL) {
	    a->im=NULL;
	    a->type=im_ResumeEvent;
	    enqAction(&Record, a);
	}
    }
    if(recordingSuspensionLevel>0) recordingSuspensionLevel--;
}

void im::QueueAnswer(char  *answer)
{
	ATKinit;

    struct action *a;
    a=newAction();
    if(a!=NULL) {
	a->im=NULL;
	a->type=im_AnswerEvent;
	if(answer!=NULL) a->v.answer=(char *)malloc(strlen(answer)+1);
	else a->v.answer=NULL;
	if(a->v.answer!=NULL) strcpy(a->v.answer, answer);
	enqAction(&AnswerQueue, a);
    }
}

void im::QueueCancellation()
{
	ATKinit;

    im::QueueAnswer(NULL);
}

/* isString(arg)
  tests arg to see if it is a string
      returns TRUE if it is and FALSE otherwise
	  */
static jmp_buf trap;

static SIGNAL_RETURN_TYPE SigHandler(int sig)
{
    longjmp(trap, 1);
}

static boolean
isString(char  *arg)
{
    SIGNAL_RETURN_TYPE (*oldBus)(int sig), (*oldSeg)(int sig); /* save signal handlers */
    char c;
    int badflag=0;

    /* first test to see if it is a character instead of a pointer */
    if ((long)arg < (1<<16) && (long)arg > -(1<<16))  
	return FALSE;	/* too small an integer to be a pointer (in ATK) */

    /* now be sure we can fetch from the location */
#ifdef SIGBUS
    oldBus = signal(SIGBUS, SigHandler);
#endif
    oldSeg = signal(SIGSEGV, SigHandler);
    if (setjmp(trap) != 0) 
	/* return here from longjmp */
	badflag=1;	/* not a legal string */
    else 
	/* normal setjmp return location */
	c = *arg;		/* this could fail if arg were not a pointer */
#ifdef SIGBUS
    signal(SIGBUS, oldBus);
#endif
    signal(SIGSEGV, oldSeg);

    /* return value depending on whether it points to a legal address */
    return !badflag;
}

static char keybinding[100];
static char keysbuf[100];
static int pos=0;
static class keymap **kmv=NULL;
int kmvc=0;

#define ROCKSEQUAL(rock, rstring, rock2) ((rock==rock2) || (rstring && isString((char*)rock2) && strcmp((char*)rock,(char*)rock2)==0))

static boolean VerifyBinding(class im  *self, char  *keysp, int  keyslen, ATK   *obj, struct proctable_Entry  *pe, long  rock, boolean  rstring)
{
    class keystate *ks;
    boolean answer=FALSE;
    boolean foundproc=FALSE;
    int ksc, i, j;

    for(ks=self->keystate,ksc=0;ks;ks=ks->next,ksc++);

    if(ksc>kmvc) {
	kmvc=ksc;
	if(kmv==NULL) kmv=(class keymap **)malloc(sizeof(class keymap *)*ksc);
	else kmv=(class keymap **)realloc(kmv, sizeof(class keymap *)*ksc);
    }

    
    if(!kmv) return FALSE;

    for(i=0,ks=self->keystate;i<ksc;i++,ks=ks->next) kmv[i]=ks->orgMap;
    
    for(j=0;j<keyslen;j++) {
	boolean foundMap;

	foundMap=FALSE;
	
	for(i=0;i<ksc;i++) {
	    struct proctable_Entry *rpe;
	    long rrock;
	    enum keymap_Types result;

	    if(kmv[i]) {
		result=(kmv[i])->Lookup( keysp[j], (ATK **)kmv+i, &rrock);

		switch(result) {
		    case keymap_Empty:
			kmv[i]=NULL;
			break;
		    case keymap_Keymap:
			foundMap=TRUE;
			break;
		    case keymap_Proc:
			if(!foundMap && !foundproc) {
			    foundproc=TRUE;

			    rpe=(struct proctable_Entry *)kmv[i];

			    answer = (pe==rpe && ROCKSEQUAL(rock, rstring, rrock));
			    if(!answer) return answer;
			}
			kmv[i]=NULL;
			break;
		    default:
			kmv[i]=NULL;
		}

	    }
	}
    }
    return answer;
}

static char *GetKeyBinding(class im  *self, class keymap  *km, ATK   *obj, struct proctable_Entry  *pe, long  rock, boolean  rstring)
{
    int i;
    int ind=strlen(keybinding);
    int lpos=pos;
    
    if(ind>sizeof(keybinding)-6) return NULL;
    if(km->sparsep) {
	struct keymap_sparsetable *kms=km->table.sparse;
	for(i=0;i<kms->numValid;i++) {
	    if(kms->types[i]==keymap_Proc) {
		if((pe==((struct proctable_Entry *)kms->objects[i])) && ROCKSEQUAL(rock, rstring, kms->rocks[i])) {
		    strcpy(keybinding+ind, charToPrintable(kms->keys[i]));
		    keysbuf[pos]=kms->keys[i];
		    if(VerifyBinding(self, keysbuf, pos+1, obj, pe, rock, rstring)) return keybinding;
		    else {
			pos=lpos;
			keybinding[ind]='\0';
			return NULL;
		    }
		}
	    } else if(kms->types[i]==keymap_Keymap) {
		strcpy(keybinding+ind, charToPrintable(kms->keys[i]));
		keysbuf[pos]=kms->keys[i];
		pos++;
		if(::GetKeyBinding(self, (class keymap*) kms->objects[i], obj, pe, rock, rstring)) {
		    return keybinding;
		}
		pos=lpos;
		keybinding[ind]='\0';
	    }
	}
    } else {
	struct keymap_fulltable *kmf=km->table.full;
	for(i=0;i<keymap_MAXKEYS;i++) {
	    if(kmf->types[i]==keymap_Proc) {
		if((pe==((struct proctable_Entry *)kmf->objects[i])) &&	ROCKSEQUAL(rock, rstring, kmf->rocks[i])) {
		    strcpy(keybinding+ind, charToPrintable(i));
		    keysbuf[pos]=i;
		    if(VerifyBinding(self, keysbuf, pos+1, obj, pe, rock, rstring)) return keybinding;
		    else {
			pos=lpos;
			keybinding[ind]='\0';
			return NULL;
		    }
		}
	    } else if(kmf->types[i]==keymap_Keymap) {
		strcpy(keybinding+ind, charToPrintable(i));
		keysbuf[pos]=i;
		pos++;
		if(::GetKeyBinding(self, (class keymap*) kmf->objects[i], obj, pe, rock, rstring)) {
		    return keybinding;
		}
		pos=lpos;
		keybinding[ind]='\0';
	    }
	}
    }		    
    return NULL;
}

char *im::GetKeyBinding(ATK   *obj, struct proctable_Entry  *pe, long  rock)
{
    class keystate *ks=this->keystate;
    boolean rstring=isString((char*)rock);

    while(ks) {
	if(ks->orgMap) {
	    memset(keybinding, 0, sizeof(keybinding));
	    pos=0;
	    ::GetKeyBinding(this, ks->orgMap, obj, pe, rock, rstring);
	    if(keybinding[0]) return keybinding;
	}
	ks=ks->next;
    }
    return NULL;
}

boolean im::RequestSelectionOwnership(class view  *requestor)
{
    if(!grokSelections && requestor!=this) {
	if(requestor) return FALSE;
	else {
	    ownerIM=NULL;
	    selectionOwner=NULL;
	    return TRUE;
	}
    }

    if(selectionOwner==requestor) return TRUE;
    
    if(selectionOwner && xSelectionLossage) {
	/* if the requestor is the im for the view currently holding the selection don't tell it it doesn't
	 have the selection anymore, just yet... */
	if(requestor && requestor->IsType(&im_ATKregistry_) && selectionOwner && selectionOwner->IsAncestor(requestor)) {
	} else (selectionOwner)->LoseSelectionOwnership();
    }
    if(selectionOwner) (selectionOwner)->RemoveObserver( this);
    if(requestor  && requestor!=this) {
	selectionOwner=requestor;
	ownerIM=(requestor)->GetIM();
	(requestor)->AddObserver( this);
	if(copyOnSelect) {
	    FILE *cf;
	    cf=this->ToCutBuffer();
	    if(cf!=NULL) {
		selectionOwner->WriteSelection(cf);
		this->CloseToCutBuffer(cf);
	    }
	}
    } else {
	selectionOwner=requestor;
	ownerIM=(struct im *)requestor;
    }
	
    return TRUE;
}

class view *im::GetSelectionOwner()
{
	ATKinit;

    return selectionOwner;
}


void im::GiveUpSelectionOwnership(class view  *requestor)
{
    /* This deliberately does NOT call LoseSelectionOwnership on the requestor, it is assumed that the requestor will have taken the appropriate action. */
    if(selectionOwner==requestor) {
	selectionOwner=NULL;
    }
}
/* Functions that support window manager delete window requests. */
im_deletefptr im::GetDeleteWindowCallback()
{
    return this->delete_window_cb;
}
long im::GetDeleteWindowCallbackRock()
{
    return this->delete_window_rock;
}
void im::SetDeleteWindowCallback(im_deletefptr  p, long  rock)
{
    this->delete_window_cb = p;
    this->delete_window_rock = rock;
}
void im::CallDeleteWindowCallback()
{
    if (this->delete_window_cb)
	this->delete_window_cb(this, this->delete_window_rock);
}

class colormap *
im::CreateColormap( )
{
    printf("im_CreateColormap should be overriden.\n");
    return(NULL);
}

class color *
im::CreateColor( char  *name, unsigned int r , unsigned int g , unsigned int b )
            {
    printf("im_CreateColor should be overriden.\n");
    return(NULL);
}

void
im::InstallColormap( class colormap  *cmap )
{
    this->installedColormap = cmap;
}

void
im::ReceiveColormap( class colormap  *cmap )
        {
    (this)->view::ReceiveColormap( cmap);
    (this->topLevel)->LinkTree( this);
}

boolean
im::ResizeWindow( int  w , int  h )
        {
    return(FALSE);
}

boolean
im::MoveWindow( int  x , int  y )
        {
    return(FALSE);
}

boolean im::RecSearch(struct SearchPattern *pat, boolean toplevel)
{
    if (this->topLevel)
	return this->topLevel->RecSearch(pat, toplevel);
    return FALSE;
}

boolean im::RecSrchResume(struct SearchPattern *pat)
{
    if (this->topLevel) return this->topLevel->RecSrchResume(pat);
    return FALSE;
}

boolean im::RecSrchReplace(class dataobject *text, long pos, long len)
{
    if (this->topLevel)
	return this->topLevel->RecSrchReplace(text, pos, len);
    return FALSE;
}

void im::RecSrchExpose(const struct rectangle &logical, struct rectangle &hit)
{
    if (this->topLevel) {
	struct rectangle logicalim;
	GetLogicalBounds(&logicalim);
	this->topLevel->RecSrchExpose(logicalim, hit);
    }
}

void im::Beep(int volume)
{
    // should be overridden
    // Volume is 0..100 and is a percentage of volume of the user's default bell.
    // A preference could allow the im to "flash" instead of beep.
}
