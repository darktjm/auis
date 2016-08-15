/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
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
*/

#include <andrewos.h>


#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/text/RCS/txtvcmds.C,v 3.7 1995/11/07 20:17:10 robr Stab74 $";
#endif



 

#include <ctype.h>
#define AUXMODULE 1
#include <textview.H>
#include <txtvinfo.h>

#include <keymap.H>
#include <proctable.H>
#include <menulist.H>
#include <environ.H>
#include <im.H>

#include <bind.H>

#include <txtproc.h>

long lcKill;
long lcYank;
long lcMove;
long lcDisplayEnvironment;
long lcInsertEnvironment;
long lcNewLine;

#ifndef NORCSID
#endif
void textview_NOOPCmd (register class textview  *self);
int charType(register char  c);
static void CheckStylePreferences(class keymap  *newKeymap, class menulist  **normalMenus);
static int TranslateKeySequence(const char  *from, char  *to);
static int parseBackslashed(const char  **fromChars);
static void adjustBindings(struct bind_Description  *bindings);
class keymap *textview_InitEmacsKeyMap(struct textview_ATKregistry_   *classInfo, class menulist  **normalMenus);
class keymap *textview_InitViCommandModeKeyMap(struct textview_ATKregistry_   *classInfo, class menulist  **Menus);
class keymap *textview_InitViInputModeKeyMap(struct textview_ATKregistry_   *classInfo, class menulist  **Menus);


void textview_NOOPCmd (register class textview  *self)
{
    /* Do nothing.  Used to rebind keys for no operation */
}

/*
 * CharType determines what kind of character has been passed
 * and is used or forward/backward word operations
 */

int charType(register char  c)
	{

	if (isspace(c))
		return (WHITESPACE);
	else if ( isalpha(c) || isdigit(c) || c == '_' )
		return (WORD);
	else
		return(SPECIAL);
}

static char styleString[] = "textview-insert-environment";

/****** EMACS key bindings ********/
static char italicString[]="italic";
static char boldString[]="bold";
static char superString[]="superscript";
static char subString[]="subscript";
static char underString[]="underline";
static char typeString[]="typewriter";
static char biggerString[]="bigger";
static char smallerString[]="smaller";
static char centerString[]="center";
static char leftString[]="flushlfeft";
static char rightString[]="flushright";
static char leftindentString[]="leftindent";
static char newString[]="new";
static struct bind_Description textviewEmacsBindings[]={
    {"textview-show-styles", "\033s",0,0,0,0,(proctable_fptr)textview_ShowStylesCmd,"Show styles at dot.", 0},
    {"textview-show-styles", "\033'\033s", 0, 0, 0, 0,(proctable_fptr) textview_ShowStylesCmd, "Show styles at dot.", 0},
    {"textview-noop", 0, 0, 0, 0, 0,(proctable_fptr) textview_NOOPCmd, "Do absolutely nothing.", 0}, 
    {"textview-insert-environment", "\033'\033l", 0, 0, 0, 0,(proctable_fptr) textview_InsertEnvironment, "Prompt for a style to use for inserting characters.", 0},
    {"textview-show-insert-environment", "\033'\033?", 0, 0, 0, 0,(proctable_fptr) textview_DisplayInsertEnvironment, "Show the environment that will be used for inserting characters.", 0},
    {"textview-left-insert-environment", "\033'\033D", 0, 0, 0, 0,(proctable_fptr) textview_LeftInsertEnvironmentCmd, "Move to the left the environment that will be used for inserting characters.", 0},
    {"textview-right-insert-environment", "\033'\033C", 0, 0, 0, 0,(proctable_fptr) textview_RightInsertEnvCmd, "Move to the right the environment that will be used for inserting characters.", 0},
    {"textview-up-insert-environment", "\033'\033A", 0, 0, 0, 0,(proctable_fptr) textview_UpInsertEnvironmentCmd, "Move up environment that will be used for inserting characters.", 0},
    {"textview-down-insert-environment", "\033'\033B", 0, 0, 0, 0,(proctable_fptr) textview_DownInsertEnvironmentCmd, "Move down environment that will be used for inserting characters.", 0},
    {"textview-up-insert-environment", "\033'\033u", 0, 0, 0, 0, 0, 0, 0},
    {"textview-down-insert-environment", "\033'\033d", 0, 0, 0, 0, 0, 0, 0},
    {styleString, "\033'\033i" , (long)italicString, 0, 0, 0, 0, 0, 0},
    {styleString, "\033'\033b", (long) boldString, 0, 0, 0, 0, 0, 0},
    {styleString, "\033'\033^", (long) superString, 0, 0, 0, 0, 0, 0},
    {styleString, "\033'\033!", (long) subString, 0, 0, 0, 0, 0, 0},
    {styleString, "\033'\033_", (long) underString, 0, 0, 0, 0, 0, 0},
    {styleString, "\033'\033t", (long) typeString, 0, 0, 0, 0, 0, 0},
    {styleString, "\033'\033+", (long) biggerString, 0, 0, 0, 0, 0, 0},
    {styleString, "\033'\033-", (long) smallerString, 0, 0, 0, 0, 0, 0},
    {styleString, "\033'\033=", (long) centerString, 0, 0, 0, 0, 0, 0},
    {styleString, "\033'\033<", (long) leftString, 0, 0, 0, 0, 0, 0},
    {styleString, "\033'\033>", (long) rightString, 0, 0, 0, 0, 0, 0},
    {styleString, "\033'\033\011", (long) leftindentString, 0, 0, 0, 0, 0, 0},
    {"textview-plainer", "\033'\033p", (long) newString, "Plainer~40", (long) newString, textview_NotReadOnlyMenus,(proctable_fptr) textview_PlainerCmd, "Remove style.", 0},
    {"textview-plainer", "\030\020", (long) newString, 0, 0, 0, 0, 0, 0},
    {"textview-plainest","\033'\033P", 0,"Plainest~41", 0, textview_NotReadOnlyMenus,(proctable_fptr) textview_PlainestCmd, "Remove all enclosing styles.", 0},
    {"textview-beginning-of-line", "\001",0,0,0,0,(proctable_fptr)textview_BeginningOfLineCmd,"Move to beginning of current line.", 0},
    {"textview-beginning-of-line", "\033H",0, 0, 0, 0, 0, 0, 0},
    {"textview-backward-character", "\002",0,0,0,0,(proctable_fptr)textview_BackwardCmd,"Move backward one character.", 0},
    {"textview-backward-character", "\033D",0, 0, 0, 0, 0, 0, 0},
    {"textview-delete-next-character", "\004",0,0,0,0,(proctable_fptr)textview_DeleteCmd,"Delete the next character.", 0},
    {"textview-end-of-line", "\005",0,0,0,0,(proctable_fptr)textview_EndOfLineCmd,"Move to end of the current line.", 0},

