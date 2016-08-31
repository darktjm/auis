/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
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

/* zipds01.c	Zip Data-object	-- Stream Input Parsing		      */
/* Author	TC Peters					      */
/* Information Technology Center	   Carnegie-Mellon University */



/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Zip Data-object -- Stream Input Parsing

MODULE	zipds01.c

NOTICE	IBM Internal Use Only

DESCRIPTION
	This is the suite of Methods that support the Stream facilities
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
   08/14/90	Add code to deparse color and line style attributes (SCG)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include <dataobject.H>
#include "zip.H"
#include "zipobject.H"
#include <ctype.h>


#define	 Data			      (this)
#define	 Objects(i)		      ((self->objects)[i])

static  zip_type_stream		      stream;
static  zip_type_image		      Image;
static  zip_type_figure		      figure;
static  char			      msg[512];
static  int			      position;

long zip_Deparse_Stream( class zip		      *self, zip_type_stream	       stream_object );
static int Substitute_Referenced_Stream( class zip		      *self );
static int Parse_Figure_Unit_Attributes( class zip		      *self, zip_type_figure	       figure );
static int Parse_Stream_Figure( class zip		      *self );
static int Parse_Stream_Image_Beginning( class zip		      *self );
static int Parse_Stream_Image_Ending( class zip		      *self );
static int Parse_Image_Attributes( class zip		      *self );
static int Extract_Attribute( class zip		      *self, char			     **attribute_ptr );
static char * Unique_Name( char			      *name, int			       seed );
static int Parse_Stream_Integer();
static double Parse_Stream_Real();
static int Parse_Stream_Commentary( class zip *self, char c );
static int Equivalent_Token( class zip		      *self, const char			      *token, const char			      * const table[] );
static long Parse_Presentation_Parameter( class zip		      *self );
static char NextChar();
static char PriorChar( char				      c );


long
zip::Read_Figure( zip_type_figure	       figure )
      {
  int			      status = zip_ok;

  IN(zip::Read_Figure);
  NextChar();
  if ( (status = (this)->Parse_Figure_Point(  figure,
		    &figure->zip_figure_point.zip_point_x,
		    &figure->zip_figure_point.zip_point_y )) == zip_ok )
    {
    (this)->Set_Image_Extrema(  (this)->Figure_Image(  figure ),
		    figure->zip_figure_point.zip_point_x,
		    figure->zip_figure_point.zip_point_y );
    Parse_Figure_Unit_Attributes( this, figure );
    if ( (status = (this)->Parse_Figure_Attributes(  figure )) == zip_ok )
      status = (this)->Parse_Figure_Points(  figure );
    }
  DEBUGdt(Status,status);
  OUT(zip::Read_Figure);
  return status;
  }

long
zip_Deparse_Stream( class zip		      *self, zip_type_stream	       stream_object )
      {
  long			      status = zip_ok;
  char			      c;
/*===debug=1;===*/
  IN(zip_Parse_Stream);
  figure = NULL;
  stream = stream_object;
  position = 0;
  if (( Image = (self)->Image_Root(  stream_object )) == NULL )
    {
    if ( (status = (self)->Create_Inferior_Image(  &Image,
		     "ZIP_ROOT_IMAGE", stream, NULL )) == zip_ok )
      Image->zip_image_state.zip_image_state_transient = true;
    stream->zip_stream_image_anchor = Image;
    }
  stream->zip_stream_greatest_x = 0;
  stream->zip_stream_greatest_y = 0;
/*===  stream->zip_stream_pseudo_x_offset = 0;
  stream->zip_stream_pseudo_y_offset = 0;===*/
  c = NextChar();
  while ( c  &&  status == zip_ok )
    {
    switch ( c )
      {
      case '*':
        status = Parse_Stream_Figure( self );
        break;
      case '{':
	status = Parse_Stream_Image_Beginning( self );
        break;
      case '}':
	status = Parse_Stream_Image_Ending( self );
        break;
      case '\\':
        status = (Objects(figure->zip_figure_type))->Read_Object_Stream(
		  figure,
		 stream->zip_stream_file, 12345/*===*/ );
        break;
      case '%':
	status = Parse_Presentation_Parameter( self );
	break;
      case '!':
	status = Substitute_Referenced_Stream( self );
	break;
      default:
	if ( !Parse_Stream_Commentary( self, c ) )
          status = zip_stream_positioning_error;
      }
    c = NextChar();
    }
  if ( status == zip_ok )
    {
    if ( Image  &&  Image->zip_image_figure_anchor )
	(self)->Set_Stream_Extrema(  stream, Image );
    }
    else
    {
    DEBUGdt( BAD status, status );
    DEBUGct( CURSOR AT, c);
    DEBUGdt( POSITION, position);
    }
  OUT(zip_Deparse_Stream);
  return status;
  }

