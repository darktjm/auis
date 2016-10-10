/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Calc View-object

MODULE	calcv.c

VERSION	1.0

DESCRIPTION
	This is the suite of Methods that support the Calc View-object,
	a trivial example of an ATK Inset.

HISTORY
  02/23/88	Created (TCP)
  08/03/89	Suppress Top-row Icons (TCP)
		Accept Keyboard input.
  08/07/89	Print Outline (TCP)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include <graphic.H>
#include <observable.H>
#include <view.H>
#include <im.H>
#include <rect.h>
#include <proctable.H>
#include <keymap.H>
#include <keystate.H>
#include <apt.H>
#include <aptv.H>
#include <calc.H>
#include <calcv.H>
#include <ctype.h>

#define  circle			      1
#define  box			      2
#define  roundbox		      3

#define  Balanced		     (graphic::BETWEENLEFTANDRIGHT | graphic::BETWEENTOPANDBASELINE)
#define  RightMiddle		     (graphic::ATRIGHT | graphic::BETWEENTOPANDBASELINE)

#define  Data			    ((class calc *)self->data_object)
#define  Operand1	    	     (self->operand_1)
#define  Operand2   		     (self->operand_2)
#define  Expression		     (self->expression)
#define  PriorExpression	     (self->prior_expression)
#define  PendingOp		     (self->pending_op)
#define  PendingDisplay		     (self->states.pending_display)
#define  PendingOutline		     (self->states.pending_outline)
#define  PointPresent		     (self->states.point_present)
#define  InputFocus		     (self->states.inputfocus)
#define  Keystate		     (self->keystate)

#define  Bounds			     (&self->bounds)
#define  Left			     (self->bounds.left)
#define  Top			     (self->bounds.top)
#define  Width			     (self->bounds.width)
#define  Height			     (self->bounds.height)

#define  Areas			     (self->areas)
#define  AreaCount		     (self->area_count)
#define	 Area(i)		     (Areas[i])

#define	 AreaBound(i)		     (&Area(i).bounds)
#define	 AreaBounds(i)		     (Area(i).bounds)
#define	 AreaLeft(i)		     (AreaBounds(i).left)
#define	 AreaRight(i)		     (AreaLeft(i) + AreaWidth(i) - 1)
#define	 AreaCenter(i)		     (AreaLeft(i) + (AreaWidth(i) / 2))
#define	 AreaTop(i)		     (AreaBounds(i).top)
#define	 AreaBottom(i)		     (AreaTop(i)  + AreaHeight(i) - 1)
#define	 AreaMiddle(i)		     (AreaTop(i)  + (AreaHeight(i) / 2))
#define	 AreaWidth(i)		     (AreaBounds(i).width)
#define	 AreaHeight(i)		     (AreaBounds(i).height)

#define	 AreaString(i)		      Area(i).string
#define	 AreaFont(i)		      Area(i).font
#define	 AreaAlign(i)		      Area(i).mode
#define	 AreaShape(i)		      Area(i).shape
#define	 AreaHitHandler(i)	      Area(i).hit_handler
#define	 AreaSpec(i)	    	      Area(i).spec
#define	 AreaSpecX(i)    	      Area(i).spec->x_center
#define	 AreaSpecY(i)    	      Area(i).spec->y_center
#define	 AreaSpecW(i)    	      Area(i).spec->width
#define	 AreaSpecH(i)    	      Area(i).spec->height
#define	 AreaHighlighted(i)    	      Area(i).states.highlighted

  
				      
static const char		      digit_font[] = "andysans10b",
				      oper_font[]  = "andysans16b",
				      expr_font[]  = "andysans12b";

#define  DW	/* Digit Width  */    (20)
#define  DH	/* Digit Height */    (DW)
#define  OW	/* Oper. Width  */    (25)
#define  OH	/* Oper. Height */    (OW/2)

