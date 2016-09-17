/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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
#define AUXMODULE 1
#include <textview.H>
#include <txtvcmds.h>

#include <text.H>
#include <im.H>
#include <message.H>
#include <viewref.H>
#include <environment.H>
#include <style.H>
#include <fnote.H>
#include <buffer.H>
#include <txtproc.h>

void textview_ToggleViModeCmd(class textview  *self)
{
    long lcstate = ((self)->GetIM())->GetLastCmd();

    (self)->ToggleVIMode();
    ((self)->GetIM())->SetLastCmd( lcstate);	/* be transparent */
}

void textview_ViCommandCmd(class textview	 *self, long		 key)
{
    char tc;

    tc = (self->imPtr)->GetCharacter();
    switch (tc )
    {
	case 'r':
	    textview_InsertFile(self);
	    break;
	case 'q':
	    message::DisplayString(self, 0, "Please use Quit menu item.");
	    break;
	case 'w':
	    message::DisplayString(self, 0, "Please use Save or File/Save-As menu items.");
	    break;
	case 'e':
	    message::DisplayString(self, 0, "Please use Switch File menu item.");
	    break;
    }
}

void textview_ToggleEditorCmd(class textview  *self)
{
    (self)->ToggleEditor();
}

void textview_GrabReference(class textview  *self,long  key)
{
    long pos,len;
    class viewref *vr;
    class text *d = Text(self);
    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();
    if (len == 0) len = (d)->GetLength() - pos;
    if ((vr = (d)->FindViewreference( pos, len)) == NULL)
        message::DisplayString(self, 0, "No References Found");
    else
        d->currentViewreference = vr;
}

void textview_PlaceReference(class textview  *self,long  key)
{
    long pos;
    char p[250];
    class text *d = Text(self);
    boolean promptforname = ((self)->GetIM())->ArgProvided();

    ((self)->GetIM())->ClearArg(); 

    if (ConfirmReadOnly(self))
        return;
    if((Text(self))->GetObjectInsertionFlag() == FALSE){
	message::DisplayString(self, 0, "Object Insertion Not Allowed!");
	return;
    }

    *p = '\0';
    if(d->currentViewreference == NULL) {
        message::DisplayString(self, 0, "No References Found");
        return;
    }
    pos = (self)->GetDotPosition() +  (self)->GetDotLength();
    if(promptforname && message::AskForString (self, 0, "View to place here:", d->currentViewreference->viewType, p, 200) < 0) return;
    if (p[0] == '\0')  strcpy(p,d->currentViewreference->viewType);
    if(textview_objecttest(self,p,"view") == FALSE) return;
    (d)->AddView(pos,p, d->currentViewreference->dataObject);
    (d)->NotifyObservers(observable_OBJECTCHANGED);
}

void textview_CheckSpelling(class textview  *self)
{
    message::DisplayString(self, 0,
       "Sorry; \"Check Spelling\" is not implemented.");
}

void textview_ToggleReadOnly(class textview  *self)
{
    boolean argp = ((self)->GetIM())->ArgProvided(), arg;
    class text *myText = Text(self);
    class buffer *buf;

    if (argp)
        arg = ((self)->GetIM())->Argument();

    buf = buffer::FindBufferByData(myText);

    if ((argp && arg) || (!argp && (myText)->GetReadOnly())) {
	/* In readonly mode. */
	if (buf)
	    (buf)->SetReadOnly(FALSE);
	else
	    (myText)->SetReadOnly(FALSE);
    }
    else {
	if (buf)
	    (buf)->SetReadOnly(TRUE);
	else
	    (myText)->SetReadOnly( TRUE);
    }

    if ((myText)->GetReadOnly())
	message::DisplayString(self, 0, "Text is now read only.");
    else
	message::DisplayString(self, 0, "Text is now writable.");
    (myText)->NotifyObservers( observable_OBJECTCHANGED); /* Handles updating of menus on read only transition. */
}

