/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Apt View-object

MODULE	aptv.c

VERSION	1.0

AUTHOR	TC Peters
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the suite of Methods that support the Apt View-object.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  02/23/88	Created (TCP)
  05/01/89	Change cursor font-names (shorten) (TCP)
  05/08/89	Move Cursor-setting in FullUpdate to within FullRedraw (TCP)
  05/09/89	Use GetDrawable in StringSize (TCP)
  05/18/89	Have SetDataObject do super_SetDataObject (TCP)
  05/22/89	Directly refer to Data-object Title/Legend strings (TCP)
		Correctly set fonts & sizes for each enclosure
  05/26/89	Remove area blanking for bounded string drawing (TCP)
  06/07/89	Added code the QueryFileName() and QueryDirectoryName()
	        to check for the occurance of "/.." and "/." at the 
		end of the result returned from completion_GetFilename().
		(GW Keim)
  06/16/89	Fixed a bug in aptv_Query{File,Directory}Name() that resulted when
                the completion_GetFilename() call failed.  Bogus if-then parenthesizing
		was the culprit that allowed some internal string manipulation and a
		aptv_Announce() call when the GetFilename() call was cancelled; (GW Keim)
  07/25/89	Remove arg from im_ForceUpdate (TCP)
  08/07/89	Only emit ShowPage when topLevel (TCP)
		Fix EndPS args -- drop PRF usage (TCP)
  08/24/89	Simplify use of macros for AIX compiler (TCP)
  10/07/89      Changed a few chained assignments in InitializeObject to
                single assignnments to satisfy the MIPS C compiler. (zs01)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
ATK_IMPL("aptv.H")
#include <graphic.H>
#include <fontdesc.H>
#include <observable.H>
#include <view.H>
#include <cursor.H>
#include <im.H>
#include <attribs.h>
#include <filetype.H>
#include <message.H>
#include <completion.H>
#include <text.H>
#include <texttroff.H>
#include <textview.H>
#include <rect.h>
#include <math.h>
#include <apts.H>
#include <apt.H>
#include <aptv.H>
#include <ctype.h>

#define  Balanced		    (view_BETWEENLEFTANDRIGHT | view_BETWEENTOPANDBOTTOM)

#define  Initialized		      self->states.initialized

#define  Data			      self->data_object

#define  Cursor			      self->cursor

#define  OriginalWidth		      self->original_width
#define  OriginalHeight		      self->original_height
#define  DimensionWidth		      self->dimension_width
#define  DimensionHeight	      self->dimension_height

#define  Enclosure(e)		      self->enclosures[e]
#define  EnclosureAreas(e)	    (&Enclosure(e).areas)
#define  Areas(e)		     (EnclosureAreas(e))
#define  Area(e,a)		    (&Enclosure(e).areas[a])
#define  EnclosureBounds(e)	    (&Enclosure(e).bound)
#define  Bounds(e)		     (EnclosureBounds(e))

#define  Outer		   	      0
#define  Border			      1
#define  Control	    	      2
#define  Title			      3
#define  Legend			      4
#define  Body		    	      5

#define  Left(e)		     (Bounds(e)->left)
#define  Center(e)		     (Left(e) + Width(e)/2)
#define  Right(e)		     (Left(e) + Width(e))
#define  Top(e)			     (Bounds(e)->top)
#define  Middle(e)    		     (Top(e) + Height(e)/2)
#define  Bottom(e)		     (Top(e) + Height(e))
#define  Width(e)		     (Bounds(e)->width)
#define  Height(e)		     (Bounds(e)->height)

#define  LeftArea		     0
#define  TopArea		     1
#define  RightArea		     2
#define  BottomArea		     3

#define  AreaBounds(e,a)	    (&Area(e,a)->bound)
#define  AreaSize(e,a)		     (Area(e,a)->size)
#define  AreaFont(e,a)		     (Area(e,a)->font)
#define  AreaFontName(e,a)	     (Area(e,a)->font_name)
#define  AreaStringAnchor(e,a)	     (Area(e,a)->strings)
#define  AreaStringCount(e,a)	     (Area(e,a)->strings_count)
#define  AreaString(e,a,i)	     ((Area(e,a)->strings)[i])
#define  AreaLeft(e,a)		     (Area(e,a)->bound.left)
#define  AreaCenter(e,a)	     (AreaLeft(e,a) + AreaWidth(e,a)/2)
#define  AreaRight(e,a)		     (AreaLeft(e,a) + AreaWidth(e,a))
#define  AreaTop(e,a)		     (Area(e,a)->bound.top)
#define  AreaMiddle(e,a)	     (AreaTop(e,a)  + AreaHeight(e,a)/2)
#define  AreaBottom(e,a)	     (AreaTop(e,a)  + AreaHeight(e,a))
#define  AreaWidth(e,a)		     (Area(e,a)->bound.width)
#define  AreaHeight(e,a)	     (Area(e,a)->bound.height)

#define  DefaultFont		      self->default_font
#define  DefaultFontName	      "AndySans10"
#define  IconFont		      self->icon_font
#define  IconFontName		      "apticn20"
#define  CursorFont		      self->cursor_font
#define  CursorFontName		      "aptcsr20"

#define  ShrinkIcon		      self->shrink_icon
#define  ShrinkTitle		      self->shrink_title
#define  ShrinkIconFont		      self->shrink_icon_font
#define  ShrinkTitleFont	      self->shrink_title_font
#define  Shrinking		      self->states.shrinking
#define  Shrunk			      self->states.shrunk

#define  ShrinkerBounds	    	    (&self->shrinker_bounds)
#define  ShrinkerLeft		      ShrinkerBounds->left
#define  ShrinkerCenter		     (ShrinkerLeft + ShrinkerWidth/2)
#define  ShrinkerRight		     (ShrinkerLeft + ShrinkerWidth)
#define  ShrinkerTop		      ShrinkerBounds->top
#define  ShrinkerMiddle		     (ShrinkerTop + ShrinkerHeight/2)
#define  ShrinkerBottom		     (ShrinkerTop + ShrinkerHeight)
#define  ShrinkerWidth	    	      ShrinkerBounds->width
#define  ShrinkerHeight	    	      ShrinkerBounds->height

#define  ShrinkIconBounds	    (&self->shrink_icon_bounds)
#define  ShrinkIconLeft		      ShrinkIconBounds->left
#define  ShrinkIconCenter	     (ShrinkIconLeft + ShrinkIconWidth/2)
#define  ShrinkIconRight	     (ShrinkIconLeft + ShrinkIconWidth)
#define  ShrinkIconTop		      ShrinkIconBounds->top
#define  ShrinkIconMiddle	     (ShrinkIconTop + ShrinkIconHeight/2)
#define  ShrinkIconBottom	     (ShrinkIconTop + ShrinkIconHeight)
#define  ShrinkIconWidth	      ShrinkIconBounds->width
#define  ShrinkIconHeight	      ShrinkIconBounds->height
#define  ShrinkTitleBounds	    (&self->shrink_title_bounds)
#define  ShrinkTitleLeft	      ShrinkTitleBounds->left
#define  ShrinkTitleCenter	     (ShrinkTitleLeft + ShrinkTitleWidth/2)
#define  ShrinkTitleRight	     (ShrinkTitleLeft + ShrinkTitleWidth)
#define  ShrinkTitleTop		      ShrinkTitleBounds->top
#define  ShrinkTitleMiddle	     (ShrinkTitleTop + ShrinkTitleHeight/2)
#define  ShrinkTitleBottom	     (ShrinkTitleTop + ShrinkTitleHeight)
#define  ShrinkTitleWidth	      ShrinkTitleBounds->width
#define  ShrinkTitleHeight	      ShrinkTitleBounds->height