extern int zip_Close_Stream_File( class zip *self, zip_type_stream stream );
extern int zip_Set_Stream_File_Name( class zip *self, zip_type_stream stream, const char *name );
extern int zip_Open_Stream_File( class zip *self, zip_type_stream stream, long open_mode );

static int
Substitute_Referenced_Stream( class zip		      *self )
    {
  long			      status;
  char				     *file_name;

  IN(Substitute_Referenced_Stream);
  if ( (status = Extract_Attribute( self, &file_name )) == zip_ok )
    {
    DEBUGst(File-name,file_name);
    zip_Close_Stream_File( self, stream );
    if ( (status = zip_Set_Stream_File_Name( self, stream, file_name )) == zip_ok )
      {
      if ( (status = zip_Open_Stream_File( self, stream, zip_default )) == zip_ok )
	stream->zip_stream_attributes.zip_stream_attribute_reference = true;
      }
    }
  OUT(Substitute_Referenced_Stream);
  return  status;
  }

long
zip::Parse_Figure_Point( zip_type_figure figure, zip_type_point *x , zip_type_point *y )
{
  int			      negative;
  int			      v;
  char			      c;

  IN(zip::Parse_Figure_Point);
  c = NextChar();
    negative = false;
    if ( c == '-' )
      {
      negative = true;
      c = NextChar();
      }
    v = 0;
    while ( c >= '0'  &&  c <= '9' )
      {
      v = v * 10 + ( c - '0');
      c = NextChar();
      }
    if ( negative )
      *x = -v;
      else
      *x =  v;
    DEBUG(Half-Done);
    if ( c == ',' )
      c = NextChar();
    negative = false;
    if ( c == '-' )
      {
      negative = true;
      c = NextChar();
      }
    v = 0;
    while ( c >= '0'  &&  c <= '9' )
      {
      v = v * 10 + ( c - '0');
      c = NextChar();
      }
    if ( negative )
      *y = -v;
      else
      *y =  v;
  if ( c != '\n' )
    PriorChar( c );
  OUT(zip::Parse_Figure_Point);
  return zip_ok;
  }

long
zip::Parse_Figure_Points( zip_type_figure	       figure )
      {
  int			      i = 0;
  char			      c;
  long			      status = zip_ok;

  IN(zip::Parse_Figure_Points);
  while ( Parse_Stream_Commentary( this, c = NextChar() ) ) ;
  if ( c == '>'  ||  c == ';' )
    {
    status = (this)->Allocate_Figure_Points_Vector(  &figure->zip_figure_points );
    while ( c  &&  (c == '>'  ||  c == ';')  &&  status == zip_ok )
      {
      if ( (i % zip_points_allocation) == 0 )
        status = (this)->Enlarge_Figure_Points_Vector(  &figure->zip_figure_points );
      if ( status == zip_ok  &&
	  (status = (this)->Parse_Figure_Point(  figure,
			&figure->zip_figure_points->zip_points[i].zip_point_x,
		 	&figure->zip_figure_points->zip_points[i].zip_point_y )) == zip_ok )
        {
        (this)->Set_Image_Extrema(  /*Image*/ (this)->Figure_Image(  figure ), figure->zip_figure_points->zip_points[i].zip_point_x,
		 		 figure->zip_figure_points->zip_points[i].zip_point_y );
        figure->zip_figure_points->zip_points_count = ++i;
        }
      c = NextChar();
      }
    }
  if ( c != '\n' )
    PriorChar( c );
  DEBUGdt(Status,status);
  OUT(zip::Parse_Figure_Points);
  return status; 
  }

