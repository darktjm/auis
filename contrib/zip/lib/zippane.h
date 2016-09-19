#ifndef _zippane_h
#define _zippane_h
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

/* zipefp00.h	Zip Subroutine Library Pane Objects Header		      */
/* Author	TC Peters						      */
/* Information Technology Center		   Carnegie-Mellon University */

/***  SPECIFICATION -- External Facility Suite  *********************************

TITLE	Zip Subroutine Library Pane Objects Header

MODULE	zipefp00.h

NOTICE	IBM Internal Use Only

DESCRIPTION
	This file is to be included in the compilation of both client-programs
	and the Zip Subroutine Library modules. It defines the Pane structure
	and symbolics related to that object.

END-SPECIFICATION  ************************************************************/

#include <view.H>

#define  ZIP_pane_coordinate_thickness		26
#define  ZIP_pane_coordinate_font_name		"andysans8"
#define  ZIP_pane_coordinate_mark_spacing_min	20

#define  zip_figure_point_icon			  '1'
#define  zip_figure_point_suite_icon		  '2'
#define  zip_image_point_suite_icon		  '3'

typedef  struct zip_palette			 *zip_type_palette;

struct zip_palette
  {
  char						  zip_palette_background;
  };


typedef  struct zip_pane_attributes2		  zip_type_pane_attributes;

struct zip_pane_attributes2
  {
  unsigned int					  zip_pane_attribute_stream_source	:1;
  unsigned int					  zip_pane_attribute_image_source	:1;
  unsigned int					  zip_pane_attribute_figure_source	:1;
  unsigned int					  zip_pane_attribute_block_area		:1;
  unsigned int					  zip_pane_attribute_pane_area		:1;
  unsigned int					  zip_pane_attribute_overlay		:1;
  unsigned int					  zip_pane_attribute_inverted		:1;
  unsigned int					  zip_pane_attribute_balanced		:1;
  unsigned int					  zip_pane_attribute_transparent	:1;
  };


typedef  struct zip_pane_state2			  zip_type_pane_state;

struct zip_pane_state2
  {
  unsigned int					  zip_pane_state_points_exposed		:1;
  unsigned int					  zip_pane_state_points_highlighted	:1;
  unsigned int					  zip_pane_state_coordinates_exposed	:1;
  unsigned int					  zip_pane_state_exposed		:1;
  unsigned int					  zip_pane_state_hidden			:1;
  unsigned int					  zip_pane_state_removed		:1;
  unsigned int					  zip_pane_state_inverted		:1;
  unsigned int					  zip_pane_state_paint_inverted		:1;
  unsigned int					  zip_pane_state_paint_copy		:1;
  };

#define  zip_pane_exposed			  1
#define  zip_pane_hidden			  2

typedef  struct zip_pane			 *zip_type_pane;

struct zip_pane
  {
  char						 *zip_pane_name;
  zip_type_pane_attributes			  zip_pane_attributes;
  zip_type_pane_state				  zip_pane_state;
  short					 	  zip_pane_border_font;
  char						  zip_pane_border_pattern;
  short						  zip_pane_border_thickness;
  zip_type_palette				  zip_pane_palette;
  zip_type_percent				  zip_pane_x_origin_percent;
  zip_type_percent				  zip_pane_y_origin_percent;
  zip_type_percent				  zip_pane_width_percent;
  zip_type_percent				  zip_pane_height_percent;
  zip_type_pixel				  zip_pane_x_origin;
  zip_type_pixel				  zip_pane_y_origin;
  zip_type_pixel				  zip_pane_left;
  zip_type_pixel				  zip_pane_top;
  zip_type_pixel				  zip_pane_width;
  zip_type_pixel				  zip_pane_height;
  zip_type_pixel				  zip_pane_x_offset;
  zip_type_pixel				  zip_pane_y_offset;
  float						  zip_pane_stretch_multiplier;
  float						  zip_pane_stretch_zoom_multiplier;
  float						  zip_pane_stretch_divisor;
  float						  zip_pane_scale;
  short						  zip_pane_x_flip;
  short						  zip_pane_y_flop;
  short						  zip_pane_zoom_level;
  short						  zip_pane_detail_level;
  char						 *zip_pane_client_data;
  union
    {
    struct rectangle				 *zip_pane_block;
    zip_type_pane    				  zip_pane_pane;
    }			zip_pane_area;
  union
    {
    zip_type_stream				  zip_pane_stream;
    zip_type_image				  zip_pane_image;
    zip_type_figure				  zip_pane_figure;
    }			zip_pane_source;
  zip_type_stream				  zip_pane_current_stream;
  zip_type_image				  zip_pane_current_image;
  zip_type_figure				  zip_pane_current_figure;
  short						  zip_pane_current_font;
  char						  zip_pane_highlight_icon;
  short						  zip_pane_pixels_per_point;
  short						  zip_pane_points_per_pixel;
  struct zip_pane_palettes			 *zip_pane_palette_panes;
  char						  zip_pane_cursor_icon;
  struct fontdesc				 *zip_pane_cursor_font;
  struct cursor					 *zip_pane_cursor_glyph;
  short						  zip_pane_preserved_border_thickness;
  short						  zip_pane_panning_precision;
  struct zip_pane_edit				 *zip_pane_edit;
  struct zip_pane_auxiliary_stream		 *zip_pane_auxiliary_stream;
  long						  zip_pane_hit_processor_anchor;
  long						  zip_pane_display_processor_anchor;
  long						(*zip_pane_hit_processor)( long anchor,
									  zip_type_pane pane,
									  enum view_MouseAction action,
									  long x, long y, long clicks ),
						(*zip_pane_display_preprocessor)( long anchor,
										 zip_type_pane pane,
										 long action ),
						(*zip_pane_display_processor)( long anchor,
									      zip_type_pane pane,
									      long action ),
  						(*zip_pane_display_postprocessor)( long anchor,
										  zip_type_pane pane,
										  long action );
  zip_type_figure_mode				  zip_pane_current_mode;
  long						  zip_pane_object_width;
  long						  zip_pane_object_height;
  zip_type_pixel				  zip_pane_x_origin_offset;
  zip_type_pixel				  zip_pane_y_origin_offset;
  };

