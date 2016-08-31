#ifndef _zip_H_
#define _zip_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/

/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1988
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
/*
zip.ch

  11/09/87	Support "Absolute" as well as "Relative" sizing (TCP)
  03/31/88	Revise for ATK (TCP)
  11/17/88	Add Line_Width methods (TCP/SCG)
  05/10/89	Have Contextual_Figure_Line_Width return "unsigned char" (SCG)
  08/07/89	Override GetModified to check for changes to Imbedded objects (TCP)
   08/14/90	Added numerous color and line style method declarations (SCG)
*/

#define  zip_VERSION    2

#include "zip.h"
#include "zipfig.h"
#include "zipimage.h"
#include "zipstrm.h"
#include "zippane.h"
#include "zipefc00.h"
#include "zipefn00.h"
#include "zipiff00.h"
#include "zipifm00.h"

typedef long (*zip_figureexceptfptr)(class zip *zip, zip_type_figure figure);
typedef long (*zip_generalexceptfptr) (class zip *zip, char *facility, long status);
typedef long (*zip_imageexceptfptr) (class zip *zip, zip_type_image image);
typedef long (*zip_streamexceptfptr) (class zip *zip, zip_type_stream stream);

#include <dataobject.H>
class zip : public  dataobject {
public:
	virtual ATKregistryEntry *ATKregistry();
	static boolean InitializeClass();

	zip();
	virtual ~zip();

   long Read( FILE *file, long id );
   long Write( FILE *file, long id, int level );
   long GetModified();



  virtual void Set_Debug( boolean state );
  virtual void Show_Statistics( int options );

  virtual long Create_Figure( zip_type_figure *figure, const char *name, unsigned char type, zip_type_image image, zip_type_figure peer );
  virtual long Destroy_Figure( zip_type_figure figure );
  virtual long Hook_Figure( zip_type_figure figure, zip_type_figure peer_figure );
  virtual long Unhook_Figure( zip_type_figure figure );
  virtual long Set_Figure_Name( zip_type_figure figure, const char *name );
  virtual long Set_Figure_Text( zip_type_figure figure, const char *text );
  virtual long Set_Figure_Pattern( zip_type_figure figure, char pattern );
  virtual long Set_Figure_Shade( zip_type_figure figure, unsigned char shade );
  virtual long Set_Figure_Line_Width( zip_type_figure figure, short width );
  virtual long Set_Figure_Line_Dash( zip_type_figure figure, const char *pattern, int offset, short type );
  virtual long Set_Figure_Line_Cap( zip_type_figure figure, short cap );
  virtual long Set_Figure_Line_Join( zip_type_figure figure, short join );
  virtual long Set_Figure_Line_Color( zip_type_figure figure, double red, double green, double blue );
  virtual long Set_Figure_FillFG_Color( zip_type_figure figure, double red, double green, double blue );
  virtual long Set_Figure_FillBG_Color( zip_type_figure figure, double red, double green, double blue );
  virtual long Set_Figure_Font( zip_type_figure figure, const char *font_name );
  virtual long Set_Figure_Mode( zip_type_figure figure, long mode );
  virtual long Set_Figure_Point( zip_type_figure figure, int point, long x, long y );
  virtual long Adjust_Figure_Point_Suite( zip_type_figure figure, long x_delta, long y_delta );
  virtual long Change_Figure_Point( zip_type_figure figure, long old_x, long old_y, long new_x, long new_y );
  virtual long Remove_Figure_Point( zip_type_figure figure, long old_x, long old_y );
  virtual long Add_Figure_Point( zip_type_figure figure, long new_x, long new_y );
  virtual struct zip_figure * Figure( const char *name );
  virtual struct zip_figure * Image_Figure( struct zip_image *image, const char *name );
  virtual struct zip_figure * Stream_Figure( struct zip_stream *stream, const char *name );

