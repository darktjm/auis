/* File srctextview.C created by R L Quinn
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


   srctextview, a view for editing source code. */

#include <andrewos.h>

static UNUSED const char ibmid[] = "(c) Copyright IBM Corp.  1988-1995.  All rights reserved.";

#include <ctype.h>

#include <im.H>
#include <message.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <proctable.H>
#include <bind.H>
#include <environment.H>
#include <frame.H>
#include <framecmds.H>
#include <rect.h>
#include <view.H>
#include <cursor.H>
#include <buffer.H>
#include <content.H>
#include <environ.H>
#include <toolcnt.h>

#include "compress.H"
#include "compressv.H"
#include "srctext.H"
#include "srctextview.H"

/* AutoCut was not made externally visible by txtvcmod, so WE have to check the preference TOO */
static int autocut_mode = -1;	/* uninitialized */
#define IsAutoCutMode() ((autocut_mode == -1 && (autocut_mode = environ::GetProfileSwitch("autocut", FALSE))) || autocut_mode)

static keymap *src_Map;
static menulist *src_Menus;
static cursor *waitCursor;

ATKdefineRegistry(srctextview, textview, srctextview::InitializeClass);
static void redo(srctextview *self);
static void selfinsert(srctextview *self, char key);
static void selfinsertreindent(srctextview *self, char key);
static void reindent(srctextview *self, long key);
static void reformat(srctextview *self, long key);
static void retrn(srctextview *self, long key);
static void reindretrn(srctextview *self, long key);
static void newline(srctextview *self, long key);
static void paren(srctextview *self, char key);
static void forceupperon(srctextview *self, long rock);
static void forceupperoff(srctextview *self, long rock);
static void compressfunc(srctextview *self, long key);
static void compressAll(srctextview *self, long key);
static void renameIdent(srctextview *self, long key);
static void insertComment(srctextview *self, long key);
static void insertLineComment(srctextview *self, long key);
static void startComment(srctextview *self, char key);
static void endComment(srctextview *self, char key);
static void startLineComment(srctextview *self, char key);
static void styleLabel(srctextview *self, char key);
static void styleString(srctextview *self, char key);
static void nextLongLine(srctextview *self, long key);
static void checkLineLengths(srctextview *self, char *rock);
static void toggleOverstrike(srctextview *self, long key); /*RSK91overstrike*/
static void overstrikeOn(srctextview *self, long key);
static void overstrikeOff(srctextview *self, long key);
static void whatColumn(srctextview *self);
static void gotoColumn(srctextview *self, char *rock);
static void balanceParens(srctextview *self);
static void copyrightScramble(textview *self);

static struct bind_Description srctextBindings[]={
    {"srctextview-self-insert"," ",' ', NULL,0,0, (proctable_fptr)selfinsert, "Insert a character and check for a preceding keyword."},
    {"srctextview-self-insert","(",'('},
    {"srctextview-self-insert","[",'['},
    {"srctextview-self-insert","{",'{'},
    {"srctextview-self-insert",".",'.'},
    {"srctextview-self-insert",",",','},
    {"srctextview-self-insert",";",';'},
    {"srctextview-self-insert-reindent", NULL,0, NULL,0,0, (proctable_fptr)selfinsertreindent, "Insert a character, check for preceding keyword, and reindent this line."},
    {"srctextview-redo-styles","\033r",0,"Source Text,Redo styles~10", 0,0, (proctable_fptr)redo, "Wrap styles around comments, functions, keywords, etc in the source code."},
    {"srctextview-paren",")",')',NULL,0,0,(proctable_fptr)paren,"Insert a paren, with balancing."},
    {"srctextview-paren","]",']'},
    {"srctextview-paren","}",'}'},
    {"srctextview-reformat", "\033\022",0, "Source Text,Format line/region~12",0,textview_NotReadOnlyMenus, (proctable_fptr)reformat, "Indent line or region and reflow comments."},
    {"srctextview-reindent", "\t",0, "Source Text,Indent line/region~11",0,textview_NotReadOnlyMenus, (proctable_fptr)reindent, "Indent the current line or selected region."},
    {"srctextview-tab", NULL,0, NULL,0,0, (proctable_fptr)reindent, "(Obsolete, replaced by srctextview-reindent.)"},
    {"srctextview-newline", "\n", 0, NULL,0, 0, (proctable_fptr)newline,"Insert a newline and indent properly."},
    {"srctextview-return", "\015", 0, NULL,0, 0, (proctable_fptr)retrn,"Insert a newline."},
    {"srctextview-reindent-return", 0,0, NULL,0,0, (proctable_fptr)reindretrn,"Indent current line properly and insert a newline."},
    {"srctextview-compress", NULL,0, "Source Text,Compress region~20",0,0, (proctable_fptr)compressfunc, "Compress a region of text for viewing general outline."},
    {"srctextview-compress-all", NULL,0, "Source Text,Compress all indents~21",0,0, (proctable_fptr)compressAll, "Compress all lines indented (as far or) farther than the current line."},
    {"compressv-decompress-all", NULL,0, "Source Text,Decompress all~25",0,0},
    {"srctextview-rename-identifier", "\033Q",0, "Source Text,Rename Identifier~80",0,textview_NotReadOnlyMenus, (proctable_fptr)renameIdent, "Rename identifier in selection region."},
    {"srctextview-next-long-line", "\033\024",0, NULL,0,0, (proctable_fptr)nextLongLine,"Find the next line exceeding max-length and highlight the offending characters."},
    {"srctextview-check-line-lengths", 0,0, NULL,0,0, (proctable_fptr)checkLineLengths,"Display a warning box if any lines exceed max-length."},
    {"srctextview-toggle-overstrike-mode", "\033\034",0, NULL,0,0, (proctable_fptr)toggleOverstrike, "Turn overstrike mode on or off."}, /*RSK91overstrike*/
    {"srctextview-overstrike-mode-on", NULL,0, NULL,0,0, (proctable_fptr)overstrikeOn, "Turn overstrike mode on."}, /*RSK91overstrike*/
    {"srctextview-overstrike-mode-off", NULL,0, NULL,0,0, (proctable_fptr)overstrikeOff, "Turn overstrike mode off."}, /*RSK91overstrike*/
    {"srctextview-what-column", 0,0, NULL,0,0, (proctable_fptr)whatColumn,"Display in the message window what column the cursor is in."},
    {"srctextview-goto-column", 0,0, NULL,0,0, (proctable_fptr)gotoColumn,"Prompt for a column number and move the cursor there, appending whitespace to if needed."},
    {"srctextview-insert-comment", "\0331", 0, NULL,0, 0, (proctable_fptr)insertComment, "Inserts a comment at the end of the line."},
    {"srctextview-insert-linecomment", "\0332", 0, NULL,0, 0, (proctable_fptr)insertLineComment, "Inserts a comment-to-end-of-line."},
    {"srctextview-start-comment", 0,0, NULL,0,0, (proctable_fptr)startComment, "Begins comment style if part of a comment delimiter."},
    {"srctextview-end-comment", 0,0, NULL,0,0, (proctable_fptr)endComment, "Ends a comment style if part of a comment delimiter."},
    {"srctextview-start-linecomment", 0,0, NULL,0,0, (proctable_fptr)startLineComment, "Begins a comment to end of line."},
    {"srctextview-style-label", 0,0, NULL,0,0, (proctable_fptr)styleLabel, "Puts a style on labels."},
    {"srctextview-style-string", 0,0, NULL,0,0, (proctable_fptr)styleString, "Starts or ends a string style."},
    {"srctextview-balance", "\033m",0, NULL,0,0, (proctable_fptr)balanceParens, "Balance parentheses, brackets, or braces."},
    NULL
};


