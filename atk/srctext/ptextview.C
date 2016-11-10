/* File ptextview.C
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   ptextview, a view for editing Pascal code. */

#include <andrewos.h>

static UNUSED const char ibmid[] = "(c) Copyright IBM Corp.  1988-1995.  All rights reserved.";

#include <ctype.h>

#include <im.H>
#include <message.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <bind.H>
#include <environment.H>
#include <environ.H> /* for autocut preference */
#include <proctable.H> /* for autocut preference */

#include "ptext.H"
#include "ptextview.H"
#include "srctext.H"

/* AutoCut was not made externally visible by txtvcmod, so WE have to check the preference TOO */
static int autocut_mode = -1;	/* uninitialized */
#define IsAutoCutMode() ((autocut_mode == -1 && (autocut_mode = environ::GetProfileSwitch("autocut", FALSE))) || autocut_mode)

static keymap *M_Map;
static menulist *M_Menus;

ATKdefineRegistry(ptextview, srctextview, ptextview::InitializeClass);
static void braceslash(ptextview *self, long key);
static void asterisk(ptextview *self, long key);

static struct bind_Description ptextBindings[]={
    {"ptextview-brace","}",'}', NULL,0, 0, (proctable_fptr)braceslash,""},
    {"ptextview-slash","/",'/', NULL,0,0, (proctable_fptr)braceslash,""},
    {"ptextview-asterisk","*",'*', NULL,0, 0, (proctable_fptr)asterisk,""},
    {"ptextview-startbrace","{",'{',NULL,0,0, (proctable_fptr)asterisk,""},
    NULL
};

boolean ptextview::InitializeClass()
{
    M_Menus = new menulist;
    M_Map = new keymap;
    bind::BindList(ptextBindings,M_Map,M_Menus, ATK::LoadClass("srctextview"));
    return TRUE;
}

ptextview::ptextview()
{
    ATKinit;
    this->c_state = keystate::Create(this, M_Map);
    this->c_menus = (M_Menus)->DuplicateML(this);
    SetBorder(5,5);
    THROWONFAILURE(TRUE);
}

ptextview::~ptextview()
{
    ATKinit;
    delete this->c_state;
    delete this->c_menus;
}

class keystate *ptextview::PrependKeyState()
{
    this->c_state->next= NULL;
    return (this->c_state)->AddBefore((this)->srctextview::PrependKeyState());
}


void ptextview::PostMenus(menulist *menulist)
{
    (this->c_menus)->ChainBeforeML(menulist, (long)this);
    (this)->srctextview::PostMenus(this->c_menus);
}

void ptextview::Paren(char key /* must be char for "&" to work. */)
{
    ptext *pt = (ptext *)GetDataObject();
    long oldpos, pos;

    oldpos = GetDotPosition();
    pos = oldpos+1;
    if (pos)
	(pt)->BackwardCheckWord(pos - 1, 0);

    SelfInsert(key);

    if (oldpos&&((pt)->GetChar(oldpos-1)=='*')) {
	if ((pt)->GetStyle(oldpos-1) == pt->srctext::comment_style)
	    ((pt->text::rootEnvironment)->GetEnclosing(oldpos+1))->SetStyle(FALSE, FALSE);
	else {
	    /* wrap a new style */
	    long start=oldpos-1;
	    while (--start>0) {
		if ((pt)->GetChar(start)=='*') {
		    if ((pt)->GetChar(start-1)=='(') {
			/* found start of comment, wrap style */
			--start;
			if ((pt)->InString(start))
			    break;
			(pt)->WrapStyleNow(start, oldpos-start+1, pt->srctext::comment_style, FALSE,FALSE);
			break;
		    } else if ((pt)->GetChar(start+1)==')')
			/* uh-oh, found another end of comment! */
			break;
		}
	    }
	}
    } else
	MatchParens(key);
    (pt)->NotifyObservers(0);
}

static void braceslash(ptextview *self, long key /* must be char for "&" to work. */)
{
    ptext *pt = (ptext *)(self)->GetDataObject();
    int count = ((self)->GetIM())->Argument(), pos, oldpos;

    if (IsAutoCutMode() && (self)->GetDotLength()>0)
	((self)->GetIM())->HandleMenu(proctable::Lookup("textview-zap-region"), self, 0); /* not a particularly efficient way to call textview_ZapRegionCmd, but what else ya gonna do? */
    oldpos = pos = (self)->CollapseDot();
    char ckey = key;
    while (count--) (pt)->InsertCharacters(pos++,&ckey,1);
    if (key == '}') 
	if((pt)->GetStyle(oldpos) == pt->srctext::comment_style)
	    ((pt->text::rootEnvironment)->GetEnclosing(oldpos))->SetStyle(FALSE, FALSE);
	else {
	    /* wrap a new style */
	    long start=oldpos;
	    while (--start>0) {
		if ((pt)->GetChar(start)=='{') {
		    /* found start of comment, wrap style */
		    if ((pt)->InString(start))
			break;
		    (pt)->WrapStyleNow(start, oldpos-start+1, pt->srctext::comment_style, FALSE, FALSE);
		    break;
		} else if ((pt)->GetChar(start)=='}')
		    /* uh-oh, found another end of comment! */
		    break;
	    }
	}
    else if (key == '/') {
	if((pt)->GetStyle(oldpos) == pt->srctext::comment_style)
	    ((pt->text::rootEnvironment)->GetEnclosing(oldpos))->SetStyle(FALSE, FALSE);
	else {
	    /* wrap a new style */
	    long start=oldpos-1;
	    while (--start>0) {
		if ((pt)->GetChar(start)=='*') {
		    if ((pt)->GetChar(start-1)=='/') {
			/* found start of comment, wrap style */
			--start;
			if ((pt)->InString(start))
			    break;
			(pt)->WrapStyleNow(start, oldpos-start+1, pt->srctext::comment_style, FALSE, FALSE);
			break;
		    } else if ((pt)->GetChar(start+1)=='/')
			/*uh-oh, found another end of comment! */
			break;
		}
	    }
	}
    }
    (self)->SetDotPosition(pos);
    (self)->FrameDot(pos);
    (pt)->NotifyObservers(0);
}

