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

 /*
 * runbtnv -- runbuttonview
 *
 */

#include <andrewos.h>
#include <bind.H>
#include <keymap.H>
#include <keystate.H>
#include <im.H>
#include <menulist.H>
#include <message.H>
#include <sbutton.H>
#include <runbutton.H>
#include <rbttnv.H>

// For printing...
#include <text.H>
#include <textview.H>
#include <style.H>
#include <fontdesc.H>
#include <fnote.H>
#include <fnotev.H>

ATKdefineRegistry(runbuttonview, sbuttonv, runbuttonview::InitializeClass);

#define PROMPTFONT "andysans12b"

menulist *runbuttonview::sMenuList = 0;
keymap *runbuttonview::sKeyMap = 0;
runbuttonview *runbuttonview::rbViewList = 0;

static struct bind_Description runbtnBindings[] = {
    {"runbutton-set-command", 0, 0, "RunButton,Set Command~12", 0, 0,
     (proctable_fptr)runbuttonview::SetCommandProc, "Set the runbutton command to execute."},
    {"runbutton-set-label", 0, 0, "RunButton,Set Label~22", 0, 0,
     (proctable_fptr)runbuttonview::SetLabelProc, "Set the runbutton label."},
    NULL
};

boolean runbuttonview::InitializeClass()
{
    runbuttonview::sMenuList = new menulist;
    runbuttonview::sKeyMap = new keymap;
    bind::BindList(runbtnBindings, runbuttonview::sKeyMap, runbuttonview::sMenuList, &runbuttonview_ATKregistry_ );
    return TRUE;
}

runbuttonview::runbuttonview()
{
    ATKinit;
    ml = sMenuList->DuplicateML(this);
    ks = keystate::Create(this, sKeyMap);
    next = rbViewList;
    rbViewList = this;
    tidata = NULL;
    if (!ml || !ks)
	THROWONFAILURE(FALSE);
}

runbuttonview::~runbuttonview()
{
    runbuttonview *rbv, *prev;
    /* Remove self from the list of active views (rbViewList). */
    prev = NULL;
    for (rbv = rbViewList; rbv; prev = rbv, rbv = rbv->next)
	if (rbv == this) {
	    /* Remove ebv from the list. */
	    if (prev == NULL)
		rbViewList = rbv->next;
	    else
		prev->next = rbv->next;
	    break;
	}
    DestroyTextPrintData();
}


/*
 * This classprocedure tests if ebv is a valid runbuttonview.
 */
boolean runbuttonview::IsValidRunButtonView(runbuttonview *rbv)
{
    ATKinit;
    for (runbuttonview *r = rbViewList; r; r = r->next) {
	if (r == rbv)
	    return TRUE;
    }
    return FALSE;
}


void runbuttonview::ReceiveInputFocus()
{
    PostMenus(ml);
    ks->next = NULL;
    PostKeyState(ks);
}

/*
 * The user has clicked on the button.
 *
 * A right click means we should take the focus, post our
 *   menus, and print the command string on the message line.
 * A left click means we should execute the command.
 *
 * XXX we should perform these actions on the UP transition.
 */
boolean runbuttonview::Touch(int ind, enum view_MouseAction action)
{
    class runbutton *b = (runbutton *)ButtonData();
    char *m, *cmd;
    char msg[4096];

    switch (action) {
	case view_RightDown:
	    WantInputFocus(this);
	    cmd = b->GetCommandString();
	    if (cmd) {
		sprintf(msg, "This command will be executed: %0.4000s", cmd);
		m = msg;
	    } else
		m = "No command will be executed when this button is pushed.";
	    message::DisplayString(this, 0, m);
	    break;
	case view_LeftDown:
	    b->ExecuteCommand(this);
	    break;
    }
    return FALSE;
}

void runbuttonview::FullUpdate(enum view_UpdateType type, long left, long top, long width, long right)
{
    sbutton *b;
    struct sbutton_prefs *prefs;
    static boolean first_update = TRUE;

    if (first_update) {
	/* Force the box style.  We want people to know this is
	 * a runbutton, and the fewer variations the better.
	 */
	b = ButtonData();
	prefs = b->GetDefaultPrefs();
	if (DisplayClass() & graphic_Monochrome) {
	    sbutton::GetStyle(prefs) = sbutton_BOXEDRECT;
	    sbutton::GetForeground(prefs)=NULL;
	    sbutton::GetBackground(prefs)=NULL;
	} else {
	    sbutton::GetStyle(prefs) = sbutton_MOTIF;
	}
	first_update = FALSE;
    }
    sbuttonv::FullUpdate(type, left, top, width, right);
}

/*
 * Menu functions
 *
 */
void runbuttonview::SetCommandProc(runbuttonview *self, const char *arg)
{
    char buf[4096];
    runbutton *b = (runbutton *)self->ButtonData();
    char *old_cmd;

    old_cmd = b->GetCommandString();
    if (message::AskForString(self,50,"Enter new command for button: ", old_cmd, buf, sizeof(buf)) >= 0) {
	b->SetCommandString(buf);
	b->SetModified();
	b->NotifyObservers(observable_OBJECTCHANGED);
	message::DisplayString(self, 10, "Changed button command.");
    }
}

/*
 * The user wants to change the label.  Prompt for a new one and change it.
 */
void runbuttonview::SetLabelProc(runbuttonview *self, const char *arg)
{
    char buf[256];
    runbutton *b = self->GetRunButton();
    char *oldtext;

    oldtext = b->GetLabelString();
    if (message::AskForString(self,50,"Enter new label for button: ", oldtext, buf, sizeof(buf)) >= 0) {
	if (strlen(buf) > 50) {
	    message::DisplayString(self, 10, "The label must be less than 50 characters.");
	} else {
	    b->SetLabelString(buf);
	    message::DisplayString(self, 10, "Changed button label.");
	}
    }
}


void *runbuttonview::GetPSPrintInterface(char *printtype)
{
    if (strcmp(printtype, "text") == 0)
	return GetTextPrintData();
    // XXX Need to implement "generic" for insets like figure.
    // Probably the best thing to do is draw a button.
    return NULL;
}

struct textview_insetdata *runbuttonview::GetTextPrintData()
{
    runbutton *b = GetRunButton();
    char *lbl = b->GetLabelString();
    char *cmd = b->GetCommandString();
    int lbl_len = strlen(lbl);
    int cmd_len = strlen(cmd);

    DestroyTextPrintData();
    tidata = new textview_insetdata;
    if (!tidata)
	return NULL;
    tidata->type = textview_StitchText;
    tidata->u.StitchText = new textview;
    text *t = new text;
    t->InsertCharacters(0, lbl, lbl_len);
    style *s = new style;
    s->AddDottedBox();
    s->AddNewFontFace(fontdesc_Bold);
    s->RemoveIncludeEnd();
    t->AddStyle(0, lbl_len, s);
    if (cmd[0] != '\0' && strcmp(lbl, cmd) != 0) {
	// The label is not the command.  Insert as a footnote.
	fnote *fn = new fnote;
	fn->InsertCharacters(0, cmd, cmd_len);
	t->AddView(lbl_len, "fnotev", fn);
    }
    tidata->u.StitchText->SetDataObject(t);
    return tidata;
}

// This method cleans up stuff allocated in GetTextPrintData.
void runbuttonview::DestroyTextPrintData()
{
    if (tidata) {
	class dataobject *d = tidata->u.StitchText->GetDataObject();
	if (d) d->Destroy();
	tidata->u.StitchText->Destroy();
	delete tidata;
	tidata = 0;
    }
}