boolean srctextview::InitializeClass()
{
    src_Menus = new menulist;
    src_Map = new keymap;
    bind::BindList(srctextBindings,src_Map,src_Menus,&srctextview_ATKregistry_);
    proctable::DefineProc("srctextview-copyright-scramble", (proctable_fptr)copyrightScramble, ATK::LoadClass("textview"), NULL, "Scrambles or descrambles code. Used to defeat 'copyright-key' protection.");
    waitCursor= cursor::Create(NULL);
    (waitCursor)->SetStandard(Cursor_Wait);
    return TRUE;
}

srctextview::srctextview()
{
    ATKinit;
    this->src_state = keystate::Create(this, src_Map);
    this->src_menus = (src_Menus)->DuplicateML(this);
    (this->src_menus)->SetMask(textview_NoMenus);
    SetBorder(5,5); 
    THROWONFAILURE(TRUE);
}

srctextview::~srctextview()
{
    ATKinit;
    delete this->src_state;
    delete this->src_menus;
}

/* override */
void srctextview::LoseInputFocus()
{
    this->textview::hasInputFocus= FALSE;
    WantUpdate(this);
}

/* override */
/* srctextview_ReceiveInputFocus does mostly the same thing as textview's, but it calls _PrependKeyState to allow srctextview and its subclasses to prepend their keystate overrides to its own. */
void srctextview::ReceiveInputFocus()
{
    this->textview::hasInputFocus= TRUE;
    this->textview::keystate->next= NULL;
    (this->textview::menus)->SetMask(textview_NoMenus);
    (this->src_menus)->SetMask(textview_NoMenus);
    if (this->textview::keystate == this->textview::viCommandModeKeystate)
	/* uh-oh, don't be messin' with vi-emulations's Command Mode! */
	PostKeyState(this->textview::keystate);
    else
	PostKeyState(PrependKeyState());
    if (GetEditor() == VI)
	if (GetVIMode() == COMMAND)
	    message::DisplayString(this, 0, "Command Mode");
	else
	    message::DisplayString(this, 0, "Input Mode");
    WantUpdate(this);
}

void srctextview::PostMenus(class menulist *menulist)
{
    (this->src_menus)->ChainBeforeML(menulist, (long)this);
    (this)->textview::PostMenus(this->src_menus);
}

/* srctextview_PrependKeyState is called by _ReceiveInputFocus, in order to set up all the subclasses' keybinding overrides. This is overridden by subclasses, who should call super_PrependKeyState and then return that, prepended with their OWN keystate. This is a pretty dorky system, but it succeeds in binding keys with the proper precedence. */
keystate *srctextview::PrependKeyState()
{
    this->src_state->next= NULL;
    return (this->src_state)->AddBefore(this->textview::keystate);
}

static void redo(srctextview *self)
{
    (self)->RedoStyles();
}

void srctextview::RedoStyles()
{
    srctext *c=(srctext *)GetDataObject();
    WaitCursorOn();
    (c)->RedoStyles();
    WaitCursorOff();
    (c)->RegionModified(0, (c)->GetLength());
    (c)->NotifyObservers(0);
}

/* friendly read-only behavior stolen from txtvcmod.c */
boolean srctextview::ConfirmReadOnly()
{
    if (((srctext *)GetDataObject())->GetReadOnly()) {
	message::DisplayString(this, 0, "Document is read only.");
	return TRUE;
    } else
	return FALSE;
}

/* override to disable "feature" of picking up the style at the front of a paragragh */
void srctextview::PrepareInsertion(boolean insertingNewLine)
{
    return;
}

static void selfinsert(srctextview *self, char key)
{
    self->SelfInsert(key);
}

void srctextview::SelfInsert(char key /* must be char for "&" to work. */)
{
    srctext *ct=(srctext *)GetDataObject();
    int count=(GetIM())->Argument();
    long pos, oldpos;
    if (ConfirmReadOnly())
	return;
    if (IsAutoCutMode() && GetDotLength()>0)
	(GetIM())->HandleMenu(proctable::Lookup("textview-zap-region"), this, 0); /* not a particularly efficient way to call textview_ZapRegionCmd, but what else ya gonna do? */
    pos= oldpos= CollapseDot();
    while(count--)
	(ct)->InsertCharacters(pos++, &key, 1);
    if (oldpos && !(ct)->IsTokenChar(key))
	(ct)->BackwardCheckWord(oldpos-1,0);
    SetDotPosition(pos);
    FrameDot(pos);
    (ct)->NotifyObservers(0);
}

static void selfinsertreindent(srctextview *self, char key)
{
    self->SelfInsertReindent(key);
}

void srctextview::SelfInsertReindent(char key /* must be char for "&" to work. */)
{
    srctext *ct=(srctext *)GetDataObject();
    long pos;
    if (ConfirmReadOnly())
	return;
    SelfInsert(key);
    pos= GetDotPosition();
    if ((ct)->IndentingEnabled() && !(ct)->InCommentStart(pos))
	(ct)->ReindentLine(pos);
}

static void reindent(srctextview *self, long key)
{
    (self)->Reindent();
}

/* srctextview_Reindent is called when the Tab key is pressed.  If the cursor is in front of a line, it will reindent the line (if enable-indentation is on).  Otherwise, it'll jump to the next tab stop or multiple of tab size, depending on their settings.  If Overstrike mode is on, it will MOVE the cursor there instead of inserting whitespace to that point. */
void srctextview::Reindent()
{
    srctext *ct=(srctext *)GetDataObject();
    long pos=GetDotPosition(), len=GetDotLength();
    int c=0;
    mark *mark=(ct)->CreateMark(pos,len);

    if (len>0 && (ct)->IndentingEnabled()) {
	/* region selected; reindent it */
	WaitCursorOn();
	(ct)->Indent(mark);
	WaitCursorOff();
    } else {
	long oldPos=pos;
	if (len>0)
	    /* region selected; collapse it and add tabs at END of region */
	    pos= oldPos= CollapseDot();
	do  {
	    --pos;
	} while (pos>=0 && (c=(ct)->GetChar(pos))!='\n' && isspace(c));

	if ((pos<0 || c=='\n') && (ct)->IndentingEnabled()) {
	    /* in front of the line; reindent it */
	    (mark)->SetPos(pos+1);
	    SetDotPosition((ct)->Indent(mark));
	} else { /* in the middle/end of the line */
	    int tabstop=(ct)->NextTabStop((ct)->CurrentColumn(oldPos));
	    if (oldPos)
		(ct)->BackwardCheckWord(oldPos-1,0);
	    if (tabstop==0) {
		/* past last tab stop; clean up whitespace and jump to next line */
		long eol=oldPos;
		if ((ct)->IsInOverstrikeMode())
		    /* in Insert mode, oldPos IS the (new) end of line. in Overstrike mode, it's NOT; find it */
		    eol= (ct)->GetEndOfLine(eol);
		if ((eol>=(ct)->GetLength() || !(ct)->IsInOverstrikeMode()) && !(ct)->GetReadOnly()) {
		    /* add a newline and check for bang-comment, because either we're in insert mode, or the end of the file */
		    (ct)->InsertCharacters(eol, "\n",1);
		    if ((ct)->GetStyle(eol) == ct->linecomment_style)
			(ct->text::rootEnvironment)->Remove(eol,1, environment_Style,FALSE);
		}
		for (oldPos=eol; oldPos>0 && is_whitespace((ct)->GetChar(oldPos-1)); --oldPos) ;
		if (oldPos<eol && !(ct)->GetReadOnly()) {
		    (ct)->DeleteCharacters(oldPos, eol-oldPos);
		    (ct)->SetModified();
		}
		SetDotPosition(++oldPos);
	    } else
		if ((ct)->IsInOverstrikeMode())
		    /* just MOVE the cursor, we're in Overstrike mode */
		    GotoColumn(tabstop);
		else if (!(ct)->GetReadOnly())
		    /* insert whitespace to next tab stop, we're in Insert mode */
		    SetDotPosition((ct)->TabAndOptimizeWS(oldPos,tabstop));
	}
    }
    (ct)->RemoveMark(mark);
    delete mark;
    (ct)->NotifyObservers(0);
}

