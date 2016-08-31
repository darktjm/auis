/* File rexxtextview.C created by R S Kemmetmueller
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

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

   rexxtextview, a view for editing REXX code. */

#include <andrewos.h>

static UNUSED const char ibmid[] = "(c) Copyright IBM Corp.  1988-1995.  All rights reserved.";

#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <bind.H>
#include <im.H>
#include <environment.H>
#include <environ.H> /* for autocut preference */
#include <proctable.H> /* for autocut preference */

#include "rexxtext.H"
#include "rexxtextview.H"

/* AutoCut was not made externally visible by txtvcmod, so WE have to check the preference TOO */
static int autocut_mode = -1;	/* uninitialized */
#define IsAutoCutMode() ((autocut_mode == -1 && (autocut_mode = environ::GetProfileSwitch("autocut", FALSE))) || autocut_mode)

static keymap *rexx_Map;
static menulist *rexx_Menus;

static struct bind_Description rexxtextBindings[]={
    {"srctextview-style-label",":",':'},
    {"srctextview-start-comment", "*",'*'},
    {"srctextview-end-comment", "/",'/'},
    {"srctextview-style-string","\"",'"'},
    {"srctextview-style-string","'",'\''},
    NULL
};

ATKdefineRegistry(rexxtextview, srctextview, rexxtextview::InitializeClass);

boolean rexxtextview::InitializeClass()
{
    rexx_Menus = new menulist;
    rexx_Map = new keymap;
    bind::BindList(rexxtextBindings,rexx_Map,rexx_Menus,ATK::LoadClass("srctextview"));
    return TRUE;
}

rexxtextview::rexxtextview()
{
    ATKinit;
    this->rexx_state = keystate::Create(this, rexx_Map);
    this->rexx_menus = (rexx_Menus)->DuplicateML(this);
    SetBorder(5,5);
    THROWONFAILURE(TRUE);
}

rexxtextview::~rexxtextview()
{
    ATKinit;
    delete this->rexx_state;
    delete this->rexx_menus;
}

void rexxtextview::PostMenus(menulist *menulist)
{
    (this->rexx_menus)->ChainBeforeML(menulist, (long)this);
    (this)->srctextview::PostMenus(this->rexx_menus);
}

class keystate *rexxtextview::PrependKeyState()
{
    this->rexx_state->next= NULL;
    return (this->rexx_state)->AddBefore((this)->srctextview::PrependKeyState());
}

/* matchingdelim finds the slash-star that matches the star-slash at pos, taking nesting into account */
static long matchingdelim(rexxtext *ct, long pos)
{
    int delimcount=1;
    while (--pos>0) {
	if ((ct)->GetChar(pos)=='*')
	    if ((ct)->GetChar(pos-1)=='/') {
		/* found start delimiter */
		if (--delimcount < 1)
		    /* found match */
		    return pos-1;
	    } else if ((ct)->GetChar(pos+1)=='/')
		++delimcount;
    }
    return 0;
}

/* override */
/* rexxtextview_EndComment handles NESTED slash-star comments */
void rexxtextview::EndComment(char key /* must be char for "&" to work. */)
{
    rexxtext *ct=(rexxtext *)this->view::dataobject;
    int count=(GetIM())->Argument();
    long pos,oldpos;
    if (ConfirmReadOnly())
	return;
    if (IsAutoCutMode() && GetDotLength()>0)
	(GetIM())->HandleMenu(proctable::Lookup("textview-zap-region"), this, 0); /* not a particularly efficient way to call textview_ZapRegionCmd, but what else ya gonna do? */
    oldpos= pos= CollapseDot();
    while (count--) (ct)->InsertCharacters(pos++,&key,1);
    if (oldpos && (ct)->GetChar(oldpos-1)=='*')
	if ((ct)->GetStyle(oldpos+1)==ct->srctext::comment_style) {
	    /* terminate existing style IF this isn't a NESTED star-slash */
	    if ((ct)->GetStyle(matchingdelim(ct,oldpos-1)) != ct->srctext::comment_style)
		((ct->text::rootEnvironment)->GetEnclosing(oldpos+1))->SetStyle(FALSE,FALSE);
	} else if (!(ct)->GetStyle(oldpos)) {
	    /* wrap a new style */
	    long start=matchingdelim(ct, oldpos-1);
	    boolean isnested=((ct)->GetStyle(start) == ct->srctext::comment_style);
	    if ((ct)->GetStyle(start+1) && (ct)->GetStyle(start+1) != ct->srctext::comment_style)
		/*NOP*/; /* inside some unidentifed style, like a string. abort */
	    else
		(ct)->WrapStyleNow(start,oldpos-start+1, ct->srctext::comment_style, FALSE,isnested);
	}
    SetDotPosition(pos);
    FrameDot(pos);
    (ct)->NotifyObservers(0); 
}