#define  HelperBounds	    	    (&self->helper_bounds)
#define  HelperLeft		      HelperBounds->left
#define  HelperCenter		     (HelperLeft + HelperWidth/2)
#define  HelperRight		     (HelperLeft + HelperWidth)
#define  HelperTop		      HelperBounds->top
#define  HelperMiddle		     (HelperTop + HelperHeight/2)
#define  HelperBottom		     (HelperTop + HelperHeight)
#define  HelperWidth	    	     (HelperBounds->width)
#define  HelperHeight	    	     (HelperBounds->height)
#define  HelpDisplayed		      self->states.help_displayed
#define  HelpText		      self->help_text
#define  HelpTextView		      self->help_textview
#define  HelpString		      self->help_string
#define  HelpFileName		      self->help_file_name

#define  BypassUpdate		      self->bypass_update

#define  InputFocus		      self->states.inputfocus
#define  ControlSuppressed	      self->options.controls_suppressed
#define  BorderSuppressed	      self->options.border_suppressed
#define  EnclosuresSuppressed	      self->options.enclosures_suppressed
#define  GrayFill		      self->gray_fill
#define  WhiteFill		      self->white_fill

#define  PrintStream		     (self->print_stream)
#define  PrintFile		     (PrintStream->file)
#define  PrintProcessor		     (PrintStream->processor)
#define  PrintFormat		     (PrintStream->format)
#define  PrintLevel		     (PrintStream->level)
#define  PrintTopLevel		     (PrintStream->level==1)
#define  PrintPrefix		     (PrintStream->prefix)
#define  PrintPortrait		     (PrintStream->portrait)
#define  PrintLandScape		     (PrintStream->landscape)
#define  PrintResolution	     (PrintStream->pixels_per_inch)
#define  PrintResolutionFactor	     (PrintStream->resolution_factor)
#define  PRF(x)			     (PrintResolutionFactor*x)
#define  PrintUnitInchWidth	     (PrintStream->unit_inch_width)
#define  PrintUnitInchHeight	     (PrintStream->unit_inch_height)
#define  PrintPageInchWidth	     (PrintStream->page_inch_width)
#define  PrintPageInchHeight	     (PrintStream->page_inch_height)
#define  PrintPagePelWidth	     (PrintStream->page_pel_width)
#define  PrintPagePelHeight	     (PrintStream->page_pel_height)
#define  PrintPreserveAspect	     (PrintStream->preserve_aspect)
#define  PrintFillPage		     (PrintStream->fill_page)

#define  Troff			      (1<<0)
#define  PostScript		      (1<<1)









struct  aptv_print_stream
  {
  FILE				     *file;
  char				      processor;
  char				      format;
  short				      level;
  const char			     *prefix;
  float				      pixels_per_inch, resolution_factor;
  float				      unit_inch_width, unit_inch_height;
  float				      page_inch_width, page_inch_height;
  float				      page_pel_width, page_pel_height;
  char				      portrait, landscape, preserve_aspect,
				      fill_page;
  };



ATKdefineRegistry(aptv, view, NULL);
static class aptv * Parent_AptView( class aptv	       *self );
static void Print_Area( class aptv	      *self, long		       enclosure , long		       area );
static void Size_Enclosures( class aptv	      *self );
static void Help( class aptv	      *self );
static void Help_FullUpdate( class aptv	      *self );
static void Unhelp( class aptv	      *self );
static void Draw_String( class aptv	      *self, const char		      *string, class fontdesc    *font, struct rectangle   *bounds, long		       x , long		       y , long		       mode );
static void Draw_Enclosures( class aptv	      *self );


aptv::aptv( )
      {
class aptv *self=this;

  long		       e, a;

  IN(aptv_InitializeObject);
  DEBUGst(RCSID,rcsidaptv);
  this->imPtr = NULL;
  Data = NULL;
  PrintStream = (struct aptv_print_stream *)
		calloc( 1, sizeof(struct aptv_print_stream) );
  HelpText = NULL;
  HelpTextView = NULL;
  HelpString = HelpFileName = NULL;
  BypassUpdate = HelpDisplayed = false;
  Cursor = cursor::Create( this );
  IconFont = (this)->BuildFont(  IconFontName, NULL );
  CursorFont = (this)->BuildFont(  CursorFontName, NULL );
  DefaultFont = (this)->BuildFont(  DefaultFontName, NULL );
  ShrinkIcon[0] = 0;
  ShrinkIcon[1] = 0;
  ShrinkTitle = NULL;
  Initialized = false;
  InputFocus = false;
  Shrunk = false;
  Shrinking = false;
  ControlSuppressed = false;
  BorderSuppressed = false;
  EnclosuresSuppressed = false;
  OriginalWidth = OriginalHeight = DimensionWidth = DimensionHeight = 0;
  for ( e = 0; e < EnclosureCount; e++ )
    for ( a = 0; a < 4; a++ )
      {
      AreaSize(e,a) = 0;
      AreaFont(e,a) = DefaultFont;
      AreaFontName(e,a) = NULL;
      AreaStringAnchor(e,a) = NULL;
      AreaStringCount(e,a) = 0;
      }
  AreaSize(Border,LeftArea) = AreaSize(Border,TopArea) =
   AreaSize(Border,RightArea) = AreaSize(Border,BottomArea) = 3;
  AreaSize(Control,TopArea) = 20; /*===*/
  OUT(aptv_InitializeObject);
  THROWONFAILURE(  TRUE);
  }

aptv::~aptv( )
      {
class aptv *self=this;

  IN(aptv_FinalizeObject);
  if ( PrintStream )	free( PrintStream );
  if ( Cursor )		delete Cursor ;
  OUT(aptv_FinalizeObject);
  }

void
aptv::SetDataObject( class dataobject	       *data )
      {
class aptv *self=this;

  IN(aptv_SetDataObject);
  (this)->view::SetDataObject(  data );
  Data = (class apt *)data;
  (Data)->AddObserver(  this );
  OUT(aptv_SetDataObject);
  }

void
aptv::SetOptions( long		        options )
      {
class aptv *self=this;

  long		       e, a;

  IN(aptv_SetOptions);
  if ( options & aptv_Iconified )   Shrunk = true;
  if ( options & aptv_SuppressControl )
    {
    ControlSuppressed = true;
    AreaSize(Control,TopArea) = 0; /*===*/
    }
  if ( options & aptv_SuppressBorder )
    {
    BorderSuppressed = true;
    AreaSize(Border,LeftArea) = AreaSize(Border,TopArea) =
	AreaSize(Border,RightArea) = AreaSize(Border,BottomArea) = 0;
    }
  if ( options & aptv_SuppressEnclosures )
    {
    EnclosuresSuppressed = true;
    for ( e = 0; e < EnclosureCount; e++ )
      for ( a = 0; a < 4; a++ )
       AreaSize(e,a) = 0;
    }
  OUT(aptv_SetOptions);
  }

