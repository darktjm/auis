/* File mtextview.C created by R L Quinn
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

   mtextview, a view for editing Modula-2 code. */

#include <andrewos.h>

static UNUSED const char ibmid[] = "(c) Copyright IBM Corp.  1988-1995.  All rights reserved.";
static UNUSED const char rcsHeader[] = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/srctext/RCS/mtextview.C,v 2.1 1995/02/09 21:57:36 susan Stab74 $";

#include <im.H>
#include <message.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <bind.H>
#include <environ.H> /* for autocut preference */
#include <proctable.H> /* for autocut preference */

#include "mtext.H"
#include "mtextview.H"

/* AutoCut was not made externally visible by txtvcmod, so WE have to check the preference TOO */
static int autocut_mode = -1;	/* uninitialized */
#define IsAutoCutMode() ((autocut_mode == -1 && (autocut_mode = environ::GetProfileSwitch("autocut", FALSE))) || autocut_mode)

static keymap *m_Map;
static menulist *m_Menus;

ATKdefineRegistry(mtextview, modtextview, mtextview::InitializeClass);
static void definition(mtextview *self, long key);
static void implementation(mtextview *self, long key);
static void asterisk(mtextview *self, char key /* must be char for "&" to work. */);

static struct bind_Description mtextBindings[]={
    {"mtextview-asterisk","*",'*', NULL,0, 0, (proctable_fptr)asterisk,"If preceded by an open-paren, start a comment."},
    {"mtextview-display-definition",NULL,0, "Source Text,Display Definition~30", 0,0, (proctable_fptr)definition, "Find the definition module where selected identifier is declared."},
    {"mtextview-display-implementation",NULL,0, "Source Text,Display Implementation~31", 0,0, (proctable_fptr)implementation, "Find the implementation module where selected procedure's code lies."},
    NULL
};

boolean mtextview::InitializeClass()
{
    m_Menus = new menulist;
    m_Map = new keymap;
    bind::BindList(mtextBindings,m_Map,m_Menus,ATK::LoadClass("srctextview"));
    return TRUE;
}

mtextview::mtextview()
{
    ATKinit;
    this->m_state = keystate::Create(this, m_Map);
    this->m_menus = (m_Menus)->DuplicateML(this);
    SetBorder(5,5); 
    THROWONFAILURE(TRUE);
}

mtextview::~mtextview()
{
    ATKinit;
    delete this->m_state;
    delete this->m_menus;
}

void mtextview::PostMenus(menulist *menulist)
{
    (this->m_menus)->ChainBeforeML(menulist, (long)this);
    (this)->modtextview::PostMenus(this->m_menus);
}

class keystate *mtextview::PrependKeyState()
{
    this->m_state->next= NULL;
    return (this->m_state)->AddBefore((this)->modtextview::PrependKeyState());
}

/* FindDefinitionOrImplementation() isolates the identifier pointed to by the caret and tries to find its corresponding definition module (if implementation is FALSE) or implementation module (if implementation is TRUE) */
static void FindDefinitionOrImplementation(mtextview *self, boolean implementation)
{
    mtext *ct = (mtext *)self->view::dataobject;
    long pos,oldpos;
    char name[256],proc[256],msg[300];
    char filename[256],bufname[256],searchpath[1024];

    /*extract module name from document*/
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
	if (implementation)
	    strcat(filename,".mod");
	else
	    strcat(filename,".def");
	/*find buffer name*/
	strcpy(bufname,name);
	if (implementation)
	    strcat(bufname,"-implementation");
	else
	    strcat(bufname,"-definition");
	/*find search path (environment variables MPath,m2mi, and m2mp, and DEFAULT_SYM_DIR)*/
	strcpy(searchpath,getenv("MPath")); strcat(searchpath,":");
	strcat(searchpath,getenv("m2mi")); strcat(searchpath,":");
	strcat(searchpath,getenv("m2mp"));
#ifdef DEFAULT_SYM_DIR
	strcat(searchpath,":");
	strcat(searchpath,DEFAULT_SYM_DIR);
#else /*DEFAULT_SYM_DIR*/
	message::DisplayString(self, 0, "Your Makefile should define a DEFAULT_SYM_DIR.");
#endif /*DEFAULT_SYM_DIR*/
	if (!(self)->FindSubFile(filename, bufname,proc, searchpath)) {
	    msg[0]='\0';
	    sprintf(msg, "Couldn't find %s.", filename);
	    message::DisplayString(self, 0, msg);
	}
    } else {
	if (implementation)
	    message::DisplayString(self, 0, "Cursor must be on an implementation name.");
	else
	    message::DisplayString(self, 0, "Cursor must be on a definition name.");
    }
    (ct)->NotifyObservers(0);
}

static void definition(mtextview *self, long key)
{
    FindDefinitionOrImplementation(self,FALSE);
}

static void implementation(mtextview *self, long key)
{
    FindDefinitionOrImplementation(self,TRUE);
}

/* any modifications to asterisk should be duplicated in m3textv.c */
static void asterisk(mtextview *self, char key /* must be char for "&" to work. */)
{
    mtext *ct=(mtext *)self->view::dataobject;
    int count=((self)->GetIM())->Argument();
    long pos,oldpos;

    if ((self)->ConfirmReadOnly())
	return;
    if (IsAutoCutMode() && (self)->GetDotLength()>0)
	((self)->GetIM())->HandleMenu(proctable::Lookup("textview-zap-region"), self, 0); /* not a particularly efficient way to call textview_ZapRegionCmd, but what else ya gonna do? */
    oldpos= pos= (self)->CollapseDot();
    while (count--) (ct)->InsertCharacters(pos++,&key,1);
    if (oldpos && (ct)->GetChar(oldpos-1)=='(' && !(ct)->GetStyle(oldpos-1) && !(ct)->InString(oldpos))
	(ct)->WrapStyleNow(oldpos-1,pos-oldpos+1, ct->srctext::comment_style, FALSE,TRUE);
    (self)->SetDotPosition(pos);
    (ct)->NotifyObservers(0);
}