static void reformat(srctextview *self, long key)
{
    (self)->Reformat();
}

/* Reformat calls the same ReindentLine routine as Ctrl-J, on the whole region. This will break up too-long lines, instead of just indenting. */
void srctextview::Reformat()
{
    srctext *ct=(srctext *)GetDataObject();
    long pos=GetDotPosition(), len=GetDotLength();
    mark *endregion, *endofline, *endofflow;
    boolean flowed, oldReindCmmntsValue=ct->reindentComments;
    if (ConfirmReadOnly())
	return;
    endregion= (ct)->CreateMark(pos+len,0);
    endofline= (ct)->CreateMark(pos,0);
    endofflow= (ct)->CreateMark(0,0);
    ct->reindentComments= TRUE; /* formatter ignores value of "reindent-comments" */
    if (len>0) WaitCursorOn();
    /* put pos at beginning of line so its value will still be valid after the ReindentLine call */
    pos= (ct)->GetBeginningOfLine(pos);
    do  {
	(ct)->ReindentLine(pos);
	/* WARNING: the call to ReindentLine invalidates the value of pos; (we assume it's still on the correct line, though) */
	pos= (ct)->GetEndOfLine(pos);
	(endofline)->SetPos(pos);
	/* remove trailing whitespace */
	while (is_whitespace((ct)->GetChar(pos-1))) --pos;
	if (pos<(endofline)->GetPos())
	    (ct)->DeleteCharacters(pos,(endofline)->GetPos()-pos);
	/* pull the following comment up with this one if this one isn't blank */
	flowed= (ct)->GetChar(pos-1)!='\n' && (ct)->ReflowComment(pos+1);
	if (flowed)
	    /* move mark to the new end of the line */
	    (endofflow)->SetPos((ct)->GetEndOfLine((endofline)->GetPos()));
	if ((ct)->GetMaxLineLength())
	    /* break the extras back off so it fits in max-length */
	    (ct)->BreakLine(flowed ? endofflow : endofline);
	if (flowed) /* reformat the last line of the flowed result */
	    pos= (endofflow)->GetPos();
	else /* we're all done here; move to next line */
	    pos= (endofline)->GetPos()+1;
    } while (pos<(endregion)->GetPos());
    ct->reindentComments= oldReindCmmntsValue; /* restore old value of "reindent-comments" */
    (ct)->RemoveMark(endregion);
    (ct)->RemoveMark(endofline);
    (ct)->RemoveMark(endofflow);
    delete endregion;
    delete endofline;
    delete endofflow;
    if (len>0) WaitCursorOff();
    (ct)->NotifyObservers(0);
}

static void retrn(srctextview *self, long key)   
{
    (self)->HandleNewlineAndRetrn(key,FALSE,FALSE);
}

static void reindretrn(srctextview *self, long key)
{
    (self)->HandleNewlineAndRetrn(key,TRUE,FALSE);
}

static void newline(srctextview *self, long key)
{
    (self)->HandleNewlineAndRetrn(key,TRUE,TRUE);
}

/* HandleEndOfLineStyle will terminate linecomments when a newline is inserted. Override it if there are *other* such styles (such as preprocessor style in C) */
void srctextview::HandleEndOfLineStyle(long pos)
{
    srctext *ct = (srctext *)GetDataObject();

    if ((ct)->GetStyle(pos)==ct->linecomment_style) {
	long start=pos;
	while ((ct)->GetChar(start-1)=='\n' && !(ct)->Quoted(start-1)) --start;
	(ct->text::rootEnvironment)->Remove(start,pos-start+1, environment_Style, FALSE);
    }
}

void srctextview::HandleNewlineAndRetrn(long key, boolean reindentThisLine, boolean preindentNewLine)
{
    int newlines = (GetIM())->Argument();
    srctext *ct = (srctext *)GetDataObject();
    mark *endofline, *startofnext;
    int c;
    long pos,end;

    if (ConfirmReadOnly())
	return;
    if (IsAutoCutMode() && GetDotLength()>0)
	(GetIM())->HandleMenu(proctable::Lookup("textview-zap-region"), this, 0); /* not a particularly efficient way to call textview_ZapRegionCmd, but what else ya gonna do? */
    pos= CollapseDot();
    if (pos)
	(ct)->BackwardCheckWord(pos-1,0);

    /* delete trailing whitespace from line key was pressed on */
    end= pos;
    while(pos>0 && ((c=(ct)->GetChar(pos-1))==' ' || c=='\t'))
	--pos;
    if (pos<end)
	(ct)->DeleteCharacters(pos,end-pos);

    /* insert newlines and remember significant positions */
    for (end=newlines; end>0; --end)
	(ct)->InsertCharacters(pos++,"\n",1);
    endofline= (ct)->CreateMark(pos-newlines,0);
    startofnext= (ct)->CreateMark((ct)->SkipWhitespace(pos, (ct)->GetLength())+1,0);

    /* terminate any bang-comments that might be there */
    HandleEndOfLineStyle(pos-1);

    /* avoid reindenting hassles at very beginning of file */
    if ((endofline)->GetPos()<1)
	goto DoneReindenting;

    /* reindent the (non-blank) line the key was pressed on (Newline & ReindRetrn) */
    if (reindentThisLine && (ct)->GetChar((endofline)->GetPos()-1)!='\n')
	(ct)->ReindentLine((endofline)->GetPos());

    /* break up line to enforce max-length if need be */
    if ((ct)->GetMaxLineLength())
	(ct)->BreakLine(endofline);

    /* preindent the new line that was just inserted (Newline only) */
    if (preindentNewLine) {
	/* delete any existing leading whitespace in case ^J hit in middle of line */
	long strt=(startofnext)->GetPos()-1;
	while(strt>0 && ((c=(ct)->GetChar(strt-1))==' ' || c=='\t'))
	    --strt;
	if (strt<(startofnext)->GetPos()-1)
	    (ct)->DeleteCharacters(strt,(startofnext)->GetPos()-1-strt);
	/* indent this new line the way it oughta be, even if it's in a comment */
	pos= (ct)->TabAndOptimizeWS((startofnext)->GetPos()-1,(ct)->Indentation((startofnext)->GetPos()-1));
    } else /* leave caret in left margin (Retrn or ReindRetrn) */
	pos= (endofline)->GetPos()+newlines;

    DoneReindenting: ; /* jumped here if at very beginning of file */
    SetDotPosition(pos);
    FrameDot(pos);
    (ct)->RemoveMark(endofline);
    (ct)->RemoveMark(startofnext);
    delete endofline;
    delete startofnext;
    (ct)->NotifyObservers(0);
}