void
aptv::SetDimensions( long		        width , long		        height )
      {
class aptv *self=this;

  IN(aptv_SetDimensions);
  DimensionWidth  = OriginalWidth  = width;
  DimensionHeight = OriginalHeight = height;
  OUT(aptv_SetDimensions);
  }

void
aptv::SetHelpString( const char		       *string )
      {
class aptv *self=this;

  HelpString = string;
  }

void
aptv::SetHelpFileName( const char		       *file_name )
      {
class aptv *self=this;

  HelpFileName = file_name;
  }

class fontdesc *
aptv::BuildFont( const char		       *font_name, long	       *height )
        {
  class fontdesc    *font = NULL;
  char			      family[257];
  long			      style, size;
  struct FontSummary *summary;

  IN(aptv_BuildFont);
  DEBUGst(Font-name,font_name);
  fontdesc::ExplodeFontName( font_name, family, sizeof(family), &style, &size );
  DEBUGst(Family,family);
  font = fontdesc::Create( family, style, size );
  if ( height  &&  (this )->GetIM( ) )
    {
    if ( ( summary = (font)->FontSummary(  (this )->GetDrawable( ) ) ) )
      *height = summary->maxHeight;
      else  *height = 0;
    }
  OUT(aptv_BuildFont);
  return  font;
  }

void
aptv::SetShrinkIcon( const char		        icon, const char		       *icon_font_name, const char		       *title, const char		       *title_font_name )
            {
class aptv *self=this;

  long			       height = 0;

  IN(aptv_SetShrinkIcon);
  DEBUGct(Icon,icon);
  DEBUGst(Icon-font-name,icon_font_name);
  DEBUGst(Title,title);
  DEBUGst(Title-font-name,title_font_name);
  ShrinkIcon[0] = icon; ShrinkIcon[1] = 0;
  if ( icon_font_name  &&  *icon_font_name )
    ShrinkIconFont = (this)->BuildFont(  icon_font_name, NULL );
  if ( title  &&  *title )
    {
    ShrinkTitle = strdup( title );
    if ( title_font_name  &&  *title_font_name )
      {
      ShrinkTitleFont = (this)->BuildFont(  title_font_name, &height );
      height += 2;
      }
    ShrinkTitleHeight = height;
    }
  OUT(aptv_SetShrinkIcon);
  }

static class aptv *
Parent_AptView( class aptv	       *self )
    {
  class aptv	      *parent = NULL;
  class view *candidate=self->parent;
  IN(Parent_AptView);
  while ( candidate  &&  parent == NULL )
    {
    DEBUGst(Parent Name,(candidate )->GetTypeName( ));
    if ( ATK::IsTypeByName( (candidate )->GetTypeName( ), "aptv" ) )
      parent = (class aptv *)candidate;
      else
      candidate = candidate->parent;
    }
  OUT(Parent_AptView);
  return  parent;
  }

void
aptv::ShrinkView( class aptv	       *apt_view )
      {
  IN(aptv_ShrinkView);
  /* NOP */
  OUT(aptv_ShrinkView);
  }

void
aptv::ExpandView( class aptv	       *apt_view )
      {
  IN(aptv_ExpandView);
  /* NOP */
  OUT(aptv_ExpandView);
  }

void
aptv::Shrink( )
    {
class aptv *self=this;

  class aptv	      *parent;

  IN(aptv_Shrink);
  if ( ! Shrunk )
    {
    DEBUG(Shrink);
    Shrinking = true;
/*===
    OriginalWidth = Width(Outer);
    OriginalHeight = Height(Outer);
===*/
    if ( ( parent = Parent_AptView( this ) ) )
      (parent)->ShrinkView(  this );
      else
      (this)->WantNewSize(  this );
    }
  OUT(aptv_Shrink);
  }

void
aptv::Expand( )
    {
class aptv *self=this;

  class aptv	      *parent;

  IN(aptv_Expand);
  if ( Shrunk )
    {
    DEBUG(Expand);
    Shrunk = false;
    if ( ( parent = Parent_AptView( this ) ) )
      (parent)->ExpandView(  this );
      else
      (this)->WantNewSize(  this );
    }
  OUT(aptv_Expand);
  }

void 
aptv::FullUpdate( enum view_UpdateType	  type, long			  left , long			  top , long			  width , long			  height )
        {
class aptv *self=this;

  long			 e;

  IN(aptv_FullUpdate);
  if ( Data  &&  (type == view_FullRedraw || type == view_LastPartialRedraw) )
    {
    Size_Enclosures( this );
    for ( e = 0; e < EnclosureCount; e++ )
      Left(e) = Top(e) = Width(e) = Height(e) = 0;
    (this)->GetLogicalBounds(  Bounds(Outer) );
    ShrinkerLeft    = ShrinkerTop     = ShrinkerWidth    =
    ShrinkIconLeft  = ShrinkIconTop   = ShrinkIconWidth  =
    ShrinkTitleLeft = ShrinkTitleTop  = ShrinkTitleWidth =
    HelperLeft	    = HelperTop	      = HelperWidth	 = 
    ShrinkTitleHeight = 0;
    if ( ! Shrunk )
      {
      for ( e = 1; e < EnclosureCount; e++ )
	{
	Left(e) = Left(e-1) + AreaSize(e-1,LeftArea);
	Top(e)  = Top(e-1)  + AreaSize(e-1,TopArea);
	AreaLeft(e,LeftArea)     = AreaLeft(e,TopArea)  = AreaLeft(e,BottomArea) = Left(e);
	AreaTop(e,LeftArea)      = AreaTop(e,TopArea)   = 
	AreaTop(e,RightArea)     = Top(e);
	AreaWidth(e,LeftArea)    = AreaSize(e,LeftArea);
	AreaWidth(e,RightArea)   = AreaSize(e,RightArea);
	AreaHeight(e,TopArea)    = AreaSize(e,TopArea);
	AreaHeight(e,BottomArea) = AreaSize(e,BottomArea);
	}
      for ( e = 1; e < EnclosureCount; e++ )
	{
	Width(e)  = (((Right(e-1) - AreaSize(e-1,RightArea)) - Left(e))) -
		     (AreaSize(e-1,RightArea)  ? 1 : ((e==1) ? 1 : 0));
	Height(e) = (((Bottom(e-1) - AreaSize(e-1,BottomArea)) - Top(e))) -
		     (AreaSize(e-1,BottomArea) ? 1 : ((e==1) ? 1 : 0));
	AreaWidth(e,TopArea)   = AreaWidth(e,BottomArea) = Width(e);
	AreaHeight(e,LeftArea) = AreaHeight(e,RightArea) = Height(e);
	AreaTop(e,BottomArea)  = Bottom(e) - AreaSize(e,BottomArea);
	AreaLeft(e,RightArea)  = Right(e)  - AreaSize(e,RightArea);
	}
      if ( ! ControlSuppressed )
        {
        ShrinkerLeft    = Left(Control) + 1;
        ShrinkerTop     = Top(Control) + 1;
        ShrinkerWidth   = 20 /*===*/;
        ShrinkerHeight  = 20 /*===*/;
        HelperLeft	= Right(Control) - 22;
        HelperTop       = Top(Control) + 1;
        HelperWidth     = 20 /*===*/;
        HelperHeight    = 20 /*===*/;
        }
      }
      else /* Shrunk */
      {
      Width(Border)	= Width(Outer) - 1;
      Height(Border)	= Height(Outer) - 1;
      ShrinkIconLeft    =
      ShrinkTitleLeft	= Left(Border);
      ShrinkIconTop     = Top(Border);
      ShrinkIconWidth   = Width(Border);
      ShrinkIconHeight  = Height(Border);
      if ( ShrinkIconWidth > ShrinkIconHeight )
	ShrinkIconWidth  = ShrinkTitleWidth = ShrinkIconHeight;
	else
	ShrinkIconHeight = ShrinkTitleWidth = ShrinkIconWidth;
      ShrinkTitleTop	= Bottom(Border) - ShrinkTitleHeight;
      }
    GrayFill  = (this)->GrayPattern(  1, 2 );
    WhiteFill = (this )->WhitePattern( );
    Draw_Enclosures( this );
    if ( HelpDisplayed )  Help_FullUpdate( this );
    (this )->UseNormalCursor( );
    }
  OUT(aptv_FullUpdate);
  }