    {"textview-end-of-line", "\033F",0, 0, 0, 0,0, 0, 0},
    {"textview-forward-character", "\006",0,0,0,0,(proctable_fptr)textview_ForwardCmd,"Move forward one character.", 0},
    {"textview-forward-character", "\033C",0, 0, 0, 0,0, 0, 0},
    {"textview-delete-previous-character", "\010",0,0,0,0,(proctable_fptr)textview_RuboutCmd,"Delete the previous character.", 0},
    {"textview-delete-previous-character", "\177",0, 0, 0, 0,0, 0, 0},
    {"textview-insert-newline", "\015",0,0,0,0,(proctable_fptr)textview_InsertNLCmd,"Insert a newline character.", 0},
    {"textview-insert-soft-newline", "\033\015",0,0,0,0,(proctable_fptr)textview_InsertSoftNewLineCmd,"Insert a soft newline character.", 0},
    {"textview-next-line", "\016",0,0,0,0,(proctable_fptr)textview_NextLineCmd,"Move to next line.", 0},
    {"textview-next-line", "\033B",0, 0, 0, 0,0, 0, 0},
    {"textview-previous-line", "\020",0,0,0,0,(proctable_fptr)textview_PreviousLineCmd,"Move to previous line.", 0},
    {"textview-previous-line", "\033A",0, 0, 0, 0,0, 0, 0},

    {"textview-indent", "\033i",0,0,0,0,(proctable_fptr)textview_IndentCmd,"Indent current line.", 0},

    {"textview-GrabReference", "\030w",0,0,0,0,(proctable_fptr)textview_GrabReference,"Grab the next viewref", 0},

    {"textview-PlaceReference", "\030y",0,0,0,0,(proctable_fptr)textview_PlaceReference,"Create a new viewref", 0},

    {"textview-unindent", "\033u",0,0,0,0,(proctable_fptr)textview_UnindentCmd,"Un-indent current line.", 0},

    {"textview-exch", "\030\030",0,0,0,0,(proctable_fptr)textview_ExchCmd,"Exchange dot and mark.", 0},

    {"textview-select-region", "\033\200",0,0,0,0,(proctable_fptr)textview_SelectRegionCmd,"Select between dot and mark.", 0},
    {"textview-copy-region", "\033w",0,"Copy~11",0,textview_SelectionMenus,(proctable_fptr)textview_CopyRegionCmd,"Copy region to kill-buffer.", 0},

    {"textview-mylf", "\012",0,0,0,0,(proctable_fptr)textview_MyLfCmd,"Skip to next line and indent.", 0},

    {"textview-my-soft-lf", "\033j",0,0,0,0,(proctable_fptr)textview_MySoftLfCmd,"Skip to next line/soft return and indent.", 0},

    {"textview-zap-region", "\027",0,"Cut~10",0, textview_SelectionMenus | textview_NotReadOnlyMenus,(proctable_fptr)textview_ZapRegionCmd,"Remove the text within the selection region and place it in a cutbuffer.", 0},

    {"textview-append-next-cut", "\033\027",0,0,0,0,(proctable_fptr)textview_AppendNextCut, "Make next cut command append to the kill-buffer as opposed to making a new buffer.", 0},

    {"textview-yank", "\031",0,"Paste~10",0,textview_NotReadOnlyMenus | textview_NoSelectionMenus,(proctable_fptr)textview_YankCmd,"Yank text back from kill-buffer.", 0},

    {"textview-insert-file", "\030\t", 0,"File~10,Insert File~10",0,textview_NotReadOnlyMenus,(proctable_fptr)textview_InsertFile,"Prompt for a filename and insert that file's contents into the document.", 0},

    {"textview-rotate-backward-paste", "\033\031",0,0,0,0,(proctable_fptr)textview_BackwardsRotatePasteCmd,"Rotate kill-buffer backwards.", 0},

    {"textview-line-to-top", "\033!",0,0,0,0,(proctable_fptr)textview_LineToTopCmd,"Move current line to top of screen.", 0},

    {"textview-rotate-paste", "\033y",0,0,0,0,(proctable_fptr)textview_RotatePasteCmd,"Rotate kill-buffer.", 0},
    
    {"textview-forward-para", "\033]",0,0,0,0,(proctable_fptr)textview_ForwardParaCmd,"Move to the next paragraph.", 0},

    {"textview-backward-para", "\033[",0,0,0,0,(proctable_fptr)textview_BackwardParaCmd,"Move to the previous paragraph.", 0},

    {"textview-open-line", "\017",0,0,0,0,(proctable_fptr)textview_OpenLineCmd,"Insert blank line at dot", 0},
    {"textview-prev-screen", "\033v",0,0,0,0,(proctable_fptr)textview_PrevScreenCmd,"Move back to previous screen", 0},
    {"textview-prev-screen", "\033G",0, 0, 0, 0, 0, 0, 0},

    {"textview-next-screen", "\026",0,0,0,0,(proctable_fptr)textview_NextScreenCmd,"Move forward to next screen", 0},
    {"textview-next-screen", "\033E",0, 0, 0, 0,0, 0, 0},

    {"textview-prev-screen-move", 0,0,NULL,0,0,(proctable_fptr)textview_PrevScreenMoveCmd,"Move back to previous screen moving dot.", NULL},
    {"textview-next-screen-move", 0,0,NULL,0,0,(proctable_fptr)textview_NextScreenMoveCmd,"Move forward to next screen moving dot.", NULL},

    {"textview-glitch-down", "\021",0,0,0,0,(proctable_fptr)textview_GlitchDownCmd,"Glitch screen down one line.", 0},
    {"textview-glitch-down", "\033z",0, 0, 0, 0,0, 0, 0},

    {"textview-glitch-up", "\032",0,0,0,0,(proctable_fptr)textview_GlitchUpCmd,"Glitch screen up one line.", 0},

    {"textview-twiddle-chars", "\024",0,0,0,0,(proctable_fptr)textview_TwiddleCmd,"Exchange previous two chars.", 0},

    {"textview-kill-line", "\013",0,0,0,0,(proctable_fptr)textview_KillLineCmd,"Kill rest of line.", 0},

    {"textview-MIT-kill-line", 0,0,0,0,0,(proctable_fptr)textview_MITKillLineCmd,"Kill rest of line.", 0},

    {"textview-search", "\023",0,"Search/Spell~1,Forward~10",0,0,(proctable_fptr)textview_SearchCmd,"Search forward.", 0},

    {"textview-reverse-search", "\022",0,"Search/Spell~1,Backward~11",0,0,(proctable_fptr)textview_RSearchCmd,"Search backward.", 0},

    {"textview-search-again",0,0,"Search/Spell~1,Search Again~12",0,0,(proctable_fptr)textview_SearchAgain,"Repeat last search.", 0},

    {"textview-insert-inset-here", "\033\t",0,0,0,0,(proctable_fptr)textview_InsertInsetCmd,"Add inset at this location.", 0},

    {"textview-what-paragraph", "\033N",0,0,0,0,(proctable_fptr)textview_WhatParagraphCmd,"Print current paragraph number.", 0},

    {"textview-goto-paragraph", "\033n",0,0,0,0,(proctable_fptr)textview_GotoParagraphCmd,"Go to specific paragraph.", 0},

    {"textview-query-replace", "\033q",0,"Search/Spell~1,Query Replace~20",0,textview_NotReadOnlyMenus,(proctable_fptr)textview_QueryReplaceCmd,"Query replace.", 0},
    
#ifdef IBM
    {"spell-check-document",0,0,"Search/Spell~1,Check Spelling~30", 0,textview_NotReadOnlyMenus,(proctable_fptr)textview_CheckSpelling,"Checks spelling from the caret on.", "spell"},
#else /* IBM */
    {"spell-check-document",0,0,"Search/Spell~1,Check Spelling~30", 0,textview_NotReadOnlyMenus,0,"Checks spelling from dot to end.", "spell"},
#endif /* IBM */

