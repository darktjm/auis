/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
static class keymap *ssmap;
ATK_IMPL("metextview.H")
#include <keystate.H>
#include <proctable.H>
#include <keymap.H>
#include <mentertext.H>
#include <valueview.H>
#include <message.H>
#include <menulist.H>
#include <metextview.H>
#define Text(A) ((class mentertext *)(A->dataobject))

ATKdefineRegistry(metextview, textview, metextview::InitializeClass);
static void metextview_CancelCommand(class metextview  *self );
static void metextview_ReturnCommand(class metextview  *self );


static void metextview_CancelCommand(class metextview  *self )
{
    class mentertext *txt = Text(self);
    if((txt)->Changed()){
	(txt)->ClearLine((self)->GetDotPosition());
	(txt)->NotifyObservers(0);
    }
}
static void metextview_ReturnCommand(class metextview  *self )
{
    long np;
    char resp[64];
    class mentertext *txt = Text(self);
    long pos = (self)->GetDotPosition();
    long len = (txt)->GetLength();
    np = (txt)->Index(pos,'\n',len - pos);
    if(np != EOF){
	pos =  (txt)->Index(np + 1,'\n',len - (np + 1));
	if(pos == EOF) {
	    if(len > np) np = len ;
	}
	else np = pos;
	(self)->SetDotPosition(np);
	(self)->FrameDot(np);
    }
    else{
	if(message::AskForString(self,0,"Confirm[y] ",0,resp,63) == -1)
	    return;
	if(*resp != 'n' &&  *resp != 'N') {
	    (txt)->updatebuf();
	}
    }
}
#if 0  /* use commented out below */
static void metextview_ClearCommand(class metextview  *self )
{
    class mentertext *txt = Text(self);
    (txt)->Clear();
    (txt)->NotifyObservers(0);
}
#endif
void metextview::ReceiveInputFocus()
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
    (this->menus)->SetMask( textview_NoMenus);
/*    metextview_PostMenus(self,self->header.textview.menus); */
}
void metextview::LoseInputFocus()
{
    if(this->valueview)
	(this->valueview)->Dehighlight();
    if(Text(this) && this->ResetOnLIF) {
	metextview_CancelCommand(this);
    }
    (this)->textview::LoseInputFocus();
}
metextview::metextview()
{
	ATKinit;

    this->keystate = keystate::Create(this, ssmap);
    this->ClearOnRIF = FALSE;
    this->ResetOnLIF = FALSE;
    this->valueview = NULL;
    THROWONFAILURE( TRUE);
}

boolean metextview::InitializeClass()
{    
    struct ATKregistryEntry  *classInfo = &metextview_ATKregistry_ ;
    struct proctable_Entry *tempProc;

    ssmap = new keymap;
    tempProc=proctable::DefineProc("metextview-return-cmd", (proctable_fptr)metextview_ReturnCommand, classInfo, NULL, "Handle menter key");
    (ssmap)->BindToKey( "\015", tempProc, 0);
/*
    tempProc=proctable_DefineProc("metextview-cancel-cmd", metextview_CancelCommand, classInfo, NULL, "Handle ^G");
    keymap_BindToKey(ssmap, "\007", tempProc, 0);
    tempProc=proctable_DefineProc("metextview-clear-cmd", metextview_ClearCommand, classInfo, NULL, "Handle ^U");
    keymap_BindToKey(ssmap, "\025", tempProc, 0);
*/
    return TRUE;
}
void metextview::ObservedChanged(class observable  *changed,long  value)
{
    (this)->textview::ObservedChanged(changed,value);
    if(value != observable_OBJECTDESTROYED){
	if((this)->GetDotPosition() == 0 && 
	   (value = (Text(this))->GetLength()) > 0 &&
	   (value = (Text(this))->Index(0,'\n',value)) != EOF)
	    (this)->SetDotPosition(value);
    }
}