  virtual long Create_Peer_Image( struct zip_image **image, const char *name, struct zip_stream	      *stream, struct zip_image *peer );
  virtual long Create_Inferior_Image( struct zip_image **image, const char *name, struct zip_stream	      *stream, struct zip_image *superior );
  virtual long Destroy_Image(  struct zip_image	*image );
  virtual long Hook_Peer_Image( struct zip_image *image, struct zip_image *peer_image );
  virtual long Hook_Inferior_Image( struct zip_image *image, struct zip_image *superior_image );
  virtual long Unhook_Image( struct zip_image *image );
  virtual struct zip_image * Image_Left_Peer( zip_type_image image );
  virtual long Set_Image_Name( struct zip_image *image, const char *name );
  virtual long Set_Image_Text( struct zip_image *image, const char *text );
  virtual long Set_Image_Pattern( struct zip_image *image, unsigned char pattern );
  virtual long Set_Image_Shade( struct zip_image *image, unsigned char shade );
  virtual long Set_Image_Line_Width( struct zip_image *image, long width );
  virtual long Set_Image_Line_Dash( struct zip_image *image, char *pattern, int offset, short type );
  virtual long Set_Image_Line_Cap( struct zip_image *image, short cap );
  virtual long Set_Image_Line_Join( struct zip_image *image, short join );
  virtual long Set_Image_Line_Color( struct zip_image *image, double red, double green, double blue );
  virtual long Set_Image_FillFG_Color( struct zip_image *image, double red, double green, double blue );
  virtual long Set_Image_FillBG_Color( struct zip_image *image, double red, double green, double blue );
  virtual long Set_Image_Font( struct zip_image *image, char * font_name );
  virtual char Superior_Image_Pattern( struct zip_image *image );
  virtual char Superior_Image_Shade( struct zip_image *image );
  virtual unsigned char Superior_Image_Line_Width( struct zip_image *image );
  virtual long Superior_Image_Line_Dash( struct zip_image *image, char **pattern, int *offset, short *type );
  virtual short Superior_Image_Line_Cap( struct zip_image *image );
  virtual short Superior_Image_Line_Join( struct zip_image *image );
  virtual struct zip_color * Superior_Image_Line_Color( struct zip_image *image );
  virtual struct zip_color * Superior_Image_FillFG_Color( struct zip_image *image );
  virtual struct zip_color * Superior_Image_FillBG_Color( struct zip_image *image );
  virtual char * Superior_Image_Text( struct zip_image *image );
  virtual class fontdesc * Superior_Image_Font( struct zip_image *image );
  virtual long Adjust_Image_Point_Suite( struct zip_image *image, long x_delta, long y_delta );
  virtual struct zip_image * Image( const char *name );
  virtual struct zip_image * Stream_Image( struct zip_stream *stream, const char *image_name );
  virtual struct zip_image * Next_Image( struct zip_image *image );

  virtual long Open_Stream( struct zip_stream **stream, char *name, long mode );
  virtual long Close_Stream( struct zip_stream *stream );
  virtual long Read_Stream( struct zip_stream *stream );
  virtual long Write_Stream( struct zip_stream *stream );
  virtual long Create_Stream( struct zip_stream **stream, char *name, unsigned char mode );
  virtual long Set_Stream_Name( struct zip_stream *stream, char *name );
  virtual long Set_Stream_Pattern( struct zip_stream *stream, unsigned char pattern );
  virtual long Set_Stream_Line_Width( struct zip_stream *stream, long width );
  virtual long Set_Stream_Line_Dash( struct zip_stream *stream, char *pattern, int offset, short type );
  virtual long Set_Stream_Line_Cap( struct zip_stream *stream, short cap );
  virtual long Set_Stream_Line_Join( struct zip_stream *stream, short join );
  virtual long Set_Stream_Line_Color( struct zip_stream *stream, double red, double green, double blue );
  virtual long Set_Stream_FillFG_Color( struct zip_stream *stream, double red, double green, double blue );
  virtual long Set_Stream_FillBG_Color( struct zip_stream *stream, double red, double green, double blue );
  virtual long Set_Stream_Text( struct zip_stream *stream, char *text );
  virtual long Set_Stream_Font( struct zip_stream *stream, const char *font_name );
  virtual long Set_Stream_Source( struct zip_stream *stream, const char *name );
  virtual struct zip_stream * Stream( char *name );
  virtual long Destroy_Stream( zip_type_stream stream );
  virtual char Contextual_Figure_Pattern( zip_type_figure figure );
  virtual char Contextual_Figure_Shade( zip_type_figure figure );
  virtual unsigned char Contextual_Figure_Line_Width( zip_type_figure figure );
  virtual void Contextual_Figure_Line_Dash( zip_type_figure figure, char **pattern, int *offset, short *type );
  virtual short Contextual_Figure_Line_Join( zip_type_figure figure );
  virtual short Contextual_Figure_Line_Cap( zip_type_figure figure );
  virtual long Contextual_Figure_Line_Color( zip_type_figure figure, double *red, double *green, double *blue );
  virtual long Contextual_Figure_FillFG_Color( zip_type_figure figure, double *red, double *green, double *blue );
  virtual long Contextual_Figure_FillBG_Color( zip_type_figure figure, double *red, double *green, double *blue );


