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
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atkams/messages/lib/RCS/messaux.C,v 1.7 1995/11/07 20:17:10 robr Stab74 $";
#endif


 

#include <sys/param.h>

#include <textview.H>

#include <cui.h>
#include <fdphack.h>
#include <errprntf.h>

#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
/* #include <bind.ih> */
#include <text.H>
#include <textview.H>
#include <environ.H>
#include <environment.H>
#include <message.H>
#include <ams.H>
#include <amsutil.H>
#include <text822.H>
#include <t822view.H>
#include <captions.H>
#include <folders.H>
#include <sendmessage.H>
#define AUXMODULE 1
#include <messages.H>
#undef AUXMODULE
#include <options.H>



#ifndef NORCSID
#endif
void sm_SetMessagesOptions(class messages  *self);
class captions *GetCaptions(class messages  *self);
class t822view *GetBodies(class messages  *self);
void MessagesFocusFolders(class messages  *self);
void MessagesSendmessageCommand(class messages  *self, char  *cmds);
void BSM_ShowHelp(class messages  *self);
boolean ClearSM(class captions  *self);
void BSM_CheckNewPlease(class messages  *self);
void BSM_ReadMailPlease(class messages  *self);
void BSM_ShowNewPlease(class messages  *self);
void BSM_ShowPersonalPlease(class messages  *self);
void BSM_ShowAllPlease(class messages  *self);
void BSM_ShowSubscribedPlease(class messages  *self);
void messages_DuplicateWindow(class messages  *self);
class captions *GetCaptionsNoCreate(class messages  *self);
class folders *GetFolders(class messages  *self);
void MessagesFoldersCommand(class messages  *self, char  *cmds);
void FSearchFPlease(class messages  *self);
void FSearchRPlease(class messages  *self);

extern void CheckMenuMasks(class messages  *self);

void sm_SetMessagesOptions(class messages  *self)
{
    (GetCaptions(self))->ResetVisibleCaption();
    options::SetMessagesOptions(GetBodies(self));
}

extern  proctable_fptr messtextv_ForwardSearchCmd, messtextv_ReverseSearchCmd;

void messages::PostKeyState(class keystate  *ks)
{
    class keystate *tmp;
    this->mypermkeys->next=NULL;
    this->mykeys->next=NULL;
    this->textview::keystate->next=NULL;
    if (amsutil::GetOptBit(EXP_KEYSTROKES) 
		&& (this->WhatIAm != WHATIAM_BODIES
		    || ((class text *) (GetBodies(this))->GetDataObject()
		)->GetReadOnly(
			))) {
	tmp = (this->mypermkeys)->AddBefore( ks);
	tmp = (this->mykeys)->AddBefore( tmp);
	(this)->textview::PostKeyState( tmp);
    } else {
	tmp = (this->mypermkeys)->AddBefore( ks);
	(this)->textview::PostKeyState( tmp);
    }
}

void messages::PostMenus(class menulist  *ml)
{
    CheckMenuMasks(this);
    (this->mymenulist)->ClearChain();

    (this->mymenulist)->ChainAfterML( this->mypermmenulist, (long)this->mypermmenulist);
    if (ml) (this->mymenulist)->ChainAfterML( ml, (long)ml);
    if (this->fileintomenulist) {
	(this->mymenulist)->ChainAfterML( this->fileintomenulist, (long)this->fileintomenulist);
    }
    (this)->textview::PostMenus( this->mymenulist);
}

class captions *GetCaptions(class messages  *self)
{
    class captions *c = NULL;
    switch(self->WhatIAm) {
	case WHATIAM_FOLDERS:
	    c = ((class folders *) self)->GetCaptions();
	    break;
	case WHATIAM_CAPTIONS:
	    c = (class captions *) self;
	    break;
	case WHATIAM_BODIES:
	    c = ((class t822view *) self)->GetCaptions();
	    break;
    }
    if (!c) {
	(ams::GetAMS())->ReportError( "Out of memory; a core dump is imminent.", ERR_FATAL, FALSE, 0);
    }
    return(c);
}

class t822view *GetBodies(class messages  *self)
{
    class t822view *tv = NULL;
    switch(self->WhatIAm) {
	case WHATIAM_FOLDERS:
	    tv = (((class folders *) self)->GetCaptions())->GetBodView();
	    break;
	case WHATIAM_CAPTIONS:
	    tv = ((class captions *) self)->GetBodView();
	    break;
	case WHATIAM_BODIES:
	    tv = (class t822view *) self;
	    break;
    }
    if (!tv) {
	(ams::GetAMS())->ReportError( "Out of memory; a core dump is imminent.", ERR_FATAL, FALSE, 0);
    }
    return(tv);
}

