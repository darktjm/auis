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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/textobjects/RCS/printopts.C,v 1.17 1995/02/03 21:40:07 Zarf Stab74 $";
#endif

extern char ProgramName[];	/* blechhh -- used by Print and Preview commands to guess a filename for printing. Not really necessary, but nice to have around. */

#include <andrewos.h> /* strings.h */
ATK_IMPL("printopts.H")

#include <rect.h>

#include <atom.H>
#include <atomlist.H>
#include <bind.H>
#include <buffer.H>
#include <completion.H>
#include <cursor.H>
#include <dataobject.H>
#include <environ.H>
#include <frame.H>
#include <im.H>	
#include <event.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <message.H>
#include <style.H>
#include <stylesheet.H>
#include <environment.H>
#include <text.H>
#include <view.H>
#include <print.H>

#include <printopts.H>

#define debug (FALSE)
#define DEBUG(s) {if (debug) {printf s ; fflush(stdout);}}
#define ENTER(r) DEBUG(("Enter %s(0x%lx)\n", Stringize(r), self))
#define LEAVE(r) DEBUG(("Leave %s(0x%lx)\n", Stringize(r), self))

#define ANSWERLEN (12)

static class atom *A_int, *A_string, *A_file, *A_boolean, *A_tofile, *A_printer, *A_psfile;

static void ApplyValues(class printopts  *self);
static void printopts_Nop(class printopts  *self, long  rock);
static void MenuDone(class printopts  *self, long  rock);
static void MenuCancel(class printopts  *self, long  rock);
static void MenuPrint(class printopts  *self, long  rock);
static void MenuPreview(class printopts  *self, long  rock);
static void CreateWindow(class view  *vp, long  rock);
static void DestroyWindow(class printopts  *self );

ATKdefineRegistry(printopts, textview, printopts::InitializeClass);

static class cursor *waitCursor;
static class menulist *Menus;
static class keymap *Keys;

boolean printopts::InitializeClass()
{
    struct ATKregistryEntry  *frameClassinfo;
    struct proctable_Entry *proc = NULL;

    A_int = atom::Intern("int");
    A_string = atom::Intern("string");
    A_file = atom::Intern("file");
    A_boolean = atom::Intern("boolean");
    A_tofile = atom::Intern("tofile");
    A_psfile = atom::Intern("psfile");
    A_printer = atom::Intern("printer");

    waitCursor = cursor::Create(NULL);
    (waitCursor)->SetStandard( Cursor_Wait);

    Menus = new menulist;
    Keys  = new keymap;

    proc = proctable::DefineProc("printopts-nop", (proctable_fptr) printopts_Nop, &printopts_ATKregistry_ , NULL, "Do nothing.");
    Keys->BindToKey( "\033~", proc, 0); /* override toggle-read-only */

    proc = proctable::DefineProc("printopts-done", (proctable_fptr)MenuDone, &printopts_ATKregistry_ , NULL, "Remove dialog and accept choices.");
    Menus->AddToML( "Done~90", proc, 0, 0);
    Menus->AddToML( "Quit~99", proc, 0, 0); /* right now, the actual quit procedure tries to bring up a dialog box, and if there already is one, the process exits silently *even if there are unsaved documents*. Bad bad bad. Overriding the Quit menu and key commands is a bad solution, but it will do for now. */
    Keys->BindToKey( "\030\004", proc, 0); /* ^X^D */
    Keys->BindToKey( "\030\003", proc, 0); /* ^X^C */

    proc = proctable::DefineProc("printopts-cancel", (proctable_fptr)MenuCancel, &printopts_ATKregistry_ , NULL, "Remove dialog and cancel choices.");
    Menus->AddToML( "Cancel~91", proc, 0, 0);

    proc = proctable::DefineProc("printopts-print", (proctable_fptr)MenuPrint, &printopts_ATKregistry_ , NULL, "Print document.");
    Menus->AddToML( "Print", proc, 0, 0);

    proc = proctable::DefineProc("printopts-preview", (proctable_fptr)MenuPreview, &printopts_ATKregistry_ , NULL, "Preview document.");
    Menus->AddToML( "Preview", proc, 0, 0);

    frameClassinfo = ATK::LoadClass("frame");
    if (frameClassinfo != NULL) {
	proc = proctable::DefineProc("printopts-post-window", (proctable_fptr)CreateWindow, frameClassinfo, NULL, "Set options for printing of document");
	return TRUE;
    }

    return FALSE;
}

