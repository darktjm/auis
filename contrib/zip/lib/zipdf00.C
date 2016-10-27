/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1988
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
/* zipedf.c	Zip Data-object	-- Figures			      */
/* Author	TC Peters					      */
/* Information Technology Center	   Carnegie-Mellon University */


/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Zip Data-object -- Figures

MODULE	zipedf.c

NOTICE	IBM Internal Use Only

DESCRIPTION
	This is the suite of Methods that support the Figure facilities
	of the Zip Data-object.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  03/31/88	Created (TCP)
  11/17/88	Add Set_Figure_Line_Width (TCP/SG)
   08/14/90	Add Set_Figure_<Attribute> methods for color and line styles (SCG)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include "dataobject.H"
#include "graphic.H"
#include "zip.H"
#include "zipifm00.h"
#include "zipobject.H"

#define	 Data			      this
#define	 Objects(i)		      ((this->objects)[i])
#define	 SetStreamModified	      {(this)->SetModified();\
					figure->zip_figure_image->zip_image_stream->\
					zip_stream_states.zip_stream_state_modified = 1;}


long
zip::Create_Figure( zip_type_figure	      *figure, const char			      *name, unsigned char type, zip_type_image	       image, zip_type_figure	       peer )
            {
    class zip *self=this;
  long			      status = zip_ok;
  zip_type_figure	      peer_ptr = peer;

  IN(zip_Create_Figure);
  if ( image )
    {
    if ( ( *figure = (zip_type_figure) calloc( 1, sizeof(struct zip_figure) ) ) )
      {
      (*figure)->zip_figure_image = image;
      (*figure)->zip_figure_image->zip_image_stream->
		zip_stream_states.zip_stream_state_modified = 1;
      if ( peer_ptr )
        {
        (*figure)->zip_figure_next = peer_ptr->zip_figure_next;
        peer_ptr->zip_figure_next = *figure;
        }
        else
        {
	peer_ptr = image->zip_image_figure_anchor;
	while ( peer_ptr  &&  peer_ptr->zip_figure_next )
	  peer_ptr = peer_ptr->zip_figure_next;
	if ( peer_ptr )
	  peer_ptr->zip_figure_next = *figure;
	  else
	  image->zip_image_figure_anchor = *figure;
        }
      (*figure)->zip_figure_type = type;
      (*figure)->zip_figure_line_width = 255;
      (*figure)->zip_figure_line_cap = (graphic::LineCap)-1;
      (*figure)->zip_figure_line_join = (graphic::LineJoin)-1;
      if ( name  &&  *name )
        status = (self)->Set_Figure_Name(  *figure, name );
      }
      else  status = zip_insufficient_figure_space;
    }
    else  status = zip_image_non_existent;
  OUT(zip_Create_Figure);
  return status;
  }

long
zip::Destroy_Figure( zip_type_figure	       figure )
      {
  int			      status = zip_ok;

  IN(zip_Destroy_Figure);
  if ( figure )
    {
    if ( figure->zip_figure_name )
      {
      SetStreamModified;
      symtab_delete( ((zip_type_stream)(this)->Containing_Figure_Stream(  figure ))->
			zip_stream_symbol_table,
		   (unsigned char *) figure->zip_figure_name );
      free( figure->zip_figure_name );
      }
    if ( figure->zip_figure_datum.zip_figure_text )
      free( figure->zip_figure_datum.zip_figure_text );
    if ( figure->zip_figure_points )
      free( figure->zip_figure_points );
    if ( ! figure->zip_figure_state.zip_figure_state_unhooked )
      (this)->Unhook_Figure(  figure );
    free( figure );
    }
    else  status = zip_figure_non_existent;
  ZIP_STATUS(this);
  OUT(zip_Destroy_Figure);
  return status;
  }

long
zip::Hook_Figure( zip_type_figure	       figure  , zip_type_figure	       peer_figure )
      {
  int				  status = zip_ok;

  IN(zip_Hook_Figure);
  if ( figure  &&  peer_figure )
    {
    if ( figure->zip_figure_state.zip_figure_state_unhooked )
      {
      figure->zip_figure_next = peer_figure->zip_figure_next; 
      peer_figure->zip_figure_next = figure;
      figure->zip_figure_image = peer_figure->zip_figure_image;
      (this)->Set_Figure_Name(  figure, (this)->Figure_Name(  figure ) );
      figure->zip_figure_state.zip_figure_state_unhooked = false;
      SetStreamModified;
      }
      else
      status = zip_failure; /*=== s/b "Already Hooked" ==*/
    }
    else
    status = zip_figure_non_existent;
  ZIP_STATUS(this);
  OUT(zip_Hook_Figure);
  return status;
  }

