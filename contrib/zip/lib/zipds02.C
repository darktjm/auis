/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1988
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
/* zipds02.c	Zip Data-object	-- Stream Output Parsing	      */
/* Author	TC Peters					      */
/* Information Technology Center	   Carnegie-Mellon University */


/*
    $Log: zipds02.C,v $
 * Revision 1.3  1993/06/17  04:28:00  rr2b
 * made apt_MM_compare a macro which uses ULstrcmp
 * to avoid multiple definitions of apt_MM_compare.
 *
 * Revision 1.2  1993/06/09  17:45:37  gk5g
 * functional C++ version
 *
 * Revision 1.3  1992/12/15  21:57:55  rr2b
 * more disclaimerization fixing
 *
 * Revision 1.2  1992/12/15  04:00:36  rr2b
 * disclaimerization
 *
 * Revision 1.1  1992/10/06  18:33:00  susan
 * Initial revision
 *
 * Revision 2.8  1991/09/12  16:42:05  bobg
 * Update copyright notice and rcsid
 *
 * Revision 2.7  1991/09/10  20:52:14  gk5g
 * Changes in support of SGI_4d platform.
 * Mostly added forward delcarations.
 *
 * Revision 2.6  1990/08/21  14:19:24  sg08
 * Add code to enparse color and line style attributes
 *
 * Revision 2.5  89/02/17  18:07:12  ghoti
 * ifdef/endif,etc. label fixing - courtesy of Ness
 * 
 * Revision 2.4  89/02/08  16:49:11  ghoti
 * change copyright notice
 * 
 * Revision 2.3  89/02/04  19:05:26  ghoti
 * first pass porting changes: filenames and references to them
 * 
 * Revision 2.2  88/11/18  21:07:54  tom
 * Change null line-with to 255 indicator.
 * 
 * Revision 2.1  88/09/27  18:11:41  ghoti
 * adjusting rcs #
 * 
 * Revision 1.2  88/09/15  17:28:01  ghoti
 * copyright fix
 * 
 * Revision 1.1  88/09/14  17:43:33  tom
 * Initial revision
 * 
*/

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Zip Data-object -- Stream Output Parsing

MODULE	zipds02.c

NOTICE	IBM Internal Use Only

DESCRIPTION
	This is the suite of Methods that support the Stream Writing facilities
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
  11/17/88	Handle line_width attribute (TCP/SG)
   08/14/90	Add code to enparse color and line style attributes (SCG)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include "dataobject.H"
#include "zip.H"
#include "zipobject.H"
#include <ctype.h>


#define	 Data			      (this)
#define	 Objects(i)		    ((self->objects)[i])

extern NO_DLL_EXPORT long zip_Enparse_Stream( class zip		      *self, struct zip_stream	      *stream );
static int Write_Inferior_Image( class zip		      *self, zip_type_image	       image );
static int Write_Image_Beginning( class zip		      *self, zip_type_image	       image );
static int Write_Image_Ending( class zip		      *self, zip_type_image	       image );