boolean
aptv::Within( long		       x , long		       y, struct rectangle   *bounds )
        {
  char		      status = 0;

  IN(aptv_Within);
  if ( x >= bounds->left  &&  x <= (bounds->left + bounds->width)  &&
       y >= bounds->top   &&  y <= (bounds->top  + bounds->height ) )
    status = 1;;
  OUT(aptv_Within);
  return  status;
  }

class view *
aptv::Hit( enum view_MouseAction   action, long			   x , long			   y , long			   clicks )
        {
class aptv *self=this;

  class view		 *hit = NULL;

  IN(aptv_Hit );
  if ( Shrunk  &&  action == view_LeftDown )
    {
    DEBUG(De-Shrinking);
    (this )->Expand( );
    }
  else
  if ( HelpDisplayed  &&  (this)->Within(  x, y, Bounds(Title ) ) )
    {
    DEBUG(Helping);
    hit = (HelpTextView)->Hit(  action,
			(HelpTextView)->EnclosedXToLocalX(  x ),
			(HelpTextView)->EnclosedYToLocalY(  y ), clicks );
    }
  else
  if ( ! (this)->Within(  x, y, Bounds(Body) ) )
    switch ( action )
      {
      case  view_LeftDown:
	if ( ! ControlSuppressed  && (this)->Within(  x, y, Bounds(Control) ) )
	  {
	  if ( (this)->Within(  x, y, ShrinkerBounds ) )
	    {
	    DEBUG(Shrinking);
	    (this )->Shrink( );
	    }
	  if ( (this)->Within(  x, y, HelperBounds ) )
	    {
	    DEBUG(Helping);
	    if ( HelpDisplayed )
	      Unhelp( this );
	      else
	      Help( this );
	    }
	  }
        break;
      case  view_LeftMovement:

        break;
      default:
      case  view_LeftUp:

        break;
      }
  OUT(aptv_Hit );
  return  hit;
  }

void
aptv::ClearBody( )
    {
class aptv *self=this;

  IN(aptv_ClearBody);
  (this)->SetTransferMode(  graphic_WHITE );
  (this)->FillRect(  Bounds(Body), graphic_WHITE );
  (this)->SetTransferMode(  graphic_BLACK );
  OUT(aptv_ClearBody);
  }

void
aptv::PrintContinue( )
    {
  IN(aptv_PrintContinue);
  /*  NOP  */
  OUT(aptv_PrintContinue);
  }

void
aptv::PrintObject( FILE		      *file, const char		      *processor, const char		      *format, boolean	       level, aptv_printfptr printer )
              {
class aptv *self=this;

  long		      e, a;

  IN(aptv_PrintObject);
  (this)->OpenPrintStream(  file, processor, format, level );
  (this)->SetPrintLineWidth(  2 );
  if ( !BorderSuppressed )
    (this)->PrintFilledRoundBox(  Left(Border),  Top(Border),
			      Width(Border), Height(Border), 0, 1 );
  (this)->SetPrintLineWidth(  1 );
  for ( e = 0; e < EnclosureCount; e++ )
    for ( a = 0; a < 4; a++ )
      Print_Area( this, e, a );
  (this )->PreservePrintState( );
  if ( printer )
    (*printer)( this );
  (this )->RestorePrintState( );
  (this )->ClosePrintStream( );
  OUT(aptv_PrintObject);
  }

static
void Print_Area( class aptv	      *self, long		       enclosure , long		       area )
      {
  long		      i, width, center;

  if ( AreaSize(enclosure,area)  &&  AreaStringAnchor(enclosure,area) )
    {
    if ( AreaStringAnchor(enclosure,area)  &&  AreaFont(enclosure,area) )
      (self)->SetPrintFont(  AreaFontName(enclosure,area) );
    width = AreaWidth(enclosure,area) / AreaStringCount(enclosure,area);
    center = AreaLeft(enclosure,area) + width / 2;
    for ( i = 0; i < AreaStringCount(enclosure,area); i++ )
      {
      (self)->PrintString(  center,
				     AreaMiddle(enclosure,area),
				     AreaString(enclosure,area,i), 0 );
      center += width;
      }
    }
  }

static const char 	* const PostScript_prelude[] =
{
"% Begin AptView PostScript Prelude  Version 0.0",
"gsave % Save Environment Around AptView Drawing",
"1 -1 scale",
"1 setlinewidth",
"/andy { /Times-Roman findfont exch scalefont} def",
"/Andy { /Times-Roman findfont exch scalefont} def",
"/andysans { /Helvetica findfont exch scalefont} def",
"/AndySans { /Helvetica findfont exch scalefont} def",
"/Andysans { /Helvetica findfont exch scalefont} def",
"/andyb { /Times-Bold findfont exch scalefont} def",
"/Andyb { /Times-Bold findfont exch scalefont} def",
"/andysansb { /Helvetica-Bold findfont exch scalefont} def",
"/AndySansb { /Helvetica-Bold findfont exch scalefont} def",
"/Andysansb { /Helvetica-Bold findfont exch scalefont} def",
"/andyi { /Times-Italic findfont exch scalefont} def",
"/Andyi { /Times-Italic findfont exch scalefont} def",
"/andysansi { /Helvetica-Oblique findfont exch scalefont} def",
"/AndySansi { /Helvetica-Oblique findfont exch scalefont} def",
"/Andysansi { /Helvetica-Oblique findfont exch scalefont} def",
"/andybi { /Times-BoldItalic findfont exch scalefont} def",
"/Andybi { /Times-BoldItalic findfont exch scalefont} def",
"/andysansbi { /Helvetica-BoldOblique findfont exch scalefont} def",
"/AndySansbi { /Helvetica-BoldOblique findfont exch scalefont} def",
"/Andysansbi { /Helvetica-BoldOblique findfont exch scalefont} def",
"/centerstring {",
"	gsave",
"	1 -1 scale",
"	dup stringwidth",
"	pop -5 exch 2 div neg exch",
"	rmoveto",
"	show",
"	grestore",
"} def",
"/box {",
"	gsave",
"	left top	moveto",
"	right top	lineto	% Across",
"	right bottom	lineto	% Down",
"	left  bottom	lineto	% Back",
"	left  top	lineto	% Up",
"	closepath  stroke",
"	grestore",
"} def",
"/roundbox {",
"	gsave",
"	/radius 10 def",
"	left radius add   top moveto",
"	right top    right bottom radius arcto  4 {pop} repeat	% Across",
"	right bottom left  bottom radius arcto  4 {pop} repeat	% Down",
"	left  bottom left  top    radius arcto  4 {pop} repeat	% Back",
"	left  top    right top    radius arcto  4 {pop} repeat	% Up",
"	stroke",
"	grestore",
"} def",
"/fillroundbox {",
"	gsave",
"	/radius 10 def",
"	left radius add   top moveto",
"	right top    right bottom radius arcto  4 {pop} repeat	% Across",
"	right bottom left  bottom radius arcto  4 {pop} repeat	% Down",
"	left  bottom left  top    radius arcto  4 {pop} repeat	% Back",
"	left  top    right top    radius arcto  4 {pop} repeat	% Up",
"	gsave shade setgray fill grestore stroke",
"	grestore",
"} def",
"% End AptView PostScript Prelude  Version 0.0",
"",
"12 andysans setfont",
"% Begin AptView Drawing",
NULL
};