    {"textview-quote", "\030\021",0,0,0,0,(proctable_fptr)textview_QuoteCmd,"Uninterpreted insert of any char or octal code.", 0},

    {"textview-kill-white-space", "\033k",0,0,0,0,(proctable_fptr)textview_KillWhiteSpaceCmd,"Delete spaces and tabs around dot", 0},

    {"textview-ctrl-at", "\200",0,0,0,0,(proctable_fptr)textview_CtrlAtCmd,"Set a mark.", 0},

    {"textview-backward-word", "\033b",0,0,0,0,(proctable_fptr)textview_BackwardWordCmd,"Move backward to beginning of word.", 0},
    {"textview-delete-next-word", "\033d",0,0,0,0,(proctable_fptr)textview_DeleteWordCmd,"Delete  the next word.", 0},
    {"textview-kill-next-word", 0,0,0,0,0,(proctable_fptr)textview_ForeKillWordCmd,"Kill  the next word, saving it on the kill ring.", 0},
    {"textview-forward-word", "\033f",0,0,0,0,(proctable_fptr)textview_ForwardWordCmd,"Move forward to end of word.", 0},
    {"textview-delete-previous-word", "\033h",0,0,0,0,(proctable_fptr)textview_RuboutWordCmd,"Delete the previous word.", 0},
    {"textview-delete-previous-word", "\033\b",0,0,0,0,(proctable_fptr)textview_RuboutWordCmd,0, 0},
    {"textview-delete-previous-word", "\033\177",0,0,0,0,(proctable_fptr)textview_RuboutWordCmd,0, 0},
    {"textview-kill-previous-word", 0,0,0,0,0,(proctable_fptr)textview_BackKillWordCmd,"Kill the previous word, saving it on the kill ring.", 0},
    {"textview-end-of-text", "\033>",0,0,0,0,(proctable_fptr)textview_EndOfTextCmd,"Move to end of text.", 0},
    {"textview-beginning-of-text", "\033<",0,0,0,0,(proctable_fptr)textview_BeginningOfTextCmd,"Move to beginning of text.", 0},
    {"textview-toggle-character-case", "\036",0,0,0,0,(proctable_fptr)textview_ToggleCase,"Toggle the case of the character at the dot.", 0},
    {"textview-lowercase-word", "\033l",0,0,0,0,(proctable_fptr)textview_LowercaseWord,"Convert word (or region) to lower case.", 0},
    {"textview-uppercase-word",0,0,0,0,0,(proctable_fptr)textview_UppercaseWord,"Convert word (or region) to upper case.", 0},
    {"textview-capitalize-word",0,0,0,0,0,(proctable_fptr)textview_CapitalizeWord,"Capitalize word (or all words within a region).", 0},
    {"textview-cursor-to-top", "\033,",0,0,0,0,(proctable_fptr)textview_CursorToTop,"Moves cursor to the beginning of the line currently at the top of the screen.", 0},
    {"textview-cursor-to-bottom", "\033.",0,0,0,0,(proctable_fptr)textview_CursorToBottom,"Moves cursor to the beginning of the line currently at the bottom of the screen.", 0},
    {"textview-cursor-to-center", "\033/",0,0,0,0,(proctable_fptr)textview_CursorToCenter,"Moves cursor to the beginning of the line currently at the center of the screen.", 0},
    {"textview-change-template",0,0,"File~10,Add Template~31",0,textview_NotReadOnlyMenus,(proctable_fptr)textview_ChangeTemplate, "Change to named template.", 0},
    {"textview-toggle-read-only", "\033~",0,0,0,0,(proctable_fptr)textview_ToggleReadOnly,"Change read only status of text object.", 0},
    {"textview-toggle-expose-styles", 0,0,0,0,0,(proctable_fptr)textview_ToggleExposeStyles,"Expose/hide style information", 0},
    {"textview-toggle-color-styles", 0,0,0,0,0,(proctable_fptr)textview_ToggleColorStyles,"Show/don't show color styles", 0},
    {"textview-toggle-line-display", 0,0,0,0,0,(proctable_fptr)textview_ToggleLineDisplay,"Show/don't show line display", 0},
    {"textview-edit-styles", 0, 0, 0, 0, textview_NotReadOnlyMenus,(proctable_fptr) textview_ExposeStyleEditor,"Expose style editor", 0},
    {"lookzview-edit-styles", 0, 0, "File~10,Edit Styles~30", 0, textview_NotReadOnlyMenus, 0,"Bring up style editor in separate window", "lookzview"},
    {"textview-insert-pagebreak", 0,0,"Page~9,Insert Pagebreak~11",0,textview_NotReadOnlyMenus,(proctable_fptr)textview_InsertPageBreak,"Add page break at this location.", 0},
    {"textview-next-page", 0,0,"Page~9,Next Page~12",0,0,(proctable_fptr)textview_NextPage,"Frame text at next page break object", 0},
    {"textview-last-page", 0,0,"Page~9,Previous Page~13",0,0,(proctable_fptr)textview_LastPage,"Frame text at last page break object", 0},
    {"textview-insert-footnote", 0,0,"Page~9,Insert Footnote~20",0,textview_NotReadOnlyMenus,(proctable_fptr)textview_InsertFootnote,"Add footnote at this location.", 0},
    {"contentv-make-window", 0,0,"Page~9,Table of Contents~30",0,0,0,"Make a table of contents window","contentv"},
    {"textview-open-footnotes",0,0,"Page~9,Open Footnotes~22",0,0,(proctable_fptr)textview_OpenFootnotes,"Open all footnotes", 0},
    {"textview-close-footnotes",0,0,"Page~9,Close Footnotes~23",0,0,(proctable_fptr)textview_CloseFootnotes,"Close all footnotes", 0}, {"textview-write-footnotes",0,0,0,0,0,(proctable_fptr)textview_WriteFootnotes,"Write all footnotes", 0},
    {"textview-toggle-editor", "\033t",0,0,0,0,(proctable_fptr)textview_ToggleEditorCmd, "Switch to vi editor.", 0},
    {"textview-balance", "\033m",0,0,0,0,(proctable_fptr)textview_BalanceCmd,"Balance parentheses, brackets, or braces.", 0},
    {"textview-forward-beginning-of-whitespace-word", NULL,0,NULL,0,0,(proctable_fptr)textview_ForwardBeginWSWordCmd,"Move forward to beginning of whitespace word.", NULL},
    {"textview-extend-forward", NULL,0,NULL,0,0,(proctable_fptr)textview_ExtendForwardCmd,"Extend the dot one char forward.", NULL},
    {"textview-extend-backward", NULL,0,NULL,0,0,(proctable_fptr)textview_ExtendBackwardCmd,"Extend the dot one char backward.", NULL},
    {"textview-extend-forward-line", NULL,0,NULL,0,0,(proctable_fptr)textview_ExtendForwardLineCmd,"Extend the dot one line forward.", NULL},
    {"textview-extend-backward-line", NULL,0,NULL,0,0,(proctable_fptr)textview_ExtendBackwardLineCmd,"Extend the dot one line backward.", NULL},
    {"textview-extend-forward-screen", NULL,0,NULL,0,0,(proctable_fptr)textview_ExtendForwardScreenCmd,"Extend the dot one screen forward.", NULL},
    {"textview-extend-backward-screen", NULL,0,NULL,0,0,(proctable_fptr)textview_ExtendBackwardScreenCmd,"Extend the dot one screen backward.", NULL},
    {"textview-extend-end", NULL,0,NULL,0,0,(proctable_fptr)textview_ExtendEndCmd,"Extend the dot to end of text.", NULL},
    {"textview-extend-beginning", NULL,0,NULL,0,0,(proctable_fptr)textview_ExtendBeginCmd,"Extend the dot to beginning of text.", NULL},
    {"textview-shift-left", NULL,0,NULL,0,0,(proctable_fptr)textview_ShiftLeftCmd,"Shift the view horizontally left.", NULL},
    {"textview-shift-right", NULL,0,NULL,0,0,(proctable_fptr)textview_ShiftRightCmd,"Shift the view horizontally right.", NULL},
    {"textview-shift-home", NULL,0,NULL,0,0,(proctable_fptr)textview_ShiftHomeCmd,"Shift the view horizontally home.", NULL},
    {0,0,0,0,0,0,0,0,0}
};

