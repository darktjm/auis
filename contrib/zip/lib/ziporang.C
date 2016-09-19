/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* ziporang.c	Zip Object -- Roundangle					      */
/* Author	TC Peters						      */
/* Information Technology Center		   Carnegie-Mellon University */

/**  SPECIFICATION -- Internal Facility Suite  *********************************

TITLE	Zip Object -- Roundangle

MODULE	ziporang.c

NOTICE	IBM Internal Use Only

DESCRIPTION

.......................

HISTORY
  04/13/88	Created (TC Peters)
  05/01/89	Use symbolic font-names (TCP)
  08/24/89	Remove excess SetTransferMode() activity in Draw() (SCG)
   08/28/89	Modify Build to handle non-refresh of pane on build completion (SCG)

END-SPECIFICATION  ************************************************************/


#include <andrewos.h>
#include <view.H>
#include "zip.H"
#include "zipview.H"
#include "zipedit.H"
#include "zipprint.H"
#include "zipobject.H"
#include "ziporang.H"


ATKdefineRegistry(ziporang, ziporect, NULL);

static long Draw( class ziporang  *self, zip_type_figure  figure, zip_type_pane   pane, long   action );


char
ziporang::Object_Icon( )
    {
  IN(ziporang::Object_Icon);
  OUT(ziporang::Object_Icon);
  return  'I';
  }

char
ziporang::Object_Icon_Cursor( )
    {
  IN(ziporang::Object_Icon_Cursor);
  OUT(ziporang::Object_Icon_Cursor);
  return  'S';
  }

char
ziporang::Object_Datastream_Code( )
    {
  IN(ziporang::Object_Datastream_Code);
  OUT(ziporang::Object_Datastream_Code);
  return  'N';
  }

long
ziporang::Show_Object_Properties( zip_type_pane		   pane, zip_type_figure		   figure )
{
  (this->view_object)->Announce(  "Draw RoundAngle from Upper-left to Lower-right." );
  return  zip_ok;
  }

long
ziporang::Build_Object( zip_type_pane	   pane, enum view_MouseAction   action , long   x , long  y , long  clicks, zip_type_point		   X , zip_type_point  Y )
          {
  long				  status = zip_ok;
  zip_type_figure		  figure;

  IN(ziporang::Build_Object);
  switch ( action )
    {
    case view_LeftDown:
      if ( (status =
        (this->data_object)->Create_Figure(  &CurrentFigure, NULL, zip_roundangle_figure,
			 CurrentImage, NULL )) == zip_success )
	{
        (this)->Set_Object_Point(  CurrentFigure,	zip_figure_origin_point, X, Y );
        (this)->Set_Object_Point(  CurrentFigure, zip_figure_auxiliary_point, X, Y );
	CurrentFigure->zip_figure_zoom_level = pane->zip_pane_zoom_level;
        pane->zip_pane_edit->zip_pane_edit_last_point_id = zip_figure_auxiliary_point;
	(this->data_object)->Set_Figure_Shade(  CurrentFigure,
			      pane->zip_pane_edit->zip_pane_edit_current_shade );
	(this->view_object)->Set_Pane_Painting_Mode(  pane, zipview_paint_inverted );
	}
      break;
    case view_LeftUp:
      if ( ( figure = CurrentFigure ) )
	{
	(this->view_object)->Set_Pane_Painting_Mode(  pane, zip_default );
	if ( figure_x_point == figure_x_points(0)  &&
	     figure_y_point == figure_y_points(0) )
	  {
	  (this->edit_object)->Delete_Figure(  figure, pane ); 
	  }
          else
	  {
	  (this->view_object)->Draw_Figure(  CurrentFigure, pane );
	  }
	}
        break;
    case view_LeftMovement:
      if ( CurrentFigure )
	{
	(this->view_object)->Draw_Figure(  CurrentFigure, pane );
        (this)->Set_Object_Point(  CurrentFigure, zip_figure_auxiliary_point, X, Y );
	(this->view_object)->Draw_Figure(  CurrentFigure, pane );
	}
      break;
    default:
      break;
    }
  OUT(ziporang::Build_Object);
  return  status;
  }

long
ziporang::Draw_Object( zip_type_figure		   figure, zip_type_pane		   pane )
{
  long				  status = zip_ok;

  if ( (this->view_object)->Condition(  pane, figure, zip_draw ) )
    status = Draw( this, figure, pane, zip_draw );
  OUT(ziporang::Draw_Object);
  return  status;
  }

