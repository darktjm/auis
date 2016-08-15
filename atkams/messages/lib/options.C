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

#include <andrewos.h>

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atkams/messages/lib/RCS/options.C,v 1.5 1994/08/11 03:05:07 rr2b Stab74 $";
#endif


 

#include <textview.H>
#include <stdio.h>
#include <errprntf.h>

#include <view.H>
#include <im.H>
#include <text.H>
#include <message.H>
#include <lpair.H>
#include <style.H>
#include <fontdesc.H>
#include <environ.H>
#include <value.H>
#include <valueview.H>
#include <atom.H>
#include <atomlist.H>

#include <cui.h>
#include <fdphack.h>
#include <ams.H>
#include <amsutil.H>
#include <captions.H>
#include <sendmessage.H>
#include <options.H>
#include <t822view.H>
#include <folders.H>


ATKdefineRegistry(options, ATK, NULL);
#ifndef NORCSID
#endif
void ExpLevelHit(ATK * self, class value  *val, long  which , long  hisrock);
void IntegerHit(ATK * self, class value  *val, long  which , long  hisrock);
void ButtonHit(ATK * self, class value  *val, long  which , long  hisrock);
int saveprofilestring(char  *prog , char  *pref , char  *val, int  warn);
int CountCommas(char  *s);

#define PREF_ERR 0
#define PREF_TEMP 1
#define PREF_PERM 2
#define PREF_ABORT 3

#define OT_EXPLEVEL 0
#define OT_INTEGER 1
#define OT_SPECIAL 3

#define OT_INTEGER_MAXCLASSMENU 0
#define OT_INTEGER_FONTSIZE 1
#define OT_INTEGER_FOLDERPIXELS 2
#define OT_INTEGER_HEADBODYSPLIT 3
#define OT_INTEGER_VERYNARROW 4
#define OT_INTEGER_CKP 5
#define OT_INTEGER_MAXTOTALCLASSMENU 6

static char *IntegerMessages[] = {
    "Changed the maximum size of the File Into... menu to %d.",
    "Changed the default font size to %d.",
    "Changed the number of pixels for the folders view to %d.",
    "Changed the percentage of caption/body area for bodies to %d.",
    "Changed the threshhold for the 'narrow' folder list setup to %d pixels.",
    "Changed the checkpoint interval to %d seconds.",
    "Changed the maximum number of folders on a File Into... menu card to %d."
};

int IntegerDefaults[] = {8, 12, 8, 50, 12, 1, 25};

#define OT_SPECIAL_CRUCIALCLASSES 0
#define OT_SPECIAL_FONT 1
#define OT_SPECIAL_CKPWHERE 2
#define OT_SPECIAL_KEYHEADS 3

struct OptionChoice {
    char *OptionName;
    int OptType, OptParm, IsStartup;
    char *HelpText;
};

