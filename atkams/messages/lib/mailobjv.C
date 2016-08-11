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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atkams/messages/lib/RCS/mailobjv.C,v 1.5 1995/07/07 19:26:34 rr2b Stab74 $";
#endif


 


#include <andrewos.h>
#include <sbutton.H>
#include <mailobjv.H>
#include <mailobj.H>
#include <bind.H>
#include <menulist.H>
#include <keymap.H>
#include <keystate.H>
#include <ams.H>
#include <completion.H>
#include <message.H>

static class keymap *mailobjv_standardkeymap;
static class menulist *mailobjv_standardmenulist;


ATKdefineRegistry(mailobjv, sbuttonv, mailobjv::InitializeClass);
#ifndef NORCSID
#endif
void ChangeContents(class mailobjv  *self);


void ChangeContents(class mailobjv  *self)
{
    char ctype[300], fname[1000], Label[200];
    FILE *fp;
    class mailobj *mo = (class mailobj*) (self)->GetDataObject();

    if (mo->RawData
	&& !(ams::GetAMS())->GetBooleanFromUser( "Do you want to overwrite the data that is already here", FALSE)) {
	message::DisplayString(self, 10, "Use the LEFT mouse button to activate this object.");
	return;
    }
    if (completion::GetFilename(self, "Name of raw data file: ", "", fname, sizeof(fname), FALSE, TRUE) == -1 ) {
	return;
    }
    if (message::AskForString(self, 50, "Content-type: ", NULL, ctype, sizeof(ctype)) < 0) {
	return;
    }
    if (message::AskForString(self, 50, "Content-Description: ", NULL, Label, sizeof(Label)) < 0) {
	return;
    }
    fp = fopen(fname, "r");
    if (!fp) return;
    (mo)->ReadAlienMail( ctype, NULL, fp, 0);
    (mo)->SetLabel( 0, Label);
    fclose(fp);
}

static struct bind_Description mailobjv_standardbindings [] = {
    /* procname, keysequenece, key rock, menu string, menu rock, menu mask, proc, docstring, dynamic autoload */
    {"mailobjv-change-contents", "c", 0, "Mail Object,Change Contents", 0, 0, (proctable_fptr)ChangeContents, "Change the contents of a mailobj"},
    {NULL, NULL, 0, NULL, 0, 0, NULL, NULL, NULL},
};

boolean mailobjv::InitializeClass() 
{
    mailobjv_standardmenulist = new menulist;
    mailobjv_standardkeymap = new keymap;
    bind::BindList(mailobjv_standardbindings, mailobjv_standardkeymap, mailobjv_standardmenulist, &mailobjv_ATKregistry_ );
    return(TRUE);
}

void mailobjv::PostKeyState(class keystate  *ks)
{
    this->mykeys->next=NULL;
    (this->mykeys)->AddBefore( ks);
    (this)->sbuttonv::PostKeyState( this->mykeys);
}

void mailobjv::PostMenus(class menulist  *ml)
{
    (this->mymenulist)->ClearChain();
    if (ml) (this->mymenulist)->ChainAfterML( ml, (long)ml);
    (this)->sbuttonv::PostMenus( this->mymenulist);
}

mailobjv::mailobjv()
{
	ATKinit;

    this->mykeys = keystate::Create(this, mailobjv_standardkeymap);
    this->mymenulist = (mailobjv_standardmenulist)->DuplicateML( this);
    THROWONFAILURE((TRUE));
}

mailobjv::~mailobjv()
{
	ATKinit;

    delete this->mymenulist;
    delete this->mykeys;
}

boolean mailobjv::Touch(int  ind, enum view_MouseAction  action)
{
    class mailobj *mo;

    class sbutton *b=(this)->ButtonData();

    (b)->GetHitFuncRock()=action;
    
    mo = (class mailobj*) (this)->GetDataObject();
    if (action == view_LeftUp  && mo->RawData) {
	(mo)->RunMetamail();
    } else if(action == view_LeftUp || action == view_RightUp) {
	if (action == view_RightUp || (ams::GetAMS())->GetBooleanFromUser( "There is no data here yet.  Do you want to read in data from a file", FALSE)) {
	    ChangeContents(this);
	}
    }
    return (this)->sbuttonv::Touch( ind, action);
}
