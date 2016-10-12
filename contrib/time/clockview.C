/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#define clock hidden_clock
#include <math.h>
#undef clock

#include <clockview.H>
#include <graphic.H>
#include <observable.H>
#include <proctable.H>
#include <view.H>
#include <util.h>

#undef MIN
/* Defined constants and macros */
#define MENUTITLE "Clock %s,%s"
#define HOURSTORADIANS(x) ((15.0-(x))/6.0*M_PI)
#define MINUTESTORADIANS(x) ((75.0-(x))/30.0*M_PI)
#define MIN(a,b) ((a)<(b)?(a):(b))

/* External Declarations */

/* Forward Declarations */

/* Global Variables */
static class menulist *clockview_menulist = NULL;
static const char * const label_set1[3][1] = {{"12"}, {"XII"}, {"Twelve"}};
static const char * const label_set4[3][4] = {{"12", "3", "6", "9"},
				   {"XII", "III", "VI", "IX"},
				   {"Twelve", "Three", "Six", "Nine"}};
static const char * const label_set12[3][12] = {{"12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11"},
				   {"XII", "I", "II", "III", "IIII", "V", "VI", "VII", "VIII", "IX", "X", "XI"},
				   {"Twelve", "One", "Two",
				      "Three", "Four", "Five",
				      "Six", "Seven", "Eight",
				      "Nine", "Ten", "Eleven"}};



ATKdefineRegistry(clockview, view, clockview::InitializeClass);
static void MenuSetShape(class clockview  *self, char  *format);
static void MenuSetLabels(class clockview  *self, char  *format);
static void MenuSetTicks(class clockview  *self, char  *format);
static void MenuSetSeconds(class clockview  *self, char  *format);
static void PlotLabels(class clockview  *self, double  theta, int  radius, const char  *label, enum border_shapes  shape);
static void PlotPoints(class clockview  *self, double  theta, int  radius , int  thickness, enum border_shapes  shape);
static void Redraw(class clockview  *self);


static void MenuSetShape(class clockview  *self, char  *format)
          {
  class clock *b = (class clock *) (self)->GetDataObject();
  struct clock_options *options;

  options = (b)->GetOptions();
  switch (format[0]) {
  case 'A':
    options->border_shape = circle;
    break;
  case 'B':
    options->border_shape = square;
    break;
  case '1':
    options->border_width = 0;
    break;
  case '2':
    options->border_width = 1;
    break;
  case '3':
    options->border_width = 2;
    break;
  }
  (b)->SetOptions( options);
}


static void MenuSetLabels(class clockview  *self, char  *format)
          {
  class clock *b = (class clock *) (self)->GetDataObject();
  struct clock_options *options;
  int style;

  options = (b)->GetOptions();
  if (options->num_labels > 0) {
    switch (options->labels[0][0]) {
    case '1':
      style = 0;
      break;
    case 'X':
      style = 1;
      break;
    case 'T':
      style = 2;
      break;
    default:
      style = 0;
      break;
    }
  } else {
    style = 0;
  }

  switch (format[0]) {
  case 'A':
    options->num_labels = 0;
    options->labels = NULL;
    break;
  case 'B':
    options->num_labels = 1;
    options->labels = label_set1[style];
    break;
  case 'C':
    options->num_labels = 4;
    options->labels = label_set4[style];
    break;
  case 'D':
    options->num_labels = 12;
    options->labels = label_set12[style];
    break;
  case '1':
  case '2':
  case '3':
    style = (int)(format[0] - '1');
    switch(options->num_labels) {
    case 1:
      options->labels = label_set1[style];
      break;
    case 4:
      options->labels = label_set4[style];
      break;
    default:
      options->labels = label_set12[style];
      options->num_labels = 12;
      break;
    }
  }
  (b)->SetOptions( options);
}