#define  DR1	/* Digit ROW 1 */     (30)
#define  DR2	/* Digit ROW 1 */     (DR1 + DH)
#define  DR3	/* Digit ROW 1 */     (DR2 + DH)
#define  DR4	/* Digit ROW 1 */     (DR3 + DH)
#define  DC1	/* Digit Col 1 */     (14)
#define  DC2	/* Digit Col 2 */     (DC1 + DW )
#define  DC3	/* Digit Col 3 */     (DC2 + DW )
#define  OC1	/* Oper. Col 1 */     (85)
#define  OR1	/* Oper. Row 1 */     (30)
#define  OR2	/* Oper. Row 2 */     (OR1 + OH)
#define  OR3	/* Oper. Row 3 */     (OR2 + OH)
#define  OR4	/* Oper. Row 4 */     (OR3 + OH)
#define  OR5	/* Oper. Row 5 */     (OR4 + OH)
#define  OR6	/* Oper. Row 6 */     (OR5 + OH)

ATKdefineRegistry(calcv, aptv, calcv::InitializeClass);
static void Replace_String( class calcv	      *self, char		      *old, char		      *new_c, long		       area );
static long Which_Area( class calcv	      *self, long		       x , long		       y );
static void Printer( class calcv	      *self );
static void Stroke( class calcv	      *self, long		       area );
static void Digit( class calcv	      *self, long		       area );
static void Operator( class calcv	      *self, long		       area );
static void Shrink( char		      *string );
static void Clear( class calcv	      *self, long		       area );
static void Display( class calcv	      *self, long		       area );
static void Fill_Area( class calcv	      *self, long		       area, long		       op );
static void Highlight_Area( class calcv	      *self, long		       area );
static void Normalize_Other_Areas( class calcv	      *self, long		       area );
static void Normalize_Area( class calcv	      *self, long			       area );
static void Draw_Calc( class calcv	      *self );
static void Draw_Outline( class calcv	      *self );


struct calcv_setup
  {
  const char				     *string;
  const char				     *font_name;
  int				      mode;
  char				      shape;
  calcv_hitfptr			      hit_handler;
  long				      x_center, y_center, width, height;
  };

static const struct calcv_setup setups[] =
   {
 { "7",	  digit_font, Balanced, box,   Digit,	    DC1,DR1, DW,DH },/* Area  0 */
 { "8",   digit_font, Balanced, box,   Digit,	    DC2,DR1, DW,DH },/* Area  1 */
 { "9",   digit_font, Balanced, box,   Digit,	    DC3,DR1, DW,DH },/* Area  2 */
 { "4",   digit_font, Balanced, box,   Digit,	    DC1,DR2, DW,DH },/* Area  3 */
 { "5",   digit_font, Balanced, box,   Digit,	    DC2,DR2, DW,DH },/* Area  4 */
 { "6",   digit_font, Balanced, box,   Digit,	    DC3,DR2, DW,DH },/* Area  5 */
 { "1",   digit_font, Balanced, box,   Digit,	    DC1,DR3, DW,DH },/* Area  6 */
 { "2",   digit_font, Balanced, box,   Digit,	    DC2,DR3, DW,DH },/* Area  7 */
 { "3",   digit_font, Balanced, box,   Digit,	    DC3,DR3, DW,DH },/* Area  8 */
 { "0",   digit_font, Balanced, box,   Digit,	    DC1,DR4, DW,DH },/* Area  9 */
 { ".",   digit_font, Balanced, box,   Digit,	    DC2,DR4, DW,DH },/* Area 10 */
 { "=",   oper_font,  Balanced, box,   Operator,    OC1,OR1, OW,OH },/* Area 11 */
 { "+",   oper_font,  Balanced, box,   Operator,    OC1,OR2, OW,OH },/* Area 12 */
 { "-",   oper_font,  Balanced, box,   Operator,    OC1,OR3, OW,OH },/* Area 13 */
 { "X",   oper_font,  Balanced, box,   Operator,    OC1,OR4, OW,OH },/* Area 14 */
 { "/",   oper_font,  Balanced, box,   Operator,    OC1,OR5, OW,OH },/* Area 15 */
 { "AC",  oper_font,  Balanced, box,   Clear,	    OC1,OR6, OW,OH },/* Area 16 */
 { "0",   expr_font, RightMiddle, roundbox, Display, 50,10, 98,18 },/* Area 17 */
 { NULL }
   };