static void CheckStylePreferences(class keymap  *newKeymap, class menulist  **normalMenus)
{
    struct proctable_Entry *pl;

    /*
      This code is here to handle allow users to select which style
      of plainer that they want to use: old or new
      */

    if (! environ::GetProfileSwitch("UseNewStylePlainer", TRUE)) {
	pl = proctable::Lookup("textview-plainer");
	(newKeymap)->BindToKey( "\030\020", pl, (long) "old");
	if (normalMenus != NULL && *normalMenus != NULL) {
	    (*normalMenus)->AddToML( "Plainer~40", pl, (long) "old", textview_NotReadOnlyMenus);
	}
	pl = proctable::Lookup("textview-plainest");
	if (normalMenus != NULL && *normalMenus != NULL) {
	    (*normalMenus)->AddToML( "Plainest~40", pl, 0, textview_SelectionMenus | textview_NotReadOnlyMenus);
	}
    }

    /* Override binding of our stylizing prefix char with noop */
    if (! environ::GetProfileSwitch("UseStyleKeybindings", TRUE)) {
	pl = proctable::Lookup("textview-noop");
	(newKeymap)->BindToKey( "\033'", pl, 0);
    }
}

/* These two procs ripped out of basics/common/init.c */
/* Translate a key sequence that has ^A, \ddd, and \c conventions. */
static int TranslateKeySequence(const char  *from, char  *to)
        {
    while (*from != '\0') {
        if (*from == '\\') {

            int temp = parseBackslashed(&from);

            if (temp == -1)
                return -1;
            else
                *to++ = temp;
        }
        else if (*from == '^') {
            ++from;
            if (*from == 0)
                return -1;
            *to++ = (*from++) & 0x1f;
        }
        else
            *to++ = *from++;
    }
    *to++ = 0;
    return 0;
}

static int parseBackslashed(const char  **fromChars)
    {

    int returnChar;
    const char *from = *fromChars;
    static char *bsSource = "ebrnt";
    static char *bsDest = "\033\b\r\n\t";

    if (*from == '\\') {
        ++from;
        if (*from == 0)
            return -1;
        if (isdigit(*from)) {

            int sum = 0, i;

            for (i = 0; i < 3; ++i) {
                if (!isdigit(*from))
                    break;
                if (*from == '8' || *from == '9')
                    return -1;
                sum = sum * 8 + *from - '0';
                ++from;
            }
            returnChar = sum;
        }
        else {

            char *p;

            p = strchr(bsSource, *from);
            if (p != NULL)
                returnChar = bsDest[p-bsSource];
            else
                returnChar = *from;
            ++from;
        }
    }
    else
        return -1;
    *fromChars = from;
    return returnChar;
}

static void adjustBindings(struct bind_Description  *bindings)
{
    struct bind_Description *bd;
    const char *stylePrefixPref;
    char stylePrefixStr[100];

    stylePrefixPref = environ::GetProfile("StyleCommandPrefix");
    if (stylePrefixPref != NULL) {
        if (TranslateKeySequence(stylePrefixPref, stylePrefixStr) >= 0) {
            long plen = strlen(stylePrefixStr);

            for (bd = bindings; bd != NULL && (bd->procName || bd->keyVector || bd->menuEntry); bd++) {
		if (bd->keyVector != NULL){
		    if (strcmp(bd->keyVector, stylePrefixStr) == 0) {
			bd->keyVector = NULL;
		    }
		    else if (bd->keyVector[0] == '\033' && bd->keyVector[1] == '\'') {
			long vlen = strlen(bd->keyVector);
			char *oldVector = bd->keyVector;

			if (bd->keyVector = (char *) malloc(plen + vlen - 1)) {
			strcpy(bd->keyVector, stylePrefixStr);
			strcat(bd->keyVector, &oldVector[2]);
			}
		    }
		}
            }
        }
    }
}

class keymap *textview_InitEmacsKeyMap(struct ATKregistryEntry  *classInfo, class menulist  **normalMenus)
{
    class keymap *newKeymap = new keymap;
    register long i;
    char str[2];
    struct proctable_Entry *si;
    
    if(normalMenus!=NULL)
	*normalMenus=new menulist;

    lcKill = im::AllocLastCmd();
    lcYank = im::AllocLastCmd();
    lcMove = im::AllocLastCmd();
    lcDisplayEnvironment = im::AllocLastCmd();
    lcInsertEnvironment = im::AllocLastCmd();
    lcNewLine = im::AllocLastCmd();

    adjustBindings(textviewEmacsBindings);
    bind::BindList(textviewEmacsBindings, newKeymap, *normalMenus, classInfo);

    si=proctable::DefineProc("textview-self-insert", (proctable_fptr) textview_SelfInsertCmd, classInfo, NULL, "Insert a character.");

    str[0] = ' ';
    str[1] = '\0';
    for (i = 32; i < 127; i++)  {
	(newKeymap)->BindToKey( str, si, i);
	str[0]++;
    }
    /* add bindings for iso keyboards */
    str[0] = (unsigned char ) 160;
    for (i = 160 ; i < 256; i++)  {
	(newKeymap)->BindToKey( str, si, i);
	str[0]++;
    }
    (newKeymap)->BindToKey( "\t", si, '\t');

    CheckStylePreferences(newKeymap, normalMenus);

    /* Disable keystroke conversion to VI commands unless preference specifically allows it. */
    if (! environ::GetProfileSwitch("AllowKeyToggleToVIMode", FALSE)) {
	si = proctable::Lookup("textview-noop");
	(newKeymap)->BindToKey( "\033t", si, 0);
    }

    return newKeymap;
}

/*************** VI Command mode key bindings *********************/

static struct bind_Description textviewViCommandModeBindings[]={
    {"textview-noop", 0, 0, 0, 0, 0,(proctable_fptr) textview_NOOPCmd, "Do absolutely nothing.", 0}, 
    {"textview-vi-command", ":",0,0,0,0,(proctable_fptr)textview_ViCommandCmd, "Execute a vi command via commmand line.", 0},
    {"textview-input-mode", "a",0,0,0,0,(proctable_fptr)textview_ToggleViModeCmd, "Switch to input mode.", 0},
    {"textview-input-mode", "i",0, 0, 0, 0,0, 0, 0},