static void MenuSetTicks(class clockview  *self, char  *format)
          {
  class clock *b = (class clock *) (self)->GetDataObject();
  struct clock_options *options;

  options = (b)->GetOptions();
  switch (format[0]) {
  case 'A':
    options->major_ticks = 0;
    options->minor_ticks = 0;
    break;
  case 'B':
    options->major_ticks = 1;
    options->minor_ticks = 4;
    break;
  case 'C':
    options->major_ticks = 4;
    options->minor_ticks = 12;
    break;
  case 'D':
    options->major_ticks = 12;
    options->minor_ticks = 60;
    break;
  }
  (b)->SetOptions( options);
}


static void MenuSetSeconds(class clockview  *self, char  *format)
          {
  class clock *b = (class clock *) (self)->GetDataObject();
  struct clock_options *options;

  options = (b)->GetOptions();
  switch (format[0]) {
  case 'A':
    options->seconds_length = 0;
    break;
  case 'B':
    options->seconds_length = 100;
    break;
  case 'C':
    options->seconds_length = -20;
    break;
  }
  (b)->SetOptions( options);
}


boolean
clockview::InitializeClass()
{
/* 
  Initialize all the class data.
*/
  struct proctable_Entry *proc = NULL;
  char menuname[255];

  clockview_menulist = new menulist;

  proc = proctable::DefineProc("clock-set-num-labels", (proctable_fptr)MenuSetShape, &clockview_ATKregistry_ , NULL, "Set the clock inset's shape parameters.");
  sprintf(menuname, MENUTITLE, "Shape~10", "Circle~1");
  (clockview_menulist)->AddToML( menuname, proc, (long)"A", 0);
  sprintf(menuname, MENUTITLE, "Shape~10", "Square~2");
  (clockview_menulist)->AddToML( menuname, proc, (long)"B", 0);
  sprintf(menuname, MENUTITLE, "Shape~10", "No border~11");
  (clockview_menulist)->AddToML( menuname, proc, (long)"1", 0);
  sprintf(menuname, MENUTITLE, "Shape~10", "Thin border~12");
  (clockview_menulist)->AddToML( menuname, proc, (long)"2", 0);
  sprintf(menuname, MENUTITLE, "Shape~10", "Thick border~13");
  (clockview_menulist)->AddToML( menuname, proc, (long)"3", 0);

  proc = proctable::DefineProc("clock-set-labels", (proctable_fptr)MenuSetLabels, &clockview_ATKregistry_ , NULL, "Set the clock inset's label parameters.");
  sprintf(menuname, MENUTITLE, "Labels~11", "None~1");
  (clockview_menulist)->AddToML( menuname, proc, (long)"A", 0);
  sprintf(menuname, MENUTITLE, "Labels~11", "1  (12)~2");
  (clockview_menulist)->AddToML( menuname, proc, (long)"B", 0);
  sprintf(menuname, MENUTITLE, "Labels~11", "4 (12  3  6  9)~3");
  (clockview_menulist)->AddToML( menuname, proc, (long)"C", 0);
  sprintf(menuname, MENUTITLE, "Labels~11", "12 (12  1  2 ...)~4");
  (clockview_menulist)->AddToML( menuname, proc, (long)"D", 0);

  sprintf(menuname, MENUTITLE, "Labels~11", "Arabic (12)~11");
  (clockview_menulist)->AddToML( menuname, proc, (long)"1", 0);
  sprintf(menuname, MENUTITLE, "Labels~11", "Roman (XII)~12");
  (clockview_menulist)->AddToML( menuname, proc, (long)"2", 0);
  sprintf(menuname, MENUTITLE, "Labels~11", "English (Twelve)~13");
  (clockview_menulist)->AddToML( menuname, proc, (long)"3", 0);

  proc = proctable::DefineProc("clock-set-ticks", (proctable_fptr)MenuSetTicks, &clockview_ATKregistry_ , NULL, "Set the clock inset's tick count parameters.");
  sprintf(menuname, MENUTITLE, "Ticks~12", "None~1");
  (clockview_menulist)->AddToML( menuname, proc, (long)"A", 0);
  sprintf(menuname, MENUTITLE, "Ticks~12", "1 / 4~2");
  (clockview_menulist)->AddToML( menuname, proc, (long)"B", 0);
  sprintf(menuname, MENUTITLE, "Ticks~12", "4 / 12~3");
  (clockview_menulist)->AddToML( menuname, proc, (long)"C", 0);
  sprintf(menuname, MENUTITLE, "Ticks~12", "12 / 60~4");
  (clockview_menulist)->AddToML( menuname, proc, (long)"D", 0);

  proc = proctable::DefineProc("clock-set-seconds", (proctable_fptr)MenuSetSeconds, &clockview_ATKregistry_ , NULL, "Set the clock inset's seconds hand parameters.");
  sprintf(menuname, MENUTITLE, "Seconds~13", "No Second Hand~1");
  (clockview_menulist)->AddToML( menuname, proc, (long)"A", 0);
  sprintf(menuname, MENUTITLE, "Seconds~13", "Radial Hand~2");
  (clockview_menulist)->AddToML( menuname, proc, (long)"B", 0);
  sprintf(menuname, MENUTITLE, "Seconds~13", "Floating Tick~3");
  (clockview_menulist)->AddToML( menuname, proc, (long)"C", 0);

  return(TRUE);
}


