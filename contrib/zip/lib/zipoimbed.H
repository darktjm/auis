#ifndef _zipoimbed_H_
#define _zipoimbed_H_
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
zipoimbed.H

  03/31/88	Create for ATK (TCP)
  08/07/89	Add Object_Modified override (TCP)
*/

#define  zipoimbed_VERSION    1

#include <ziporect.H>
class zipoimbed : public  ziporect
  {
public:
	virtual ATKregistryEntry *ATKregistry();




   char Object_Icon();
   char Object_Icon_Cursor();
   char Object_Datastream_Code();

   class view * Object_Hit( zip_type_figure figure, enum view_MouseAction action, long x , long  y, long clicks );
   long Object_Modified( zip_type_figure object );

   long Build_Object( zip_type_pane pane, enum view_MouseAction action, long x, long y, long clicks, zip_type_point X, zip_type_point Y );
   void Destroy_Object( zip_type_figure object );
   long Show_Object_Properties( zip_type_pane pane, zip_type_figure figure );
   long Read_Object( zip_type_figure object );
   long Read_Object_Stream( zip_type_figure figure, FILE *file, long id );
   long Write_Object( zip_type_figure object );
   long Draw_Object( zip_type_figure object, zip_type_pane pane );
   long Clear_Object( zip_type_figure object, zip_type_pane pane );
   long Print_Object( zip_type_figure object, zip_type_pane pane );


  
	zipoimbed();


  boolean						no_outline;

  };

/** @} */
#endif /* _zipoimbed_H_ */