long
zip::Unhook_Figure( zip_type_figure	       figure )
      {
  int			      status = zip_ok;
  zip_type_figure	      figure_ptr;

  IN(zip_Unhook_Figure);
  if ( figure )
    {
    if ( ! figure->zip_figure_state.zip_figure_state_unhooked )
      {
      SetStreamModified;
      figure_ptr = (zip_type_figure) figure->zip_figure_image->zip_image_figure_anchor;
      if ( figure_ptr == figure )
	figure->zip_figure_image->zip_image_figure_anchor = figure->zip_figure_next;
	else
	while( figure_ptr  &&  figure_ptr->zip_figure_next != figure )
	  figure_ptr = figure_ptr->zip_figure_next;
      figure_ptr->zip_figure_next = figure->zip_figure_next;
      figure->zip_figure_next = NULL;
      figure->zip_figure_image = NULL;
      figure->zip_figure_state.zip_figure_state_unhooked = true;
      }
      else  status = zip_failure; /*=== s/b/ "Already Un-hooked" ===*/
    }
    else  status = zip_figure_non_existent;
  ZIP_STATUS(this);
  OUT(zip_Unhook_Figure);
  return status;
  }

long
zip::Set_Figure_Name( zip_type_figure figure, const char  *name )
{
  int			      status = zip_ok;
  zip_type_figure		      duplicate;

  IN(zip_Set_Figure_Name);
  if ( figure )
    {
    DEBUGst(Given Name ,name);
    DEBUGst(Figure Name,figure->zip_figure_name);
    SetStreamModified;
    if ( figure->zip_figure_name )
      *figure->zip_figure_name = '\0';
    if ( name  &&  *name != '\0' )
      {
      if ( symtab_find( figure->zip_figure_image->zip_image_stream->zip_stream_symbol_table,
		 (unsigned char *) name, (struct user_data  **) &duplicate ) == 0
/*===something wrong in symtab===*/ && strcmp(name,duplicate->zip_figure_name)==0 )
        {
        status = zip_duplicate_figure_name;
/*===
printf( "DUP FIG NAME '%s' (Stream '%s'  Figure '%s')\n",
name, figure->zip_figure_image->zip_image_stream->zip_stream_name,duplicate->zip_figure_name);
===*/
        }
        else
        {
        if ( (figure->zip_figure_name = strdup( name ) ) == NULL )
          status = zip_insufficient_figure_space;
          else
          {
          symtab_add( figure->zip_figure_image->zip_image_stream->zip_stream_symbol_table,
		    (unsigned char *) figure->zip_figure_name, (struct user_data  *) figure );
          }
	}
      }
      else
      {
      if ( figure->zip_figure_name )
        free( figure->zip_figure_name );
      figure->zip_figure_name = NULL;
      }
    }
    else  status = zip_figure_non_existent;
  ZIP_STATUS(this);
  OUT(zip_Set_Figure_Name);
  return status;
  }

long
zip::Set_Figure_Text( zip_type_figure figure, const char *text )
{
  int			      status = zip_ok;

  IN(zip_Set_Figure_Text);
  if ( figure )
    {
    DEBUGst(Given Text,text);
    DEBUGst(Figure Name,figure->zip_figure_name);
    SetStreamModified;
    if ( text ) {
      if ( (figure->zip_figure_datum.zip_figure_text = strdup( text ) ) == NULL )
        status = zip_insufficient_figure_space;
    }
      else figure->zip_figure_datum.zip_figure_text = NULL;
    }
    else
    status = zip_figure_non_existent;
  ZIP_STATUS(this);
  OUT(zip_Set_Figure_Text);
  return status;
  }

long
zip::Set_Figure_Pattern( zip_type_figure	       figure, char			       pattern )
        {
  int			      status = zip_ok;

  IN(zip_Set_Figure_Pattern);
  if ( figure )
    {
    DEBUGct(Given Pattern,pattern);
    DEBUGst(Figure Name,figure->zip_figure_name);
    SetStreamModified;
    figure->zip_figure_fill.zip_figure_pattern = pattern;
    if ( pattern )
      figure->zip_figure_mode.zip_figure_mode_patterned = on;
      else
      figure->zip_figure_mode.zip_figure_mode_patterned = off;
    }
    else
    status = zip_figure_non_existent;
  ZIP_STATUS(this);
  OUT(zip_Set_Figure_Pattern);
  return status;
  }

