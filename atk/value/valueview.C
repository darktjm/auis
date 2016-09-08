/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
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
ATK_IMPL("valueview.H")
#include <atom.H>
#include <atomlist.H>
#include <graphic.H>
#include <rm.H>
#include <view.H>
#include <value.H>
#include <valueview.H>
#define False 0
#define True  1
static class atomlist *  AL_background;
static class atomlist *  AL_border;
static class atomlist *  AL_border_width;
static class atom *  A_graphic;
static class atom *  A_long;
#define InternAtoms ( \
   AL_background = atomlist::StringToAtomlist("background") ,\
   AL_border = atomlist::StringToAtomlist("border") ,\
   AL_border_width = atomlist::StringToAtomlist("border-width") ,\
   A_graphic = atom::Intern("graphic") ,\
   A_long = atom::Intern("long") )

/**************** private functions ****************/


ATKdefineRegistry(valueview, view, valueview::InitializeClass);
static void LookupParameters(class valueview  * self);
static void DrawFromScratch(class valueview  * self);
static void DA(class valueview  * self);
static void DDA(class valueview  * self);
static void DNV(class valueview  * self);
static void DBV(class valueview  * self);
static void DH(class valueview  * self);
static void DDH(class valueview  * self);
void valueview__GetCenter( class valueview  * self, long  * x , long  * y );


static void LookupParameters(class valueview  * self)
     {
  struct resourceList parameters[4] = {};
  parameters[0].name = AL_background;
  parameters[0].type = A_graphic;
  parameters[1].name = AL_border;
  parameters[1].type = A_graphic;
  parameters[2].name = AL_border_width;
  parameters[2].type = A_long;
  parameters[3].name = NULL;
  parameters[3].type = NULL;

  (self)->GetManyParameters(parameters,NULL,NULL);
  if (parameters[0].found)
    self->back = (class graphic *)parameters[0].data;
  else
    self->back = (self)->WhitePattern();

  if (parameters[1].found)
    self->border = (class graphic *)parameters[1].data;
  else
    self->border = (self)->BlackPattern();

  if (parameters[2].found)
    self->borderPixels = parameters[2].data;
  else
    self->borderPixels = 1;

  if (self->deactivationMask == NULL)
    self->deactivationMask = (self)->GrayPattern(1,10);

  (self)->LookupParameters();
}


static void DrawFromScratch(class valueview  * self)
     {
  long x,y,width,height;

  x = (self)->GetLogicalLeft();
  y = (self)->GetLogicalTop();
  width = (self)->GetLogicalWidth();
  height = (self)->GetLogicalHeight();

/*  valueview_RestoreGraphicsState( self ); */
  (self )->ClearClippingRect( );

  (self)->SetTransferMode(  graphic_COPY );

  if (width < (self->borderPixels << 1) || height < (self->borderPixels << 1))
    {
      self->x = x;
      self->y = y;
      self->width = width;
      self->height = height;
    }
  else
    {
      if (self->borderPixels != 0)
	(self)->FillRectSize( x, y, width, height, self->border);
      self->x = x + self->borderPixels;
      self->y = y + self->borderPixels;
      self->width = width - (self->borderPixels << 1);
      self->height = height - (self->borderPixels << 1);
    }
  self->viewx = x;
  self->viewy = y;
  self->viewheight = height;
  self->viewwidth = width;

  (self)->FillRectSize( self->x, self->y, self->width,
			  self->height, self->back);
  (self)->DrawFromScratch( self->x, self->y, self->width,
			  self->height);

  if (!self->active)
    {
      (self)->SetTransferMode(  self->deactivationTransferMode );
      (self)->FillRectSize( self->x, self->y,
			      self->width, self->height,
			      self->deactivationMask);
    }
  else if (self->mouseIsOnTarget)
    (self)->DrawHighlight();

}





/**************** class methods ****************/

boolean valueview::InitializeClass()
{
  InternAtoms;
  return TRUE;
}


/**************** instance methods ****************/

valueview::valueview()
         {
	ATKinit;

  this->borderPixels = 1;
  this->back =  NULL;
  this->border =  NULL;
  this->deactivationMask = NULL;
  this->deactivationTransferMode = graphic_AND;
  this->active = True;
  this->mouseIsOnTarget = False;
  this->HasInputFocus = FALSE;
  this->updateqp = new updateq;
  (this->updateqp)->SetView( this);
  THROWONFAILURE( TRUE);
}