/* MatchParens will wrap the cursor around the matched parens, *even if* the starting one is off the screen. */
void srctextview::MatchParens(char key)
{
    srctext *ct = (srctext *)GetDataObject();
    long start = GetDotPosition(), openparen = (ct)->ReverseBalance(start), pos;
    char buffer[256], *ptr;
    int i;

    if (openparen != EOF) {
	if (!Visible(openparen)) {
	    for (pos = openparen; pos > 0; --pos) {
		if ((ct)->GetChar(pos) == '\n') {
		    ++pos;
		    break;
		}
	    }
	    ptr = buffer;
	    for (i = sizeof(buffer) - 1; i > 0; --i)
		if ((*ptr++ = (ct)->GetChar(pos++)) == '\n')
		    break;
	    *ptr = '\0';

	    message::DisplayString(this, 0, buffer);
	}
	if (!IsAutoCutMode()) {
	    /* wrap region whether openparen is visible or not */
	    SetDotPosition(openparen);
	    SetDotLength(start - openparen);
	}
    } else
	switch (key) {
	    case '}':
		message::DisplayString(this, 0, "No matching open brace.");
		break;
	    case ')':
		message::DisplayString(this, 0, "No matching open parenthesis");
		break;
	    case ']':
		message::DisplayString(this, 0, "No matching open bracket.");
		break;
	    default:
		message::DisplayString(this, 0, "Parenthesis mis-match.\n");
	}
}

static void paren(srctextview *self, char key)
{
    (self)->Paren(key);
}

/* Paren is overridden by modtextview, because it must check for nested comments and such */
void srctextview::Paren(char key /* must be char for "&" to work. */)
{
    srctext *ct = (srctext *)GetDataObject();
    long pos=GetDotPosition();
    SelfInsert(key);
    if (!(ct)->InCommentStart(pos) && !(ct)->InString(pos))
	MatchParens(key);
}

/* balanceParens is a replacement for textview_BalanceCmd, but this knows language syntax */
static void balanceParens(srctextview *self)
{
    self->BalanceParens();
}

void srctextview::BalanceParens()
{
    srctext *ct= (srctext *)GetDataObject();
    long pos= GetDotPosition()-1, ctlen= ct->GetLength();
    long start= EOF, end= EOF;
    struct paren_node {
	long type;
	struct paren_node *next;
    } *parenstack=NULL, *temp;
    static const char opens[]="({[", closes[]=")}]";
    const char *parentype;
    int ch;
    while (pos<ctlen) {
	ch= ct->GetChar(++pos);
	if ((parentype= strchr(closes, ch)) != NULL) {
	    if (ct->InCommentStart(pos) || ct->InString(pos))
		/* we're in a comment or string. Ignore character! */
		continue;
	    if (parenstack==NULL || parenstack->type != (parentype-closes)) {
		/* found our unmatched close paren */
		end= pos+1;
		break;
	    }
	    /* pop open-paren off stack */
	    temp= parenstack;
	    parenstack= parenstack->next;
	    free(temp);
	} else if ((parentype= strchr(opens, ch)) != NULL) {
	    if (ct->InCommentStart(pos) || ct->InString(pos))
		/* we're in a comment or string. Ignore character! */
		continue;
	    /* push open-paren onto stack */
	    temp= (struct paren_node *)malloc(sizeof(struct paren_node));
	    temp->type= parentype - opens;
	    temp->next= parenstack;
	    parenstack= temp;
	}
    }
    /* free stack if we found a bogus paren or made it to end of file */
    while (parenstack) {
	temp= parenstack;
	parenstack= parenstack->next;
	free(temp);
    }
    /* find match for the odd end-paren we're inside of, and wrap the cursor */
    if (end != EOF)
	start= ct->ReverseBalance(end);
    if (start != EOF) {
	SetDotPosition(start);
	SetDotLength(end-start);
	FrameDot(start);
	WantUpdate(this);
    } else
	message::DisplayString(this, 0, "Sorry, could not balance.");
}

static void forceupperon(srctextview *self, long rock /* required by key bindings, but not used */)
{
    (self)->ForceUpperOn();
}

/* Set the data object's data item forceUpper to TRUE and */
/* change the menu list entry to Force Upper Off */
void srctextview::ForceUpperOn()
{
    struct proctable_Entry *proc;

    ((srctext *)GetDataObject())->SetForceUpper(TRUE);

    /* delete existing menu item */
    (this->src_menus)->DeleteFromML("Source Text,Force Upper On~90");

    /* define the procedure for the new menu option */
    proc = proctable::DefineProc("srctextview-force-upper-off", (proctable_fptr)forceupperoff, &srctextview_ATKregistry_, NULL, "Turn off auto upper casing of keywords");

    /* add Force Upper Off option to Source Text menu card */
    (this->src_menus)->AddToML("Source Text,Force Upper Off~90", proc, 0, 0);

    WantInputFocus(this);
}

static void forceupperoff(srctextview *self, long rock /* required by key bindings, but not used */)
{
    (self)->ForceUpperOff();
}

/* Set the data object's data item forceUpper to FALSE and */
/* change the menu list entry to Force Upper On */
void srctextview::ForceUpperOff()
{
    struct proctable_Entry *proc;

    ((srctext *)GetDataObject())->SetForceUpper(FALSE);

    /* delete existing menu item */
    (this->src_menus)->DeleteFromML("Source Text,Force Upper Off~90");

    /* define the procedure for the new menu option */
    proc = proctable::DefineProc("srctextview-force-upper-on", (proctable_fptr)forceupperon, &srctextview_ATKregistry_, NULL, "Turn on auto upper casing of keywords");

    /* add Force Upper On option to Source Text menu card */
    (this->src_menus)->AddToML("Source Text,Force Upper On~90", proc, 0, 0);

    WantInputFocus(this);
}

/*-- Override-- */
/* Associate the view with the data object, and */
/* initialize the force upper on/off menu option */
void srctextview::SetDataObject(class dataobject *dataobj)
{
    /* call overriden SetDataObject */
    textview::SetDataObject(dataobj);
    (this->src_menus)->SetMask(textview_NoMenus);

    if (((srctext *)dataobj)->GetForceUpper())
	forceupperon(this,0);
    else
	forceupperoff(this,0);

    if (((srctext *)dataobj)->GetMaxLineLength())
	(this->src_menus)->AddToML("Source Text,Find Next too-long Line~60", proctable::Lookup("srctextview-next-long-line"), 0, 0);
    else
	(this->src_menus)->DeleteFromML("Source Text,Find Next too-long Line~60");
}

void srctextview::DeleteMenuItem(char menuitem[])
{
    (this->src_menus)->DeleteFromML(menuitem);
    return;
}

static void compressfunc(srctextview *self, long key)
{
    (self)->Compress();
}

void srctextview::Compress()
{
    srctext *ct=(srctext *)GetDataObject();
    long pos=GetDotPosition(), len=GetDotLength();
    long modValue=(ct)->GetModified();

    if (len==0) {
	long ind;
	if ((ct)->GetChar(pos)=='\n') --pos;
	if ((ind=(ct)->CurrentIndent(pos)) < 2 || pos<1) {
	    /* don't bother trying to compress the WHOLE FILE! */
	    message::DisplayString(this,0, "Deliberately ignoring your request to compress the entire file into a box.");
	    return;
	}
	(ct)->ExtendToOutdent(ind, &pos,&len);
    }
    compress::Compress(ct,pos,len);

    CollapseDot();
    (ct)->RestoreModified(modValue);
    (ct)->NotifyObservers(0);
    ToolCount("EditViews-compress",NULL);
}

static void compressAll(srctextview *self, long key)
{
    (self)->CompressAll();
}