    {"textview-toggle-editor", "\033t",0,0,0,0,(proctable_fptr)textview_ToggleEditorCmd, "Switch to emacs editor.", 0},
    
    {"textview-open-line-before", "O",0,0,0,0,(proctable_fptr)textview_OpenLineBeforeCmd, "Insert a new line before the current line.", 0},

    {"textview-open-line-after", "o",0,0,0,0,(proctable_fptr)textview_OpenLineAfterCmd, "Insert a new line after the current line.", 0},

    {"textview-insert-at-beginning", "I",0,0,0,0,(proctable_fptr)textview_InsertAtBeginningCmd,"Insert at beginning of current line.", 0},

    {"textview-insert-at-end", "A",0,0,0,0,(proctable_fptr)textview_InsertAtEndCmd,"Insert at end of current line.", 0},

    {"textview-change-rest-of-line", "C",0,0,0,0,(proctable_fptr)textview_ChangeRestOfLineCmd,"Replace rest of current line.", 0},

    {"textview-change-rest-of-line", "S",0, 0, 0, 0,0, 0, 0},

    {"textview-change-line", "cc",0,0,0,0,(proctable_fptr)textview_ChangeLineCmd,"Replace current line.", 0},

    {"textview-change-word", "cw",0,0,0,0,(proctable_fptr)textview_ChangeWordCmd,"Replace current word.", 0},

    {"textview-change-selection", "cs",0,0,0,0,(proctable_fptr)textview_ChangeSelectionCmd,"Replace current selection.", 0},

    {"textview-replace-char", "r",0,0,0,0,(proctable_fptr)textview_ReplaceCharCmd,"Replace character after cursor.", 0},

    {"textview-substitute-char", "s",0,0,0,0,(proctable_fptr)textview_SubstituteCharCmd,"Insert over character after cursor.", 0},

    {"textview-beginning-of-previous-line", "-",0,0,0,0,(proctable_fptr)textview_BeginningOfPreviousLineCmd,"Move to beginning of previous line.", 0},

    {"textview-beginning-of-next-line", "+",0,0,0,0,(proctable_fptr)textview_BeginningOfNextLineCmd,"Move to beginning of next line.", 0},

    {"textview-beginning-of-next-line", "\015",0, 0, 0, 0,0, 0, 0},

    {"textview-beginning-of-first-word", "^",0,0,0,0,(proctable_fptr)textview_BeginningOfFirstWordCmd,"Move to beginning of first word on current line.", 0},

    {"textview-backward-character", "\010",0,0,0,0,(proctable_fptr)textview_BackwardCmd,"Move backward one character.", 0},

    {"textview-backward-character", "h",0, 0, 0, 0,0, 0, 0},

    {"textview-backward-character", "\177",0, 0, 0, 0,0, 0, 0},

    {"textview-backward-character", "\033D",0, 0, 0, 0,0, 0, 0},
 
    {"textview-vi-delete-next-character", "x",0,0,0,0,(proctable_fptr)textview_ViDeleteCmd,"Delete the next character.", 0},

    {"textview-end-of-line", "$",0,0,0,0,(proctable_fptr)textview_EndOfLineCmd,"Move to end of the current line.", 0},

    {"textview-end-of-line", "\033F",0, 0, 0, 0,0, 0, 0},
    {"textview-beginning-of-line", "0",0,0,0,0,(proctable_fptr)textview_BeginningOfLineCmd,"Move to beginning of current line.", 0},

    {"textview-beginning-of-line", "\033H",0, 0, 0, 0,0, 0, 0},

    {"textview-forward-character", "040",0,0,0,0,(proctable_fptr)textview_ForwardCmd,"Move forward one character.", 0},

    {"textview-forward-character", "l",0, 0, 0, 0,0, 0, 0},

    {"textview-forward-character", "\011",0, 0, 0, 0,0, 0, 0},

    {"textview-forward-character", " ",0, 0, 0, 0,0, 0, 0},

    {"textview-forward-character", "\033C",0, 0, 0, 0,0, 0, 0},

    {"textview-delete-previous-character", "X",0,0,0,0,(proctable_fptr)textview_RuboutCmd,"Delete the previous character.", 0},

    {"textview-next-line", "\016",0,0,0,0,(proctable_fptr)textview_NextLineCmd,"Move to next line.", 0},

    {"textview-next-line", "\012",0, 0, 0, 0,0, 0, 0},

    {"textview-next-line", "j",0, 0, 0, 0,0, 0, 0},

    {"textview-next-line", "\033B",0, 0, 0, 0,0, 0, 0},

    {"textview-previous-line", "\020",0,0,0,0,(proctable_fptr)textview_PreviousLineCmd,"Move to previous line.", 0},

    {"textview-previous-line", "\013",0, 0, 0, 0,0, 0, 0},

    {"textview-previous-line", "k",0, 0, 0, 0,0, 0, 0},

    {"textview-previous-line", "\033A",0, 0, 0, 0,0, 0, 0},

    {"textview-shift-up", "\005",0,0,0,0,(proctable_fptr)textview_GlitchUpCmd,"Shift display up one line.", 0},

    {"textview-shift-down", "\031",0,0,0,0,(proctable_fptr)textview_GlitchDownCmd,"Shift display down one line.", 0},

    {"textview-GrabReference", "\030w",0,0,0,0,(proctable_fptr)textview_GrabReference,"Grab the next viewref", 0},

    {"textview-PlaceReference", "\030y",0,0,0,0,(proctable_fptr)textview_PlaceReference,"Create a new viewref", 0},

    {"textview-exch", "\030\030",0,0,0,0,(proctable_fptr)textview_ExchCmd,"Exchange dot and mark.", 0},

    {"textview-copy-region", "ys",0,"Copy~11",0,textview_SelectionMenus,(proctable_fptr)textview_CopyRegionCmd,"Yank region to kill-buffer.", 0},
    {"textview-zap-region", "ds",0,"Cut~10",0, textview_SelectionMenus | textview_NotReadOnlyMenus,(proctable_fptr)textview_ZapRegionCmd,"Remove the text within the selection region and place it in a cutbuffer.", 0},

    {"textview-yank", 0,0,"Paste~10",0,textview_NotReadOnlyMenus | textview_NoSelectionMenus,(proctable_fptr)textview_YankCmd,"Yank text back from kill-buffer.", 0},

    {"textview-put-before", "P",0,0,0,0,(proctable_fptr)textview_PutBeforeCmd,"Put text back from kill-buffer, before current line.", 0},

    {"textview-put-after", "p",0,0,0,0,(proctable_fptr)textview_PutAfterCmd,"Put text back from kill-buffer, after current line.", 0},

    {"textview-insert-file", ":r", 0,"File~10,Insert File~10",0,textview_NotReadOnlyMenus,(proctable_fptr)textview_InsertFile,"Prompt for a filename and insert that file's contents into the document.", 0},

    {"textview-forward-para", "}",0,0,0,0,(proctable_fptr)textview_ForwardParaCmd,"Move to the next paragraph.", 0},

    {"textview-backward-para", "{",0,0,0,0,(proctable_fptr)textview_BackwardParaCmd,"Move to the next paragraph.", 0},

    {"textview-prev-screen", "\002",0,0,0,0,(proctable_fptr)textview_PrevScreenCmd,"Move forward to previous screen", 0},