printopts::printopts()
{
    ATKinit;

    this->keys  = keystate::Create(this, Keys);
    this->menus = (Menus)->DuplicateML( this);
    this->oplist = NULL;
    this->opview = NULL;
    this->parentframe = NULL;

    THROWONFAILURE( TRUE);
}

printopts::~printopts()
{
    int ix;
    ATKinit;

    if (this->keys) delete this->keys;
    if (this->menus) delete this->menus;
    if (this->oplist) {
	for (ix=0; ix<this->numops; ix++)
	    free(this->oplist[ix].answer);
	free(this->oplist);
    }
}

struct view_printoptlist *printopts::PrintOptions()
{
    /* This makes extra-sure nobody tries anything stupid with recursive dialog boxes. */
    return NULL;
}

void printopts::PostMenus(class menulist  *ml)
{
    (this->menus)->ClearChain();
    if (ml) (this->menus)->ChainAfterML( ml, (long)ml);
    (this)->textview::PostMenus( this->menus);
}

void printopts::PostKeyState(class keystate  *ks)
{
    class keystate *newch;

    if (this->parent != NULL) {
	newch = (this->keys)->AddBefore(ks);
	(this->parent)->PostKeyState(newch);
    }
}

static void AdjustHiddenEnv(class printopts *self, boolean tofile)
{
    class text *tx = (class text *)self->GetDataObject();
    long pos, len;
    class environment *envtohide;
    class style *hiddenstyle;

    pos = (-1);
    envtohide = self->oplist[tofile ? self->printnamenum : self->filenamenum].env;
    if (!envtohide) {
	if (self->hiddenenv) {
	    tx->RegionModified(self->hiddenenv->Eval(), self->hiddenenv->GetLength());
	    self->hiddenenv->Delete();
	    self->hiddenenv = NULL;
	}
	return;
    }

    pos = envtohide->Eval();
    len = envtohide->GetLength();

    if (self->hiddenenv) {
	if (pos == self->hiddenenv->Eval()) {
	    /* already in right place */
	    return;
	}
	tx->RegionModified(self->hiddenenv->Eval(), self->hiddenenv->GetLength());
	self->hiddenenv->Delete();
	self->hiddenenv = NULL;
    }

    hiddenstyle = tx->GetStyleSheet()->Find("hidden");
    if (!hiddenstyle)
	return;
    self->hiddenenv = tx->AlwaysAddStyle(pos, len, hiddenstyle);
    if (self->hiddenenv)
	self->hiddenenv->SetStyle(FALSE, FALSE);
}

static void SetNewValue(class printopts *self, struct p_option *rec, char *str)
{
    class text *tx = (class text *)self->GetDataObject();
    long pos, len;
    int strlength;

    rec->changed = TRUE;
    pos = rec->ansenv->Eval();
    len = rec->ansenv->GetLength();
    tx->AlwaysDeleteCharacters(pos+1, len-2);
    strlength = strlen(str);
    if (strlength > ANSWERLEN) {
	free(rec->answer);
	rec->answer = (char *)malloc(strlength+1);
    }
    strcpy(rec->answer, str);
    tx->AlwaysInsertCharacters(pos+1, rec->answer, strlength);
    if (rec->op->name == A_tofile) {
	AdjustHiddenEnv(self, rec->answer[0] == 'Y');
    }
    tx->NotifyObservers(observable_OBJECTCHANGED);
}

