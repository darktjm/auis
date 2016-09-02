/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

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
ATK_IMPL("prefsv.H")


#include "prefsv.H"

#include "prefs.H"

#include <message.H>
#include <proctable.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <bind.H>
#include <observable.H>

#define DATA(self) ((class prefs *)(self)->GetDataObject())

static class menulist *prefsvMenus=NULL;
static class keymap *prefsvKeymap=NULL;


ATKdefineRegistry(prefsv, textview, prefsv::InitializeClass);
static void sort(class prefsv  *pv, char  *rock);


static void sort(class prefsv  *pv, char  *rock)
{
    char buf[256];
    if((long)rock<=255) return;
    printf("sorting by %s\n",rock);
    strcpy(buf, "Sorted by ");
    switch(*rock) {
	case 'N':
	    (DATA(pv))->Sort( prefs_Name, TRUE);
	    strcat(buf, "name.");
	    break;
	case 'A':
	    (DATA(pv))->Sort( prefs_App, TRUE);
	    strcat(buf, "application.");
	    break;
	case 'G':
	    (DATA(pv))->Sort( prefs_Group, TRUE);
	    strcat(buf, "group.");
	    break;
	case 'O':
	    (DATA(pv))->Sort( prefs_Order, TRUE);
	    strcat(buf, "order.");
	    break;
	default:
	    if(strlen(rock)<200) {
		strcpy(buf, rock);
		strcat(buf, " is an invalid argument to sort.");
	    }
	    break;
    }
    (DATA(pv))->NotifyObservers( observable_OBJECTCHANGED);
    message::DisplayString(pv, 0, buf);
}

static struct bind_Description prefsvBindings[]={
    {
	"prefsv-sort",
	0,
	0,
	"Sort~20,By name",
	(long)"Name",
	0,
	(proctable_fptr)sort,
	"Sorts the preferences. (Valid arguments are: Name, Group, App, Order)",
	"prefsv"
    },{
	"prefsv-sort",
	0,
	0,
	"Sort~20,By order",
	(long)"Order",
	0,
	(proctable_fptr)sort,
	"Sorts the preferences. (Valid arguments are: Name, Group, App, Order)",
	"prefsv"
    },
    {
	"prefsv-sort",
	0,
	0,
	"Sort,By group",
	(long)"Group",
	0,
	(proctable_fptr)sort,
	"Sorts the preferences. (Valid arguments are: Name, Group, App, Order)",
	"prefsv"
    },
    {
	"prefsv-sort",
	0,
	0,
	"Sort,By application",
	(long)"App",
	0,
	(proctable_fptr)sort,
	"Sorts the preferences. (Valid arguments are: Name, Group, App, Order)",
	"prefsv"
    },
    {
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0
    }
};
    
    
boolean prefsv::InitializeClass()
{
    prefsvMenus=new menulist;
    prefsvKeymap=new keymap;

    if(prefsvMenus == NULL || prefsvKeymap==NULL) {
	if(prefsvMenus != NULL) delete prefsvMenus;
	if(prefsvKeymap != NULL) delete prefsvKeymap;
	return FALSE;
    }
    
    bind::BindList(prefsvBindings, prefsvKeymap, prefsvMenus, &prefsv_ATKregistry_ );
    return TRUE;
}

prefsv::prefsv()
{
	ATKinit;

    this->ks=keystate::Create(this, prefsvKeymap);
    if(this->ks==NULL) THROWONFAILURE( FALSE);

    this->menulistp=(prefsvMenus)->DuplicateML( this);

    if(this->menulistp==NULL) {
	delete this->ks;
	THROWONFAILURE( FALSE);
    }
    THROWONFAILURE( TRUE);
}

prefsv::~prefsv()
{
	ATKinit;

    if(this->ks) delete this->ks;
    if(this->menulistp) delete this->menulistp;
}


void prefsv::PostMenus(class menulist  *ml)
{
    if(this->menulistp) {
	if(ml) (this->menulistp)->ChainAfterML( ml, (long)ml);
	(this)->textview::PostMenus( this->menulistp);
    } else (this)->textview::PostMenus( ml);
}

void prefsv::PostKeyState(class keystate  *ks)
{
    class keystate *lks=this->ks;
    if(lks) {
	(lks)->AddBefore( ks);
	(this)->textview::PostKeyState( lks);
    } else (this)->textview::PostKeyState( ks);
}


