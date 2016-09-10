#define NESS_INTERNALS
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

#include <andrewos.h>	/* for index() */
#include <physical.h>
#include <util.h>		/* for FOLDEDEQ */

#include <im.H>
#include <init.H>
#include <view.H>
#include <proctable.H>
#include <menulist.H>
#include <keymap.H>
#include <message.H>
#include <lpair.H>
#include <arbiterview.H>
#include <celview.H>
#include <cel.H>
#include <value.H>
#include <valueview.H>

/* for FrameMark */
#include <text.H>
#include <textview.H>
#include <atom.H>
#include <observable.H>

#include <nessmark.H>
#include <ness.H>
#include <rules.H>
#include <tlex.H>


/* data for functions reporting event parameters */
static long MouseX = 0;
static long MouseY = 0;
static long MouseAction = -99;
static long SavedClickCount = 1;
static char *LastKeys = NULL;
static char *LastMenu = NULL;

static class init *globalInit = NULL;

static char *pfreeze(char  *name);
static char * CelSymName(char *name);
class nesssym * neventStartExtend(class nesssym  *currobj, class nesssym  *name);
class nesssym * neventFinishExtend(class nesssym  *obj);
class nesssym * neventStartEvent(class nesssym  *currobj, class nesssym  *e, 
		class nesssym  *spec);
class nesssym * neventFinishEvent(class nesssym  *event , class nesssym  *locals, 
		class nesssym  *e);
static union stackelement * unstackLong(union stackelement  *NSP, 
		unsigned char *iar, long  *pLong);
static union stackelement * unstackObject(union stackelement  *NSP, unsigned char *iar,
		class valueview  **pObj, struct ATKregistryEntry   *type);
void neventInfo(unsigned char op, unsigned char *iar, class ness  *ness);
void InterpretEvent(class view  *obj, struct eventnode  *enode);
static class view * MouseProcStub(class view  *obj, enum view_MouseAction  action, 
		long  x , long  y , long  nclicks, struct eventnode  *enode);
static void MenuProcStub(class view  *obj, char  *enodeptr);
static void KeyProcStub(class view  *obj, char  *enodeptr);
static void EventEventStub(class ATK *tness, class observable *tobj, 
	long int tenode);
static void PostMenuEvent(class ness  *ness, struct objnode  *onode, 
		struct eventnode  *enode);
static void PostKeysEvent(class ness  *ness, struct objnode  *onode,
		struct eventnode  *enode);
static void PostMouseEvent(class ness  *ness, struct objnode  *onode, 
		struct eventnode  *enode);
static boolean TryTrigger(class observable  *obj, class atom  *trigger, 
		class ness  *ness, struct eventnode  *enode);
static void PostEventEvent(class ness  *ness, struct objnode  *onode, 
		struct eventnode  *enode);
static void postevents(class ness  *ness, class nesssym  *attr);
static void unpostevents(class ness  *ness, class nesssym  *attr, boolean  debug);
static void HandleNamedCel(class celview  *cv, long int ness);
void  neventPost (class ness  *ness, boolean  debug);
void neventUnpost (class ness  *ness, boolean  debug);


/* This is used to generate a permanent copy of something.  
	Hopefully we will want this same string many times, 
	and won't want too many different strings frozen. 
*/
	static char *
pfreeze(char  *name) {
	class atom *a=atom::Intern(name);
	return (a)->Name();
}


#define 		MAXNAME 100

	static char *
CelSymName(char *name) {
	static char buf[MAXNAME+1];
	strncpy(buf+1, name, MAXNAME-1);
	buf[MAXNAME] = '\0';	/* truncate if too long */
	buf[0] = '&';
	return buf;
}

#undef 		MAXNAME

/* took out inline since cfront didn't like it... sigh... -rr2b */
	static char *
GetOnodeTypeName(struct objnode *onode) {
	if (onode->ExtendAView)
		return (char *)((struct ATKregistryEntry *)(onode->obj))
				->GetTypeName();
	return (char *)(onode->obj)->GetTypeName();
}


/* neventStartExtend(currobj, name)
	start compilation of an object extension
	check that there is no prior extension in progress (currobj == NULL)
	establish a new scope for global names
	uniquify the object name string to be different than a string constant
	and Return the unique object nesssym with its info field pointing at an objnode
*/
	class nesssym *
neventStartExtend(class nesssym  *currobj, class nesssym  *name) {
	class nesssym *newobj;
	boolean new_c;
	struct ATKregistryEntry  *viewClass = NULL;
	char buf[100];
	boolean isView = FALSE;
	char *cname = CelSymName((name)->ToC(curComp->ness, buf, sizeof(buf)));
	if (currobj != NULL) 
		ReportError(":'Extend' is not allowed inside an 'extend' block", -1);

	newobj = nesssym::NLocate(cname, name, (name)->NGetScope(), &new_c);
	if ( ! new_c) {
		compPushScope(nesssym_NGetINode(newobj, objnode)->scope);
		return newobj;
	}

	newobj->flags = flag_xobj;
	newobj->type = Tptr;
	newobj->loc = name->loc;
	newobj->len = name->len;
	if (strncmp(cname, "&view:", 6) == 0) {
		/* set viewClass from the rest of the name */
		isView = TRUE;
		viewClass = ATK::LoadClass(cname+6);
		if (viewClass == NULL)
			ReportError(":unknown view name", 0);
	}
	nesssym_NSetINode(newobj, objnode,
			objnode_Create(NULL, compNewScope(), 
				(curComp->tlex)->RecentPosition( -1, 0), 999,
				(class celview *) viewClass,
				NULL, NULL,		/* pe's */
				NULL, NULL, FALSE, isView));	/* data */

	/* link this extend so codeloc entries from ON xxx are attached to the Ness */
	genLinkGlobal(newobj);
	return newobj;
}