long
zip::Set_Figure_Shade( zip_type_figure figure, unsigned char shade )
{
  long			      status = zip_ok;

  IN(zip_Set_Figure_Shade);
  if ( figure )
    {
    DEBUGdt(Given Shade,shade);
    DEBUGst(Figure Name,figure->zip_figure_name);
    SetStreamModified;
    status = (Objects(figure->zip_figure_type))->Set_Object_Shade( figure, shade );
    }
    else
	status = zip_figure_non_existent;
  ZIP_STATUS(this);
  OUT(zip_Set_Figure_Shade);
  return status;
}

long
zip::Set_Figure_Font( zip_type_figure	       figure, const char			      *name )
        {
  long			     status = zip_ok;
  short				     font = 0;

  IN(zip_Set_Figure_Font);
  if ( figure  &&  name  &&  (this)->Define_Font(  name, &font ) )
    {
    DEBUGst(Given Font,font);
    DEBUGst(Figure Name,figure->zip_figure_name);
    SetStreamModified;
    status = (Objects(figure->zip_figure_type))->Set_Object_Font(
		 figure, font );
    }
    else
    {
    if ( figure == NULL )
      status = zip_figure_non_existent;
    else
    if ( name == NULL )
      figure->zip_figure_font = 0;
    }
  ZIP_STATUS(this);
  OUT(zip_Set_Figure_Font);
  return status;
  }

long
zip::Set_Figure_Mode( zip_type_figure	       figure, long mode )
      {
  int			      status = zip_ok;

  IN(zip_Set_Figure_Mode);
  if ( figure )
    {
    DEBUGst(Figure Name,figure->zip_figure_name);
    SetStreamModified;
    figure->zip_figure_mode.zip_figure_mode_top    = off;
    figure->zip_figure_mode.zip_figure_mode_middle = off;
    figure->zip_figure_mode.zip_figure_mode_bottom = off;
    figure->zip_figure_mode.zip_figure_mode_left   = off;
    figure->zip_figure_mode.zip_figure_mode_center = off;
    figure->zip_figure_mode.zip_figure_mode_right  = off;
    figure->zip_figure_mode.zip_figure_mode_halo   = off;

    if ( mode & zip_top )	figure->zip_figure_mode.zip_figure_mode_top    = on;
    else
    if ( mode & zip_middle )	figure->zip_figure_mode.zip_figure_mode_middle = on;
    else
    if ( mode & zip_bottom )	figure->zip_figure_mode.zip_figure_mode_bottom = on;

    if ( mode & zip_left )	figure->zip_figure_mode.zip_figure_mode_left   = on;
    else
    if ( mode & zip_center )	figure->zip_figure_mode.zip_figure_mode_center = on;
    else
    if ( mode & zip_right )	figure->zip_figure_mode.zip_figure_mode_right  = on;

    if ( mode & zip_halo )	figure->zip_figure_mode.zip_figure_mode_halo   = on;
    }
    else
    status = zip_figure_non_existent;
  ZIP_STATUS(this);
  OUT(zip_Set_Figure_Mode);
  return status;
  }

long
zip::Set_Figure_Line_Width( zip_type_figure figure, short width )
{
  int			      status = zip_ok;

  IN(zip_Set_Figure_Line_Width);
  if ( figure )
    {
    DEBUGdt(Given Width,width);
    DEBUGst(Figure Name,figure->zip_figure_name);
    SetStreamModified;
    figure->zip_figure_line_width = width;
    }
    else
    status = zip_figure_non_existent;
  ZIP_STATUS(this);
  OUT(zip_Set_Figure_Line_Width);
  return status;
  }

long
zip::Set_Figure_Line_Dash( zip_type_figure		 figure, const unsigned char				 *pattern, int				 offset, graphic::LineDash			 type )
            {
  int			      status = zip_ok;

      IN(zip_Set_Figure_Line_Dash);
      if ( figure )
      {
	  if ( pattern )
            {
	      figure->zip_figure_line_dash_pattern = (unsigned char *)strdup( (char *)pattern );
	      figure->zip_figure_line_dash_offset = offset;
	      figure->zip_figure_line_dash_type = type;
	    }
	  else figure->zip_figure_line_dash_type = graphic::LineSolid;
	  SetStreamModified;
      }
      else
	  status = zip_figure_non_existent;
      ZIP_STATUS(this);
      OUT(zip_Set_Figure_Line_Dash);
      return status;
  }

