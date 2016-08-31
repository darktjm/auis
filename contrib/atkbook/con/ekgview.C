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
#include <ekgview.H>
#include <conob.H>


ATKdefineRegistry(ekgview, conview, NULL);


ekgview::ekgview()
{
    this->histctr = 0;
    THROWONFAILURE((TRUE));
}

ekgview::~ekgview()
{
}

#define CALCX(i) (r->left + (((i) * r->width) / HISTSIZE))
#define CALCY(i) (r->top + r->height - ((r->height * (this->history[(i)] - co->displaymin)) / (co->displaymax - co->displaymin)))

void ekgview::DrawMyself(struct rectangle  *r, boolean  IsFullUpdate)
{
    int i;
    class conob *co = (class conob *)
      (this)->GetDataObject();

    if (!IsFullUpdate) {
	this->history[this->histctr++] = (co)->GetNumval();
	if (this->histctr >= HISTSIZE) {
	    /* White out, advance history,
	         then fall through to fullupdate */
	    for(i=0; i<HISTSIZE-SLIDEBY; ++i) {
		this->history[i] = this->history[i+SLIDEBY];
	    }
	    this->histctr -= SLIDEBY;
	    (this)->EraseRect( r);
	} else { /* Really NOT a full update */
	    if (this->histctr <= 1) {
		(this)->MoveTo( CALCX(0), CALCY(0));
	    } else {
		(this)->MoveTo( CALCX(this->histctr-2),
			       CALCY(this->histctr-2));
	    }
	    (this)->DrawLineTo( CALCX(this->histctr-1),
			       CALCY(this->histctr-1));
	    return; /* Done with Update operation */
	}
    }
    (this)->MoveTo( CALCX(0), CALCY(0));
    for (i=1; i<this->histctr; ++i) {
	(this)->DrawLineTo( CALCX(i), CALCY(i));
    }
}