/* neventFinishExtend(obj)
	complete processing of EXTEND ... END
	restore script scope
	return the obj
*/
	class nesssym *
neventFinishExtend(class nesssym  *obj) {
	struct objnode *onode = nesssym_NGetINode(obj, objnode);
	long loc, len;
	loc = (curComp->tlex)->RecentPosition( 0, &len);
	onode->len = loc + len - onode->loc;
	compPopScope();
	return obj;
}

static struct eventnode *allevents=NULL;

/* neventStartEvent(currobj, e, spec)
	start processing an ON ... END:
	check that 'currobj' is not null 
	uniqify the name, hang a new eventnode off it,
	save 'e' in the eventnode for check at finish
	start a function definition and put object code loc in event node
	Return the new symbol
*/
	class nesssym *
neventStartEvent(class nesssym  *currobj, class nesssym  *e, class nesssym  *spec) {
	class nesssym *newevent;
	boolean new_c;
	char buf[400];
	nesssym_scopeType oldscope = curComp->scopes[curComp->scopex];
	static long nextevent = 0;
	class text *newt;
	class nessmark *newm;
	char *cname;
	struct eventnode *lastnode=allevents;

	newt = new text;
	(newt)->AlwaysCopyTextExactly( 0, curComp->ness, spec->loc,
			spec->len);
	BackSlashReduce(newt);
	newm = new nessmark;   
	(newm)->Set( newt, 0, (newt)->GetLength());
	cname = (char *)(newm)->ToC();
	delete newm;		/* also destroys newt */

	sprintf(buf, "\\ Event %ld (%s)", nextevent++, cname);

	if (currobj == NULL) 
		ReportError(":'on' is not allowed outside an 'extend' block", -2);
	newevent = nesssym::NLocate(buf, spec, oldscope, &new_c);
	newevent->flags = flag_event;
	newevent->type = Tfunc;
	allevents = eventnode_Create(genEnter(),
		oldscope, compNewScope(),
		(curComp->ness)->CreateMark( 
			(curComp->tlex)->RecentPosition( -2, 0),  999),
		NULL, e, 
		(char *)cname,	/* pass ownership of cname */
		curComp->ness,
		NULL,		/* no TriggerHolder */
		FALSE,		/* not enabled yet */
		NULL, 		/* no rock yet */
		allevents, 	/* chain all the events together. */
		&allevents 	/* where to store ptr to this eventnode */
	);
	nesssym_NSetINode(newevent, eventnode, allevents);

	/* update the ptr to where the ptr to the last node is kept. */
	if(lastnode) lastnode->meptr= &(allevents->next);

	curComp->curfunctype = Tvoid;	/* for RETURN */

	return newevent;
}


/* neventFinishEvent(event, locals, e)
	complete processing ON ... END
	check 'e' versus value in event's eventnode
	finish the function and do fixups for it 
*/
	class nesssym *
