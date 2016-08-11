/* File ctextview.C created by R L Quinn
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

   ctextview, a view for editing C code. */

static char ibmid[] = "(c) Copyright IBM Corp.  1988-1995.  All rights reserved.";
static char rcsHeader[] = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/srctext/RCS/ctextview.C,v 2.0 1995/01/27 19:38:38 rr2b Stab74 $";

#include <andrewos.h>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <bind.H>
#include <environment.H>
#include <im.H>
#include <environ.H> /* for autocut preference */
#include <proctable.H> /* for autocut preference */

#include "ctext.H"
#include "ctextview.H"
#include "srctext.H"

/* AutoCut was not made externally visible by txtvcmod, so WE have to check the preference TOO */
static int autocut_mode = -1;	/* uninitialized */
#define IsAutoCutMode() ((autocut_mode == -1 && (autocut_mode = environ::GetProfileSwitch("autocut", FALSE))) || autocut_mode)

static keymap *c_Map;
static menulist *c_Menus;

ATKdefineRegistry(ctextview, srctextview, ctextview::InitializeClass);
static void startPreproc(ctextview *self, char key);

static struct bind_Description ctextBindings[]={
    {"ctextview-start-preproc","#",'#', NULL,0,0, (proctable_fptr)startPreproc, "Start preprocessor style if pressed at start of line."},
    {"srctextview-self-insert-reindent","{",'{'},
    {"srctextview-start-comment", "*",'*'},
    {"srctextview-end-comment", "/",'/'},
    {"srctextview-style-label",":",':'},
    {"srctextview-style-string","\"",'"'},
    {"srctextview-style-string","'",'\''},
    NULL
};

boolean ctextview::InitializeClass()
{
    c_Menus = new menulist;
    c_Map = new keymap;
    bind::BindList(ctextBindings,c_Map,c_Menus,ATK::LoadClass("srctextview"));
    return TRUE;
}

ctextview::ctextview()
{
    ATKinit;
    this->c_state = keystate::Create(this, c_Map);
    this->c_menus = (c_Menus)->DuplicateML(this);
    SetBorder(5,5);
    THROWONFAILURE(TRUE);
}

ctextview::~ctextview()
{
    ATKinit;
    delete this->c_state;
    delete this->c_menus;
}

void ctextview::PostMenus(menulist *menulist)
{
    (this->c_menus)->ChainBeforeML(menulist, (long)this);
    (this)->srctextview::PostMenus(this->c_menus);
}

keystate *ctextview::PrependKeyState()
{
    this->c_state->next= NULL;
    return (this->c_state)->AddBefore((this)->srctextview::PrependKeyState());
}

/* override */
/* Paren is overridden so that an end-brace will reindent the line */
void ctextview::Paren(char key /* must be char for "&" to work. */)
{
    ctext *ct = (ctext *)this->view::dataobject;
    long pos=GetDotPosition();
    if (key=='}')
	SelfInsertReindent(key);
    else
	SelfInsert(key);
    if (!(ct)->InCommentStart(pos) && !(ct)->InString(pos))
	MatchParens(key);
}

/* override */
/* HandleEndOfLineStyle will terminate both linecomment AND preprocessor styles when a newline is added (if not \quoted) */
void ctextview::HandleEndOfLineStyle(long pos)
{
    ctext *ct=(ctext *)this->view::dataobject;

    if ((ct)->GetStyle(pos)==ct->srctext::linecomment_style || ((ct)->GetStyle(pos)==ct->srctext::kindStyle[PREPRC] && !(ct)->Quoted(pos))) {
	long start=pos;
	while ((ct)->GetChar(start-1)=='\n' && !(ct)->Quoted(start-1)) --start;
	(ct->text::rootEnvironment)->Remove(start,pos-start+1, environment_Style, FALSE);
    }
}

static void startPreproc(ctextview *self, char key)
{
    ctext *ct=(ctext *)self->view::dataobject;
    long pos, oldpos;
    if ((self)->ConfirmReadOnly())
	return;
    if (IsAutoCutMode() && (self)->GetDotLength()>0)
	((self)->GetIM())->HandleMenu(proctable::Lookup("textview-zap-region"), self, 0); /* not a particularly efficient way to call textview_ZapRegionCmd, but what else ya gonna do? */
    pos= oldpos= (self)->CollapseDot();
    while (pos>0 && is_whitespace((ct)->GetChar(--pos))) ;
    if ((pos==0 || ((ct)->GetChar(pos)=='\n' && !(ct)->Quoted(pos))) && !(ct)->GetStyle(oldpos)) {
	int count=((self)->GetIM())->Argument();
	pos= oldpos;
	while (count--) (ct)->InsertCharacters(pos++,&key,1);
	(ct)->WrapStyleNow(oldpos,pos-oldpos, ct->srctext::kindStyle[PREPRC], FALSE,TRUE);
	(self)->SetDotPosition(pos);
	if ((ct)->IndentingEnabled())
	    (ct)->ReindentLine(pos);
	(ct)->NotifyObservers(0);
    } else /* not the start of a preprocessor line */
	(self)->SelfInsert(key);
}