view_DSattributes valueview::DesiredSize(long  width , long  height, enum view_DSpass  pass, long  *desiredwidth , long  *desiredheight)

{
    *desiredwidth = 75;
    *desiredheight = 75;
    return (view_DSattributes)( view_WidthFlexible | view_HeightFlexible) ;
}

void valueview::RequestUpdateFunction(updateq_fptr fp)
          {
  (this->updateqp)->EnqueueUpdateFunction(  fp );
  (this)->WantUpdate(  this );
}


void valueview::RequestFullUpdate( )
     {
  (this->updateqp )->ClearUpdateQueue( );
  (this)->RequestUpdateFunction((updateq_fptr)::DrawFromScratch );
}


void valueview::FullUpdate( enum view_UpdateType  type, long  x, long  y, long  width, long  height )
               {
  if (/* type == view_NewParameters  ||  */ this->back == NULL) 
    ::LookupParameters(this);
  switch (type) {
      case view_FullRedraw:
      case view_LastPartialRedraw:
	  ::DrawFromScratch(this);
	  break;
      case view_PartialRedraw:
      case view_MoveNoRedraw:
      case view_Remove:
      default:
	  break;
  }
}
#define MAXPAR 36
static struct resourceList par[MAXPAR];
int setpar = 0;
void valueview::GetManyParameters( struct resourceList  * resources, class atomlist  * name, class atomlist  * class_c )
{
    (this)->view::GetManyParameters(  resources, name, class_c );
    if(setpar){
	int i;
	for(i = 0; i < MAXPAR; i++){
	    par[i] = *resources++;
	    if(par[i].name == NULL) break;
	}
    }
}
struct resourceList *valueview::GetDesiredParameters()
{
    setpar = TRUE;
    (this)->LookupParameters();
    setpar = FALSE;
    return par;
}
    
void valueview::LookupParameters()
     {
  /* subclass responsability */
}

void valueview::ObservedChanged( class observable  * observed, long  status )
               {
  switch (status)
    {
    case observable_OBJECTDESTROYED:
      (this)->Destroyed();
      break;
    case value_NEWVALUE:
      (this)->NewValue();
      break;
    case value_NOCHANGE:
      (this)->NoChange();
      break;
    case value_BADVALUE:
      (this)->BadValue();
      break;
    case value_NOTACTIVE:
      (this)->Deactivate();
      break;
    case value_ACTIVE:
      (this)->Activate();
      break;
    default:
      (this)->Changed( status);
      break;
    }
}

static void DA(class valueview  * self)
     {
    (self)->DrawActivation();
}

static void DDA(class valueview  * self)
     {
    (self)->DrawDeactivation();
}
static void DNV(class valueview  * self)
     {
    (self)->DrawNewValue();
}

static void DBV(class valueview  * self)
     {
    (self)->DrawBadValue();
}
static void DH(class valueview  * self)
     {
    (self)->DrawHighlight();
}

static void DDH(class valueview  * self)
     {
    (self)->DrawDehighlight();
}

void valueview::Activate()
     {
  if (!this->active)
    {
      this->active = True;
      (this)->RequestUpdateFunction((updateq_fptr)DA);
    }
}

void valueview::Deactivate()
     {
  if (this->active)
    {
      this->active = False;
      (this)->RequestUpdateFunction((updateq_fptr)DDA);
    }
}

void valueview::Update()
     {
/*  valueview_RestoreGraphicsState( self );*/
  (this->updateqp )->ExecuteUpdateQueue( );
}


void valueview::DrawFromScratch(long x, long y, long width, long height )
     {
  /* Subclass responsibility */
}


void valueview::DrawActivation()
     {
  ::DrawFromScratch( this );
}



void valueview::DrawDeactivation()
     {
  (this )->ClearClippingRect( );
  (this)->SetTransferMode(  this->deactivationTransferMode );
  (this)->FillRectSize( this->x, this->y,
			  this->width, this->height,
			  this->deactivationMask);
}


void valueview::DrawNewValue()
     {
  /* subclass responsibility */
}


void valueview::DrawBadValue()
     {
  class graphic * black;
  (this )->ClearClippingRect( );
  black = (this)->BlackPattern();
  (this)->SetTransferMode( graphic_XOR);
  (this)->FillRectSize( this->viewx, this->viewy,
			  this->viewwidth, this->viewheight, black);
  (this)->FillRectSize( this->viewx, this->viewy,
			  this->viewwidth, this->viewheight, black);
}


void valueview::DrawNoChange()
     {
  /* subclass responsibility */
}