static void HandleClick(class printopts *self, class environment *env)
{
    int ix;
    struct p_option *rec;

    if (env==self->doneenv) {
	MenuDone(self, 0);
	return;
    }
    else if (env==self->cancelenv) {
	MenuCancel(self, 0);
	return;
    }
    else if (env==self->printenv) {
	MenuPrint(self, 0);
	return;
    }
    else if (env==self->previewenv) {
	MenuPreview(self, 0);
	return;
    }

    for (ix=0; ix<self->numops; ix++) {
	if (env == self->oplist[ix].env)
	    break;
    }
    if (ix==self->numops)
	return;

    rec = (&(self->oplist[ix]));
    if (rec->op->type == A_boolean) {
	boolean newval = (rec->answer[0]!='Y');
	SetNewValue(self, rec, (char *)(newval ? "Yes" : "No"));
    }
    else if (rec->op->type == A_string) {
	char msgbuf[256];
	char bufr[256];
	int res;

	sprintf(msgbuf, "%s:  ", rec->op->label);
	res = message::AskForString(self, 40, msgbuf, rec->answer, bufr, 250); 
	if (res<0 || !strcmp(bufr, rec->answer)) {
	    message::DisplayString(self, 10, "Cancelled.");
	    return;
	}
	SetNewValue(self, rec, bufr);
    }
    else if (rec->op->type == A_file) {
	char bufr[MAXPATHLEN];
	char msgbuf[256];
	int res;

	sprintf(msgbuf, "%s:  ", rec->op->label);

	res = completion::GetFilename(self, msgbuf, rec->answer, bufr, sizeof(bufr), FALSE, FALSE);

	if (res<0 || bufr[strlen(bufr)-1]=='/' || !strcmp(bufr, rec->answer)) {
	    message::DisplayString(self, 10, "Cancelled.");
	    return;
	}
	SetNewValue(self, rec, bufr);
    }
    else if (rec->op->type == A_int) {
	char msgbuf[256];
	char bufr[32];
	int res;
	long newval;

	sprintf(msgbuf, "%s:  ", rec->op->label);
	res = message::AskForString(self, 40, msgbuf, rec->answer, bufr, 30); 
	if (res<0
	    || strlen(bufr)==0
	    || ((newval=atol(bufr))==0 && bufr[0]!='0')) {
	    message::DisplayString(self, 10, "Cancelled.");
	    return;
	}
	sprintf(bufr, "%ld", newval);
	SetNewValue(self, rec, bufr);
    }
    else {
	/* unknown type */
    }
}

class view *printopts::Hit(enum view_MouseAction action, long x, long y, long   num_clicks)
{
    long pos;
    class environment *env;
    class style *style;
    char *stylename;
    long textstart, textlen;

    if (!this->hasInputFocus) {
	this->WantInputFocus(this);
	this->hitenv = FALSE;
    }

    switch (action) {
	case view_LeftDown:
	    pos = this->Locate(x, y, NULL);
	    env = (this)->GetEnclosedStyleInformation( pos, &textlen);
	    while (env) {
		style = env->data.style;
		if (env->type == environment_Style && style) {
		    stylename = style->name;
		    if (stylename && strcmp(stylename, "option") == 0) {
			break;
		    }
		}
		env = (class environment *) env->parent;
	    }
	    this->hitenv = env;
	    break;
	case view_LeftUp:
	    if (!this->hitenv)
		break;
	    pos = this->Locate(x, y, NULL);
	    env = (this)->GetEnclosedStyleInformation( pos, &textlen);
	    while (env) {
		style = env->data.style;
		if (env->type == environment_Style && style) {
		    stylename = style->name;
		    if (stylename && strcmp(stylename, "option") == 0) {
			break;
		    }
		}
		env = (class environment *) env->parent;
	    }
	    if (env && env==this->hitenv) {
		HandleClick(this, env);
	    }
	    this->hitenv = NULL;
	    break;
	default:
	    break;
    }

    return this;
}

view_DSattributes printopts::DesiredSize( long  width, long  height, enum view_DSpass  pass, long  *desiredWidth, long  *desiredHeight ) 
{
    *desiredWidth = 550;
    *desiredHeight = 322;
    DEBUG(("Desired Size %d x %d\n", *desiredWidth, *desiredHeight));
    return view_Fixed;
}

