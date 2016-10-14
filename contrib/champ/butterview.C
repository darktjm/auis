/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>

#include <butterview.H>
#include <butter.H>
#include <fontdesc.H>
#include <graphic.H>
#include <cursor.H>
#include <view.H>



ATKdefineRegistryNoInit(butterview, view);

void
butterview::FullUpdate(enum view::UpdateType  type, long  left , long  top , long  width , long  height)
{
    struct rectangle Rect;
    class butter *b = (class butter *) (this)->GetDataObject();

    if((type == view::LastPartialRedraw) || (type == view::FullRedraw))
	if (b) {
	    if (!(b)->GetButtonFont()) {
		(b)->SetButtonFont( fontdesc::Create(b->myfontname ? b->myfontname : "andy", b->myfonttype, b->myfontsize));
	    }
	    if (!b->mycursor) {
		b->mycursor = cursor::Create(this);
		(b->mycursor)->SetStandard( Cursor_Octagon);
	    }
	    (this)->GetLogicalBounds( &Rect);
	    (this)->SetTransferMode( graphic::WHITE);
	    (this)->FillRect( &Rect, (this)->GetDrawable());
	    (this)->PostCursor( &Rect, b->mycursor);
	    (this)->SetFont( (b)->GetButtonFont());
	    (this)->SetTransferMode( graphic::BLACK);
	    (this)->MoveTo( (Rect.left + Rect.width) / 2, (Rect.top + Rect.height) / 2);
	    (this)->DrawString( (b)->GetText() ? (b)->GetText() : "<BEEP>", graphic::BETWEENLEFTANDRIGHT | graphic::BETWEENTOPANDBASELINE);
	}
}

void
butterview::Update()
{
    struct rectangle r;

    (this)->GetLogicalBounds( &r);
    (this)->FullUpdate( view::FullRedraw, r.left, r.top, r.width, r.height);
}

class view *
butterview::Hit(enum view::MouseAction  action , long  x , long  y , long  numclicks  )
{
    class butter *b = (class butter *) (this)->GetDataObject();

    if (b && (b)->GetHitFunction()) {
	(b)->GetHitFunction()
	  ((b)->GetRockPtr(), (b)->GetRockInt(), b, action);
    }
    return((class view *)this);
}
