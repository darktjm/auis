/* File m3textview.C created by R L Quinn
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

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

   m3textview, a view for editing Modula-3 code. */

#include <andrewos.h>

static UNUSED const char ibmid[] = "(c) Copyright IBM Corp.  1988-1995.  All rights reserved.";

#include <im.H>
#include <message.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <bind.H>
#include <environment.H>
#include <environ.H> /* for autocut preference */
#include <proctable.H> /* for autocut preference */

#include "m3text.H"
#include "m3textview.H"

/* AutoCut was not made externally visible by txtvcmod, so WE have to check the preference TOO */
static int autocut_mode = -1;	/* uninitialized */
#define IsAutoCutMode() ((autocut_mode == -1 && (autocut_mode = environ::GetProfileSwitch("autocut", FALSE))) || autocut_mode)

static class keymap *m3_Map;
static class menulist *m3_Menus;

ATKdefineRegistry(m3textview, modtextview, m3textview::InitializeClass);
static void interface(m3textview *self, long key);
static void module(m3textview *self, long key);
static void asterisk(m3textview *self, char key);
static void m3pragma(m3textview *self, char key);

static struct bind_Description m3textBindings[]={
    {"m3textview-asterisk","*",'*', NULL,0, 0, (proctable_fptr)asterisk,""},
    {"m3textview-pragma",">",'>', NULL,0, 0, (proctable_fptr)m3pragma, ""},
    {"m3textview-display-interface",NULL,0, "Source Text,Display Interface~30", 0,0, (proctable_fptr)interface, "Find the interface where selected identifier is declared."},
    {"m3textview-display-module",NULL,0, "Source Text,Display Module~31", 0,0, (proctable_fptr)module, "Find the module where selected procedure's code lies."},
    NULL
};

boolean m3textview::InitializeClass()
{
    m3_Menus = new menulist;
    m3_Map = new keymap;
    bind::BindList(m3textBindings,m3_Map,m3_Menus,ATK::LoadClass("srctextview"));
    return TRUE;
}

m3textview::m3textview()
{
    ATKinit;
    this->m3_state = keystate::Create(this, m3_Map);
    this->m3_menus = (m3_Menus)->DuplicateML(this);
    SetBorder(5,5); 
    THROWONFAILURE(TRUE);
}

m3textview::~m3textview()
{
    ATKinit;
    delete this->m3_state;
    delete this->m3_menus;
}

void m3textview::PostMenus(menulist *menulist)
{
    (this->m3_menus)->ChainBeforeML(menulist, (long)this);
    (this)->modtextview::PostMenus(this->m3_menus);
}

class keystate *m3textview::PrependKeyState()
{
    this->m3_state->next= NULL;
    return (this->m3_state)->AddBefore((this)->modtextview::PrependKeyState());
}

/* FindInterfaceOrModule() isolates the identifier pointed to by the caret and tries to find its corresponding interface (if module is FALSE) or module (if module is TRUE) */
static void FindInterfaceOrModule(m3textview *self, boolean module)
{
    m3text *ct = (m3text *)self->view::dataobject;
    long pos,oldpos;
    char name[256],proc[256],msg[300];
    char filename[256],bufname[256],searchpath[1024];

    /*extract interface/module name from document*/
    (self)->CollapseDot();
    pos=oldpos=(self)->GetDotPosition();
    while (pos>0 && (ct)->IsTokenOrPeriod((ct)->GetChar(pos-1))) --pos;
    oldpos=pos;
    while ((ct)->IsTokenChar(name[pos-oldpos]=(ct)->GetChar(pos)) && pos-oldpos<256) ++pos;
    name[pos-oldpos]='\0';
    /*extract procedure name if present*/
    if ((ct)->GetChar(pos)=='.') {
	oldpos=(++pos);
	while ((ct)->IsTokenChar(proc[pos-oldpos]=(ct)->GetChar(pos)) && pos-oldpos<256) ++pos;
	proc[pos-oldpos]='\0';
    } else
	proc[0]='\0';
    if (strlen(name)>0) {
	/*find filename*/
	strcpy(filename,name);
	if (module)
	    strcat(filename,"Impl.m3");
	else
	    strcat(filename,".i3");
	/*find buffer name*/
	strcpy(bufname,name);
	if (module)
	    strcat(bufname,"-module");
	else
	    strcat(bufname,"-interface");
	/*find search path (environment variable M3Path & DEFAULT_SY_DIR)*/
	strcpy(searchpath,getenv("M3Path"));
#ifdef DEFAULT_SY_DIR
	strcat(searchpath,":");
	strcat(searchpath,DEFAULT_SY_DIR);
#else /*DEFAULT_SY_DIR*/
	message::DisplayString(self, 0, "Your Makefile should define a DEFAULT_SY_DIR.");
#endif /*DEFAULT_SY_DIR*/
	if (!(self)->FindSubFile(filename, bufname,proc, searchpath)) {
	    msg[0]='\0';
	    sprintf(msg, "Couldn't find %s.", filename);
	    message::DisplayString(self, 0, msg);
	}
    } else {
	if (module)
	    message::DisplayString(self, 0, "Cursor must be on a module name.");
	else
	    message::DisplayString(self, 0, "Cursor must be on an import name.");
    }
    (ct)->NotifyObservers(0);
}

static void interface(m3textview *self, long key)
{
    FindInterfaceOrModule(self,FALSE);
}

static void module(m3textview *self, long key)
{
    FindInterfaceOrModule(self,TRUE);
}

/* identical to mtext's asterisk() with the exception of pragma-checking */
static void asterisk(m3textview *self, char key /* must be char for "&" to work. */)
{
    m3text *ct=(m3text *)self->view::dataobject;
    int count=((self)->GetIM())->Argument();
    long pos,oldpos;

    if ((self)->ConfirmReadOnly())
	return;
    if (IsAutoCutMode() && (self)->GetDotLength()>0)
	((self)->GetIM())->HandleMenu(proctable::Lookup("textview-zap-region"), self, 0); /* not a particularly efficient way to call textview_ZapRegionCmd, but what else ya gonna do? */
    oldpos= pos= (self)->CollapseDot();
    while (count--) (ct)->InsertCharacters(pos++,&key,1);
    if (oldpos && !(ct)->GetStyle(oldpos-1) && !(ct)->InString(oldpos)) {
	if((ct)->GetChar(oldpos-1)=='(')
	    (ct)->WrapStyleNow(oldpos-1,pos-oldpos+1, ct->srctext::comment_style, FALSE,TRUE);
	else if((ct)->GetChar(oldpos-1)=='<')
	    (ct)->WrapStyleNow(oldpos-1,pos-oldpos+1, ct->srctext::kindStyle[PRAGMA], FALSE,TRUE);
    }
    (self)->SetDotPosition(pos);
    (ct)->NotifyObservers(0);
}

/* m3pragma should be functionally equivalent to paren function in modtextv.c */
static void m3pragma(m3textview *self, char key /* must be char for "&" to work. */)
{
    m3text *ct=(m3text *)self->view::dataobject;
    long openpragma,oldpos=(self)->GetDotPosition();
    if ((self)->ConfirmReadOnly())
	return;
    (self)->SelfInsert(key);
    openpragma= (ct)->ReverseBalance((self)->GetDotPosition());
    if (oldpos && ((ct)->GetChar(oldpos-1)=='*') && ((ct)->GetStyle(oldpos-1)==ct->srctext::kindStyle[PRAGMA]))
	((ct->text::rootEnvironment)->GetEnclosing(oldpos))->SetStyle(FALSE,FALSE);
    (ct)->NotifyObservers(0);
}