/* We use this to override newline self-insert. */
static void printopts_Nop(class printopts  *self, long  rock)
{
    DEBUG(("printopts_Nop\n"));
}

static void MenuDone(class printopts  *self, long  rock)
{
    DEBUG(("Done with dialog\n"));

    ApplyValues(self);

    im::ForceUpdate();
    im::EnqueueEvent((event_fptr)DestroyWindow, (char *)self, 0);
}

static void MenuCancel(class printopts  *self, long  rock)
{
    DEBUG(("Cancel dialog\n"));

    im::ForceUpdate();
    im::EnqueueEvent((event_fptr)DestroyWindow, (char *)self, 0);
}

static void MenuPrint(class printopts  *self, long  rock)
{
    boolean useps;
    int pmode, res;
    char *fname = NULL;
    char fnamebuf[256];

    DEBUG(("Print document\n"));
    ApplyValues(self);

#ifdef PSPRINTING_ENV
    useps = TRUE;
#else
    useps = FALSE;
#endif

    pmode = (useps ? print_PRINTPOSTSCRIPT : print_PRINTTROFF);

    im::SetProcessCursor(waitCursor);
    message::DisplayString(self, 0, "Processing print request.");
    im::ForceUpdate();

    if (ATK::LoadClass("print") == NULL) {
	message::DisplayString(self, 0, "Print aborted; could not load class \"print\".");
	im::SetProcessCursor(NULL);
	return;
    }
    if (ATK::LoadClass("buffer") != NULL) {
	class buffer *buf;
	class dataobject *dobj = self->opview->GetDataObject();
	if (dobj) {
	    buf = buffer::FindBufferByData(dobj);
	    if (buf)
		fname = buf->GetFilename();
	}
    }

    if (!fname) {
	if (ProgramName[0]) {
	    strcpy(fnamebuf, ProgramName);
	    fname = fnamebuf;
	}
    }

    res = print::ProcessView(self->opview, pmode, 1, fname, "");
    if (res==0) {
	message::DisplayString(self, 0, "Print request submitted.");
    }
    else {
	/* an error occurred -- we shouldn't display any error, because the print module already has. */
    }
    im::SetProcessCursor(NULL);

}

static void MenuPreview(class printopts  *self, long  rock)
{
    boolean useps;
    int pmode, res;
    char *fname = NULL;
    char fnamebuf[256];

    DEBUG(("Print document\n"));
    ApplyValues(self);

#ifdef PSPRINTING_ENV
    useps = TRUE;
#else
    useps = FALSE;
#endif

    pmode = (useps ? print_PREVIEWPOSTSCRIPT : print_PREVIEWTROFF);

    im::SetProcessCursor(waitCursor);
    message::DisplayString(self, 0, "Processing preview request.");
    im::ForceUpdate();

    if (ATK::LoadClass("print") == NULL) {
	message::DisplayString(self, 0, "Preview aborted; could not load class \"print\".");
	im::SetProcessCursor(NULL);
	return;
    }
    if (ATK::LoadClass("buffer") != NULL) {
	class buffer *buf;
	class dataobject *dobj = self->opview->GetDataObject();
	if (dobj) {
	    buf = buffer::FindBufferByData(dobj);
	    if (buf)
		fname = buf->GetFilename();
	}
    }

    if (!fname) {
	if (ProgramName[0]) {
	    strcpy(fnamebuf, ProgramName);
	    fname = fnamebuf;
	}
    }

    res = print::ProcessView(self->opview, pmode, 1, fname, "");
    if (res==0) {
	message::DisplayString(self, 0, "Preview window should appear soon.");
    }
    else {
	/* an error occurred -- we shouldn't display any error, because the print module already has. */
    }
    im::SetProcessCursor(NULL);

}

