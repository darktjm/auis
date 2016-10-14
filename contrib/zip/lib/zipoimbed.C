/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* zipoimbd.c	Zip Object -- Imbed					      */
/* Author	TC Peters						      */
/* Information Technology Center		   Carnegie-Mellon University */

/**  SPECIFICATION -- Internal Facility Suite  *********************************

TITLE	Zip Object -- Imbed

MODULE	zipoimbd.c

NOTICE	IBM Internal Use Only

DESCRIPTION

.......................

HISTORY
  04/13/88	Created (TC Peters)
  11/23/88	Change object-type query format (TCP)
  05/01/89	Use symbolic font-names (TCP)
  06/30/89	Use Announce only when View-object exists (TCP)
		Various ensurance checks for existence of Object.
  08/07/89	Add Object_Modified override (TCP)
		Correct Print language/processor references.

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include <view.H>
#include <dataobject.H>
#include "zip.H"
#include "zipview.H"
#include "zipedit.H"
#include "zipprint.H"
#include "zipobject.H"
#include "zipoimbed.H"

struct imbed
  {
  class dataobject			 *data_object;
  class view				 *view_object;

  };

ATKdefineRegistryNoInit(zipoimbed, ziporect);

static long Load_Object( class zipoimbed *self, zip_type_figure  figure, const char *name, boolean announce );
static long Draw( class zipoimbed *self, zip_type_figure figure, zip_type_pane pane, long  action );


zipoimbed::zipoimbed( )
      {
  IN(zipoimbed_InitializeObject);
  this->no_outline = false;
  OUT(zipoimbed_InitializeObject);
  THROWONFAILURE(  true);
  }

void
zipoimbed::Destroy_Object( zip_type_figure		   figure )
      {
  struct imbed			 *imbed =
		     (struct imbed *)figure->zip_figure_datum.zip_figure_anchor;

  IN(zipoimbed_Destroy_Object);
  (imbed->view_object )->Destroy();
  (imbed->data_object )->Destroy();
  OUT(zipoimbed_Destroy_Object);
  }

char
zipoimbed::Object_Icon( )
    {
  IN(zipoimbed::Object_Icon);
  OUT(zipoimbed::Object_Icon);
  return  'E';
  }

char
zipoimbed::Object_Icon_Cursor( )
    {
  IN(zipoimbed::Object_Icon_Cursor);
  OUT(zipoimbed::Object_Icon_Cursor);
  return  'B';
  }

char
zipoimbed::Object_Datastream_Code( )
    {
  IN(zipoimbed::Object_Datastream_Code);
  OUT(zipoimbed::Object_Datastream_Code);
  return  'I';
  }

long
zipoimbed::Show_Object_Properties( zip_type_pane  pane, zip_type_figure  figure )
{
  char					 *response;

  (this->view_object)->Set_Pane_Cursor(  pane, '@', IconFontName );
  CurrentFigure = NULL;
  (this->view_object)->Query(  "Outline Imbedded Object? [Yes]: ",
					NULL, &response );
  if ( *response == 'n'  ||  *response == 'N' )
      this->no_outline = true;
  (this->view_object)->Announce(  "Draw Imbedded Object from Upper-left to Lower-right." );
  (this->view_object)->Set_Pane_Cursor(  pane, 'B', CursorFontName );
  return  zip_ok;
  }

long
zipoimbed::Build_Object( zip_type_pane		   pane, enum view::MouseAction				   action , long				   x , long				   y , long				   clicks, zip_type_point		   X , zip_type_point		   Y )
          {
  long				  status = zip_ok;
  zip_type_figure		  figure;
  char					 *response;

  IN(zipoimbed::Build_Object);
  switch ( action )
    {
    case view::LeftDown:
      if ( (status =
        (this->data_object)->Create_Figure(  &CurrentFigure, NULL, zip_imbed_figure,
			   CurrentImage, 0 )) == zip_success )
	{
        (this)->Set_Object_Point(  CurrentFigure, zip_figure_origin_point,X, Y );
        (this)->Set_Object_Point(  CurrentFigure, zip_figure_auxiliary_point, X, Y );
	CurrentFigure->zip_figure_zoom_level = pane->zip_pane_zoom_level;
        pane->zip_pane_edit->zip_pane_edit_last_point_id = zip_figure_auxiliary_point;
	if ( this->no_outline )
	    CurrentFigure->zip_figure_style = 1;
	}
      break;
    case view::LeftUp:
      (this->view_object)->Set_Pane_Cursor(  pane, 'B', CursorFontName );
      if ( ( figure = CurrentFigure ) )
	{
	if ( figure_x_point == figure_x_points(0)  &&
	     figure_y_point == figure_y_points(0) )
	  {
	  (this->edit_object)->Delete_Figure(  figure, pane );
          break;
	  }
	  else
	  {
	  (this->view_object)->Query(  "Enter Object Type [text]: ",
					NULL, &response );
          const char *ot = response;
	  if ( response == NULL  ||  *response == 0 )
	    ot = "text";
	  (this->view_object )->Use_Working_Pane_Cursors( );
	  if ( (status = Load_Object( this, figure, ot, true )) != zip_ok )
    	    (this->edit_object)->Delete_Figure(  figure, pane );
	  (this->view_object )->Use_Normal_Pane_Cursors( );
	  }
	}
      /* Fall-thru */
    case view::LeftMovement:
      if ( CurrentFigure  &&  status == zip_ok )
	{
	(this->view_object)->Set_Pane_Painting_Mode(  pane, zipview_paint_inverted );
	(this->view_object)->Draw_Figure(  CurrentFigure, pane );
        (this)->Set_Object_Point(  CurrentFigure, zip_figure_auxiliary_point, X, Y );
	(this->view_object)->Draw_Figure(  CurrentFigure, pane );
	(this->view_object)->Set_Pane_Painting_Mode(  pane, zip_default );
	}
      break;
    default:
      break;
    }
  OUT(zipoimbed::Build_Object);
  return  status;
  }

