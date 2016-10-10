/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* zipobj.c	Zip Object -- Super-class				      */
/* Author	TC Peters						      */
/* Information Technology Center		   Carnegie-Mellon University */

/**  SPECIFICATION -- Internal Facility Suite  *********************************

TITLE	Zip Object -- Super-class

MODULE	zipobj.c

NOTICE	IBM Internal Use Only

DESCRIPTION

.......................

HISTORY
  04/13/88	Created (TC Peters)
  07/12/89	Added Object_Visible stub (SCG)
  08/07/89	Add Object_Modified stub (TCP)
  07/12/90	Have Object_Point return the first coordinate (SCG)
   08/14/90	Add Contains stub. Remove Object_Visible stub (SCG)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include <view.H>
#include "zip.H"
#include "zipprint.h"
#include "zipview.H"
#include "zipedit.H"
#include "zipprint.H"
#include "zipobject.H"

static boolean debug=TRUE;

ATKdefineRegistry(zipobject, view, NULL);

zipobject::zipobject( )
      {
/*===debug=1;===*/
  IN(zipobject::zipobject);
  this->data_object  = NULL;
  this->view_object  = NULL;
  this->edit_object  = NULL;
  this->print_object = NULL;
  OUT(zipobject::zipobject);
  THROWONFAILURE(  true);
  }

void
zipobject::Destroy_Object( zip_type_figure		   object )
      {
  IN(zipobject::Destroy_Object);
  /*=== NULL ===*/
  OUT(zipobject::Destroy_Object);
  }

void
zipobject::Set_Data_Object( class zip *data_object )
      {
  IN(zipobject::Set_Data_Object);
  this->data_object = data_object;
  OUT(zipobject::Set_Data_Object);
  }

void
zipobject::Set_View_Object( class zipview *view_object )
      {
  IN(zipobject::Set_View_Object);
  this->view_object = view_object;
  OUT(zipobject::Set_View_Object);
  }

void
zipobject::Set_Edit_Object( class zipedit	          *edit_object )
      {
  IN(zipobject::Set_Edit_Object);
  this->edit_object = edit_object;
  OUT(zipobject::Set_Edit_Object);
  }

void
zipobject::Set_Print_Object( class zipprint	          *print_object )
      {
  IN(zipobject::Set_Print_Object);
  this->print_object = print_object;
  OUT(zipobject::Set_Print_Object);
  }

void
zipobject::Set_Debug( boolean state )
      {
  IN(zipobject::Set_Debug);
  debug = state;
  OUT(zipobject::Set_Debug);
  }

long
zipobject::Object_Modified( zip_type_figure	   object )
      {
  IN(zipobject_Object_Modified);
  OUT(zipobject_Object_Modified);
  return  0;
  }

view::DSattributes
zipobject::Object_DesiredSize( long width, long height, enum view::DSpass pass, long *desired_width , long *desired_height )
{
  IN(zipobject::Object_DesiredSize);
  /*===*/
  OUT(zipobject::Object_DesiredSize);
  return  0;
}

void 
zipobject::Object_FullUpdate( enum view::UpdateType	       type, long			       left , long			       top , long			       width , long			       height )
        {
  IN(zipobject::Object_FullUpdate);
  /*=== NULL ===*/
  OUT(zipobject::Object_FullUpdate);
  }

void 
zipobject::Object_Update( )
    {
  IN(zipobject::Object_Update);
  /*=== NULL ===*/
  OUT(zipobject::Object_Update);
  }

class view *
zipobject::Object_Hit( zip_type_figure	       figure, enum view::MouseAction       action, long			       x , long			       y , long			       clicks )
          {
  IN(zipobject::Object_Hit );
  /*=== NULL ===*/
  OUT(zipobject::Object_Hit );
  return  NULL;
  }

char
zipobject::Object_Icon( )
    {
  IN(zipobject::Object_Icon);
  /****   NULL  ****/
  OUT(zipobject::Object_Icon);
  return  0;
  }

const char *
zipobject::Object_Icon_Font_Name( )
    {
  IN(zipobject::Object_Icon_Font_Name);
  OUT(zipobject::Object_Icon_Font_Name);
  return  IconFontName;
  }

char
zipobject::Object_Icon_Cursor( )
    {
  IN(zipobject::Object_Icon_Cursor);
  /****   NULL  ****/
  OUT(zipobject::Object_Icon_Cursor);
  return  0;
  }

const char *
zipobject::Object_Icon_Cursor_Font_Name( )
    {
  IN(zipobject::Object_Icon_Cursor_Font_Name);
  OUT(zipobject::Object_Icon_Cursor_Font_Name);
  return  CursorFontName;
  }