clockview::clockview()
{
	ATKinit;

/*
  Set up the data for each instance of the object.
*/
  this->need_full_update = TRUE;
  if (!(this->cursor = cursor::Create(this))) THROWONFAILURE((FALSE));
  (this->cursor)->SetStandard( Cursor_Gunsight);
  this->ml = (clockview_menulist)->DuplicateML( this);
  this->last_options_timestamp = 0;

  THROWONFAILURE((TRUE));
}


clockview::~clockview()
{
	ATKinit;

  if (this->cursor) delete this->cursor;
  this->cursor = NULL;
  if (this->ml) delete this->ml;
  this->ml = NULL;
  return;
}



static void
PlotLabels(class clockview  *self, double  theta, int  radius, const char  *label, enum border_shapes  shape)
{
  struct rectangle rect;
  long max_radius;
  long x0, y0, x1, y1, x2, y2;

  if (radius == 0) return;

  (self)->GetLogicalBounds( &rect);
  max_radius = MIN(rect.width-1, rect.height-1)/2;

  /* center */
  x0 = (rect.left + rect.width - 1)/2;
  y0 = (rect.top + rect.height - 1)/2;
  if (radius>0) {
    /* normal hand, from center */
    x2 = (long)((x0) + (max_radius*((double)radius/100.0*cos(theta))));
    y2 = (long)((y0) - (max_radius*((double)radius/100.0*sin(theta))));
    (self)->MoveTo( x0, y0);
  } else {
    /* hand from edge */
    if (shape == circle) {
      x1 = (long)((x0) + (max_radius*((1.0 + (double)radius/100.0)*cos(theta))));
      y1 = (long)((y0) - (max_radius*((1.0 + (double)radius/100.0)*sin(theta))));
      (self)->MoveTo( x1, y1);
    } else {
      /* Canonicalize theta */
      while (theta>2*M_PI) theta = theta-2*M_PI;
      if (theta<0) theta = theta+2*M_PI;

      if ((theta < M_PI/4.0) || (theta >= 7.0*M_PI/4.0)) {
	x1 = max_radius;
	y1 = (long)(max_radius*tan(theta));
      } else if ((theta >= M_PI/4.0) && (theta < 3.0*M_PI/4.0)) {
	x1 = (long)(-max_radius*tan(theta-M_PI/2.0));
	y1 = max_radius;
      } else if ((theta >= 3.0*M_PI/4.0) && (theta < 5.0*M_PI/4.0)) {
	x1 = -max_radius;
	y1 = (long)(-max_radius*tan(theta-M_PI));
      } else {
	y1 = -max_radius;
	x1 = (long)(max_radius*tan(theta-3.0*M_PI/2.0));
      }
      x1 = x0 + x1;
      y1 = y0 - y1;
      x2 = (long)((x1) + (max_radius*((double)radius/-100.0*cos(theta+M_PI))));
      y2 = (long)((y1) - (max_radius*((double)radius/-100.0*sin(theta+M_PI))));
      (self)->MoveTo( x2, y2);
    }
  }
  (self)->DrawString( label,  graphic::BETWEENLEFTANDRIGHT | graphic::BETWEENTOPANDBOTTOM);

  return;
}


