/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include <timeodayview.H>
#include <timeoday.H>
#include <graphic.H>
#include <message.H>
#include <observable.H>
#include <view.H>
#include <fontdesc.H>
#include <proctable.H>
#include <util.h>

/* Defined constants and macros */
#define FUDGEFACTOR 1.1

/* External Declarations */

/* Forward Declarations */

/* Global Variables */
static class menulist *timeodayview_menulist = NULL;

static const char * const formats[] = {
  "Default~1", NULL,
  "H:MM AM/PM~10", "%u:%M %P",
  "Month DD YYYY~11", "%o %A, %Y",
  NULL, NULL};



ATKdefineRegistry(timeodayview, view, timeodayview::InitializeClass);
static void MenuSetFormat(class timeodayview  *self, char  *format);
static void Redraw(class timeodayview  *self);


static void MenuSetFormat(class timeodayview  *self, char  *format)
          {
  class timeoday *b = (class timeoday *) (self)->GetDataObject();

  (b)->SetFormat( format);
  (b)->SetModified();
}


boolean
timeodayview::InitializeClass()
{
/* 
  Initialize all the class data.
*/
  int i;
  char *menuname;
  char temp[250];
  struct proctable_Entry *proc = NULL;
  char menutitlefmt[200];
  static char procname[200];

  timeodayview_menulist = new menulist;
  
  sprintf(procname, "%s-set-format", "timeoday");
  proc = proctable::DefineProc(procname, (proctable_fptr)MenuSetFormat, &timeodayview_ATKregistry_ , NULL, "Set the timeoday inset's format.");
  
  sprintf(menutitlefmt, "%s,%%s", "Time O'Day");
  
  for (i=0; formats[i]; i+=2) {
    sprintf(temp, menutitlefmt, formats[i]);
    menuname = NewString(temp);
    (timeodayview_menulist)->AddToML( menuname, proc, (long)formats[i+1], 0);
  }
  
  return(TRUE);
}


timeodayview::timeodayview()
{
	ATKinit;

/*
  Set up the data for each instance of the object.
*/
  this->ml = (timeodayview_menulist)->DuplicateML( this);

  if (!(this->cursor = cursor::Create(this))) THROWONFAILURE((FALSE));
  (this->cursor)->SetStandard( Cursor_Gunsight);

  THROWONFAILURE((TRUE));
}


timeodayview::~timeodayview()
{
	ATKinit;

  if (this->cursor) delete this->cursor;
  this->cursor = NULL;
  if (this->ml) delete this->ml;
  this->ml = NULL;
  return;
}



static void
Redraw(class timeodayview  *self)
{
/*
  Redisplay this object.
*/
  class timeoday *b = (class timeoday *) (self)->GetDataObject();
  struct rectangle rect;
  char *tod;
  struct FontSummary *my_FontSummary = NULL;
  class fontdesc *my_fontdesc;
  class graphic *my_graphic;
  long new_width, new_height;
  
  (self)->EraseVisualRect();
  (self)->GetLogicalBounds( &rect);
  tod = (b)->GetTod();
  my_graphic = (class graphic *)(self)->GetDrawable();
  my_fontdesc= (b)->GetFont();
  if (my_fontdesc) {
    (my_fontdesc)->StringSize( my_graphic, (b)->GetTod(), &new_width, &new_height);
    my_FontSummary =  (my_fontdesc)->FontSummary( my_graphic);
    if (my_FontSummary) new_height = my_FontSummary->maxHeight;
    if ((new_width > self->last_width) || (new_height > self->last_height)
	|| (new_width < self->last_width/FUDGEFACTOR) || (new_height < self->last_height/FUDGEFACTOR)) {
      (self)->WantNewSize(self);
    }
  }
  if (my_FontSummary) (self)->MoveTo( rect.left + rect.width/2, rect.top + my_FontSummary->maxHeight - my_FontSummary->maxBelow);

  if (my_fontdesc) (self)->SetFont( my_fontdesc);
  (self)->DrawString( tod,  graphic_BETWEENLEFTANDRIGHT | graphic_ATBASELINE);
}


void
timeodayview::FullUpdate(enum view_UpdateType  type, long  left , long  top , long  width , long  height)
{
/*
  Do an update.
*/
  if ((type == view_FullRedraw) || (type == view_LastPartialRedraw)) {
    this->need_full_update = TRUE;
    Redraw(this);
  }
}


void
timeodayview::Update()
{
  Redraw(this);
}


class view *
timeodayview::Hit(enum view_MouseAction  action, long  x , long  y, long  numclicks  )
{
/*
  Handle the button event.  Currently, semantics are:
*/
  (this)->WantInputFocus( this);
  return((class view *)this);
}


void
timeodayview::Print(FILE  *file, const char  *processor , const char  *finalFormat, boolean  topLevel)
                    {
    char *time_str;
    class timeoday *time_do =  (class timeoday *) (this)->GetDataObject();

    time_str = (time_do)->GetTod();

    if (time_str == NULL) {
	fprintf(stderr, "?timeodayview_Print:  there doesn't appear to be any formatted time string.\n");
	fprintf(file, "\n");
    } else {
	fprintf(file, "%s\n", time_str);
    }
}


view_DSattributes 
timeodayview::DesiredSize(long  width, long  height, enum view_DSpass  pass, long  *desired_width, long  *desired_height)
{
/* 
  Tell parent that this object  wants to be as big as the box around its
  text string.  For some reason IM allows resizing of this object. (BUG)
*/

  class fontdesc *my_fontdesc;
  class graphic *my_graphic;
  struct FontSummary *my_FontSummary = NULL;
  class timeoday *b = (class timeoday *) (this)->GetDataObject();

  my_graphic = (class graphic *)(this)->GetDrawable();
  my_fontdesc= (b)->GetFont();
  if (my_fontdesc) {
    (my_fontdesc)->StringSize( my_graphic, (b)->GetTod(), desired_width, desired_height);
    *desired_width = (long)(FUDGEFACTOR*(*desired_width));
    this->last_width = *desired_width;
    my_FontSummary =  (my_fontdesc)->FontSummary( my_graphic);
  }
  if (my_FontSummary)
    *desired_height = my_FontSummary->maxHeight;
  this->last_height = *desired_height;

  return(view_Fixed); /* (BUG) should disable user sizing, but this doesn't */
}


void
timeodayview::GetOrigin(long  width , long  height, long  *originX , long  *originY)
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
  class timeoday *b = (class timeoday *) (this)->GetDataObject();

  my_graphic = (class graphic *)(this)->GetDrawable();
  my_fontdesc= (b)->GetFont();
  if (my_fontdesc) {
    my_FontSummary =  (my_fontdesc)->FontSummary( my_graphic);
  }

  *originX = 0;
  if (my_FontSummary)
    *originY = (my_FontSummary->maxHeight) - (my_FontSummary->maxBelow) + 1;
  return;
}


void
timeodayview::PostMenus(class menulist  *ml)
{
/*
  Enable the menus for this object.
*/

  (this->ml)->ClearChain();
  if (ml) (this->ml)->ChainAfterML( ml, (long)ml);
  (this)->view::PostMenus( this->ml);
}