void srctextview::CompressAll()
{
    srctext *ct=(srctext *)GetDataObject();
    long pos=1, oldpos=CollapseDot(), srclen;
    long count, nextcount=(ct)->GetLength()-4096;
    int curInd;
    long modValue=(ct)->GetModified();

    if ((ct)->GetChar(oldpos)=='\n') --oldpos;
    curInd= (ct)->CurrentIndent(oldpos);
    if (curInd<2 || oldpos<1) {
	/* don't bother trying to compress the WHOLE FILE! */
	message::DisplayString(this,0, "Deliberately ignoring your request to compress the entire file into a box.");
	return;
    }
    WaitCursorOn();
    do	{
	long startcompress, len;
	srclen=(ct)->GetLength();
	/* find next line indented as far as, or farther than, curInd */
	while (pos<srclen && (ct)->CurrentIndent(pos)<curInd)
	    pos= (ct)->GetEndOfLine(pos) +1;
	startcompress= pos;
	if (pos<srclen)
	    if ((ct)->ExtendToOutdent(curInd, &startcompress,&len)) {
		compress::Compress(ct,startcompress,len);
		pos= startcompress+1; /* move past the new inset */
		count= srclen-pos;
		if (count < nextcount) {
		    /* only do this at minimal intervals of 4K chars, to avoid excessive message-flashing */
		    char countdown[256];
		    nextcount= count-4096;
		    /* shr 10 bits to produce a value more meaningful than a jumpy 6-digit number */
		    sprintf(countdown, "compressing (countdown: %ld)", count >> 12);
		    message::DisplayString(this,0, countdown);
		    im::ForceUpdate();
		}
	    } else {
		pos= startcompress+len+1+1; /* skip insignificant indented region */
	    }
    } while (pos<srclen); /* srclen may be invalid, but no matter. we'll figure that out on the next loop */
    message::DisplayString(this,0, "compressing completed.");
    WaitCursorOff();
    (ct)->RestoreModified(modValue);
    (ct)->NotifyObservers(0);
    ToolCount("EditViews-compress",NULL);
}

/* This method is only around for backward compatibility with things like watson */
/* dummy is a dummy parameter left here for no reason other than to be consistent with previous versions. */
void srctextview::DecompressAll(long dummy)
{
    srctext *ct=(srctext *)GetDataObject();
    long modValue=(ct)->GetModified();
    compress::DecompressAll(ct);
    (ct)->RestoreModified(modValue);
    (ct)->NotifyObservers(0);
}

static void renameIdent(srctextview *self, long key)
{
    (self)->RenameIdent();
}

void srctextview::RenameIdent()
{
    srctext *ct = (srctext *)GetDataObject();
    long pos, len, newlen;
    char promptbuf[256];
    const char *prompt;
    static char defaultorig[40] = "";
    static char defaultrep[40] = "";
    char orig[40], rep[40];
    int origlen, replen;
    int occurrences=0;

    if (ConfirmReadOnly())
	return;
    pos = GetDotPosition();
    newlen = len = GetDotLength();

    if (len == 0) {
	message::DisplayString(this, 0, "No region selected\n");
	return;
    }

    /* prompt for original identifier name */
    if (defaultorig[0]) {
	sprintf(promptbuf, "Replace identifier [%s] : ", defaultorig);
	prompt= promptbuf;
    } else
	prompt= "Replace identifier : ";
    if (message::AskForString(this, 0, prompt, NULL, orig, sizeof(orig)) < 0) {
	return;
    }
    if (!orig[0] && defaultorig[0])
	/* nothing entered, use default string */
	sprintf(orig, "%s", defaultorig);
    if (!orig[0]) {
	message::DisplayString(this, 0, "Cancelled.");
	return;
    }

    /* prompt for what to rename it to */
    if (defaultrep[0]) {
	sprintf(promptbuf, "New identifier name [%s] : ", defaultrep);
	prompt= promptbuf;
    } else
	prompt= "New identifier name : ";
    if (message::AskForString(this, 0, prompt, NULL, rep, sizeof(rep)) < 0) {
	return;
    }
    if (!rep[0] && defaultrep[0])
	/* nothing entered, use default string */
	sprintf(rep, "%s", defaultrep);
    if (!rep[0]) {
	message::DisplayString(this, 0, "Cancelled.");
	return;
    }

    /* skip any identifier partially outside region */
    if (pos>0 && (ct)->IsTokenChar((ct)->GetChar(pos-1)))
	while (len>0 && (ct)->IsTokenChar((ct)->GetChar(pos)))
	    ++pos, --len;
    origlen = strlen(orig);
    replen = strlen(rep);

    /* hunt down identifiers and replace them */
    for (; len >= origlen; ++pos, --len)
	if ((ct)->IsTokenChar((ct)->GetChar(pos)) &&
	    !(ct)->Quoted(pos) &&
	    (ct)->Strncmp(pos,orig,origlen)==0 &&
	    !(ct)->IsTokenChar((ct)->GetChar(pos+origlen)) &&
	    !(ct)->IsTokenChar((ct)->GetChar(pos-1)) &&
	    !(ct)->InCommentStart(pos) &&
	    !(ct)->InString(pos)) {
	    /* found a legit match. change it */
	    (ct)->ReplaceCharacters(pos,origlen, rep,replen);
	    newlen+= replen-origlen;
	    pos+= replen-origlen;
	    ++occurrences;
	}
    SetDotLength(newlen);
    strcpy(defaultorig, orig);
    strcpy(defaultrep, rep);
    if (occurrences)
	sprintf(promptbuf, "Replaced %d occurrences.", occurrences);
    else
	sprintf(promptbuf, "No occurrences of identifier '%s' found.", orig);
    message::DisplayString(this, 0, promptbuf);
    (ct)->NotifyObservers(0);
}

/*-----------------copied from 'compile.c'-------------------*/
struct finderInfo {
    frame *myFrame, *otherFrame, *bestFrame;
    buffer *myBuffer;
};
static boolean FrameFinder(frame *frame, long infop)
{
    struct finderInfo *info= (struct finderInfo *)infop;
    struct rectangle bogus;

    if (info->myFrame == frame)
	return FALSE;
    if (info->otherFrame != NULL && info->bestFrame != NULL)
	return TRUE;
    (frame)->GetVisualBounds(&bogus);
    if (!rectangle_IsEmptyRect(&bogus))
	if ((frame)->GetBuffer() == info->myBuffer) {
	    info->bestFrame = frame;
	    return FALSE;
	} else {
	    if ((frame)->GetBuffer() != NULL) {
		info->otherFrame = frame;
		return FALSE;
	    }
	}
    return FALSE;
}
static boolean ViewEqual(frame *frame, long view)
{
    return ((frame)->GetView() == (class view *)view);
}
static frame *FindByView(srctextview *view)
{
    return frame::Enumerate(ViewEqual, (long)view);
}
/* Find a window other than the one that contains this inset.  Create one if we have to. */
static view *PutInAnotherWindow(srctextview *view, buffer *buffer, int forceWindow)
{
    frame *frame;
    struct finderInfo myInfo;

    myInfo.myFrame = FindByView(view);
    myInfo.otherFrame = NULL;
    myInfo.bestFrame = NULL;
    myInfo.myBuffer = buffer;
    frame::Enumerate(FrameFinder, (long)&myInfo);
#ifdef NEVER_WASTE_SCREEN_SPACE
    frame = (myInfo.bestFrame != NULL) ? myInfo.bestFrame : ((myInfo.otherFrame != NULL) ? myInfo.otherFrame : NULL);
#else /*NEVER_WASTE_SCREEN_SPACE*/
    frame= (myInfo.bestFrame != NULL) ? myInfo.bestFrame : NULL;
    if (!frame && environ::GetProfileSwitch("WasteScreenSpace",TRUE)==FALSE && myInfo.otherFrame!=NULL)
	frame= myInfo.otherFrame;
#endif /*NEVER_WASTE_SCREEN_SPACE*/
    if (frame == NULL) {
	im *newIM;

	if (!forceWindow)
	    return NULL;
	newIM = im::Create(NULL);
	frame = frame::Create(buffer);
	(newIM)->SetView(frame);

	/* This is here because the frame_Create call can't set the input focus because the frame didn't have a parent when it called view_WantInputFocus.  This is bogus but hard to fix... */
	((frame)->GetView())->WantInputFocus((frame)->GetView());
    } else {
	im *im;
	if ((frame)->GetBuffer() != buffer)
	    (frame)->SetBuffer(buffer, TRUE);
	im= (frame)->GetIM();
	if (im) (im)->ExposeWindow(); /* make sure window gets uniconified */
    }
    return (frame)->GetView();
}
/*------------------------------------------------------*/

