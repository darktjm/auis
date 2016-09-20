#ifndef _zipiff00_h_
#define _zipiff00_h_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libzip
 * @{ */
/* zipiff00.h	Zip Figure Visual Manipulator -- Header File			      */
/* Author	TC Peters						      */
/* Information Technology Center		   Carnegie-Mellon University */

/***  SPECIFICATION -- Internal Facility Suite  *********************************

TITLE	Zip Figure Visual Manipulator -- Header File

MODULE	zipiff00.h

NOTICE	IBM Internal Use Only

DESCRIPTION
	Performs all manipulations of displayed Figure imagery. Generic Facilities
	perform drawing, clearing, point-approximation, etc, by indirectly invoking
	the appropriate specific Facility for the given Figure's type. (This is a
	weak form of "object-oriented" structuring.)

HISTORY
  06/26/86	Created (RM LeVine)
  07/15/86	Changed the casting of ZIP_Superior_Image_Font so that it is short.
  08/07/86	Remove ZIP_print_file extern (place into printing struct) (TCP)
		Removed reference to ZIP_x/y_Origins from slug-point computation macros
  02/20/87	Try Changing ZIP_X_multiplier and ZIP_Y_divisor to float (TCP)
		-- too slow
  09/31/87	USe ZIP_type_fonts for migration (TCP)
  03/31/88	Revise for ATK (TCP)
  10/27/88	Optimize wind_x_point, etc, by dropping flip/flop multiply (TCP)

END-SPECIFICATION  ************************************************************/

#include "zipifm00.h"

#define  figure_x_point\
	(figure->zip_figure_point.zip_point_x)
#define  figure_y_point\
	(figure->zip_figure_point.zip_point_y)

#define  figure_x_points(i)\
	(figure->zip_figure_points->zip_points[i].zip_point_x)
#define  figure_y_points(i)\
	(figure->zip_figure_points->zip_points[i].zip_point_y)

#define  window_x_point\
	((long)((pane->zip_pane_x_origin_offset) + figure->zip_figure_point.zip_point_x\
	 * (pane->zip_pane_stretch_zoom_multiplier) /\
	 (pane->zip_pane_stretch_divisor)))
#define  window_y_point\
	((long)((pane->zip_pane_y_origin_offset) - figure->zip_figure_point.zip_point_y\
	 * (pane->zip_pane_stretch_zoom_multiplier) /\
	 (pane->zip_pane_stretch_divisor)))

#define  window_x_points(i)\
	((long)((pane->zip_pane_x_origin_offset) +\
	 figure->zip_figure_points->zip_points[i].zip_point_x\
	 * (pane->zip_pane_stretch_zoom_multiplier) /\
	 (pane->zip_pane_stretch_divisor)))
#define  window_y_points(i)\
	((long)((pane->zip_pane_y_origin_offset) -\
	 figure->zip_figure_points->zip_points[i].zip_point_y\
	 * (pane->zip_pane_stretch_zoom_multiplier) /\
	 (pane->zip_pane_stretch_divisor)))

#define print_x_point\
	((long)((Printing->zip_printing_pel_width>>1) + figure->zip_figure_point.zip_point_x\
	 * (Printing->zip_printing_stretch_zoom_multiplier) /\
	 (Printing->zip_printing_stretch_divisor)))
#define print_y_point\
	((long)((Printing->zip_printing_pel_height>>1) - figure->zip_figure_point.zip_point_y\
	 * (Printing->zip_printing_stretch_zoom_multiplier) /\
	 (Printing->zip_printing_stretch_divisor)))
#define print_x_length	\
	((long)( figure->zip_figure_point.zip_point_x\
	 * (Printing->zip_printing_stretch_zoom_multiplier) /\
	 (Printing->zip_printing_stretch_divisor)))
#define print_y_length \
	((long)( figure->zip_figure_point.zip_point_y\
	 * (Printing->zip_printing_stretch_zoom_multiplier) /\
	 (Printing->zip_printing_stretch_divisor)))

#define print_x_points(i) \
	((long)(	(Printing->zip_printing_pel_width>>1) + \
	 figure->zip_figure_points->zip_points[i].zip_point_x \
	 * (Printing->zip_printing_stretch_zoom_multiplier) / \
	 (Printing->zip_printing_stretch_divisor)))
#define print_y_points(i) \
	((long)( (Printing->zip_printing_pel_height>>1) - \
	 figure->zip_figure_points->zip_points[i].zip_point_y \
	 * (Printing->zip_printing_stretch_zoom_multiplier) / \
	 (Printing->zip_printing_stretch_divisor)))
#define print_x_lengths(i) \
	((long)( figure->zip_figure_points->zip_points[i].zip_point_x \
	 * (Printing->zip_printing_stretch_zoom_multiplier) / \
	 (Printing->zip_printing_stretch_divisor)))
#define print_y_lengths(i) \
	((long)( figure->zip_figure_points->zip_points[i].zip_point_y \
	 * (Printing->zip_printing_stretch_zoom_multiplier) / \
	 (Printing->zip_printing_stretch_divisor)))

#define  ExposePoints \
	    (pane->zip_pane_state.zip_pane_state_points_exposed  || \
	     figure->zip_figure_state.zip_figure_state_points_exposed)
#define  HighlightPoints \
	    (pane->zip_pane_state.zip_pane_state_points_highlighted  || \
	     figure->zip_figure_state.zip_figure_state_points_highlighted)
/** @} */
#endif /* _zipiff00_h_ */
