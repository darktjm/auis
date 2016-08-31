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
#include <stdio.h>

#include <textview.H>

#include <cui.h>
#include <fdphack.h>
#include <hdrparse.h>
#include <mailconf.h>


#include <lpair.H>
#include <message.H>
#include <environ.H>
#include <captions.H>
#include <folders.H>
#include <ams.H>
#include <amsutil.H>
#include <text.H>
#include <t822view.H>
#include <messwind.H>
#include <sys/param.h>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <bind.H>
#include <im.H>
#include <view.H>


ATKdefineRegistry(messwind, lpair, messwind::InitializeClass);
static void DuplicateWindow(class messwind  *self);
void CheckVerticalHorizontal(class messwind  *mess);
void messwindCompound(struct messwind  *mess, char  *cmds);
void messwindFoldersCommand(class messwind  *mess, char  *cmds);
static void DeleteWindow(class messwind  *self);
static void messwind_wrap_ToggleSideBySide(class messwind *self);

static struct bind_Description messwind_standardbindings [] = {
    /* procname, keysequenece, key rock, menu string, menu rock, menu mask, proc, docstring, dynamic autoload */
    {"messwind-delete-window", "\030\004", 0, "Delete Window~89", 0, 0, (proctable_fptr)DeleteWindow, "Delete messages window"},
    {"messwind-duplicate", "\0302", 0, NULL, 0, 0, (proctable_fptr)DuplicateWindow, "Open another messages window"},
    {"messwind-compound-operation", NULL, 0, NULL, 0, 0, (proctable_fptr)messwindCompound, "Execute a compound messwind operation"},
    {"messwind-folders-compound", NULL, 0, NULL, 0, 0, (proctable_fptr)messwindFoldersCommand, "Execute a compound 'captions' operation on the folders"},
    {"messwind-toggle-layout", NULL, 0, NULL, 0, 0, (proctable_fptr)messwind_wrap_ToggleSideBySide, "Toggle the vertical/horizontal positioning of folders & captions"},
    {NULL, NULL, 0, NULL, 0, 0, NULL, NULL, NULL},
};


static void DuplicateWindow(class messwind  *self)
{
    class messwind *m = (class messwind *) new messwind;

    ams::InstallInNewWindow(m, "messages", "Messages", environ::GetProfileInt("messages.width", -1), environ::GetProfileInt("messages.height", -1), m->foldersp);
}

static class keymap *messwind_standardkeymap;
static class menulist *messwind_standardmenulist;

messwind::messwind()
{
	ATKinit;

    int hbsplit;
    class text *t;

    this->mykeys = keystate::Create(this, messwind_standardkeymap);
    this->mymenulist = (messwind_standardmenulist)->DuplicateML( this);

    t = new text;
    (t)->SetReadOnly( TRUE);
    this->bview = new t822view;
    (this->bview)->SetDataObject( t);
    this->foldersp = new folders;
    this->captionsp = new captions;
    (this->foldersp)->SetCaptions( this->captionsp);
    (this->captionsp)->SetFolders( this->foldersp);
    (this->captionsp)->SetBodies( this->bview);
    (this->bview)->SetCaptions( this->captionsp);
    this->SideBySide = amsutil::GetOptBit(EXP_SIDEBYSIDE) ? 1 : 0;

    hbsplit = environ::GetProfileInt("messages.headbodysplit", 50);
    if (hbsplit < 0) hbsplit *= -1;
    if (hbsplit > 100) hbsplit = 50;
    this->capbodylp =new lpair;
    (this->capbodylp)->VSplit( (this->captionsp)->GetApplicationLayer(), (this->bview)->GetApplicationLayer(), hbsplit, 1);
    this->foldapplayer = (this->foldersp)->GetApplicationLayer();
    CheckVerticalHorizontal(this);
    THROWONFAILURE((TRUE));
}

void CheckVerticalHorizontal(class messwind  *mess)
{
    int foldpix = environ::GetProfileInt("messages.folderpixels", mess->SideBySide ? 200 : 80);

    if (mess->SideBySide) {
	(mess)->HTFixed( mess->foldapplayer, mess->capbodylp, foldpix, 1);
    } else {
	(mess)->VTFixed( mess->foldapplayer, mess->capbodylp, foldpix, 1);
    }
    (mess->foldersp)->SetVeryNarrow( mess->SideBySide);
}

static void messwind_wrap_ToggleSideBySide(class messwind *self)
{
    self->ToggleSideBySide();
}

void messwind::ToggleSideBySide()
{
    class im *myim = (this)->GetIM();
    class view *v = myim ? (myim)->GetInputFocus() : NULL;

    this->SideBySide = !this->SideBySide;
    CheckVerticalHorizontal(this);
    if (v) (v)->WantInputFocus( v);
    if (myim) (myim)->RedrawWindow();
}

void messwindCompound(struct messwind  *mess, char  *cmds)
{
    (ams::GetAMS())->GenericCompoundAction( mess, "messwind", cmds);
}

void messwindFoldersCommand(class messwind  *mess, char  *cmds)
{
    (ams::GetAMS())->GenericCompoundAction( mess->foldersp, "folders", cmds);
}

static const char lastWindowWarning[] = "This is the last window.";
static const char * const lastWindowChoices[] = {
    "Continue Running",
    "Quit Messages",
    NULL
};

#define lastWindow_CANCEL 0
#define lastWindow_QUIT   1

static void DeleteWindow(class messwind  *self)
{
    if (ams::CountAMSViews() > 3) {
	ams::CommitState(FALSE, FALSE, FALSE, FALSE);
	if ((self)->GetIM()) ((self)->GetIM())->Destroy();
	(self)->Destroy();
    }
    else {
	long answer;
	if (message::MultipleChoiceQuestion(NULL, 0, lastWindowWarning, lastWindow_CANCEL, &answer, lastWindowChoices, NULL) == -1)
	    return;
	switch(answer){
	    case lastWindow_CANCEL:
		return;

	    case lastWindow_QUIT :
		ams::CommitState(TRUE, FALSE, TRUE, TRUE);
	}
    }

}

boolean messwind::InitializeClass() 
{
    messwind_standardmenulist = new menulist;
    messwind_standardkeymap = new keymap;
    bind::BindList(messwind_standardbindings, messwind_standardkeymap, messwind_standardmenulist, &messwind_ATKregistry_ );
    return(TRUE);
}

void messwind::PostKeyState(class keystate  *ks)
{
    if (amsutil::GetOptBit(EXP_KEYSTROKES)
		&& ((class text *)this->bview->dataobject)->GetReadOnly()) {
        (this->mykeys)->AddBefore( ks);
	(this)->lpair::PostKeyState( this->mykeys);
    } else {
	(this)->lpair::PostKeyState( ks);
    }
}

void messwind::PostMenus(class menulist  *ml)
{
    (this->mymenulist)->ClearChain();
    if (ml) (this->mymenulist)->ChainAfterML( ml, (long)ml);
    (this)->lpair::PostMenus( this->mymenulist);
}

messwind::~messwind()
{
	ATKinit;

    (this)->SetNth( 0, NULL);
    (this)->SetNth( 1, NULL);
    (this->capbodylp)->Destroy();
    (this->captionsp)->Destroy();
    (this->foldersp)->Destroy();
    (this->bview)->Destroy();
    delete this->mymenulist;
    delete this->mykeys;
}
