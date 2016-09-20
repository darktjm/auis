#ifndef _zipprint_h_
#define _zipprint_h_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libzip
 * @{ */
/* zipefr00.h	Zip  Subroutine Library Printing Objects Header			*/
/* Author	TC Peters & RM LeVine					        */
/* Information Technology Center		   Carnegie-Mellon University   */

/***  SPECIFICATION -- External Facility Suite  *********************************

TITLE	Zip Figure X

MODULE	zipefr00.h

NOTICE	IBM Internal Use Only

DESCRIPTION
	These facilities are the External structure definitions and some macros.

HISTORY
  07/04/86	Created (RM LeVine)
  08/07/86	Clean-up (TCP)
  09/22/87	Add Print Language (TCP)
   08/14/90	Add fields for line styles to zip_printing struct (SCG)
  
END-SPECIFICATION ************************************************************/

#define  zip_troff				1
#define  zip_postscript				2

#define  zip_printing_page_width_default 	8
#define  zip_printing_page_height_default	11
#define  zip_printing_resolution_default	7200

#define  Printing			  self->view_object->printing

typedef  struct zip_printing		 *zip_type_printing;

struct zip_printing
  {
  long					  zip_printing_pel_width;
  long					  zip_printing_pel_height;
  float					  zip_printing_inch_width;
  float					  zip_printing_inch_height;
  float					  zip_printing_user_width;
  float					  zip_printing_user_height;
  float					  zip_printing_resolution; /* Number of dots per inch */
  float					  zip_printing_stretch_divisor;
  float					  zip_printing_stretch_multiplier;
  float					  zip_printing_stretch_zoom_multiplier;
  zip_type_percent			  zip_printing_height_percent;
  zip_type_percent			  zip_printing_width_percent;
  zip_type_percent			  zip_printing_x_origin_percent;
  zip_type_percent			  zip_printing_y_origin_percent;
					  /* Page info */
  FILE					 *zip_printing_file;
  zip_type_pixel			  zip_printing_current_x;
  zip_type_pixel			  zip_printing_current_y;
  char					  zip_printing_language;
  char					  zip_printing_processor;
  boolean				  zip_printing_level;
  long					  zip_printing_orientation;
  const char				 *zip_printing_prefix;
  char					  zip_printing_line_width;
  const char				 *zip_printing_line_dash_pattern;
  int					  zip_printing_line_dash_offset;
  short					  zip_printing_line_cap;
  short					  zip_printing_line_join;
  float					  zip_printing_shade;
  };

#define  zipprint_Printing_File( Print )\
	(Printing->zip_printing_file)

#define  zipprint_Printing_Prefix( Print )\
	(Printing->zip_printing_prefix)

#define  zipprint_Printing_X_Origin( Print )\
	(Printing->zip_printing_pel_width/2)
#define  zipprint_Printing_Y_Origin( Print )\
	(Printing->zip_printing_pel_height/2)

#define  zipprint_Printing_Current_X( Print )\
	(Printing->zip_printing_current_x)
#define  zipprint_Printing_Current_Y( Print )\
	(Printing->zip_printing_current_y)

#define  zipprint_Printing_Line_Width( Print )\
	(Printing->zip_printing_line_width)

#define  zipprint_Printing_Shade( Print )\
	(Printing->zip_printing_shade)
/** @} */
#endif /* _zipprint_h_*/
