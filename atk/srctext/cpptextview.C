/* File cpptextview.C created by R L Quinn
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   cpptextview, a view for editing C++ code. */

#include <andrewos.h>

static UNUSED const char ibmid[] = "(c) Copyright IBM Corp.  1988-1995.  All rights reserved.";

#include <im.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <bind.H>
#include <environment.H> /* for comment insertion */
#include <environ.H> /* for autocut preference */
#include <proctable.H> /* for autocut preference */

#include "cpptext.H"
#include "cpptextview.H"

/* AutoCut was not made externally visible by txtvcmod, so WE have to check the preference TOO */
static int autocut_mode = -1;	/* uninitialized */
#define IsAutoCutMode() ((autocut_mode == -1 && (autocut_mode = environ::GetProfileSwitch("autocut", FALSE))) || autocut_mode)

static keymap *cpp_Map;
static menulist *cpp_Menus;

ATKdefineRegistry(cpptextview, ctextview, cpptextview::InitializeClass);
static void slash(cpptextview *self, long key);

static struct bind_Description cpptextBindings[]={
    {"cpptextview-slash","/",'/',NULL,0,0,(proctable_fptr)slash,"Insert a slash, possibly an end comment delimiter or the start of a line comment."},
    NULL
};

boolean cpptextview::InitializeClass()
{
    cpp_Menus = new menulist;
    cpp_Map = new keymap;
    bind::BindList(cpptextBindings,cpp_Map,cpp_Menus,ATK::LoadClass("srctextview"));
    return TRUE;
}

cpptextview::cpptextview()
{
    ATKinit;
    this->cpp_state = keystate::Create(this, cpp_Map);
    this->cpp_menus = (cpp_Menus)->DuplicateML(this);
    SetBorder(5,5);
    THROWONFAILURE(TRUE);
}

cpptextview::~cpptextview()
{
    ATKinit;
    delete this->cpp_state;
    delete this->cpp_menus;
}

void cpptextview::PostMenus(menulist *menulist) 
{
    (this->cpp_menus)->ChainBeforeML(menulist, (long)this);
    (this)->ctextview::PostMenus(this->cpp_menus);
}

keystate *cpptextview::PrependKeyState()
{
    this->cpp_state->next= NULL;
    return (this->cpp_state)->AddBefore((this)->ctextview::PrependKeyState());
}

/* slash will END an existing comment style if preceded by an asterisk, or start a line comment style if preceded by another slash */
static void slash(cpptextview *self, long key)
{
    cpptext *ct=(cpptext *)self->view::GetDataObject();
    int count=((self)->GetIM())->Argument();
    long pos,oldpos;
    if ((self)->ConfirmReadOnly())
	return;
    if (IsAutoCutMode() && (self)->GetDotLength()>0)
	((self)->GetIM())->HandleMenu(proctable::Lookup("textview-zap-region"), self, 0); /* not a particularly efficient way to call textview_ZapRegionCmd, but what else ya gonna do? */
    oldpos= pos= (self)->CollapseDot();
    char ckey = key;
    while (count--) (ct)->InsertCharacters(pos++, &ckey, 1);
    if (oldpos && (ct)->GetChar(oldpos-1)=='*')
	if ((ct)->GetStyle(oldpos+1)==ct->srctext::comment_style)
	    /* terminate existing style */
	    ((ct->text::rootEnvironment)->GetEnclosing(oldpos+1))->SetStyle(FALSE,FALSE);
	else {
	    /* wrap a new style */
	    long start=oldpos-1;
	    while (--start>0) {
		if ((ct)->GetChar(start)=='*') {
		    if ((ct)->GetChar(start-1)=='/') {
			/* found start of comment, wrap style */
			if ((ct)->GetStyle(start) && (ct)->GetStyle(start)!=ct->srctext::comment_style)
			    /* must be inside a preprocessor directive or some other unknown style. abort */
			    break;
			--start;
			if ((ct)->InString(start))
			    break;
			(ct)->WrapStyleNow(start,oldpos-start+1, ct->srctext::comment_style, FALSE,FALSE);
			break;
		    } else if ((ct)->GetChar(start+1)=='/')
			/* uh-oh, found another end of comment! */
			break;
		}
	    }
	}
    else if (oldpos && (ct)->GetChar(oldpos-1)=='/' && !(ct)->GetStyle(oldpos-1) && !(ct)->InString(pos))
	(ct)->WrapStyleNow(oldpos-1,pos-oldpos+1, ct->srctext::linecomment_style, FALSE,TRUE);
    (self)->SetDotPosition(pos);
    (self)->FrameDot(pos);
    (ct)->NotifyObservers(0);
}
