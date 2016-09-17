#ifndef _zipoline_H_
#define _zipoline_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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
zipoline.ch

  03/31/88	Create for ATK (TCP)
   08/16/90	Add Contains method (SCG)
*/

#define  zipoline_VERSION    1

#include <zipobject.H>
class zipoline : public  zipobject
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
   long Highlight_Object_Points( zip_type_figure object, zip_type_pane pane );
   long Normalize_Object_Points( zip_type_figure object, zip_type_pane pane );
   long Expose_Object_Points( zip_type_figure object, zip_type_pane pane );
   long Hide_Object_Points( zip_type_figure object, zip_type_pane pane );
   boolean Contains( zip_type_figure figure, zip_type_pane pane, zip_type_pixel x,  zip_type_pixel y );
  };

#endif /* _zipoline_H_ */
