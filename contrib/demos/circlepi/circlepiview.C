/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include <math.h>

#include <circlepiview.H>
#include <circlepi.H>
#include <cursor.H>
#include <graphic.H>
#include <menulist.H>
#include <message.H>
#include <observable.H>
#include <proctable.H>
#include <view.H>

/* Defined constants and macros */

/* External Declarations */

/* Forward Declarations */


/* Global Variables */
static class menulist *vmenulist = NULL;



ATKdefineRegistry(circlepiview, view, circlepiview::InitializeClass);
static int inside_circle(double  x,double  y);
static void step(class circlepiview  *self  , double  x , double  y , double  side, double  *inside , double  *outside, int  depth , int  depth_limit);
static void LimitProc(class circlepiview  *self, long  param);


boolean
circlepiview::InitializeClass()
{
/* 
  Initialize all the class data.
*/
    struct proctable_Entry *proc = NULL;
    char menubuf[80];
    int i;

    if ((vmenulist = new menulist) == NULL) return(FALSE);

    if ((proc = proctable::DefineProc("circpi-set-depth-limit", (proctable_fptr)LimitProc, &circlepiview_ATKregistry_ , NULL, "Takes the integer rock and uses it to set the depth limit on recursion.")) == NULL) return(FALSE);
    for (i = 1; i <= 10; ++i) {
	sprintf(menubuf, "Depth Limit~10,%d~%d", i, i);
	(vmenulist)->AddToML( menubuf, proc, (long)i, 0);
    }
    
    return(TRUE);
}


circlepiview::circlepiview()
{
	ATKinit;

/*
  Set up the data for each instance of the object.
*/
  if (!(this->cursor = cursor::Create(this))) THROWONFAILURE((FALSE));
  (this->cursor)->SetStandard( Cursor_Gunsight);
  this->color = 0;		/* assume monochrome, for now */
  this->ml = (vmenulist)->DuplicateML( this);

  THROWONFAILURE((TRUE));
}


circlepiview::~circlepiview()
{
	ATKinit;

  if (this->cursor) delete this->cursor;
  this->cursor = NULL;
  return;
}


void
circlepiview::ObservedChanged(class observable  *b, long  v)
               {
    (this)->view::ObservedChanged( b, v);
    if (v == observable::OBJECTCHANGED) {
	this->need_full_update = TRUE;
	(this)->WantUpdate( this);
    }
}


void
circlepiview::PostMenus(class menulist  *ml)
          {
    /*
      Enable the menus for this object.
      */

    (this->ml)->ClearChain();
    if (ml) (this->ml)->ChainAfterML( ml, (long)ml);
    (this)->view::PostMenus( this->ml);
}


void
circlepiview::LinkTree(class view  *parent)
          {
    (this)->view::LinkTree( parent);

    if ((this)->GetIM() != NULL) {
	if ((this)->DisplayClass() & graphic::Color) {
	    this->color = 1;
	} else {
	    this->color = 0;
	}
    }
}


void
circlepiview::FullUpdate(enum view::UpdateType  type, long  left , long  top , long  width , long  height)
{
/*
  Do an update.
*/

  if ((type == view::FullRedraw) || (type == view::LastPartialRedraw)) {
    this->need_full_update = TRUE;
    (this)->WantUpdate( this);
  }
}

static int inside_circle(double  x,double  y)
     {
  return((x*x + y*y) <= 1.0);
}