static struct OptionChoice Options[] = {
	{"Keystroke commands", OT_EXPLEVEL, EXP_KEYSTROKES, 0, "If you turn on keystroke commands, Messages will make a wide variety of its features available by typing single-letter commands at the keyboard.  See the help on 'messages-keys' for a listing of the keystroke commands available."},
	{"Marking of messages", OT_EXPLEVEL, EXP_MARKING, 0, "If you turn on marking of messages, then Messages will allow you to mark a set of messages and act on them as a group.  You mark messages by clicking on the captions with the right mouse button once you have turned on message marking.  When you have marked messages, two new menu cards ('Marked Messages' and 'Send/File Marked') appear, offering you several ways to act on all the messages at once."},
	{"Basic folder menus/features", OT_EXPLEVEL, EXP_FILEINTO, 0, "The basic menus include 'Create Folder', 'Rename folder', 'Delete Folder', and 'Expose/Hide Personal Folders'."},
	{"File into... menus", OT_EXPLEVEL, EXP_FILEINTOMENU, 0, "These menus allow you to insert messages into your personal folders with a simple menu item.  You can control the contents of the 'File into...' menus with other options, explained below."},
	{"File into... total number of folders", OT_INTEGER, OT_INTEGER_MAXCLASSMENU, 1, "This option tells Messages how big you will allow your 'File into...' menu to grow when it is in the shrunken (default) state; by default, it is limited to 8 items.  This is primarily of interest to people with MANY mail folders."},
	{"File into... folders per menu card", OT_INTEGER, OT_INTEGER_MAXTOTALCLASSMENU, 1, "This option tells Messages how big you will allow each individual 'File into...' menu card to grow; by default, it is limited to 25 items, setting it to anything larger will have no effect.  This is primarily of interest to people with MANY mail folders."},
	{"File into... menu contents", OT_SPECIAL, OT_SPECIAL_CRUCIALCLASSES, 0, "You can specify a special ordering or set of extra items to appear on your 'File into...' menu."},
	{"Show More/Show Next menus", OT_EXPLEVEL, EXP_SHOWMORENEXT, 0, "These menus items are useful for paging though messages in sequence via menu options.  They are equivalent to the <space> and 'n' keystrokes, if you have keyboard commands turned on."},
	{"Mark as Unseen menu", OT_EXPLEVEL, EXP_MARKASUNREAD, 0, "This menu item allows you to mark messages as 'never seen', but this information will only be remembered permanently (that is, beyond this Messages session) for your personal messages."},
	{"Extra Marked Messages Menus", OT_EXPLEVEL, EXP_MARKEDEXTRAS, 0, "This option adds 'Append To Folder', 'Append To File', and 'Copy All Into' to the 'Send/File Marked' menu card."},
	{"Set Quit Here menu", OT_EXPLEVEL, EXP_SETQUITHERE, 0, "This menu can be used to tell Messages that, when you leave the current folder and look at another one, then the next time you come back to the current folder, messages below where you are now should be shown to you again as 'new'.  In response to 'Set Quit Here', Messages will re-position the little line that indicates how far you have read."},
	{"Reply to Readers/Both menus", OT_EXPLEVEL, EXP_THREEREPLIES, 0, "Normally, the 'Reply to All' menu makes a 'best guess' as to whether or not to send the mail to the sender as well. If you choose, you can have two menus in its place, 'Reply to Readers' and 'Reply to Both', which let you make the decision explicitly."},
	{"Descramble/Fixed Width menus", OT_EXPLEVEL, EXP_FORMATMENUS, 0, "This option adds two menus to the 'This Message' card which allow you to easily put the message on display in fixed width, to scramble it with the 'rot13' algorithm."},
	{"Special headers to highlight", OT_SPECIAL, OT_SPECIAL_KEYHEADS, 0, "This option alters the list of headers that are highlighted (not hidden) when you display a message.  Your entry should be a list of words separated by colons, with no spaces.  The default list is \"From:Date:Subject:To:CC:ReSent-From:ReSent-To\".  The option after this one can be used to alter the meaning of THIS option, so that it is a list of those headers NOT to highlight, and all non-specified headers WILL be highlighted."},
	{"Highlight non-listed headers", OT_EXPLEVEL, EXP_SHOWALLBUTKEYS, 0, "By default, most of a message's headers are hidden from you, and you have to scroll backwards to see them.  You can alter the list of those few headers that ARE highlighted, using the previous option.  Using THIS option, you can change it so that the list of headers below is a list of those headers that do NOT get highlighted by default."},
	{"Highlight NO headers", OT_EXPLEVEL, EXP_SHOWNOHEADS, 0, "This option turns off all header highlighting, leaving all of the bodies area for the message bodies themselves"},
	{"Automatically purge", OT_EXPLEVEL, EXP_PURGEONQUIT, 0, "If you set 'Automatically purge', then deleted messages will ALWAYS be purged whenever you quit Messages.  Ordinarily, you are asked if you want to purge."},
	{"Terse subscription question", OT_EXPLEVEL, EXP_SUBSEXPERT, 0, "The terse version of the 'subscribe to <new folder name>' question assumes that you understand the meaning of unusual subscription types like ask-subscribed.  By default, you get a short question and then a very verbose question, explaining the types of subs.  The terse question is simpler if you already understand the subscription types."},
	{"Offer core dump with bug report", OT_EXPLEVEL, EXP_DUMPCORE, 0, "If you set this option, then whenever messages offers you the option of sending an automatic bug report, it will also offer a second option that will dump core after doing so, so that you may show the core dump to the local messages maintainer.  At some sites, the program may be configured so that this option has no effect."},
	{"Warp mouse to sending window", OT_EXPLEVEL, EXP_WARPWINDOW, 0, "If this option is set, then whenever you do something in the messages window to send mail (such as 'reply to sender') the mouse focus will hop to that other window."},
	{"Set checkpoint frequency", OT_INTEGER, OT_INTEGER_CKP, 0, "This option tells Messages how often (in half-minutes) to save a copy of the mail you are composing and other relevant state."},
	{"More white space in captions", OT_EXPLEVEL, EXP_WHITESPACE, 1, "This option tells Messages to display the captions in a larger font and with more white space between them."},
	{"Default font family", OT_SPECIAL, OT_SPECIAL_FONT, 1, "This option allows you to change the default font family."},
	{"Default font size", OT_INTEGER, OT_INTEGER_FONTSIZE, 1, "This option allows you to change the default font size."},
	{"Fixed-width caption font", OT_EXPLEVEL, EXP_FIXCAPTIONS, 1, "This option tells Messages that you want to use a fixed-width caption font.  The big advantage of doing so currently is that the subject, date, and sender will line up in neat columns."},
	{"Set 'narrow' threshhold", OT_INTEGER, OT_INTEGER_VERYNARROW, 1, "This option controls how narrow (in centimeters, approximately) the folders view can get before changing to its 'narrow' layout, in which the buttons are at the top rather than the side."},
	{"Don't show first folder at startup", OT_EXPLEVEL, EXP_NOFIRSTFOLDER, 1, "This option tells Messages whether or not to show you the first folder on your folder list at normal startup."},
	{"Add Punt menu", OT_EXPLEVEL, EXP_PUNTMENU, 0, "This option tells Messages whether or not to add an extra 'Punt' menu item which marks all of the current folder as 'seen' and takes you on to the next folder."},
	{"Add Punt button", OT_EXPLEVEL, EXP_PUNTBUTT, 1, "This option tells Messages whether or not to add an extra button at the top of the screen.  The extra button always moves the 'last message seen' marker to the end of the folder, and it also moves on to the next folder if you used the left mouse button."},
	{"Show Folders Next To Captions", OT_EXPLEVEL, EXP_SIDEBYSIDE, 1, "This option tells Messages whether you want your folder lists to appear above the captions area (the default) or to the left of it."},
	{"Screen space for folders", OT_INTEGER, OT_INTEGER_FOLDERPIXELS, 1, "This option controls how much of the startup single-window messages screen, in centimeters (approximately), is devoted to folders."}, 
	{"Caption/body split in Messages window", OT_INTEGER, OT_INTEGER_HEADBODYSPLIT, 1, "This option is a percentage, telling Messages (the single-window version) how much of the startup window NOT already allocated to the folders region should be allocated to the bodies region."},
	{"Expose/Grow Folders After Hiding Sendmessage", OT_EXPLEVEL, EXP_GROWFOLDS, 0, "This option controls whether or not the messages/folders window will be exposed and/or enlarged automatically when the sendmessage window is hidden."},
	{"Allow sending of empty messages", OT_EXPLEVEL, EXP_SENDEMPTY, 0, "This option determines whether or not you will be allowed to send a message with no content."},
	{"Extra style menus", OT_EXPLEVEL, EXP_BIGSTYLES, 0, "By default, only a small set of formatting menus appear in the message composition window.  This menu option approximately doubles the number of styles available for formatting the messages you send."},
	{"Insert Header menu", OT_EXPLEVEL, EXP_INSERTHEADER, 0, "This menu is useful for mail gurus who want to add new headers to outgoing messages."},
	{"Check Recipients menu", OT_EXPLEVEL, EXP_CHECKRECIP, 0, "This menu is useful for checking the recipient list to make sure it is valid BEFORE you send the mail."},
	{"Checkpoint sendmessage in /tmp", OT_EXPLEVEL, EXP_CKPONTMP, 1, "This option, if set, tells sendmessage to write its periodic checkpoint files on /tmp.  Not using /tmp is more secure, as /tmp files can easily get lost (especially if you're running on a remote machine) but on an AFS system, using /tmp (which is not on AFS) will be notably faster.  If this option is off, checkpoint files will be written as guided by the ``Set checkpoint directory'' option, which follows."},
	{"Set checkpoint directory", OT_SPECIAL, OT_SPECIAL_CKPWHERE, 1, "This option tells Messages where to save a copy (checkpoint) of the mail you are composing.  (Using ~ as shorthand for your home directory is OK.)  If the ``Checkpoint sendmessage in /tmp'' option is set, it overrides this option.  If neither option is set, checkpoint files are written in your home directory."},
	{"Hide after sending", OT_EXPLEVEL, EXP_HIDEAFTER, 0, "This option causes the message composition window to be hidden after successful delivery."},
	{"Clear after sending", OT_EXPLEVEL, EXP_CLEARAFTER, 0, "This option causes the message composition window to be cleared out after successful delivery."},
	{"Sign mail", OT_EXPLEVEL, EXP_SIGNMAIL, 0, "This option causes the file '.signature' in your home diretory to be appended to your mail before it is sent."},
	{"Keep blind copy", OT_EXPLEVEL, EXP_KEEPBLIND, 0, "This option tells Messages whether or not to keep a blind (invisible to the recipients) copy of every message you send."},
	{"Send Formatted/Unformatted menus", OT_EXPLEVEL, EXP_FORCESEND, 0, "These menus can be used to avoid being asked about sending formatted messages to external recipients.  Send Formatted should be used only when all of the recipients will be using an AMS interface to read your message; Send Unformatted should be used only when none of them will be doing so."},
	{"Show folder icon", OT_EXPLEVEL, EXP_FILEICONCAPTIONS, 0, "If you tell Messages to show folder icons, it will place a special icon next to the help icon wherever the name of a folder appears.  Clicking with the left button on the folder icon will insert the current message into that folder; clicking with the right button on the folder icon will insert all marked messages into that folder, if you have turned on the option that allows you to mark sets of messages."},
	{"Expose folders on mail-only startup", OT_EXPLEVEL, EXP_SHOWCLASSES, 1, "This option tells Messages that when you start up in 'mail-only' mode (the -m option), it should expose a list of all of your personal mail folders."},
	{"Append By Name menu", OT_EXPLEVEL, EXP_APPENDBYNAME, 1, "This menu allows you to file into a folder by name in such a way that the message is placed at the very end of the folder, rather than being filed into a folder and sorted by date."},
        {"Hide by Vanishing", OT_EXPLEVEL, EXP_VANISH, 0, "This option, if turned on, will cause Messages to hide windows by making them vanish completely rather than by shrinking them to an icon (Messages optionally hides the sending window, for example, after a message is sent).  This option will only work reliably with window managers that conform to the ICCCM X11 standards.  For others, it may or may not do the right thing.  Try it and see."},
	{NULL, 0, 0, 0, NULL}
};