boolean
aptv::OpenPrintStream( FILE		      *file, const char		      *processor, const char		      *format, long		       level )
            {
class aptv *self=this;

  const char		    * const *ptr = PostScript_prelude;

  if ( PrintStream  ||  (PrintStream = (struct aptv_print_stream *)
		calloc( 1, sizeof(struct aptv_print_stream) )) )
    {
    PrintFile = file;
    if ( apts::CompareStrings( processor, "troff" ) == 0 )
      PrintProcessor = Troff;
    if ( apts::CompareStrings( processor, "PostScript" ) == 0 )
      PrintProcessor = PostScript;
    if ( apts::CompareStrings( format, "troff" ) == 0 )
      PrintFormat = Troff;
    if ( apts::CompareStrings( format, "PostScript" ) == 0 )
      PrintFormat = PostScript;
    PrintResolution = (this )->GetHorizontalResolution( );
    DEBUGdt(H-Res,PrintResolution);
    PrintResolutionFactor = 72.0 / PrintResolution;
    DEBUGgt(H-ResFac,PrintResolutionFactor);
    PrintPageInchWidth = 8.5;
    PrintPageInchHeight = 11.0;
    PrintPagePelWidth  = PrintPageInchWidth  * 72;
    PrintPagePelHeight = PrintPageInchHeight * 72;
    PrintLevel = level;
    }
  if ( PrintTopLevel  &&  PrintProcessor == Troff )
    texttroff::BeginDoc( PrintFile );
  if ( PrintProcessor == Troff )
    {
    texttroff::BeginPS( PrintFile, Width(Outer), Height(Outer) );
    PrintPrefix = "\\!";
    }
  if ( PrintProcessor == PostScript )
    {
    PrintPrefix = "";
    }
  if ( PrintTopLevel  ||  PrintProcessor == Troff )
    fprintf( PrintFile, "%s%%!PS-Adobe-2.0 EPSF-1.2\n", PrintPrefix );
  while ( *ptr )  /* Emit PostScript Prolog */
    fprintf( PrintFile, "%s%s\n", PrintPrefix, *ptr++ );
  if ( PrintLandScape )
    {
    fprintf( PrintFile, "%s-90 rotate  -17 0 translate  %% LandScape Orientation\n",
	     PrintPrefix );
    if ( PrintTopLevel )
      fprintf( PrintFile, "%s%g %g translate  %% Centering\n",
	     PrintPrefix, (PrintPagePelHeight - PRF(Width(Outer)))/2,
			  (PrintPagePelWidth  - PRF(Height(Outer)))/2 );
    }
    else
    {
    fprintf( PrintFile, "%s%d %ld translate  %% Portrait Orientation\n",
	     PrintPrefix, 0, -Height(Outer) );
    if ( PrintTopLevel )
      fprintf( PrintFile, "%s%g %g translate  %% Centering\n",
	     PrintPrefix, (PrintPagePelWidth  - PRF(Width(Outer)))/2,
			  (PrintPagePelHeight - PRF(Height(Outer)))/2 );
    }
  fprintf( PrintFile, "%s%g %g translate\n",
	   PrintPrefix, PRF(Left(Outer))+10,PRF(Top(Outer))+10 );
  fprintf( PrintFile, "%s /width %g def /height %g def\n",
	   PrintPrefix, PRF(Width(Outer)), PRF(Height(Outer)) );
  fprintf( PrintFile, "%s newpath 0 0 moveto\n", PrintPrefix );
  fprintf( PrintFile, "%s 0 height lineto width height lineto\n", PrintPrefix );
  fprintf( PrintFile, "%s width 0 lineto 0 0 lineto  newpath\n", PrintPrefix );
  return  TRUE;
  }

void
aptv::ClosePrintStream( )
    {
class aptv *self=this;

  fprintf( PrintFile, "%sgrestore  %% Restore Environment Around AptView Drawing\n", PrintPrefix );
  if ( PrintProcessor == PostScript )
    {
    if ( PrintTopLevel )
      fprintf( PrintFile, "%sshowpage  %% End AptView Drawing\n", PrintPrefix );
    }
  if ( PrintProcessor == Troff )
    {
    texttroff::EndPS( PrintFile,  Width(Outer), Height(Outer) );
    if ( PrintTopLevel )
      texttroff::EndDoc( PrintFile );
    }
  }

void
aptv::PreservePrintState( )
    {
class aptv *self=this;

  fprintf( PrintFile, "%sgsave  %% Preserve Nested Print State\n", PrintPrefix );
  }

void
aptv::RestorePrintState( )
{
    class aptv *self=this;
  fprintf( PrintFile, "%sgrestore  %% Restore Nested Print State\n", PrintPrefix );
  }

void
aptv::SetPrintStream( struct aptv_print_stream  *print_stream )
      {
class aptv *self=this;

  IN(aptv_SetPrintStream);
  PrintStream = print_stream;
  OUT(aptv_SetPrintStream);
  }

void
aptv::SetPrintOrigin( long		       left , long		       top )
      {
class aptv *self=this;

  fprintf( PrintFile, "%s%g %g translate\n", PrintPrefix, PRF(left), PRF(top) );
  }

void
aptv::SetPrintResolution( float	       pixels_per_inch )
      {
class aptv *self=this;

  PrintResolution = pixels_per_inch;
  }

void
aptv::SetPrintUnitDimensions( float	       inch_width , float	       inch_height )
      {
class aptv *self=this;

  PrintUnitInchWidth = inch_width;
  PrintUnitInchHeight = inch_height;
  }

void
aptv::SetPrintPageDimensions( float	       inch_width , float	       inch_height )
      {
class aptv *self=this;

  PrintPageInchWidth = inch_width;
  PrintPageInchHeight = inch_height;
  }

void
aptv::SetPrintOptions( long		       options )
      {
class aptv *self=this;

  if ( options & aptv_PrintPortrait )
    PrintPortrait = true;
  if ( options & aptv_PrintLandScape )
    PrintLandScape = true;
  if ( options & aptv_PrintPreserveAspect )
    PrintPreserveAspect = true;
  if ( options & aptv_PrintFillPage )
    PrintFillPage = true;
  }

void
aptv::SetPrintPath( struct aptv_path   *path )
      {
class aptv *self=this;

  long		      i;

  fprintf( PrintFile, "%s newpath %g %g moveto\n",
	   PrintPrefix, PRF(path->points[0].x), PRF(path->points[0].y) );
  for ( i = 1; i < path->count; i++ )
    fprintf( PrintFile, "%s%g %g lineto\n",
	     PrintPrefix, PRF(path->points[i].x), PRF(path->points[i].y) );
  fprintf( PrintFile, "%s closepath\n", PrintPrefix );
  }

