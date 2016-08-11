/* File modtextview.C created by R L Quinn
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

   modtextview, a view for editing Modula-x code. */

static char ibmid[] = "(c) Copyright IBM Corp.  1988-1995.  All rights reserved.";
static char rcsHeader[] = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/srctext/RCS/modtextview.C,v 2.0 1995/01/27 19:38:38 rr2b Stab74 $";

#include <andrewos.h>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <bind.H>
#include <environment.H> /* for comment insertion */
#include <im.H>
#include <environ.H> /* for autocut preference */
#include <proctable.H> /* for autocut preference */

#include "modtext.H"
#include "modtextview.H"

/* AutoCut was not made externally visible by txtvcmod, so WE have to check the preference TOO */
static int autocut_mode = -1;	/* uninitialized */
#define IsAutoCutMode() ((autocut_mode == -1 && (autocut_mode = environ::GetProfileSwitch("autocut", FALSE))) || autocut_mode)

static keymap *mod_Map;
static menulist *mod_Menus;

ATKdefineRegistry(modtextview, srctextview, modtextview::InitializeClass);
static void startPreproc(modtextview *self, char key);

static struct bind_Description modtextBindings[]={
    {"modtextview-start-preproc","#",'#', NULL,0,0, (proctable_fptr)startPreproc, "Start preprocessor style if pressed at start of line."},
    {"srctextview-self-insert-reindent","|",'|'},
    {"srctextview-style-string","\"",'"'},
    {"srctextview-style-string","'",'\''},
    NULL
};

boolean modtextview::InitializeClass()
{
    mod_Menus = new menulist;
    mod_Map = new keymap;
    bind::BindList (modtextBindings,mod_Map,mod_Menus,ATK::LoadClass("srctextview"));
    return TRUE;
}

modtextview::modtextview()
{
    ATKinit;
    this->mod_state = keystate::Create(this, mod_Map);
    this->mod_menus = (mod_Menus)->DuplicateML(this);
    SetBorder(5,5); 
    THROWONFAILURE(TRUE);
}

modtextview::~modtextview()
{
    ATKinit;
    delete this->mod_state;
    delete this->mod_menus;
}

void modtextview::PostMenus(menulist *menulist)
{
    (this->mod_menus)->ChainBeforeML(menulist, (long)this);
    (this)->srctextview::PostMenus(this->mod_menus);
}

class keystate *modtextview::PrependKeyState()
{
    this->mod_state->next= NULL;
    return (this->mod_state)->AddBefore((this)->srctextview::PrependKeyState());
}

/* override */
void modtextview::Paren(char key /* must be char for "&" to work. */)
{
    if (key=='}')
	SelfInsertReindent(key);
    else if (key==')') {
	modtext *ct=(modtext *)this->view::dataobject;
	long openparen,oldpos=GetDotPosition();
	SelfInsert(key);
	openparen= (ct)->ReverseBalance(GetDotPosition());
	if (oldpos && ((ct)->GetChar(oldpos-1)=='*') && ((ct)->GetStyle(oldpos-1)==ct->srctext::comment_style))
	    if ((ct)->GetStyle(openparen) != ct->srctext::comment_style) /* only stops italicizing if matching "(*" was NOT italicized (not nested) */
		((ct)->GetEnvironment(oldpos))->SetStyle(FALSE,FALSE);
    } else
	SelfInsert(key);
    MatchParens(key);
}

/* override */
/* HandleEndOfLineStyle will terminate both linecomment AND preprocessor styles when a newline is added (if not \quoted) */
void modtextview::HandleEndOfLineStyle(long pos)
{
    modtext *ct=(modtext *)this->view::dataobject;
    if (ct->preprocessor) {
	if ((ct)->GetStyle(pos)==ct->srctext::linecomment_style || ((ct)->GetStyle(pos)==ct->srctext::kindStyle[PREPRC] && !(ct)->Quoted(pos))) {
	    long start=pos;
	    while ((ct)->GetChar(start-1)=='\n' && !(ct)->Quoted(start-1)) --start;
	    (ct->text::rootEnvironment)->Remove(start,pos-start+1, environment_Style, FALSE);
	}
    } else
	(this)->srctextview::HandleEndOfLineStyle(pos);
}

static void startPreproc(modtextview *self, char key)
{
    modtext *ct=(modtext *)self->view::dataobject;
    long pos, oldpos;
    if ((self)->ConfirmReadOnly())
	return;
    if (IsAutoCutMode() && (self)->GetDotLength()>0)
	((self)->GetIM())->HandleMenu(proctable::Lookup("textview-zap-region"), self, 0); /* not a particularly efficient way to call textview_ZapRegionCmd, but what else ya gonna do? */
    pos= oldpos= (self)->CollapseDot();
    while (pos>0 && is_whitespace((ct)->GetChar(--pos))) ;
    if (ct->preprocessor && (pos==0 || ((ct)->GetChar(pos)=='\n' && !(ct)->Quoted(pos))) && !(ct)->GetStyle(oldpos)) {
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
