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
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <bind.H>
#include <view.H>
#include <im.H>
#include <lpair.H>
#include <org.H>
#include <ams.H>
#include <foldertreev.H>

#define  menu_default		  (1<<0)
#define  menu_buttons		  (1<<3)
#define  menu_exploded		  (1<<4)
#define  menu_imploded		  (1<<5)
#define  menu_description_hidden  (1<<6)
#define  menu_description_exposed (1<<7)
#define  menu_palette_hidden	  (1<<8)
#define  menu_palette_exposed	  (1<<9)
#define  menu_horizontal	  (1<<10)
#define  menu_vertical		  (1<<11)
#define  menu_folded		  (1<<12)
#define  menu_unfolded		  (1<<13)
#define  menu_debug		  (1<<14)

static class keymap *foldertreev_standardkeymap = NULL;
static class menulist *foldertreev_standardmenulist = NULL;



ATKdefineRegistry(foldertreev, orgv, foldertreev::InitializeClass);
static void QuitFolderTree( class foldertreev	 *self );
static void QuitMessagesProc( class foldertreev	 *self );


static struct bind_Description foldertreev_standardbindings [] = {
    {"foldertreev-delete-window", "\030\004", 0, "Delete Window~89", 0, 0, (proctable_fptr)QuitFolderTree, "exit folder tree"},
    {"foldertreev-quit", "\030\003", 0, "Quit~99", 0, 0, (proctable_fptr)QuitMessagesProc, "exit messages process"},
    {NULL,NULL,0,NULL,0,0,NULL,NULL,NULL}
};


boolean foldertreev::InitializeClass( )
{
    foldertreev_standardmenulist = new menulist;
    foldertreev_standardkeymap = new keymap;
    bind::BindList(foldertreev_standardbindings, foldertreev_standardkeymap, foldertreev_standardmenulist, &foldertreev_ATKregistry_ );
    return(TRUE);
}

foldertreev::~foldertreev( )
{
    ATKinit;

    if(this->menulistp) {
	delete this->menulistp;
	this->menulistp = NULL;
    }
    if(this->keystatep) {
	delete this->keystatep;
	this->keystatep = NULL;
    }
}

foldertreev::foldertreev( )
{
    ATKinit;

    this->menulistp = (foldertreev_standardmenulist)->DuplicateML(this);
    this->keystatep = keystate::Create(this,foldertreev_standardkeymap);
    THROWONFAILURE(((this->menulistp && this->keystatep) ? TRUE : FALSE));
}

void foldertreev::PostMenus( class menulist *menulist )
{
    if(menulist) {
	(menulist)->SetMask((menulist)->GetMask() & ~menu_description_exposed & ~menu_palette_exposed & ~menu_description_hidden & ~menu_palette_hidden & ~menu_default & ~menu_folded & ~menu_unfolded);
	(this->menulistp)->ChainBeforeML(menulist, 0);
    }
    ((class view*)this->parent)->PostMenus(this->menulistp);
}

static void QuitFolderTree( class foldertreev	 *self )
  {
  class org *org = NULL;

  org = (class org*)(self)->GetDataObject();
  /* messages::ObservedChanged will do all the other work... */
  self->NotifyObservers(1);
  (self)->Destroy();
  (org)->Destroy();
}

static void QuitMessagesProc( class foldertreev	 *self )
  {
  ams::CommitState(TRUE, FALSE, TRUE, TRUE);
}

void foldertreev::PostKeyState( class keystate	 *keystate )
{
    if(keystate)
	(this->keystatep)->AddBefore(keystate);
    ((class view*)this->parent)->PostKeyState(this->keystatep);
}

void foldertreev::FullUpdate( enum view_UpdateType  type, long  left , long  top , long  width , long  height )
{
    class view *v1, *v2;
    class lpair *lp = NULL;

    lp = this->pair_view;
    v1 = (lp)->GetNth( 0);
    v2 = (lp)->GetNth( 1);
    (lp)->VSplit( v1, v2, 0, 1);
    (this)->orgv::FullUpdate(type,left,top,width,height);
}