static void asterisk(ptextview *self, long key /* must be char for "&" to work. */)
{
    ptext *pt = (ptext *)(self)->GetDataObject();
    int count = ((self)->GetIM())->Argument(), oldpos, pos = (self)->GetDotPosition();

    if (IsAutoCutMode() && (self)->GetDotLength()>0)
	((self)->GetIM())->HandleMenu(proctable::Lookup("textview-zap-region"), self, 0); /* not a particularly efficient way to call textview_ZapRegionCmd, but what else ya gonna do? */
    oldpos=pos = (self)->CollapseDot();

    char ckey = key;
    while (count--) (pt)->InsertCharacters(pos++,&ckey,1);

    if (key == '{')
	(pt)->WrapStyle(oldpos, pos-oldpos, pt->srctext::comment_style, FALSE, TRUE);
    else if (oldpos) {
	if ((pt)->GetChar(oldpos-1)=='(' || ((pt)->GetChar(oldpos-1)=='/' && pt->slashAsteriskComments)) {
	    (pt)->WrapStyle(oldpos-1, pos-oldpos+1, pt->srctext::comment_style, FALSE, TRUE);
	}
    }
    (self)->SetDotPosition(pos);

    (pt)->NotifyObservers(0);
}

void ptextview::HandleNewlineAndRetrn(long key, boolean reindentThisLine, boolean preindentNewLine)
{
    int newlines = (GetIM())->Argument();
    ptext *pt = (ptext *)GetDataObject();
    int c;
    long pos,end;
    mark *mark;

    if (IsAutoCutMode() && GetDotLength()>0)
	(GetIM())->HandleMenu(proctable::Lookup("textview-zap-region"), this, 0); /* not a particularly efficient way to call textview_ZapRegionCmd, but what else ya gonna do? */
    pos=CollapseDot();
    if(pos) (pt)->BackwardCheckWord(pos-1,0);
    (pt)->ReindentLine(pos);
    end=pos=GetDotPosition(); /* may have changed with ReindentLine */
    while(pos>0 && ((c=(pt)->GetChar(pos-1))==' ' || c=='\t'))
	--pos;
    if(pos<end)
	(pt)->DeleteCharacters(pos,end-pos);

    while(newlines--)
	(pt)->InsertCharacters(pos++,"\n",1);

    mark=(pt)->CreateMark(pos,0);

    SetDotPosition((pt)->Indent(mark));
    (pt)->RemoveMark(mark);
    delete mark;
    (pt)->NotifyObservers(0);
}


static boolean isident(char c)
{
    return (isalnum(c) || c == '_');
}

void ptextview::RenameIdent()
{
    ptext *pt = (ptext *)GetDataObject();
    int pos, len, newlen;
    boolean esc, discardIdent;
    int insideQuotes;
    char orig[40], rep[40];
    int origlen, replen;

    pos = GetDotPosition();
    newlen = len = GetDotLength();

    if (len == 0) {
	message::DisplayString(this, 0, "No region selected\n");
	return;
    }

    if (message::AskForString(this, 0, "Replace identifier: ",
			      NULL, orig, sizeof (orig)) < 0 || orig[0] == '\0' ||
	message::AskForString(this, 0, "New string: ", NULL,
			      rep, sizeof (rep)) < 0) {
	message::DisplayString(this, 0, "Cancelled.");
	return;
    }

    origlen = strlen(orig);
    replen = strlen(rep);

    /* Skip an identifier that's partially outside region */

    discardIdent =
      (isident((pt)->GetChar(pos)) && pos != 0 &&
       isident((pt)->GetChar(pos - 1)));

    insideQuotes = 0;       /* Likely a correct assumption */
    esc = FALSE;

    for (; len >= origlen; ++pos, --len) {
	int c = (pt)->GetChar(pos);
	if (esc) {
	    esc = FALSE;
	    continue;
	}
	if (c == '\\') {
	    esc = TRUE;
	    continue;
	}
	if (discardIdent) {
	    if (isident(c))
		continue;
	    discardIdent = FALSE;
	}
	if (insideQuotes) {
	    if (c == insideQuotes)
		insideQuotes = 0;
	    continue;
	}
	if (c == '"' || c == '\'')
	    insideQuotes = c;
	if (! isident(c))
	    continue;
	discardIdent = TRUE;
	if ((pt)->Strncmp(pos, orig, origlen) == 0 &&
	    ! isident((pt)->GetChar(pos + origlen))) {
	    (pt)->ReplaceCharacters(pos, origlen, rep, replen);
	    len -= origlen - replen - 1;
	    newlen += replen - origlen;
	}
    }

    SetDotLength(newlen);
    (pt)->NotifyObservers(0);
}
