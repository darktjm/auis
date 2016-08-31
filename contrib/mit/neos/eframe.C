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

/* eframe.c 
 * This is a silly little view so we
 * can create a buffer window with our own local
 * command set.
 */

/* ************************************************************
 *  Copyright (C) 1989, 1990, 1991
 *  by the Massachusetts Institute of Technology
 *   For full copyright information see:'mit-copyright.h'     *
 ************************************************************ */

#include <andrewos.h>
#include <mit-copyright.h>
#include <bind.H>
#include <buffer.H>

#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <environ.H>
#include <fontdesc.H>
#include <frame.H>
#include <im.H>
#include <proctable.H>
#include <style.H>
#include <text.H>
#include <textview.H>

#include <eframe.H>

static class keymap *eframeKeys;
static class menulist *eframeMenu;


ATKdefineRegistry(eframe, view, eframe::InitializeClass);
void eframe_SetDefault(class eframe  *self);
void eframe_SetDisplay(class eframe  *self);
void eframe_NewWindow(class eframe  *self);


void eframe_SetDefault(class eframe  *self)
    {
    class textview *tv;
    long dsize, usize, fontStyle;
    enum style_FontSize basis;
    char *font, fontname[100];
    class buffer *buffer;

    /* tickle the font size if it's different from the default */
    tv = (class textview *) (self->frame)->GetView();
    if (strcmp ((tv)->GetTypeName(), "textview") == 0) {
	class style *ds = (tv)->GetDefaultStyle();

	if ((font = environ::GetProfile("bodyfont")) == NULL || ! fontdesc::ExplodeFontName(font, fontname, sizeof(fontname), &fontStyle, &dsize)) {
	    dsize = 12;
	}

	usize = environ::GetProfileInt("DisplayFontsize", 20);
	(ds)->GetFontSize( &basis, &usize);
	if (basis == style_ConstantFontSize && dsize != usize)
	    (ds)->SetFontSize( style_ConstantFontSize, dsize);
	(self)->WantUpdate( self);
    }
}

void eframe_SetDisplay(class eframe  *self)
    {
    class textview *tv;
    long dsize, usize;
    enum style_FontSize basis;
    class buffer *buffer;

    /* tickle the font size if it's different from the default */
    tv = (class textview *) (self->frame)->GetView();
    if (strcmp ((tv)->GetTypeName(), "textview") == 0) {
	class style *ds = (tv)->GetDefaultStyle();
	usize = environ::GetProfileInt("DisplayFontsize", 20);
	(ds)->GetFontSize( &basis, &dsize);
	if (basis == style_ConstantFontSize && dsize != usize)
	    (ds)->SetFontSize( style_ConstantFontSize, usize);
	(self)->WantUpdate( self);
    }
}

void eframe_NewWindow(class eframe  *self)
    {
    class eframe *new_c;
    register class buffer *buffer;
    class im *window;
    new_c = new class eframe;

    window = im::Create(NULL);
    (window)->SetView( new_c);

    buffer = (self->frame)->GetBuffer();
    (new_c->frame)->SetBuffer( buffer, TRUE);
    (new_c)->WantInputFocus( new_c);
}

static struct bind_Description eframeBindings[]={
/*  { proc-name,     keybinding,rock, Menu name,        Menu rock, menuflag,
      function, documentation [, module-name]}
 */
    {"eframe-new-window",    "\0302", 0, NULL,         0, EFRAMEMENUS_general,   (proctable_fptr)eframe_NewWindow,      "Creates a new window"},
    {"eframe-set-display-font",    NULL, 0, "View in BIG Font",         0, EFRAMEMENUS_general,   (proctable_fptr)eframe_SetDisplay,      "View buffer in Display Font"},
    {"eframe-set-default-font",    NULL, 0, "View in Default Font",         0, EFRAMEMENUS_general,   (proctable_fptr)eframe_SetDefault,      "View buffer in Default Font"},
    NULL
};

boolean eframe::InitializeClass()
{
    eframeMenu = new class menulist;
    eframeKeys = new class keymap;
    bind::BindList(eframeBindings, eframeKeys, eframeMenu, &eframe_ATKregistry_ );

    return TRUE;
}

void eframe::PostMenus(class menulist  *menulist)
{
    /* Received the children's menus. Want to add our own options,
           then pass menulist up to superclass */
    (this->menus)->ClearChain();
    (this->menus)->SetMask( this->menuflags);
    if (menulist) (this->menus)->ChainAfterML( menulist, (long)menulist);

    (this)->view::PostMenus(this->menus); 
}


void eframe::ReceiveInputFocus()
/* 
  We want to ensure that when we receive the focus, the focus is passed on
  into the editing window
 */

{
    (this)->WantInputFocus( (this->frame)->GetView());
}

eframe::eframe()
{
	ATKinit;

    this->frame = new class frame;
    (this->frame)->SetCommandEnable( TRUE);

    /* Menus */
    this->menuflags = EFRAMEMENUS_general;
    this->menus = (eframeMenu)->DuplicateML( this);

    this->keys = keystate::Create(this, eframeKeys);

   /* Place the end result into the view-tree */
    (this->frame)->LinkTree( this);
   
    THROWONFAILURE( TRUE);
}

eframe::~eframe()
{
	ATKinit;

    if (!this) return;

    if (this->menus) delete this->menus;
    if (this->keys) delete this->keys;
}

void eframe::LinkTree(class view  *parent)
{
    (this)->view::LinkTree( parent);

    if (this->frame)
        (this->frame)->LinkTree( this);
}

void eframe::FullUpdate(enum view_UpdateType  type, long  left , long  top , long  width , long  height)
/* Replace the 'eframe' view with the view wanted: self->frame */
{
    struct rectangle childRect;

    (this)->GetLogicalBounds( &childRect);
    (this->frame)->InsertView( this, &childRect);
    (this->frame)->FullUpdate( type, left, top, width, height);

}

class view *eframe::Hit(enum view_MouseAction  action, long  x , long  y , long  clicks)
/*
  All we want to do is to pass the hit into the sub-views
 */
{
    return (this->frame)->Hit( action, x, y, clicks);
}

void eframe::PostKeyState(class keystate  *ks)
/* Want to add our own keybindings into the chain that gets passed to us */
{
    if (!ks) return;

    this->keys->next = NULL;
    (this->keys)->AddBefore( ks); 
    (this)->view::PostKeyState( this->keys);
}

void eframe::Update()
{
    (this)->EraseVisualRect();
    (this)->FullUpdate( view_FullRedraw, (this)->GetLogicalTop(), (this)->GetLogicalLeft(), (this)->GetLogicalWidth(), (this)->GetLogicalHeight());
}