char
zipobject::Object_Datastream_Code( )
    {
  IN(zipobject::Object_Datastream_Code);
  /****   NULL  ****/
  OUT(zipobject::Object_Datastream_Code);
  return  0;
  }

long
zipobject::Build_Object( zip_type_pane pane, enum view::MouseAction action, long x, long y, long clicks, zip_type_point X , zip_type_point Y )
  {
  IN(zipobject::Build_Object);
  /****   NULL  ****/
  OUT(zipobject::Build_Object);
  return zip_failure;
  }

long
zipobject::Show_Object_Properties( zip_type_pane		   pane, zip_type_figure		   figure )
        {
  IN(zipobject::Show_Object_Properties);
  /****   NULL  ****/
  OUT(zipobject::Show_Object_Properties);
  return  zip_ok;
  }

long
zipobject::Read_Object( zip_type_figure		   figure )
      {
  long				  status;

  IN(zipobject::Read_Object);
  status = (this->data_object)->Read_Figure(  figure );
  OUT(zipobject::Read_Object);
  return  status;
  }

long
zipobject::Read_Object_Stream( zip_type_figure		   figure, FILE				  *file, long				   id )
          {
  IN(zipobject::Read_Object_Stream);
  /****   NULL  ****/
  OUT(zipobject::Read_Object_Stream);
  return  zip_failure;
  }

long
zipobject::Write_Object( zip_type_figure		   figure )
      {
  long				  status = zip_ok;

  IN(zipobject::Write_Object);
  status = (this->data_object)->Write_Figure(  figure );
  OUT(zipobject::Write_Object);
  return  status;
  }

long
zipobject::Draw_Object( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  IN(zipobject::Draw_Object);
  /****   NULL  ****/
  OUT(zipobject::Draw_Object);
  return zip_failure;
  }

long
zipobject::Clear_Object( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  IN(zipobject::Clear_Object);
  /****   NULL  ****/
  OUT(zipobject::Clear_Object);
  return zip_failure;
  }

long
zipobject::Print_Object( zip_type_figure figure, zip_type_pane pane )
      {
  IN(zipobject::Print_Object);
  /****   NULL  ****/
  OUT(zipobject::Print_Object);
  return zip_failure;
  }

long
zipobject::Proximate_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		   x , zip_type_pixel		   y )
          {
  IN(zipobject::Proximate_Object_Points);
  /****   NULL  ****/
  OUT(zipobject::Proximate_Object_Points);
  return zip_failure;
  }

long
zipobject::Within_Object( zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		   x , zip_type_pixel		   y )
          {
  IN(zipobject::Within_Object);
  /****   NULL   ****/
  OUT(zipobject::Within_Object);
  return  -1;
  }

boolean
zipobject::Enclosed_Object( zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		   x , zip_type_pixel		   y , zip_type_pixel		   w , zip_type_pixel		   h )
          {
  IN(zipobject::Enclosed_Object);
  /****   NULL   ****/
  OUT(zipobject::Enclosed_Object);
  return  false;
  }

long
zipobject::Object_Enclosure( zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		   *x , zip_type_pixel		   *y , zip_type_pixel		   *w , zip_type_pixel		   *h )
          {
  IN(zipobject::Object_Enclosure);
  /****   NULL   ****/
  *x = *y = *w = *h = 0;
  OUT(zipobject::Object_Enclosure);
  return  zip_failure;
  }

long
zipobject::Highlight_Object_Points( zip_type_figure figure, zip_type_pane pane )
      {
  IN(zipobject::Highlight_Object_Points);
  /****   NULL  ****/
  OUT(zipobject::Highlight_Object_Points);
  return zip_failure;
  }

long
zipobject::Normalize_Object_Points( zip_type_figure figure, zip_type_pane pane )
      {
  IN(zipobject::Normalize_Object_Points);
  /****   NULL  ****/
  OUT(zipobject::Normalize_Object_Points);
  return zip_failure;
  }

long
zipobject::Expose_Object_Points( zip_type_figure figure, zip_type_pane pane )
      {
  IN(zipobject::Expose_Object_Points);
  /****   NULL  ****/
  OUT(zipobject::Expose_Object_Points);
  return zip_failure;
  }

long
zipobject::Hide_Object_Points( zip_type_figure figure, zip_type_pane pane )
      {
  IN(zipobject::Hide_Object_Points);
  /****   NULL  ****/
  OUT(zipobject::Hide_Object_Points);
  return zip_failure;
  }

long
zipobject::Set_Object_Shade( zip_type_figure figure, unsigned char shade )
        {
  IN(zipobject::Set_Object_Shade);
  /****   NULL  ****/
  OUT(zipobject::Set_Object_Shade);
  return  zip_failure;
  }