    {"textview-prev-screen", "\033G",0, 0, 0, 0,0, 0, 0},

    {"textview-next-screen", "\006",0,0,0,0,(proctable_fptr)textview_NextScreenCmd,"Move forward to next screen", 0},

    {"textview-next-screen", "\033E",0, 0, 0, 0,0, 0, 0},

    {"textview-down", "\004",0,0,0,0,(proctable_fptr)textview_DownCmd,"Move down. ", 0},

    {"textview-up", "\025",0,0,0,0,(proctable_fptr)textview_UpCmd,"Move up. ", 0},
    {"textview-go-to", "G",0,0,0,0,(proctable_fptr)textview_GoToLineCmd,"Go to line number.", 0},

    {"textview-go-to", "g",0, 0, 0, 0,0, 0, 0},
    
    {"textview-balance", "%",0,0,0,0,(proctable_fptr)textview_BalanceCmd,"Balnce parentheses, brackets, or braces.", 0},

    {"textview-twiddle-chars", "\024",0,0,0,0,(proctable_fptr)textview_TwiddleCmd,"Exchange previous two chars.", 0},

    {"textview-kill-line", "D",0,0,0,0,(proctable_fptr)textview_KillLineCmd,"Kill rest of line.", 0},

    {"textview-kill-line", "d$",0, 0, 0, 0,0, 0, 0},

    {"textview-yank-line", "y$",0,0,0,0,(proctable_fptr)textview_YankLineCmd,"Yank rest of line.", 0},

    {"textview-vi-delete-line", "dd",0,0,0,0,(proctable_fptr)textview_ViDeleteLineCmd,"Delete line.", 0},

    {"textview-vi-yank-line", "yy",0,0,0,0,(proctable_fptr)textview_ViYankLineCmd,"Yank line.", 0},

    {"textview-vi-yank-line", "Y",0, 0, 0, 0,0, 0, 0},

    {"textview-join", "J",0,0,0,0,(proctable_fptr)textview_JoinCmd,"Join current and next line.", 0},

    {"textview-backward-word", "b",0,0,0,0,(proctable_fptr)textview_BackwardWordCmd,"Move backward to beginning of word.", 0},

    {"textview-backward-whitespace-word", "B",0,0,0,0,(proctable_fptr)textview_BackwardWSWordCmd,"Move backward to beginning of word separated by whitespace.", 0},

    {"textview-end-of-word", "e",0,0,0,0,(proctable_fptr)textview_EndOfWordCmd,"Move forward to end of word.", 0},

    {"textview-end-of-whitespace-word", "E",0,0,0,0,(proctable_fptr)textview_EndOfWSWordCmd,"Move forward to end of word separated by whitespace.", 0},

    {"textview-forward-beginning-of-word", "w",0,0,0,0,(proctable_fptr)textview_ForwardWordCmd,"Move forward to beginning of word.", 0},

    {"textview-forward-whitespace-word", "W",0,0,0,0,(proctable_fptr)textview_ForwardWSWordCmd,"Move forward to beginning of word separated space.", 0},

    {"textview-delete-next-word", "dw",0,0,0,0,(proctable_fptr)textview_DeleteWordCmd,"Delete the next word.", 0},

    {"textview-yank-next-word", "yw",0,0,0,0,(proctable_fptr)textview_YankWordCmd,"Yank the next word.", 0},

    {"textview-delete-end-of-word", "de",0,0,0,0,(proctable_fptr)textview_DeleteEndOfWordCmd,"Delete end of word.", 0},

    {"textview-yank-end-of-word", "ye",0,0,0,0,(proctable_fptr)textview_YankEndOfWordCmd,"Yank end of word.", 0},

    {"textview-delete-backward-word", "db",0,0,0,0,(proctable_fptr)textview_DeleteBackwardWordCmd,"Delete the previous word.", 0},

    {"textview-yank-previous-word", "yb",0,0,0,0,(proctable_fptr)textview_YankBackwardWordCmd,"Yank the previous word.", 0},

    {"textview-delete-next-whitespace-word", "dW",0,0,0,0,(proctable_fptr)textview_DeleteWSWordCmd,"Delete the next word separated by whitespace.", 0},

    {"textview-yank-next-whitespace-word", "yW",0,0,0,0,(proctable_fptr)textview_YankWSWordCmd,"Yank the next word separated by whitespace.", 0},

    {"textview-delete-end-of-whitespace-word", "dE",0,0,0,0,(proctable_fptr)textview_DeleteEndOfWSWordCmd,"Delete end of word separated by whitespace.", 0},

    {"textview-yank-end-of-whitespace-word", "yE",0,0,0,0,(proctable_fptr)textview_YankEndOfWSWordCmd,"Yank end of word separated by whitespace.", 0},

    {"textview-delete-previous-whitespace-word", "dB",0,0,0,0,(proctable_fptr)textview_DeleteBackwardWSWordCmd,"Delete the previous word separated by whitespace.", 0},

    {"textview-yank-previous-whitespace-word", "yB",0,0,0,0,(proctable_fptr)textview_YankBackwardWSWordCmd,"Yank the previous word separated by whitespace.", 0},

    {"dynsearch-search-forward", "/",0,"Search/Spell~1,Forward~10",0,0,0,0, 0},

    {"dynsearch-search-reverse", "?",0,"Search/Spell~1,Backward~11",0,0, 0, 0, 0},

    {"dynsearch-search-again","n",0,"Search/Spell~1,Search Again~12",0,0, 0, 0, 0},

    {"dynsearch-search-again-opposite", "N", 0, 0, 0, 0, 0, 0, 0},

    {"textview-query-replace", "\033q",0,"Search/Spell~1,Query Replace~20",0,textview_NotReadOnlyMenus,(proctable_fptr)textview_QueryReplaceCmd,"Query replace.", 0},
#ifdef IBM
    {"spell-check-document",0,0,"Search/Spell~1,Check Spelling~30", 0,textview_NotReadOnlyMenus,(proctable_fptr)textview_CheckSpelling,"Checks spelling from the caret on.", "spell"},
#else
    {"spell-check-document",0,0,"Search/Spell~1,Check Spelling~30", 0,textview_NotReadOnlyMenus,0,"Checks spelling from the caret on.", "spell"},
#endif

    {"textview-ctrl-at", "\200",0,0,0,0,(proctable_fptr)textview_CtrlAtCmd,"Set a mark.", 0},

    {"textview-cursor-to-top", "H",0,0,0,0,(proctable_fptr)textview_CursorToTop,"Moves cursor to the beginning of the line currently at the top of the screen.", 0},

    {"textview-cursor-to-bottom", "L",0,0,0,0,(proctable_fptr)textview_CursorToBottom,"Moves cursor to the beginning of the line currently at the bottom of the screen.", 0},

    {"textview-cursor-to-center", "M",0,0,0,0,(proctable_fptr)textview_CursorToCenter,"Moves cursor to the beginning of the line currently at the center of the screen.", 0},

    {"textview-change-template",0,0,"File~10,Add Template~31",0,textview_NotReadOnlyMenus,(proctable_fptr)textview_ChangeTemplate, "Change to named template.", 0},


/*************** from original textview **************/
    {"textview-indent", "\033i",0,0,0,0,(proctable_fptr)textview_IndentCmd,"Indent current line.", 0},

