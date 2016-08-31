/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

/*
 * newbuttonv.c
*/

/* ************************************************************
 *  Copyright (C) 1989, 1990, 1991
 *  by the Massachusetts Institute of Technology
 *   For full copyright information see:'mit-copyright.h'     *
 *  Adapted from ATK pushbuttonview by Nick Williams, July 1989
 *************************************************************/

#include <andrewos.h>
#include <mit-copyright.h>
#include <stdio.h>

#include <newbuttonview.H>
#include <pushbutton.H>
#include <andrewos.h>
#include <buffer.H>
#include <completion.H>
#include <cursor.H>
#include <environ.H>
#include <fontdesc.H>
#include <frame.H>
#include <graphic.H>
#include <im.H>
#include <menulist.H>
#include <message.H>
#include <observable.H>
#include <proctable.H>
#include <view.H>

#include "eos.h"

/* Defined constants and macros */
#define NO_MSG "Newbutton (v1)"
#define THREEDEE 2
#define ARBCON 3

#define PROMPTFONT "andy12b"
#define FONTTYPE fontdesc_Bold
#define FONTSIZE 12
#define BUTTONPRESSDEPTH 2
#define TEXTPAD 2

/* External Declarations */

/* Forward Declarations */

/* Global Variables */
static class atom *pushedtrigger;


ATKdefineRegistry(newbuttonview, view, newbuttonview::InitializeClass);
void db(class newbuttonview  * self,struct rectangle  *foo,struct rectangle  *fo);
static int RectEnclosesXY(struct rectangle  *r, long  x , long  y);
static void HighlightButton(class newbuttonview  *self);
static void UnhighlightButton(class newbuttonview  *self);


boolean newbuttonview::InitializeClass()
{
/* 
  Initialize all the class data.
*/

  pushedtrigger = atom::Intern("buttonpushed");
  return(pushedtrigger != NULL);
}


newbuttonview::newbuttonview()
{
	ATKinit;

/*
  Set up the data for each instance of the object.
*/
    this->lit = 0;

    if (!(this->cursor = cursor::Create(this))) THROWONFAILURE((FALSE));
    (this->cursor)->SetStandard( Cursor_Arrow);

    this->bfont = (char *) malloc(33);
    this->depth = environ::GetProfileInt("buttondepth", 4);
    strcpy(this->bfont, "helvetica");
    this->bfontsize = 12;
    this->enabled = TRUE;
    observable::DefineTrigger(this->ATKregistry(), pushedtrigger);

    THROWONFAILURE((TRUE));
}

newbuttonview::~newbuttonview()
{
	ATKinit;

  if (this->cursor) delete this->cursor;
  this->cursor = NULL;
  free(this->bfont);
  this->bfont = NULL;
  return;
}

/* The code below is from the value buttonview class */
void db(class newbuttonview  * self,struct rectangle  *foo,struct rectangle  *fo)
{
    (self)->DrawRect( fo);
    (self)->DrawRect(foo);   
    (self)->MoveTo(  fo->left, fo->top );
    (self)->DrawLineTo(  foo->left, foo->top);
    (self)->MoveTo(  fo->left + fo->width, fo->top );
    (self)->DrawLineTo(  foo->left + foo->width, foo->top);
    (self)->MoveTo(  fo->left , fo->top + fo->height);
    (self)->DrawLineTo(  foo->left, foo->top + foo->height);
    (self)->MoveTo(  fo->left + fo->width, fo->top + fo->height);
    (self)->DrawLineTo(  foo->left + foo->width, foo->top + foo->height);

}