void textview_InsertPageBreak (class textview  *self)
    {
    long pos;
    class text *d;

    d = Text(self);
    if((d)->GetObjectInsertionFlag() == FALSE){
	message::DisplayString(self, 0, "Object Insertion Not Allowed!");
	return;
    }
    pos = (self)->GetDotPosition();
    (self)->PrepareInsertion( TRUE);
    if((d)->GetChar(pos) != '\n'){
	(d)->InsertCharacters(pos,"\n",1);
    }
    if((d)->GetChar(pos - 1) != '\n'){
	(d)->InsertCharacters(pos,"\n",1);
	pos++;
    }

    /* self->currentViewreference = */
    (d)->InsertObject( pos,"bp","bpv"); 
    (self)->FinishInsertion();
    (d)->NotifyObservers(observable_OBJECTCHANGED);
    if (((self)->GetIM())->GetLastCmd() == lcInsertEnvironment) {
	((self)->GetIM())->SetLastCmd( lcInsertEnvironment);
    }
}

void textview_NextPage (class textview  *self)
    {
    long pos,len;
    class text *d;
    class viewref *vr;
    d = Text(self);
    len = (d)->GetLength();
    pos = (self)->GetDotPosition();
    while ( pos <= len && (pos = (d)->Index(pos,TEXT_VIEWREFCHAR,len - pos)) != EOF){
	if((vr = (d)->FindViewreference(pos,1)) != NULL && 
	   *(vr->viewType) == 'b' &&
	   strcmp(vr->viewType,"bpv") == 0){
	    (self)->SetDotPosition(pos + 2);
	    (self)->SetTopPosition(pos + 2);
	    break;
	}
	pos++;
    }
}
static long text_rindex(class text  *txt,long  pos,char  c)
{
    for(;pos > 0;pos--){
	if((txt)->GetChar(pos) == c) return pos;
    }
    return EOF;
}
void textview_LastPage (class textview  *self)
    {
    long pos,cnt;
    class text *d;
    class viewref *vr;
    d = Text(self);
    pos = (self)->GetDotPosition();
    for ( cnt = 0;pos > 0 && (pos = text_rindex(d,pos,TEXT_VIEWREFCHAR)) != EOF;pos--){
	if((vr = (d)->FindViewreference(pos,1)) != NULL && 
	   *(vr->viewType) == 'b' &&
	   strcmp(vr->viewType,"bpv") == 0){
	    if(cnt++ == 0) continue;
	    (self)->SetDotPosition(pos + 2);
	    (self)->SetTopPosition(pos + 2);
	    return;
	}
    }
    (self)->SetDotPosition(0);
    (self)->SetTopPosition(0);
}

void textview_InsertFootnote(class textview  *self)
    {
    long pos;
    class fnote *fn;

    if((Text(self))->GetObjectInsertionFlag() == FALSE){
	message::DisplayString(self, 0, "Object Insertion Not Allowed!");
	return;
    }
    pos = (self)->GetDotPosition();
    fn = new fnote;
/*    self->currentViewreference = text_InsertObject(Text(self), pos,"fnote","fnotev"); */
    (Text(self))->AddView( pos,"fnotev",fn);
    (fn)->addenv(Text(self),pos);
    
    (Text(self))->NotifyObservers(observable_OBJECTCHANGED);
    (self)->SetDotPosition(pos + 1);
}
void textview_OpenFootnotes(class textview  *self)
    {
    fnote::OpenAll(Text(self));
}
void textview_CloseFootnotes(class textview  *self)
    {
    fnote::CloseAll(Text(self));
}
void textview_WriteFootnotes(class textview  *self)
    {
    FILE *f;
    class text *tmpt;
    f = fopen("/tmp/notes","w");
    tmpt = new text;
    fnote::CopyAll(Text(self),tmpt,1,TRUE);
    (tmpt)->Write(f,0,0);
    fclose(f);
    (tmpt)->Destroy();
}

void textview_ToggleLineDisplay(textview *self, char *arg)
{
    if ((int)(long) arg < 256) {
	/* No argument...toggle. */
	self->LineNumberDisplay(!self->show_para_display);
    } else {
	/* Argument...check for on or off. */
	if (strcmp(arg, "on") == 0)
	    self->LineNumberDisplay(TRUE);
	else if (strcmp(arg, "off") == 0)
	    self->LineNumberDisplay(FALSE);
	else
	    message::DisplayString(self, 75, "textview-toggle-line-display:  expecting \"on\" or \"off\" as an argument.");
    }
    self->WantUpdate(self);
}
