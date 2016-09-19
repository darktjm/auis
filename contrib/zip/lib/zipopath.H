#ifndef _zipopath_H_
#define _zipopath_H_
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
zipopath.H

  03/31/88	Create for ATK (TCP)
   08/14/90	Add Contains Method (SCG)
*/

#define  zipopath_VERSION    1

#include <zipobject.H>
class zipopath : public  zipobject
  {
public:
	virtual ATKregistryEntry *ATKregistry();




   char Object_Icon();
   char Object_Icon_Cursor();
   char Object_Datastream_Code();

   long Build_Object( zip_type_pane pane, enum view_MouseAction action, long x, long y, long clicks, zip_type_point X, zip_type_point Y );
   long Show_Object_Properties( zip_type_pane pane, zip_type_figure figure );
   long Draw_Object( zip_type_figure object, zip_type_pane pane );
   long Clear_Object( zip_type_figure object, zip_type_pane pane );
   long Print_Object( zip_type_figure object, zip_type_pane pane );
   long Proximate_Object_Points( zip_type_figure object, zip_type_pane pane, zip_type_pixel x, zip_type_pixel y );
   boolean Enclosed_Object( zip_type_figure object, zip_type_pane pane, zip_type_pixel x, zip_type_pixel y, zip_type_pixel w, zip_type_pixel h );
   long Object_Enclosure( zip_type_figure object, zip_type_pane pane, zip_type_pixel *x, zip_type_pixel *y, zip_type_pixel *w, zip_type_pixel *h );
   long Within_Object( zip_type_figure figure, zip_type_pane pane,zip_type_pixel x,  zip_type_pixel y );
   long Highlight_Object_Points( zip_type_figure object, zip_type_pane pane );
   long Normalize_Object_Points( zip_type_figure object, zip_type_pane pane );
   long Expose_Object_Points( zip_type_figure object, zip_type_pane pane );
   long Hide_Object_Points( zip_type_figure object, zip_type_pane pane );
   long Set_Object_Point( zip_type_figure object, int point, zip_type_point x, zip_type_point y );
   long Object_Point( zip_type_figure figure, long point, zip_type_point *x, zip_type_point *y );
   long Adjust_Object_Point_Suite( zip_type_figure object, zip_type_point x_delta, zip_type_point y_delta );
   long Set_Object_Shade( zip_type_figure object, unsigned char shade );
   boolean Contains( zip_type_figure figure, zip_type_pane pane, zip_type_pixel x,  zip_type_pixel y );
  };


#endif /* _zipopath_H_ */