void valueview::DrawDestroyed()
     {
  /* subclass responsibility */
}


void valueview::DrawHighlight()
     {
  /* subclass responsibility */
}


void valueview::DrawDehighlight()
     {
  /* subclass responsibility */
}


void valueview::DeactivationMask( class graphic  * map )
          {
  this->deactivationMask = map;
  if (!this->active)
    (this)->RequestUpdateFunction((updateq_fptr)::DrawFromScratch );
}


void valueview::SetDeactivationTransfer( short  mode )
          {
  this->deactivationTransferMode = mode;
  if (!this->active)
    (this)->RequestUpdateFunction((updateq_fptr)::DrawFromScratch );
}

void valueview::NewValue()
     {
  (this)->RequestUpdateFunction((updateq_fptr)DNV);
}

void valueview::BadValue()
     {
  (this)->RequestUpdateFunction((updateq_fptr)DBV);
}

void valueview::NoChange()
     {
}

void valueview::Changed(long status)
     {
  /* subclasses should override this if there observed dataobject
     will change with a status other than NOCHANGE, NEWVALUE, or BADVALUE */
}

void valueview::Destroyed()
     {
  this->dataobject = NULL;
  (this)->Deactivate();
}


class valueview * valueview::DoHit(enum view_MouseAction  type, long  x , long  y , long  numberOfClicks)
{
    /* this is a subclass responsability */
    return this;
}

boolean valueview::OnTarget(long  x,long  y)
{
    return (this->viewx <= x && this->viewx + this->viewwidth >= x &&
	     this->viewy <= y && this->viewy + this->viewheight >= y);
}

/* Highlight the value that's handling events */
/* Dehighlight it and stop sending movement events if the mouse cursor */
/* wanders off.  Dehighlight it on an up transition. */
class view * valueview::Hit(enum view_MouseAction  type, long  x , long  y , long  numberOfClicks)
               {
#if 0
  short sendEvent;
  if (this->active)
    {
      switch (type)
	{
	case view_NoMouseEvent:
	  sendEvent = False;
	  break;
	case view_LeftDown:
	  this->mouseState = valueview_LeftIsDown;
	  (this)->Highlight();
	  sendEvent = True;
	  break;
	case view_RightDown:
	  this->mouseState = valueview_RightIsDown;
	  (this)->Highlight();
	  sendEvent = True;
	  break;
	case view_LeftUp:
	case view_RightUp:
	  if (this->mouseIsOnTarget)
	    {
	      (this)->Dehighlight();
	      sendEvent = True;
	    }
	  else sendEvent = False;
	  break;
	case view_LeftMovement:
	case view_RightMovement:
	    if((this)->OnTarget(x,y))
	    {
	      (this)->Highlight();
	      sendEvent = True;
	    }
	  else
	    {
	      (this)->Dehighlight();
	      sendEvent = False;
	    }
	  break;
	}
    }
  else
    sendEvent = False;
  if (sendEvent)
    return (this)->DoHit( type, x, y, numberOfClicks);
  else
    return this;
#else /* 0 */
  if(this->HasInputFocus == FALSE)
      (this)->WantInputFocus(this);

  if (this->active)
    {
      switch (type)
      {
	  case view_NoMouseEvent:
	      return this;
	      /* break; */
	  case view_LeftDown:
	  case view_RightDown:
	      (this)->Highlight();
	      break;
	  case view_LeftUp:
	  case view_RightUp:
	      (this)->Dehighlight();
	      break;
	  default:
	      break;
      }

    return (this)->DoHit( type, x, y, numberOfClicks);
  }
  return this;
#endif /* 0 */
}


void valueview::Highlight( )
     {
  if (!this->mouseIsOnTarget)
    (this)->RequestUpdateFunction((updateq_fptr)DH);
  this->mouseIsOnTarget = True;
}


void valueview::Dehighlight( )
     {
  if (this->mouseIsOnTarget)
    (this)->RequestUpdateFunction((updateq_fptr)DDH);
  this->mouseIsOnTarget = False;
}



void valueview__GetCenter( class valueview  * self, long  * x , long  * y )
          {
  /* this controls the location of a label. */
  *x = self->x + (self->width >> 1);
  *y = self->y + (self->height >> 1);
}
void valueview::ReceiveInputFocus()
{
    (this)->PostMenus(NULL);
    (this)->PostKeyState(NULL);
    this->HasInputFocus = TRUE;
}
void valueview::LoseInputFocus()
{
    this->HasInputFocus = FALSE;
}