#define  AssignArea		      11
#define  MultiplyArea		      14
#define  ClearArea		      16
#define  DisplayArea		      17

static class keymap		     *class_keymap;



boolean 
calcv::InitializeClass( )
    {
  IN(calcv_InitializeClass);
  class_keymap = new keymap;
  OUT(calcv_InitializeClass);
  return TRUE;
  }

calcv::calcv( )
{
    class calcv *self=this;
	ATKinit;

  long		       i;
  struct proctable_Entry *key_proc;

  IN(calcv_InitializeObject);
  (this)->SetOptions(  aptv_SuppressControl |
			  aptv_SuppressBorder |
			  aptv_SuppressEnclosures );
  (this)->SetDimensions(  150, 175 );
  bzero( &this->states, sizeof(struct calcv_states) );
  Keystate = keystate::Create( this, class_keymap );
  key_proc = proctable::DefineProc( "calcv-stroke", (proctable_fptr)Stroke,
		&calcv_ATKregistry_ , NULL, "Type Digit or Operator" );
  AreaCount = 0;
  for ( i = 0; setups[i].string; i++ )
    {
    AreaCount++;
    bzero( &Area(i), sizeof(struct calcv_area) );
    AreaString(i)	= setups[i].string;
    AreaAlign(i)	= setups[i].mode;
    AreaShape(i)	= setups[i].shape;
    AreaHitHandler(i)	= setups[i].hit_handler;
    AreaSpec(i)		= &setups[i];
    if(i < DisplayArea)
	(class_keymap)->BindToKey(  AreaString(i), key_proc, i );
    }
  (class_keymap)->BindToKey(  "x",	  key_proc, MultiplyArea );
  (class_keymap)->BindToKey(  "*",	  key_proc, MultiplyArea );
  (class_keymap)->BindToKey(  "\012", key_proc, AssignArea );
  (class_keymap)->BindToKey(  "\015", key_proc, AssignArea );
  (class_keymap)->BindToKey(  " ",    key_proc, ClearArea );
  (class_keymap)->BindToKey(  "ac",   key_proc, ClearArea );
  strcpy( Expression, AreaString(DisplayArea) );
  strcpy( PriorExpression, Expression );
  AreaString(DisplayArea) = Expression;
  *Operand1 = *Operand2 = PendingOp = 0;
  OUT(calcv_InitializeObject);
  THROWONFAILURE(  TRUE);
  }

calcv::~calcv( )
      {
    class calcv *self=this;
	ATKinit;

  IN(calcv_FinalizeObject);
  if ( Keystate )	delete Keystate ;
  OUT(calcv_FinalizeObject);
  }

void
calcv::SetDataObject( class dataobject	       *data )
      {
    class calcv *self=this;
  IN(calcv_SetDataObject);
  (this)->aptv::SetDataObject(  data );
  DEBUGgt(Value,(Data )->Value( ));
  sprintf( Operand1, "%lf", (Data )->Value( ) );
  ::Shrink( Operand1 );
  strcpy( Expression, Operand1 );
  strcpy( PriorExpression, Expression );
  DEBUGst(Operand1,Operand1);
  OUT(calcv_SetDataObject);
  }

void 
calcv::ReceiveInputFocus( )
    {
    class calcv *self=this;
  IN(calcv_ReceiveInputFocus);
  InputFocus = true;
  Keystate->next = NULL;
  (this)->PostKeyState(  Keystate );
  PendingOutline = true;
  (this)->WantUpdate(  this );
  OUT(calcv_ReceiveInputFocus);
  }

void
calcv::LoseInputFocus( )
    {
    class calcv *self=this;
  IN(calcv_LoseInputFocus);
  InputFocus = false;
  PendingOutline = true;
  (this)->WantUpdate(  this );
  OUT(calcv_LoseInputFocus);
  }