static void
PlotPoints(class clockview  *self, double  theta, int  radius , int  thickness, enum border_shapes  shape)
{
  struct rectangle rect;
  long max_radius;
  long x0, y0, x1, y1, x2, y2;
  
  if (radius == 0) return;

  (self)->GetLogicalBounds( &rect);
  max_radius = MIN(rect.width, rect.height)/2;

  /* center */
  x0 = (rect.left + rect.width)/2;
  y0 = (rect.top + rect.height)/2;
  (self)->SetLineWidth( thickness);
  if (radius>0) {
    /* normal hand, from center */
    x2 = (long)((x0) + (max_radius*((double)radius/100.0*cos(theta))));
    y2 = (long)((y0) - (max_radius*((double)radius/100.0*sin(theta))));
    (self)->MoveTo( x0, y0);
  } else {
    /* hand from edge */
    if (shape == circle) {
      x1 = (long)((x0) + (max_radius*((1.0 + (double)radius/100.0)*cos(theta))));
      y1 = (long)((y0) - (max_radius*((1.0 + (double)radius/100.0)*sin(theta))));
      x2 = (long)((x0) + (max_radius*cos(theta)));
      y2 = (long)((y0) - (max_radius*sin(theta)));
    } else {
      /* Canonicalize theta */
      while (theta>2*M_PI) theta = theta-2*M_PI;
      if (theta<0) theta = theta+2*M_PI;

      if ((theta < M_PI/4.0) || (theta >= 7.0*M_PI/4.0)) {
	x1 = max_radius;
	y1 = (long)(max_radius*tan(theta));
      } else if ((theta >= M_PI/4.0) && (theta < 3.0*M_PI/4.0)) {
	x1 = (long)(-max_radius*tan(theta-M_PI/2.0));
	y1 = max_radius;
      } else if ((theta >= 3.0*M_PI/4.0) && (theta < 5.0*M_PI/4.0)) {
	x1 = -max_radius;
	y1 = (long)(-max_radius*tan(theta-M_PI));
      } else {
	y1 = -max_radius;
	x1 = (long)(max_radius*tan(theta-3.0*M_PI/2.0));
      }
      x1 = x0 + x1;
      y1 = y0 - y1;
      x2 = (long)((x1) + (max_radius*((double)radius/-100.0*cos(theta+M_PI))));
      y2 = (long)((y1) - (max_radius*((double)radius/-100.0*sin(theta+M_PI))));
    }
    (self)->MoveTo( x1, y1);
  }
  (self)->DrawLineTo( x2, y2);

  return;
}