void newbuttonview::FullUpdate(enum view_UpdateType  type, long  left , long  top , long  width , long  height)
{
/*
  Redisplay this object.  Specifically, set my font, and put my text label
  in the center of my display box.
*/

  struct rectangle Rect, Rect2, Rect3;
  class pushbutton *b = (class pushbutton *) (this)->GetDataObject();
  class fontdesc *my_fontdesc;
  class graphic *my_graphic;
  struct FontSummary *my_FontSummary;
  int r2_bot, r_bot;
  int tx = 0, ty = 0;
  short t_op;
  char text[128];
  int style;

  if (b) {
    style = (b)->GetStyle();
    (this)->GetLogicalBounds( &Rect);
    (this)->PostCursor( &Rect, this->cursor);
    my_graphic = (class graphic *)(this)->GetDrawable();
    if (!(my_fontdesc = (b)->GetButtonFont())) {
      my_fontdesc= fontdesc::Create(this->bfont, FONTTYPE, this->bfontsize);
    }
    if (my_fontdesc) {
      (this)->SetFont( my_fontdesc);
      my_FontSummary =  (my_fontdesc)->FontSummary( my_graphic);
    }

    (this)->SetTransferMode( graphic_SOURCE);

    t_op = graphic_BETWEENLEFTANDRIGHT | graphic_BETWEENTOPANDBASELINE;
    strcpy(text, (b)->GetText() ? (b)->GetText() : NO_MSG);
    if (!this->enabled) {
        char *temporary;
        for (temporary = text; *temporary != '\0'; *temporary++ = ' ');
    } 
      /* Rect2 is the inner (Text) region */
      Rect2.top    = Rect.top + this->depth;
      Rect2.left   = Rect.left + this->depth;
      Rect2.width  = Rect.width - 2*this->depth;
      Rect2.height = Rect.height - 2*this->depth;
      Rect3.top    = Rect2.top;
      Rect3.left   = Rect2.left + this->depth;
      Rect3.width  = Rect2.width - 2*this->depth;
      Rect3.height = Rect2.height - 2*this->depth;
      r2_bot = (Rect2.top)+(Rect2.height);
      r_bot = (Rect.top)+(Rect.height);
      tx = TEXTPAD + (Rect2.left + Rect2.width) / 2;

    switch (style) {
    case THREEDEE:
      ty = Rect2.top + Rect2.height/2;

      (this)->FillRectSize( Rect.left, Rect.top, this->depth, Rect.height, (this)->GrayPattern( 1, 4));	/* left bar */

      (this)->FillRectSize( Rect.left + Rect.width - this->depth, Rect.top, this->depth, Rect.height, (this)->GrayPattern( 3, 4)); /* right bar */

      (this)->FillTrapezoid( Rect2.left, r2_bot, Rect2.width, Rect.left, r_bot, Rect.width, (this)->GrayPattern( 3, 4)); /* lower trapz */

      (this)->FillTrapezoid( Rect.left, Rect.top, Rect.width, Rect2.left, Rect2.top, Rect2.width, (this)->GrayPattern( 1, 4)); /* upper trapz */

      (this)->FillRect( &Rect2, (this)->GrayPattern(1,2)); /* the middle box */

      (this)->SetTransferMode( graphic_WHITE);
      (this)->MoveTo( tx+1, ty);
      (this)->DrawString( text, t_op);
      (this)->MoveTo( tx, ty+1);
      (this)->DrawString( text, t_op);
      (this)->MoveTo( tx+1, ty+1);
      (this)->DrawString( text, t_op);
      (this)->SetTransferMode( graphic_BLACK);
      (this)->MoveTo( tx, ty);
      (this)->DrawString( text, t_op);
      break;
    case ARBCON:
      (this)->SetTransferMode( graphic_COPY);
      (this)->FillRect( &Rect, (this)->GrayPattern( 4, 16));
      (this)->SetTransferMode( graphic_WHITE);
      (this)->FillRect( &Rect2, 0);
      (this)->SetTransferMode( graphic_COPY);      
      db(this, &Rect3, &Rect2);
      (this)->MoveTo( Rect3.left + Rect3.width/2, Rect3.top + Rect3.height/2);
      (this)->DrawString( text, t_op);
      break;
    default:
      tx = (Rect.left + Rect.width) / 2;
      if (my_FontSummary)
	ty = (Rect.top + my_FontSummary->maxHeight - my_FontSummary->maxBelow);

      (this)->SetTransferMode( graphic_SOURCE);
      (this)->MoveTo( tx, ty);
      (this)->DrawString( text, t_op);
      break;
    } /* switch (style) */

  } /* if (b) */
}


void newbuttonview::Update()
{
/*
  Do an update.  Just set up the call to FullUpdate method.
*/
    struct rectangle r;

    (this)->EraseVisualRect();
    (this)->GetLogicalBounds( &r);
    (this)->FullUpdate(view_FullRedraw, r.left, r.top, r.width, r.height);
}


static int RectEnclosesXY(struct rectangle  *r, long  x , long  y)
{
  return(   ( ((r->top)  <= y) && ((r->top + r->height) >= y) )
	 && ( ((r->left) <= x) && ((r->left + r->width) >= x) )
	 );
}