void 
calcv::FullUpdate( enum view::UpdateType	 type, long		       left , long		       top , long		       width , long		       height )
        {
    class calcv *self=this;
  long		      i, L, T, W, H;

  IN(calcv_FullUpdate);
  if ( Data  &&  (type == view::FullRedraw || type == view::LastPartialRedraw) )
    {
      (this)->GetLogicalBounds(  Bounds );
      (this)->aptv::FullUpdate(  type, 0,0, Width, Height );
    if ( ! (this)->BypassUpdate() )
      {
      L = Left + 2;  T = Top + 2;
      for ( i = 0; i < AreaCount; i++ )
        {
        W = ((AreaSpecW(i) * (Width-4)) / 100);
        H = ((AreaSpecH(i) * (Height-4)) / 100);
        if ( AreaShape(i) == circle ) {
	  if ( W > H )  W = H;
	  else if ( H > W )  H = W;
	}
        AreaWidth(i)  = W - 2;
        AreaHeight(i) = H - 2;
        AreaLeft(i) = L + (((AreaSpecX(i) * (Width-4))  / 100) - W/2);
        AreaTop(i)  = T + (((AreaSpecY(i) * (Height-4)) / 100) - H/2);
	AreaFont(i) = (this)->BuildFont(  setups[i].font_name, NULL );
        }
      Draw_Calc( this );
      Draw_Outline( this );
      }
    }
  OUT(calcv_FullUpdate);
  }

void 
calcv::Update( )
    {
    class calcv *self=this;
  IN(calcv_Update);
  if ( PendingDisplay )
    {
    PendingDisplay = false;
    Replace_String( this, PriorExpression, Expression, DisplayArea );
    }
  else
  if ( PendingOutline )
    {
    PendingOutline = false;
    Draw_Outline( this );
    }
  OUT(calcv_Update);
  }

static
void Replace_String( class calcv	      *self, char		      *old, char		      *new_c, long		       area )
        {

  (self)->ClearBoundedString(  old, AreaFont(area), AreaBound(area),
			  AreaRight(area) - 5, AreaMiddle(area), AreaAlign(area) );
  (self)->DrawBoundedString(  new_c, AreaFont(area), AreaBound(area),
			  AreaRight(area) - 5, AreaMiddle(area), AreaAlign(area) );
  strcpy( old, new_c );
  }

void
calcv::ObservedChanged( class observable  *changed, long		      value )
        {
    class calcv *self=this;
  IN(calcv_ObservedChanged);
  switch ( value )
    {
    case calc_value_changed:
	sprintf( Expression, "%lf", (Data )->Value( ) );
	::Shrink( Expression );
      strcpy( Operand1, Expression );
      *Operand2 = 0;
      PendingDisplay = true;
      (this)->WantUpdate(  this );
      break;
    }
  OUT(calcv_ObservedChanged);
  }

static long
Which_Area( class calcv	      *self, long		       x , long		       y )
      {
  long		      i;

  for ( i = 0; i < AreaCount; i++ )
    if ( (self)->Within(  x, y, AreaBound(i) ) )
      break;
  return  i;
  }

class view *
calcv::Hit( enum view::MouseAction  action, long		       x , long		       y , long		       clicks )
        {
    class calcv *self=this;
  class view	     *hit = NULL;
  long		      which;

  IN(calcv_Hit );
  if ( ! InputFocus )
    (this)->WantInputFocus(  this );
  if ( InputFocus  &&  (hit = (this)->aptv::Hit(  action, x, y, clicks )) == NULL )
    if ( (this)->Within(  x, y, Bounds ) )
      {
      hit = (class view *) this;
      if ( (which = Which_Area( this, x, y )) < AreaCount )
        switch ( action )
          {
          case  view::LeftDown:
	    (AreaHitHandler(which))( this, which );
            break;
          case  view::LeftMovement:
            break;
          case  view::LeftUp:
            break;
	  default:
	    break;
        }
      }
  OUT(calcv_Hit );
  return  hit;
  }

