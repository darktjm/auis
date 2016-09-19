ATK_INTER
#ifndef _aptv_H_
#define _aptv_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/***  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Apt View-object

MODULE	aptv.H

VERSION	0.0

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

END-SPECIFICATION  ************************************************************/

#define  aptv_VERSION		    1

#include "rect.h"

/*  Options  */

#define  aptv_Iconified		    (1<<0)
#define  aptv_SuppressControl	    (1<<1)
#define  aptv_SuppressBorder	    (1<<2)
#define  aptv_SuppressEnclosures    (1<<3)

/*  Print Options  */
#define  aptv_PrintPortrait	    (1<<0)
#define  aptv_PrintLandScape	    (1<<1)
#define  aptv_PrintLandscape	    (1<<1)
#define  aptv_PrintPreserveAspect   (1<<2)
#define  aptv_PrintFillPage	    (1<<3)

struct aptv_area
  {
  long				      size;
  struct rectangle		      bound;
  class fontdesc		     *font;
  const char			     *font_name;
  long				      strings_count;
  char				  **strings;
  };

struct aptv_enclosure
  {
  struct rectangle		      bound;
  struct aptv_area		      areas[4];
  };

#define  EnclosureCount		      6

struct aptv_point
  {
  long				      x;
  long				      y;
  };

struct aptv_path
  {
  long				      count;
  struct aptv_point		      points[1];
  };


struct aptv_options
  {
  unsigned int		   controls_suppressed	    : 1;
  unsigned int		   border_suppressed	    : 1;
  unsigned int		   enclosures_suppressed    : 1;
  unsigned int		   initialize_iconified	    : 1;
  };

struct aptv_states
  {
  unsigned int		   initialized		    : 1;
  unsigned int		   inputfocus		    : 1;
  unsigned int		   shrinking		    : 1;
  unsigned int		   shrunk		    : 1;
  unsigned int		   help_displayed	    : 1;
  };

  class aptv;
  typedef void (*aptv_printfptr)(class aptv *self);
  