long
zip::Set_Figure_Line_Cap( zip_type_figure		 figure, graphic::LineCap			 cap )
        {
  int			      status = zip_ok;

      IN(zip_Set_Figure_Line_Cap);
      if ( figure )
      {
	  SetStreamModified;
	  figure->zip_figure_line_cap = cap;
      }
      else
	  status = zip_figure_non_existent;
      ZIP_STATUS(this);
      OUT(zip_Set_Figure_Line_Cap);
      return status;
  }

long
zip::Set_Figure_Line_Join( zip_type_figure		 figure, graphic::LineJoin			 join )
        {
  int			      status = zip_ok;

      IN(zip_Set_Figure_Line_Join);
      if ( figure )
      {
	  SetStreamModified;
	  figure->zip_figure_line_join = join;
      }
      else
	  status = zip_figure_non_existent;
      ZIP_STATUS(this);
      OUT(zip_Set_Figure_Line_Join);
      return status;
  }

long
zip::Set_Figure_Line_Color( zip_type_figure		 figure, double			 red , double			 green , double			 blue )
        {
  int			      status = zip_ok;

     IN(zip_Set_Figure_Line_Color);
     if ( figure )
      {
	  if ( figure->zip_figure_color_values == NULL )
	    figure->zip_figure_color_values = (this )->Allocate_Color_Values( );
          if ( figure->zip_figure_color_values &&
	     ( figure->zip_figure_color_values->line == NULL ))
	    figure->zip_figure_color_values->line = (this )->Allocate_Color( );
 	  if (  figure->zip_figure_color_values && figure->zip_figure_color_values->line )
            {
  	      SetStreamModified;
  	      figure->zip_figure_color_values->line->red = red;
  	      figure->zip_figure_color_values->line->green = green;
  	      figure->zip_figure_color_values->line->blue = blue;
	    }
	  else status = zip_failure;
      }
      else
	  status = zip_figure_non_existent;
      ZIP_STATUS(this);
      OUT(zip_Set_Figure_Line_Color);
      return status;
  }

long
zip::Set_Figure_FillFG_Color( zip_type_figure		 figure, double			 red , double			 green , double			 blue )
        {
  int			      status = zip_ok;

     IN(zip_Set_Figure_FillFG_Color);
     if ( figure )
      {
	  if ( figure->zip_figure_color_values == NULL )
	    figure->zip_figure_color_values = (this )->Allocate_Color_Values( );
          if ( figure->zip_figure_color_values &&
	     ( figure->zip_figure_color_values->fillfg == NULL ))
	    figure->zip_figure_color_values->fillfg = (this )->Allocate_Color( );
 	  if (  figure->zip_figure_color_values && figure->zip_figure_color_values->fillfg )
            {
  	      SetStreamModified;
  	      figure->zip_figure_color_values->fillfg->red = red;
  	      figure->zip_figure_color_values->fillfg->green = green;
  	      figure->zip_figure_color_values->fillfg->blue = blue;
	    }
	  else status = zip_failure;
      }
      else
	  status = zip_figure_non_existent;
      ZIP_STATUS(this);
      OUT(zip_Set_Figure_FillFG_Color);
      return status;
  }

long
zip::Set_Figure_FillBG_Color( zip_type_figure		 figure, double			 red , double			 green , double			 blue )
        {
  int			      status = zip_ok;

     IN(zip_Set_Figure_FillBG_Color);
     if ( figure )
      {
	  if ( figure->zip_figure_color_values == NULL )
	    figure->zip_figure_color_values = (this )->Allocate_Color_Values( );
          if ( figure->zip_figure_color_values &&
	     ( figure->zip_figure_color_values->fillbg == NULL ))
	    figure->zip_figure_color_values->fillbg = (this )->Allocate_Color( );
 	  if (  figure->zip_figure_color_values && figure->zip_figure_color_values->fillbg )
            {
  	      SetStreamModified;
  	      figure->zip_figure_color_values->fillbg->red = red;
  	      figure->zip_figure_color_values->fillbg->green = green;
  	      figure->zip_figure_color_values->fillbg->blue = blue;
	    }
	  else status = zip_failure;
      }
      else
	  status = zip_figure_non_existent;
      ZIP_STATUS(this);
      OUT(zip_Set_Figure_FillBG_Color);
      return status;
  }