static void HighlightButton(class newbuttonview  *self)
{
  class pushbutton *b = (class pushbutton *) (self)->GetDataObject();
  struct rectangle Rect, Rect2, Rect3;
  int style;
  class fontdesc *my_fontdesc;
  int tx, ty;
  short t_op;
  char *text;
  
  if (!(self->lit)) {
    style = (b)->GetStyle();
    (self)->GetLogicalBounds( &Rect);
    
    Rect2.top    = Rect.top + self->depth;
    Rect2.left   = Rect.left + self->depth;
    Rect2.width  = Rect.width - 2*self->depth;
    Rect2.height = Rect.height - 2*self->depth;
    Rect3.top    = Rect2.top;
    Rect3.left   = Rect2.left + self->depth;
    Rect3.height = Rect2.height - 2*self->depth;
    Rect3.width  = Rect2.width - 2*self->depth;
    if (!(my_fontdesc = (b)->GetButtonFont())) {
        my_fontdesc= fontdesc::Create(self->bfont, FONTTYPE, self->bfontsize);
    }
    text = (b)->GetText() ? (b)->GetText() : NO_MSG;
    if (my_fontdesc)
	(self)->SetFont( my_fontdesc);
    t_op = graphic_BETWEENLEFTANDRIGHT | graphic_BETWEENTOPANDBASELINE;
	  
    switch(style) {

    case THREEDEE:
      tx = TEXTPAD + (Rect2.left + Rect2.width) / 2;
      ty = Rect2.top + Rect2.height/2;
      (self)->SetTransferMode( graphic_WHITE);
      (self)->MoveTo( tx, ty);
      (self)->DrawString( text, t_op);
      break;

    case ARBCON:
      Rect3.top    = Rect2.top;
      Rect3.left   = Rect2.left + self->depth;
      Rect3.height = Rect2.height - 2*self->depth;
      Rect3.width  = Rect2.width - 2*self->depth;
      my_fontdesc = fontdesc::Create(self->bfont, fontdesc_Bold, self->bfontsize);
      (self)->SetFont( my_fontdesc);
      (self)->SetTransferMode( graphic_WHITE);
      (self)->FillRect( &Rect3, 0);
      (self)->SetTransferMode( graphic_COPY);      
      db(self, &Rect3, &Rect2);
      (self)->MoveTo( Rect3.left + Rect3.width/2, Rect3.top + Rect3.height/2);
      (self)->DrawString( text, t_op);
      break;
      
    default:
      (self)->SetTransferMode( graphic_INVERT);
      (self)->FillRect(&Rect,(self)->BlackPattern());
      break;
    }
  }
  self->lit = 1;
}


static void UnhighlightButton(class newbuttonview  *self)
{
  class pushbutton *b = (class pushbutton *) (self)->GetDataObject();
  struct rectangle Rect, Rect2, Rect3;
  int style;
  class fontdesc *my_fontdesc;
  int tx, ty;
  short t_op;
  char *text;
  
  if (self->lit) {
    style = (b)->GetStyle();
    (self)->GetLogicalBounds( &Rect);
    text = (b)->GetText() ? (b)->GetText() : NO_MSG;
    /* Rect2 is the inner (Text) region */
    Rect2.top = Rect.top + self->depth;
    Rect2.left = Rect.left + self->depth;
    Rect2.width = Rect.width - 2*self->depth;
    Rect2.height = Rect.height - 2*self->depth;
    if (!(my_fontdesc = (b)->GetButtonFont())) {
	my_fontdesc= fontdesc::Create(self->bfont, FONTTYPE, self->bfontsize);
    }
    if (my_fontdesc)
	(self)->SetFont( my_fontdesc);
    t_op = graphic_BETWEENLEFTANDRIGHT | graphic_BETWEENTOPANDBASELINE;
   
    switch(style) {
    case THREEDEE:
      tx = TEXTPAD + (Rect2.left + Rect2.width) / 2;
      ty = Rect2.top + Rect2.height/2;
      (self)->SetTransferMode( graphic_BLACK);
      (self)->MoveTo( tx, ty);
      (self)->DrawString( text, t_op);
      break;

    case ARBCON:
      Rect3.top    = Rect2.top;
      Rect3.left   = Rect2.left + self->depth;
      Rect3.height = Rect2.height - 2*self->depth;
      Rect3.width  = Rect2.width - 2*self->depth;
      my_fontdesc = fontdesc::Create(self->bfont, fontdesc_Plain, self->bfontsize);
      (self)->SetFont( my_fontdesc);
      (self)->SetTransferMode( graphic_WHITE);
      (self)->FillRect( &Rect3, 0);
      (self)->SetTransferMode( graphic_COPY);      
      db(self, &Rect3, &Rect2);
      (self)->MoveTo( Rect3.left + Rect3.width/2, Rect3.top + Rect3.height/2);
      (self)->DrawString( text, t_op);
      break;
      
    default:
      (self)->SetTransferMode( graphic_INVERT);
      (self)->FillRect(&Rect,(self)->BlackPattern());
      break;
    }
  }
  self->lit = 0;
}


class view *newbuttonview::Hit(enum view_MouseAction  action, long  x , long  y, long  numclicks  )
{
/*
  Handle the button event.  Currently, semantics are:
    
    Left Down  -- Draw button pressed
    Right Down -- select button (Receive input focus, for menuing without activating)
    Left Up    -- draw button at rest, pull trigger
    Right Up   -- No Op
    Left Movement     -- unhighlight if moved off, highlight if moved on
    Right Movement -- No Op
*/
  class cursor *wait_cursor;
  if (!this->enabled) return (class view *) this;