static
void Printer( class calcv	      *self )
    {
  long		      i, x, y;

  (self)->PrintRoundBox(  Left+1, Top+1, Width-3, Height-3, 0 );
  for ( i = 0; i < AreaCount; i++ )
    {
    switch ( AreaShape(i) )
      {
      case  circle:
	break;
      case  box:
	(self)->PrintBox(  AreaLeft(i), AreaTop(i),
			    AreaWidth(i), AreaHeight(i), 0 );
    	break;
      case  roundbox:
	(self)->PrintRoundBox(  AreaLeft(i), AreaTop(i),
				  AreaWidth(i), AreaHeight(i), 0 );
	break;
      }
    x = AreaCenter(i);
    y = AreaMiddle(i);
/*===
    switch ( AreaAlign(i) )
      {
      case Balanced:	x = AreaCenter(i);	break;
      case RightMiddle:	x = AreaRight(i) - 5;	break;
      }
===*/
    if ( AreaFont(i) )
      (self)->SetPrintFont(  AreaSpec(i)->font_name );
    (self)->PrintString(  x, y, AreaString(i), 0 );
    }
  }

void
calcv::Print( FILE		      *file, const char		      *processor, const char		      *format, boolean	       level )
            {
  IN(calcv_Print);
  (this)->PrintObject(  file, processor, format, level, (aptv_printfptr)Printer );
  OUT(calcv_Print);
  }

static void
Stroke( class calcv	      *self, long		       area )
      {
  IN(Stroke);
  DEBUGdt(Area,area);
  if ( area >= 0  &&  area <= 10 )  Digit( self, area );    
  else
  if ( area >= 11  &&  area <= 15 ) Operator( self, area );
  else
  if ( area == 16 )		    Clear( self, area );
  OUT(Stroke);
  }

static void
Digit( class calcv	      *self, long		       area )
      {
  IN(Digit);
  Highlight_Area( self, area );
  if ( Expression[0] == '0'  &&  Expression[1] != '.'  &&
       *AreaString(area) != '.')
    strcpy( Expression, Expression + 1 );
  if ( PendingOp )
    {
    if ( PendingOp == '=' )
      {
      PendingOp = 0;
      *Expression = 0;
      strcpy( Operand1, AreaString(area) );
      }
      else  strcat( Operand2, AreaString(area) );
    }
    else  strcat( Operand1, AreaString(area) );
  sscanf( Operand1, "%lf", &(Data )->Value( ) );
  if ( !(*AreaString(area) == '.'  &&  PointPresent) )
    {
    if ( *AreaString(area) == '.' )   PointPresent = true;
    strcat( Expression, AreaString(area) );
    PendingDisplay = true;
    (self)->WantUpdate(  self );
    }
  OUT(Digit);
  }

static void
Operator( class calcv	      *self, long		       area )
      {
  double		  operand_1, operand_2, value;

  IN(Operator);
  Highlight_Area( self, area );
  if ( *Operand1 )
    {
    sscanf( Operand1, "%lf", &value );
    DEBUGst(Operand1,Operand1);
    PointPresent = false;
    if ( PendingOp )
      {
      DEBUGct(PendingOp,PendingOp);
      if ( *Operand2 )
        {
	DEBUGst(Operand2,Operand2);
        sscanf( Operand1, "%lf", &operand_1 );
        sscanf( Operand2, "%lf", &operand_2 );
        switch ( PendingOp )
          {
          case  '+':  value = operand_1 + operand_2;        break;
          case  '-':  value = operand_1 - operand_2;        break;
          case  'X':  value = operand_1 * operand_2;        break;
          case  '/':  value = operand_1 / ((operand_2) ? operand_2 : 1); break;
          }
	sprintf( Expression, "%lf", value );
	::Shrink( Expression );
        strcpy( Operand1, Expression );
        *Operand2 = 0;
        }
        else  sprintf( Expression, "%s", Operand1 );
      }
    if ( *AreaString(area) != '=' )
      {
      sprintf( Expression, "%s %s ", Expression, AreaString(area) );
      PendingDisplay = true;
      (self)->WantUpdate(  self );
      }
      else
      {
      (Data)->SetValue(  value );
      (Data)->NotifyObservers(  calc_value_changed );
      }
    PendingOp = *AreaString(area);
    }
  OUT(Operator);
  }

