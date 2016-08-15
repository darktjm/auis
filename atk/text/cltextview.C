/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/

#include <andrewos.h>

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/text/RCS/cltextview.C,v 3.2 1994/11/30 20:42:06 rr2b Stab74 $";
#endif


 


ATK_IMPL("cltextview.H")
#include <textview.H>
#include <cursor.H>
#include <cltextview.H>
#define INITIALNUMOBSERVERS 4


ATKdefineRegistry(cltextview, textview, NULL);
#ifndef NORCSID
#endif
static short FindObserverCallBack(class cltextview  * self, ATK   *observer, cltextview_hitfptr  callBack);
static short FindObserver( class cltextview  * self, ATK   *observer );
static short FreeSlot( class cltextview  * self );


static short FindObserverCallBack(class cltextview  * self, ATK   *observer, cltextview_hitfptr  callBack)
               {
  short i = 0;

  for (i = 0; i < self->maxObservers; ++i)
    if (self->observers[i].observer == observer
	&& self->observers[i].callBack == callBack)
      return i;
  return -1;
}


static short FindObserver( class cltextview  * self, ATK   *observer )
          {
  short i = 0;

  for (i = 0; i < self->maxObservers; ++i)
    if (self->observers[i].observer == observer)
      return i;
  return -1;
}


static short FreeSlot( class cltextview  * self )
     {
  short i,j;
  
  for (i = 0; i < self->maxObservers; ++i)
    if (self->observers[i].observer == NULL)
      return i;

  if (self->maxObservers == 0)
    {
      self->maxObservers = INITIALNUMOBSERVERS;
      self->observers = (struct cltextview_observer *) malloc (INITIALNUMOBSERVERS * sizeof (struct cltextview_observer));
    }
  else
    {
      self->maxObservers += self->maxObservers / 2;
      self->observers = (struct cltextview_observer *)
	realloc(self->observers, self->maxObservers * sizeof (struct cltextview_observer));
    }
  j = i;
  while (i < self->maxObservers)
    self->observers[i++].observer = NULL;
  return j;
}

void cltextview::GetClickPosition(long  position, long  numberOfClicks, enum view_MouseAction  action, long  startLeft, long  startRight, long  *leftPos, long  *rightPos)
                                    {
	int i;
	for (i = 0; i < this->maxObservers; ++i)
	    if (this->observers[i].observer != NULL)
		(*(this->observers[i].callBack))(this->observers[i].observer, this,
						 &position, &numberOfClicks, &action, &startLeft, &startRight, leftPos, rightPos, this->observers[i].rock ,cltextview_PREPROCESS);
	(this)->textview::GetClickPosition( position, numberOfClicks, action, startLeft, startRight, leftPos, rightPos);
	for (i = 0; i < this->maxObservers; ++i)
	    if (this->observers[i].observer != NULL)
		(*(this->observers[i].callBack))(this->observers[i].observer, this,
						 &position, &numberOfClicks, &action, &startLeft, &startRight, leftPos, rightPos, this->observers[i].rock ,cltextview_POSTPROCESS);
    }


void cltextview::AddClickObserver( ATK   * observer, cltextview_hitfptr  callBack, long  rock )
                    {
  short free;

  if (FindObserver( this, observer) == -1)
    {
      free = FreeSlot(this);
      this->observers[free].observer = observer;
      this->observers[free].callBack = callBack;
      this->observers[free].rock = rock;
    }
}


void cltextview::RemoveClick( ATK   * observer, cltextview_hitfptr  callBack )
               {
  short i;

  if ((i = FindObserverCallBack(this, observer, callBack)) != -1)
    {
      this->observers[i].observer = NULL;
    }
}
     

void cltextview::RemoveClickObserver( ATK   * observer )
          {
  short i;

  while ((i = FindObserver( this, observer )) != -1)
    this->observers[i].observer = NULL;
}


cltextview::cltextview()
{
  this->maxObservers = 0;
  this->observers = NULL;
  this->cursor = cursor::Create(this);
  (this->cursor)->SetStandard(Cursor_LeftPointer);
  THROWONFAILURE( TRUE);
}
void cltextview::FullUpdate(enum view_UpdateType  type, long  left, long  top, long  width, long  height)
{
    struct rectangle rect;
    (this)->textview::FullUpdate( type, left, top, width, height);
    (this)->GetVisualBounds(&rect);
    (this)->PostCursor(&rect,this->cursor);
}
cltextview::~cltextview()
{
    if(this->cursor)
	delete this->cursor;
}