  switch (action) {
  case view_LeftDown: 
    HighlightButton(this);
    (this)->WantInputFocus(this);
    break;
  case view_LeftMovement:
    {
      struct rectangle r;

      (this)->GetVisualBounds( &r);
      if (RectEnclosesXY(&r, x, y))
	HighlightButton(this);
      else
	UnhighlightButton(this);
    }
    break;
  case view_LeftUp:
    {
      short litp = this->lit;

      UnhighlightButton(this);
      if (litp) {
	if (wait_cursor = cursor::Create(this)) {
	  (wait_cursor)->SetStandard( Cursor_Wait);
	  im::SetProcessCursor(wait_cursor);
	  (this)->PullTrigger( pushedtrigger);
	  im::SetProcessCursor(NULL);
	  delete wait_cursor;
	}
      }
    }
    break;
  case view_RightDown:
    (this)->WantInputFocus( this);
    break;
  }
  return((class view *)this);
}


enum view_DSattributes newbuttonview::DesiredSize(long  width, long  height, enum view_DSpass  pass, long  *desired_width, long  *desired_height)
{
/* 
  Tell parent that this object  wants to be as big as the box around its
  text string.  For some reason IM allows resizing of this object. (BUG)
*/

  class fontdesc *my_fontdesc;
  struct FontSummary *my_FontSummary;
  class graphic *my_graphic;
  class pushbutton *b = (class pushbutton *) (this)->GetDataObject();
  int style;

  style = (b)->GetStyle();

  my_graphic = (class graphic *)(this)->GetDrawable();
  if (!(my_fontdesc = (b)->GetButtonFont())) {
    my_fontdesc= fontdesc::Create(this->bfont, FONTTYPE, this->bfontsize);
  }
  if (my_fontdesc) {
    (my_fontdesc)->StringSize( my_graphic, (b)->GetText() ? (b)->GetText() : NO_MSG, desired_width, desired_height);
    my_FontSummary =  (my_fontdesc)->FontSummary( my_graphic);
  }

  switch (style) {
  case THREEDEE:
    *desired_width = *desired_width + 2*TEXTPAD + 2*this->depth;
    if (my_FontSummary)
      *desired_height = my_FontSummary->maxHeight + 2*TEXTPAD + 2*this->depth;
    break;
  default:
    if (my_FontSummary)
      *desired_height = my_FontSummary->maxHeight;
    break;
  }

/*
  (BUG) I don't check to see if I can specify a size, I just do it.
  Will this break things?  What if I can't change my size?  Will I be
  Ugly?  What to do, what to do....
*/

  return(view_Fixed); /* (BUG) should disable user sizing, but this doesn't */
}


void newbuttonview::GetOrigin(long  width , long  height, long  *originX , long  *originY)
{
/*
  We want this object to sit in-line with text, not below the baseline.
  Simply, we could negate the height as the originX, but then our
  text would be too high.  So, instead, we use the height of
  our font above the baseline
*/

  struct FontSummary *my_FontSummary;
  class fontdesc *my_fontdesc;
  class graphic *my_graphic;
  class pushbutton *b = (class pushbutton *) (this)->GetDataObject();
  int style;

  style = (b)->GetStyle();

  my_graphic = (class graphic *)(this)->GetDrawable();
  if (!(my_fontdesc = (b)->GetButtonFont())) {
    my_fontdesc= fontdesc::Create(this->bfont, FONTTYPE, this->bfontsize);
  }
  if (my_fontdesc) {
      my_FontSummary =  (my_fontdesc)->FontSummary( my_graphic);
  }

  *originX = 0;
  switch(style) {
  case THREEDEE:
    if (my_FontSummary) 
      *originY = (my_FontSummary->maxHeight) - (my_FontSummary->maxBelow) + 1 + TEXTPAD + this->depth;
    break;
  default:
    if (my_FontSummary)
      *originY = (my_FontSummary->maxHeight) - (my_FontSummary->maxBelow) + 1;
    break;
  }
  return;
}

void newbuttonview::Enable(boolean  want_to_enable)
{
    class pushbutton *b;
    if (this->enabled == want_to_enable) return;

    this->enabled = want_to_enable;
    b = (class pushbutton *) (this)->GetDataObject();

    if (want_to_enable)
        (b)->EnableTrigger( pushedtrigger);
    else
        (b)->DisableTrigger( pushedtrigger);

    (this)->WantUpdate( this);
}

boolean newbuttonview::IsEnabled()
{
    return this->enabled;
}