class view *
zipoimbed::Object_Hit( zip_type_figure		   figure, enum view::MouseAction	   action, long			           x , long			           y , long			           clicks )
          {
  struct imbed			 *imbed = (struct imbed *)
						   figure->zip_figure_datum.zip_figure_anchor;
  class view			 *view = NULL;
  zip_type_pane		  pane;
  zip_type_figure		  next = figure->zip_figure_next;

  IN(zipoimbed::Object_Hit );
  if ( next )
    { DEBUG(Popping Figure to Front);
    (this->data_object)->Unhook_Figure(  figure );
    while ( next  &&  next->zip_figure_next ) next = next->zip_figure_next;
    (this->data_object)->Hook_Figure(  figure, next );
    pane = (this->view_object)->Which_Pane(  x, y );
    if ( (this->view_object)->Condition(  pane, figure, zip_draw ) )
      Draw( this, figure, pane, zip_draw );
    }
  if ( imbed )
    view = (imbed->view_object)->Hit(  action,
		   (imbed->view_object)->EnclosedXToLocalX(  x ),
		   (imbed->view_object)->EnclosedYToLocalY(  y ), clicks );
  OUT(zipoimbed::Object_Hit );
  return  view;
  }

long
zipoimbed::Read_Object( zip_type_figure		   figure )
      {
  long				  status = zip_ok;

  IN(zipoimbed::Read_Object);
  status = (this->data_object)->Read_Figure(  figure );
  OUT(zipoimbed::Read_Object);
  return  status;
  }

long
zipoimbed::Read_Object_Stream( zip_type_figure		   figure, FILE				  *file, long				   id )
          {
  long				  status = zip_ok, imbed_id;
  struct imbed			 *imbed = (struct imbed *)
						   figure->zip_figure_datum.zip_figure_anchor;
  char					  line[512],
					  name[100], id_string[20];
  char				 *s, *t;

  IN(zipoimbed::Read_Object_Stream);
  fgets( line, sizeof(line) - 1, file );
  DEBUGst(Line,line);
  if ( strncmp( line, "begindata", 9 ) == 0 )
    { DEBUG(Begindata OK);
    if ( ( s = (char *) index( line, '{' ) ) )
      { DEBUG(LP OK);
      t = name;  s++;
      while ( *s != ',' )  *t++ = *s++;  *t = 0;
      DEBUGst(Load-name,name);
      t = id_string;  s++;
      while ( *s != '}' )  *t++ = *s++;  *t = 0;
      DEBUGst(ID-string,id_string);
      imbed_id = atoi( id_string );
      DEBUGdt(ID,imbed_id);
      if ( (status = Load_Object( this, figure, name, false )) == zip_ok )
        { DEBUG(Load OK);
        imbed = (struct imbed *) figure->zip_figure_datum.zip_figure_anchor;
	status = (imbed->data_object)->Read(  file, imbed_id );
        }
      }
      else
      { DEBUG(LP ERROR);
      status = zip_failure;/*==s/b ???===*/
      }
    }
    else
    { DEBUG(Begindata EROR);
    status = zip_failure;/*==s/b ???===*/
    }
  DEBUGdt(Status,status);
  OUT(zipoimbed::Read_Object_Stream);
  return  status;
  }

