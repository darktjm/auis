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

#include <butterview.H>
#include <butter.H>
#include <fontdesc.H>
#include <graphic.H>
#include <cursor.H>
#include <view.H>



ATKdefineRegistry(butterview, view, NULL);

void
butterview::FullUpdate(enum view_UpdateType  type, long  left , long  top , long  width , long  height)
{
    struct rectangle Rect;
    class butter *b = (class butter *) (this)->GetDataObject();

    if((type == view_LastPartialRedraw) || (type == view_FullRedraw))
	if (b) {
	    if (!(b)->GetButtonFont()) {
		(b)->SetButtonFont( fontdesc::Create(b->myfontname ? b->myfontname : "andy", b->myfonttype, b->myfontsize));
	    }
	    if (!b->mycursor) {
		b->mycursor = cursor::Create(this);
		(b->mycursor)->SetStandard( Cursor_Octagon);
	    }
	    (this)->GetLogicalBounds( &Rect);
	    (this)->SetTransferMode( graphic_WHITE);
	    (this)->FillRect( &Rect, (this)->GetDrawable());
	    (this)->PostCursor( &Rect, b->mycursor);
	    (this)->SetFont( (b)->GetButtonFont());
	    (this)->SetTransferMode( graphic_BLACK);
	    (this)->MoveTo( (Rect.left + Rect.width) / 2, (Rect.top + Rect.height) / 2);
	    (this)->DrawString( (b)->GetText() ? (b)->GetText() : "<BEEP>", graphic_BETWEENLEFTANDRIGHT | graphic_BETWEENTOPANDBASELINE);
	}
}

void
butterview::Update()
{
    struct rectangle r;

    (this)->GetLogicalBounds( &r);
    (this)->FullUpdate( view_FullRedraw, r.left, r.top, r.width, r.height);
}

class view *
butterview::Hit(enum view_MouseAction  action , long  x , long  y , long  numclicks  )
{
    class butter *b = (class butter *) (this)->GetDataObject();

    if (b && (b)->GetHitFunction()) {
	(b)->GetHitFunction()
	  ((b)->GetRockPtr(), (b)->GetRockInt(), b, action);
    }
    return((class view *)this);
}