long
zipobject::Set_Object_Font( zip_type_figure		   figure, short			   font )
        {
  IN(zipobject::Set_Object_Font);
  /****   NULL  ****/
  OUT(zipobject::Set_Object_Font);
  return  zip_failure;
  }

/*  Generic Object Point Setting

    Object-points are "set" by specifying a "Handle" and the x/y coordinate
    at which the Handle is to be placed. Handles form a rectangular perimeter 
    that govern the position of the figure bounded by the Handles. Handles
    are designated thus --

	9 ***** 2 ***** 3
	*		*
	*		*
	8	1	4
	*		*
	*		*
	7 ***** 6 ***** 5

    (that is, Handle-1 is the centroid, Handle-2 is at Noon, Handle-3 at
    the Upper-Right corner, Handle-4 at 3 O'clock, etc -- proceeding in a
    clockwise direction). Creation of a figure entails setting of the
    appropriate Handles; eg, creation of a Rectangle figure is usually
    effected by setting Handle-9 and Handle-5, the Upper-left and the Lower-right
    Handles.

*/

long
zipobject::Set_Object_Point( zip_type_figure figure, int point, zip_type_point x, zip_type_point y )
{
  long				  status = zip_ok;

  IN(zipobject::Set_Object_Point);
  if ( figure->zip_figure_points == NULL  &&
       (status = (this->data_object)->Allocate_Figure_Points_Vector(
		     &figure->zip_figure_points )) == zip_ok )
    figure->zip_figure_points->zip_points_count = 1;
  if ( status == zip_ok )
    {
    switch ( point )
      {
      case 1:	/* Center */
	(this)->Adjust_Object_Point_Suite(  figure, 
		x - (figure_x_point + (figure_x_points(0) - figure_x_point)/2),
		y - (figure_y_point - (figure_y_point - figure_y_points(0))/2) );
	break;
      case 2:	/* 12 O'Clock */
	figure_y_points(0) += figure_y_point - y;
	figure_y_point = y;
	break;
      case 3:	/* Upper-Right Corner */
	figure_x_points(0) = x;
	figure_y_point = y;
	break;
      case 4:	/* 3 O'Clock */
	figure_x_point += figure_x_points(0) - x;
	figure_x_points(0) = x;
	break;
      case 5:	/* Lower-Right Corner */
	figure_x_points(0) = x;
	figure_y_points(0) = y;
	break;
      case 6:	/* 6 O'Clock */
	figure_y_point += figure_y_points(0) - y;
	figure_y_points(0) = y;
	break;
      case 7:	/* Lower-Left Corner */
	figure_x_point = x;
	figure_y_points(0) = y;
	break;
      case 8:	/* 9 O'Clock */
	figure_x_points(0) += figure_x_point - x;
	figure_x_point = x;
	break;
      case 9:	/* Upper-Left Corner */
	figure_x_point = x;
	figure_y_point = y;
	break;
      default: status = zip_failure; /*=== s/b zip_invalid_point ===*/
      }
    if ( status == zip_ok )
      {
      (this->data_object)->Set_Image_Extrema(  figure->zip_figure_image, x, y );
      (this->data_object)->Set_Stream_Extrema(  figure->zip_figure_image->zip_image_stream,
			      figure->zip_figure_image );
      }
    }
  OUT(zipobject::Set_Object_Point);
  return  status;
  }

long
zipobject::Object_Point( zip_type_figure		   figure, long				   point, zip_type_point		   *x , zip_type_point		   *y )
          {
  IN(zipobject::Object_Point);
  if ( figure )
    { /* just return the first coordinate */
    *x = figure_x_point;
    *y = figure_y_point;
    }
  else *x = *y = 0;
  OUT(zipobject::Object_Point);
  return zip_failure;
  }

long
zipobject::Adjust_Object_Point_Suite( zip_type_figure		   figure, zip_type_point		   x_delta , zip_type_point		   y_delta )
        {
  long				  status = zip_ok;

  IN(zipobject::Adjust_Object_Point_Suite);
  figure_x_point += x_delta;
  figure_y_point += y_delta;
  figure_x_points(0) += x_delta;
  figure_y_points(0) += y_delta;
  (this->data_object)->Set_Image_Extrema(  figure->zip_figure_image, figure_x_point, figure_y_point );
  (this->data_object)->Set_Stream_Extrema(  figure->zip_figure_image->zip_image_stream,
			    figure->zip_figure_image );
  OUT(zipobject::Adjust_Object_Point_Suite);
  return  status;
  }

boolean
zipobject::Contains( zip_type_figure		 figure, zip_type_pane		 pane, zip_type_pixel		 x , zip_type_pixel		 y )
        
  {
    return FALSE;
  }
