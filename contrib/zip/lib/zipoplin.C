/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* zipoplin.c	Zip Object -- Ploy-Line					      */
/* Author	TC Peters						      */
/* Information Technology Center		   Carnegie-Mellon University */

/**  SPECIFICATION -- Internal Facility Suite  *********************************

TITLE	Zip Object -- PolyLine

MODULE	zipoplin.c

NOTICE	IBM Internal Use Only

DESCRIPTION

.......................

HISTORY
  04/13/88	Created (TC Peters)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include "zip.H"
#include "environ.H"
#include "zipview.H"
#include "zipedit.H"
#include "zipprint.H"
#include "zipobject.H"
#include "zipoplin.H"

ATKdefineRegistryNoInit(zipoplin, zipopath);

zipoplin::zipoplin( )
      {
  IN(zipoplin_InitializeObject);
  this->tolerance = environ::GetProfileInt( "ZipCreateTolerance", 10 );
  DEBUGdt(Tolerance,this->tolerance);
  OUT(zipoplin_InitializeObject);
  THROWONFAILURE(  true);
  }

char
zipoplin::Object_Icon( )
    {
  IN(zipoplin::Object_Icon);
  OUT(zipoplin::Object_Icon);
  return  'D';
  }


char
zipoplin::Object_Icon_Cursor( )
    {
  IN(zipoplin::Object_Icon_Cursor);
  OUT(zipoplin::Object_Icon_Cursor);
  return  'J';
  }

char
zipoplin::Object_Datastream_Code( )
    {
  IN(zipoplin::Object_Datastream_Code);
  OUT(zipoplin::Object_Datastream_Code);
  return  'D';
  }

long
zipoplin::Show_Object_Properties( zip_type_pane		   pane, zip_type_figure		   figure )
        {
  (this->view_object)->Announce(  "Draw PolyLine in Segments." );
  return  zip_ok;
  }

long
zipoplin::Build_Object( zip_type_pane		   pane, enum view::MouseAction				   action , long				   x , long				   y , long				   clicks, zip_type_point		   X , zip_type_point		   Y )
          {
  long				  status = zip_ok;
  static long				  initial_x, initial_y,
					  initial_X, initial_Y,
					  prior_X, prior_Y,
					  prior_x, prior_y;

  IN(zipoplin::Build_Object);
  if ( action == (long)view::LeftDown  &&
       (abs(x - prior_x) < this->tolerance  &&  abs(y - prior_y) < this->tolerance) )
    {
    action = view::NoMouseEvent;
    CurrentFigure = NULL;
    prior_x = prior_y = prior_X = prior_Y = initial_x = initial_y = initial_X = initial_Y = 0;
    pane->zip_pane_edit->zip_pane_edit_build_figure = true;
    }
  switch ( action )
    {
    case view::LeftDown:
      prior_x = x;  prior_y = y;
      (this->view_object)->Set_Pane_Painting_Mode(  pane, zipview_paint_inverted );
      if ( pane->zip_pane_edit->zip_pane_edit_build_figure )
	{
        initial_x = x;  initial_y = y;
        initial_X = prior_X = X;  initial_Y = prior_Y = Y;
	if ( (status = (this->data_object)->Create_Figure(  &CurrentFigure, NULL,
		zip_poly_line_figure, CurrentImage, NULL )) == zip_success )
          {
          (this)->Set_Object_Point(  CurrentFigure,
		zip_figure_origin_point, X, Y );
          (this)->Set_Object_Point(  CurrentFigure,
		zip_figure_auxiliary_point, X, Y );
  	  CurrentFigure->zip_figure_zoom_level = pane->zip_pane_zoom_level;
	  pane->zip_pane_edit->zip_pane_edit_build_figure = false;
	  pane->zip_pane_edit->zip_pane_edit_last_point_id = zip_figure_auxiliary_point;
	  (this->data_object)->Set_Figure_Shade(  CurrentFigure,
			      pane->zip_pane_edit->zip_pane_edit_current_shade );
	  }
	}
	else
	{
	if ( X != prior_X  ||  Y != prior_Y )
	  {
	  if ( abs(initial_x - x) <= this->tolerance  &&  abs(initial_y - y) <= this->tolerance )
	    { DEBUG(Close Path);
	    X = initial_X;  Y = initial_Y;
	    }
          (this)->Set_Object_Point(  CurrentFigure,
		   ++pane->zip_pane_edit->zip_pane_edit_last_point_id,
		   prior_X = X, prior_Y = Y );
	  (this->view_object)->Draw_Figure(  CurrentFigure, pane );
/*	  if ( X == initial_X && Y == initial_Y )
	    pane->zip_pane_edit->zip_pane_edit_build_figure = true; */
	  }
	}
      break;
    case view::LeftUp:
      if ( CurrentFigure )
	{
        prior_x = x;  prior_y = y;
        (this->view_object)->Draw_Figure(  CurrentFigure, pane );
	}
      break;
    case view::LeftMovement:
      if ( CurrentFigure  &&  !pane->zip_pane_edit->zip_pane_edit_build_figure  )
	{
        (this->view_object)->Set_Pane_Painting_Mode(  pane, zipview_paint_inverted );
	(this->view_object)->Draw_Figure(  CurrentFigure, pane );
        (this)->Set_Object_Point(  CurrentFigure,
		     pane->zip_pane_edit->zip_pane_edit_last_point_id, X, Y );
	(this->view_object)->Draw_Figure(  CurrentFigure, pane );

	}
      break;
    default:
      break;
    }
  (this->view_object)->Set_Pane_Painting_Mode(  pane, zip_default );
  OUT(zipoplin::Build_Object);
  return  status;
  }