long
zip::Write_Figure( struct zip_figure	      *figure )
      {
  int			     i, status = zip_ok;
  FILE			     *file =
	figure->zip_figure_image->zip_image_stream->zip_stream_file;
  static char			     horizontal[2] = "?",
				     vertical[2]   = "?",
				     halo[2]       = "?";
  int			     pseudo_x=0, pseudo_y=0;
  unsigned char		    *p;
  short		    c = 0; // init to shut gcc up

  IN(zip::Write_Figure);
  if ( ! figure->zip_figure_state.zip_figure_state_deleted )
    {
/*===    pseudo_x = figure->zip_figure_image->zip_image_stream->zip_stream_pseudo_x_offset;
    pseudo_y = figure->zip_figure_image->zip_image_stream->zip_stream_pseudo_y_offset;===*/
    fprintf( file, "*%c;%ld,%ld\n", ('A' - 1) + figure->zip_figure_type,
	   figure->zip_figure_point.zip_point_x - pseudo_x,
	   figure->zip_figure_point.zip_point_y - pseudo_y );
    if ( figure->zip_figure_name )
      fprintf( file, "N%s\n", figure->zip_figure_name );
    if ( figure->zip_figure_font )
	fprintf( file, "F%s\n",
		 Fonts->zip_fonts_vector[figure->zip_figure_font].
		    zip_fonts_table_name );  
    if ( figure->zip_figure_datum.zip_figure_text )
      fprintf( file, "T%s\n", figure->zip_figure_datum.zip_figure_text );
    if ( figure->zip_figure_style )
      fprintf( file, "S%d\n", figure->zip_figure_style );
    if ( figure->zip_figure_line_width != 255 )
      fprintf( file, "LW%d\n", figure->zip_figure_line_width );
    if ( figure->zip_figure_line_cap != -1 )
    {
	switch ( figure->zip_figure_line_cap )
	{
	    case graphic::CapNotLast: c = 'N'; break;
            case graphic::CapButt: c = 'B'; break;
            case graphic::CapRound: c = 'R'; break;
            case graphic::CapProjecting: c = 'P'; break;
	}
	fprintf( file, "LC%c\n", c );
    }
    if ( figure->zip_figure_line_join != -1 )
    {
	switch ( figure->zip_figure_line_join )
	{
	    case graphic::JoinMiter: c = 'M'; break;
            case graphic::JoinRound: c = 'R'; break;
            case graphic::JoinBevel: c = 'B'; break;
	}
	fprintf( file, "LJ%c\n", c );
    }
    if ( figure->zip_figure_line_dash_pattern )
    {
	switch ( figure->zip_figure_line_dash_type )
        {
	    case graphic::LineSolid: c = 'S'; break;
	    case graphic::LineDoubleDash: c = 'D'; break;
	    case graphic::LineOnOffDash: c = 'O'; break;
	}
	fprintf( file, "LD%c,%d,", c, figure->zip_figure_line_dash_offset );
	p = figure->zip_figure_line_dash_pattern;
	while( *p != 0 )
        {
	    if ( *( p + 1 ) == 0 ) fprintf( file, "%d\n", ( int ) *p );
	    else fprintf( file, "%d,", ( int ) *p );
	    p++;
	}
    }
    if ( figure->zip_figure_color_values && figure->zip_figure_color_values->line )
	fprintf( file, "CL%g,%g,%g\n", figure->zip_figure_color_values->line->red,
  	      figure->zip_figure_color_values->line->green, figure->zip_figure_color_values->line->blue );
    if ( figure->zip_figure_color_values && figure->zip_figure_color_values->fillfg )
	fprintf( file, "CL%g,%g,%g\n", figure->zip_figure_color_values->fillfg->red,
  	      figure->zip_figure_color_values->fillfg->green, figure->zip_figure_color_values->fillfg->blue );
    if ( figure->zip_figure_color_values && figure->zip_figure_color_values->fillbg )
	fprintf( file, "CL%g,%g,%g\n", figure->zip_figure_color_values->fillbg->red,
  	      figure->zip_figure_color_values->fillbg->green, figure->zip_figure_color_values->fillbg->blue );
    if ( figure->zip_figure_zoom_level )
      fprintf( file, "Z%d\n", figure->zip_figure_zoom_level );
    if ( figure->zip_figure_detail_level )
      fprintf( file, "D%d\n", figure->zip_figure_detail_level );
    if ( figure->zip_figure_mode.zip_figure_mode_patterned  &&
	 figure->zip_figure_fill.zip_figure_pattern  )
      fprintf( file, "P%c\n", figure->zip_figure_fill.zip_figure_pattern );
    if ( figure->zip_figure_mode.zip_figure_mode_shaded  &&
	 figure->zip_figure_fill.zip_figure_shade )
      fprintf( file, "G%d\n", figure->zip_figure_fill.zip_figure_shade );
    if ((figure->zip_figure_mode.zip_figure_mode_left    ||
	 figure->zip_figure_mode.zip_figure_mode_center  ||
	 figure->zip_figure_mode.zip_figure_mode_right   ||
	 figure->zip_figure_mode.zip_figure_mode_top     ||
	 figure->zip_figure_mode.zip_figure_mode_middle  ||
	 figure->zip_figure_mode.zip_figure_mode_bottom  ||
	 figure->zip_figure_mode.zip_figure_mode_halo)  )
      {
      if ( figure->zip_figure_mode.zip_figure_mode_left )   *horizontal = 'L';
      if ( figure->zip_figure_mode.zip_figure_mode_center ) *horizontal = 'C';
      if ( figure->zip_figure_mode.zip_figure_mode_right )  *horizontal = 'R';
      if ( figure->zip_figure_mode.zip_figure_mode_top )    *vertical = 'T';
      if ( figure->zip_figure_mode.zip_figure_mode_middle ) *vertical = 'M';
      if ( figure->zip_figure_mode.zip_figure_mode_bottom ) *vertical = 'B';
      if ( figure->zip_figure_mode.zip_figure_mode_halo )   *halo = 'H';
      fprintf( file, "M%s%s%s\n", horizontal, vertical, halo );
      }
    if ( figure->zip_figure_points )
      {
/*=== Begin HACK of 5/8/87 ===!/
      if ( figure->zip_figure_type == zip_circle_figure  ||
	   figure->zip_figure_type == zip_ellipse_figure  ||
	   figure->zip_figure_type == zip_arc_figure  )
	fprintf( file, ">%d,%d",
		(figure->zip_figure_points->zip_points[0].zip_point_x),
		(figure->zip_figure_points->zip_points[0].zip_point_y) );
	else===*/
	fprintf( file, ">%ld,%ld",
		figure->zip_figure_points->zip_points[0].zip_point_x - pseudo_x,
		figure->zip_figure_points->zip_points[0].zip_point_y - pseudo_y );
/*=== End HACK of 5/8/87 ===*/
      for ( i = 1; i < figure->zip_figure_points->zip_points_count; i++ )
        fprintf( file, ";%ld,%ld",
		 figure->zip_figure_points->zip_points[i].zip_point_x - pseudo_x,
		 figure->zip_figure_points->zip_points[i].zip_point_y - pseudo_y );
      fprintf( file, "\n" );
      }
    }
  IN(zip::Write_Figure);
  return status;
  }