#include <view.H>
class aptv : public  view
  {
public:
	virtual ATKregistryEntry *ATKregistry();



   view_DSattributes DesiredSize( long width, long height, enum view_DSpass pass, long *dWidth, long *dheight )
	 ;
   void FullUpdate( enum view_UpdateType type, long left, long top, long width, long height );
   class view * Hit( enum view_MouseAction action, long x, long y, long n )	 ;
   void SetDataObject( class dataobject *d );



  virtual void SetOptions( long options );
  virtual void SetDimensions( long width, long height );
  virtual class fontdesc * BuildFont( const char *font_name, long *height )		     ;
  virtual void DrawBoundedString( const char *string, class fontdesc *font,
	       struct rectangle *bounds, long center, long middle, long mode );
  virtual void ClearBoundedString( char *string, class fontdesc *font,
		struct rectangle *bounds, long center, long middle, long mode );
  virtual boolean Within( long x, long y, struct rectangle *bounds )	     ;
  virtual void SetShrinkIcon( const char icon, const char *icon_font_name, const char *title, const char *title_font_name );
  virtual void Shrink();
  virtual void ShrinkView( class aptv *aptv );
  virtual void Expand();
  virtual void ExpandView( class aptv *aptv );
  virtual void ClearBody();

  virtual void PrintObject( FILE *file, const char *processor, const char *finalFormat, boolean topLevel, aptv_printfptr printer );
  virtual void PrintContinue();
  virtual boolean OpenPrintStream( FILE *file, const char *processor, const char *format, long level )  ;
  virtual void ClosePrintStream();
  virtual void SetPrintStream( struct aptv_print_stream *stream );
  virtual void PreservePrintState();
  virtual void RestorePrintState();
  virtual void PrintBox( long left, long top, long width, long height, long mode );
  virtual void PrintRoundBox( long left, long top, long width, long height, long mode );
  virtual void PrintFilledRoundBox( long left, long top, long width, long height, long mode, long shade );
  virtual void PrintLine( long x1, long y1, long x2, long y2 );
  virtual void PrintCircle( long x1, long y1, long radius );
  virtual void PrintSlice( long x1, long y1, long radius, float start_angle, float end_angle,
	       long shade_numerator, long shade_denominator, long mode  );
  virtual void PrintMoveTo( long x, long y );
  virtual void PrintLineTo( long x, long y );
  virtual void SetPrintOrigin( long left, long top );
  virtual void SetPrintUnitDimensions( float inch_width, float inch_height );
  virtual void SetPrintPageDimensions( float inch_width, float inch_height );
  virtual void SetPrintResolution( float pixels_per_inch );
  virtual void SetPrintOptions( long options );
  virtual void SetPrintLineWidth( long width );
  virtual void SetPrintGrayLevel( float level );
  virtual void SetPrintPath( struct aptv_path *path );
  virtual void PrintPath( struct aptv_path *path );
  virtual void PrintPathFilled( struct aptv_path *path );
  virtual void SetPrintFont( const char *font_name );
  virtual void ResetPrintFont( );
  virtual void PrintString( long x1, long y1, const char *string, long mode );

  virtual void SetHelpString( const char *string );
  virtual void SetHelpFileName( const char *file_name);
  virtual void UseWaitCursor();
  virtual void UseNormalCursor();
  virtual void UseInvisibleCursor();
  virtual long Query(const char *query , const char *default_response , char **response );
  virtual long QueryFileName(const char *query, char **response );
  virtual long QueryDirectoryName(const char *query, char **response );
  virtual long Announce(const char *message );



  inline class fontdesc * IconFont()	    { return ((this)->icon_font); }
  inline class fontdesc * CursorFont()	    { return ((this)->cursor_font); }
  inline struct aptv_print_stream * PrintStream()	    { return ((this)->print_stream); }
  inline struct rectangle * Bounds()	    { return (&(this)->enclosures[0].bound); }
  inline struct rectangle * BodyBounds()	    { return (&(this)->enclosures[5].bound); }
  inline long BodyLeft()	    { return ((this)->BodyBounds()->left); }
  inline long BodyTop()	    { return ((this)->BodyBounds()->top); }
  inline long BodyWidth()	    { return ((this)->BodyBounds()->width); }
  inline long BodyHeight()	    { return ((this)->BodyBounds()->height); }
  inline long BodyCenter()	    { return ((this)->BodyLeft() + (this)->BodyWidth()/2); }
  inline long BodyRight()	    { return ((this)->BodyLeft() + (this)->BodyWidth() - 1); }
  inline long BodyMiddle()	    { return ((this)->BodyTop()  + (this)->BodyHeight()/2); }
  inline long BodyBottom()	    { return ((this)->BodyTop()  + (this)->BodyHeight() - 1); }
  inline char BypassUpdate()    { return ((this)->bypass_update); }



  
	aptv();
  
	virtual ~aptv();



  class apt			    *data_object;
  struct aptv_options		     options;
  struct aptv_states		     states;
  class fontdesc		    *icon_font, *cursor_font, *default_font;
  class text			    *help_text;
  class	view			    *help_textview;
  const char			    *help_string;
  const char			    *help_file_name;
  struct aptv_enclosure		     enclosures[EnclosureCount];
  struct rectangle		     shrinker_bounds;
  struct rectangle		     helper_bounds;
  long				     original_width, original_height;
  class graphic		    *gray_fill, *white_fill;
  struct aptv_print_stream	    *print_stream;
  char				     shrink_icon[2];
  class fontdesc		    *shrink_icon_font;
  struct rectangle		     shrink_icon_bounds;
  char				    *shrink_title;
  class fontdesc		    *shrink_title_font;
  struct rectangle		     shrink_title_bounds;
  long				     dimension_width, dimension_height;
  char				     bypass_update;
  class cursor			    *cursor;
  };


/*
    $Log: aptv.H,v $
// Revision 1.7  1994/11/30  20:42:06  rr2b
// Start of Imakefile cleanup and pragma implementation/interface hack for g++
//
// Revision 1.6  1993/12/21  20:36:48  rr2b
// Fixed so that the help object isn't kept as a textview *
// since it may really be any view *.
// BUG
//
 * Revision 1.5  1993/09/28  04:52:23  rr2b
 * Fixed view_DSattributes to be an int not an enum.
 *
 * Revision 1.4  1993/06/10  20:21:03  rr2b
 * Great atk_inited removal.
 *
 * Revision 1.3  1993/05/18  15:22:09  rr2b
 * Corrected for the proper type of the strings member of the apt_strings structure.
 * (It was a *(*)[], but it is really just a **.)
 *
 * Revision 1.2  1993/05/13  14:19:29  rr2b
 * Fixed size member of struct area to be a long instead of a short.
 *  Fixed char *((*)[]) types to be char ***.
 * Fixed a syntax error in the declaration of SetDataObject (also fixed its argument type.)
 * Fixed short *height to be long *height in method aptv::BuildFont.
 * Fixed MISSING_ARGUMENT_TYPE in PrintObject, OpenPrintStream, Query,
 * QueryFileName, QueryDirectoryName and Announce methods.
 * Added typedef for the printer function to PrintObject.
 *
*Revision 1.5  1992/12/14  23:20:33  rr2b
*add $Logs back after disclaimerization took them out
*
Revision 1.3  1991/09/12  19:20:06  bobg
Update copyright notice

Revision 1.2  1989/06/05  18:27:37  gk5g
Added method aptv_QueryDirectoryName().

Revision 1.1  89/04/28  17:45:48  tom
Initial revision

*/

#endif /* _aptv_H_ */