static void ApplyValues(class printopts  *self)
{
    int ix;
    struct p_option *rec;

    /* now to apply the values */
    for (ix=0; ix<self->numops; ix++) {
	rec = (&(self->oplist[ix]));
	if (rec->op->type == A_int) {
	    long newval = atol(rec->answer);
	    self->opview->SetPrintOption(rec->op, newval);
	}
	else if (rec->op->type == A_boolean) {
	    long newval = (rec->answer[0] == 'Y');
	    self->opview->SetPrintOption(rec->op, newval);
	}
	else if (rec->op->type == A_string || rec->op->type == A_file) {
	    self->opview->SetPrintOption(rec->op, (long)(rec->answer));
	}
    }
}

static void LoadInitValue(class printopts *self, int onum)
{
    struct p_option *rec = (&(self->oplist[onum]));
    long value;
    short gotit;
    char *str;
    int strlength;

    value = self->opview->GetPrintOption(rec->op->name);

    if (rec->op->type == A_boolean) {
	str = (char *)(value ? "Yes" : "No");
    }
    else if (rec->op->type == A_string || rec->op->type == A_file) {
	str = value ? (char *)value : (char *)"";
    }
    else if (rec->op->type == A_int) {
	static char numbuf[12];
	sprintf(numbuf, "%d", value);
	str = numbuf;
    }
    strlength = strlen(str);
    if (strlength < ANSWERLEN) strlength = ANSWERLEN;
    rec->answer = (char *)malloc(strlength+1);
    strcpy(rec->answer, str);
    rec->changed = FALSE;
}

#define BASERESOLUTION 72
#define SCALEWIDTH(v, x) (x*((class view *)(v))->GetHorizontalResolution()/BASERESOLUTION)
#define SCALEHEIGHT(v, y) (y*((class view *)(v))->GetVerticalResolution()/BASERESOLUTION)
#define DIMENSION_X SCALEWIDTH(vp_IM, 350)
#define DIMENSION_Y SCALEHEIGHT(vp_IM, 252)