long
zip::Set_Figure_Point( zip_type_figure  figure, int point, long x, long y )
{
  long			      status = zip_ok;

  IN(zip_Set_Figure_Point);
  if ( figure )
    {
    SetStreamModified;
    status = (Objects(figure->zip_figure_type))->Set_Object_Point(
		 figure, point, x, y );
    }
    else  status = zip_figure_non_existent;
  ZIP_STATUS(this);
  OUT(zip_Set_Figure_Point);
  return status;
  }

long
zip::Adjust_Figure_Point_Suite( zip_type_figure figure, long x_delta, long y_delta )
      {
  long			      status = zip_ok;

  IN(zip_Adjust_Figure_Point_Suite);
  if ( figure )
    {
    SetStreamModified;
    status = (Objects(figure->zip_figure_type))->Adjust_Object_Point_Suite(
		 figure, x_delta, y_delta );
    }
    else  status = zip_figure_non_existent;
  ZIP_STATUS(this);
  OUT(zip_Adjust_Figure_Point_Suite);
  return status;
  }

long
zip::Change_Figure_Point( zip_type_figure	       figure, long			       old_x , long			       old_y , long			       new_x , long			       new_y )
        {
    SetStreamModified;
return  zip_failure;/*===*/
  }

long
zip::Remove_Figure_Point( zip_type_figure	       figure, long			       old_x , long			       old_y )
        {
    SetStreamModified;
return  zip_failure;/*===*/
  }

long
zip::Add_Figure_Point( zip_type_figure	       figure, long			       new_x , long			       new_y )
        {
    SetStreamModified;
return  zip_failure;/*===*/
  }


struct zip_figure *
zip::Figure( const char			      *name )
      {
  zip_type_figure		      figure = NULL;
  zip_type_stream_chain      stream_link = StreamAnchor;

  IN(zip_Figure);
  while ( stream_link )
    {
    if ( stream_link->zip_stream_chain_ptr->zip_stream_symbol_table )
       if ( symtab_find( stream_link->zip_stream_chain_ptr->zip_stream_symbol_table,
	     (unsigned char *) name, (struct user_data  **) &figure ) == 0 )
	  goto exit_point;
    stream_link = stream_link->zip_stream_chain_next;
    }
  exit_point:
  OUT(zip_Figure);
  return figure;
  }

struct zip_figure *
zip::Image_Figure( struct zip_image	      *image, const char			      *name )
        {
  zip_type_figure		      figure = NULL;

  IN(zip_Image_Figure);
  if ( image  &&  name )
    if ( image->zip_image_stream->zip_stream_symbol_table )
      symtab_find( image->zip_image_stream->zip_stream_symbol_table, (unsigned char *) name, (struct user_data  **) &figure );
  OUT(zip_Image_Figure);
  return figure;
  }

struct zip_figure *
zip::Stream_Figure( struct zip_stream	      *stream, const char			      *name )
        {
  zip_type_figure		      figure = NULL;

  IN(zip_Stream_Figure);
  if ( stream  &&  name )
    if ( stream->zip_stream_symbol_table )
      symtab_find( stream->zip_stream_symbol_table, (unsigned char *) name, (struct user_data  **) &figure );
  OUT(zip_Stream_Figure);
  return figure;
  }

int
zip::Allocate_Figure_Points_Vector( zip_type_point_pairs	      *anchor )
{
  int			      status = zip_ok;

  IN(zip::Allocate_Figure_Points_Vector);
  if ( (*anchor = (zip_type_point_pairs)
	 malloc( sizeof(struct zip_point_pairs) +
	 (zip_points_allocation * sizeof(struct zip_point_pair)) )) == NULL )
    status = zip_insufficient_figure_space;
    else
    (*anchor)->zip_points_count = 0;
  OUT(zip::Allocate_Figure_Points_Vector);
  return status;
  }


int
zip::Enlarge_Figure_Points_Vector( zip_type_point_pairs	      *anchor )
{
  int			      status = zip_ok;

  IN(zip::Enlarge_Figure_Points_Vector);
  if ( (*anchor = (zip_type_point_pairs)
	 realloc( *anchor, sizeof(struct zip_point_pairs) +
		 ((*anchor)->zip_points_count * sizeof(struct zip_point_pair)) +
		 (zip_points_allocation * sizeof(struct zip_point_pair)) )) == NULL )
    status = zip_insufficient_figure_space;
  OUT(zip::Enlarge_Figure_Points_Vector);
  return status;
  }
