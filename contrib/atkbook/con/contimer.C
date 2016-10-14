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
#include <contimer.H>
#include <im.H>
#include <event.H>
#include <observable.H>




ATKdefineRegistryNoInit(contimer, observable);
void HandleTimer(class contimer  *self);


contimer::contimer()
{
    this->proc = NULL;
    /* First timer event in one second */
    this->queuedevent = im::EnqueueEvent((event_fptr)HandleTimer, (char *)this, event_MSECtoTU(1000));
    /* Subsequently every 15 unless SetInterval is called  */
    this->interval = 15000;
    THROWONFAILURE((TRUE));
}

contimer::~contimer()
{
    if (this->queuedevent != NULL) {
	(this->queuedevent)->Cancel();
    }
    (this)->NotifyObservers( observable::OBJECTDESTROYED);
}

void HandleTimer(class contimer  *self)
{
    if (self->proc) (self->proc)(self);
    (self)->NotifyObservers( observable::OBJECTCHANGED);
    self->queuedevent = im::EnqueueEvent((event_fptr)HandleTimer, (char *)self, event_MSECtoTU(self->interval)); 
}

void contimer::SetInterval(int  ms)
{
    this->interval = ms;
}

void contimer::SetDataCollectionProc(collection_fptr proc)
{
    this->proc = proc;
}
