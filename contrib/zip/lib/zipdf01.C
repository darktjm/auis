/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* zipedf01.c	Zip Data-object	-- Figures  Generics		      */
/* Author	TC Peters					      */
/* Information Technology Center	   Carnegie-Mellon University */


/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Zip Data-object -- Figures  Generics

MODULE	zipedf01.c

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
  11/17/88	Add Superior_Image_Line_Width (TCP/SCG)
  05/10/89	Improve behaviour of Superior_Image_Line_Width (SCG)
   08/14/90	Add Superior_Image_<Attribute> methods for color and line styles (SCG)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include "dataobject.H"
#include "zip.H"
#include <sys/stat.h>
#include <ctype.h>

#define	 Data			      this



char
zip::Superior_Image_Pattern( zip_type_image	       image )
      {
  char			      pattern = 0;

  while ( image )
    {
    if ( image->zip_image_fill.zip_image_pattern )
      {
      pattern = image->zip_image_fill.zip_image_pattern;
      break;
      }
    image = image->zip_image_superior;
    }
  return pattern;
  }

char
zip::Superior_Image_Shade( zip_type_image	       image )
      {
  char			      shade = 0;

  while ( image )
    {
    if ( image->zip_image_fill.zip_image_shade )
      {
      shade = image->zip_image_fill.zip_image_shade;
      break;
      }
    image = image->zip_image_superior;
    }
  return shade;
  }

unsigned char
zip::Superior_Image_Line_Width( zip_type_image	       image )
      {
  unsigned char	      width = 255;

  while ( image )
    {
    if ( image->zip_image_line_width != 255 )
      {
      width = image->zip_image_line_width;
      break;
      }
    image = image->zip_image_superior;
    }
  return width;
  }

long
zip::Superior_Image_Line_Dash( zip_type_image	       image, unsigned char			      **pattern, int			      *offset, graphic::LineDash		      *type )
            {
  long			    status = zip_failure;

  while ( image )
    {
    if ( image->zip_image_line_dash_pattern )
      {
      *pattern = image->zip_image_line_dash_pattern;
      *offset = image->zip_image_line_dash_offset;
      *type = image->zip_image_line_dash_type;
      status = zip_ok;
      break;
      }
    image = image->zip_image_superior;
    }
  return status;
  }

graphic::LineCap
zip::Superior_Image_Line_Cap( zip_type_image	       image )
      {
  short	      cap = -1;

  while ( image )
    {
    if ( image->zip_image_line_cap != -1 )
      {
      cap = image->zip_image_line_cap;
      break;
      }
    image = image->zip_image_superior;
    }
  return (graphic::LineCap)cap; // tjm - -1 isn't really LineCap
  }

graphic::LineJoin
zip::Superior_Image_Line_Join( zip_type_image	       image )
      {
  short	      join = -1;

  while ( image )
    {
    if ( image->zip_image_line_join != -1 )
      {
      join = image->zip_image_line_join;
      break;
      }
    image = image->zip_image_superior;
    }
  return (graphic::LineJoin)join; // tjm - -1 isn't really LineJoin
  }

struct zip_color *
zip::Superior_Image_Line_Color( zip_type_image	     image )
      {
  struct zip_color	    *color = NULL;

  while ( image )
    {
    if ( image->zip_image_color_values && image->zip_image_color_values->line )
      {
      color = image->zip_image_color_values->line;
      break;
      }
    image = image->zip_image_superior;
    }
  return color;
  }

struct zip_color *
zip::Superior_Image_FillFG_Color( zip_type_image	     image )
      {
  struct zip_color	    *color = NULL;

  while ( image )
    {
    if ( image->zip_image_color_values && image->zip_image_color_values->fillfg )
      {
      color = image->zip_image_color_values->fillfg;
      break;
      }
    image = image->zip_image_superior;
    }
  return color;
  }

struct zip_color *
zip::Superior_Image_FillBG_Color( zip_type_image	     image )
      {
  struct zip_color	    *color = NULL;

  while ( image )
    {
    if ( image->zip_image_color_values && image->zip_image_color_values->fillbg )
      {
      color = image->zip_image_color_values->fillbg;
      break;
      }
    image = image->zip_image_superior;
    }
  return color;
  }

char *
zip::Superior_Image_Text( zip_type_image	       image )
      {
  char			     *text = NULL;

  while ( image )
    {
    if ( image->zip_image_text )
      {
      text = image->zip_image_text;
      break;
      }
    image = image->zip_image_superior;
    }
  return text;
  }


class fontdesc *
zip::Superior_Image_Font( zip_type_image	       image )
      {
  class fontdesc 	     *font = NULL;

  while ( image )
    {
    if ( image->zip_image_font )
      {
      font = Fonts->
		zip_fonts_vector[image->zip_image_font].zip_fonts_table_font;
      break;
      }
    image = image->zip_image_superior;
    }
  return font;
  }