neventFinishEvent(class nesssym  *event , class nesssym  *locals , class nesssym  *e) {
	struct eventnode *enode = nesssym_NGetINode(event, eventnode);
	long loc, len;
	
	if (e != enode->varsym) {
		char buf[50];
		sprintf(buf, "*Should end \"%s\"",  (enode->varsym)->NGetName());
		ReportError(strdup(buf), -1);
	}
	enode->locallist = locals;
	loc = (curComp->tlex)->RecentPosition( 0, &len);
	(enode->where)->SetLength( 
				loc + len - (enode->where)->GetPos());

	if ( ! genExit(NULL, locals))
		SaveError(":More than 10 arguments and locals", 
				(enode->where)->GetPos(),
				(enode->where)->GetLength());

	compPopScope();
	genLinkGlobal(event);
	return event;
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = *\

	Run Time
	Event information return

\* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

	static union stackelement *
unstackLong(union stackelement  *NSP, unsigned char *iar, long  *pLong) {
	if (NSP->l.hdr != longHdr)
		RunError(":arg should be an integer", iar);
	*pLong = NSP->l.v;
	return NSPopSpace(longstkelt); 
}


/* unstackObject(NSP, iar, pObj, type)
	pop an object value off the stack and into *pObj
	if 'type' is not NULL, check that *pObj is a non-NULL ptr to the given type
	Return the new value of NSP.
	Use iar for RunError.
	The types of pObj and type are specific to valueview, but the routine can 
	be used to pop any object by using casts on the arguments.
*/
	static union stackelement *
unstackObject(union stackelement  *NSP, unsigned char *iar, 
		class valueview  **pObj, struct ATKregistryEntry   *type) {
	if (NSP->p.hdr != ptrHdr)
		RunError(":arg should be an object", iar);
	if (type == NULL) {}
	else if (NSP->p.v == NULL)
		RunError(":ptr to object is NULL", iar);
	else {
		*pObj = (class valueview *)
			ProperPtr((ATK  *)NSP->p.v, type);
		if (*pObj == NULL) {
			/* XXX duplicates code in call.c:callCFunc */
			char *buf;
			buf = (char *)malloc(60 
					+ strlen((NSP->p.v)->GetTypeName()) 
					+ strlen((type)->GetTypeName()));
			sprintf(buf, 
				"*object is /%s/ and does not contain /%s/",
				(NSP->p.v)->GetTypeName(),
				(type)->GetTypeName());
			RunError(buf, iar);
		}
	}
	return NSPopSpace(ptrstkelt); 
}

/* neventInfo(op, iar, ness)
	return information about the most recent interactive input
*/
	void
neventInfo(unsigned char op, unsigned char *iar, class ness  *ness) {
	union stackelement *NSP = NSPstore;
	long v;
	boolean bv;
	char *s;
	class view *vptr;
	class valueview *vvwobj;
	class value *vobj;

	switch (op) {
	case 'a':
		v = (long)view_LeftDown;
		goto stackLong;
	case 'b':
		v = (long)view_LeftUp;
		goto stackLong;
	case 'c':
		v = (long)view_LeftMovement;
		goto stackLong;
	case 'd':
		v = (long)view_RightDown;
		goto stackLong;
	case 'e':
		v = (long)view_RightUp;
		goto stackLong;
	case 'f':
		v = (long)view_RightMovement;
		goto stackLong;
	case 'k':
		if (LastKeys == NULL)
			RunError(":no keys event in progress", iar);
		s = LastKeys;
		goto stackString;
	case 'm':
		if (LastMenu == NULL)
			RunError(":no menu event in progress", iar);
		s = LastMenu;
		goto stackString;

	case 'n':		/* value_getvalue(obj) => long */
		NSP = unstackObject(NSP, iar, &vvwobj, valueviewClass);
		vobj = (class value *)vvwobj->dataobject;
		v = (vobj)->GetValue();
		goto stackLong;
	case 'o':		/* value_getarraysize(obj) => long  */
		NSP = unstackObject(NSP, iar, &vvwobj, valueviewClass);
		vobj = (class value *)vvwobj->dataobject;
		v = (vobj)->GetArraySize();
		goto stackLong;
	case 'p':		/* value_getstring(obj) => string */
		NSP = unstackObject(NSP, iar, &vvwobj, valueviewClass);
		vobj = (class value *)vvwobj->dataobject;
		s = (char *)(vobj)->GetString();
		goto stackString;
	case 'q':	{	/* value_getarrayelt(obj, index) => string */
		long index;
		char **sar;
		NSP = unstackLong(NSP, iar, &index);
		NSP = unstackObject(NSP, iar, &vvwobj, valueviewClass);
		vobj = (class value *)vvwobj->dataobject;
		if (index <0  ||  index > (vobj)->GetArraySize())
			RunError(":index out of bounds", iar);
		sar = (char **)(vobj)->GetStringArray();
		s = sar[index];
	}	goto stackString;
	case 'r':		/* value_setvalue(obj, long) */
		NSP = unstackLong(NSP, iar, &v);
		NSP = unstackObject(NSP, iar, &vvwobj, valueviewClass);
		vobj = (class value *)vvwobj->dataobject;
		(vobj)->SetValue( v);
		break;
	case 's':		/* value_setarraysize(obj, long)  */
		NSP = unstackLong(NSP, iar, &v);
		NSP = unstackObject(NSP, iar, &vvwobj, valueviewClass);
		vobj = (class value *)vvwobj->dataobject;
		if (v != (vobj)->GetArraySize()) {
			/* create new array of the given size
				containng NULL ptrs
				XXX never freed  */ 
			char *s = (char *)calloc(v, sizeof(char *)); 
			(vobj)->SetStrArrayAndSize(&s, v);
		}
		break;

	case 't':	{	/* value_setstring(obj, str) */
		s = (FETCHMARK(NSP))->ToC();
		NSP = popValue(NSP);
		NSP = unstackObject(NSP, iar, &vvwobj, valueviewClass);
		vobj = (class value *)vvwobj->dataobject;
		/* store a pointer to the malloced string returned by ToC
			XXX never freed */
		(vobj)->SetString( s);
	}	break;
	case 'u':	{	/* value_setarrayelt(obj, index, str) */
		long index;
		char **sar;
		s = (FETCHMARK(NSP))->ToC();
		NSP = popValue(NSP);	/* discard the marker now */
		NSP = unstackLong(NSP, iar, &index);
		NSP = unstackObject(NSP, iar, &vvwobj, valueviewClass);
		vobj = (class value *)vvwobj->dataobject;
		if (index <0  ||  index >= (vobj)->GetArraySize())
			RunError(":index out of bounds", iar);
		/* store the pointer to the malloced string into the malloced array
			XXX never freed */
		sar = (char **)(vobj)->GetStringArray();
		sar[index] = s;
		(vobj)->NotifyObservers( value_NEWVALUE);
	}	break;
	case 'v':		/* value_setnotify(obj, bool) */
		/* NSP = unstackBool(NSP, iar, &v); */
		if (NSP->b.hdr != boolHdr)
			RunError(":arg should be a boolean", iar);
		v = NSP->b.v;
		NSPopSpace(boolstkelt); 
		NSP = unstackObject(NSP, iar, &vvwobj, valueviewClass);
		vobj = (class value *)vvwobj->dataobject;
		(vobj)->SetNotify( v);
		break;

	case 'w':		/* mouseaction */
		if (MouseAction == -99)
			RunError(":no mouse event in progress", iar);
		v = MouseAction;
		goto stackLong;
	case 'x':		/* mousex */
		if (MouseAction == -99)
			RunError(":no mouse event in progress", iar);
		v = MouseX;
		goto stackLong;
	case 'y':		/* mousey */
		if (MouseAction == -99)
			RunError(":no mouse event in progress", iar);
		v = MouseY;
		goto stackLong;
	case 'C':
		im::QueueCancellation();
		break;
	case 'F':		/* currentinputfocus */
		vptr = (im::GetLastUsed() == NULL) ? NULL
				:  (im::GetLastUsed())->GetInputFocus();	
		goto stackPointer;
	case 'H':  {	/* DoHit(inset, action, x, y) */
		/* XXX ought to guarantee an up for every transmitted down */
		long x, y;
		enum view_MouseAction act;
		class view *inset;
		static class view *hitee = NULL, *hiteesource = NULL;
		NSP = unstackLong(NSP, iar, &y);
		NSP = unstackLong(NSP, iar, &x);
		NSP = unstackLong(NSP, iar, (long *)&act);
		/* now inset is at top of stack */
		if (NSP->p.hdr != ptrHdr)
			RunError(":arg should be an object", iar);
		inset = (class view *)NSP->p.v;
		NSPopSpace(ptrstkelt);
		if (inset == NULL  ||  !  (inset)->IsType( viewClass))
			RunError(":first arg should be a view", iar);
		if (ness->CurrentInset == inset  &&  (inset)->IsType( celviewClass)) {
			/* we short cut around the celview to avoid retrapping a mouse hit */
			if (((class celview *)inset)->GetApplication() != NULL)
				inset = ((class celview *)inset)->GetApplication();
			else if (((class celview *)inset)->GetTrueChild() != NULL)
				inset = ((class celview *)inset)->GetTrueChild();
		}

		if (hiteesource != inset  ||  act == (long)view_LeftDown  ||  act == (long)view_RightDown) {
			hiteesource = inset;
			hitee = (inset)->Hit( act, x, y, SavedClickCount);  /* XXX do we need a click count? */
		}
		else {
			/* move or up, with inset the same as where we sent the last hit:
				 send to hitee */
		    hitee = (hitee)->Hit( act,physical_GlobalXToLogicalX((hitee)->GetDrawable(), physical_LogicalXToGlobalX((inset)->GetDrawable(), x)), physical_GlobalYToLogicalY((hitee)->GetDrawable(), physical_LogicalYToGlobalY((inset)->GetDrawable(), y)), SavedClickCount); /* XXX do we need a click count? */
		}
		if(hitee) vptr=hitee;
		else vptr=ness->CurrentInset;
		if (vptr == NULL) 
		    vptr = (class view *)ness->DefaultText;
		goto stackPointer;
	}	break;	/* end case 'H' */
	case 'I':		/* currentinset */
		vptr = ness->CurrentInset;
		if (vptr == NULL) 
			vptr = (class view *)ness->DefaultText;
		goto stackPointer;
	case 'J':		/* currentwindow */
		vptr = ness->CurrentInset;
		if (vptr == NULL) 
			vptr = (class view *)ness->DefaultText;
		if (vptr == NULL) 
			goto stackPointer;
		vptr = (class view *)(vptr)->GetIM();
		goto stackPointer;

 	/* launchApplication(marker, title, programname, enablecommands)
	{"launchApplication", "UP", {Tvoid, Tstr, Tstr, Tstr, Tbool, Tend}, ness_codeGreen}, */

	case 'P':	{
		boolean enablecommands;
		char *pgmnm, * title;
		class text *text;
		long pos, len;

		if (NSP->l.hdr != boolHdr)
			RunError(":last argument should be a boolean value", iar);
		enablecommands = NSP->b.v;
		NSP = popValue(NSP);

		pgmnm = (char *)(FETCHMARK(NSP))->ToC();
		NSP = popValue(NSP);

		title = (char *)(FETCHMARK(NSP))->ToC();
		NSP = popValue(NSP);

		text = FrameMark(ness, FETCHMARK(NSP), 
			title, pgmnm, enablecommands, &pos, &len);

		free(pgmnm);
		free(title);
		(FETCHMARK(NSP))->Set( text, pos, len);
	}	break;
	/* QueueAnswer(marker)
	 {"QueueAnswer", "UQ", {Tvoid, Tstr, Tend}, ness_codeGreen}, */
        case 'Q': {
		char *buf;
		buf = (FETCHMARK(NSP))->ToC();
		im::QueueAnswer(buf);
		free(buf);
		NSP = popValue(NSP);
	}	break;
	case 'R':		/* isreadonly(string) */
		bv = ((FETCHMARK(NSP))->GetText())->GetReadOnly();
		NSP = popValue(NSP);
		goto stackBool;
	case 'T':	{	/* TellUser(msg) */
		char *buf;
		buf = (FETCHMARK(NSP))->ToC();
		message::DisplayString(im::GetLastUsed(), 0, buf);
		im::ForceUpdate();
		free(buf);
		NSP = popValue(NSP);
		ness->ToldUser = TRUE;
	}	break;
	case 'U':	{	/* AskUser(prompt, default) */
		char *prompt, *defaultstring;
		char buf[301];

		defaultstring = (FETCHMARK(NSP))->ToC();
		NSP = popValue(NSP);
		prompt = (FETCHMARK(NSP))->ToC();
		NSP = popValue(NSP);
		if (message::AskForString(im::GetLastUsed(), 0,  prompt, 
				defaultstring, buf, 300) < 0) {
			message::DisplayString(im::GetLastUsed(), 0, "Cancelled.");
			strcpy(buf, "CANCEL");
		}
		s = buf;
		free(prompt);			
		free(defaultstring);
		ness->ToldUser = FALSE;
	}	goto stackString;


	stackBool:
		NSPushSpace(boolstkelt); 
		NSP->b.hdr = boolHdr;
		NSP->b.v = bv;
		break;
	stackLong:
		NSPushSpace(longstkelt); 
		NSP->l.hdr = longHdr;
		NSP->l.v = v;
		break;
	stackString:
		PUSHMARK(NULL)->MakeConst(s);
		break;
	stackPointer:
		NSPushSpace(ptrstkelt); 
		NSP->p.hdr = ptrHdr;
		NSP->p.v = (ATK  *)vptr;
		break;
	}		/* end switch(op) */
}

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = *\

	Run Time
	Event handlers

\* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

static boolean PostDebug = FALSE;	/* side arg to HandleNamedCell */

	void
InterpretEvent(class view  *obj, struct eventnode  *enode) {
	class ness *ness = enode->parentness;
	class view *RememberCurrentInset;
		/* The "CurrentInset" value in a normal Ness is its text 
			or maybe the inset active when ESC-ESC was picked
		   We save the currentinset value and replace it with the one
		   from the inset that had the event. */
	if ( ! enode->enabled) 
		return;
	(ness)->EstablishViews( obj);
	RememberCurrentInset = ness->CurrentInset;
	ness->CurrentInset = obj;

	ness->ErrorList = callInitAll(ness);
	if (ness->ErrorList == NULL)
		ness->ErrorList = interpretNess(enode->SysGlobOffset, 
				NULL, ness);
	ness->CurrentInset = RememberCurrentInset;

	if (ness->ErrorList != NULL) {
		neventUnpost(ness, FALSE);	/* remove old postings */
		MapRunError(ness);
		(ness)->Expose();
	}
}


/* MouseProcStub(obj, action, x, y, nclicks, enode)
	This procedure is called by a mouse event.
	From the xattr it gets everything it needs to initiate the proper Ness stmtList.
	 	these are the possible mouse actions:
			view_NoMouseEvent,
			view_LeftDown, view_LeftUp, view_LeftMovement,
			view_RightDown, view_RightUp, view_RightMovement
*/
	static class view *
MouseProcStub(class view  *obj	/* the celview */, enum view_MouseAction  action, 
		long  x , long  y , long  nclicks, struct eventnode  *enode) {
	/* XXX we need to scan xattr looking for other mouse event handlers */
	MouseX = x;
	MouseY = y;
	MouseAction = (long)action;
	SavedClickCount = nclicks;
	InterpretEvent(obj, enode);
	MouseAction = -99;
	SavedClickCount = 1;		/* yuck XXX should give this to client */
	return obj;
}

/* MenuProcStub(obj, enode)
	This procedure is called by a menu selection event.  
	From the xattr it gets everything it needs to initiate the proper Ness stmtList.
*/
	static void
MenuProcStub(class view  *obj	/* the celview, I think. I will ignore it */, char  *enodeptr) {
	struct eventnode *enode;
	struct eventnode *p=allevents;

	if(!enodeptr) return;
	enode=(struct eventnode *)atol(enodeptr);

	/* Make sure this event still exists. */
	while(p && p!=enode) p=p->next;

	if(!p) return;
	
	LastMenu = enode->spec;
	InterpretEvent(obj, enode);
	LastMenu = NULL;
}

/* KeyProcStub(obj, enode)
	This procedure is called by a key sequence event.
	From the enode it gets everything it needs to initiate the proper Ness stmtList.
*/
	static void
KeyProcStub(class view  *obj	/* the celview, I think. I will ignore it */, char  *enodeptr) {
	struct eventnode *enode;
	struct eventnode *p=allevents;
	
	if(!enodeptr) return;
	enode=(struct eventnode *)atol(enodeptr);

	/* Make sure this event still exists.  */
	while(p && p!=enode) p=p->next;

	if(!p) return;

	LastKeys = enode->spec;
	InterpretEvent(obj, enode);
	LastKeys = NULL;
}


	static void 
EventEventStub(class ATK *tness, class observable *tobj, long int tenode) {
	InterpretEvent((class view *)tobj, (struct eventnode *)tenode);
}


/* PostMenuEvent(ness, onode, enode)
	the script 'ness' has an extension for object described by objnode 'onode'
		and having a Menu event 'enode'
	post it to onode->obj
*/
	static void
PostMenuEvent(class ness  *ness, struct objnode  *onode, struct eventnode  *enode) {
	char name[400];
	if (onode->menupe == NULL) {
		/* create (or find) menu pe for this class */
	        struct ATKregistryEntry *OnodeRegistryEntry;
	        OnodeRegistryEntry =  (onode->ExtendAView ? (struct ATKregistryEntry *) onode->obj : (onode->obj)->ATKregistry());
		sprintf(name, "%s-NessMenuEvent",
				GetOnodeTypeName(onode));
		onode->menupe = proctable::DefineProc(pfreeze(name),
				(long int (*)(class ATK*,long int))
				MenuProcStub, 
				OnodeRegistryEntry, 
				GetOnodeTypeName(onode),
				"Call Ness for a menu event");
	}
	if (onode->menulist == NULL) {
		/* need to create and post the menulist */
		onode->menulist = menulist::Create(onode->obj);
		if (onode->ExtendAView) {
			if (globalInit != NULL)
				(globalInit)->AddMenuBinding( 
					GetOnodeTypeName(onode),
					TRUE, onode->menulist);
		}
		else (onode->obj)->SetMenulist( onode->menulist);
	}
	sprintf(name, "%ld", (long)enode);
	enode->rock=strdup(name);
	if(enode->rock) {
	/* add the key sequence to the keymap */
		   (onode->menulist)->AddToML( enode->spec, onode->menupe, (long)enode->rock, 0);
		   enode->enabled = TRUE;
	}
}

/* PostKeysEvent(ness, onode, enode)
	the script 'ness' has an extension for object described by objnode 'onode'
		and having a Keys event 'enode'
	post it to onode->obj
*/
	static void
PostKeysEvent(class ness  *ness, struct objnode  *onode, struct eventnode  *enode) {
	char name[400];
	if (onode->keype == NULL) {
		/* create (or find) keys pe for this class */
	        struct ATKregistryEntry *OnodeRegistryEntry;
	        OnodeRegistryEntry =  (onode->ExtendAView ? (struct ATKregistryEntry *) onode->obj : (onode->obj)->ATKregistry());
		sprintf(name, "%s-NessKeysEvent",
				GetOnodeTypeName(onode));
		onode->keype = proctable::DefineProc(pfreeze(name),
				(long int (*)(class ATK*,long int))
				KeyProcStub,
				OnodeRegistryEntry, 
				GetOnodeTypeName(onode),
				"Call Ness for a keys event");
	}
	if (onode->keymap == NULL) {
		/* need to create and post the keymap */
		onode->keymap = new keymap;
		if (onode->ExtendAView) {
			if (globalInit != NULL)
				(globalInit)->AddKeyBinding( 
					GetOnodeTypeName(onode),
					TRUE, onode->keymap);
		}
		else (onode->obj)->SetKeymap( onode->keymap);
	}

	sprintf(name, "%ld", (long)enode);
	enode->rock=strdup(name);
	if(enode->rock) {
	    /* add the key sequence to the keymap */
	    (onode->keymap)->BindToKey( enode->spec, onode->keype, 
			(long)enode->rock);
	    enode->enabled = TRUE;
	}
}

/* PostMouseEvent(ness, onode, enode)
	the script 'ness' has an extension for object described by objnode 'onode'
		and having a Mouse event 'enode'
	post it to onode->obj
*/
	static void
PostMouseEvent(class ness  *ness, struct objnode  *onode, struct eventnode  *enode) {
	if (onode->MouseEnabled || onode->ExtendAView) 
		return;
	(onode->obj)->SetHitfunc((celview_hitfptr) MouseProcStub, (long)enode);
	onode->MouseEnabled = TRUE;
	enode->enabled = TRUE;
}


/* TryTrigger(obj, trigger, ness, enode)
	Try AddRecipient for 'obj' to see if it has the trigger
	If successful set enode->TriggerHolder and enode->enable
	return TRUE for success and FALSE for failure
*/
	static boolean
TryTrigger(class observable  *obj, class atom  *trigger, class ness  *ness, 
		struct eventnode  *enode) {
	if (obj == NULL) {
		return FALSE;
	}
	if ((obj)->AddRecipient(trigger, ness, EventEventStub, (long)enode)) {
		enode->TriggerHolder = obj;
		enode->enabled = TRUE;
		return TRUE;
	}
	if ((obj)->IsType( viewClass)) {
	    boolean foo = TryTrigger(((view *)obj)->GetDataObject(), trigger, ness, enode);
	    return foo;
	}
	return FALSE;
}


/*  PostEventEvent(ness, onode, enode)
	post  the event 'enode' to the object given by 'onode', using the 'ness'

	This event derives from   --  on event "..." --  in the source;
	it corresponds to a trigger defined by the target object.

	To process triggers we just do observable_AddRecipient().
	If it returns FALSE, we try another portion of the cel.
	We look at the cel, the celview, the child, and the true child,
	the child's data object, and the true child's data object.
*/
	static void 
PostEventEvent(class ness  *ness, struct objnode  *onode, struct eventnode  *enode) {
	class atom *trigger;

	if (FOLDEDEQ("becamevisible", enode->spec)) {
		enode->enabled = TRUE;
		return;
	}
	if (onode->ExtendAView) 
		return;
	trigger = atom::Intern(enode->spec);
	if (TryTrigger(onode->obj, trigger, ness, enode))  
		return;
	if ((onode->obj)->IsType( celviewClass)) {
		class celview *cv = (class celview *)onode->obj;
		if (TryTrigger((cv)->GetTrueChild(), trigger, ness, enode))  
			return;
		if ((cv)->GetApplication()  !=  (cv)->GetTrueChild()
				&&  TryTrigger((cv)->GetApplication(),
					trigger, ness, enode))
			return;
	}
	if ((onode->obj)->IsType( imClass)) {
		class view *v = ((class im *)onode->obj)->topLevel;
		if (TryTrigger(v, trigger, ness, enode))
			return;
		if ((v)->IsType( lpairClass)  &&
				TryTrigger(((class lpair *)v)->GetNth(0),
					trigger, ness, enode))
			return;
	}
	/* well, we didn't find it.  
	XXX we ought to post an error message */
}


/* postevents(ness, attr)
	the script 'ness' has an extension for object with name 'attr'
	post all events intended for this object
*/
	static void
postevents(class ness  *ness, class nesssym  *attr) {
	boolean repost;
	class nesssym *xattr;
	struct objnode *onode;
	char buf[100];

	repost = FALSE;

	if (PostDebug)
		printf("Posting events for %s\n",  
			(attr)->ToC( ness, buf, sizeof(buf)-1));

	/* traverse xattr's, posting each event */
	onode = nesssym_NGetINode(attr, objnode);
	for (xattr = onode->attrs; xattr != NULL; xattr = xattr->next) 
		if (xattr->flags == flag_event) {
			struct eventnode *enode = nesssym_NGetINode(xattr, eventnode);
			int n;
			if (PostDebug)
				printf("	%s event (%s)\n", 
					(enode->varsym)->NGetName(),
					enode->spec);
			xattr->parent.nesssym = attr;	/* ensure that parent is set */
			n = enode->varsym->toknum;
			if (n == MENU) {
				PostMenuEvent(ness, onode, enode);
				repost = TRUE;
			}
			else if (n == KEYS) {
				PostKeysEvent(ness, onode, enode);
				repost = TRUE;
			}
			else if (n == MOUSE) 
				PostMouseEvent(ness, onode, enode);
			else if (n == EVENT) 
				PostEventEvent(ness, onode, enode);
		} /* end if (and for) */

	if (repost  && ! onode->ExtendAView)  
		(onode->obj)->Repost();
}

/* unpostevents(ness, attr, xattr)
	the script 'ness' has an extension for object with name 'attr'
	unpost all its events
*/
	static void
unpostevents(class ness  *ness, class nesssym  *attr, boolean  debug) {
	struct objnode *onode = nesssym_NGetINode(attr, objnode);
	class nesssym *xattr;
	boolean repost;
	char buf[100];

	if (onode->ExtendAView) {
		if (onode->menulist != NULL && globalInit != NULL)
		    (globalInit)->DeleteMenuBinding( 
				GetOnodeTypeName(onode),
				TRUE, onode->menulist);
		if (onode->keymap != NULL && globalInit != NULL)
			(globalInit)->DeleteKeyBinding( 
				GetOnodeTypeName(onode),
				TRUE, onode->keymap);
		if (onode->keymap != NULL)  {
		    delete onode->keymap;
		    onode->keymap = NULL;
		}
		if (onode->menulist != NULL) {
		    delete onode->menulist;
		    onode->menulist = NULL;
		}
		return;
	}
	/* it is an extended object   (ExtendAView is false)*/

	/* set onode->obj or give error
		look up the object by name again in case it has been cut from the base text */
	if ((ness)->GetArbiter() == NULL  ||  
			(onode->obj=arbiterview::GetNamedCelview(
					(ness)->GetArbiter(),
					(attr)->ToC( ness, buf, sizeof(buf)-1))) 
				== NULL) {
		if (debug) {
			printf("unpostevents could not find \"%s\" in arb at 0x%p\n",
				(attr)->ToC( ness, buf, sizeof(buf)-1),
				(ness)->GetArbiter());
		}
		return;
	}

	/* traverse xattr's, disabling each event */
	for (xattr = onode->attrs; xattr != NULL; xattr = xattr->next) {
		struct eventnode *enode 
				= nesssym_NGetINode(xattr, eventnode);
		if (xattr->flags == flag_event) 
			enode->enabled = FALSE;
		if (enode->TriggerHolder != NULL) {
			(enode->TriggerHolder)->DeleteRecipient( 
				atom::Intern(enode->spec), ness);
			enode->TriggerHolder = NULL;
		}
	}

	repost = FALSE;

	if (onode->obj == NULL) 
		return;
	if (onode->menulist != NULL) {
		/* tell the object it has no menulist */
		(onode->obj)->SetMenulist( NULL); 
		repost = TRUE;
	}
	if (onode->keymap != NULL)  {
		/* tell the object it has no keymap */
		(onode->obj)->SetKeymap( NULL);
		repost = TRUE;
	}
	if (onode->MouseEnabled) {
		(onode->obj)->SetHitfunc( NULL, 0);
		onode->MouseEnabled = FALSE;
	}

	if (repost)   (onode->obj)->Repost();

	if (onode->keymap != NULL)  {
		delete onode->keymap;
		onode->keymap = NULL;
	}
	if (onode->menulist != NULL) {
		delete onode->menulist;
		onode->menulist = NULL;
	}
}

/* HandleNamedCel(cv, ness)
	This function is called each time an arbiter finds out about a named cel.
	It scans the NessList looking for a ness with an extend for a cel of that name.
	(The ness arg is ignored because arbiters may have multiple nesses.)
*/
	static void
HandleNamedCel(class celview  *cv, long int ness) {
	class cel *c;
	char *tname;
	class ness *n;
	class nesssym *attr;
	struct objnode *onode;
	class nesssym *xattr;

	c = (class cel *)cv->dataobject;
	if (c == NULL  ||  (c)->GetRefName() == NULL) return;
	tname = CelSymName((char *)(c)->GetRefName());
	for (n = ness::GetList();  n != NULL;  n = n->next) {
		if ( ! n->compiled  ||  (n)->GetArbiter() == NULL)  continue;
		if (cv->arb == NULL || (n)->GetArbiter() != cv->arb) continue;
		if (strcmp(tname, "&defaulttext") == 0)
			/* We are being told of a child called "defaulttext" and n
			    is associated with cv->arb.  Set n->DefaultText.*/
			(n)->SetDefaultText( (class textview *)
					ProperPtr((ATK  *)cv, 
						textviewClass));
		attr = nesssym::NFind(tname, n->constScope);
		if (attr == NULL || attr->flags != flag_xobj) continue;

		/* 'attr' is the symbol entry for an EXTEND block for the given name */

		onode = nesssym_NGetINode(attr, objnode);
		onode->obj = cv;
		attr->parent.ness = n;	/* set parent */
		postevents(n, attr);

		/* check to see if it has an enode for 'becameVisible' */

		for (xattr = onode->attrs; xattr != NULL; xattr = xattr->next) 
		    if (xattr->flags == flag_event) {
			struct eventnode *enode = nesssym_NGetINode(xattr, eventnode);
			if (enode->varsym->toknum == EVENT &&
				    FOLDEDEQ ("becamevisible", enode->spec)) {
				/* it has "becameVisible": call it */
				EventEventStub(n, (class view *)cv, 
					(long)enode);
				break;	/* from the for(xattr) */
			}
		    } /* end if (and for) */
		return;
	}
}



/* neventPost (ness)
	Post all events for the extended objects in ness->globals
*/
	void 
neventPost (class ness  *ness, boolean  debug) {
	class arbiterview *arb = (ness)->GetArbiter();
	class nesssym *attr;

	PostDebug = debug;

	globalInit = im::GetGlobalInit();

	if (arb != NULL)
		(arb)->AddHandler( (arbiterview_hfptr)HandleNamedCel, 0L);

	/* process -extend "view:name"- */
	for (attr = ness->globals; attr != NULL; attr = attr->next) 
		/* visit each global entity in the script,
		   if it is an xobj with name beginning "&view:"
		   post the events */
		if (attr->flags == flag_xobj  
				&& nesssym_NGetINode(attr, objnode)->ExtendAView) {
			/* process its -on proc-s */
			attr->parent.ness = ness;	/* set parent */
			postevents(ness, attr); 
		}
}


/* neventUnpost (ness)
	Remove all events posted for the extended objects in ness->globals
*/
	void
neventUnpost (class ness  *ness, boolean  debug) {
	class nesssym *attr;
	char buf[100];

	globalInit = im::GetGlobalInit();

	for (attr = ness->globals; attr != NULL; attr = attr->next) 
		/* visit each global entity in the script,
		   if it is an xobj, and has events, unpost them */
		if (attr->flags == flag_xobj) {
			if (debug)
				printf("Unposting events for %s\n",
					(attr)->ToC( ness, buf, sizeof(buf)-1));
			attr->parent.ness = ness;	/* set parent */
			unpostevents(ness, attr, debug);
		}
}