static int
Parse_Figure_Unit_Attributes( class zip		      *self, zip_type_figure	       figure )
      {
  char			      c;

  IN(Parse_Figure_Unit_Attributes);
  /* Skip over any obsolete syntactic elements */
  if ( (c = NextChar()) == ';' )
    while ( (c = NextChar()) != '\n' ) ;
    else  PriorChar( c );
  OUT(Parse_Figure_Unit_Attributes);
  return  zip_ok;
  }

long
zip::Parse_Figure_Attributes( zip_type_figure	       figure )
      {
  int			      end_of_attributes, offset, i = 0;
  long			      status = zip_ok;
  char			      c, first, second;
  unsigned char			      element = 1;
  char				     *attribute_ptr,  pattern[10];
  short			cap;
  double			red, green, blue;

  IN(zip::Parse_Figure_Attributes);
  end_of_attributes = false;
  while ( ! end_of_attributes  &&  status == zip_ok )
    {
    switch ( c = NextChar() )
      {
      case  'A':	/* Annotation */
	DEBUG(Attribute A);
	status = Extract_Attribute( this, &attribute_ptr
			     /*===&figure->zip_figure_annotation===*/ );
	break;
      case  'C':	/* Color */
	DEBUG(Attribute C);
	c = NextChar();
        red = Parse_Stream_Real(); NextChar();
        green = Parse_Stream_Real(); NextChar();
        blue = Parse_Stream_Real();
	switch ( c )
        {
	    case 'L': (this)->Set_Figure_Line_Color(  figure, red, green, blue ); break;
       	    case 'F': (this)->Set_Figure_FillFG_Color(  figure, red, green, blue ); break;
       	    case 'B': (this)->Set_Figure_FillBG_Color(  figure, red, green, blue ); break;
	}
        break;
      case  'M':	/* Mode */
	DEBUG(Attribute M);
        first  = c = NextChar();/*===cleanse===*/
	DEBUGct(first,first);
        second = c = NextChar();
        DEBUGct(second,second);
	if ( first == 'T'  ||  first == 't'  || second == 'T'  || second == 't' )
	  figure->zip_figure_mode.zip_figure_mode_top = on;
	if ( first == 'M'  ||  first == 'm'  || second == 'M'  || second == 'm' )
	  figure->zip_figure_mode.zip_figure_mode_middle = on;
	if ( first == 'B'  ||  first == 'b'  || second == 'B'  || second == 'b' )
	  figure->zip_figure_mode.zip_figure_mode_bottom = on;
	if ( first == 'L'  ||  first == 'l'  || second == 'L'  || second == 'l' )
	  figure->zip_figure_mode.zip_figure_mode_left = on;
	if ( first == 'C'  ||  first == 'c'  || second == 'C'  || second == 'c' )
	  figure->zip_figure_mode.zip_figure_mode_center = on;
	if ( first == 'R'  ||  first == 'r'  || second == 'R'  || second == 'r' )
	  figure->zip_figure_mode.zip_figure_mode_right = on;
	if ( (c = NextChar()) == 'H' )
	  figure->zip_figure_mode.zip_figure_mode_halo = on;
	  else  PriorChar( c );
        break;
      case  'P':	/* Pattern */
	DEBUG(Attribute P);
	figure->zip_figure_mode.zip_figure_mode_patterned = on;
	figure->zip_figure_fill.zip_figure_pattern = NextChar();
        break;
      case  'G':	/* Grey-level Shade */
	DEBUG(Attribute G);
	figure->zip_figure_mode.zip_figure_mode_shaded = on;
	figure->zip_figure_fill.zip_figure_shade = Parse_Stream_Integer();
        break;
      case  'F':	/* Font */
	DEBUG(Attribute F);
	if ( (status = Extract_Attribute( this, &attribute_ptr )) == zip_ok )
	  {
	  (this)->Define_Font(  attribute_ptr, &figure->zip_figure_font );
	  free( attribute_ptr );
/*===Need to set extrema for Caption, but cannot now compute: need to be in View!===*/
	  }
        break;
      case  'N':	/* Name */
	DEBUG(Attribute N);
  	status = Extract_Attribute( this, &attribute_ptr );
        if ( attribute_ptr )
	  {
	  if ( (status = (this)->Set_Figure_Name(  figure, attribute_ptr )) ==
		    zip_duplicate_figure_name )
	    {
	    int		  suffix = 0;
	    char		 *new_name = NULL;

	    while( status == zip_duplicate_figure_name )
	      {
	      status = (this)->Set_Figure_Name(  figure, new_name =
			    Unique_Name( attribute_ptr, suffix++ ) );
	      }
/*===*/printf("Changed Dup Figure Name To '%s'\n", new_name);
	    status = zip_ok;
	    }
	  free( attribute_ptr );
	  }
        break;
      case  'R':	/* Reference */
	DEBUG(Attribute R);
  	status = Extract_Attribute( this, &figure->zip_figure_datum.zip_figure_text );
        break;
      case  'T':	/* Text */
	DEBUG(Attribute T);
	if ( (status = Extract_Attribute( this,
			    &figure->zip_figure_datum.zip_figure_text )) == zip_ok )
	  {
	  if ( figure->zip_figure_type == zip_caption_figure  &&
	       figure->zip_figure_font )
	    {
/*===Need to set extrema for Caption, but cannot now compute: need View!
	    zip_Select_Figure_Font( this, figure );
	    ZIP_WM_StringWidth( figure->zip_figure_datum.zip_figure_text,
				&string_width, &string_height );
            zip_Set_Image_Extrema( this, Image, figure->zip_figure_point.zip_point_x +
					    string_width,
			                  figure->zip_figure_point.zip_point_y );
===*/
	    }
	  }
        break;
      case  'Z':	/* Zoom-level */
	DEBUG(Attribute Z);
        figure->zip_figure_zoom_level = Parse_Stream_Integer();
        break;
      case  'D':	/* Detail-level */
	DEBUG(Attribute D);
	figure->zip_figure_detail_level = Parse_Stream_Integer();
        break;
      case  'S':	/* Style */
	DEBUG(Attribute S);
	figure->zip_figure_style = Parse_Stream_Integer();
        break;
      case  'L':	/* Line ... */
	DEBUG(Attribute L);
	c = NextChar();
	switch ( c )
        {
	    case 'W': figure->zip_figure_line_width = Parse_Stream_Integer(); break;
	    case 'C': if (( c = NextChar()) == 'N' ) cap = graphic_CapNotLast;
		else if ( c == 'B' ) cap = graphic_CapButt;
		else if ( c == 'R' ) cap = graphic_CapRound;
		else if ( c == 'P' ) cap = graphic_CapProjecting;
		else { cap = graphic_CapButt; printf( "zip: unrecognized line cap attribute: '%c'\n", c ); }
		(this)->Set_Figure_Line_Cap(  figure, cap ); break;
	    case 'J': if (( c = NextChar()) == 'M' ) cap = graphic_JoinMiter;
		else if ( c == 'R' ) cap = graphic_JoinRound;
		else if ( c == 'B' ) cap = graphic_JoinBevel;
		else { cap = graphic_JoinMiter; printf( "zip: unrecognized line join attribute: '%c'\n", c ); }
		(this)->Set_Figure_Line_Join(  figure, cap ); break;
	    case 'D': if (( c = NextChar()) == 'S' ) cap = graphic_LineSolid;
	        else if ( c == 'D' ) cap = graphic_LineDoubleDash;
		else if ( c == 'O' ) cap = graphic_LineOnOffDash;
		NextChar(); offset = Parse_Stream_Integer(); NextChar();
		while( element != 0 )
		{
		    pattern[i++] = element = ( char ) Parse_Stream_Integer();
		    if ( element ) NextChar();
		}
		pattern[i] = 0;
		(this)->Set_Figure_Line_Dash(  figure, pattern, offset, cap );
		break;
	}
        break;
      default:
  	DEBUG(Attribute Default);
	if ( ! Parse_Stream_Commentary( this, c ) )
	  {
	  c = PriorChar( c );
	  if ( c != '*'  &&  c != '>'  &&
	       c != '{'  &&  c != '}'  &&
	       c != '\0' )
	    {
	    status = zip_unrecognized_stream_object_attribute;
	    sprintf( msg, "Unrecognized Figure Attribute '%c' At Position %d\n",
		     c, position );
/*===	    while ( apt_Acknowledge( msg ) == -1 );===*/
	    }
	  end_of_attributes = true;
	  }
	break;
      }
    }
  DEBUGdt(Status,status);
  OUT(zip::Parse_Figure_Attributes);
  return status;
  }

