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
#include <hanimapp.H>
#include <im.H>
#include <event.H>
#include <text.H>


#define ANIMATIONDELAY event_MSECtoTU(100)


ATKdefineRegistryNoInit(hanimapp, hwargapp);
void AnimateText(class hanimapp  *self, long time);


hanimapp::hanimapp()
{
    this->GoingForward = TRUE;
    THROWONFAILURE((TRUE));
}

hanimapp::~hanimapp()
{
}

boolean hanimapp::Start()
{
    if (!(this)->hwargapp::Start()) return(FALSE);
    im::EnqueueEvent((event_fptr)AnimateText, (char *)this, ANIMATIONDELAY);
    return(TRUE);
}

void AnimateText(class hanimapp  *self, long time)
{
    class text *t = (self)->GetText();
    int len = (t)->GetLength();

    if (self->GoingForward) {
	if (len > 60) self->GoingForward = FALSE;
	(t)->InsertCharacters( 0, " ", 1);
    } else {
	if (len < 20) self->GoingForward = TRUE;
	(t)->DeleteCharacters( 0, 1);
    }
    (t)->NotifyObservers( observable::OBJECTCHANGED); 
    im::EnqueueEvent((event_fptr)AnimateText, (char *)self, ANIMATIONDELAY);
}
