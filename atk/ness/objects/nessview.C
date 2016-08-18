/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
	Copyright Carnegie Mellon University 1993 - All Rights Reserved
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

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/ness/objects/RCS/nessview.C,v 1.11 1995/03/01 01:51:33 rr2b Stab74 $";
#endif


/*
 *   $Log: nessview.C,v $
 * Revision 1.11  1995/03/01  01:51:33  rr2b
 * Fixed scrollbar.
 * BUG
 *
 * Revision 1.10  1994/12/10  04:44:48  rr2b
 * Fixed coredump when the scroll destructor tries to unlink its child
 * and finds it already destroyed.
 * BUG
 *
 * Revision 1.9  1994/11/30  20:42:06  rr2b
 * Start of Imakefile cleanup and pragma implementation/interface hack for g++
 *
 * Revision 1.8  1994/08/12  20:43:31  rr2b
 * The great gcc-2.6.0 new fiasco, new class foo -> new foo
 *
 * Revision 1.7  1994/03/21  17:00:38  rr2b
 * bzero->memset
 * bcopy->memmove
 * index->strchr
 * rindex->strrchr
 * some mktemp->tmpnam
 *
 * Revision 1.6  1994/03/12  22:42:52  rr2b
 * Removed second argument to Stringize, an empty string may not be stringized
 *
 * Revision 1.5  1993/10/20  17:00:08  rr2b
 * Used Stringize and concat macros.
 *
 * Revision 1.4  1993/09/28  04:52:23  rr2b
 * Fixed view_DSattributes to be an int not an enum.
 *
 * Revision 1.3  1993/07/23  00:23:42  rr2b
 * Split off a version of CopyText which will copy surrounding
 * styles as well as embedded styles.
 *
 * Revision 1.2  1993/06/29  18:06:33  wjh
 * checkin to force update
 *
 * Revision 1.1  1993/06/21  20:43:31  wjh
 * Initial revision
 *
 * Revision 1.21  1993/01/08  16:33:17  rr2b
 * cutting down on duplicate global symbols
 *
 * Revision 1.20  1992/12/15  21:38:20  rr2b
 * more disclaimerization fixing
 *
 * Revision 1.19  1992/12/14  20:50:00  rr2b
 * disclaimerization
 *
 * Revision 1.18  1992/11/26  02:38:01  wjh
 * converted CorrectGetChar to GetUnsignedChar
 * moved ExtendShortSign to interp.h
 * remove xgetchar.h; use simpletext_GetUnsignedChar
 * nessrun timing messages go to stderr
 * replaced curNess with curComp
 * replaced genPush/genPop with struct compilation
 * created compile.c to handle compilation
 * moved scope routines to compile.c
 * converted from lex to tlex
 * convert to use lexan_ParseNumber
 * truncated logs to 1992 only
 * use bison and gentlex instead of yacc and lexdef/lex
 *
 * .
 *
 * Revision 1.17  92/06/05  16:39:31  rr2b
 * added interface for visiting a named script, for creating
 * a ness script from a keyboard macro.
 * 
log elided 6/92 -wjh

*/


ATK_IMPL("nessview.H")

#include <dataobject.H>
#include <menulist.H>
#include <keymap.H>
#include <keystate.H>
#include <mark.H>
#include <bind.H>
#include <im.H>
#include <event.H>
#include <view.H>
#include <graphic.H>
#include <rect.h>
#include <physical.h>
#include <fontdesc.H>
#include <style.H>
#include <buffer.H>
#include <completion.H>
#include <proctable.H>
#include <text.H>
#include <frame.H>
#include <framemessage.H>
#include <message.H>
#include <arbiterview.H>
#include <cursor.H>
#include <environ.H>

/* include headers for the data object and this view */
#include <nessview.H>
#include <ness.H>
#include <textview.H>


static boolean  debug;      /* This debug switch is toggled with ESC-^D-D */
#define DEBUG(s) {if (debug) {printf s ; fflush(stdout);}}
#define ENTER(r) DEBUG(("Enter %s\n", Stringize(r)))
#define LEAVE(r) DEBUG(("Leave %s\n", Stringize(r)))


#define	MenuOff		(0)
#define	MenuOrigin	(1<<0)
#define	MenuScan	(1<<1)
#define	MenuErrors	(1<<2)
#define	MenuAuthor	(1<<3)
#define	MenuMain	(1<<4)
#define	MenuEmpower	(1<<5)
#define	MenuUser	(1<<6)
#define	MenuDanger	(1<<7)

	
ATKdefineRegistry(nessview, scroll, nessview::InitializeClass);

static void  PostMenus(class nessview  *self);
static void setExecFunc(class nessview  *self, char  *funcname);
static void nessview_NextError(register class nessview  *self, long  rock);
static void nessview_Compile(register class nessview  *self, ness_access  accesslevel);
static void nessview_Execute(register class nessview  *self, long  rock);
static void ShowOrigin(class nessview  *self);
static void DeauthButton(class nessview  *self);
static void AuthorButton(class nessview  *self, long  option);
static void ScanButton(class nessview  *self);
static void CompileButton(class nessview  *self);
static void CompileMenu(class nessview  *self);
static void ToggleDebug(register class nessview  *self, long  rock);
static void WaitOn();
static void WaitOff();
static boolean DoAppend(class nessview  *self, class ness  *src, class ness  *dest);
static void UpdateFile(class nessview  *self, class ness  *src, class buffer  *b, char  *filename);
static char *GetName(class ness  *n);
static long match(class ness  *n,struct helpRock  *h);
static void helpProc(char  *partial, long lrock,message_workfptr  HelpWork,long  rock);
static enum message_CompletionCode mycomplete(char  *partial, struct helpRock  *myrock, char  *buffer, int  bufferSize);
static long AskForScript(class view  *self, class ness  *current, char  *prompt, char  *buf, long  bufsiz);
static void ScriptAppend(class nessview  *self, long  rock);
static void Append(class nessview  *self, long  rock);
static void Visit(class view  *self, long  rock);
static void DisplayDialogBox(class nessview  *self, long  time  /* ignored */);
static	void Invert(class nessview  *self);


	static void 