static int
Parse_Stream_Figure( class zip		      *self )
    {
  long			      status;
  char			      c;

  IN(Parse_Stream_Figure);
  c = NextChar();
  if ( (status = (self)->Create_Figure(  &figure, NULL, 0, Image, figure )) == zip_ok )
    {
    if ( isascii( c )  &&  isupper( c ) )
      figure->zip_figure_type = c - '@'; /* 'A' == '1', etc */
      else 
      figure->zip_figure_type = c - '0';
    DEBUGdt(Figure-type, figure->zip_figure_type);
    status = (Objects(figure->zip_figure_type))->Read_Object(  figure );
    }
  DEBUGdt(Status,status);
  OUT(Parse_Stream_Figure);
  return status;
  }

static int
Parse_Stream_Image_Beginning( class zip *self )
{
    int			      status, page_count;
    char				     *ptr;

    IN(Parse_Stream_Image_Beginning);
    figure = NULL;
    if ( (status = (self)->Create_Inferior_Image(  &Image, NULL, stream, Image )) ==
	zip_ok ) {
	if ( (status = Extract_Attribute( self, &ptr )) == zip_ok ) {
	    status = Parse_Image_Attributes( self );
	    if ( status == zip_ok  &&  ptr )
		if ( (status = (self)->Set_Image_Name(  Image, ptr )) == zip_duplicate_image_name ) {
		    int		  suffix = 0;
		    char		 *new_name = NULL;

		    while( status == zip_duplicate_image_name ) {
			status = (self)->Set_Image_Name(  Image, new_name =
							Unique_Name( ptr, suffix++ ) );
		    }
                    /*===*/
		    printf("Changed Dup Image Name To '%s'\n", new_name);
		    status = zip_ok;
		}
		else
		    if ( Image->zip_image_name &&
			strncmp( Image->zip_image_name, "ZIP_PAGE_IMAGE_", 15 ) == 0 )
		    {
			page_count = atoi( Image->zip_image_name + 15 );
			DEBUGdt(Page,page_count);
			if ( page_count > self->page_count )
			    self->page_count = page_count;
		    }
	}
    }
    OUT(Parse_Stream_Image_Beginning);
    return status;
}


