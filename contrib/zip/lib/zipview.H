#ifndef _zipview_H_
#define _zipview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1988
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
/*
zipview.H

  11/09/87	Support "Absolute" as well as "Relative" sizing (TCP)
  03/31/88	Revise for ATK (TCP)
  06/14/89	Add Pane_Hit_Processor access macromethod (SCG)
   08/16/90	Add {Ensure,Normalize}_{Line,Fill}_Attributes methods
                            Add square, zipmin, zipmax macros (SCG)
*/

#define  zipview_VERSION		    2

#define  zipview_invert			    (1<<1)

#define  zipview_default_block_width	    512
#define  zipview_default_block_height	    512


#define  zipview_pane_top_edge		    (1 << 1)
#define  zipview_pane_bottom_edge	    (1 << 2)
#define  zipview_pane_left_edge		    (1 << 3)
#define  zipview_pane_right_edge	    (1 << 4)

#define  zipview_paint_inverted		    (1 << 1)
#define  zipview_paint_copy		    (1 << 2)

#define	 square(x)			    ((x)*(x))
#define	 zipmin(x,y)			    ((x) < (y) ? (x) : (y))
#define	 zipmax(x,y)			    ((x) < (y) ? (y) : (x))

struct zipview_options
  {
  unsigned int				    manual_refresh	: 1;
  };

struct zipview_states
  {
  unsigned int				    inputfocus		: 1;
  unsigned int				    editing		: 1;
  unsigned int				    grid_exposed	: 1;
  unsigned int				    coordinates_exposed	: 1;
  unsigned int				    fonts_exposed	: 1;
  unsigned int				    figures_exposed	: 1;
  unsigned int				    first_time	    	: 1;
  unsigned int				    abeyant	    	: 1;
  unsigned int				    application	    	: 1;
  };


#include <view.H>
#include "zip.h"
#include "zipfig.h"
#include "zipimage.h"
#include "zipstrm.h"
#include "zippane.h"

typedef long (*zip_panehitproc)(long anchor, zip_type_pane pane, enum view_MouseAction action, long x, long y, long clicks);
typedef long (*zip_panedisplayproc)(long anchor, zip_type_pane pane, long action);
typedef void (*zip_paneexceptproc)(struct zipview *, zip_type_pane pane);

