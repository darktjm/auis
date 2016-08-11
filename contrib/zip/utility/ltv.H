#ifndef _ltv_H_
#define _ltv_H_
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


/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The LightTable View-object Specification

MODULE	ltv.c

NOTICE	IBM Internal Use Only

DESCRIPTION
	This defines the suite of Methods that support the LightTable View-object.

HISTORY
  010/10/88	Created (TCP)

END-SPECIFICATION  ************************************************************/

#define  ltv_VERSION		    1


#include <view.H>
class ltv : public  view {
public:
  virtual ATKregistryEntry *ATKregistry();

   view_DSattributes DesiredSize( long width, long height, enum view_DSpass pass, long *dwidth, long *dheight )
	 ;
   void FullUpdate( enum view_UpdateType type, long left, long top, long width, long height );
   void LinkTree ( class view *parent );
   void Update();
   void ObservedChanged( class observable *changed, long value );
   class view * Hit( enum view_MouseAction action, long x, long y, long n );
   void SetDataObject( class dataobject *data );
   void ReceiveInputFocus();
   void LoseInputFocus();

   virtual void Set_Debug( boolean debug );
 
   static boolean InitializeClass();
   static boolean atk_inited;

   ltv();
   virtual ~ltv();

   class lt			    *data;
   class zipview		    *zipviewp;
   struct zip_pane		    *background_pane, *foreground_pane;
   class rasterview		    *rasterviewp;
   class suite			    *buttons;
   struct suite_item		    *left_name_item, *right_name_item;
   struct rectangle		     block, enclosure;
   class menulist		    *menu;
   class cursor			    *cursor;
   unsigned char			     background_light;
   boolean			     background_exposed;
   boolean			     enclosure_exposed;
   boolean			     building;
   boolean			     build;
   boolean			     tracking;
   boolean			     modifying;
   boolean			     modified;
   boolean			     foreground_panning;
   boolean			     input_focus;
   struct zip_stream		    *stream;
   struct zip_image		    *image;
   struct zip_figure		    *figure;
   long				     point;
   long				     initial_x_pixel, initial_y_pixel;
   long				     background_left, background_top, background_x_offset, background_y_offset;
   unsigned char		     wm_type;
   char				     tentative[512];
};

#endif /* _ltv_H_ */