static int
Parse_Stream_Image_Ending( class zip		      *self )
    {
  int			      status = zip_ok;
  char				     *attribute_ptr;

  IN(Parse_Stream_Image_Ending);
  Extract_Attribute( self, &attribute_ptr );
  if ( Image->zip_image_figure_anchor )
    (self)->Set_Stream_Extrema(  stream, Image );
  Image = Image->zip_image_superior;
  figure = NULL;
  OUT(Parse_Stream_Image_Ending);
  return status;
  }

static int
Parse_Image_Attributes( class zip		      *self )
    {
  short			cap;
  boolean		      end_of_attributes;
  int			      status = zip_ok, offset;
  char				     *attribute_ptr, pattern[10];
  char			      c, element = 1;
  long			      i = 0;
  double			red, green, blue;

  IN(Parse_Image_Attributes);
  end_of_attributes = false;
  while ( ! end_of_attributes  &&  status == zip_ok )
    {
    switch ( c = NextChar() )
      {
      case  'A':	/* Annotation */
	DEBUG(Attribute A);
	status = Extract_Attribute( self, &attribute_ptr
		 /*=== &figure->zip_image_annotation===*/ );
	break;
      case  'C':	/* Color */
	DEBUG(Attribute C);
	c = NextChar();
        red = Parse_Stream_Real(); NextChar();
        green = Parse_Stream_Real(); NextChar();
        blue = Parse_Stream_Real();
	switch ( c )
        {
	    case 'L': (self)->Set_Image_Line_Color(  Image, red, green, blue ); break;
       	    case 'F': (self)->Set_Image_FillFG_Color(  Image, red, green, blue ); break;
       	    case 'B': (self)->Set_Image_FillBG_Color(  Image, red, green, blue ); break;
	}
        break;
      case  'P':	/* Pattern */
	DEBUG(Attribute P);
	Image->zip_image_mode.zip_image_mode_patterned = on;
	Image->zip_image_fill.zip_image_pattern = NextChar();
        break;
      case  'G':	/* Grey-level Shade */
	DEBUG(Attribute G);
	Image->zip_image_mode.zip_image_mode_shaded = on;
        while ( (c = NextChar())  &&  c >= '0'  &&  c <= '9' )
	  Image->zip_image_fill.zip_image_shade =
	     Image->zip_image_fill.zip_image_shade * 10 + (c - '0');
	PriorChar( c );
        break;
      case  'F':	/* Font */
	DEBUG(Attribute F)
	if ( (status = Extract_Attribute( self, &attribute_ptr )) == zip_ok )
	  {
	  (self)->Define_Font(  attribute_ptr, &Image->zip_image_font );
	  free( attribute_ptr );
	  }
        break;
      case  'T':	/* Text */
	DEBUG(Attribute T);
	status = Extract_Attribute( self, &Image->zip_image_text );
        break;
      case  'N':	/* Name */
	DEBUG(Attribute N);
  	status = Extract_Attribute( self, &Image->zip_image_name );
	if ( strncmp( Image->zip_image_name, "ZIP_PAGE_IMAGE_", 15 ) == 0 )
	  {
	  i = atoi( Image->zip_image_name + 15 );
	  DEBUGdt(Page,i);
	  if ( i > self->page_count )
	    self->page_count = i;
	  }
        break;
      case  'Z':	/* Zoom-level */
	DEBUG(Attribute Z);
	Image->zip_image_zoom_level = Parse_Stream_Integer();
        break;
      case  'D':	/* Detail-level */
	DEBUG(Attribute D);
	Image->zip_image_detail_level = Parse_Stream_Integer();
        break;
      case  'S':	/* Style */
	DEBUG(Attribute S);
	Image->zip_image_style = Parse_Stream_Integer();
        break;
      case  'L':	/* Line ... */
	DEBUG(Attribute L);
	c = NextChar();
	switch ( c )
        {
	    case 'W': Image->zip_image_line_width = Parse_Stream_Integer(); break;
	    case 'C': if (( c = NextChar()) == 'N' ) cap = graphic_CapNotLast;
		else if ( c == 'B' ) cap = graphic_CapButt;
		else if ( c == 'R' ) cap = graphic_CapRound;
		else if ( c == 'P' ) cap = graphic_CapProjecting;
		else { cap = graphic_CapButt; printf( "zip: unrecognized line cap attribute: '%c'\n", c ); }
		(self)->Set_Image_Line_Cap(  Image, cap ); break;
	    case 'J': if (( c = NextChar()) == 'M' ) cap = graphic_JoinMiter;
		else if ( c == 'R' ) cap = graphic_JoinRound;
		else if ( c == 'B' ) cap = graphic_JoinBevel;
		else { cap = graphic_JoinMiter; printf( "zip: unrecognized line join attribute: '%c'\n", c ); }
		(self)->Set_Image_Line_Join(  Image, cap ); break;
	    case 'D': if (( c = NextChar()) == 'S' ) cap = graphic_LineSolid;
	        else if ( c == 'D' ) cap = graphic_LineDoubleDash;
		else if ( c == 'O' ) cap = graphic_LineOnOffDash;
		NextChar(); offset = Parse_Stream_Integer(); NextChar(); i = 0;
		while( element != 0 )
		{
		    pattern[i++] = element = ( char ) Parse_Stream_Integer();
		    if ( element ) NextChar();
		}
		pattern[i] = 0;
		(self)->Set_Image_Line_Dash(  Image, pattern, offset, cap );
		break;
	}
        break;
      default:
	DEBUG(Attribute Default);
	if ( ! Parse_Stream_Commentary( self, c ) )
	  {
	  c = PriorChar( c );
	  if ( c != '*'  &&  c != '>'  &&
	       c != '{'  &&  c != '}'  &&
	       c != '\0' )
	    {
	    status = zip_unrecognized_stream_object_attribute;
	    sprintf( msg, "Unrecognized Image Attribute '%c' At Position %d\n",
		     c, position );
/*===	    while ( apt_Acknowledge( msg ) == -1 );===*/
	    }
	  end_of_attributes = true;
	  }
	break;
      }
    }
  DEBUGdt(Status,status);
  OUT(Parse_Image_Attributes);
  return status;
  }