    {"textview-unindent", "\033u",0,0,0,0,(proctable_fptr)textview_UnindentCmd,"Un-indent current line.", 0},

    {"textview-select-region", "\033\200",0,0,0,0,(proctable_fptr)textview_SelectRegionCmd,"Select between dot and mark.", 0},

    {"textview-append-next-cut", "\033\027",0,0,0,0,(proctable_fptr)textview_AppendNextCut, "Make next cut command append to the cutbuffer as opposed to making a new buffer.", 0},

    {"textview-rotate-backward-paste", "\033\031",0,0,0,0,(proctable_fptr)textview_BackwardsRotatePasteCmd,"Rotate kill-buffer backwards.", 0},

    {"textview-line-to-top", "\033!",0,0,0,0,(proctable_fptr)textview_LineToTopCmd,"Move current line to top of screen.", 0},

    {"textview-rotate-paste", "\033y",0,0,0,0,(proctable_fptr)textview_RotatePasteCmd,"Rotate kill-buffer.", 0},
    
    {"textview-insert-inset-here", "\033\t",0,0,0,0,(proctable_fptr)textview_InsertInsetCmd,"Add inset at this location.", 0},

    {"textview-what-paragraph", "\033N",0,0,0,0,(proctable_fptr)textview_WhatParagraphCmd,"Print current paragraph number.", 0},

    {"textview-goto-paragraph", "\033n",0,0,0,0,(proctable_fptr)textview_GotoParagraphCmd,"Go to specific paragraph.", 0},

#ifdef IBM
    {"spell-check-document",0,0,"Search/Spell~1,Check Spelling~30", 0,textview_NotReadOnlyMenus,(proctable_fptr)textview_CheckSpelling,"Checks spelling from the caret on.", "spell"},
#else /* IBM */
    {"spell-check-document",0,0,"Search/Spell~1,Check Spelling~30", 0,textview_NotReadOnlyMenus,0,"Checks spelling from the caret on.", "spell"},
#endif /* IBM */

    {"textview-kill-white-space", "\033k",0,0,0,0,(proctable_fptr)textview_KillWhiteSpaceCmd,"Delete spaces and tabs around the current pos.", 0},

    {"textview-toggle-character-case", "\036",0,0,0,0,(proctable_fptr)textview_ToggleCase,"Toggle the case of the character at the dot.", 0},
    {"textview-lowercase-word", "\033l",0,0,0,0,(proctable_fptr)textview_LowercaseWord,"Convert word (or region) to lower case.", 0},
    {"textview-uppercase-word",0,0,0,0,0,(proctable_fptr)textview_UppercaseWord,"Convert word (or region) to upper case.", 0},
    {"textview-capitalize-word",0,0,0,0,0,(proctable_fptr)textview_CapitalizeWord,"Capitalize word (or all words within a region).", 0},

    {"textview-toggle-read-only", "\033~",0,0,0,0,(proctable_fptr)textview_ToggleReadOnly,"Change read only status of text object.", 0},
    {"textview-toggle-expose-styles", 0,0,0,0,0,(proctable_fptr)textview_ToggleExposeStyles,"Expose/hide style information", 0},
    {"textview-toggle-color-styles", 0,0,0,0,0,(proctable_fptr)textview_ToggleColorStyles,"Show/don't show color styles", 0},
    {"textview-toggle-line-display", 0,0,0,0,0,(proctable_fptr)textview_ToggleLineDisplay,"Show/don't show line display", 0},
    {"textview-edit-styles", 0, 0, 0, 0, textview_NotReadOnlyMenus,(proctable_fptr) textview_ExposeStyleEditor,"Expose style editor", 0},
    {"textview-insert-pagebreak", 0,0,"Page~9,Insert Pagebreak~11",0,textview_NotReadOnlyMenus,(proctable_fptr)textview_InsertPageBreak,"Add page break at this location.", 0},
    {"textview-next-page", 0,0,"Page~9,Next Page~12",0,0,(proctable_fptr)textview_NextPage,"Frame text at next page break object", 0},
    {"textview-last-page", 0,0,"Page~9,Previous Page~13",0,0,(proctable_fptr)textview_LastPage,"Frame text at last page break object", 0},
    {"textview-insert-footnote", 0,0,"Page~9,Insert Footnote~20",0,textview_NotReadOnlyMenus,(proctable_fptr)textview_InsertFootnote,"Add footnote at this location.", 0},
    {"contentv-make-window", 0,0,"Page~9,Table of Contents~30",0,0,0,"Make a table of contents window","contentv"},
    {"textview-open-footnotes",0,0,"Page~9,Open Footnotes~22",0,0,(proctable_fptr)textview_OpenFootnotes,"Open all footnotes", 0},
    {"textview-close-footnotes",0,0,"Page~9,Close Footnotes~23",0,0, (proctable_fptr)textview_CloseFootnotes,"Close all footnotes", 0},
    {"textview-write-footnotes", 0,0,0,0,0, (proctable_fptr)textview_WriteFootnotes,"Write all footnotes", 0},

    {"textview-show-styles", "\033s",0,0,0,0,(proctable_fptr)textview_ShowStylesCmd,"Show styles at dot.", 0},
    {"textview-show-styles", "\033'\033s", 0, 0, 0, 0,(proctable_fptr) textview_ShowStylesCmd, "Show styles at dot.", 0},

    {"textview-insert-environment", "\033'\033l", 0, 0, 0, 0,(proctable_fptr) textview_InsertEnvironment, "Prompt for a style to use for inserting characters.", 0},
    {"textview-show-insert-environment", "\033'\033?", 0, 0, 0, 0,(proctable_fptr) textview_DisplayInsertEnvironment, "Show the environment that will be used for inserting characters.", 0},
    {"textview-left-insert-environment", "\033'\033D", 0, 0, 0, 0,(proctable_fptr) textview_LeftInsertEnvironmentCmd, "Move to the left the environment that will be used for inserting characters.", 0},
    {"textview-right-insert-environment", "\033'\033C", 0, 0, 0, 0,(proctable_fptr) textview_RightInsertEnvCmd, "Move to the right the environment that will be used for inserting characters.", 0},
    {"textview-up-insert-environment", "\033'\033A", 0, 0, 0, 0,(proctable_fptr) textview_UpInsertEnvironmentCmd, "Move up environment that will be used for inserting characters.", 0},
    {"textview-down-insert-environment", "\033'\033B", 0, 0, 0, 0,(proctable_fptr) textview_DownInsertEnvironmentCmd, "Move down environment that will be used for inserting characters.", 0},
    {"textview-up-insert-environment", "\033'\033u", 0, 0, 0, 0,0, 0, 0},
    {"textview-down-insert-environment", "\033'\033d", 0, 0, 0, 0,0, 0, 0},