static void
Redraw(class clockview  *self)
{
/*
  Redisplay this object.
*/
  struct clock_time *clockface;
  struct clock_options *options;
  struct rectangle rect, border;
  long min_dimension;
  class fontdesc *myfontdesc;

  clockface = ((class clock *) (self)->GetDataObject())->ReadClock();
  options = ((class clock *) (self)->GetDataObject())->GetOptions();
  if (self->last_options_timestamp != options->timestamp) {
    self->last_options_timestamp = options->timestamp;
    self->need_full_update = TRUE;
  }

  (self)->GetLogicalBounds( &rect);
  if (self->need_full_update) {
    /* need to redraw face and hands */
    (self)->SetTransferMode( graphic::SOURCE);
    (self)->EraseVisualRect();
    min_dimension = MIN(rect.width, rect.height);
    if (options->border_width > 0) {
      border.left = rect.left+(rect.width-min_dimension+options->border_width)/2;
      border.top = rect.top+(rect.height-min_dimension+options->border_width)/2;
      border.width = min_dimension - options->border_width + (rect.width-min_dimension+options->border_width)%2;
      border.height = min_dimension - options->border_width + (rect.height-min_dimension+options->border_width)%2;

      (self)->SetLineWidth( options->border_width);
      if (options->border_shape == circle) {
	(self)->DrawArc( &border, 0, 360);
      } else {
	(self)->DrawRect( &border);
      }
    }

#define FACELOOP(num, func, where, what) \
    if (num) { \
      int i; \
      for (i = 0; i < (num); ++i) { \
	(func)(self, M_PI/2.0 - ((double)i)/((double)(num))*2.0*M_PI, (where)*(options->tick_length), what, options->border_shape); \
      } \
    }

    FACELOOP(options->minor_ticks, PlotPoints, -1, 1);
    FACELOOP(options->major_ticks, PlotPoints, -2, 1);
    myfontdesc = fontdesc::Create(options->fontfamily, options->fontface, (options->tick_length*min_dimension/(options->major_ticks?150:75)));
    if (myfontdesc) (self)->SetFont( myfontdesc);
    FACELOOP(options->num_labels, PlotLabels, -3, options->labels[i]);

    (self)->SetTransferMode( graphic::XOR);
    PlotPoints(self, HOURSTORADIANS(clockface->hours), options->hours_length, options->hours_width, options->border_shape);
    PlotPoints(self, MINUTESTORADIANS(clockface->minutes), options->minutes_length, options->minutes_width, options->border_shape);
    PlotPoints(self, MINUTESTORADIANS(clockface->seconds), options->seconds_length, options->seconds_width, options->border_shape);
  } else {
    (self)->SetTransferMode( graphic::XOR);
    if (self->lastclockface.hours != clockface->hours) {
      /* redraw hour hand */
      PlotPoints(self, HOURSTORADIANS(self->lastclockface.hours), options->hours_length, options->hours_width, options->border_shape);
      PlotPoints(self, HOURSTORADIANS(clockface->hours), options->hours_length, options->hours_width, options->border_shape);
    }

    if (self->lastclockface.minutes != clockface->minutes) {
      /* redraw minute hand */
      PlotPoints(self, MINUTESTORADIANS(self->lastclockface.minutes), options->minutes_length, options->minutes_width, options->border_shape);
      PlotPoints(self, MINUTESTORADIANS(clockface->minutes), options->minutes_length, options->minutes_width, options->border_shape);
    }

    if (self->lastclockface.seconds != clockface->seconds) {
      /* redraw second hand */
      PlotPoints(self, MINUTESTORADIANS(self->lastclockface.seconds), options->seconds_length, options->seconds_width, options->border_shape);
      PlotPoints(self, MINUTESTORADIANS(clockface->seconds), options->seconds_length, options->seconds_width, options->border_shape);
    }
  }
  self->lastclockface.hours = clockface->hours;
  self->lastclockface.minutes = clockface->minutes;
  self->lastclockface.seconds = clockface->seconds;
  self->need_full_update = FALSE;
}


void
clockview::FullUpdate(enum view::UpdateType  type, long  left , long  top , long  width , long  height)
{
/*
  Do an update.
*/
  if ((type == view::FullRedraw) || (type == view::LastPartialRedraw)) {
    this->need_full_update = TRUE;
    Redraw(this);
  }
}


void
clockview::Update()
{
  Redraw(this);
}


class view *
clockview::Hit(enum view::MouseAction  action, long  x , long  y, long  numclicks  )
{
/*
  Handle the button event.  Currently, semantics are:
*/
  (this)->WantInputFocus( this);
  return((class view *)this);
}


void
clockview::Print(FILE  *file, const char  *processor , const char  *finalFormat, boolean  topLevel)
{
  time_t t;

  t = time(0);
  fprintf(file, "%s", ctime(&t));
}



void
clockview::PostMenus(class menulist  *ml)
{
/*
  Enable the menus for this object.
*/

  (this->ml)->ClearChain();
  if (ml) (this->ml)->ChainAfterML( ml, (long)ml);
  (this)->view::PostMenus( this->ml);
}