void
aptv::PrintPath( struct aptv_path   *path )
      {
class aptv *self=this;

  if ( path )
    (this)->SetPrintPath(  path );
  fprintf( PrintFile, "%s stroke\n", PrintPrefix );
  }

void
aptv::PrintPathFilled( struct aptv_path   *path )
      {
class aptv *self=this;

  if ( path )
    (this)->SetPrintPath(  path );
  fprintf( PrintFile, "%s fill\n", PrintPrefix );
  }

void
aptv::PrintBox( long		       left , long		       top , long		       width , long		       height, long		       mode )
        {
class aptv *self=this;

  fprintf( PrintFile,"%s/left %g def /top %g def /right %g def /bottom %g def box\n",
	   PrintPrefix, PRF(left), PRF(top),
			PRF(left) + (PRF(width) - 1), PRF(top) + (PRF(height) - 1) );
  }

void
aptv::PrintRoundBox( long		       left , long		       top , long		       width , long		       height, long		       mode )
        {
class aptv *self=this;

  fprintf( PrintFile,"%s/left %g def /top %g def /right %g def /bottom %g def roundbox\n",
	   PrintPrefix, PRF(left), PRF(top),
			PRF(left) + (PRF(width) - 1), PRF(top) + (PRF(height) - 1) );
  }

void
aptv::PrintFilledRoundBox( long		       left , long		       top , long		       width , long		       height, long		       mode , long		       shade )
        {
class aptv *self=this;

  fprintf( PrintFile,"%s/left %g def /top %g def /right %g def /bottom"
	            " %g def /shade %ld def fillroundbox\n",
	    PrintPrefix, PRF(left), PRF(top),
			 PRF(left) + (PRF(width) - 1), PRF(top) + (PRF(height) - 1), shade );
  }

void
aptv::PrintLine( long		       x1 , long		       y1 , long		       x2 , long		       y2 )
      {
class aptv *self=this;

  fprintf( PrintFile,"%s%g %g moveto %g %g lineto stroke\n",
	   PrintPrefix, PRF(x1), PRF(y1), PRF(x2), PRF(y2) );
  }

void
aptv::PrintMoveTo( long		       x , long		       y )
      {
class aptv *self=this;

  fprintf( PrintFile,"%s%g %g moveto\n", PrintPrefix, PRF(x), PRF(y) );
  }

void
aptv::PrintLineTo( long		       x , long		       y )
      {
class aptv *self=this;

  fprintf( PrintFile,"%s%g %g lineto stroke\n", PrintPrefix, PRF(x), PRF(y) );
  }

void
aptv::PrintCircle( long		       x1 , long		       y1 , long		       r )
      {
class aptv *self=this;

  fprintf( PrintFile,"%snewpath %g %g %g 0 360 arc closepath stroke\n",
	    PrintPrefix, PRF(x1), PRF(y1), PRF(r) );
  }

void
aptv::PrintSlice( long		       x , long		       y , long		       r , float	       start , float	       end, long		       shade_n , long		       shade_d, long		       mode )
          {
class aptv *self=this;

  fprintf( PrintFile,"%sgsave newpath %g %g moveto %g %g %g %.3f %.3f arcn closepath 1.415 setmiterlimit\n",
	    PrintPrefix, PRF(x), PRF(y), PRF(x), PRF(y), PRF(r), 360.0 - start, 360.0 - end );
  fprintf( PrintFile,"%sgsave %.3f setgray fill grestore stroke grestore\n",
	    PrintPrefix, 1.0 - ((shade_n * 1.0)/(shade_d * 1.0)) );
  }

void
aptv::SetPrintGrayLevel( float	       level )
      {
class aptv *self=this;

  fprintf( PrintFile, "%s %f setgray\n", PrintPrefix, level );
  }

void
aptv::SetPrintLineWidth( long		       width )
      {
class aptv *self=this;

  fprintf( PrintFile, "%s %g setlinewidth\n", PrintPrefix, PRF(width) );
  }

void
aptv::SetPrintFont( const char		      *font_name )
      {
  char			      family[257], style_name[3];
  long			      style, size;

  IN(aptv_SetPrintFont);
  if ( font_name  &&  *font_name )
    {
class aptv *self=this;

    fontdesc::ExplodeFontName( font_name, family, sizeof(family), &style, &size );
    *style_name = 0;
    if ( style & fontdesc_Bold )    strcat( style_name, "b" );
    if ( style & fontdesc_Italic )  strcat( style_name, "i" );
    fprintf( PrintFile, "%s %ld %s%s  setfont\n", PrintPrefix, size, family, style_name );
    }
  OUT(aptv_SetPrintFont);
  }

void
aptv::ResetPrintFont( )
    {
  IN(aptv_ResetPrintFont);
/*===*/
  OUT(aptv_ResetPrintFont);
  }

void
aptv::PrintString( long		       x, long		       y, const char		      *string, long		       placement )
          {
class aptv *self=this;

  const char		     *place;
/*===
static const char * const places[] =
    {"aptv_LeftTop","aptv_LeftMiddle","apt_LeftBottom",
===*/
place = "centerstring";
  fprintf( PrintFile, "%s%g %g moveto ( %s ) %s\n",
			PrintPrefix, PRF(x), PRF(y), string, place );
  }

view_DSattributes
aptv::DesiredSize( long		       given_width , long		       given_height,
		      enum view_DSpass    pass, long		      *desired_width , long		      *desired_height )
          {
class aptv *self=this;

  view_DSattributes result = (view_DSattributes)(view_WidthFlexible |       view_HeightFlexible);

  IN(aptv_DesiredSize);
  if ( Shrinking  ||  Shrunk )
    {
    DEBUG(Shink);
    Shrinking = false;
    Shrunk = true;
    *desired_width = 50;
    *desired_height = 50;
    }
    else
    {
    if ( OriginalWidth  &&  OriginalHeight )
      {
      DEBUG(Original);
      *desired_width  = OriginalWidth;
      *desired_height = OriginalHeight;
      }
      else
      {
      if ( DimensionWidth  &&  DimensionHeight )
	{
	DEBUG(Dimensioned);
        *desired_width   = DimensionWidth;
        *desired_height  = DimensionHeight;
	}
	else
	{
	DEBUG(Default);
        *desired_width  = 300;
        *desired_height = 300;
	}
      }
    }
  OUT(aptv_DesiredSize);
  return  result;
  }