    {styleString, "\033'\033i", (long) italicString, 0, 0, 0,0, 0, 0},
    {styleString, "\033'\033b", (long) boldString, 0, 0, 0,0, 0, 0},
    {styleString, "\033'\033^", (long) superString, 0, 0, 0,0, 0, 0},
    {styleString, "\033'\033!", (long) subString, 0, 0, 0,0, 0, 0},
    {styleString, "\033'\033_", (long) underString, 0, 0, 0,0, 0, 0},
    {styleString, "\033'\033t", (long) typeString, 0, 0, 0,0, 0, 0},
    {styleString, "\033'\033+", (long) biggerString, 0, 0, 0,0, 0, 0},
    {styleString, "\033'\033-", (long) smallerString, 0, 0, 0,0, 0, 0},
    {styleString, "\033'\033=", (long) centerString, 0, 0, 0,0, 0, 0},
    {styleString, "\033'\033<", (long) leftString, 0, 0, 0,0, 0, 0},
    {styleString, "\033'\033>", (long) rightString, 0, 0, 0,0, 0, 0},
    {styleString, "\033'\033\011", (long) leftindentString, 0, 0, 0,0, 0, 0}, 

    {"textview-plainer", "\033'\033p", (long) newString, "Plainer~40", (long) newString, textview_NotReadOnlyMenus,(proctable_fptr) textview_PlainerCmd, "Remove style.", 0},
    {"textview-plainer", "\030\020", (long) newString, 0, 0, 0,0, 0, 0},
    {"textview-plainest","\033'\033P", 0,"Plainest~41", 0, textview_NotReadOnlyMenus,(proctable_fptr) textview_PlainestCmd, "Remove all enclosing styles.", 0},
    {0,0,0,0,0,0,0,0, 0}
};

class keymap *textview_InitViCommandModeKeyMap(struct ATKregistryEntry   *classInfo, class menulist  **Menus)
{
    class keymap *newKeymap = new keymap;
    register long i;
    char str[2];
    struct proctable_Entry *dig;

    adjustBindings(textviewViCommandModeBindings);
    bind::BindList(textviewViCommandModeBindings, newKeymap, NULL, classInfo);

    dig=proctable::DefineProc("textview-digit", (proctable_fptr) textview_DigitCmd, classInfo, NULL, "Insert a character.");

    str[0] = ' ';
    str[1] = '\0';
    for (i = str[0] = '0'; i <= '9'; i = ++str[0])
	    (newKeymap)->BindToKey( str, dig, i);

    CheckStylePreferences(newKeymap, Menus);

    return newKeymap;
}

/*************** VI Input mode  key bindings *********************/
static struct bind_Description textviewViInputModeBindings[] = {
    {"textview-noop", 0, 0, 0, 0, 0,(proctable_fptr) textview_NOOPCmd, "Do absolutely nothing.", 0},

    {"textview-delete-previous-character", "\010",0,0,0,0,(proctable_fptr)textview_RuboutCmd,"Delete the previous character.", 0},

    {"textview-delete-previous-character", "\177",0, 0},

    {"textview-insert-newline", "\015",0,0,0,0,(proctable_fptr)textview_InsertNLCmd,"Insert a newline.", 0},

    {"textview-toggle-mode", "\033\033",0,0,0,0,(proctable_fptr)textview_ToggleViModeCmd,"Switch to command mode. ", 0},

    {"textview-toggle-mode", "\005",0, 0},

    {"textview-show-styles", "\033s",0,0,0,0,(proctable_fptr)textview_ShowStylesCmd,"Show styles at dot.", 0},
    {"textview-show-styles", "\033'\033s", 0, 0, 0, 0,(proctable_fptr) textview_ShowStylesCmd, "Show styles at dot.", 0},

    {"textview-insert-environment", "\033'\033l", 0, 0, 0, 0,(proctable_fptr) textview_InsertEnvironment, "Prompt for a style to use for inserting characters.", 0},
    {"textview-show-insert-environment", "\033'\033?", 0, 0, 0, 0,(proctable_fptr) textview_DisplayInsertEnvironment, "Show the environment that will be used for inserting characters.", 0},
    {"textview-left-insert-environment", "\033'\033D", 0, 0, 0, 0,(proctable_fptr) textview_LeftInsertEnvironmentCmd, "Move to the left the environment that will be used for inserting characters.", 0},
    {"textview-right-insert-environment", "\033'\033C", 0, 0, 0, 0,(proctable_fptr) textview_RightInsertEnvCmd, "Move to the right the environment that will be used for inserting characters.", 0},
    {"textview-up-insert-environment", "\033'\033A", 0, 0, 0, 0,(proctable_fptr) textview_UpInsertEnvironmentCmd, "Move up environment that will be used for inserting characters.", 0},
    {"textview-down-insert-environment", "\033'\033B", 0, 0, 0, 0,(proctable_fptr) textview_DownInsertEnvironmentCmd, "Move down environment that will be used for inserting characters.", 0},
    {"textview-up-insert-environment", "\033'\033u", 0},
    {"textview-down-insert-environment", "\033'\033d", 0},

    {styleString, "\033'\033i", (long) italicString, 0, 0, 0, 0, 0, 0},
    {styleString, "\033'\033b", (long) boldString, 0, 0, 0, 0, 0, 0},
    {styleString, "\033'\033^", (long) superString, 0, 0, 0, 0, 0, 0},
    {styleString, "\033'\033!", (long) subString, 0, 0, 0, 0, 0, 0},
    {styleString, "\033'\033_", (long) underString, 0, 0, 0, 0, 0, 0},
    {styleString, "\033'\033t", (long) typeString, 0, 0, 0, 0, 0, 0},
    {styleString, "\033'\033+", (long) biggerString, 0, 0, 0, 0, 0, 0},
    {styleString, "\033'\033-", (long) smallerString, 0, 0, 0, 0, 0, 0},
    {styleString, "\033'\033=", (long) centerString, 0, 0, 0, 0, 0, 0},
    {styleString, "\033'\033<", (long) leftString, 0, 0, 0, 0, 0, 0},
    {styleString, "\033'\033>", (long) rightString, 0, 0, 0, 0, 0, 0},
    {styleString, "\033'\033\011", (long) leftindentString, 0, 0, 0, 0, 0, 0},

    {"textview-plainer", "\033'\033p", (long) newString, "Plainer~40", (long) newString, textview_NotReadOnlyMenus,(proctable_fptr) textview_PlainerCmd, "Remove style.", 0},
    {"textview-plainer", "\030\020", (long) newString, 0},
    {"textview-plainest","\033'\033P", 0,"Plainest~41", 0, textview_NotReadOnlyMenus,(proctable_fptr) textview_PlainestCmd, "Remove all enclosing styles.", 0},
    0
 };
 
class keymap *textview_InitViInputModeKeyMap(struct ATKregistryEntry   *classInfo, class menulist  **Menus)
{
    class keymap *newKeymap = new keymap;
    register long i;
    unsigned char str[2];
    struct proctable_Entry *si, *proc;
    
    adjustBindings(textviewViInputModeBindings);
    bind::BindList(textviewViInputModeBindings, newKeymap, NULL, classInfo);

    si=proctable::DefineProc("textview-self-insert", (proctable_fptr) textview_SelfInsertCmd, classInfo, NULL, "Insert a character.");

    str[0] = ' ';
    str[1] = '\0';
    for (i = 32; i < 127; i++)  {
	(newKeymap)->BindToKey( (char *)str, si, i);
	str[0]++;
    }
    /* add bindings for iso keyboards */
    str[0] = (unsigned char ) 160;
    for (i = 160 ; i < 256; i++)  {
	(newKeymap)->BindToKey( (char *)str, si, i);
	str[0]++;
    }
    (newKeymap)->BindToKey( "\t", si, '\t');

    CheckStylePreferences(newKeymap, Menus);

    return newKeymap;
}

