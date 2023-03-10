#ifndef _zipfig_h_
#define _zipfig_h_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libzip
 * @{ */
/* zipfig.h	Zip Subroutine Library Figure Objects Header		      */
/* Author	TC Peters						      */
/* Information Technology Center		   Carnegie-Mellon University */

/***  SPECIFICATION -- External Facility Suite  *********************************

TITLE	Zip Subroutine Library Figure Objects Header

MODULE	zipfig.h

NOTICE	IBM Internal Use Only

DESCRIPTION
	This file is to be included in the compilation of both client-programs
	and the Zip Subroutine Library modules. It defines the Figure structure
	and symbolics related to that object.

HISTORY
  05/01/85	Created (TCP)
  12/10/85	Changed zip_points_allocation from 5 to 2 (TCP)
  01/10/86	Add Print symbolic (TCP)
  05/01/86	Add Photo and Glide symbolics (TCP)
		Add zip_dot_font_name
  05/15/86      Added zip_Containing_... casting objects (RM LeVine)
  05/28/86      Added zip_Figure casting object (RML)
  07/03/86	Added more castings (TCP)
		Changed zip_dot_icon to 'A'
  07/06/86	Add set_point function (TCP)
		Add point-symbolics (origin, auxiliary, etc)
		Add text-placement symbolics (top, bottom, etc)
  07/15/86	Changed the font stuff to be "short" and not struct font. (RML)
  07/15/86	Added "hidden" state for figures (TCP)
  07/20/86	Reduced zip_proximal_point from 5 to 2 (TCP)
  07/21/86	Add zip_write_file and zip_adjust_point_suite (TCP)
		Revert to 5 for zip_proximal_distance
  07/29/86	Add zip_figure_mode (TCP)
  08/15/86	Add zip_figure_visibility (TCP)
  09/05/86	Add ellipse, roundangle, and arc symbolics (TCP)
		Add deleted state
  09/11/86	Use "zipdots" rather than "aptdots" (TCP)
  01/13/87	Change old zip_type_point to zip_typ_point_pair (TCP)
		(To introduce proper use of zip_type_point)
  03/05/87	Add Detail cell in Figure-object (TCP)
  07/16/87	Add zip_Figure_Root (TCP)
  07/21/87	Add "style" to Figure-object (TCP)
  03/31/88	Revised for ATK (TCP)
  05/01/89	Use symbolic font-names (TCP)
   08/14/90	Revise figure struct for new color and line style attributes (SCG)

END-SPECIFICATION  ************************************************************/

#define  zip_caption_figure			  1	/* A */
#define  zip_flexible_caption_figure		  2	/* B */
#define  zip_line_figure			  3	/* C */
#define  zip_poly_line_figure			  4	/* D */
#define  zip_polygon_figure			  5	/* E */
#define  zip_trapezoid_figure			  6	/* F */
#define  zip_rectangle_figure			  7	/* G */
#define  zip_path_figure			  8	/* H */
#define  zip_imbed_figure			  9	/* I */
#define  zip_circle_figure			  10	/* J */
#define  zip_photo_figure			  11	/* K */
#define  zip_ellipse_figure			  12	/* L */
#define  zip_arc_figure	    			  13	/* M */
#define  zip_roundangle_figure			  14	/* N */
#define  zip_arrow_figure			  15	/* O */
#define  zip_symbol_figure			  16	/* P */


#define  zip_draw				  1
#define  zip_clear				  2
#define  zip_print				  3
#define  zip_proximate_points			  4
#define  zip_highlight_points			  5
#define  zip_normalize_points			  6
#define  zip_expose_points			  7
#define  zip_hide_points			  8
#define  zip_glide				  9
#define  zip_set_point				  10
#define  zip_write_file				  11
#define  zip_adjust_point_suite			  12
#define  zip_draw_list				  13

#define  zip_figure_origin_point		  1
#define  zip_figure_auxiliary_point		  2

#define  zip_figure_exposed			  1
#define  zip_figure_hidden			  2

#define  zip_top				  (1<<0)
#define  zip_middle				  (1<<1)
#define  zip_baseline				  (1<<2)
#define  zip_bottom				  (1<<3)
#define  zip_left				  (1<<4)
#define  zip_center				  (1<<5)
#define  zip_right				  (1<<6)
#define  zip_halo				  (1<<7)

#define  zip_figure_style_default		  0
#define  zip_figure_style_broad			  1
#define  zip_figure_style_dotted		  2
#define  zip_figure_style_dashed		  3

#define  zip_points_allocation			  2
#define  zip_proximal_distance			  5
#define  zip_points_font_name			  "andysans6"
#define  zip_icon_font_name			  IconFontName

#define  zip_dot_icon				  'A'
#define  zip_dot_font_name 			  DotFontName

typedef long					  zip_type_point;

typedef  struct zip_point_pair			  zip_type_point_pair;
struct zip_point_pair
  {
  zip_type_point				  zip_point_x;
  zip_type_point				  zip_point_y;
  };


typedef  struct zip_point_pairs			 *zip_type_point_pairs;
struct zip_point_pairs
  {
  long						  zip_points_count;
  zip_type_point_pair				  zip_points[1];
  };

typedef  struct zip_figure_state		  zip_type_figure_state;
struct zip_figure_state
  {
  unsigned int					  zip_figure_state_points_exposed	:1;
  unsigned int					  zip_figure_state_points_highlighted	:1;
  unsigned int					  zip_figure_state_deleted		:1;
  unsigned int					  zip_figure_state_point_deleted	:1;
  unsigned int					  zip_figure_state_unhooked		:1;
  };


typedef  struct zip_figure_mode			  zip_type_figure_mode;
struct zip_figure_mode
  {
  unsigned int					  zip_figure_mode_top			:1;
  unsigned int					  zip_figure_mode_middle		:1;
  unsigned int					  zip_figure_mode_baseline		:1;
  unsigned int					  zip_figure_mode_bottom		:1;
  unsigned int					  zip_figure_mode_left			:1;
  unsigned int					  zip_figure_mode_center		:1;
  unsigned int					  zip_figure_mode_right			:1;
  unsigned int					  zip_figure_mode_shaded		:1;
  unsigned int					  zip_figure_mode_patterned		:1;
  unsigned int					  zip_figure_mode_halo			:1;
  };

#include <graphic.H>

typedef  struct zip_figure			 *zip_type_figure;
struct zip_figure
  {
  zip_type_figure				  zip_figure_next;
  char						 *zip_figure_name;
  struct zip_image				 *zip_figure_image;
  union
    {
    char					 *zip_figure_text;
    char					 *zip_figure_anchor;
    }			zip_figure_datum;
  short						  zip_figure_font;
  unsigned char					  zip_figure_type;
  zip_type_figure_state				  zip_figure_state;
  unsigned char					  zip_figure_visibility;
  zip_type_figure_mode				  zip_figure_mode;
  unsigned char					  zip_figure_style;
  unsigned char					  zip_figure_line_width;
  graphic::LineCap				  zip_figure_line_cap;
  graphic::LineJoin				  zip_figure_line_join;
  unsigned char					  *zip_figure_line_dash_pattern;
  int						  zip_figure_line_dash_offset;
  graphic::LineDash				  zip_figure_line_dash_type;
  struct zip_color_values			  *zip_figure_color_values;
  union
    {
    unsigned char				  zip_figure_pattern;
    unsigned char				  zip_figure_shade;
    }			zip_figure_fill;
  unsigned char					  zip_figure_zoom_level;
  unsigned char					  zip_figure_detail_level;
  zip_type_point_pair				  zip_figure_point;
  zip_type_point_pairs				  zip_figure_points;
  };
/** @} */
#endif /* _zipfig_h_ */