static void step(class circlepiview  *self  , double  x , double  y , double  side, double  *inside , double  *outside, int  depth , int  depth_limit)
                    {
  struct rectangle vbounds, fill;
  class graphic *tile;

  (self)->GetVisualBounds( &vbounds);
  if (depth < depth_limit) {
    if (inside_circle(x, y)) {
      *inside += (side*side)/(double)(4.0);
      if (self->color) {
	  (self)->SetFGColor( 0.0, 0.0, 1.0);
	  (self)->SetTransferMode( graphic::COPY);
	  tile = NULL;
      } else {
	  tile = (self)->GrayPattern( 2, 2);
      }

      fill.left = vbounds.left + (long)((x-side/(double)(2.0))*(double)(vbounds.width) + (double)(0.5));
      fill.top = vbounds.top + (long)((y-side/(double)(2.0))*(double)(vbounds.height) + (double)(0.5));
      fill.width = (long)((double)(vbounds.width) * side/(double)(2.0) + (double)(0.5));
      fill.height = (long)((double)(vbounds.height) * side/(double)(2.0)+ (double)(0.5));
      (self)->FillRect( &fill, tile);

      /* recurse on NE quadrant */
      step(self, x+side/(double)(4.0), y+side/(double)(4.0), side/(double)(2.0), inside, outside, depth+1, depth_limit);
    } else {
      *outside += (side*side)/(double)(4.0);
      if (self->color) {
	  (self)->SetFGColor( 1.0, 0.0, 0.0);
	  (self)->SetTransferMode( graphic::COPY);
	  tile = NULL;
      } else {
	  tile = (self)->GrayPattern( 0, 2);
      }
      fill.left = vbounds.left + (long)(x*(double)(vbounds.width) + (double)(0.5));
      fill.top = vbounds.top + (long)(y*(double)(vbounds.height) + (double)(0.5));
      fill.width = (long)((double)(vbounds.width) * side/(double)(2.0) + (double)(0.5));
      fill.height = (long)((double)(vbounds.height) * side/(double)(2.0)+ (double)(0.5));
      (self)->FillRect( &fill, tile);

      /* recurse on SW quadrant */
      step(self, x-side/(double)(4.0), y-side/(double)(4.0), side/(double)(2.0), inside, outside, depth+1, depth_limit);
    }
    step(self, x-side/(double)(4.0), y+side/(double)(4.0), side/(double)(2.0), inside, outside, depth+1, depth_limit);
    step(self, x+side/(double)(4.0), y-side/(double)(4.0), side/(double)(2.0), inside, outside, depth+1, depth_limit);
    
  } /* if (depth < depth_limit) */

  return;
}


void
circlepiview::Update()
{
/*
  Redisplay this object.
*/
  class circlepi *b = (class circlepi *) (this)->GetDataObject();
  class graphic *tile;
  struct rectangle vbounds;
  double inside = 0.0, outside = 0.0;
  char msg[80];

  if (this->need_full_update) {
    (this)->EraseVisualRect();
    if (this->color) {
	(this)->SetFGColor( 0.0, 1.0, 0.0);
	(this)->SetTransferMode( graphic::COPY);
	tile = NULL;
    } else {
	tile = (this)->GrayPattern( 1, 2);
    }
    (this)->GetVisualBounds( &vbounds);
    (this)->FillRect( &vbounds, tile);

    step(this, 0.5, 0.5, 1.0, &inside, &outside, 0, (b)->GetDepth());

    sprintf(msg, "pi, inside:  %f;  outside:  %f;  unknown:  %3f%%.",
	    inside * 4.0,
	    (1.0 - outside) * 4.0,
	    (1.0 - (inside + outside))/1.0*100.0);
    message::DisplayString(this, 1, msg);

    this->need_full_update = FALSE;
  } else {
				/* compute delta */
  }
}



class view *
circlepiview::Hit(enum view::MouseAction  action, long  x , long  y, long  numclicks  )
{
/*
  Handle the button event.  Currently, semantics are:
  left up  --  receive input focus (to post menus)
*/
    switch (action) {
      case view::LeftUp:
	(this)->WantInputFocus(this);
	break;
      default:
	break;
    }
    return((class view *)this);
}


static void
LimitProc(class circlepiview  *self, long  param)
          {
    class circlepi *c = (class circlepi *)(self)->GetDataObject();

    (c)->SetDepth( param);
}