static
void Size_Enclosures( class aptv	      *self )
    {
  long		      i;
  long			      w, h;
  const char		     *font_name;

  IN(Size_Enclosures);
  if ( Data  &&  ! EnclosuresSuppressed )
    {
    DEBUG(Data Exists);
    Initialized = true;
    for ( i = 0; i < 4; i++ )
      {
      AreaSize(Title,i) = 0;
      if ( (Data)->AreaTitle(  i ) )
	{
	DEBUGst(Title,(Data)->AreaTitle(  i ));
	AreaStringAnchor(Title,i) = (Data)->AreaTitlesAnchor(  i );
	AreaStringCount(Title,i) = (Data)->AreaTitlesCount(  i );
	if ( (Data)->AreaTitleFontName(  i ) )
	  {
	  DEBUGst(Title Font,(Data)->AreaTitleFontName(  i ));
	  font_name = AreaFontName(Title,i) = (Data)->AreaTitleFontName(  i );
	  }
	  else  font_name = DefaultFontName;
        AreaFont(Title,i) = (self)->BuildFont(  font_name, &AreaSize(Title,i) );
	(AreaFont(Title,i))->StringSize( 
				(self )->GetDrawable( ), AreaString(Title,i,0), &w, &h );
	if ( i == LeftArea  ||  i == RightArea )
	  AreaSize(Title,i) = w;
	AreaSize(Title,i) += 2;
	}
      AreaSize(Legend,i) = 0;
      if ( (Data)->AreaLegend(  i ) )
	{
	DEBUGst(Legend,(Data)->AreaLegend(  i ));
	AreaStringAnchor(Legend,i) = (Data)->AreaLegendsAnchor(  i );
	AreaStringCount(Legend,i) = (Data)->AreaLegendsCount(  i );
	if ( (Data)->AreaLegendFontName(  i ) )
	  {
	  DEBUGst(Legend Font,(Data)->AreaLegendFontName(  i ));
	  font_name = AreaFontName(Legend,i) = (Data)->AreaLegendFontName(  i );
	  }
	  else  font_name = DefaultFontName;
        AreaFont(Legend,i) = (self)->BuildFont(  font_name, &AreaSize(Legend,i) );
	(AreaFont(Legend,i))->StringSize( 
				(self )->GetDrawable( ), AreaString(Legend,i,0), &w, &h );
	if ( i == LeftArea  ||  i == RightArea )
	  AreaSize(Legend,i) = w;
	AreaSize(Legend,i) += 2;
	}
      }
    }
  OUT(Size_Enclosures);
  }

static
void Help( class aptv	      *self )
    {
  static const char	      notice[] = "Sorry, No help available for this Object.";
  FILE			     *file;
  struct attributes	      attrs;
  long			      id;

  HelpDisplayed = true;
  BypassUpdate = true;
  (self )->UseWaitCursor( );
  if ( ! HelpText )
    {
    HelpText = new text;
    attrs.next = 0;
    attrs.key = "readonly";
    attrs.value.integer = 0;
    (HelpText )->Clear( );
    if ( HelpString )
      (HelpText)->InsertCharacters(  0, HelpString, strlen( HelpString ) );
    else
    if ( HelpFileName  &&  (file = fopen( HelpFileName, "r" )) )
      {
      filetype::Lookup( file, (char *) 0, &id, 0);
      (HelpText)->Read(  file, id );
      fclose( file );
      }
    else  (HelpText)->InsertCharacters(  0, notice, strlen(notice) );
    attrs.value.integer = 1;
    (HelpText)->SetAttributes(  &attrs );
    HelpTextView = new textview;
    (HelpTextView)->SetDataObject(  HelpText );
    HelpTextView = (HelpTextView )->GetApplicationLayer( );
    }
  Help_FullUpdate( self );
  (self )->UseNormalCursor( );
  }

static
void Help_FullUpdate( class aptv	      *self )
    {
  (self )->ClearClippingRect( );
  (self)->SetTransferMode(  graphic_WHITE );
  (self)->EraseRect(  Bounds(Title) );
  (self)->SetTransferMode(  graphic_BLACK );
  (self)->DrawRect(  Bounds(Title) );
  (HelpTextView)->LinkTree(  self );
  (HelpTextView)->InsertViewSize(  self,
		       Left(Title)+1, Top(Title)+1, Width(Title)-3, Height(Title)-3 );
  (HelpTextView)->FullUpdate(  view_FullRedraw,
		       Left(Title)+1, Top(Title)+1, Width(Title)-3, Height(Title)-3 );
  (HelpTextView)->WantInputFocus(  HelpTextView );
  }

static
void Unhelp( class aptv	      *self )
    {
  HelpDisplayed = false;
  BypassUpdate = false;
  (self )->UseWaitCursor( );
  (self)->WantInputFocus(  self );
  im::ForceUpdate();
  (self )->ClearClippingRect( );
  (HelpTextView)->FullUpdate(  view_Remove, 0,0,0,0 );
  (self)->SetTransferMode(  graphic_WHITE );
  (self)->EraseRectSize(  Left(Title),    Top(Title),
			    Width(Title)+1, Height(Title)+1 );
  (self)->FullUpdate(  view_FullRedraw,
	    Left(Outer), Top(Outer), Width(Outer), Height(Outer) );
  (self )->UseNormalCursor( );
  }

void
aptv::UseNormalCursor( )
{
    class aptv *self=this;

  if ( Cursor  &&  (this )->GetIM( ) )
    {
    (Cursor)->SetStandard(  Cursor_Arrow );
    ((this )->GetIM( ))->SetWindowCursor(  NULL );
    }
  }

void
aptv::UseWaitCursor( )
    {
class aptv *self=this;

  if ( Cursor  &&  (this )->GetIM( ) )
    {
    (Cursor)->SetStandard(  Cursor_Wait );
    ((this )->GetIM( ))->SetWindowCursor(  Cursor );
    im::ForceUpdate();
    }
  }

void
aptv::UseInvisibleCursor( )
    {
class aptv *self=this;

  if ( Cursor  &&  (this )->GetIM( ) )
    {
    (Cursor)->SetGlyph(  CursorFont, '@' );
    ((this )->GetIM( ))->SetWindowCursor(  Cursor );
    im::ForceUpdate();
    }
  }

void
aptv::DrawBoundedString( const char		      *string, class fontdesc    *font, struct rectangle   *bounds, long		       x , long		       y , long		       mode )
            {
  IN(aptv_DrawBoundedString);
  if ( string  &&  *string )
    {
    (this)->SetTransferMode(  graphic_BLACK );
    Draw_String( this, string, font, bounds, x, y, mode );
    }
  OUT(aptv_DrawBoundedString);
  }

void
aptv::ClearBoundedString( char		      *string, class fontdesc    *font, struct rectangle   *bounds, long		       x , long		       y , long		       mode )
            {
  IN(aptv_ClearBoundedString);
  if ( string  &&  *string )
    {
    (this)->SetTransferMode(  graphic_WHITE );
    Draw_String( this, string, font, bounds, x, y, mode );
    }
  OUT(aptv_ClearBoundedString);
  }

static
void Draw_String( class aptv	      *self, const char		      *string, class fontdesc    *font, struct rectangle   *bounds, long		       x , long		       y , long		       mode )
            {
  struct rectangle	      bound_interior;

  IN(Draw_String);
  bound_interior.left = bounds->left + 1;
  bound_interior.top = bounds->top + 1;
  bound_interior.width = bounds->width - 2;
  bound_interior.height = bounds->height - 2;
  (self)->SetClippingRect(  &bound_interior );
  if ( font )
    (self)->SetFont(  font );
  (self)->MoveTo(  x, y );
  (self)->DrawString(  string, mode );
  (self)->SetClippingRect(  Bounds(Body) );
  OUT(Draw_String);
  }