static
long Load_Object( class zipoimbed		  *self, zip_type_figure		   figure, const char				  *name, boolean			   announce )
          {
  long				  status = zip_ok;
  struct imbed			 *imbed;
  class dataobject		 *data_object;
  class view			 *view_object;
  const char				 *view_name;
  char					  msg[512];

  IN(Load_Object);
  DEBUGst(Load-name,name);
  if ( announce  &&  self->view_object )
    {
    sprintf( msg, "Loading '%s' Object", name );
    (self->view_object)->Announce(  msg );
    }
  figure->zip_figure_datum.zip_figure_anchor = NULL;
  if ( ( data_object = (class dataobject *) ATK::NewObject( name ) ))
    { DEBUG(DataObject Loaded);
    view_name = (data_object )->ViewName( );
    DEBUGst(ViewName,view_name);
    if ( ( view_object = (class view *) ATK::NewObject( view_name ) ) )
      { DEBUG(ViewObject Loaded);
      figure->zip_figure_datum.zip_figure_anchor = (char *) calloc( 1, sizeof(struct imbed) );
      imbed = (struct imbed *) figure->zip_figure_datum.zip_figure_anchor;
      imbed->data_object = data_object;
      imbed->view_object = view_object;
      (view_object)->SetDataObject(  data_object );
      if ( announce  &&  self->view_object )  (self->view_object)->Announce(  "Done" );
      }
      else
      { DEBUG(ViewObject Load ERROR);
      sprintf( msg, "Unable to Load '%s'ViewObject", name );
      if ( self->view_object )
        (self->view_object)->Announce(  msg );
        else  fprintf( stderr, "Zip: %s\n", msg );
      status = zip_failure; /*===s/b/object not found===*/
      }
    }
    else
    { DEBUG(DataObject Load ERROR);
    sprintf( msg, "Unable to Load '%s' DataObject", name );
    if ( self->view_object )
      (self->view_object)->Announce(  msg );
      else  fprintf( stderr, "Zip: %s\n", msg );
    status = zip_failure; /*===s/b/object not found===*/
    }
  DEBUGdt(Status,status);
  OUT(Load_Object);
  return  status;
  }

long
zipoimbed::Write_Object( zip_type_figure		   figure )
      {
  long				  status = zip_ok;
  struct imbed			 *imbed =
		     (struct imbed *)figure->zip_figure_datum.zip_figure_anchor;

  IN(zipoimbed::Write_Object);
  if ( ! figure->zip_figure_state.zip_figure_state_deleted  &&
       imbed  &&  imbed->data_object )
    {
    figure->zip_figure_datum.zip_figure_anchor = NULL;
    if ( (status = (this->data_object)->Write_Figure(  figure )) == zip_ok )
      (imbed->data_object)->Write( 
		    figure->zip_figure_image->zip_image_stream->zip_stream_file,
		    this->data_object->write_stream_id,
		    1 /* Indicate Not Top-level */ );
    figure->zip_figure_datum.zip_figure_anchor = (char *) imbed;
    DEBUGdt(Status,status);
    }
  OUT(zipoimbed::Write_Object);
  return  status;
  }

long
zipoimbed::Object_Modified( zip_type_figure		   figure  )
      {
  struct imbed			 *imbed =
		     (struct imbed *)figure->zip_figure_datum.zip_figure_anchor;
  long				  modified = 0;

  IN(zipoimbed_Object_Modified);
  if ( imbed  &&  imbed->data_object )
    modified = (imbed->data_object )->GetModified( );
  OUT(zipoimbed_Object_Modified);
  return  modified;
  }

long
zipoimbed::Draw_Object( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(zipoimbed::Draw_Object);
  if ( (this->view_object)->Condition(  pane, figure, zip_draw ) )
    status = Draw( this, figure, pane, zip_draw );
  OUT(zipoimbed::Draw_Object);
  return  status;
  }

long
zipoimbed::Clear_Object( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(zipoimbed::Clear_Object);
  if ( (this->view_object)->Condition(  pane, figure, zip_clear ) )
    status = Draw( this, figure, pane, zip_clear );
  OUT(zipoimbed::Clear_Object);
  return  status;
  }

