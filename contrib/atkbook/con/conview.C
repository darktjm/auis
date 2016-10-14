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
#include <conview.H>
#include <conob.H>
#include <message.H>

 


ATKdefineRegistryNoInit(conview, view);


conview::conview()
{
    this->laststring = NULL;
    THROWONFAILURE((TRUE));
}

conview::~conview()
{
    if (this->laststring != NULL) {
	free(this->laststring);
	this->laststring = NULL;
    }
}

view::DSattributes conview::DesiredSize(long  width , long  height, enum view::DSpass  pass, long  *desiredwidth , long  *desiredheight)

{
    *desiredwidth = 200;
    *desiredheight = 50;
    return(view::WidthFlexible | view::HeightFlexible);
}

void conview::SetLastString(char  *buf)
{
    if (buf == NULL) {
	if (this->laststring != NULL) free(this->laststring);
	this->laststring = NULL;
	return;
    }
    if (this->laststring == NULL) {
	this->laststring = (char *)malloc(1+strlen(buf));
    } else {
	this->laststring = (char *)realloc(this->laststring,
				   1+strlen(buf));
    }
    if (this->laststring != NULL) strcpy(this->laststring, buf);
}

class view *conview::Hit(enum view::MouseAction  action, long  x , long  y , long  nc)
{
    if (action == view::LeftDown || action == view::RightDown) {
	char buf[1000];
	class conob *c;

	(this)->WantInputFocus( this);
	c = (class conob *) (this)->GetDataObject();
	(c)->GetStringToDisplay( buf, sizeof(buf), TRUE);
	message::DisplayString(this, 10, buf);
    }
    return((class view *) this);
}

void conview::FullUpdate(enum view::UpdateType  type, long  left , long  top , long  width , long  height)
{
    struct rectangle Rect;
    class conob *co;

    (this)->GetLogicalBounds( &Rect);
    co = (class conob *) (this)->GetDataObject();
    if (co != NULL && co->Boxed) {
	(this)->DrawRectSize( Rect.left,	Rect.top,
			Rect.width - 1, Rect.height - 1);
    }
    Rect.left += 1;
    Rect.top +=	1;
    Rect.width -= 2;
    Rect.height -= 2;
    if (Rect.width <= 0 || Rect.height <= 0) return;
    (this)->DrawMyself( &Rect, TRUE);
}

void conview::Update()
 {
    struct rectangle Rect;

    (this)->GetLogicalBounds( &Rect);
    Rect.left += 1;
    Rect.top +=	1;
    Rect.width -= 2;
    Rect.height -= 2;
    if (Rect.width <= 0 || Rect.height <= 0) return;
    (this)->DrawMyself( &Rect, FALSE);
}

void conview::DrawMyself(struct rectangle  *r, boolean  IsFullUpdate)
{
    char buf[1000];
    class conob *co = (class conob *)
      (this)->GetDataObject();

    if (co == NULL) {
	strcpy(buf, "<No object>");
    } else {
	(co)->GetStringToDisplay( buf, sizeof(buf), FALSE);
    }
    if (!IsFullUpdate) {
	if (co == NULL) return;
	if ((this->laststring != NULL)
	    && !strcmp(this->laststring, buf)) return;
	/* White out and force full update */
	(this)->EraseRect( r);
    }	
    (this)->MoveTo( (r->left + r->width) / 2,
		      (r->top + r->height) / 2);
    (this)->DrawString( buf,
	graphic::BETWEENLEFTANDRIGHT
	| graphic::BETWEENTOPANDBASELINE);
    (this)->SetLastString( buf);
}

