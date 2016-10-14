#ifndef _ziporang_H_
#define _ziporang_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libzip
 * @{ */
/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1988
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
/*
ziporang.H

  03/31/88	Create for ATK (TCP)
*/

#define  ziporang_VERSION    1

#include <ziporect.H>
class ziporang : public  ziporect
  {
public:
	TRACED_CLASS_NODEST(ziporang);




   char Object_Icon();
   char Object_Icon_Cursor();
   char Object_Datastream_Code();

   long Build_Object( zip_type_pane pane, enum view::MouseAction action, long x, long y, long clicks, zip_type_point X, zip_type_point Y );
   long Show_Object_Properties( zip_type_pane pane, zip_type_figure figure );
   long Draw_Object( zip_type_figure object, zip_type_pane pane );
   long Clear_Object( zip_type_figure object, zip_type_pane pane );
   long Print_Object( zip_type_figure object, zip_type_pane pane );

  };

/** @} */
#endif /* _ziporang_H_ */
