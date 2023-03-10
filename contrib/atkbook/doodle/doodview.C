/* **************************************************** *\
Copyright 1989 Nathaniel S. Borenstein
Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear in
supporting documentation, and that the name of 
Nathaniel S. Borenstein not be used in
advertising or publicity pertaining to distribution of the software
without specific, written prior permission. 

Nathaniel S. Borenstein DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
Nathaniel S. Borenstein BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY
DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
\* ***************************************************** */
#include <andrewos.h>
#include <doodview.H>
#include <view.H>
#include <graphic.H>


ATKdefineRegistryNoInit(doodview, view);
static void DrawMyRect(class doodview  *self, short  transfermode, boolean  FillTheRectangle);


doodview::doodview()
{
    this->downhitX = this->downhitY = this->lastuphitX = this->lastuphitY = this->linewidth = 0;
    THROWONFAILURE((TRUE));
}

doodview::~doodview()
{
}

class view *
doodview::Hit(enum view::MouseAction  action, long  x , long  y , long  numclicks  )
{
    boolean IsUpHit = FALSE;

    switch(action) {
	    case view::LeftDown:
	    case view::RightDown:
		this->linewidth= (2*numclicks) - 1;
		this->downhitX = x;
		this->downhitY = y;
		this->lastuphitX = this->lastuphitY = -9999;
		break;
	    case view::LeftUp:
	    case view::LeftMovement:
		(this)->SetLineWidth( this->linewidth);
		if (this->lastuphitX != -9999) {
		    /* white out old line */
		    (this)->SetTransferMode(
			graphic::WHITE);
		    (this)->MoveTo( this->downhitX,
			this->downhitY);
		    (this)->DrawLineTo(
			this->lastuphitX, this->lastuphitY);
		}
		/* draw new line */
		(this)->SetTransferMode( graphic::BLACK);
		(this)->MoveTo( this->downhitX,
				  this->downhitY);
		(this)->DrawLineTo( x, y);
		this->lastuphitX = x;
		this->lastuphitY = y;
		break;
	    case view::RightUp:
		IsUpHit = TRUE;
		/* Drop through to next case */
	    case view::RightMovement:
		(this)->SetLineWidth( this->linewidth);
		if (this->lastuphitX != -9999) {
		    /* white out old rectangle */
		    DrawMyRect(this, graphic::WHITE, FALSE);
		}
		/* draw new rectangle */
		this->lastuphitX = x;
		this->lastuphitY = y;
		DrawMyRect(this, graphic::BLACK, IsUpHit);
		break;
	    default: // nomouseevent upmovement *filedrop
		break;
    }
    return((class view *) this);
}

static void DrawMyRect(class doodview  *self, short  transfermode, boolean  FillTheRectangle)
{
    struct rectangle Rect;

    if (self->lastuphitX > self->downhitX) {
	Rect.left = self->downhitX;
	Rect.width = self->lastuphitX - self->downhitX;
    } else {
	Rect.left = self->lastuphitX;
	Rect.width = self->downhitX - self->lastuphitX;
    }
    if (self->lastuphitY > self->downhitY) {
	Rect.top = self->downhitY;
	Rect.height = self->lastuphitY - self->downhitY;
    } else {
	Rect.top = self->lastuphitY;
	Rect.height = self->downhitY - self->lastuphitY;
    }
    (self)->SetTransferMode( transfermode);
    (self)->DrawRect( &Rect);
    if (FillTheRectangle) {
	(self)->FillRect( &Rect,
		(self)->BlackPattern());
    }
}