void ExpLevelHit(ATK * self, class value  *val, long  which , long  hisrock)
{
    int parm = Options[which].OptParm;
    char MyBuf[500];

    if (hisrock == value_OBJECTDESTROYED) return;
    ams::WaitCursor(TRUE);
    amsutil::SetOptBit(parm, !amsutil::GetOptBit(parm));
    amsutil::SetPermOptBit(parm, amsutil::GetOptBit(parm));
    amsutil::SetOptMaskBit(parm, 1);
    amsutil::BuildOptionPreference(MyBuf);
    if (saveprofilestring("messages", "BinaryOptions", MyBuf, Options[which].IsStartup) != PREF_ABORT) {
	sprintf(MyBuf, "Turned %s the \"%s\" option%s.", amsutil::GetOptBit(parm) ? "ON" : "OFF", Options[which].OptionName, Options[which].IsStartup ? " (starting next time)" : "");
	message::DisplayString(NULL, 10, MyBuf);
	im::ForceUpdate();
    }
    ams::WaitCursor(FALSE);
}

void IntegerHit(ATK * self, class value  *val, long  which , long  hisrock)
{
    char Msg[1000], AnsBuf[25], *pref;
    int parm, numval;

    if (hisrock == value_OBJECTDESTROYED) return;
    ams::WaitCursor(TRUE);
    parm = Options[which].OptParm;
    numval = (val)->GetValue();
    switch(parm) {
	case OT_INTEGER_MAXTOTALCLASSMENU:
	    pref = "MaxTotalClassMenu";
	    break;
	case OT_INTEGER_MAXCLASSMENU:
	    pref = "MaxClassMenu";
	    break;
	case OT_INTEGER_FONTSIZE:
	    pref = "FontSize";
	    break;
	case OT_INTEGER_FOLDERPIXELS:
	    pref = "FolderPixels";
	    numval *= 22;
	    break;
	case OT_INTEGER_HEADBODYSPLIT:
	    pref = "HeadBodySplit";
	    break;
	case OT_INTEGER_VERYNARROW:
	    pref = "VeryNarrowFolders";
	    numval *= 22;
	    break;
	case OT_INTEGER_CKP:
	    pref = "CheckpointFrequency";
	    numval *= 30;
	    break;
	default:
	    break;
    }
    sprintf(AnsBuf, "%d", numval);
    if (saveprofilestring("messages", pref, AnsBuf, Options[which].IsStartup) != PREF_ABORT) {
	if (parm == OT_INTEGER_MAXCLASSMENU || parm == OT_INTEGER_MAXTOTALCLASSMENU) ams::ResetClassList();
	if (parm == OT_INTEGER_CKP) ams::SetCheckpointFrequency(numval);
    }
    sprintf(Msg, IntegerMessages[parm], numval);
    message::DisplayString(NULL, 10, Msg);
    ams::WaitCursor(FALSE);
}
   