static int
Extract_Attribute( class zip		      *self, char			     **attribute_ptr )
      {
  int			      status = zip_ok;
  char			     *counter, c;
  char				      buffer[zip_default_buffer_size + 1];

  IN(Extract_Attribute);
  *attribute_ptr = NULL;
  counter = buffer;
  if ( (c = NextChar()) != '\n' )
    {
  *counter++ = c;
  while ( c  &&  counter - buffer < zip_default_buffer_size )
    {
    if ( (c = NextChar()) == '\n' )
      break;
      else
      *counter++ = c;
    }
  *counter = 0;
  if ( counter == buffer + zip_default_buffer_size )
    status = zip_insufficient_figure_space;
  else if ( *attribute_ptr = (char *) malloc( strlen( buffer ) + 1 ) )
    {
    counter = buffer;
    while ( *counter )
      { /*+++ Two statements because of HC bug on "++" +++*/
      *( *attribute_ptr + ( counter - buffer) ) = *counter; counter++;
      }
    *( *attribute_ptr + ( counter - buffer ) ) = 0;
    DEBUGst( Extracted Attribute,*attribute_ptr);
    }
    else status = zip_insufficient_figure_space;
    }
  OUT(Extract_Attribute);
  return status;
  }

static char *
Unique_Name( char			      *name, int			       seed )
      {
  static char			      alternate[512];

  sprintf( alternate, "%s[%d]", name, seed );
  return alternate;
  }

