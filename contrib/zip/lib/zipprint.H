#ifndef _zipprint_H_
#define _zipprint_H_
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
zipprint.H

  03/31/88	Created for ATK (TCP)
   08/20/90	Add Ensure_Line_Attributes method (SCG)
*/

#define  zipprint_VERSION		    1

#include "zip.h"
#include "zipfig.h"
#include "zipimage.h"
#include "zipstrm.h"
#include "zippane.h"
#include "zipprint.h"

struct zipprint_options
  {
  unsigned int				    invert		: 1;
  };

struct zipprint_states
  {
  unsigned int				    xxxx		: 1;
  unsigned int				    user_def_size	: 1;
  };


#include <ATK.H>
class zipprint : public ATK 
  {
public:
	virtual ATKregistryEntry *ATKregistry();





  virtual void Set_Debug( boolean state );
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
  virtual long Set_Print_Size( float width, float height );
  virtual void Set_Default_Print_Size( class zip *data );
  virtual void Set_Data_Object(  class zip *data_object );
  virtual void Set_View_Object( class zipview *view_object );
  virtual void Set_Line_Width( long line_width );
  virtual long Ensure_Line_Attributes( zip_type_figure figure );
  virtual void Set_Shade( long shade );
  virtual void Move_To( long x, long y );
  virtual void Draw_To( long x, long y );
  virtual void Draw_Multi_Line( int count, int x, int y, zip_type_point_pairs points );
  virtual void Draw_String( int x, int y, char *string, long mode );
  virtual void Draw_Line( int x1, int y1, int x2, int y2 );
  virtual void Draw_Rectangle( long x1, long y1, long x2, long y2 );
  virtual void Draw_Round_Rectangle( long  x1, long y1, long x2, long y2, long xr, long yr );
  virtual void Draw_Circle( int x, int y, int radius );
  virtual void Draw_Ellipse( int x, int y, int x_radius, int y_radius );
  virtual void Draw_Arc( long x, long y, long x_radius, long y_radius, long x_start, long y_start, long x_end, long y_end );
  virtual void Arc_To( int x_center, int y_center, int x_radius, int y_radius, int x_start, int y_start, int x_end, int y_end );
  virtual void Fill_Trapezoid( int x1, int y1, int x2, int y2, int l1, int l2, char pattern );

  virtual void Close_Path();
  virtual void Change_Font( class fontdesc *font );
  virtual void Restore_Font();
  virtual long Try_Printing_Exception_Handler( zip_type_printing printing );





  
	zipprint();
  
	virtual ~zipprint();




  class zip			*data_object;
  class zipview			*view_object;
  struct zipprint_options	 options;
  struct zipprint_states	 states;
  };

#endif /* _zipprint_H_ */
