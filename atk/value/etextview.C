/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
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
static class keymap *ssmap;
ATK_IMPL("etextview.H")
#include <keystate.H>
#include <proctable.H>
#include <keymap.H>
#include <entertext.H>
#include <valueview.H>
#include <etextview.H>
#define Text(A) ((class entertext *)(A->dataobject))

ATKdefineRegistry(etextview, textview, etextview::InitializeClass);
void etextview_CancelCommand(class etextview  *self );
void etextview_ReturnCommand(class etextview  *self );
void etextview_ClearCommand(class etextview  *self );


void etextview_CancelCommand(class etextview  *self )
{
    class entertext *txt = Text(self);
    if((txt)->Changed()){
	if((txt)->buflen())
	    (txt)->SetChars((txt)->GetString() ,(txt)->buflen() - 1);
	else (txt)->Clear();
	(txt)->NotifyObservers(0);
    }
}
void etextview_ReturnCommand(class etextview  *self )
{
    class entertext *txt = Text(self);
    (self)->SetDotPosition(0);
    (self)->SetDotLength(0);
    (txt)->updatebuf();
}
void etextview_ClearCommand(class etextview  *self )
{
    class entertext *txt = Text(self);
    (txt)->Clear();
    (txt)->NotifyObservers(0);
}

void etextview::ReceiveInputFocus()
{
    this->hasInputFocus = TRUE;
    this->keystate->next = NULL;
    this->textview::keystate->next = NULL; /* Unforgivably bogus... */

    (this->keystate)->AddBefore( this->textview::keystate); /* Slightly bogus. */
    (this )->PostKeyState( this->keystate);
    if(Text(this) && this->ClearOnRIF) {
	(Text(this))->Clear();
	(Text(this))->NotifyObservers(0);
    }
    else (this)->WantUpdate(this);
    if(this->valueview)
	(this->valueview)->Highlight();
}
void etextview::LoseInputFocus()
{
    if(this->valueview)
	(this->valueview)->Dehighlight();
    if(Text(this) && this->ResetOnLIF) {
	etextview_CancelCommand(this);
    }
    (this)->textview::LoseInputFocus();
}
etextview::etextview()
{
	ATKinit;

    this->keystate = keystate::Create(this, ssmap);
    this->ClearOnRIF = FALSE;
    this->ResetOnLIF = FALSE;
    this->valueview = NULL;
    THROWONFAILURE( TRUE);
}

boolean etextview::InitializeClass()
{    
    struct ATKregistryEntry  *classInfo = &etextview_ATKregistry_ ;
    struct proctable_Entry *tempProc;

    ssmap = new keymap;
    tempProc=proctable::DefineProc("etextview-return-cmd", (proctable_fptr)etextview_ReturnCommand, classInfo, NULL, "Handle enter key");
    (ssmap)->BindToKey( "\015", tempProc, 0);
    tempProc=proctable::DefineProc("etextview-cancel-cmd", (proctable_fptr)etextview_CancelCommand, classInfo, NULL, "Handle ^G");
    (ssmap)->BindToKey( "\007", tempProc, 0);
    tempProc=proctable::DefineProc("etextview-clear-cmd", (proctable_fptr)etextview_ClearCommand, classInfo, NULL, "Handle ^U");
    (ssmap)->BindToKey( "\025", tempProc, 0);
    return TRUE;
}