  /****  Following Facilities For Sub-Class (Internal) Usage Only  ****/

  virtual struct zip_color_values * Allocate_Color_Values();
  virtual struct zip_color * Allocate_Color();
  virtual fontdesc *Define_Font( const char *font_name, short *font_index );
  virtual long Read_Figure( zip_type_figure figure );
  virtual long Write_Figure( zip_type_figure figure );
  virtual long Parse_Figure_Point( zip_type_figure figure, zip_type_point *x, zip_type_point *y );
  virtual long Parse_Figure_Points( zip_type_figure figure );
  virtual long Parse_Figure_Attributes( zip_type_figure figure );
  virtual int Allocate_Figure_Points_Vector( zip_type_point_pairs *anchor );
  virtual int Enlarge_Figure_Points_Vector( zip_type_point_pairs *anchor );
  virtual int Set_Image_Extrema( zip_type_image image, long x, long y );
  virtual long Set_Stream_Extrema( struct zip_stream *stream, zip_type_image image );
  virtual int Try_general_Exception_Handler();
  virtual int Try_Figure_Exception_Handler( zip_type_figure figure );
  virtual int Try_Image_Exception_Handler( struct zip_image *image );
  virtual int Try_Stream_Exception_Handler( struct zip_stream *stream );


  inline char *Figure_Name( zip_type_figure figure )		    { return ((figure) ? (figure)->zip_figure_name : NULL ); }
  inline unsigned char Figure_Type( zip_type_figure figure )		    { return ((figure) ? (figure)->zip_figure_type : 0 ); }
  inline char *Figure_Text( zip_type_figure figure )		    { return ((figure) ? (figure)->zip_figure_datum.zip_figure_text : NULL ); }
  inline char *Figure_Anchor( zip_type_figure figure )	    { return ((figure) ? (figure)->zip_figure_datum.zip_figure_anchor : NULL ); }
  inline unsigned char Figure_Pattern( zip_type_figure figure )	    { return ((figure) ? (figure)->zip_figure_fill.zip_figure_pattern : 0 ); }
  inline unsigned char Figure_Shade( zip_type_figure figure )	    { return ((figure) ? (figure)->zip_figure_fill.zip_figure_shade : 0 ); }
  inline struct zip_image *Figure_Image( zip_type_figure figure )	    { return ((figure) ? (figure)->zip_figure_image : NULL ); }
  inline unsigned char Figure_Zoom_Level( zip_type_figure figure )	    { return ((figure) ? (figure)->zip_figure_zoom_level : 0 ); }
  inline unsigned char Figure_Detail_Level( zip_type_figure figure )	    { return ((figure) ? (figure)->zip_figure_detail_level : 0 ); }
  inline zip_type_figure Next_Figure( zip_type_figure figure )		    { return ((figure) ? (figure)->zip_figure_next : NULL ); }
  inline zip_type_figure Figure_Root( struct zip_image *image )		    { return ((image) ? (image)->zip_image_figure_anchor : NULL ); }

