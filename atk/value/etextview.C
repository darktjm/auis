/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
static class keymap *ssmap;
ATK_IMPL("etextview.H")
#include <keystate.H>
#include <proctable.H>
#include <keymap.H>
#include <entertext.H>
#include <valueview.H>
#include <etextview.H>
#define Text(A) ((class entertext *)(A->GetDataObject()))

ATKdefineRegistry(etextview, textview, etextview::InitializeClass);
static void etextview_CancelCommand(class etextview  *self );
static void etextview_ReturnCommand(class etextview  *self );
static void etextview_ClearCommand(class etextview  *self );


static void etextview_CancelCommand(class etextview  *self )
{
    class entertext *txt = Text(self);
    if((txt)->Changed()){
	if((txt)->buflen())
	    (txt)->SetChars((txt)->GetString() ,(txt)->buflen() - 1);
	else (txt)->Clear();
	(txt)->NotifyObservers(0);
    }
}
static void etextview_ReturnCommand(class etextview  *self )
{
    class entertext *txt = Text(self);
    (self)->SetDotPosition(0);
    (self)->SetDotLength(0);
    (txt)->updatebuf();
}
static void etextview_ClearCommand(class etextview  *self )
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
