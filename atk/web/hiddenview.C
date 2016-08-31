/* File hiddenview.C created by Ward Nelson
      (c) Copyright IBM Corp 1995.  All rights reserved. 

   hiddenview, an inset for stashing non-displayed text inside of */

#include <andrewos.h>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <proctable.H>
#include <bind.H>
#include <message.H>
#include <view.H>
#include "hidden.H"
#include "hiddenview.H"

keymap *hiddenview::hid_Map;
menulist *hiddenview::hid_Menus;

ATKdefineRegistry(hiddenview, textview, hiddenview::InitializeClass);

static struct bind_Description hiddenBindings[]={
    NULL
};

view_DSattributes hiddenview::DesiredSize(long width, long height, enum view_DSpass pass, long *desiredwidth, long *desiredheight)
{
    if (((hidden *)GetDataObject())->IsVisible()) {
	if (((hidden *)GetDataObject())->IsFullScreen())
	    return this->textview::DesiredSize(width, height, pass, desiredwidth, desiredheight);
	else {
	    view_DSattributes retval;
	    long fracwidth= 50;
	    do  {
		/* keep trying to widen box until it's TOO wide, or short ENOUGH, to be a cute inset */
		retval= this->textview::DesiredSize(fracwidth, height, pass, desiredwidth, desiredheight);
		*desiredwidth= fracwidth;
		fracwidth*= 2;
	    } while (fracwidth<width && *desiredheight>50);
	    if (*desiredheight>50) *desiredheight= 50;
	    return retval;
	}
    }
    *desiredwidth = 0;
    *desiredheight = 0;
    return (view_HeightFlexible | view_WidthFlexible);
}

void hiddenview::LoseInputFocus()
{
    this->textview::hasInputFocus= FALSE;
    WantUpdate(this);
}

void hiddenview::ReceiveInputFocus()
{
    this->textview::hasInputFocus = TRUE;
    this->textview::keystate->next= NULL;
    this->hid_state->next= NULL;
    (this->textview::menus)->SetMask(textview_NoMenus);
    (this->hid_menus)->SetMask(textview_NoMenus);
    if (((hidden *)GetDataObject())->IsFullScreen())
	(this->hid_menus)->SetMask( (this->hid_menus)->GetMask() | hiddenview_FullScreenMenus);
    else
	(this->hid_menus)->SetMask( (this->hid_menus)->GetMask() & ~hiddenview_FullScreenMenus);
    PostKeyState((this->hid_state)->AddBefore(this->textview::keystate)); /* Yech, this makes no sense to me, but works */
    PostMenus(NULL);
    if (GetEditor() == VI)
	if (GetVIMode() == COMMAND)
	    message::DisplayString(this, 0, "Command Mode");
	else
	    message::DisplayString(this, 0, "Input Mode");
    WantUpdate(this);
}

void hiddenview::PostMenus(class menulist *menulist)
{
    (this->hid_menus)->ChainBeforeML(menulist, (long)this);
    (this)->textview::PostMenus(this->hid_menus);
}


static void DoUpdate(hiddenview *self, boolean full)
{
    struct rectangle enclosingRect;
    enclosingRect.top = 0; enclosingRect.left = 0;
    enclosingRect.width  = (self)->GetLogicalWidth();
    enclosingRect.height = (self)->GetLogicalHeight();

    if (!((hidden *)(self)->GetDataObject())->IsVisible())
	(self)->EraseRect(&(enclosingRect));
    else if (!((hidden *)(self)->GetDataObject())->IsFullScreen()) {
	enclosingRect.width--; enclosingRect.height--;
	(self)->DrawRect(&(enclosingRect));
    }
}

void hiddenview::FullUpdate(enum view_UpdateType type, long left, long top, long width, long height)
{
    if (((hidden *)GetDataObject())->IsVisible())
	(this)->textview::FullUpdate(type, left, top, width, height);
    DoUpdate(this, TRUE);
}

void hiddenview::Update()
{
    if (((hidden *)GetDataObject())->IsVisible())
	(this)->textview::Update();
    DoUpdate(this,FALSE);
}


void hiddenview::Print(FILE *f, const char *process, const char *final, boolean toplevel)
{
    /* NO-OP.  Don't print anything for this. */
}


boolean hiddenview::InitializeClass()
{
    hiddenview::hid_Menus= new menulist;
    hiddenview::hid_Map= new keymap;
    bind::BindList(hiddenBindings, hiddenview::hid_Map,hiddenview::hid_Menus, &hiddenview_ATKregistry_);
    return TRUE;
}

hiddenview::hiddenview()
{
    ATKinit;
    this->hid_state= keystate::Create(this, hid_Map);
    this->hid_menus= (hid_Menus)->DuplicateML(this);
    THROWONFAILURE(TRUE);
}

hiddenview::~hiddenview()
{
    ATKinit;
    delete this->hid_state;
    delete this->hid_menus;
    return;
}