void ButtonHit(ATK * self, class value  *val, long  which , long  hisrock)
{
    char Msg[1000], AnsBuf[5000], *pref, *prompt = "bogus";
    const char *defaultans = "bogus";
    int parm, numans;

    if (hisrock == value_OBJECTDESTROYED) return;
    parm = Options[which].OptParm;
    switch(parm) {
	case OT_SPECIAL_CRUCIALCLASSES:
	    pref = "CrucialClasses";
	    prompt = "Enter the list of folders you want on the 'File Into...' menu: ";
	    break;
	case OT_SPECIAL_FONT:
	    pref = "FontFamily";
	    prompt = "Enter the name of the font family you want to use: ";
	    break;
	case OT_SPECIAL_CKPWHERE:
	    pref = "CheckpointDir";
	    prompt = "Enter the name of the directory where you want to write checkpoint files: ";
	    break;
	case OT_SPECIAL_KEYHEADS:
	    pref = "KeyHeaders";
	    prompt = "Enter or edit the list of 'special' highlighting headers: ";
	    break;
    }
    defaultans = environ::GetProfile(pref);
    if (message::AskForString(NULL, 99, prompt, defaultans, AnsBuf, sizeof(AnsBuf)) < 0) {
	return;
    }
    ams::WaitCursor(TRUE);
    if (saveprofilestring("messages", pref, AnsBuf, Options[which].IsStartup) != PREF_ABORT) {
	switch(parm) {
	    case OT_SPECIAL_CRUCIALCLASSES:
		numans = environ::GetProfileInt("maxclassmenu", 8) -1;
		if (numans < CountCommas(AnsBuf)) {
		    char Nbuf[20];
		    sprintf(Nbuf, "%d", 1+CountCommas(AnsBuf));
		    saveprofilestring("messages", "MaxClassMenu", Nbuf, TRUE);
		}
		ams::ResetClassList();
		break;
	    case OT_SPECIAL_KEYHEADS:
		amsutil::ParseKeyHeaders();
		break;
	}
    }
    sprintf(Msg, "Set %s preference to %s", pref, AnsBuf);
    message::DisplayString(NULL, 10, Msg);
    ams::WaitCursor(FALSE);
}