static long tcpos(content *txt, char *strng)
{
    long pos=0, end=(txt)->GetLength();
    char buf[256];
    while (pos<end) {
	int i=0;
	while (i<256 && pos+i<end && (buf[i]=(txt)->GetChar(pos+i))!='\n')
	    ++i;
	buf[i]='\0';
	if (strcmp(strng,buf)==0)
	    return pos;
	pos+=i+1;
    }
    return -1;
}

void srctextview::PutFileIntoNewWindow(char *bufname, char *proc, char *filename)
{
    view *newView=0;
    view *viewp=(view *)(GetIM()->topLevel);
    buffer *bf;
    static content *TofC=NULL;
    mark *loc;
    if ((bf=buffer::FindBufferByFile(filename))==NULL) {
	bf=buffer::GetBufferOnFile(filename,FALSE);
	if (bf) {
	    (bf)->ReadFile(filename);
	    (bf)->SetName(bufname);
	}
    }
    if (bf) {
	if (viewp->dataobject!=(bf)->GetData()) /*not viewing Int/Mod already*/
	    newView=PutInAnotherWindow(this,bf,TRUE);
	if (proc && strlen(proc)>0) {
	    if (!TofC)
		TofC=new content;
	    if (TofC) {
		long pos;
		(TofC)->SetSourceText((text *)(bf)->GetData());
		pos=tcpos(TofC,proc);
		if (pos<0) return;
		loc=(TofC)->locate(pos);
		if (loc) {
		    srctextview *mnewv=(srctextview *)newView;
		    (mnewv)->SetDotPosition((loc)->GetPos());
		    (mnewv)->SetDotLength((loc)->GetLength());
		    (mnewv)->FrameDot((loc)->GetPos());
		}
	    }
	}
    }
}

/* fileExists(self,filename) returns true if "filename" exists */
static boolean fileExists(srctextview *self, char *filename)
{
    FILE *fp;
    if ((fp=fopen(filename,"r"))==NULL)
	return FALSE;
    else {
	fclose(fp);
	return TRUE;
    }
}

/* FindSubFile() attempts to locate "filename" somewhere in the "searchpath", and then PutFileIntoNewWindow() */
boolean srctextview::FindSubFile(char *filename, char *bufname, char *procname, char *searchpath)
{
    char path[1024];
    long pos=0,oldpos=0;

    /*check directory of source file*/
    class dataobject *dtobj= (class dataobject *)GetDataObject();
    buffer *buf= buffer::FindBufferByData(dtobj);
    strcpy(path,(buf)->GetFilename());
    *(strrchr(path,'/')+1)='\0'; /*chop off filename to get source's path*/
    strcat(path,filename);
    if (fileExists(this,path)) {
	PutFileIntoNewWindow(bufname,procname,path);
	return TRUE;
    }

    /*check current working directory*/
    strcpy(path,"./"); strcat(path,filename);
    if (fileExists(this,path)) {
	PutFileIntoNewWindow(bufname,procname,path);
	return TRUE;
    }

    /*check Search Path*/
    if (searchpath!=NULL && strlen(searchpath)>0 && searchpath[0]!=' ') {
	while (pos<strlen(searchpath)) {
	    while (pos<strlen(searchpath) && searchpath[pos]!=':') ++pos;
	    strncpy(path,searchpath+oldpos,pos-oldpos);
	    path[pos-oldpos]='\0';
	    oldpos=(++pos);
	    strcat(path,"/"); strcat(path,filename);
	    if (fileExists(this,path)) {
		PutFileIntoNewWindow(bufname,procname,path);
		return TRUE;
	    }
	}
    }
    return FALSE;
}

static void insertComment(srctextview *self, long key)   
{
    (self)->InsertComment();
}

/* InsertComment gets called when Esc-1 is pressed in the view, to insert a comment. If there's already a comment there, it will be reformatted */
void srctextview::InsertComment()
{
    srctext *ct=(srctext *)GetDataObject();
    long endofline, pos;
    int desiredCol=ct->commentCol, linelength, spaceWanted;
    if (ConfirmReadOnly())
	return;
    /* first make sure this language HAS comments */
    if (ct->commentString==NULL)
	return;
    pos= CollapseDot();
    endofline= (ct)->GetEndOfLine(pos);
    if ((ct)->GetStyle(endofline-1)==ct->comment_style) {
	/* already a comment there! just reformat it */
	Reformat();
	return;
    }
    /* remove trailing whitespace, but remember how much in case the user explicitly ADDED trailing whitespace */
    linelength= (ct)->CurrentColumn(endofline);
    while (is_whitespace((ct)->GetChar(endofline-1)))
	(ct)->DeleteCharacters(--endofline,1);
    spaceWanted= linelength-(ct)->CurrentColumn(endofline);
    if (spaceWanted<ct->remarkPadding)
	spaceWanted= ct->remarkPadding;
    linelength= (ct)->CurrentColumn(endofline);
    if (linelength+spaceWanted>desiredCol)
	if (ct->commentFixed)
	    /* it's not going to fit and it MUST be in commentCol, so start a new line */
	    (ct)->JustInsertCharacters(endofline++,"\n",1);
	else
	    /* we just have to nudge it over a little */
	    desiredCol= linelength+spaceWanted;
    if ((ct)->CurrentColumn(endofline)<desiredCol)
	endofline= (ct)->TabAndOptimizeWS(endofline,desiredCol);
    /* insert the comment and style it */
    (ct)->JustInsertCharacters(endofline, ct->commentString, strlen(ct->commentString));
    (ct)->WrapStyleNow(endofline,strlen(ct->commentString), ct->comment_style, FALSE,FALSE);
    /* move cursor to middle of comment */
    SetDotPosition(endofline+strlen(ct->commentString)/2);
    (ct)->NotifyObservers(0);
}

static void insertLineComment(srctextview *self, long key)  
{
    (self)->InsertLineComment();
}

/* InsertLineComment gets called when Esc-2 is pressed in the view, to insert a line-comment. If there's already a comment there, it will be reformatted */
void srctextview::InsertLineComment()
{
    srctext *ct=(srctext *)GetDataObject();
    long endofline, pos;
    int desiredCol=ct->linecommentCol, linelength, spaceWanted;
    if (ConfirmReadOnly())
	return;
    /* first make sure this language HAS line-comments */
    if (ct->linecommentString==NULL)
	return;
    pos= CollapseDot();
    endofline= (ct)->GetEndOfLine(pos);
    if ((ct)->GetStyle(endofline-1)==ct->linecomment_style) {
	/* already a line-comment there! just reformat it */
	Reformat();
	return;
    }
    /* remove trailing whitespace, but remember how much in case the user explicitly ADDED trailing whitespace */
    linelength= (ct)->CurrentColumn(endofline);
    while (is_whitespace((ct)->GetChar(endofline-1)))
	(ct)->DeleteCharacters(--endofline,1);
    spaceWanted= linelength-(ct)->CurrentColumn(endofline);
    if (spaceWanted<ct->remarkPadding)
	spaceWanted= ct->remarkPadding;
    linelength= (ct)->CurrentColumn(endofline);
    if (linelength+spaceWanted>desiredCol)
	if (ct->linecommentFixed)
	    /* it's not going to fit and it MUST be in linecommentCol, so start a new line */
	    (ct)->JustInsertCharacters(endofline++,"\n",1);
	else
	    /* we just have to nudge it over a little */
	    desiredCol= linelength+spaceWanted;
    if ((ct)->CurrentColumn(endofline)<desiredCol)
	endofline= (ct)->TabAndOptimizeWS(endofline,desiredCol);
    /* insert the line-comment and style it */
    (ct)->JustInsertCharacters(endofline, ct->linecommentString, strlen(ct->linecommentString));
    if (!(ct)->InCommentStart(endofline))
	(ct)->WrapStyleNow(endofline,strlen(ct->linecommentString), ct->linecomment_style, FALSE,TRUE);
    /* move cursor to end of line-comment */
    SetDotPosition(endofline+strlen(ct->linecommentString));
    (ct)->NotifyObservers(0);
}