class zipview : public  view
  {
public:
	virtual ATKregistryEntry *ATKregistry();
	static boolean InitializeClass();
	zipview();
	~zipview();



   view_DSattributes DesiredSize( long width, long height, enum view_DSpass pass, long *dWidth, long *dheight );
   void FullUpdate( enum view_UpdateType type, long left, long top, long width, long height );
   void Update();
   void WantUpdate( class view *requestor );
   class view * Hit( enum view_MouseAction action, long x, long y, long n);
   void ReceiveInputFocus();
   void LoseInputFocus();
   void SetDataObject( class dataobject *d );
   void ObservedChanged ( class observable *changed, long value );
   void Print( FILE *file, const char *processor, const char *finalFormat, boolean topLevel );
   class view * GetApplicationLayer();
   const void *GetInterface( const char *interface_name );
   void PostMenus( class menulist *menulist );
   void PostKeyState( class keystate *keystatep );



  virtual void SetOptions( int options );
  virtual void Set_Debug( boolean state );

  virtual long Display_Figure( zip_type_figure figure, zip_type_pane pane );
  virtual long Draw_Figure( zip_type_figure figure, zip_type_pane pane );
  virtual long Clear_Figure( zip_type_figure figure, zip_type_pane pane );
  virtual long Hide_Figure( zip_type_figure figure, zip_type_pane pane );
  virtual long Expose_Figure( zip_type_figure figure, zip_type_pane pane );
  virtual struct zip_figure * Which_Figure( zip_type_pixel x, zip_type_pixel y );
  virtual struct zip_figure * Which_Pane_Figure( zip_type_pixel x, zip_type_pixel y, zip_type_pane pane );
  virtual struct zip_figure * Within_Which_Figure( long x, long y );
  virtual boolean Figure_Visible( zip_type_figure figure, zip_type_pane pane );

  virtual long Display_Image( zip_type_image image, zip_type_pane pane );
  virtual long Draw_Image( zip_type_image image, zip_type_pane pane );
  virtual long Clear_Image( zip_type_image image, zip_type_pane pane );
  virtual long Hide_Image( zip_type_image image, zip_type_pane pane );
  virtual long Expose_Image( zip_type_image image, zip_type_pane pane );
  virtual struct zip_image * Which_Image( long x, long y );
  virtual boolean Image_Visible( zip_type_image image, zip_type_pane pane );

  virtual long Display_Stream( zip_type_stream stream, zip_type_pane pane );
  virtual long Draw_Stream( zip_type_stream stream, zip_type_pane pane );
  virtual long Clear_Stream( zip_type_stream stream, zip_type_pane pane );
  virtual struct zip_stream * Which_Stream( long x, long y );
  virtual boolean Stream_Visible( zip_type_stream stream, zip_type_pane pane );

  virtual struct zip_pane * Pane( char *name );
  virtual long Create_Pane( zip_type_pane *pane,  const char  *name,  struct rectangle		  *block,  long   attributes );
  virtual long Create_Nested_Pane( zip_type_pane *pane, const char *name,  zip_type_pane master_pane,  long attributes );
  virtual long Destroy_Pane( zip_type_pane pane );
  virtual long Redisplay_Panes();
  virtual long Redraw_Panes();
  virtual long Redisplay_Pane_Suite( zip_type_pane pane );
  virtual long Redraw_Pane_Suite( zip_type_pane pane );

  virtual long Set_Pane_Coordinates( zip_type_pane  pane, long x_origin, long y_origin, long width, long height );
  virtual long Set_Pane_Border( zip_type_pane  pane, char	*font_name, char pattern, long thickness );
  virtual long Set_Pane_Stream( zip_type_pane pane, zip_type_stream stream );
  virtual long Reset_Pane_Stream( zip_type_pane pane, zip_type_stream stream );
  virtual struct zip_stream * Pane_Stream( zip_type_pane pane );

  virtual long Set_Pane_Auxiliary_Stream( zip_type_pane pane, zip_type_stream stream );
  virtual long Reset_Pane_Auxiliary_Stream( zip_type_pane pane, zip_type_stream stream );
  virtual long Reset_Pane_Auxiliary_Streams( zip_type_pane pane );
  virtual long Set_Pane_Image( zip_type_pane pane, zip_type_image image );
  virtual long Reset_Pane_Image( zip_type_pane pane, zip_type_image image );
  virtual struct zip_image * Pane_Image( zip_type_pane pane );
  virtual long Set_Pane_Figure( zip_type_pane pane, zip_type_figure figure );
  virtual long Reset_Pane_Figure( zip_type_pane pane, zip_type_figure figure );
  virtual struct zip_figure * Pane_Figure( zip_type_pane pane );
  virtual long Set_Pane_Scale( zip_type_pane pane, float scale );
  virtual long Set_Pane_Cursor( zip_type_pane pane, char cursor_icon, const char *cursor_font );
  virtual long Set_Pane_Painting_Mode( zip_type_pane pane, char mode );

  virtual long Display_Pane( zip_type_pane pane );
  virtual long Draw_Pane( zip_type_pane pane );
  virtual long Clear_Pane( zip_type_pane pane );
  virtual long Zoom_Pane( zip_type_pane pane, long level );
  virtual long Zoom_Pane_To_Point( zip_type_pane pane, zip_type_point x, zip_type_point y, long level, long mode );
  virtual long Scale_Pane( zip_type_pane pane, float scale );
  virtual long Scale_Pane_To_Point( zip_type_pane pane, zip_type_point x, zip_type_point y, float scale, long mode );
  virtual long Pan_Pane( zip_type_pane pane, zip_type_pixel x_offset, zip_type_pixel y_offset );
  virtual long Pan_Pane_To_Edge( zip_type_pane pane, long edge );
  virtual long Flip_Pane( zip_type_pane pane );
  virtual long Flop_Pane( zip_type_pane pane );
  virtual long Invert_Pane( zip_type_pane pane );
  virtual long Balance_Pane( zip_type_pane pane );
  virtual long Center_Pane( zip_type_pane pane );
  virtual long Normalize_Pane( zip_type_pane pane );
  virtual long Hide_Pane( zip_type_pane pane );
  virtual long Expose_Pane( zip_type_pane pane );
  virtual long Remove_Pane( zip_type_pane pane );
  virtual struct zip_pane * Which_Pane( long x, long y );

  virtual long X_Pixel_To_Point( zip_type_pane pane, zip_type_figure figure, long x );
  virtual long Y_Pixel_To_Point( zip_type_pane pane, zip_type_figure figure, long y );
  virtual long X_Point_To_Pixel( zip_type_pane pane, zip_type_figure figure, long x );
  virtual long Y_Point_To_Pixel( zip_type_pane pane, zip_type_figure figure, long y );
  virtual long X_Point_Delta( zip_type_pane pane, long x_delta );
  virtual long Y_Point_Delta( zip_type_pane pane, long y_delta );

  virtual long Print_Figure( zip_type_figure figure, zip_type_pane pane );
  virtual long Print_Image( zip_type_image image, zip_type_pane pane );
  virtual long Print_Stream( zip_type_stream stream, zip_type_pane pane );
  virtual long Print_Pane( zip_type_pane pane );
  virtual long Set_Print_Resolution( long resolution );
  virtual long Set_Print_Dimensions( float width, float height );
  virtual long Set_Print_Coordinates( zip_type_percent x_origin, zip_type_percent y_origin, zip_type_percent width, zip_type_percent height );
  virtual long Set_Print_Orientation( long orientation );
  virtual long Set_Print_Language( const char *language );
  virtual long Set_Print_Processor( const char *processor );
  virtual long Set_Print_Level( long level );
  virtual long Set_Print_File( FILE *file );

  virtual long Use_Normal_Pane_Cursors();
  virtual long Use_Working_Pane_Cursors();
  virtual long Use_Alternate_Pane_Cursors( char cursor_icon, char *cursor_font );

  virtual long Handle_Panning( zip_type_pane pane, long x, long y, long x_delta, long y_delta );
  virtual long Initiate_Panning( zip_type_pane pane, long x, long y, long mode );
  virtual long Continue_Panning( zip_type_pane pane, long x, long y );
  virtual long Terminate_Panning( zip_type_pane pane, long x, long y, long *x_delta, long *y_delta, long draw );

  virtual class fontdesc * Contextual_Figure_Font( zip_type_figure figure );
  virtual class fontdesc * Select_Contextual_Figure_Font( zip_type_figure figure );

  /****  Following Facilities For Sub-Class (Internal) Usage Only  ****/

  virtual class graphic * Define_Graphic( class fontdesc *font, unsigned char pattern );
  virtual void Set_Clip_Area( zip_type_pane pane, long left, long top, long width, long height );
  virtual void Set_Pane_Clip_Area( zip_type_pane pane );
  virtual void Reset_Pane_Clip_Area( zip_type_pane pane );
  virtual void Post_Pane_Cursor( zip_type_pane pane, class cursor *glyph );
  virtual boolean Condition( zip_type_pane pane, zip_type_figure figure, int action );
  virtual boolean Proximate_Figure_Point( zip_type_pane pane, zip_type_figure figure, zip_type_pixel X, zip_type_pixel Y, zip_type_pixel x, zip_type_pixel y );
  virtual long Proximate_Enclosure( zip_type_pane pane, zip_type_pixel left, zip_type_pixel top, zip_type_pixel right, zip_type_pixel bottom, zip_type_pixel x, zip_type_pixel y );
  virtual long Try_Pane_Exception_Handler( zip_type_pane pane );
  virtual long Query( const char *query, const char *default_response, char **response );
  virtual long Query_File_Name( const char *query, char **response );
  virtual long AskOrCancel( const char *query, const char *default_resopnse, char **response );
  virtual long Announce( const char *message );
  virtual void Ensure_Fill_Attributes( zip_type_figure figure );
  virtual long Ensure_Line_Attributes( zip_type_figure figure );
  virtual void Normalize_Fill_Attributes();
  virtual void Normalize_Line_Attributes();

/* These not used */

  virtual long Hide_Stream( zip_type_stream stream, zip_type_pane  pane );
  virtual long Expose_Stream( zip_type_stream stream, zip_type_pane pane );
  virtual zip_type_stream Within_Which_Stream( long x , long	y );


  inline boolean Figure_Exposed(zip_type_figure figure)	{ return ((figure)->zip_figure_visibility == zip_figure_exposed); }
  inline boolean Figure_Hidden(zip_type_figure figure)		{ return ((figure)->zip_figure_visibility == zip_figure_hidden); }

  inline boolean Image_Exposed( zip_type_image image )	{ return ((image)->zip_image_visibility == zip_image_exposed); }
  inline boolean Image_Hidden( zip_type_image image )		{ return ((image)->zip_image_visibility == zip_image_hidden); }
  
  inline char *Pane_Name(zip_type_pane pane)		{ return ((pane)->zip_pane_name); }
  inline unsigned char Pane_Exposed(zip_type_pane pane)		{ return ((pane)->zip_pane_state.zip_pane_state_exposed); }
  inline void Set_Pane_Exposed(zip_type_pane pane, boolean exposed)		{ (pane)->zip_pane_state.zip_pane_state_exposed = (exposed); }
  inline unsigned char Pane_Hidden(zip_type_pane pane)		{ return ((pane)->zip_pane_state.zip_pane_state_hidden); }
  inline unsigned char Pane_Removed(zip_type_pane pane)		{ return ((pane)->zip_pane_state.zip_pane_state_removed); }
  inline unsigned char Pane_Inverted(zip_type_pane pane)		{ return ((pane)->zip_pane_state.zip_pane_state_inverted); }
  inline void Set_Pane_Inverted(zip_type_pane pane, boolean inverted)		{ ((pane)->zip_pane_state.zip_pane_state_inverted) = (inverted); }
  inline unsigned char Pane_Coordinates_Exposed(zip_type_pane pane) { return ((pane)->zip_pane_state.zip_pane_state_coordinates_exposed); }
  inline unsigned int Pane_Overlaying(zip_type_pane pane)		{ return ((pane)->zip_pane_attributes.zip_pane_attribute_overlay); }
  inline unsigned int Pane_Transparent(zip_type_pane pane)	{ return ((pane)->zip_pane_attributes.zip_pane_attribute_transparent); }

  inline zip_type_percent Pane_X_Percent_Origin(zip_type_pane pane)	{ return ((zip_type_percent)(pane)->zip_pane_x_origin_percent); }
  inline zip_type_percent Pane_Y_Percent_Origin(zip_type_pane pane)	{ return ((zip_type_percent)(pane)->zip_pane_y_origin_percent); }
  inline zip_type_percent Pane_Percent_Width(zip_type_pane pane)	{ return ((zip_type_percent)(pane)->zip_pane_width_percent); }
  inline zip_type_percent Pane_Percent_Height(zip_type_pane pane)	{ return ((zip_type_percent)(pane)->zip_pane_height_percent); }
  inline zip_type_percent Pane_X_Origin(zip_type_pane pane)		{ return ((zip_type_pixel)(pane)->zip_pane_x_origin); }
  inline zip_type_percent Pane_Y_Origin(zip_type_pane pane)		{ return ((zip_type_pixel)(pane)->zip_pane_y_origin); }
  inline zip_type_pixel Pane_Left(zip_type_pane pane)		{ return ((zip_type_pixel)(pane)->zip_pane_left); }
  inline zip_type_pixel Pane_Right(zip_type_pane pane)		{ return ((zip_type_pixel)(pane)->zip_pane_left + (pane)->zip_pane_width); }
  inline zip_type_pixel Pane_Top(zip_type_pane pane)		{ return ((zip_type_pixel)(pane)->zip_pane_top); }
  inline zip_type_pixel Pane_Bottom(zip_type_pane pane)		{ return ((zip_type_pixel)(pane)->zip_pane_top + (pane)->zip_pane_height); }
  inline zip_type_pixel Pane_Width(zip_type_pane pane)		{ return ((zip_type_pixel)(pane)->zip_pane_width); }
  inline zip_type_pixel Pane_Height(zip_type_pane pane)		{ return ((zip_type_pixel)(pane)->zip_pane_height); }
  inline long Pane_Object_Width(zip_type_pane pane)	{ return ((zip_type_pixel)(pane)->zip_pane_object_width); }
  inline long Pane_Object_Height(zip_type_pane pane)	{ return ((zip_type_pixel)(pane)->zip_pane_object_height); }
  inline float Pane_Scale(zip_type_pane pane)		{ return ((pane)->zip_pane_scale); }

  inline char *Pane_Datum(zip_type_pane pane)		{ return ((pane)->zip_pane_client_data); }

  inline long Set_Pane_Object_Width( zip_type_pane pane, long width ) 
  { return (pane)->zip_pane_object_width = width;}
  inline long Set_Pane_Object_Height( zip_type_pane pane, long height ) 
  { return (pane)->zip_pane_object_height = height;}

  inline short Set_Pane_Panning_Precision( zip_type_pane pane, short precision ) 
  { return (pane)->zip_pane_panning_precision = precision;}
  inline short Set_Pane_Zoom_Level( zip_type_pane pane, short level ) 
  { return (pane)->zip_pane_zoom_level = level;}
  inline short Pane_Zoom_Level( zip_type_pane pane ) 
  { return (pane)->zip_pane_zoom_level; }
  inline short Set_Pane_Detail_Level( zip_type_pane pane, short level ) 
  { return (pane)->zip_pane_detail_level = level;}
  inline short Pane_Detail_Level( zip_type_pane pane ) 
  { return (pane)->zip_pane_detail_level; }

  inline void Set_Pane_Hit_Processor( zip_type_pane pane, zip_panehitproc processor, long anchor ) 
    {(pane)->zip_pane_hit_processor = 
    processor;(pane)->zip_pane_hit_processor_anchor = (long)anchor;}
  inline zip_panehitproc Pane_Hit_Processor( zip_type_pane pane ) 
  { return (pane)->zip_pane_hit_processor; }
  inline void Reset_Pane_Hit_Processor( zip_type_pane pane ) 
    {(pane)->zip_pane_hit_processor = NULL;}

  inline void Set_Pane_Display_Preprocessor( zip_type_pane pane, zip_panedisplayproc processor, long anchor ) 
    {(pane)->zip_pane_display_preprocessor = 
    processor;(pane)->zip_pane_display_processor_anchor = anchor;}
  inline void Reset_Pane_Display_Preprocessor( zip_type_pane pane ) 
    {(pane)->zip_pane_display_preprocessor = NULL;}

  inline void Set_Pane_Display_Processor( zip_type_pane pane, zip_panedisplayproc processor, long anchor ) 
    {(pane)->zip_pane_display_processor = 
    processor;(pane)->zip_pane_display_processor_anchor = anchor;}
  inline void Reset_Pane_Display_Processor( zip_type_pane pane ) 
    {(pane)->zip_pane_display_processor = NULL;}

  inline void Set_Pane_Display_Postprocessor( zip_type_pane pane, zip_panedisplayproc processor, long anchor ) 
    {(pane)->zip_pane_display_postprocessor = 
    processor;(pane)->zip_pane_display_processor_anchor = anchor;}
  inline void Reset_Pane_Display_Postreprocessor( zip_type_pane pane ) 
    {(pane)->zip_pane_display_postprocessor = NULL;}

  inline void Set_Pane_Exception_Handler( zip_paneexceptproc handler ) 
    {(this)->pane_exception_handler = handler;}
  inline void Reset_Pane_Exception_Handler() 
    {(this)->pane_exception_handler = NULL;}

  class zip			*data_object;
  class zipedit			*edit_object;
  class zipprint		*print_object;
  class zipobject	        **objects;
  struct zip_pane_chain	*pane_anchor;
  struct zip_printing		*printing; 
  class menulist		*menu;
  class keymap			*keymapp;
  class keystate		*keystatep;
  class cursor			*cursor;
  struct zipview_options	 options;
  struct zipview_states	 states;
  zip_type_pane			 pane, current_pane;
  struct rectangle		 block;
  long				 action;
  enum view_MouseAction		 mouse_action;
  long				 mouse_x,
				 mouse_y,
				 mouse_clicks;
  long				 edge;
  long				 width;
  long				 height;
  zip_paneexceptproc		 pane_exception_handler;
  class fontdesc		*points_font,
				*icon_font,
				*dot_font,
				*shade_font;
  long				 current_page;
  long				 panning_precision;
};

#endif /* _zipview_H_ */