PostMenus(class nessview  *self) {
	class ness *dobj = (class ness *)self->dataobject;
	long menumask;
	DEBUG(("Enter PostMenus(%d)\n", self->HasIF));
	if (dobj->hasWarningText) menumask = MenuOrigin;
	else {
		menumask = MenuOrigin | MenuScan | MenuUser;
		if ((dobj)->GetReadOnly()) menumask |= MenuAuthor;
		if ( ! dobj->compiled  || dobj->accesslevel < ness_codeGreen
				|| self->compmod != (dobj)->GetModified())
			menumask |= MenuEmpower;
	}
	if (dobj->ErrorList) menumask |= (self->justscanned) ? MenuDanger : MenuErrors;
	if (dobj->accesslevel >= ness_codeGreen) menumask |= MenuMain;
	(self->Menus)->SetMask( menumask);
	DEBUG(("PostMenus mask = 0x%lx\n", menumask));
	if ( ! self->HasIF) 
		return;
	(self)->PostMenus(self->Menus);
	DEBUG(("Leave PostMenus\n"));
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
 *	User Interface 
 *	
 *	Routines called from keystrokes or menu
 *
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static char *defaultExecFunc = "main";

	static void
setExecFunc(class nessview  *self, char  *funcname) {
	if (self->ExecFunction != defaultExecFunc)
		free(self->ExecFunction);
	self->ExecFunction = (funcname == NULL || *funcname == '\0')
				? defaultExecFunc : freeze(funcname);
}


	static void
nessview_NextError(register class nessview  *self, long  rock) {
	register class ness *dobj = (class ness *)self->dataobject;
	class im *im = (self)->GetIM();

	if  (im && (im)->ArgProvided()) 
		self->CurrError = (dobj)->PreviousError( self->CurrError);
	else
		self->CurrError = (dobj)->NextError( self->CurrError);
	
	if (self->CurrError == NULL) {
		(self->SourceText)->CollapseDot();
		message::DisplayString(self, 0, "No more errors");
	}
	else {
		/* select and view the error */
		(self->SourceText)->SetDotPosition(
			((self->CurrError)->where)->GetPos());
		(self->SourceText)->SetDotLength(
			((self->CurrError)->where)->GetLength());
		(self->SourceText)->FrameDot(
			((self->CurrError)->where)->GetPos());

		message::DisplayString(self, 0, (self->CurrError)->msg+1);
	}
}

/* FirstError(struct nessview *self)
	display message and highlight for first error 
*/
	void
nessview::FirstError() {
	if (((class ness *)this->dataobject)->GetErrors() == NULL)
		return;

	if((this)->GetDataObject()) ((class ness *)(this)->GetDataObject())->errorpending=FALSE;
		
	if ( ! this->HasIF) {
		this->ErrorHighlightPending = TRUE;
		(this)->WantInputFocus( this);
	}
	else {
		this->ErrorHighlightPending = FALSE;
		this->CurrError = NULL;
		nessview_NextError(this, 0);	
	}
}


	static void
nessview_Compile(register class nessview  *self, ness_access  accesslevel) {
	register class ness *dobj = (class ness *)self->dataobject;
	struct errornode *errs;

	(dobj)->SetAccessLevel( accesslevel);

	if (dobj->PromptBeforeCompile && accesslevel >= ness_codeGreen) {
		/* give a dialog box to verify the compile choice */
		long choice;
		static char *choices[] = {
			"Cancel - Keep Ness script inactive", 
			"Empower - Let script do its thing", 
			NULL
		};
		if (message::MultipleChoiceQuestion(NULL, 50, 
			"Do you really want to empower this Ness script?",
				0, &choice, choices, "ce") < 0 
				|| choice != 1) 
			return;
		dobj->PromptBeforeCompile = FALSE;
	}
	self->CurrError = NULL;
	(dobj)->EstablishViews( self);
	dobj->CurrentInset = NULL;

	errs =  (dobj)->Compile();
	self->compmod = (dobj)->GetModified();

	if (errs == NULL) 
		message::DisplayString(self, 0, "Done");
	else 
		(self)->FirstError();
	self->justscanned = (accesslevel < ness_codeGreen);

	/* need to wantinputfocus, since Menu AND keybindings may have changed. */
	(self)->WantInputFocus( self);
	/* PostMenus(self); */
}

	static void
nessview_Execute(register class nessview  *self, long  rock) {
	register class ness *dobj = (class ness *)self->dataobject;
	char buffer[1000];
	struct errornode *errs = NULL;

	self->CurrError = NULL;
	dobj->ToldUser = FALSE;

	if (((self)->GetIM())->ArgProvided()) {
		/* prompt for name of function to execute */
		if (message::AskForString(self, 0, "Execute function: ", 
				self->ExecFunction, buffer, sizeof(buffer)) != -1)
			setExecFunc(self, buffer);
		else
			return;
	}

	if ( ! dobj->compiled) {
		nessview_Compile(self, dobj->accesslevel);
		errs = dobj->ErrorList;
		if ( ! dobj->compiled  && errs == NULL) {
			/* user probably refused permission */
			message::DisplayString(self, 0, "Not done");
			return;
		}
	}
	(dobj)->EstablishViews( self);
	dobj->CurrentInset = NULL;

	if (errs == NULL) {
		/* if the function is defined, execute it.  
			If not and the function is the default
				and there are no extended objects, give error */
		/* sigh XXX this code is a lot like ness_Execute */
		boolean hasxobj;
		class nesssym *funcsym;
		hasxobj = FALSE;

		for (funcsym = dobj->globals; funcsym != NULL; 
				funcsym = funcsym->next)
			if (funcsym->flags == flag_xobj)
				hasxobj = TRUE;
			else if (strcmp((char *)funcsym->name, self->ExecFunction) == 0) 
				break;
		if (funcsym != NULL)
			errs = (dobj)->Execute( self->ExecFunction);
		else if (self->ExecFunction != defaultExecFunc || ! hasxobj) {
			char buf[200];
			sprintf(buf, "Couldn't find %s\n", self->ExecFunction);
			message::DisplayString(self, 0, buf);
			::PostMenus(self);	
			return;
		}
	}

	if (errs == NULL) {
		if ( ! dobj->ToldUser)
			message::DisplayString(self, 0, "Done");
	}
	else 
		(self)->FirstError();
	::PostMenus(self);
}

	static void
ShowOrigin(class nessview  *self) {
	class ness *dobj = (class ness *)self->dataobject;
	char buf[500];
	char *date, *author;

	(dobj)->GetOriginData( &date, &author);
	sprintf(buf, "Probably last modified by %s on %s", author, date);
	free(date);
	free(author);
	message::DisplayString(self, 0, buf);
}

/* DeauthButton(self)
	processes a click on the Deauthentication button
	prompts the user to see if s/he really means it
	if yes, deauthenticate this process
*/
	static void
DeauthButton(class nessview  *self) {
	/* XXX give dialog box to see if he really wants to */
	/* when this is implemented it should deauthenticate first 
		and then Empower-compile */

	message::DisplayString(self, 0, "Deauthentication not implemented");
}

/* AuthorButton(self, option)
	processes ESC-~, a click on the Author button, or one of the menu items
		Author Mode   and  Add Warning
	option 0 - remove warning text;    1 - add warning text
		-1  -  toggle read-only and make warning match
	prompts the user to see if s/he really means it
	if yes, change to author mode
*/
	static void
AuthorButton(class nessview  *self, long  option) {
	class ness *dobj = (class ness *)self->dataobject;

	(self)->WantInputFocus( self);
	if (dobj->hasWarningText  && 
			(((dobj)->GetReadOnly()  && option != 1)
			|| ( ! (dobj)->GetReadOnly()  && option == 0)))  {
		/*  In 3 of the 12 cases we remove the warning */
		/* prompt first to make sure the user wants to */
		long choice;
		static char *choices[] = {
			"Cancel - Leave script read-only", 
			"Author mode - Make script read-write", 
			NULL
		};
		if (message::MultipleChoiceQuestion(NULL, 50, 
			"Do you want to be able to modify this Ness script?",
				0, &choice, choices, "ca") < 0 
				|| choice != 1) 
			return;

		/* ok, go ahead */
		(self)->PostMenus( NULL);	/* remove menus (HACK TO GET AROUND BUG */
		(dobj)->RemoveWarningText();
		(self->SourceText)->SetDotPosition( 0);
		(self->SourceText)->SetDotLength( 0);
		(self->SourceText)->FrameDot( 0);
   		(self->SourceText)->WantUpdate( self->SourceText);
		(dobj)->NotifyObservers( ness_WARNINGTEXTCHANGED);
	}

	/* set read-onliness */
	(dobj)->SetReadOnly( 
			(option==0) ? FALSE : (option==1) ? TRUE
				:  ! (dobj)->GetReadOnly());

	/* add warning text if needed */
	if ( ! dobj->hasWarningText  &&  (dobj)->GetReadOnly()) {
		(dobj)->AddWarningText();
		(self->SourceText)->SetDotPosition( 0);
		(self->SourceText)->FrameDot( 0);
		(dobj)->NotifyObservers( ness_WARNINGTEXTCHANGED);
	}
	::PostMenus(self);
}

/* ScanButton(self)
	processes a click on the Scan button:
	compiles the text at level orange
*/
	static void
ScanButton(class nessview  *self) {
	nessview_Compile(self, ness_codeOrange);
	(self)->WantInputFocus( self);		/* to ensure having Ness menus */
}

/* CompileButton(self)
	processes a click on the Compile button:
	compiles at level Green
*/
	static void
CompileButton(class nessview  *self) {
	nessview_Compile(self, ness_codeGreen);
	(self)->WantInputFocus( self);		/* to ensure having Ness menus */
}

/* CompileMenu(self)
	processes a click on the Compile Menu:
	turns off the PromptBeforeCompile switch
	compiles at level Green
*/
	static void
CompileMenu(class nessview  *self) {
	class ness *dobj = (class ness *)self->dataobject;
	dobj->PromptBeforeCompile = FALSE;
	nessview_Compile(self, ness_codeGreen);
}

	static void
ToggleDebug(register class nessview  *self, long  rock) {
	debug = ! debug;
	ness::SetDebug(debug);
	printf("nessview debug is now %d\n", debug);  fflush (stdout);
}

/* Stuff for Appending to files, or to other Nesses */
static class cursor *waitcursor=NULL;

static int waitcount=0;

/* Turn on the wait cursor so the user knows we're busy.  Keeps a count of the number of invocations of WaitOn, so that functions called from a caller of WaitOn won't
  screw up the state of the wait cursor. 
*/
	static void 
WaitOn() {
	class im *last=im::GetLastUsed();
	waitcount++;

	if(waitcount==1) {

		if(last==NULL || waitcursor!=NULL) return;

		waitcursor=cursor::Create(last);

		if(waitcursor!=NULL) {
			(waitcursor)->SetStandard( Cursor_Wait);
			im::SetProcessCursor(waitcursor);
		}
		
	}
}

/* Turns off the wait cursor if no one wants it on anymore. */
	static void 
WaitOff() {
	waitcount--;
	if(waitcount<=0) {
		waitcount=0;
		if(waitcursor!=NULL) {
			im::SetProcessCursor(NULL);
			delete waitcursor;
			waitcursor=NULL;
		}
	}
}

/* Append the src ness to the end ness.  First it compiles the src if it has not already been compiled.  Errors are sent to the src view and to stderr, a notice is displayed in a dialog if any errors occurred. If the src compiles successfullly, it is appended to the dest and the dest is compiled.  Any errors encountered in compiling the dest are treated the same way as errors compiling the src. */
	static boolean 
DoAppend(class nessview  *self, class ness  *src, class ness  *dest) {
	long end;
	(dest)->SetAccessLevel( ness_codeUV);
	(dest)->SetNeedsDialogBox( FALSE);
	(dest)->SetReadOnly( FALSE);

	(src)->SetAccessLevel( ness_codeUV);
	(src)->SetNeedsDialogBox( FALSE);
	
	if(!src->compiled) {
		WaitOn();
		message::DisplayString(self, 0, "Testing compilability...");

		im::ForceUpdate();

		if((src)->Compile()) {
			(src)->Expose();
			(src)->printerrors(stderr);
			message::DisplayString(self, 0, "");
			message::DisplayString(self, 100, "Error recompiling!");
			WaitOff();
			return FALSE;
		}
		WaitOff();
	}

	end=(dest)->GetLength();

	if(end>0 && (dest)->GetChar( end-1)!='\n') (dest)->InsertCharacters( end, "\n", 1);

	(dest)->CopyTextExactly( (dest)->GetLength(), src, 0, (src)->GetLength());

	end=(dest)->GetLength();

	if(end>0 && (dest)->GetChar( end-1)!='\n') (dest)->InsertCharacters( end, "\n", 1);
	
	WaitOn();
	message::DisplayString(self, 0, "Re-compiling.");

	im::ForceUpdate();

	if((dest)->Compile()) {
		(dest)->Expose();
		(dest)->printerrors(stderr);
		message::DisplayString(self, 0, "");
		message::DisplayString(self, 100, "Error recompiling! Some bindings may be unavailable.");
		WaitOff();
		return FALSE;
	}
	
	WaitOff();
	
	return TRUE;
}

/* Saves the buffer b to filename, and if src is in a buffer which is visible replaces it with the buffer b on the destination of the append. */
	static void 
UpdateFile(class nessview  *self, class ness  *src, class buffer  *b, char  *filename) {
	class buffer *ob;
	WaitOn();
	message::DisplayString(self, 0, "Saving...");
	im::ForceUpdate();

	if((b)->WriteToFile( filename, buffer_ReliableWrite|buffer_MakeBackup)<0) {
		message::DisplayString(self, 0, "");
		message::DisplayString(self, 100, "Write failed!");
		WaitOff();
		return;
	}
	(b)->SetIsModified( FALSE);
	WaitOff();

	message::DisplayString(self, 0, "Done re-compiling and saving.");

	ob=buffer::FindBufferByData(src);

	if(ob!=NULL) {
		class frame *f=frame::GetFrameInWindowForBuffer(ob);
		if(f!=NULL) {
			(f)->SetBuffer( b, TRUE);
			(b)->SetIsModified( FALSE);
			(ob)->Destroy();
		}
	}
}


static class style *fixed=NULL, *boldulined=NULL;

/* Structure used to provide help information and completion when asking for a script name. */
struct helpRock {
	message_workfptr HelpWork; /* The function to call to add to the help text. */
	class text *text; /* The Help text, so that styles can be added. */
	long rock;		/* The rock, this is actually a pointer to a private structure 
				in framemsg, which contains the text, but we don't know that :-( */
	char *partial;  /* The best completion so far. */
	class ness *n; /* The current ness, so that it won't be listed 
					or used as a valid completion. */
};

/* Get a name for a ness, either uses its Adew name, or the tail of its filename, if any.  Otherwise returns NULL. */
	static char *
GetName(class ness  *n) {
	char *name=(char *)(n)->GetName(), *p;
	if(name) return name;
	name=(n)->GetFilename();
	if(name==NULL) return NULL;
	p=strrchr(name, '/');
	if(p) return p+1;
	else return name;
}

/* Appends the name of the ness n to the help text if it matches the partial name 
			in the help rock. */
	static long 
match(class ness  *n,struct helpRock  *h) {
	char buf[1024];
	int len;
	char *name=GetName(n);
	if(name==NULL) return 0;
	if(!strncmp(name, h->partial, strlen(h->partial))) {
		memset(buf,0, sizeof(buf));
		strncpy(buf,name,sizeof(buf)-1);
		(*h->HelpWork)(h->rock,message_HelpListItem,buf,NULL);
	}
	return 0;
}


static char *headingtxt="Active Ness Scripts\n";

/* This is the function called to provide help when the user uses '?' 
		in a prompt for a Ness script. 
*/
	static void
helpProc(char *partial, long lrock, message_workfptr HelpWork, long rock) {
	struct helpRock *myrock = (struct helpRock *) lrock;
	class text *t=myrock->text;
	class ness *n=ness::GetList();
	int len=strlen(headingtxt);
	if(!HelpWork) return;
	(*HelpWork)(rock, message_HelpGenericItem, headingtxt, NULL);
	
	myrock->HelpWork=HelpWork;
	myrock->rock=rock;
	myrock->partial=partial;
	while(n) {
		if(n!=myrock->n) match(n, myrock);
		n=n->next;
	}
	(myrock->text)->AddStyle( 0, len-1, boldulined);
	(myrock->text)->AddStyle( len, (myrock->text)->GetLength()-len, fixed);
}

/* Does the actual completion by iterating over all the nesses except the 
  current one and ones with no name; for each, call completion_CompletionWork. 
*/
	static enum message_CompletionCode 
mycomplete(char *partial, struct helpRock *myrock, char *buffer, 
			int bufferSize) {
	struct result restemp;
	char textBuffer[1024];
	class ness *n = ness::GetList();

	*textBuffer = 0;
	restemp.partial = partial;
	restemp.partialLen = strlen(partial);
	restemp.bestLen = 0;
	restemp.code = message_Invalid;
	restemp.best = textBuffer;
	restemp.max = sizeof(textBuffer) - 1; /* Leave extra room for a NUL. */
	while (n) {
		if (n != myrock->n && GetName(n)) 
			completion::CompletionWork(GetName(n), &restemp);
		n = n->next;
	}

/* buffer[0] = 'a';     :  gives parse error on next line */

	strncpy(buffer, restemp.best, bufferSize);
	if (restemp.bestLen >= bufferSize) {
		/* Now make sure buffer ends in a NUL. */
 		*(buffer+bufferSize-1) = '\0';
	}
	return  restemp.code;
}

static char sbuf[1024]=".atkmacros";

	static long 
AskForScript(class view *self, class ness *current, char *prompt, char *buf, long bufsiz) {
	class ness *n=ness::GetList();
	struct helpRock myrock;
	class framemessage *fmsg=(class framemessage *)(self)->WantHandler("message");
	class buffer *b;

	/* This first part is a hack to get the help text object so that we can add styles 
			to the help we give. */
	/* If the message handler installed is not a framemessage, punt. */
	if(!fmsg || !ATK::IsTypeByName((fmsg)->GetTypeName(),"framemessage")) 
		return -1;

	/* Get the help buffer from the message handlers frame. */
	b=(fmsg->frame)->GetHelpBuffer();
	/* If there is no help buffer then punt, otherwise
	  remember the help text. */
	if(b!=NULL) myrock.text=(class text *)(b)->GetData();
	else return -1;

	/* Set the "Current" ness, this is so that it will not be used as a valid answer ever. */
	myrock.n=current;

	/* does the default exist? */
	while(n) {
		char *sn=GetName(n);
		if(sn && strcmp(sn, sbuf)==0) break;
		n=n->next;
	}

	/* Actually ask, if n is non-NULL sbuf contains the name of an existing ness script
			which is the default answer. */
	if(message::AskForStringCompleted(self, 0, prompt, n?sbuf:NULL, 
			buf, bufsiz, NULL, (message_completionfptr)mycomplete,
			(message_helpfptr)helpProc, (long)&myrock, 
			message_MustMatch)) {
		message::DisplayString(self,0,"Cancelled.");
		return -1;
	}
	/* Make this answer the new default. */
	strcpy(sbuf, buf);
	return 0;
}

/* Appends one Ness script to another.  If the rock is >255 it is assumed to be a pointer to the name of the destination Ness script, otherwise AskForScript is used to get the name of the destination. If the destination has a filename associated with it the file is updated to reflect the new contents of the script, throwing away any changes which had been made to the file. The destination Ness will be recompiled with the new code, and the source will be destroyed if it has a buffer. */
	static void 
ScriptAppend(class nessview  *self, long  rock) {
	char buf[1024];
	char *name=NULL;
	class ness *n=ness::GetList();
	
	class buffer *b;
	
	if(rock>255) name=(char *)rock;
	
	if (name == NULL) {
		if (AskForScript(self, (class ness *)(self)->GetDataObject(), 
				"Append to script:", buf, sizeof(buf)) < 0) 
			return;
		name=buf;
	}
	
	while(n) {
		char *sn=GetName(n);
		if(sn && strcmp(sn, name)==0) break;
		n=n->next;
	}
	
	if(n==NULL) {
		message::DisplayString(self, 0, "Couldn't find the specified Ness script.");
		return;
	}
	
	if(!DoAppend(self, (class ness *)(self)->GetDataObject(), n)) return;
 
	if((n)->GetFilename()==NULL) {
		message::DisplayString(self, 0, 
				"Ness has no associated file, changes have NOT been saved.");
		return;
	}
	
	b=buffer::FindBufferByData(n);

	/* If we create the buffer here it will NOT
	  destroy its data when it goes away. */
	if(b==NULL) b=buffer::Create(GetName(n), (n)->GetFilename(), NULL, n);

	if(b==NULL) {
		message::DisplayString(self, 0, 
				"Couldn't get a buffer on the Ness, changes have NOT been saved.");
		return;
	}

	UpdateFile(self, (class ness *)(self)->GetDataObject(), b, 
			(n)->GetFilename());
}

static char dbuf[1024]="~/.atk.macros";

static char *appendchoices[]={
	"Yes",
	"No",
	NULL
};

/* Appends a Ness script to a file.  If the rock is >255 it is assumed to be a pointer to the filename of the destination, otherwise completion_GetFilename is used to get the filename of the destination. The destination is read in if necessary and the script is appended to it. The destination Ness will be recompiled with the new code, and the source will be destroyed if it has a buffer. Changes to the destination will be saved or a warning will be given. */
	static void 
Append(class nessview  *self, long  rock) {
	char buf[1024];
	char *filename=NULL, *p, *q;
	class buffer *b, *ob;
	ATK  *data;
	class ness *dest, *src=(class ness *)(self)->GetDataObject();
	class ness *n=ness::GetList();
	long result=2;
	boolean diff=FALSE;
	class ness *nm=NULL;
	
	if(rock>255) filename=(char *)rock;
	if(filename==NULL) {
		if(completion::GetFilename(self, "Append to file:", dbuf, buf, sizeof(buf), FALSE, FALSE)<0) {
			message::DisplayString(self, 0, "Cancelled.");
			return;
		}
		filename=buf;
	}

	strcpy(dbuf, filename);
	q=strrchr(filename, '/');
	if(q) {
		q++;
		while(n) {
			p=GetName(n);
			if(p && strcmp(p, q)==0) break;
			n=n->next;
		}
	} else n=NULL;
	
	/* If the buffer already exists we leave its data destroying flag intact. */
	b=buffer::FindBufferByFile(filename);

#if 0
	if(n && !(b && (b)->GetData()==(class dataobject *)n)) {
		if(message::MultipleChoiceQuestion(self, 100, 
				"Warning a Ness Script with that filename is currently active.", 
				0, &result, appendchoices, NULL)<0 || result==0) {
			message::DisplayString(self, 0, "Cancelled!");
			return;
		}
	}

	switch(result) {
		case 1: /* visiting existing script. */
			b=buffer::Create(GetName(n), (n)->GetFilename(), NULL, n);
			break;
		case 2: /* visiting file. */

			if(b==NULL) {
				b=buffer::GetBufferOnFile(filename, 0);
				/* If we created the buffer we don't want the data to go away 
						when the buffer does. */
				if(b!=NULL) (b)->SetDestroyData( FALSE);
			}
	}
#else
	if(n) {
		if(b) {
			nm=(class ness *)(b)->GetData();
			if(nm!=n) diff=TRUE;
		} else diff=TRUE;
		if(diff) {
			char buf[1500];
			sprintf(buf, "Replace script from file %s?", (n)->GetFilename());
			if(message::MultipleChoiceQuestion(self, 100, buf, 0, 
						&result, appendchoices, NULL)<0) {
				message::DisplayString(self, 0, "Cancelled!");
				return; 
			}
			if(result==0) (n)->Destroy();
			else message::DisplayString(self, 0, "Old Ness script not destroyed.");
		}
	}
	
	if(b==NULL) {
		b=buffer::GetBufferOnFile(filename, 0);
		if(b!=NULL) (b)->SetDestroyData( FALSE);
	}
#endif
	
	if(b==NULL) {
		message::DisplayString(self, 0, "Couldn't get a buffer on the specified file.");
		return;
	}
	
	if((b)->GetReadOnly()) {
		message::DisplayString(self, 0, "File is read only. Append cancelled.");
		(b)->Destroy();
		return;
	}
	
	data=(ATK  *)(b)->GetData();
	if(strcmp((data)->GetTypeName(), "ness")!=0) {
		message::DisplayString(self, 0, "The named file is not a ness file.");
		(b)->Destroy();
		return;
	}
	
	dest=(class ness *)data;

	if(!DoAppend(self, src, dest)) return;

	UpdateFile(self, src, b, filename);
}

/* Visit an existing Ness script, brings up the specified Ness script in a buffer in a window. */
	static void 
Visit(class view  *self, long  rock) {
	class ness *n=ness::GetList();
	char buf[1024];
	class buffer *b;
	class frame *f;
	char *name=NULL;

	if(rock>255) name=(char *)name;
	
	if(name==NULL) {
		if(AskForScript(self, NULL, "Visit script:", buf, sizeof(buf))<0) return;
		else name=buf;
	}
	
	while(n) {
		char *sn=GetName(n);
		if(sn && strcmp(sn, name)==0) break;
		n=n->next;
	}
	
	if(n==NULL) {
		message::DisplayString(self, 0, "Couldn't find the specified Ness script.");
		return;
	}

	/* Ensure that no unnecessary headaches occur... */
	(n)->SetAccessLevel( ness_codeUV);
	(n)->SetNeedsDialogBox( FALSE);
	(n)->SetReadOnly( FALSE);
	
	b=buffer::FindBufferByData(n);
	
	if(b==NULL) b=buffer::Create(GetName(n), (n)->GetFilename(), NULL, n);

	if(b==NULL) {
		message::DisplayString(self, 0, 
				"Couldn't get a buffer on the selected Ness script.");
		return;
	}

	f=frame::GetFrameInWindowForBuffer(b);

	if(f==NULL) {
		message::DisplayString(self, 0, 
				"Couldn't get a window on the selected Ness script.");
		return;
	} else {
		class view *v=(f)->GetView();
		if(v!=NULL) (v)->WantInputFocus( v);
	}
}
	

/*  Menus and Keymaps
	The menus and keymaps are initialized to those of
	    EmbeddedMenus and EmbeddedKeymap in InitializeObject.
	The ApplicationMenus and ApplicationKeymap get
	    added to the menus and keymap in GetApplicationLayer

	Debugging can be toggled with the key sequence  ESC-^D -D
*/
static class menulist  *EmbeddedMenus, *ApplicationMenus;
static class keymap  *EmbeddedKeymap, *ApplicationKeymap;

static struct bind_Description EmbeddedBindings[]={
	{"nessview-append-to-file", NULL, 0,
			"Ness,Append to File~40", 0, 0,
			(proctable_fptr)Append,
			"Append the current buffer to a Ness file.", NULL},/*
	{"nessview-append-to-script", NULL, 0,
			"Ness,Append to Script~45", 0, 0,
			(proctable_fptr)ScriptAppend,
			"Append the current buffer to a Ness script.", 
			NULL}, */
	{"nessview-next-error", "\030\016", 0, 	/* ^X^N */
			"Ness~10,Next Error~10", 0, MenuErrors, 
			(proctable_fptr)nessview_NextError, 
			"See the source code for the next error", NULL},
	{"nessview-next-error", "\030\016", 0, 	/* ^X^N */
			"Ness~10,Next Danger~10", 0, MenuDanger, 
			(proctable_fptr)nessview_NextError, 
			"See the source code for the next potentially dangerous statement",
			 NULL},
	{"nessview-show-origin", "\030\016O", 0, 	/* ^X^NO */
			"Ness~10,Show Origin~12", 0, MenuOrigin, 
			(proctable_fptr)ShowOrigin, 
			"Show the author and date of last change", NULL},
	{"nessview-scan-for-danger", "\030\016S", 0, 	/* ^X^NS */
			"Ness~10,Scan for danger~15", 0, MenuScan, 
			(proctable_fptr)ScanButton, 
			"Check for any statements that might be harmful", NULL},
	{"nessview-compile", "\033\016C", 0, 	/* ESC-^N-C */
			"Ness~10,Empower-compile~22", 0, MenuEmpower,
			(proctable_fptr)CompileMenu, 
			"Compile the text in this view", NULL},
	{"nessview-execute", "\030\005", 0, 	/* ^X^E */
			"Ness~10,Do main()~24", 0, MenuMain, 
			(proctable_fptr)nessview_Execute, 
			"Compile Ness and execute function main()", NULL},
	{"nessview-author-mode", "\033~", -1, 	/* ESC-~ */
			"Ness~10,Author mode~32", 0, MenuAuthor, 
			(proctable_fptr)AuthorButton, 
			"Change to be an author", NULL},
	{"nessview-user-mode", "\033~", -1, 	/* ESC-~ */
			"Ness~10,Add warning~32", 1, MenuUser, 
			(proctable_fptr)AuthorButton, 
			"Reinsert the warning text and be readonly", NULL},
	{"ness-invert-debug", "\033\004D",		/* ESC - ^D - D */
			0, 0, 0, 0, (proctable_fptr)ToggleDebug,
			"Toggle the nessview debug flag", NULL},
	NULL
};
static struct bind_Description ApplicationBindings[]={
	NULL
};


/* winning contest entry from Adam Stoller:

A Ness Program Inset Exists In This Document

Do NOT Compile/Execute
Get Help On Ness
Scan Ness for Harmful Effects
Trust Script Author - Compile/Execute

*/

/* DisplayDialogBox(self, time)
	prompt the user as to whether to execute this Ness
 */
	static void 
DisplayDialogBox(class nessview  *self, long  time  /* ignored */) {
	class ness *dobj = (class ness *)self->dataobject;
	long choice;

	static char *choices[] = {
		"Cancel - Keep Ness script inactive", 
		"Help - Add warning text to script", 
		"Scan - Check for dangerous statements", 
	/* 	"Deauthenticate this process",     */  "-  -  -", 
		"Empower - I trust the source of this script", 
		NULL
	};
	ENTER(nessview_DisplayDialogBox);
	if ( ! dobj->DisplayDialogBox) return;	/* (enqueued twice by initial updates) */
	dobj->DisplayDialogBox = FALSE;
kludgefornathaniel:
	self->dialogboxup = TRUE;
	if (message::MultipleChoiceQuestion(NULL, 50, 
				"Ness script here.  Do you trust the user you got it from?",
				0, &choice, choices, "chs-e") < 0 
			|| choice == 0) {
		(dobj)->SetAccessLevel( ness_codeRed);
		choice = 0;
	}
	self->dialogboxup = FALSE;
	(self)->WantUpdate( self);	/* to get uninverted */
	switch (choice) {
	case 1:	/* Help - add warning text */
		(dobj)->AddWarningText();
		DEBUG(("Added warning.  Author button at %ld\n", 
			dobj->AuthorButtonLoc));
		(self->SourceText)->SetDotPosition( 0);
		(self->SourceText)->FrameDot( 0);
		(dobj)->NotifyObservers( ness_WARNINGTEXTCHANGED);
		break;
	case 2:	/* Scan - compile at level orange */
		nessview_Compile(self, ness_codeOrange);
		break;

#ifdef notdef
	case 3:	/* Deauthenticate */
		DeauthButton(self);
		break;
#endif /* not def */
	case 3:	/* repeat the dialog box */
		goto kludgefornathaniel;

	case 4:	/* Compile - carte blanche */
		dobj->PromptBeforeCompile = FALSE;
		nessview_Compile(self, ness_codeGreen);
		break;
	default:
		message::DisplayString(self, 0, "Cancelled");
		break;
	}
	LEAVE(nessview_DisplayDialogBox);
}


	boolean
nessview::InitializeClass() {
	const char *dmf;
	struct proctable_Entry *pe;
	EmbeddedMenus = new menulist;
	ApplicationMenus = new menulist;
	EmbeddedKeymap = new keymap;
	ApplicationKeymap = new keymap;
	bind::BindList(EmbeddedBindings, EmbeddedKeymap, EmbeddedMenus,
			  &nessview_ATKregistry_ );
	pe=proctable::DefineProc("nessview-visit-script", 
		(proctable_fptr)Visit, ATK::LoadClass("frame"), "nessview", 
		"Visit an existing Ness script.");
	if(pe!=NULL) (EmbeddedMenus)->AddToML( "Ness,Visit Script~50", 
			pe, 0, 0);
	bind::BindList(ApplicationBindings, ApplicationKeymap, ApplicationMenus,
			  &nessview_ATKregistry_ );
	fixed=new style;
	if(!fixed) return FALSE;
	boldulined=new style;
	if(!boldulined) {
		delete fixed;
		return FALSE;
	}
	(fixed)->SetFontFamily( "AndyType");
	(fixed)->AddNewFontFace( fontdesc_Fixed);
	(fixed)->SetFontSize( style_ConstantFontSize,10);
	(fixed)->Copy(boldulined);
	(boldulined)->AddUnderline();
	(boldulined)->AddNewFontFace(fontdesc_Bold);

	(boldulined)->SetFontSize( style_ConstantFontSize,20);
	dmf=environ::GetProfile("DefaultMacrosFile");
	if(dmf && strlen(dmf)<sizeof(dbuf)-1) strcpy(dbuf, dmf);

	return TRUE;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
 *	
 *	Override methods
 *	
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
nessview::nessview() {
	ATKinit;

	this->Menus = (EmbeddedMenus)->DuplicateML( this);
	/* ApplicationMenus are added in GetApplicationLayer */
	this->Keystate = keystate::Create(this, EmbeddedKeymap);

	this->SourceText = new textview;
	(this)->SetView( this->SourceText);
	{
	    long scrollpos=scroll_LEFT;
	    const char *pos=environ::GetProfile("ScrollbarPosition");
	    /* The horizontal scrollbar preference is mainly for testing. */
	    boolean hzscroll = environ::GetProfileSwitch("TextHorizontalScrollbar", FALSE);
	    if(pos) {
		if(!strcmp(pos,"right")) scrollpos=scroll_RIGHT;
	    } else if(environ::GetProfileSwitch("MotifScrollbars", FALSE)) scrollpos=scroll_RIGHT;

	    if (hzscroll)
		scrollpos |= scroll_BOTTOM;
	    SetLocation(scrollpos);
	    SetStandardRegions(scrollpos);
	    ti=new textviewInterface(SourceText);
	    SetCustomInterface(ti);

	}
	this->ExecFunction = defaultExecFunc;
	this->CurrError = NULL;
	this->compmod = -1;
	this->ButtonPending = 0;

	this->HasIF = FALSE;
	this->ErrorHighlightPending = FALSE;

	this->inverted = FALSE;
	this->dialogboxup = FALSE;
	this->justscanned = FALSE;

	THROWONFAILURE( TRUE);
}

nessview::~nessview() {
	ATKinit;

	delete ti;
	delete this->Menus;
	delete this->Keystate;

	SetChild(NULL);
	(this->SourceText)->Destroy();
}

	void 
nessview::SetDataObject(class dataobject *tdobj) {
	ENTER(nessview_SetDataObject);

	register struct ness *dobj = (struct ness *)tdobj;

	if (dobj->templateName == NULL) {
		/* if there is no template, read default.template 
			Read text from template if there
			is not yet text in the document.  */
		boolean usetemplatetext = ((dobj)->GetLength() == 0);
#ifdef notdef
		if ((dobj)->ReadTemplate( "ness", usetemplatetext) < 0)
#endif /* notdef */
			(dobj)->ReadTemplate( "default", usetemplatetext);
	}
	(this)->scroll::SetDataObject( dobj);
	(this->SourceText)->SetDataObject( dobj);
	(this->SourceText)->WantUpdate( this->SourceText);

	(this)->FirstError();
	setExecFunc(this, NULL);	

	this->compmod = ( ! dobj->compiled) ? -1 : (dobj)->GetModified();

	LEAVE(nessview_SetDataObject);
}


	void
nessview::WantInputFocus(class view  *requestor) {
ENTER(nessview::WantInputFocus);
	if (requestor == (this)->GetChild())
		/* if the request is for the textview, request it ourselves
			so we can post menus */
		(this)->scroll::WantInputFocus( this);
	else
		(this)->scroll::WantInputFocus( requestor);
LEAVE(nessview::WantInputFocus);
}

	void 
nessview::ReceiveInputFocus() {
	ENTER(nessview_ReceiveInputFocus);
	this->MenusPostedRecently = FALSE;
	this->KeystatePostedRecently = FALSE;
	(this->SourceText)->ReceiveInputFocus();
	this->HasIF = TRUE;

	if (! this->MenusPostedRecently) {
		::PostMenus(this);
	}
	if ( ! this->KeystatePostedRecently) {
		this->Keystate->next = NULL;
		(this)->scroll::PostKeyState( this->Keystate);
	}
	if (this->ErrorHighlightPending) 
		(this)->FirstError();

	LEAVE(nessview_ReceiveInputFocus);
}


	void 
nessview::LoseInputFocus() {
	ENTER(nessview_LoseInputFocus);
	if (this->HasIF) {
		(this->SourceText)->LoseInputFocus();
		this->HasIF = FALSE;
	}
	/* menus and keystate are deactivated by parent */

	LEAVE(nessview_LoseInputFocus);
}

	void
nessview::PostMenus(class menulist  *menulist) {
ENTER(nessview::PostMenus);
	if (menulist != this->Menus) {
		(this->Menus)->ClearChain();
		DEBUG(("chain cleared\n"));
		if (menulist != NULL)
			(this->Menus)->ChainAfterML( menulist, 0);
		::PostMenus(this);  /* self->hasIF is False, so won't actually post */
	}
	(this)->scroll::PostMenus( this->Menus);
	this->MenusPostedRecently = TRUE;
LEAVE(nessview::PostMenus);
}

	void
nessview::PostKeyState(class keystate  *keystate) {
DEBUG(("nessview_PostKeyState(0x%p)  self has 0x%p\n", keystate, this->Keystate));
	if (keystate != this->Keystate)
		(this->Keystate)->AddBefore( keystate);
	(this)->scroll::PostKeyState( this->Keystate);
	this->KeystatePostedRecently = TRUE;
LEAVE(nessview::PostKeyState);
}


	static void
Invert(class nessview  *self) {
	struct rectangle r;

	(self)->SetTransferMode( graphic_INVERT);
	(self)->GetVisualBounds( &r);
	(self)->FillRect( &r, (self)->BlackPattern());
}

	void
nessview::FullUpdate(register enum view_UpdateType   type, register long   left , 
		register long   top , register long   width , register long   height) {
	register class ness *dobj = (class ness *)this->dataobject;
	
	DEBUG(("FullUpdate(%d)\n", type));

	if (type == view_FullRedraw || type == view_LastPartialRedraw) {
		if ( ! this->inverted)
			(this)->scroll::FullUpdate( type, left, top, width, height);
		else {
			/* is inverted */
			(this)->scroll::FullUpdate( view_FullRedraw, left, top, width, height);
			if (this->dialogboxup)
				Invert(this);
			else
				this->inverted = FALSE;
		}
		if (dobj != NULL  &&  dobj->DisplayDialogBox)
			(this)->WantUpdate( this);
	}
	else if (type != view_PartialRedraw || ! this->inverted)
		(this)->scroll::FullUpdate( type, left, top, width, height);


	::PostMenus(this);
	LEAVE(nessview_FullUpdate);
}

	void
nessview::Update() {
	register class ness *dobj = (class ness *)this->dataobject;
	struct errornode *errs = NULL;

	ENTER(nessview_Update);

	if (this->inverted)
		Invert(this);
	(this)->scroll::Update();
	if (this->inverted) {
		if (this->dialogboxup)
			Invert(this);
		else
			this->inverted = FALSE;
	}

	if (dobj != NULL && ! dobj->hasWarningText 
			&& dobj->DisplayDialogBox) {
		im::EnqueueEvent((event_fptr)DisplayDialogBox, (char *)this, 0);

		/* reverse video while dialog box is up */
		this->inverted = TRUE;
		Invert(this);
	}
	if (errs != NULL)
		(this)->FirstError();
	LEAVE(nessview_Update);
}


/* nessview__Hit(self, action, x, y, numberOfClicks)
	checks to see if there is a click on a button in novice text mode
*/
	class view *
nessview::Hit(enum view_MouseAction  action, long  x , long  y, long  numberOfClicks) {
	class ness *dobj = (class ness *)this->dataobject;
	static long initialx, initialy;

	if ( ! dobj->hasWarningText || 
			(action != view_LeftDown
			&& action != view_LeftMovement
			&& action != view_LeftUp) )
		return (this)->scroll::Hit( action, x, y, numberOfClicks);

	if (action == view_LeftDown) {
		/* left mouse down and there is a warning text.
			If the down is near a button, set ButtonPending
		*/
		long tvx, tvy;
		long pos;
		class view *temp;

		this->ButtonPending = 0;
		tvx = physical_GlobalXToLogicalX((this->SourceText)->GetDrawable(),
				physical_LogicalXToGlobalX((this)->GetDrawable(), x));
		tvy = physical_GlobalYToLogicalY((this->SourceText)->GetDrawable(),
				physical_LogicalYToGlobalY((this)->GetDrawable(), y));
		if (tvx < 0 || tvy < 0)
			return (this)->scroll::Hit( action, x, y, numberOfClicks);
		initialx = x;
		initialy = y;

		pos = (this->SourceText)->Locate( tvx, tvy, &temp);

			DEBUG(("LeftDown at %ld  x %ld y %ld (Scan Button at %ld)\n", 
			pos, x, y, dobj->ScanButtonLoc));

		if (pos == dobj->ScanButtonLoc
				|| pos == dobj->ScanButtonLoc + 1)
			this->ButtonPending = 1;
		else if (pos == dobj->AuthorButtonLoc 
				|| pos == dobj->AuthorButtonLoc + 1)
			this->ButtonPending = 2;
		else if (pos == dobj->CompileButtonLoc 
				|| pos == dobj->CompileButtonLoc + 1)
			this->ButtonPending = 3;
#ifdef notdef
		else if (pos == dobj->DeauthButtonLoc
				|| pos == dobj->DeauthButtonLoc + 1)
			this->ButtonPending = 4;
#endif /* notdef */
	}

	if (this->ButtonPending == 0)
		return (this)->scroll::Hit( action, x, y, numberOfClicks);

	if (action == view_LeftMovement) {
		if (x - initialx > 5 || initialx - x > 5 || y - initialy > 5 || initialy - y > 5) {
				DEBUG(("Move too far:  x %ld  y %ld    #clicks %ld\n", 
				x, y, numberOfClicks));
			this->ButtonPending = 0;
			(this)->scroll::Hit( view_LeftDown, initialx, initialy, numberOfClicks);
			return (this)->scroll::Hit( action, x, y, numberOfClicks);
		}
	}
	else if (action == view_LeftUp) {
		if (x - initialx > 3  ||  initialx - x > 3 || y - initialy > 3  ||  initialy - y > 3) {
			(this)->scroll::Hit( view_LeftDown, initialx, initialy, numberOfClicks);
			return (this)->scroll::Hit( action, x, y, numberOfClicks);
		}

		/* defer compile and scan in case the buttons are within a script which is
			itself within a text which has its mouse hits intercepted 
			by a Ness script */

		if (numberOfClicks == 1)
		switch (this->ButtonPending) {
		case 1:	im::EnqueueEvent((event_fptr)ScanButton, (char *)this, 0);
			break;
		case 2:	AuthorButton(this, 0);
			break;
		case 3:	im::EnqueueEvent((event_fptr)CompileButton, (char *)this, 0);
			break;
		case 4:	DeauthButton(this);
			break;
		}
	}
	return (class view *)this;
}


/* nessview__DesiredSize(self, width, height, pass, desiredWidth, desiredHeight) 
	The parent calls this to find out how big the view would like to be.
	This routine sets 'desiredWidth' and 'desiredHeight' and returns a
		code value indicating which is flexible for further negotiation.
	The 'width' and 'height' parameters are tentative values the parent is suggesting.
	'pass' indicates which of 'width' and 'height' should be considered fixed.
	If neither is fixed, they may be arbitrary values. 
*/
	view_DSattributes
nessview::DesiredSize(long  width, long  height, enum view_DSpass  pass, 
		long  *desiredWidth, long  *desiredHeight)  {
	DEBUG(("DesiredSize(...%ld, %ld, %d...)\n", width, height, pass));

	*desiredWidth = 550;
	*desiredHeight = 200;

	DEBUG(("Leave DesiredSize: %ld x %ld\n", *desiredWidth, *desiredHeight));
	return view_HeightFlexible | view_WidthFlexible;
}

	void
nessview::Print(FILE  *file, char  *processor, char *finalFormat, 
		boolean topLevel) {
	DEBUG(("Ignore nessview__Print\n"));
}

/* ObservedChanged(self, changed, value)
	this is called when the ness is destroyed 
	and when NEWERROR is signalled for it
		in the latter case, we show the first error message
	if WARNINGTEXTCHANGED, we have to adjust the menus
*/
void
    nessview::ObservedChanged(register class observable  *ochanged, long  value) {
	class ness *changed=(class ness *)ochanged;
	DEBUG(("Observed changed (value=%ld)\n", value));
	if (value == ness_NEWERROR)
		/* display the error */
		(this)->FirstError();
	else if (value == ness_WARNINGTEXTCHANGED)
		(this)->WantUpdate( this);
	else if (value == 0)
		/* textviewcmds sends 0 */
		if (changed->compiled && this->compmod != (changed)->GetModified())
			/* NB. ness_NotifyObservers sets compiled=FALSE _after_ it
				passes along the status.  Thus the above test is met
				only for the _first_ change.  */
			::PostMenus(this);
		/* tell our parent data object that it has changed  XXX UGH */
		if (this->parent != NULL  &&
				this->parent->dataobject != NULL)
			(this->parent->dataobject)->SetModified();
	LEAVE(nessview_ObservedChanged);
}