static
long Draw( class zipoimbed		  *self, zip_type_figure		   figure, zip_type_pane		   pane, long				   action )
          {
  long				  status = zip_ok;
  struct imbed			 *imbed;
  long				  left, top, width, height,
					  L, T, W, H;
  short			  transfer_mode, shade;

  IN(Draw);
  left  = L = window_x_point;		 top    = T  = window_y_point;
  width = W = window_x_points(0) - left; height = H = window_y_points(0) - top;
  if ( width < 0 )
    { left  = L = window_x_points(0);  width = W = -width; }
  if ( height < 0 )
    { top  = T = window_y_points(0);   height = H = -height; }
  DEBUGdt(L,L);DEBUGdt(T,T);DEBUGdt(W,W);DEBUGdt(H,H);
  transfer_mode = (self->view_object )->GetTransferMode( );
  if ( self->view_object->mouse_action != view::LeftMovement  &&
       (imbed = (struct imbed *)figure->zip_figure_datum.zip_figure_anchor)  &&
       action == zip_draw )
    {
    if ( (shade = (self->data_object)->Contextual_Figure_Shade(  figure )) >= 1  &&
	  shade <= 100 )
      { DEBUGdt(Shade,shade);
      if ( (shade = ('0' + ((shade + 10) / 10)) - 1) > '9' )  shade = '9';
      DEBUGdt(Shade-index,shade);
      (self->view_object)->SetTransferMode(  graphic::COPY );
      (self->view_object)->FillRectSize(  left,top, width,height,
        (self->view_object)->Define_Graphic(  (self->data_object)->Define_Font(  ShadeFontName, NULL ), shade ));
      }
    (self->view_object)->SetTransferMode(  graphic::BLACK );
    (self->view_object)->Set_Pane_Clip_Area(  pane );
    DEBUG(LinkTree);
    (imbed->view_object)->LinkTree(  self->view_object );
    DEBUG(InsertViewSize);
    (imbed->view_object)->InsertViewSize(  self->view_object,
			 L + 1, T + 1, W - 2, H - 2 );
    DEBUG(FullUpdate); /*=== FORCE CLIPING TO INSIDE PANE SOMEHOW ===*/
    (imbed->view_object)->FullUpdate(  view::FullRedraw,
		     L + 1, T + 1, W - 2, H - 2 );
    (self->view_object)->Set_Pane_Clip_Area(  pane );
    }
    else
    if ( action == zip_clear )
      {
      (self->view_object)->SetTransferMode(  graphic::WHITE );
      (self->view_object)->EraseRectSize(  left+1, top+1, width-1, height-1 );
      }
  (self->view_object)->SetTransferMode(  transfer_mode );
  if ( ((self->data_object)->Contextual_Figure_Line_Width(  figure ) > 0  &&  figure->zip_figure_style == 0)  ||
       self->view_object->mouse_action == view::LeftMovement )
    (self->view_object)->DrawRectSize(  left, top, width, height );
  if ( ExposePoints )
    (self)->Expose_Object_Points(  figure, pane );
  if ( HighlightPoints )
    (self)->Highlight_Object_Points(  figure, pane );

  OUT(Draw);
  return  status;
  }

long
zipoimbed::Print_Object( zip_type_figure figure, zip_type_pane pane )
{
  class zipoimbed *self=this;
  long				  status = zip_ok;
  const char			 *language, *processor;
  struct imbed			 *imbed = (struct imbed *) figure->zip_figure_datum.zip_figure_anchor;

  IN(zipoimbed::Print_Object);
  fprintf( zipprint_Printing_File( this->print_object ), "%s %s\n%s %ld %ld %s\n",
	   zipprint_Printing_Prefix( this->print_object ),
	   "gsave   % Save Zip Environment Surrounding Imbedded Object",
	   zipprint_Printing_Prefix( this->print_object ),
	   print_x_point/100, print_y_points(0)/100,
	   "translate 1 -1 scale  % Set Zip Imbedded Object Environment"
	   );
  if ( imbed  &&  imbed->view_object )
    {
    processor = "troff";
    if ( Printing->zip_printing_processor == zip_postscript )
      processor = "PostScript";
    language = "troff";
    if ( Printing->zip_printing_language == zip_postscript )
      language = "PostScript";
    (imbed->view_object)->Print( Printing->zip_printing_file, processor, language, 0 );
    }
  fprintf( zipprint_Printing_File( this->print_object ), "%s %s\n",
	   zipprint_Printing_Prefix( this->print_object ),
	   "grestore  % Restore Zip Environment Surrounding Imbedded Object");
  OUT(zipoimbed::Print_Object);
  return  status;
  }