  inline zip_type_image Image_Root( struct zip_stream *stream )		    { return ((stream) ? stream->zip_stream_image_anchor : NULL ); }
  inline char *Image_Name( struct zip_image *image )		    { return ((image) ? (image)->zip_image_name : NULL ); }
  inline char *Image_Text( struct zip_image *image )		    { return ((image) ? (image)->zip_image_text : NULL ); }
  inline unsigned char Image_Type( struct zip_image *image )		    { return ((image) ? (image)->zip_image_type : 0 ); }
  inline unsigned char Image_Pattern( struct zip_image *image )	    { return ((image) ? (image)->zip_image_fill.zip_image_pattern : 0 ); }
  inline unsigned char Image_Shade( struct zip_image *image )		    { return ((image) ? (image)->zip_image_fill.zip_image_shade : 0 ); }
  inline char *Image_Datum( struct zip_image *image )		    { return ((image) ? (image)->zip_image_client_data : NULL ); }
  inline unsigned char Image_Zoom_Level( struct zip_image *image )	    { return ((image) ? (image)->zip_image_zoom_level : 0 ); }
  inline unsigned char Image_Detail_Level( struct zip_image *image )	    { return ((image) ? (image)->zip_image_detail_level : 0 ); }
  inline zip_type_image Image_Superior( struct zip_image *image )	    { return ((image) ? (image)->zip_image_superior : NULL ); }
  inline zip_type_image Image_Inferior( struct zip_image *image )	    { return ((image) ? (image)->zip_image_inferior : NULL ); }
  inline zip_type_point Image_Least_X( struct zip_image *image )	    { return ((image) ? (image)->zip_image_least_x : 0 ); }
  inline zip_type_point Image_Greatest_X( struct zip_image *image )	    { return ((image) ? (image)->zip_image_greatest_x : 0 ); }
  inline zip_type_point Image_Least_Y( struct zip_image *image )	    { return ((image) ? (image)->zip_image_least_y : 0 ); }
  inline zip_type_point Image_Greatest_Y( struct zip_image *image )	    { return ((image) ? (image)->zip_image_greatest_y : 0 ); }

  inline char *Stream_Name( struct zip_stream *stream )		    { return ((stream) ? (stream)->zip_stream_name : NULL ); }

  inline struct zip_stream *Containing_Figure_Stream( zip_type_figure figure ) { return ((figure) ? (figure->zip_figure_image->zip_image_stream) : NULL); }
  inline struct zip_image *Containing_Figure_Image( zip_type_figure figure ) { return ((figure) ? (figure->zip_figure_image) : NULL); }

  inline struct zip_stream *Containing_Image_Stream( struct zip_image *image ) { return ((image) ? (image->zip_image_stream) : NULL); }
  inline struct zip_image *Containing_Image_Image( struct zip_image *image ) { return ((image) ? (image->zip_image_superior) : NULL); }

  inline zip_generalexceptfptr Set_general_Exception_Handler( zip_generalexceptfptr handler ) { return this->general_exception_handler = handler; }


  zip_type_stream		 stream;
  char				*stream_file_name;
  FILE				*write_stream_file;
  long				 write_stream_id;
  long				 write_stream_level;
  long				 id;
  long				 desired_view_width;
  long				 desired_view_height;
  char				 desired_view_metric;
  long				 object_width;
  long				 object_height;
  char				 object_metric;
  float				 def_inch_width;
  float				 def_inch_height;
  char				 size_from_file;
  class zipobject	         **objects;
  long				 page_count;
  struct zip_stream_chain	*stream_anchor;
  struct zip_image		*image_anchor;
  struct zip_figure		*figure_anchor;
  struct zip_paths		*paths;
  struct zip_fonts		*fonts;
  long				 status;
  long				 status_addenda;
  char				*facility;
  zip_generalexceptfptr		general_exception_handler;
  zip_streamexceptfptr		stream_exception_handler;
  zip_imageexceptfptr		image_exception_handler;
  zip_figureexceptfptr		figure_exception_handler;
  long				(*message_acknowledger)(class zip *, char *);
  long				(*message_writer)(class zip *, char *);
  long				(*message_clearer)(class zip *);
};

#endif /* _zip_H_ */