static int
Parse_Stream_Integer()
  {
  long			      number = 0;
  char			      c;

  while ( (c = NextChar())  &&  c >= '0'  &&  c <= '9' )
    number = number * 10 + (c - '0');
  PriorChar( c );
  return  number;
  }

static double
Parse_Stream_Real()
  {
  double		number = 0.0;
  char				value[10];
  char			c, *p = value;

  while ( (c = NextChar())  && (( c >= '0'  &&  c <= '9' ) || c == '.' ))
        *p++ = c;
  *p = '\0';
  number = atof( value );
  PriorChar( c );
  return  number;
  }

static
int Parse_Stream_Commentary( class zip		      *self, char			       c )
      {
  int			      status = false;

  IN(Parse_Stream_Commentary);
  if ( c == '#'  ||  c == ' '  ||  c == '\t'  ||  c == '\n' )
    {
    status = true;
    while ( c  &&  c != '\n' )
      c = NextChar();
    }
  OUT(Parse_Stream_Commentary);
  return status;
  }

static
int Equivalent_Token( class zip		      *self, const char			      *token, const char			      * const table[] )
        {
  int			      result = 0;

  IN(Equivalent_Token);
  DEBUGst( Token, token );
  while ( token  &&  *token  &&  table  &&  *table )
    {
    DEBUGst( Table-item, *table );
    if ( ! apt_MM_Compare( token, *table++ ) )
      {
      result = 1;
      break;
      }
    }
  DEBUGdt( Result, result );
  OUT(Equivalent_Token);
  return  result;
  }

