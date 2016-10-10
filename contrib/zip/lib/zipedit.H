#ifndef _zipedit_H_
#define _zipedit_H_
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
zipedit.H

  03/31/88	Created for ATK (TCP)
*/

#define  zipedit_VERSION		    1

#include "zip.h"
#include "zipfig.h"
#include "zipimage.h"
#include "zipstrm.h"
#include "zippane.h"
#include "zipedit.h"

struct zipedit_options
  {
  unsigned int				    invert		: 1;
  };

struct zipedit_states
  {
  unsigned int				    grid_exposed	: 1;
  unsigned int				    points_constrained	: 1;
  unsigned int				    coordinates_exposed	: 1;
  unsigned int				    palettes_exposed	: 1;
  unsigned int				    background_exposed	: 1;
  unsigned int				    background_selected	: 1;
  unsigned int				    background_lightened: 1;
  unsigned int				    enclosure_exposed	: 1;
  unsigned int				    duplicate_selection	: 1;
  unsigned int				    foreground_panning	: 1;
  unsigned int				    moving		: 1;
  };


#include <ATK.H>

typedef enum view::MouseAction (*zip_keyboardfptr)(long anchor, zip_type_pane pane, char c, enum view::MouseAction action, long x, long y, long clicks);
typedef long (*zip_pendingfptr)(class ziposymbol *self, zip_type_pane pane);

class zipedit : public ATK 
{
public:
	virtual ATKregistryEntry *ATKregistry();
	static boolean InitializeClass();
	zipedit();
	virtual ~zipedit();





  virtual void Set_Debug( boolean state );
  virtual void Update();
  virtual int Accept_Hit( zip_type_pane pane, enum view::MouseAction action, long x, long y, long n );

  virtual long Set_Data_Object( class zip *data_object );
  virtual long Set_View_Object( class zipview *view_object );

  virtual long Delete_Figure( zip_type_figure figure, zip_type_pane pane );
  virtual long Undelete_Figure( zip_type_figure figure, zip_type_pane pane );

  virtual long Which_Figure_Point( zip_type_figure figure, zip_type_pane pane, long x, long y );
  virtual long Highlight_Figure_Points( zip_type_figure figure, zip_type_pane pane );
  virtual long Normalize_Figure_Points( zip_type_figure figure, zip_type_pane pane );
  virtual long Hide_Figure_Points( zip_type_figure figure, zip_type_pane pane );
  virtual long Expose_Figure_Points( zip_type_figure figure, zip_type_pane pane );

  virtual long Delete_Image( zip_type_image image, zip_type_pane pane );
  virtual long Undelete_Image( zip_type_image image, zip_type_pane pane );
  virtual long Highlight_Image_Points( zip_type_image image, zip_type_pane pane );
  virtual long Normalize_Image_Points( zip_type_image image, zip_type_pane pane );
  virtual long Hide_Image_Points( zip_type_image image, zip_type_pane pane );
  virtual long Expose_Image_Points( zip_type_image image, zip_type_pane pane );

  virtual long Set_Pane_Highlight_Icon( zip_type_pane pane, char icon );

  virtual long Hide_Pane_Points( zip_type_pane pane );
  virtual long Expose_Pane_Points( zip_type_pane pane );
  virtual long Hide_Pane_Coordinates( zip_type_pane pane );
  virtual long Expose_Pane_Coordinates( zip_type_pane pane );
  virtual long Draw_Pane_Coordinates( zip_type_pane pane );
  virtual long Hide_Pane_Grid( zip_type_pane pane );
  virtual long Expose_Pane_Grid( zip_type_pane pane );
  virtual long Draw_Pane_Grid( zip_type_pane pane );
  virtual long Halve_Pane_Grid( zip_type_pane pane );
  virtual long Double_Pane_Grid( zip_type_pane pane );
  virtual void Lighten_Pane( zip_type_pane pane, char density );
  virtual void Align_Pane( zip_type_pane pane );

  virtual long Handle_Editing( zip_type_pane pane );
  virtual long Expose_Palettes( zip_type_pane pane );
  virtual long Hide_Palettes( zip_type_pane pane );
  virtual long Set_Palettes( zip_type_pane pane, int palette_mode );
  virtual long Redisplay_Panes( zip_type_pane pane );
  virtual long Initialize_Editing( zip_type_pane pane );
  virtual long Terminate_Editing( zip_type_pane pane );
  virtual long Constrain_Points( zip_type_pane  pane );
  virtual long Unconstrain_Points( zip_type_pane  pane );

  /****  Following Facilities For Sub-class Usage Only  ****/
  virtual void Expose_Point( zip_type_pane pane, zip_type_figure figure, zip_type_pixel x, zip_type_pixel y );
  virtual void Hide_Point( zip_type_pane pane, zip_type_figure figure, zip_type_pixel x, zip_type_pixel y );
  virtual long Highlight_Point( zip_type_pane pane, zip_type_pixel x, zip_type_pixel y );
  virtual long Normalize_Point( zip_type_pane pane, zip_type_pixel x, zip_type_pixel y );
  virtual long Highlight_Handles( zip_type_pane pane, zip_type_pixel x1, zip_type_pixel x2, zip_type_pixel x3, zip_type_pixel y1, zip_type_pixel y2, zip_type_pixel y3 );
  virtual long Highlight_Handle( zip_type_pane pane, zip_type_pixel x, zip_type_pixel y );
  virtual long Normalize_Handles( zip_type_pane pane, zip_type_pixel x1, zip_type_pixel x2, zip_type_pixel x3, zip_type_pixel y1, zip_type_pixel y2, zip_type_pixel y3 );
  virtual long Normalize_Handle( zip_type_pane pane, zip_type_pixel x, zip_type_pixel y );

  inline void Set_Keyboard_Processor( long anchor, zip_keyboardfptr processor ) 
    { this->keyboard_processor = processor;this->keyboard_anchor = (long)anchor;}

  inline void Set_Pending_Processor( long anchor, zip_pendingfptr processor ) 
    {this->pending_processor = processor;this->pending_anchor = (long)anchor;}

  class zip			 *data_object;
  class zipview			 *view_object;
  struct zipedit_options	  options;
  struct zipedit_states		  states;
  long				  action;
  zip_type_pane			  pane;
  class fontdesc		 *icon_font;
  class fontdesc		 *points_font;
  class fontdesc		 *dots_font;
  zip_keyboardfptr		 keyboard_processor;
  long				  keyboard_anchor;
  class keystate		 *keystate;
  class menulist		 *menu;
  long				  prior_x, prior_y;
  char				  font_family;
  long				  font_height;
  char				  font_bold, font_italic;
  long				  font_vertical, font_horizontal;
  zip_type_pixel		  enclosure_left, enclosure_top,
				  enclosure_width, enclosure_height;
  zip_type_figure		 *enclosed_figures;
  zip_pendingfptr		  pending_processor;
  long				  pending_anchor;
  zip_type_pixel		  enclosure_shadow_start_x, enclosure_shadow_start_y,
				  enclosure_shadow_last_x, enclosure_shadow_last_y;
};
/** @} */
#endif /* _zipedit_H_ */