long
zip_Enparse_Stream( class zip		      *self, struct zip_stream	      *stream )
      {
  int			      status = zip_ok;
  zip_type_image	      image = stream->zip_stream_image_anchor;
  zip_type_figure	      figure = image->zip_image_figure_anchor;
  
  IN(zip_Enparse_Stream);
  if ( ! (image->zip_image_state.zip_image_state_deleted  ||
	  image->zip_image_state.zip_image_state_transient) )
    status = Write_Image_Beginning( self, image );
  while ( figure  &&  status == zip_ok )
    {
    status = (Objects(figure->zip_figure_type))->Write_Object(  figure );
    figure = figure->zip_figure_next;
    }
  if ( image->zip_image_inferior  &&  status == zip_ok )
    status = Write_Inferior_Image( self, image->zip_image_inferior );
  if ( status == zip_ok  &&
       ! (image->zip_image_state.zip_image_state_deleted  ||
	  image->zip_image_state.zip_image_state_transient) )
    status = Write_Image_Ending( self, image );
  stream->zip_stream_states.zip_stream_state_modified = false;
  OUT(zip_Enparse_Stream);
  return status;
  }

static int
Write_Inferior_Image( class zip		      *self, zip_type_image	       image )
      {
  int			      status = zip_ok;
  zip_type_figure	      figure = image->zip_image_figure_anchor;
  
  IN(Write_Inferior_Image);
  if ( ! image->zip_image_state.zip_image_state_deleted )
    status = Write_Image_Beginning( self, image );
  while ( figure  &&  status == zip_ok )
    {
    status = (Objects(figure->zip_figure_type))->Write_Object(  figure );
    figure = figure->zip_figure_next;
    }
  if ( image->zip_image_inferior  &&  status == zip_ok )
    status = Write_Inferior_Image( self, image->zip_image_inferior );
  if ( status == zip_ok  &&  ! image->zip_image_state.zip_image_state_deleted )
    status = Write_Image_Ending( self, image );
  if ( image->zip_image_peer  &&  status == zip_ok )
    status = Write_Inferior_Image( self, image->zip_image_peer );
  OUT(Write_Inferior_Image);
  return status;
  }

