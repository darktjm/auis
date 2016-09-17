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
#include <buttview.H>
#include <butt.H>
#include <fontdesc.H>
#include <graphic.H>
#include <cursor.H>




ATKdefineRegistry(buttview, view, NULL);


buttview::buttview()
{
    this->myfontdesc = fontdesc::Create("andy",
				fontdesc_Bold, 12);
    this->mycursor = cursor::Create(this);
    this->lasttext = NULL;
    if (this->myfontdesc == NULL || this->mycursor == NULL) {
	THROWONFAILURE( (FALSE));
    }
    (this->mycursor)->SetStandard( Cursor_Octagon);
    this->HitFunction = NULL;
    this->rock1 = NULL;
    this->rock2 = NULL;
    THROWONFAILURE((TRUE));
}

buttview::~buttview()
{
    delete this->mycursor;
    delete this->myfontdesc;
    if (this->lasttext) free(this->lasttext);
}

void
buttview::FullUpdate(enum view_UpdateType  type, long  left , long  top , long  width , long  height)
{
    struct rectangle Rect;
    class butt *b = (class butt *)
      (this)->GetDataObject();
    const char *txt = NULL;

    (this)->GetLogicalBounds( &Rect);
    (this)->EraseRect( &Rect);
    (this)->PostCursor( &Rect, this->mycursor);
    (this)->MoveTo( (Rect.left + Rect.width) / 2,
		       (Rect.top + Rect.height) / 2);
    (this)->SetFont( this->myfontdesc);
    if (b != NULL) {
	txt = (b)->GetText();
	if (this->lasttext != NULL) free(this->lasttext);
	if (txt != NULL) {
	    this->lasttext = strdup(txt);
	} else {
	    this->lasttext = NULL;
	}
    }
    if (txt == NULL) txt = "<BEEP>";
    (this)->DrawString( txt,
	graphic_BETWEENLEFTANDRIGHT
	| graphic_BETWEENTOPANDBASELINE);
}

void
buttview::Update()
{
    struct rectangle r;
    class butt *b = (class butt *)
      (this)->GetDataObject();
    boolean HasChanged = FALSE;
    const char *btext = (b)->GetText();

    if (btext == NULL) {
	if (this->lasttext != NULL) HasChanged = TRUE;
    } else if (this->lasttext == NULL
		|| strcmp(btext, this->lasttext)) {
	HasChanged = TRUE;
    }
    if (HasChanged) {
	(this)->GetLogicalBounds( &r);
	(this)->FullUpdate( view_FullRedraw, r.left, r.top,
			      r.width, r.height);
    } /* Otherwise the text didn't change, no need to redraw */
}

class view *
buttview::Hit(enum view_MouseAction  action, long  x , long  y , long  numclicks)
{
    class butt *b = (class butt *)
      (this)->GetDataObject();
    buttview_hit_fptr hitfunc = (this)->GetHitFunction();

    if (hitfunc) {
	hitfunc( (this)->GetRock1(),
	   (this)->GetRock2(), b, action);
    }
    return((class view *)this);
}

void
buttview::SetHitFunction(buttview_hit_fptr f)
{
    this->HitFunction = f;
}

void
buttview::SetRocks(void * r1 , void * r2)
{
    this->rock1 = r1;
    this->rock2 = r2;
}