static void startComment(srctextview *self, char key /* must be char for "&" to work. */)
{
    (self)->StartComment(key);
}

/* srctextview_StartComment is written to handle slash-star comments, but not mapped in the proctable. To enable them in ctextview or plxtextview, simply bind them. */
void srctextview::StartComment(char key /* must be char for "&" to work. */)
{
    srctext *ct=(srctext *)GetDataObject();
    int count=(GetIM())->Argument();
    long pos, oldpos;
    if (ConfirmReadOnly())
	return;
    if (IsAutoCutMode() && GetDotLength()>0)
	(GetIM())->HandleMenu(proctable::Lookup("textview-zap-region"), this, 0); /* not a particularly efficient way to call textview_ZapRegionCmd, but what else ya gonna do? */
    pos= oldpos= CollapseDot();
    while (count--) (ct)->InsertCharacters(pos++, &key, 1);
    if (oldpos && (ct)->GetChar(oldpos-1)=='/' && !(ct)->GetStyle(oldpos-1) && !(ct)->InString(oldpos))
	(ct)->WrapStyleNow(oldpos-1,pos-oldpos+1, ct->comment_style, FALSE,TRUE);
    SetDotPosition(pos);
    FrameDot(pos);
    (ct)->NotifyObservers(0);
}

static void endComment(srctextview *self, char key /* must be char for "&" to work. */)
{
    (self)->EndComment(key);
}

/* srctextview_EndComment is written to handle slash-star comments, but not mapped in the proctable. To enable them in ctextview or plxtextview, simply bind them. */
void srctextview::EndComment(char key /* must be char for "&" to work. */)
{
    srctext *ct=(srctext *)GetDataObject();
    int count=(GetIM())->Argument();
    long pos,oldpos;
    if (ConfirmReadOnly())
	return;
    if (IsAutoCutMode() && GetDotLength()>0)
	(GetIM())->HandleMenu(proctable::Lookup("textview-zap-region"), this, 0); /* not a particularly efficient way to call textview_ZapRegionCmd, but what else ya gonna do? */
    oldpos= pos= CollapseDot();
    while (count--) (ct)->InsertCharacters(pos++,&key,1);
    if (oldpos && (ct)->GetChar(oldpos-1)=='*')
	if ((ct)->GetStyle(oldpos+1)==ct->comment_style)
	    /* terminate existing style */
	    ((ct->text::rootEnvironment)->GetEnclosing(oldpos+1))->SetStyle(FALSE,FALSE);
	else {
	    /* wrap a new style */
	    long start=oldpos-1;
	    while (--start>0) {
		if ((ct)->GetChar(start)=='*')
		    if ((ct)->GetChar(start-1)=='/') {
			/* found start of comment, wrap style */
			if ((ct)->GetStyle(start) && (ct)->GetStyle(start)!=ct->comment_style)
			    /* must be inside a preprocessor directive or some other unknown style. abort */
			    break;
			--start;
			if ((ct)->InString(start))
			    break;
			(ct)->WrapStyleNow(start,oldpos-start+1, ct->comment_style, FALSE,FALSE);
			break;
		    } else if ((ct)->GetChar(start+1)=='/')
			/* uh-oh, found another end of comment! */
			break;
	    }
	}
    SetDotPosition(pos);
    FrameDot(pos);
    (ct)->NotifyObservers(0); 
}

static void startLineComment(srctextview *self, char key /* must be char for "&" to work. */)
{
    (self)->StartLineComment(key);
}

/* srctextview_StartLineComment is written to handle any bang-comment character, but not mapped in the proctable. To enable a bang comment, simply bind it in the subclass. */
void srctextview::StartLineComment(char key /* must be char for "&" to work. */)
{
    srctext *ct=(srctext *)GetDataObject();
    int count=(GetIM())->Argument();
    long pos, oldpos;
    if (ConfirmReadOnly())
	return;
    if (IsAutoCutMode() && GetDotLength()>0)
	(GetIM())->HandleMenu(proctable::Lookup("textview-zap-region"), this, 0); /* not a particularly efficient way to call textview_ZapRegionCmd, but what else ya gonna do? */
    pos= oldpos= CollapseDot();
    while (count--) (ct)->InsertCharacters(pos++, &key, 1);
    if (!(ct)->GetStyle(oldpos) && !(ct)->InString(oldpos))
	(ct)->WrapStyleNow(oldpos,pos-oldpos, ct->linecomment_style, FALSE,TRUE);
    SetDotPosition(pos);
    FrameDot(pos);
    (ct)->NotifyObservers(0);
}

static void styleLabel(srctextview *self, char key /* must be char for "&" to work. */)
{
    (self)->StyleLabel(key);
}

/* StyleLabel JUST wraps the label style, if appropriate. It calls SelfInsert to do all the actual insertion/update work. */
void srctextview::StyleLabel(char key)
{
    srctext *ct=(srctext *)GetDataObject();
    long pos;
    if (ConfirmReadOnly())
	return;
    if (IsAutoCutMode() && GetDotLength()>0)
	(GetIM())->HandleMenu(proctable::Lookup("textview-zap-region"), this, 0); /* not a particularly efficient way to call textview_ZapRegionCmd, but what else ya gonna do? */
    pos= CollapseDot();
    if (pos && !(ct)->InCommentStart(pos) && !(ct)->InString(pos)) {
	mark *colonpos=(ct)->CreateMark(pos,0);
	SelfInsertReindent(key); /* do this first, so the inserted char won't be part of the label style */
	(ct)->BackwardCheckLabel((colonpos)->GetPos());
	(ct)->RemoveMark(colonpos);
	delete colonpos;
    } else
	SelfInsert(key);
}

static void styleString(srctextview *self, char key)
{
    (self)->StyleString(key);
}

/* String starts or ends a string style, as appropriate. */
void srctextview::StyleString(char key)
{
    srctext *ct=(srctext *)GetDataObject();
    long pos;
    if (ConfirmReadOnly())
	return;
    if (IsAutoCutMode() && GetDotLength()>0)
	(GetIM())->HandleMenu(proctable::Lookup("textview-zap-region"), this, 0); /* not a particularly efficient way to call textview_ZapRegionCmd, but what else ya gonna do? */
    pos= CollapseDot();
    SelfInsert(key);
    if ((ct)->Quoted(pos))
	return;
    if ((ct)->InString(pos)) {
	/* we're in a string now, let's see if it needs terminating */
	long start=(ct)->BackwardSkipString(pos-1,key)+1;
	environment *existingstyle= (ct)->GetEnvironment(pos);
	if (start<=0 || (ct)->GetStyle(start))
	    /* either we're NOT in a string, or the place we thought we started is part of some other style */
	    return;
	if (existingstyle == ct->text::rootEnvironment)
	    /* need to wrap whole string */
	    (ct)->WrapStyleNow(start,pos-start+1, ct->string_style, FALSE,FALSE);
	else if ((existingstyle)->Eval() == start)
	    /* only need to terminate existing style */
	    (existingstyle)->SetStyle(FALSE,FALSE);
    } else if (!(ct)->GetStyle(pos))
	/* we're not already IN a string; let's START one */
	(ct)->WrapStyleNow(pos,GetDotPosition()-pos, ct->string_style, FALSE,TRUE);
}