void options::SetMessagesOptions(class t822view  *bv) 
{
    static char *Intro1 = "Setting Messages Options";
    static char *Intro2 = "\nThe Messages program has a large number of options that you can alter to tailor the program's behavior to suit your needs.  A list of the options appears in the caption area, above.  Clicking on an option in the list above will scroll in this area to show you that option.\n\nTo set an option, you need to either left click a switch On or Off, change a slider value by dragging the slider or by clicking on the slider with the left button to increase the value or the right to decrease the value, or click on the Alter button which will prompt you to change the option's value.\n";
    int i, tpos = 0, len, def, cappos = 0, scaledown;
    int envPos, envLen;
    class value *v;
    static class style *MajorHeadingStyle = NULL, *CapStyle = NULL;
    char *pref;
    class textview *tv = (class textview *) bv;
    class text *t = (class text *) (bv)->GetDataObject();
    class captions *cap = (bv)->GetCaptions();
    class text *captext = (class text *) (cap)->GetDataObject();
    class folders *f;
/*
    static struct atomlist *  AL_bodyfont;
    static struct atomlist *  AL_bodyfont_size;
*/
    static class atomlist *  AL_label;
    static class atomlist *  AL_label2;
    static class atomlist *AL_max_value;
    static class atomlist *AL_min_value;
    static class atom *  A_long;
    static class atom *  A_string;
    static class atomlist *  AL_bottom_label;
    static class atomlist *  AL_top_label;

    ams::WaitCursor(TRUE);
    if (MajorHeadingStyle == NULL) {
	MajorHeadingStyle = new style;
	(MajorHeadingStyle)->SetName( "majorheading");
	(MajorHeadingStyle)->SetFontSize( style_ConstantFontSize, 16);
	(MajorHeadingStyle)->AddNewFontFace( (long) fontdesc_Bold);
	(MajorHeadingStyle)->SetJustification( style_Centered);

	CapStyle = new style;
	(CapStyle)->SetJustification( style_Centered);
	(CapStyle)->AddNewFontFace( (long) fontdesc_Italic);

/*	AL_bodyfont = atomlist_StringToAtomlist("bodyfont");
	AL_bodyfont_size = atomlist_StringToAtomlist("bodyfont-size");
*/
	AL_label = atomlist::StringToAtomlist("buttonV.label");
	AL_label2 = atomlist::StringToAtomlist("sliderV.label");
	AL_bottom_label = atomlist::StringToAtomlist("bottom label");
	AL_top_label = atomlist::StringToAtomlist("top label");
	A_long = atom::Intern("long");
	A_string = atom::Intern("string");
	AL_max_value = atomlist::StringToAtomlist("max_value");
	AL_min_value = atomlist::StringToAtomlist("min_value");
    }

    ((class view *) tv)->PostResource(AL_top_label,A_string, (long)"On");
    ((class view *) tv)->PostResource(AL_bottom_label,A_string, (long)"Off");
    ((class view *) tv)->PostResource(AL_min_value,A_long,0);
    ((class view *) tv)->PostResource(AL_max_value,A_long,100);
    ((class view *) tv)->PostResource(AL_label,A_string, (long)"Alter");
    ((class view *) tv)->PostResource(AL_label2,A_string, (long)"Adjust");
    (t)->ClearCompletely();
    (cap)->ClearAndUpdate( FALSE, TRUE);
    ams::WaitCursor(TRUE);
    len = strlen(Intro1);
    (captext)->AlwaysInsertCharacters( cappos, Intro1, len);
    envPos = cappos;
    envLen = len;
    cappos += len;
    (captext)->AlwaysInsertCharacters( cappos, "\n", 1);
    (captext)->AlwaysAddStyle( envPos, envLen, CapStyle);
    ++cappos;
    (t)->AlwaysInsertCharacters( tpos, Intro1, len);
    envPos = tpos;
    tpos += len;
    (t)->AlwaysInsertCharacters( tpos, "\n", 1);
    (t)->AlwaysAddStyle( envPos, envLen, MajorHeadingStyle);
    ++tpos;
    len = strlen(Intro2);
    (t)->AlwaysInsertCharacters( tpos, Intro2, len);
    tpos += len;
    (t)->AlwaysInsertCharacters( tpos, "\n", 1);
    ++tpos;
    for (i=0; Options[i].OptionName; ++i) {
	len = strlen(Options[i].OptionName);
	(captext)->AlwaysInsertCharacters( cappos, Options[i].OptionName, len);
	envPos = cappos;
	envLen = len;
	cappos += len;
	(captext)->AlwaysInsertCharacters( cappos, "\n", 1);
	(captext)->AlwaysAddStyle( envPos, envLen, CapStyle);
	++cappos;

	/* Insert OptionName[i] and save position of first character for InsertStyle */
	(t)->AlwaysInsertCharacters( tpos, Options[i].OptionName, len);
	envPos = tpos;

	tpos += len;
	(t)->AlwaysInsertCharacters( tpos++, "\n", 1);
	v = new value;
	if (Options[i].OptType == OT_EXPLEVEL) {
	    (v)->SetValue( amsutil::GetPermOptBit(Options[i].OptParm) ? 1 : 0);
	    (v)->AddCallBackObserver( t, (value_fptr)ExpLevelHit, i);
	    (t)->AlwaysAddView( tpos++, "onoffV", v);
	} 
	else if (Options[i].OptType == OT_INTEGER) {
	    (t)->AlwaysAddView( tpos++, "sliderV", v);
	    def = IntegerDefaults[Options[i].OptParm];
	    scaledown = 1;
	    switch(Options[i].OptParm) {
		case OT_INTEGER_MAXTOTALCLASSMENU:
		    pref = "messages.MaxTotalClassMenu";
		    break;
		case OT_INTEGER_MAXCLASSMENU:
		    pref = "messages.MaxClassMenu";
		    break;
		case OT_INTEGER_FONTSIZE:
		    pref = "messages.FontSize";
		    break;
		case OT_INTEGER_FOLDERPIXELS:
		    pref = "messages.FolderPixels";
		    def = amsutil::GetOptBit(EXP_SIDEBYSIDE) ? 9 : 4;
		    scaledown = 22;
		    break;
		case OT_INTEGER_HEADBODYSPLIT:
		    pref = "messages.HeadBodySplit";
		    break;
		case OT_INTEGER_VERYNARROW:
		    pref = "messages.VeryNarrowFolders";
		    scaledown = 22;
		    break;
		case OT_INTEGER_CKP:
		    pref = "messages.CheckpointFrequency";
		    scaledown = 30;
		    break;
	    }
	    (v)->SetValue( environ::GetProfileInt(pref, def*scaledown)/scaledown);
	    (v)->AddCallBackObserver( t, (value_fptr)IntegerHit, i);
	} 
	else {
	    (v)->AddCallBackObserver( t, (value_fptr)ButtonHit, i);
	    (t)->AlwaysAddView( tpos++, "buttonV", v);
	}

	/* Set MajorHeadingStyle around OptionName[i] and value view that follows */
	envLen = tpos - envPos;
	(t)->AlwaysInsertCharacters( tpos, "\n\n", 2);
	tpos += 2;
	(t)->AlwaysAddStyle( envPos, envLen, MajorHeadingStyle);

	len = strlen(Options[i].HelpText);
	(t)->AlwaysInsertCharacters( tpos, Options[i].HelpText, len);
	tpos += len;
	(t)->AlwaysInsertCharacters( tpos, "\n\n", 2);
	tpos += 2;
    }
    (captext)->NotifyObservers( observable_OBJECTCHANGED);
    f = (cap)->GetFolders();
    (f)->HighlightFolder( NULL, NULL);
    (f)->WantUpdate( f);
    ams::WaitCursor(FALSE);
}

int saveprofilestring(char  *prog , char  *pref , char  *val, int  warn)
{
    char ErrorText[256];
    int code;

    if (warn) message::DisplayString(NULL, 10,"Warning: this change will not take effect until you next run messages.");
    if (!val || *val == '\0' || *val == '\n') {
	val = "     ";
    }
    if (code = amsutil::setprofilestring(prog, pref, val)) {
	sprintf(ErrorText, "Warning: could not rewrite your preferences file (%d, %d)", code, errno);
	(ams::GetAMS())->ReportError( ErrorText, ERR_WARNING, FALSE, 0);
	return(PREF_ERR);
    } else {
	sprintf(ErrorText, "Your '%s' preference has been permanently changed.", pref);
	message::DisplayString(NULL, 10, ErrorText);
	return(PREF_PERM);
    }	
}

int CountCommas(char  *s)
{
    int commas = 0;
    if (s) {
	while (*s) {
	    if (*s++ == ',') ++commas;
	}
    }
    return(commas);
}
