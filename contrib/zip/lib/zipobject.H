#ifndef _zipobject_H_
#define _zipobject_H_
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
zipobject.ch

  03/31/88	Create for ATK (TCP)
  07/12/89	Added Object_Visible method (SCG)
  08/07/89	Add Object_Modified method (TCP)
   08/14/90	Add Contains stub. Remove Object_Visible stub (SCG)

*/

#define  zipobject_VERSION    1

#define  CurrentFigure			((pane)->zip_pane_current_figure)
#define  CurrentImage			((pane)->zip_pane_current_image)
#define  CurrentStream			((pane)->zip_pane_current_stream)

#include <view.H>
class zipobject : public  view {
public:
	virtual ATKregistryEntry *ATKregistry();




  virtual void Set_Debug( boolean state );
  virtual void Set_Data_Object( class zip *data_object );
  virtual void Set_View_Object( class zipview *view_object );
  virtual void Set_Edit_Object( class zipedit *edit_object );
  virtual void Set_Print_Object( class zipprint *print_object );

  virtual view_DSattributes Object_DesiredSize( long width, long height, enum view_DSpass pass, long *dWidth, long *dHeight ) ;
  virtual void Object_FullUpdate( enum view_UpdateType type, long left, long top, long width, long height );
  virtual void Object_Update();
  virtual class view * Object_Hit( zip_type_figure object, enum view_MouseAction action, long x, long y, long clicks ) ;

  virtual char Object_Icon();
  virtual char * Object_Icon_Font_Name();
  virtual char Object_Icon_Cursor();
  virtual char * Object_Icon_Cursor_Font_Name();
  virtual char Object_Datastream_Code();
  virtual long Object_Modified( zip_type_figure object );

  virtual long Build_Object( zip_type_pane pane, enum view_MouseAction action, long x, long y, long clicks, zip_type_point X, zip_type_point Y );
  virtual void Destroy_Object( zip_type_figure object );
  virtual long Show_Object_Properties( zip_type_pane pane, zip_type_figure figure );
  virtual long Read_Object( zip_type_figure object );
  virtual long Read_Object_Stream( zip_type_figure object, FILE *file, long id );
  virtual long Write_Object( zip_type_figure object );
  virtual long Draw_Object( zip_type_figure object, zip_type_pane pane );
  virtual long Clear_Object( zip_type_figure object, zip_type_pane pane );
  virtual long Print_Object( zip_type_figure object, zip_type_pane pane );
  virtual long Proximate_Object_Points( zip_type_figure object, zip_type_pane pane, zip_type_pixel x, zip_type_pixel y );
  virtual long Within_Object( zip_type_figure object, zip_type_pane pane, zip_type_pixel x, zip_type_pixel y );
  virtual boolean Enclosed_Object( zip_type_figure object, zip_type_pane pane, zip_type_pixel x, zip_type_pixel y, zip_type_pixel w, zip_type_pixel h );
  virtual long Object_Enclosure( zip_type_figure object, zip_type_pane pane, zip_type_pixel *x, zip_type_pixel *y, zip_type_pixel *w, zip_type_pixel *h );
  virtual long Highlight_Object_Points( zip_type_figure object, zip_type_pane pane );
  virtual long Normalize_Object_Points( zip_type_figure object, zip_type_pane pane );
  virtual long Expose_Object_Points( zip_type_figure object, zip_type_pane pane );
  virtual long Hide_Object_Points( zip_type_figure object, zip_type_pane pane );
  virtual long Set_Object_Point( zip_type_figure object, int point, zip_type_point x, zip_type_point y );
  virtual long Object_Point( zip_type_figure object, long point,zip_type_point *x, zip_type_point *y );
  virtual long Adjust_Object_Point_Suite( zip_type_figure object, zip_type_point x_delta, zip_type_point y_delta );
  virtual long Set_Object_Shade( zip_type_figure object, unsigned char shade );
  virtual long Set_Object_Font( zip_type_figure object, short font );
  virtual boolean Contains( zip_type_figure figure, zip_type_pane pane, zip_type_pixel x, zip_type_pixel y );



  
	zipobject();



  class zip			 *data_object;
  class zipview		 *view_object;
  class zipedit		 *edit_object;
  class zipprint		 *print_object;
  };


#endif /* _zipobject_H_ */