/*
 Setting the default print size is tricky:
 Old versions of zip Would rely on the Object
 and View parameters to set the print size.
 We must try to stay compatible with this while
 at the same time allowing a specified inch size.

 We use the multiple values of size_from_file
 to establish precedence.

 Print overrides View which overrides Object.
*/

/* All of these must be nonzero and must fit in a char */

#define ZIP_FILE_PRINT_PARAMETER 3
#define ZIP_FILE_VIEW_PARAMETER 2
#define ZIP_FILE_OBJECT_PARAMETER 1

static
long Parse_Presentation_Parameter( class zip		      *self )
    {
  long			      status = zip_ok;
  char				     *token, *ptr;
  static const char		     * const vw[] =
    { "ViewWidth", "vw", NULL };
  static const char		     * const vh[] =
    { "ViewHeight", "vh", NULL };
  static const char		     * const ow[] =
    { "ObjectWidth", "ow", NULL };
  static const char		     * const oh[] =
    { "ObjectHeight", "oh", NULL };
  static const char		     * const ps[] =
    { "PrintSize", "ps", NULL };

  IN(Parse_Presentation_Parameter);
  Extract_Attribute( self, &token );
  if ( ptr = token )
    {
    while ( *ptr  &&  *ptr != ' '  &&  *ptr != '\t'  &&  *ptr != '\n' )
      ptr++;
    if ( *ptr == ' '  ||  *ptr == '\t' )
      *ptr++ = 0;
    if ( Equivalent_Token( self, token, vw ) )
      {
      self->desired_view_width = atoi( ptr );
      if (self->size_from_file <= ZIP_FILE_VIEW_PARAMETER) {
	  self->size_from_file = ZIP_FILE_VIEW_PARAMETER;
	  self->def_inch_width = (float)self->desired_view_width / 72.0;
      }
      DEBUGdt(Computed ViewWidth,self->desired_view_width);
      }
    else
    if ( Equivalent_Token( self, token, vh ) )
      {
      self->desired_view_height = atoi( ptr );
       if (self->size_from_file <= ZIP_FILE_VIEW_PARAMETER) {
	  self->size_from_file = ZIP_FILE_VIEW_PARAMETER;
	  self->def_inch_height = (float)self->desired_view_height / 72.0;
      }
     DEBUGdt(Computed ViewHeight,self->desired_view_height);
      }
    else
    if ( Equivalent_Token( self, token, ow ) )
      {
      self->object_width = atoi( ptr );
      if (self->size_from_file <= ZIP_FILE_OBJECT_PARAMETER) {
	  self->size_from_file = ZIP_FILE_OBJECT_PARAMETER;
	  self->def_inch_width = (float)self->object_width / 72.0;
      }
      DEBUGdt(Computed ObjectWidth,self->object_width);
      }
    else
    if ( Equivalent_Token( self, token, oh ) )
      {
      self->object_height = atoi( ptr );
      if (self->size_from_file <= ZIP_FILE_OBJECT_PARAMETER) {
	  self->size_from_file = ZIP_FILE_OBJECT_PARAMETER;
	  self->def_inch_height = (float)self->object_height / 72.0;
      }
      DEBUGdt(Computed ObjectHeight,self->object_height);
      }
    else
    if ( Equivalent_Token( self, token, ps ) )
      {
	sscanf(ptr, "%fx%f", &(self->def_inch_width), &(self->def_inch_height));
	self->size_from_file = ZIP_FILE_PRINT_PARAMETER;
	DEBUGgt(Stream Inch Width, self->def_inch_width);
	DEBUGgt(Stream Inch Height, self->def_inch_height);
      }
    free( token );
    }
  OUT(Parse_Presentation_Parameter);
  return  status;
  }

static char
NextChar()
  {
  int				     c;

  if ( (c = getc( stream->zip_stream_file )) == EOF )
    {
    DEBUG(EOF);
    c = 0;
    }
  DEBUGct(C,c);
  position++;
  return c;
  }

static char
PriorChar( char				      c )
    {
  c = ungetc( c, stream->zip_stream_file );
  DEBUGct(C,c);
  position--;
  return c;
  }