static void CreateWindow(class view  *callview, long  rock)
{
    long left, top, width, height;
    class printopts *self;
    class text *tx;
    class im *vp_IM, *im;
    class frame *framep;
    struct view_printoptlist *opl, *otmp;
    int ix, num, num2, num3;
    char *label;
    class style *optionstyle, *answerstyle;
    class environment *env;
    class view *vp;
    boolean printtofile;

    vp = callview;
    if (vp->IsType("frame")) {
	vp = ((class frame *)vp)->GetView();
    }
    DEBUG(("target view is %s\n", vp->GetTypeName()));
    opl = vp->PrintOptions();
    if (!opl) {
	message::DisplayString(callview, 10, "This document has no print options.");
	return;
    }

    message::DisplayString(callview, 0, "Creating dialog box.  Please wait.");
    im::ForceUpdate();

    framep = new frame;
    self = new printopts;
    tx = new text;
    if (!self || !tx || !framep) {
	message::DisplayString(callview, 100, "Could not create the options.\nPerhaps we're out of memory?");
	return;
    }
    self->SetDataObject(tx);
    framep->SetView(self->GetApplicationLayer());
    self->parentframe = framep;

    im::GetPreferedDimensions(&left, &top, &width, &height);

    vp_IM = (vp)->GetIM();
    if (vp_IM == NULL || !vp_IM->SupportsOverride()) {
	/*printf ("Attempt to create dialogue box for nonexistent view!\n Creating top level window.");*/
	im::SetPreferedDimensions(left, top, DIMENSION_X, DIMENSION_Y);
	im = im::Create(NULL);
    } else {
	im::SetPreferedDimensions(0, 0, DIMENSION_X, DIMENSION_Y);
	im = im::CreateOverride(vp_IM);
    }

    if (im) {
	(im)->SetTitle( "Print Options");
	(im)->SetView( framep);
    } else {
	message::DisplayString(callview, 100, "Could not create a new window\nPerhaps there isn't enough memory?");
	(self)->Destroy();
	return;
    }

    self->opview = vp;
    self->opobj = vp->GetDataObject();
    /* copy options into self->oplist, reversing order of chain */
    num = 0;
    for (otmp = opl; otmp; otmp = otmp->parent) {
	num += otmp->numoptions;
    }
    self->numops = num;
    self->oplist = (struct p_option *)malloc(num * sizeof(struct p_option));
    for (otmp = opl; otmp; otmp = otmp->parent) {
	num -= otmp->numoptions;
	for (ix=0; ix<otmp->numoptions; ix++) {
	    self->oplist[num+ix].op = (&(otmp->list[ix]));
	    self->oplist[num+ix].env = NULL;
	}
    }

    tx->ReadTemplate("printopt", FALSE);
    answerstyle = tx->GetStyleSheet()->Find("answer");
    optionstyle = tx->GetStyleSheet()->Find("option");
    if (!answerstyle) {
	answerstyle = new style;
	answerstyle->SetName("answer");
	tx->GetStyleSheet()->Add(answerstyle);
    }
    if (!optionstyle) {
	optionstyle = new style;
	optionstyle->SetName("option");
	tx->GetStyleSheet()->Add(optionstyle);
    }
    if (!tx->GetStyleSheet()->Find("hidden")) {
	class style *hiddenstyle = new style;
	hiddenstyle->SetName("hidden");
	tx->GetStyleSheet()->Add(hiddenstyle);
    }

    label = "   Print   Preview   Cancel   Done \n\n";
    num = tx->GetLength();
    tx->InsertCharacters(999999, label, strlen(label));
    num2 = tx->GetLength()-2;
    env = tx->AddStyle(num, num2-num, answerstyle);
    env->SetStyle(FALSE, FALSE);
    env = tx->AddStyle(num+3, 5, optionstyle);
    env->SetStyle(FALSE, FALSE);
    self->printenv = env;
    env = tx->AddStyle(num+11, 7, optionstyle);
    env->SetStyle(FALSE, FALSE);
    self->previewenv = env;
    env = tx->AddStyle(num+21, 6, optionstyle);
    env->SetStyle(FALSE, FALSE);
    self->cancelenv = env;
    env = tx->AddStyle(num+30, 4, optionstyle);
    env->SetStyle(FALSE, FALSE);
    self->doneenv = env;

    self->hiddenenv = NULL;
    printtofile = FALSE;
    self->printnamenum = (-1);
    self->filenamenum = (-1);

    for (ix=0; ix<self->numops; ix++) {
	class atom *ax;
	num = tx->GetLength();
	label = self->oplist[ix].op->label;
	tx->InsertCharacters(999999, label, strlen(label));
	tx->InsertCharacters(999999, ": ", 2);
	num3 = tx->GetLength() - 1;
	LoadInitValue(self, ix);
	label = self->oplist[ix].answer;
	tx->InsertCharacters(999999, label, strlen(label));
	tx->InsertCharacters(999999, " \n", 2);
	ax = self->oplist[ix].op->name;
	if (ax == A_tofile)
	    printtofile = (label[0] == 'Y');
	else if (ax == A_printer)
	    self->printnamenum = ix;
	else if (ax == A_psfile)
	    self->filenamenum = ix;
	num2 = tx->GetLength();
	env = tx->AddStyle(num3, num2-num3, answerstyle);
	env->SetStyle(FALSE, FALSE);
	self->oplist[ix].ansenv = env;
	env = tx->AddStyle(num, num2-num, optionstyle);
	env->SetStyle(FALSE, FALSE);
	self->oplist[ix].env = env;
    }
    AdjustHiddenEnv(self, printtofile);

    tx->SetReadOnly(TRUE);

    im::SetPreferedDimensions(left, top, width, height);
    message::DisplayString(callview, 0, "Done.");
    (self)->WantInputFocus(self);
}   

static void DestroyWindow(class printopts  *self )
{
    class im *pim = self->GetIM();
    class text *tx;
    class view *v;

    if (pim) {
	pim->SetView(NULL);
	pim->Destroy();
    }
    v = NULL;
    if (self->parentframe) {
	v = self->parentframe->GetView();
	self->parentframe->SetView(NULL);
	self->parentframe->Destroy();
    }
    if (self->hasApplicationLayer && v) {
	self->DeleteApplicationLayer(v);
    }
    tx = (class text *)(self->GetDataObject());
    self->Destroy();
    if (tx) tx->Destroy();
}