static
void Shrink( char		      *string )
    {
  char		     *ptr;

  ptr = string + strlen( string ) - 1;
  if ( index( string, '.' ) )
    while ( *ptr == '0' ) *ptr-- = 0;
  if ( *ptr == '.' )  *ptr = 0;
  }
 
static void
Clear( class calcv	      *self, long		       area )
      {
  Highlight_Area( self, area );
  (Data)->SetValue(  0.0 );
  strcpy( Expression, "0" );
  *Operand1 = *Operand2 = PendingOp = 0;
  PointPresent = false;
  (Data)->NotifyObservers(  calc_value_changed );
  }

static void
Display( class calcv	      *self, long		       area )
      {
  }

static
void Fill_Area( class calcv	      *self, long		       area, long		       op )
        {
  (self)->SetTransferMode(  op );
  switch( AreaShape(area) )
    {
    case  circle:
      (self)->FillOval(  AreaBound(area), NULL );
      break;
    case  box:
      (self)->FillRectSize(  AreaLeft(area) + 2, AreaTop(area) + 2,
			    AreaWidth(area) - 3, AreaHeight(area) - 3, NULL );
      break;
    case  roundbox:
      (self)->FillRRectSize(  AreaLeft(area), AreaTop(area),
			AreaWidth(area), AreaHeight(area), 6,6, NULL );
      break;
    }
  }

static
void Highlight_Area( class calcv	      *self, long		       area )
      {
  if ( ! AreaHighlighted(area) )
    {
    AreaHighlighted(area) = true;
    Fill_Area( self, area, graphic::INVERT );
    Normalize_Other_Areas( self, area );
    }
  }

static
void Normalize_Other_Areas( class calcv	      *self, long		       area )
      {
  long		      i;
  for ( i = 0; i < AreaCount; i++ )
    if ( AreaHighlighted(i)  &&  i != area )
      Normalize_Area( self, i );
  }

static
void Normalize_Area( class calcv	      *self, long			       area )
      {
  if (  AreaHighlighted(area) )
    {
    AreaHighlighted(area) = false;
    Fill_Area( self, area, graphic::INVERT );
    }
  }

static
void Draw_Calc( class calcv	      *self )
    {
  long		      i, x, y;

  IN(Draw_Calc);
  (self)->SetTransferMode(  graphic::BLACK );
  for ( i = 0; i < AreaCount; i++ )
    {
    y = AreaMiddle(i);
    switch ( AreaAlign(i) )
      {
      case Balanced:	x = AreaCenter(i);	break;
      case RightMiddle:	x = AreaRight(i) - 5;	break;
      default:		x = AreaCenter(i);
      }
    (self)->DrawBoundedString(  AreaString(i), AreaFont(i),  AreaBound(i),
				x, y, AreaAlign(i) );
    switch ( AreaShape(i) )
      {
      case  circle:	(self)->DrawOval(  AreaBound(i) );	break;
      case  box:	(self)->DrawRect(  AreaBound(i) );	break;
      case  roundbox:	(self)->DrawRRectSize(  AreaLeft(i), AreaTop(i),
				AreaWidth(i), AreaHeight(i), 8,8 );
	break;
      }
    if ( AreaHighlighted(i) )  Fill_Area( self, i, graphic::INVERT );
    }
  OUT(Draw_Calc);
  }

static
void Draw_Outline( class calcv	      *self )
    {
  IN(Draw_Outline);
  (self )->ClearClippingRect( );
  (self)->SetTransferMode(  graphic::BLACK );
  (self)->DrawRRectSize(  Left+1, Top+1, Width-3,Height-3, 10,10 );
  if ( ! InputFocus )
    (self)->SetTransferMode(  graphic::WHITE );
  (self)->DrawRRectSize(  Left, Top, Width-1, Height-1, 11,11 );
  OUT(Draw_Outline);
  }