typedef	 struct zip_pane_auxiliary_stream	 *zip_type_pane_auxiliary_stream;

struct zip_pane_auxiliary_stream
  {
  zip_type_pane_auxiliary_stream		  zip_pane_auxiliary_stream_next;
  zip_type_stream				  zip_pane_auxiliary_stream_ptr;
  char						  zip_pane_auxiliary_stream_visibility;
  char						  zip_pane_auxiliary_stream_density;
  };

struct zip_pane_palettes
  {
  int						  zip_pane_palette_count;
  zip_type_pane					  zip_pane_palette_vector[100];
  };

typedef  struct zip_pane_chain			 *zip_type_pane_chain;

struct zip_pane_chain
  {
  zip_type_pane_chain				  zip_pane_chain_next;
  zip_type_pane					  zip_pane_chain_ptr;
  };

typedef  struct zip_pane_edit	 *zip_type_pane_edit;

struct zip_pane_edit
  {
  long				  zip_pane_edit_last_x_pixel;
  long				  zip_pane_edit_last_y_pixel;
  long				  zip_pane_edit_last_x_point;
  long				  zip_pane_edit_last_y_point;
  long				  zip_pane_edit_last_point_id;
  long				  zip_pane_edit_region_id;
  struct zip_pane		 *zip_pane_edit_prior_pane;
  long				  zip_pane_edit_build_constraint;
  long				  zip_pane_edit_build_pending;
  char				  zip_pane_edit_build_figure;
  char				  zip_pane_edit_beginning;
  char				  zip_pane_edit_selection_level;
  int				  zip_pane_edit_coordinate_grid;
  unsigned int                    zip_pane_edit_grid_exposed;
  unsigned int			  zip_pane_edit_constrain_points;
  float				  zip_pane_edit_mark_point_delta;
  long				  zip_pane_edit_mark_point_spacing;
  long				  zip_pane_edit_palette_mode;
  struct zip_pane		 *zip_pane_edit_containing_pane;
  unsigned char			  zip_pane_edit_current_shade;
  unsigned char			  zip_pane_edit_current_pattern;
  unsigned char			  zip_pane_edit_current_line_width;
  unsigned char			  zip_pane_edit_current_line_style;
  unsigned char			  zip_pane_edit_current_font;
  unsigned char			  zip_pane_edit_current_mode;
  struct zip_pane		 *zip_pane_edit_background_pane;
  struct raster			 *zip_pane_edit_background_data;
  struct rasterview	    	 *zip_pane_edit_background_view;
  };


typedef  struct ZIP_edit_client_data	 *ZIP_ecd;
struct ZIP_edit_client_data
  {
  char				  cursor_icon;
  char				  figure_type;
  char				 *prior_client_data;
  zip_type_percent		  prior_x_origin;
  zip_type_percent		  prior_y_origin;
  zip_type_percent		  prior_width;
  zip_type_percent		  prior_height;
  short				  prior_border_font;
  char				  prior_border_pattern;
  short				  prior_border_thickness;
  zip_type_pane			  editing_pane;
  char				  foreground_panning;
  char				  foreground_pinning;
  };

#endif /* _zippane_h */