/* nextLongLine finds the next line exceeding max-length and wraps the cursor around the offending characters */
static void nextLongLine(srctextview *self, long key)
{
    srctext *ct = (srctext *)(self)->GetDataObject();
    long max=(ct)->GetMaxLineLength();
    if (max==0)
	message::DisplayString(self,0,"None found; No maximum line length set.");
    else {
	long pos=(self)->GetDotPosition()+(self)->GetDotLength();
	long samepos=pos, start=0;
	long len=(ct)->GetLength();
	int c, linelen=0;

	for (pos=(ct)->GetBeginningOfLine(pos+1); pos<len; ++pos) {
	    c= (ct)->GetChar(pos);
	    if (c=='\n') {
		if (start) {
		    /* found the end of a line that is too long */
		    (self)->SetDotPosition(start);
		    (self)->SetDotLength(pos-start);
		    (self)->FrameDot(pos);
		    return;
		}
		linelen= 0;
	    } else if (c=='\t')
		linelen=(linelen+8)&~7;
	    else
		++linelen;
	    if (linelen>max && !start)
		/* this line is too long (and start wasn't set already) */
		start= pos;
	}
	/* display a reassuring message if end-of-file reached */
	message::DisplayString(self,0,"No more too-long lines found.");
    }
}

/* checkLineLengths calls srctext_CheckLineLengths to display a box if any lines exceed that length */
static void checkLineLengths(srctextview *self, char *rock)
{
    srctext *d = (srctext *)(self)->GetDataObject();
    int numericalvalue= (d)->GetMaxLineLength(); /* use max-length unless explicitly specified in exinit */
    if (rock) /* rock is actually a string, if from ezinit file */
	numericalvalue= atoi(rock);
    (d)->CheckLineLengths(numericalvalue,self);
}

static void toggleOverstrike(srctextview *self, long key) /*RSK91overstrike*/
{
    srctext *d = (srctext *)(self)->GetDataObject();
    if ((d)->IsInOverstrikeMode()) {
	(d)->ChangeOverstrikeMode(FALSE);
	message::DisplayString(self,0,"Normal (insert) mode.");
    } else {
	(d)->ChangeOverstrikeMode(TRUE);
	message::DisplayString(self,0,"Overstrike mode.");
    }
    /*srctext_RegionModified(d,0,srctext_GetLength(d));*/ /*wimpy way to get all the views to refresh the cursor type*/ /* not needed anyway, unless drawtxtv got changed to display a different cursor in overstrike mode */
    (d)->NotifyObservers(observable_OBJECTCHANGED);
}

static void overstrikeOn(srctextview *self, long key) /*RSK91overstrike*/
{
    srctext *d = (srctext *)(self)->GetDataObject();
    (d)->ChangeOverstrikeMode(TRUE);
    (d)->NotifyObservers(observable_OBJECTCHANGED);
}

static void overstrikeOff(srctextview *self, long key) /*RSK91overstrike*/
{
    srctext *d = (srctext *)(self)->GetDataObject();
    (d)->ChangeOverstrikeMode(FALSE);
    (d)->NotifyObservers(observable_OBJECTCHANGED);
}

/* WaitCursorOn sets the process cursor to a Clock (wait) symbol */
void srctextview::WaitCursorOn()
{
    im::SetProcessCursor(waitCursor);
}

/* WaitCursorOff sets the process cursor back to normal */
void srctextview::WaitCursorOff()
{
    im::SetProcessCursor(NULL);
}

static void whatColumn(srctextview *self)
{
    (self)->WhatColumn();
}

/* WhatColumn displays the current column of the caret (or selected region) in the message window, taking tab characters into account */
void srctextview::WhatColumn()
{
    srctext *ct = (srctext *)GetDataObject();
    long pos=GetDotPosition(), len=GetDotLength();
    char s[128];
    /* note that 1 is added to all columns to translate to left-margin-is-column-one system */
    if (len>0)
	sprintf(s,"Selected region starts in column %d and ends in column %d.", (ct)->CurrentColumn(pos)+1, (ct)->CurrentColumn(pos+len)+1);
    else
	sprintf(s,"Column %d.", (ct)->CurrentColumn(pos)+1);
    message::DisplayString(this,0,s);
}

/* gotoColumn is called from a menu card. Its rock parameter, passed from the proctable entry, defaults to 0 but may contain a left-margin-is-column-one parameter from an ezinit binding. In either case, the number will be converted to left-margin-is-column-ZERO (negative means PROMPT for column) value, and passed on to the GotoColumn method. */
static void gotoColumn(srctextview *self, char *rock)
{
    int numericalvalue= 0;
    if (rock) /* rock is actually a string, if from ezinit file */
	numericalvalue= atoi(rock);
    (self)->GotoColumn(numericalvalue-1);
}

/* GotoColumn method's rock parameter is a left-margin-is-column-zero value, which is the system used internally by all source view code. Users communicate with left-margin-is-column-ONE values, which get interpreted by gotoColumn. */
/* If rock parameter is negative, it means a proctable entry is calling this with no parameter and the user should be PROMPTED for a column number. Any other value of rock is considered a parameter and will forgo the prompting, using rock as the target column. */
void srctextview::GotoColumn(int rock)
{
    srctext *ct = (srctext *)GetDataObject();
    long p;
    char s[10];
    int newcol;
    s[0]='\0';
    p= CollapseDot();
    if (rock<0) {
	if (message::AskForString(this,0,"What column? ",NULL,s,sizeof(s))<0 || sscanf(s,"%d",&newcol)!=1) {
	    message::DisplayString(this, 0, "Cancelled.");
	    return;
	}
	--newcol; /* translate to left-margin-is-column-ZERO */
    } else
	newcol= rock; /* passed parameters are *already* in left-margin-is-column-zero format */
    if ((ct)->CurrentColumn(p)>newcol)
	/* move backward to desired column */
	while (p>0 && (ct)->CurrentColumn(p)>newcol) --p;
    else
	/* move forward to desired column */
	while (p<(ct)->GetLength() && (ct)->GetChar(p)!='\n' && (ct)->CurrentColumn(p+1)<=newcol) ++p;
    if ((ct)->CurrentColumn(p)<newcol)
	/* past end of line (or possibly at a tab char); need to slap on some whitespace */
	if ((ct)->GetReadOnly()) {
	    char msg[128];
	    SetDotPosition(p);
	    sprintf(msg, "Document is read only.  Moved to column %d.", (ct)->CurrentColumn(p)+1);
	    message::DisplayString(this, 0, msg);
	} else
	    SetDotPosition((ct)->TabAndOptimizeWS(p,newcol));
    else
	/* otherwise, just move the cursor over */
	SetDotPosition(p);
}

static void copyrightScramble(textview *self)
{
    text *txt= (text *)self->GetDataObject();
    long pos= self->GetDotPosition(), len= self->GetDotLength();
    if (txt->GetReadOnly()) {
	message::DisplayString(self, 0, "Document is read only.");
	return;
    }
    if (len<1) {
	message::DisplayString(self, 0, "No region is selected.");
	return;
    }
    srctext::CopyrightScramble(txt, pos, len);
    txt->NotifyObservers(observable_OBJECTCHANGED);
}