long
ziporang::Clear_Object( zip_type_figure		   figure, zip_type_pane		   pane )
{
  long				  status = zip_ok;

  IN(ziporang::Clear_Object);
  if ( (this->view_object)->Condition(  pane, figure, zip_clear ) )
    status = Draw( this, figure, pane, zip_clear );
  OUT(ziporang::Clear_Object);
  return  status;
  }

static
long Draw( class ziporang *self, zip_type_figure  figure, zip_type_pane  pane, long   action )
{
  long				status = zip_ok;
  long				radius, left, top, width, height;
  unsigned char			pattern = 0, shade;

  IN(Draw);
  left   = window_x_point;		top    = window_y_point;
  width  = window_x_points(0) - left;	height = window_y_points(0) - top;
  if ( width < 0 )
    { left = window_x_points(0);  width = -width; }
  if ( height < 0 )
    { top = window_y_points(0);   height = -height; }
  if ( abs(figure_x_point - figure_x_points(0)) >
       abs(figure_y_point - figure_y_points(0)) )
    radius =  abs(window_y_point - window_y_points(0) ) >> 2;
    else
    radius =  abs(window_x_point - window_x_points(0) ) >> 2;
  if ( figure->zip_figure_mode.zip_figure_mode_shaded  ||
       figure->zip_figure_mode.zip_figure_mode_patterned )
    {
    if ( self->view_object->mouse_action != view_LeftMovement  &&  action == zip_draw )
      {
      if ( figure->zip_figure_mode.zip_figure_mode_patterned  &&
	   (pattern = (self->data_object)->Contextual_Figure_Pattern(  figure )) )
        { DEBUGct(Pattern,pattern);
	(self->view_object)->FillTrapezoid( left,top,width, left,top+height+1,width+1,
	    (self->view_object)->Define_Graphic( 
	    (self->view_object)->Select_Contextual_Figure_Font(  figure ), pattern ));
        }
        else
	/* Shade of '0' means Transparent --- Shade of '1' means White */
	if ( (shade = (self->data_object)->Contextual_Figure_Shade(  figure )) >= 1  &&
	      shade <= 100 )
	  { DEBUGdt(Shade,shade);
	  if ( (shade = ('0' + ((shade + 10) / 10)) - 1) > '9' )  shade = '9';
	  DEBUGdt(Shade-index,shade);
	  (self->view_object)->Ensure_Fill_Attributes(  figure );
	  (self->view_object)->FillRRectSize(  left, top, width+1, height+1, radius, radius,
				 (self->view_object)->Define_Graphic( (self->data_object)->Define_Font(
				     ShadeFontName, NULL ), shade ) );
	  }
      }
      else
      if ( action == zip_clear )
	{
	(self->view_object)->FillRRectSize(  left, top, width, height, radius, radius, graphic_WHITE );
	}
    }
  if ( (self->view_object)->Ensure_Line_Attributes(  figure ) == zip_ok )
    (self->view_object)->DrawRRectSize(  left, top, width, height, radius, radius );
  if ( ExposePoints )	    (self)->Expose_Object_Points(  figure, pane );
  if ( HighlightPoints )    (self)->Highlight_Object_Points(  figure, pane );
  OUT(Draw);
  return  status;
  }

long
ziporang::Print_Object( zip_type_figure figure, zip_type_pane	 pane )
{
  class ziporang *self=this;
  long				  status = zip_ok;
  long				  radius, left, right, top, bottom;
  long                                    temp1, temp2;

  IN(ziporang::Print_Object);
  left = print_x_point;       top = print_y_point;
  right = print_x_points(0);  bottom = print_y_points(0);
  if ( right < left )
    { DEBUG(X-Flipped);
    left = right;
    right = print_x_point;
    }
  if ( bottom < top )
    { DEBUG(Y-Flipped);
    top = bottom;
    bottom = print_y_point;
    }
  if ( abs(figure_x_point - figure_x_points(0)) >
       abs(figure_y_point - figure_y_points(0)) ) {
    temp1 = print_y_point;
    temp2 = print_y_points(0);
    radius =  abs(temp1 - temp2) >> 2;
  }
  else  {
      temp1 = print_x_point;
      temp2 = print_x_points(0);
      radius =  abs(temp1 - temp2) >> 2;
  }
  (this->print_object)->Set_Shade(  figure->zip_figure_fill.zip_figure_shade ); 
  (this->print_object)->Ensure_Line_Attributes(  figure );
  (this->print_object)->Draw_Round_Rectangle(  left, top, right, bottom, radius, radius );
  OUT(ziporang::Print_Object);
  return  status;
  }
