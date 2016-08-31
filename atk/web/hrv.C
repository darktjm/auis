/* File hrv.C created by Ward Nelson
      (c) Copyright IBM Corp 1995.  All rights reserved. 

   hrv, an inset that draws and prints a window-width horizontal line */

#include <andrewos.h>
#include <textview.H>
#include "hrv.H"

ATKdefineRegistry(hrv, view, NULL);

view_DSattributes hrv::DesiredSize(long width, long height, enum view_DSpass pass, long *desiredwidth, long *desiredheight)
{
    *desiredwidth = width;
    *desiredheight = 1;
    return (view_HeightFlexible | view_WidthFlexible);
}

void hrv::FullUpdate(enum view_UpdateType type, long left, long top, long width, long height)
{
    struct rectangle enclosingRect;
    enclosingRect.top = 0; enclosingRect.left = 0;
    enclosingRect.width  = GetLogicalWidth() /* -1 */ ;
    enclosingRect.height = GetLogicalHeight() /* -1 */ ;

    SetTransferMode(graphic_WHITE);
    EraseRect(&(enclosingRect));

    SetTransferMode(graphic_COPY);
    MoveTo(0,0);
    DrawLineTo(enclosingRect.width, 0);
}

void hrv::LinkTree(class view *parent)
{
    this->view::LinkTree(parent);
    while (parent && !ATK::IsTypeByName(parent->GetTypeName(), "textview"))
	parent= parent->parent;
    this->parentview= (textview *)parent;
}

void hrv::ReceiveInputFocus()
{
    this->view::ReceiveInputFocus();
    if (this->parentview) {	
	/* this should only happen when the view is created */
	(this->parentview)->SetDotPosition((this->parentview)->GetDotPosition() + 1);
	(this->parentview)->WantInputFocus(this->parentview);
    }
}

void hrv::Print(FILE *f, const char *process, const char *final, boolean toplevel)
{
    fputs("\\l(5.5i_\n\\&\n.sp 12p\n", f);
}

hrv::hrv()
{
    this->parentview= NULL;
    THROWONFAILURE(TRUE);
}
