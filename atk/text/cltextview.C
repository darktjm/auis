/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("cltextview.H")
#include <textview.H>
#include <cursor.H>
#include <cltextview.H>
#define INITIALNUMOBSERVERS 4


ATKdefineRegistryNoInit(cltextview, textview);
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

// If an observer kills me, delay until Hit() complete.
class view *cltextview::Hit(enum view::MouseAction  action, long  x, long  y, long  numberOfClicks)
{
    Reference(); // in case an observer decides to delete me
    class view *ret = textview::Hit(action, x, y, numberOfClicks);
    Destroy(); // OK, observer can delete me now.
    return ret;
}


void cltextview::GetClickPosition(long  position, long  numberOfClicks, enum view::MouseAction  action, long  startLeft, long  startRight, long  *leftPos, long  *rightPos)
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

  if (::FindObserver( this, observer) == -1)
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

  while ((i = ::FindObserver( this, observer )) != -1)
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
void cltextview::FullUpdate(enum view::UpdateType  type, long  left, long  top, long  width, long  height)
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
