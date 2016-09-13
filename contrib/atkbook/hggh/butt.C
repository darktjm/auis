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
#include <butt.H>
#include <fontdesc.H>
#include <graphic.H>
#include <cursor.H>




ATKdefineRegistry(butt, view, NULL);


butt::butt()
{
    this->text = NULL;
    this->HitFunction = NULL;
    this->rock1 = NULL;
    this->rock2 = NULL;
    this->myfontdesc = fontdesc::Create("andy", fontdesc_Bold, 12);
    this->mycursor = cursor::Create(this);
    (this->mycursor)->SetStandard( Cursor_Octagon);
    THROWONFAILURE((TRUE));
}

butt::~butt()
{
    delete this->mycursor;
    delete this->myfontdesc;
    if (this->text) free(this->text);
}

class view *
butt::Hit(enum view_MouseAction  action, long  x , long  y , long  numclicks)
{
    if (this->HitFunction) {
	this->HitFunction(this->rock1, this->rock2,
			  this, action);
    }
    return((class view *)this);
}

void
butt::Update()
{
    struct rectangle r;

    (this)->GetLogicalBounds( &r);
    (this)->EraseRect( &r);
    (this)->FullUpdate( view_FullRedraw, r.left,
		       r.top, r.width, r.height);
}

void
butt::FullUpdate(enum view_UpdateType  type, long  left , long  top , long  width , long  height)
{
    struct rectangle Rect;
    char *dtext;

    (this)->GetLogicalBounds( &Rect);
    (this)->SetFont( this->myfontdesc);
    (this)->MoveTo( (Rect.left + Rect.width) / 2,
		   (Rect.top + Rect.height) / 2);
    if (this->text == NULL) {
	dtext = "<BEEP>";
    } else {
	dtext = this->text;
    }
    (this)->DrawString( dtext,
		       graphic_BETWEENLEFTANDRIGHT
		       | graphic_BETWEENTOPANDBASELINE);
    (this)->PostCursor( &Rect, this->mycursor);
}

void
butt::SetText(char  *txt)
{
    if (this->text) free(this->text);
    this->text = strdup(txt);
    (this)->WantUpdate( this);
}

void
butt::SetButtonFont(class fontdesc  *f)
{
    if (this->myfontdesc != NULL) {
	delete this->myfontdesc;
    }
    this->myfontdesc = f;
    (this)->WantUpdate( this);
}

void
butt::SetHitFunction(butt_hit_fptr f)
{
    this->HitFunction = f;
}

void
butt::SetRocks(void * r1 , void * r2)
{
    this->rock1 = r1;
    this->rock2 = r2;
}