void MessagesFocusFolders(class messages  *self)
{
    class folders *f = GetFolders(self);
    (f)->WantInputFocus( f);
}

void MessagesSendmessageCommand(class messages  *self, char  *cmds)
{
    class sendmessage *sm = (GetFolders(self))->ExposeSend();
    if (sm) {
	(ams::GetAMS())->GenericCompoundAction( sm, "sendmessage", cmds);
    }
}

void BSM_ShowHelp(class messages  *self)
{
    if (GetCaptions(self)->FullName) {
	(GetFolders(self))->ExplainDir( GetCaptions(self)->FullName, GetCaptions(self)->ShortName);
    } else {
	(GetBodies(self))->ShowHelp( NULL);
    }
}

boolean ClearSM(class captions  *self)
{
    class sendmessage *sm = ((self)->GetFolders())->ExposeSend();
    if (!sm) return(TRUE);
    (sm)->Reset();
    if ((sm)->HasChanged()) return(TRUE);
    return(FALSE);
}

void BSM_CheckNewPlease(class messages  *self)
{
    (GetFolders(self))->UpdateMsgs( 0, NULL, TRUE);
}

void BSM_ReadMailPlease(class messages  *self)
{
    if(environ::GetProfileSwitch("ReadMailFolders", FALSE))
	(GetFolders(self))->UpdateMsgs( TRUE, NULL, TRUE);
    else
	(GetFolders(self))->ReadMail( TRUE);
}

void BSM_ShowNewPlease(class messages  *self)
{
    (GetFolders(self))->Reconfigure( LIST_NEWONLY);
}

void BSM_ShowPersonalPlease(class messages  *self)
{
    (GetFolders(self))->Reconfigure( LIST_MAIL_FOLDERS);
}

void BSM_ShowAllPlease(class messages  *self)
{
    (GetFolders(self))->Reconfigure( LIST_ALL_FOLDERS);
}

void BSM_ShowSubscribedPlease(class messages  *self)
{
    (GetFolders(self))->Reconfigure( LIST_SUBSCRIBED);
}

void messages_DuplicateWindow(class messages  *self)
{
    if (self->WhatIAm == WHATIAM_FOLDERS) {
	class folders *f = new folders;

	ams::InstallInNewWindow((f)->GetApplicationLayer(), "messages-folders", "Message Folders", environ::GetProfileInt("folders.width", 600), environ::GetProfileInt("folders.height", 120), f);
    } else if (self->WhatIAm == WHATIAM_CAPTIONS) {
	class captions *f = new captions;

	ams::InstallInNewWindow((f)->GetApplicationLayer(), "messages-captions", "Message Captions", environ::GetProfileInt("captions.width", 600), environ::GetProfileInt("captions.height", 120), f);
    } else if (self->WhatIAm == WHATIAM_BODIES) {
	class t822view *f = new t822view;
	class text *t = new text;

	(f)->SetDataObject( t);
	ams::InstallInNewWindow((f)->GetApplicationLayer(), "messages-bodies", "Message Bodies", environ::GetProfileInt("bodies.width", 600), environ::GetProfileInt("bodies.height", 120), f);
    }
	
}

class captions *GetCaptionsNoCreate(class messages  *self)
{
    class captions *c = NULL;
    switch(self->WhatIAm) {
	case WHATIAM_FOLDERS:
	    c = ((class folders *) self)->mycaps;
	    break;
	case WHATIAM_CAPTIONS:
	    c = (class captions *) self;
	    break;
	case WHATIAM_BODIES:
	    c = ((class t822view *) self)->mycaps;
	    break;
    }
    return(c);
}

class folders *GetFolders(class messages  *self)
{
    class folders *f = NULL;
    switch(self->WhatIAm) {
	case WHATIAM_FOLDERS:
	    f = (class folders *) self;
	    break;
	case WHATIAM_CAPTIONS:
	    f = ((class captions *) self)->GetFolders();
	    break;
	case WHATIAM_BODIES:
	    f = (((class t822view *) self)->GetCaptions())->GetFolders();
	    break;
    }
    if (!f) {
	(ams::GetAMS())->ReportError( "Out of memory; a core dump is imminent.", ERR_FATAL, FALSE, 0);
    }
    return(f);
}

void MessagesFoldersCommand(class messages  *self, char  *cmds)
{
    (ams::GetAMS())->GenericCompoundAction( GetFolders(self), "folders", cmds);
}

void FSearchFPlease(class messages  *self)
{
    messtextv_ForwardSearchCmd(GetFolders(self), 0);
    (self)->WantInputFocus( self);
}

void FSearchRPlease(class messages  *self)
{
    messtextv_ReverseSearchCmd(GetFolders(self), 0);
    (self)->WantInputFocus( self);
}
