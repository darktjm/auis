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
#include <clogview.H>
#include <scroll.H>
#include <text.H>
#include <textview.H>
#include <conob.H>




ATKdefineRegistryNoInit(clogview, conview);


clogview::clogview()
{
    this->s = new scroll;
    this->t = new text;
    this->tv = new textview;
    if (this->s == NULL || this->t == NULL
	 || this->tv == NULL) THROWONFAILURE((FALSE));
    (this->tv)->SetDataObject( this->t);
    (this->s)->SetView( this->tv);
    (this->t)->SetReadOnly( TRUE);
    THROWONFAILURE((TRUE));
}
 
clogview::~clogview()
{
    (this->tv)->UnlinkTree();
    (this->s)->UnlinkTree();
    (this->t)->Destroy();
    (this->s)->Destroy();
    (this->tv)->Destroy();
}

class view *
clogview::Hit(enum view::MouseAction  action, long  x , long  y , long  numclicks  )
{
    return((this->s)->Hit( action, x, y, numclicks));
}

void
clogview::FullUpdate(enum view::UpdateType  type, long  left , long  top , long  width , long  height)
{
    struct rectangle Rect;
    (this)->GetVisualBounds( &Rect);
    (this->s)->InsertView( this, &Rect);
    (this->s)->FullUpdate( type, left, top, width, height);
}

void
clogview::Update()
{
    char buf[1000];
    int textlen, buflen;
    class conob *co = (class conob *)
      (this)->GetDataObject();

    if (co) {
	(co)->GetStringToDisplay( buf, sizeof(buf)-1, FALSE);
	if (index(buf, '\n') == NULL) strcat(buf, "\n");
	if (((this)->GetLastString() != NULL)
	    && !strcmp((this)->GetLastString(), buf)) {
	    return;
	}
	buflen = strlen(buf);
	textlen = (this->t)->GetLength();
	(this->t)->AlwaysInsertCharacters( textlen,
				    buf, buflen);
	(this)->SetLastString( buf);
	(this->tv)->FrameDot( textlen+buflen-1);
	(this->tv)->Update();
	if (textlen+buflen > 10000) {
	    (this->t)->AlwaysDeleteCharacters( 0, 1000);
	}
    }
}

void clogview::LinkTree(class view  *v)
{
    (this)->conview::LinkTree( v);
    (this->s)->LinkTree( this);
}