static int
Write_Image_Beginning( class zip		      *self, zip_type_image	       image )
      {
  FILE			     *file =
	image->zip_image_stream->zip_stream_file;
  int			      status = zip_ok;
  short			c = 0; // init to shut gcc up
  unsigned char		*p;

  IN(Write_Image_Beginning);
  fprintf( file, "{" );
  if ( status == zip_ok  &&  image->zip_image_name )
    fprintf( file, "%s\n", image->zip_image_name );
  else fprintf( file, "\n" );
  if ( status == zip_ok  &&  image->zip_image_font )
    fprintf( file, "F%s\n",
		 self->fonts->zip_fonts_vector[image->zip_image_font].
		    zip_fonts_table_name );  
  if ( status == zip_ok  &&  image->zip_image_text )
    {
    fprintf( file, "T%s\n", image->zip_image_text );
    }
  if ( status == zip_ok  &&  image->zip_image_zoom_level != 0 )
    {
    fprintf( file, "Z%d\n", image->zip_image_zoom_level );
    }
  if ( status == zip_ok  &&  image->zip_image_mode.zip_image_mode_patterned  &&
       image->zip_image_fill.zip_image_pattern )
    {
    fprintf( file, "P%c\n", image->zip_image_fill.zip_image_pattern );
    }
  if ( status == zip_ok  &&  image->zip_image_mode.zip_image_mode_shaded  &&
       image->zip_image_fill.zip_image_shade )
    {
    fprintf( file, "G%d\n", image->zip_image_fill.zip_image_shade );
    }
  if ( status == zip_ok  &&  image->zip_image_line_width != 255 )
    {
    fprintf( file, "LW%d\n", image->zip_image_line_width );
    }
  if ( image->zip_image_line_cap != -1 )
    {
	switch ( image->zip_image_line_cap )
	{
	    case graphic::CapNotLast: c = 'N'; break;
            case graphic::CapButt: c = 'B'; break;
            case graphic::CapRound: c = 'R'; break;
            case graphic::CapProjecting: c = 'P'; break;
	}
	fprintf( file, "LC%c\n", c );
    }
    if ( image->zip_image_line_join != -1 )
    {
	switch ( image->zip_image_line_join )
	{
	    case graphic::JoinMiter: c = 'M'; break;
            case graphic::JoinRound: c = 'R'; break;
            case graphic::JoinBevel: c = 'B'; break;
	}
	fprintf( file, "LJ%c\n", c );
    }
    if ( image->zip_image_line_dash_pattern )
    {
	switch ( image->zip_image_line_dash_type )
        {
	    case graphic::LineSolid: c = 'S'; break;
	    case graphic::LineDoubleDash: c = 'D'; break;
	    case graphic::LineOnOffDash: c = 'O'; break;
	}
	fprintf( file, "LD%c,%d,", c, image->zip_image_line_dash_offset );
	p = image->zip_image_line_dash_pattern;
	while( *p != 0 )
        {
	    if ( *( p + 1 ) == 0 ) fprintf( file, "%d\n", ( int ) *p );
	    else fprintf( file, "%d,", ( int ) *p );
	    p++;
	}
    }
    if ( image->zip_image_color_values && image->zip_image_color_values->line )
	fprintf( file, "CL%g,%g,%g\n", image->zip_image_color_values->line->red,
  	      image->zip_image_color_values->line->green, image->zip_image_color_values->line->blue );
    if ( image->zip_image_color_values && image->zip_image_color_values->fillfg )
	fprintf( file, "CL%g,%g,%g\n", image->zip_image_color_values->fillfg->red,
  	      image->zip_image_color_values->fillfg->green, image->zip_image_color_values->fillfg->blue );
    if ( image->zip_image_color_values && image->zip_image_color_values->fillbg )
	fprintf( file, "CL%g,%g,%g\n", image->zip_image_color_values->fillbg->red,
  	      image->zip_image_color_values->fillbg->green, image->zip_image_color_values->fillbg->blue );

/*  if ( status == zip_ok  &&  new_line == 0 )
    {
    fprintf( file, "\n" );
    } */
  OUT(Write_Image_Beginning);
  return status;
  }

static int
Write_Image_Ending( class zip		      *self, zip_type_image	       image )
      {
  FILE			     *file =
	image->zip_image_stream->zip_stream_file;
  int			      status = zip_ok, new_line = 0;

  IN(Write_Image_Ending);
  fprintf( file, "}" );
  if ( status == zip_ok  &&  image->zip_image_name )
    {
    new_line++;
    fprintf( file, "%s\n", image->zip_image_name );
    }
  if ( status == zip_ok  &&  new_line == 0 )
    {
    fprintf( file, "\n" );
    }
  OUT(Write_Image_Ending);
  return status;
  }