static
void Draw_Enclosures( class aptv	      *self )
    {

  long		      e, a, i, width, center, alignment;
  char		     *string;
  class fontdesc   *font;
  struct rectangle  *bounds;

  IN(Draw_Enclosures);
  (self)->SetTransferMode(  graphic_BLACK );
  (self )->ClearClippingRect( );
  if ( Shrunk )
    {
    (self)->DrawOval(  ShrinkIconBounds );
    if ( ShrinkIcon )
      (self)->DrawBoundedString(  ShrinkIcon, ShrinkIconFont, ShrinkIconBounds,
				    ShrinkIconCenter, ShrinkIconMiddle,  0 );
    if ( ShrinkTitle )
      (self)->DrawBoundedString(  ShrinkTitle, ShrinkTitleFont, ShrinkTitleBounds,
				    ShrinkTitleCenter, ShrinkTitleMiddle, Balanced  );
    }
    else
    {
    if ( ! BorderSuppressed )
      (self)->DrawRRectSize(  Left(Border),  Top(Border),
			           Width(Border), Height(Border), 10, 10 );
    if ( ! ControlSuppressed )
      {
      (self)->DrawBoundedString(  "G", IconFont, ShrinkerBounds,
			      ShrinkerCenter, ShrinkerMiddle, 0  );
      (self)->DrawBoundedString(  "B", IconFont, HelperBounds,
			      HelperCenter, HelperMiddle, 0  );
      }
    if ( ! EnclosuresSuppressed )
      {
      for ( e = 0; e < EnclosureCount; e++ )
      for ( a = 0; a < 4; a++ )
        if ( AreaSize(e,a)  &&  AreaStringCount(e,a) )
	  {
	  width = AreaWidth(e,a) / AreaStringCount(e,a);
	  center = AreaLeft(e,a) +  width / 2;
	  for ( i = 0; i < AreaStringCount(e,a); i++ )
	    {
	    string = AreaString(e,a,i);
	    font = AreaFont(e,a);
	    bounds = AreaBounds(e,a);
	    alignment = AreaMiddle(e,a);
	    (self)->DrawBoundedString(  string, font, bounds,
				   center, alignment, Balanced );
	    center += width;
	    }
	  }
      }
    }
  (self)->SetClippingRect(  Bounds(Body) );
  OUT(Draw_Enclosures);
  }

/*===  HANDLES
    aptv_MoveTo( self, Left(Border) + 9, Top(Border) );
    aptv_DrawLineTo( self, Right(Border) - 9, Top(Border) );
    aptv_MoveTo( self, Right(Border), Top(Border) + 9 );
    aptv_DrawLineTo( self, Right(Border), Bottom(Border) - 9 );
    aptv_MoveTo( self, Right(Border) - 9, Bottom(Border) );
    aptv_DrawLineTo( self, Left(Border) + 9, Bottom(Border) );
    aptv_MoveTo( self, Left(Border), Bottom(Border) - 9 );
    aptv_DrawLineTo( self, Left(Border), Top(Border) + 9 );
    aptv_SetFont( self, IconFont );
    aptv_MoveTo( self, Left(Border), Top(Border) );
    aptv_DrawString( self, "I", NULL );
    aptv_MoveTo( self, Right(Border), Top(Border) );
    aptv_DrawString( self, "K", NULL );
    aptv_MoveTo( self, Right(Border), Bottom(Border) );
    aptv_DrawString( self, "J", NULL );
    aptv_MoveTo( self, Left(Border), Bottom(Border) );
    aptv_DrawString( self, "L", NULL );
===*/

long
aptv::Query( const char		       *query , const char		       *default_response , char		       **response )
      {
  long		      status = ok;
  static char		      buffer[512];

  IN(aptv_Query);
  *buffer = 0;
  *response = buffer;
  if ( (message::AskForString( this, 0, query, default_response,
	 buffer, sizeof buffer ) == -1)  ||  *buffer == 0 )
    {
    (this)->Announce(  "Cancelled." );
    status = failure;
    }
  DEBUGst(Buffer,buffer);
  OUT(aptv_Query);
  return  status;
  }

long
aptv::QueryFileName( const char		      *query, char		     **response )
        {
  enum message_CompletionCode  result;
  static char				path[258];
  static char				buffer[513];
  static char				msg[513];
  long				status = ok;
  char					*tmp = NULL;

  IN(aptv_QueryFileName);
  im::GetDirectory(path);
  strcat(path, "/");
  *buffer = 0;
  *response = buffer;
  result = (enum message_CompletionCode) completion::GetFilename( this, 
			  query,			/* prompt */
			  path,				/* initial string */
			  buffer,			/* working area */
			  sizeof buffer - 1,		/* size of working area */
			  0,				/* want file, not directory */
			  0 );				/* need not be existing file */
  DEBUGdt(Result,result);
  DEBUGst(File-name,buffer);
  if((result != message_Complete && result != message_CompleteValid) || *buffer==0) {
      status = failure;
      (this)->Announce("Cancelled.");
  }
  else {
      if(buffer[strlen(buffer)-1] == '/')
	  buffer[strlen(buffer)-1] = 0;
      if(*(tmp = buffer + strlen(buffer) - 1) == '.') {
	  if((*(tmp-1) == '.') && (*(tmp-2) == '/')) {
	      *(tmp-2) = '\0';
	      if((tmp = (char*)strrchr(buffer,'/')))
		  *tmp = '\0';
	  }
	  else if(*(tmp-1) == '/') 
	      *(tmp-1) = '\0';
      }
      strncpy(path,buffer,sizeof(path) - 1);
      path[sizeof(path) - 1] = 0;
      sprintf(msg,"%s %s",query,path);
      (this)->Announce(msg);
  }
 OUT(aptv_QueryFileName);
  return  status;
}

long
aptv::QueryDirectoryName( const char		      *query, char		     **response )
        {
  enum message_CompletionCode  result;
  static char				path[258];
  static char				buffer[513];
  static char				msg[513];
  long				status = ok;
  char					*tmp = NULL;

  IN(aptv_QueryDirectoryName);
  im::GetDirectory(path);
  strcat(path, "/");
  *buffer = 0;
  *response = buffer;
  result = (enum message_CompletionCode) completion::GetFilename( this, 
			  query,			/* prompt */
			  path,				/* initial string */
			  buffer,			/* working area */
			  sizeof buffer - 1,		/* size of working area */
			  1,				/* want directory, not file */
			  1 );				/* need not be existing file */
  DEBUGdt(Result,result);
  DEBUGst(File-name,buffer);
  if((result != message_Complete && result != message_CompleteValid) || *buffer==0) {
      status = failure;
      (this)->Announce("Cancelled.");
  }
  else{
      if(buffer[strlen(buffer)-1] == '/')
	  buffer[strlen(buffer)-1] = 0;
      if(*(tmp = buffer+strlen(buffer)-1) == '.') {
	  if((*(tmp-1) == '.') && (*(tmp-2) == '/')) {
	      *(tmp-2) = '\0';
	      if((tmp = (char*)strrchr(buffer,'/')))
		  *tmp = '\0';
	  }
	  else if(*(tmp-1) == '/') 
	      *(tmp-1) = '\0';
      }
      strncpy( path, buffer, sizeof(path) - 1 );
      path[sizeof(path) - 1] = 0;
      sprintf(msg,"%s %s",query,path);
      (this)->Announce(msg);
  }
  OUT(aptv_QueryFileName);
  return(status);
  }


long
aptv::Announce( const char		       *message )
      {
  long		      status = ok;

  IN(aptv_Announce);
  if ( message::DisplayString( this, 0, message ) == -1 )
    status = failure;
  im::ForceUpdate();
  OUT(aptv_Announce);
  return  status;
  }
