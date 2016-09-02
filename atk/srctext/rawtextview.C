/* File rawtextv.c created by R S Kemmetmueller
   (c) Copyright IBM Corp.  1992-1995.  All rights reserved.

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

   rawtextview: a dataobject for plain ASCII files, with overstrike mode. */

#include <andrewos.h>

static UNUSED const char ibmid[] = "Copyright IBM CORP 1992-1995. All rights reserved.";

#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <proctable.H>
#include <bind.H>
#include <message.H>

#include "rawtext.H"
#include "rawtextview.H"

static keymap *raw_Map;
static menulist *raw_Menus;

ATKdefineRegistry(rawtextview, textview, rawtextview::InitializeClass);
void toggleOverstrike(rawtextview  *self,long  key);

static struct bind_Description rawtextBindings[]={
    {"rawtextview-toggle-overstrike-mode", "\033\034",0, NULL,0,0, (proctable_fptr) toggleOverstrike, "Turn overstrike mode on or off."}, /*RSK92overstrike*/
    NULL
};

boolean rawtextview::InitializeClass()
{
    raw_Menus = new menulist;
    raw_Map = new keymap;
    bind::BindList(rawtextBindings,raw_Map,raw_Menus,&rawtextview_ATKregistry_);
    return TRUE;
}

rawtextview::rawtextview()
{
    ATKinit;
    this->raw_state = keystate::Create(this, raw_Map);
    this->raw_menus = (raw_Menus)->DuplicateML(this);
    THROWONFAILURE(TRUE);
}

rawtextview::~rawtextview()
{
    ATKinit;
    delete this->raw_state;
    delete this->raw_menus;
}

void rawtextview::ReceiveInputFocus()
{
    this->textview::hasInputFocus = TRUE;
    this->textview::keystate->next= NULL;
    this->raw_state->next= NULL;
    PostKeyState((this->raw_state)->AddBefore(this->textview::keystate)); /* Yech, this makes no sense to me, but works */
    PostMenus(this->raw_menus);
    (this->raw_menus)->SetMask(textview_NoMenus);
    if (GetEditor() == VI)
	if (GetVIMode() == COMMAND)
	    message::DisplayString(this, 0, "Command Mode");
	else
	    message::DisplayString(this, 0, "Input Mode");
    WantUpdate(this);
}

/*RSK92overstrike*/
void toggleOverstrike(rawtextview  *self,long  key)
{
    rawtext *d = (rawtext *)self->view::dataobject;
    if ((d)->IsInOverstrikeMode()) {
	(d)->ChangeOverstrikeMode(FALSE);
	message::DisplayString(self,0,"Normal (insert) mode.");
    } else {
	(d)->ChangeOverstrikeMode(TRUE);
	message::DisplayString(self,0,"Overstrike mode.");
    }
    (d)->NotifyObservers(observable_OBJECTCHANGED);
}
