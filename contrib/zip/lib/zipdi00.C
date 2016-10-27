/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1988
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
/* zipedi.c	Zip Data-object	-- Images			      */
/* Author	TC Peters					      */
/* Information Technology Center	   Carnegie-Mellon University */


/*
    $Log: zipdi00.C,v $
 * Revision 1.3  1993/06/17  04:28:00  rr2b
 * made apt_MM_compare a macro which uses ULstrcmp
 * to avoid multiple definitions of apt_MM_compare.
 *
 * Revision 1.2  1993/06/09  17:45:37  gk5g
 * functional C++ version
 *
 * Revision 1.3  1992/12/15  21:57:11  rr2b
 * more disclaimerization fixing
 *
 * Revision 1.2  1992/12/15  04:00:36  rr2b
 * disclaimerization
 *
 * Revision 1.1  1992/10/06  18:33:00  susan
 * Initial revision
 *
 * Revision 2.9  1991/09/12  16:41:46  bobg
 * Update copyright notice and rcsid
 *
 * Revision 2.8  1991/09/10  20:52:14  gk5g
 * Changes in support of SGI_4d platform.
 * Mostly added forward delcarations.
 *
 * Revision 2.7  1990/08/21  14:15:29  sg08
 * Add Set_Image_<Attribute> methods for color and line styles
 *
 * Revision 2.6  89/11/02  11:18:02  sg08
 * Properly turn on mode fields on calls to Set_Image_Shade and Set_Image_Pattern.
 * 
 * Revision 2.5  89/02/17  18:06:45  ghoti
 * ifdef/endif,etc. label fixing - courtesy of Ness
 * 
 * Revision 2.4  89/02/08  16:48:54  ghoti
 * change copyright notice
 * 
 * Revision 2.3  89/02/04  18:52:47  ghoti
 * first pass porting changes: filenames and references to them
 * 
 * Revision 2.2  88/11/18  21:04:50  tom
 * Add Set_Image_Line_Width.
 * 
 * Revision 2.1  88/09/27  18:10:50  ghoti
 * adjusting rcs #
 * 
 * Revision 1.2  88/09/15  17:26:26  ghoti
 * copyright fix
 * 
 * Revision 1.1  88/09/14  17:43:16  tom
 * Initial revision
 * 
*/

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Zip Data-object -- Imagess

MODULE	zipedi.c

NOTICE	IBM Internal Use Only

DESCRIPTION
	This is the suite of Methods that support the Imagee facilities
	of the Zip Data-object.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  03/31/88	Created (TCP)
  11/17/88	Add Set_Image_Line_Width (TCP/SCG)
   11/2/89	Turn on modes in Set_Image_{Shade,Pattern} (SCG)
   08/14/90	Add Set_Image_<Attribute> methods for color and line styles (SCG)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include "zipifm00.h"
#include "dataobject.H"
#include "graphic.H"
#include "zip.H"

#define	 Data			      this
#define  SetStreamModified	      {image->zip_image_stream->\
					zip_stream_states.zip_stream_state_modified = 1;}

static int Allocate_Image( class zip		      *self, struct zip_stream	      *stream, struct zip_image	     **image, const char			      *name );
static int Destroy_Inferior_Image( class zip		      *self, zip_type_image	       image );
static void Free_Image( class zip		      *self, zip_type_image	       image );
static int Propagate_Image_Stream_Ptr( class zip		      *self, zip_type_image	       image );
static int Adjust_Inferior_Image_Point_Suite( class zip		      *self, zip_type_image	       image, int			       x_delta , int			       y_delta );


static
int Allocate_Image( class zip		      *self, struct zip_stream	      *stream, struct zip_image	     **image, const char			      *name )
          {
  int			      status = zip_ok;

  IN(Allocate_Image);
  if ( (*image = (zip_type_image) calloc( 1, sizeof(struct zip_image) )) != NULL )
    {
    (*image)->zip_image_greatest_x = -999999999;
    (*image)->zip_image_least_x    =  999999999;
    (*image)->zip_image_greatest_y = -999999999;
    (*image)->zip_image_least_y    =  999999999;
    (*image)->zip_image_stream = stream;
    (*image)->zip_image_stream->zip_stream_states.zip_stream_state_modified = 1;
    (*image)->zip_image_line_width = 255;
    (*image)->zip_image_line_cap = (graphic::LineCap)-1;
    (*image)->zip_image_line_join = (graphic::LineJoin)-1;
    if ( name  &&  *name )
      status = (self)->Set_Image_Name(  *image, name );
    }
    else  status = zip_insufficient_image_space;
  OUT(Allocate_Image);
  return  status;
  }

long
zip::Create_Peer_Image( struct zip_image	     **image, const char			      *name, struct zip_stream	      *stream, struct zip_image	      *peer )
            {
  long			      status = zip_success;
  zip_type_image	      peer_ptr = peer;

  IN(zip_Create_Peer_Image);
  if ( stream )
    {
    if ( peer_ptr )
      {
      if ( (status = Allocate_Image( this, stream, image, name )) == zip_success )
        {
        if ( status == zip_success )
	  {  
          (*image)->zip_image_peer = peer_ptr->zip_image_peer;
          peer_ptr->zip_image_peer = *image;
          (*image)->zip_image_superior = peer_ptr->zip_image_superior;
	  }
        }
      }
      else  status = (this)->Create_Inferior_Image(  image, name, stream, NULL );
    }
    else  status = zip_stream_non_existent;
  OUT(zip_Create_Peer_Image);
  return status;
  }

long
zip::Create_Inferior_Image( struct zip_image	     **image, const char			      *name, struct zip_stream	      *stream, struct zip_image	      *superior )
            {
  long			      status = zip_success;
  zip_type_image	      superior_ptr = superior, image_ptr;

  IN(zip_Create_Inferior_Image);
  if ( stream )
    {
    if ( (status = Allocate_Image( this, stream, image, name )) == zip_success )
      {
      if ( superior_ptr )
        {
        if ( ( image_ptr = superior_ptr->zip_image_inferior ) )
	  {
	  while ( image_ptr->zip_image_peer )
            image_ptr = image_ptr->zip_image_peer;
	  image_ptr->zip_image_peer = *image;
	  }
	  else /* Superior does not yet have an Inferior */
	  {
	  superior_ptr->zip_image_inferior = *image;
	  }
        (*image)->zip_image_superior = superior_ptr;
        }
        else /* Superior is Null */
        {
        if ( ( image_ptr = stream->zip_stream_image_anchor ) )
	  {
  	  while ( image_ptr->zip_image_peer )
            image_ptr = image_ptr->zip_image_peer;
	  image_ptr->zip_image_peer = *image;
	  }
	  else /* Must be first Image added to Stream */
	  {
	  stream->zip_stream_image_anchor = *image;
	  }
	}
      }
    }
    else  status = zip_stream_non_existent;
  OUT(zip_Create_Inferior_Image);
  return status;
  }

long
zip::Destroy_Image( struct zip_image	      *image )
      {
  int			      status = zip_success;
  zip_type_figure	      figure, figure_ptr;

  IN(zip::Destroy_Image);
  if ( image )
    {
    SetStreamModified;
    figure = image->zip_image_figure_anchor;
    while ( figure  &&  status == zip_success )
      {
      figure_ptr = figure->zip_figure_next;
      status = (this)->Destroy_Figure(  figure );
      figure = figure_ptr;
      }
    if ( image->zip_image_inferior  &&  status == zip_success )
      status = Destroy_Inferior_Image( this, image->zip_image_inferior );
    if (! image->zip_image_state.zip_image_state_unhooked )
      (this)->Unhook_Image(  image );
    Free_Image( this, image );
    }
    else  status = zip_image_non_existent;
  OUT(zip::Destroy_Image);
  return status;
  }

static int
Destroy_Inferior_Image( class zip		      *self, zip_type_image	       image )
      {
  int			      status = zip_success;
  zip_type_figure	      figure_ptr, figure;

  IN(Destroy_Inferior_Image);
  figure = image->zip_image_figure_anchor;
  while ( figure  &&  status == zip_success )
    {
    figure_ptr = figure->zip_figure_next;
    status = (self)->Destroy_Figure(  figure );
    figure = figure_ptr;
    }
  if ( image->zip_image_inferior  &&  status == zip_success )
    status = Destroy_Inferior_Image( self, image->zip_image_inferior );
  if ( image->zip_image_peer  &&  status == zip_success )
    status = Destroy_Inferior_Image( self, image->zip_image_peer );
  if (! image->zip_image_state.zip_image_state_unhooked )
    (self)->Unhook_Image(  image );
  Free_Image( self, image );
  OUT(Destroy_Inferior_Image);
  return  status;
  }

static
void Free_Image( class zip		      *self, zip_type_image	       image )
      {
  IN(Free_Image);
  if ( image->zip_image_name )
    {
    DEBUGst(Image-name,image->zip_image_name);
    DEBUGst(Stream-name,(self)->Containing_Image_Stream(  image )->zip_stream_name);
    symtab_delete( ((zip_type_stream)
	(self)->Containing_Image_Stream(  image ))->zip_stream_symbol_table,
	   (unsigned char *) image->zip_image_name );
    free( image->zip_image_name );
    }
  if ( image->zip_image_text )
    free( image->zip_image_text );
  free( image );
  OUT(Free_Image);
  }

long /*=== HOOK s/b in EDITING module???  ====*/
zip::Hook_Peer_Image( struct zip_image	      *image , struct zip_image	      *peer_image )
      {
  int			      status = zip_success;

  IN(zip_Hook_Peer_Image);
  if ( image  &&  peer_image )
    {
    if ( image->zip_image_state.zip_image_state_unhooked )
      {
      image->zip_image_peer = peer_image->zip_image_peer;
      peer_image->zip_image_peer = image;
      image->zip_image_superior = peer_image->zip_image_superior;
      image->zip_image_stream = peer_image->zip_image_stream;
      Propagate_Image_Stream_Ptr( this, image );

/*=== should stream's greatest/least cells be modified ??? ===*/

      (this)->Set_Image_Name(  image, (this)->Image_Name(  image ) );
      image->zip_image_state.zip_image_state_unhooked = false;
      SetStreamModified;
      }
      else
      status = zip_failure; /*=== s/b "Already Hooked" ===*/
    }
    else
    status = zip_image_non_existent;
  ZIP_STATUS(this);
  OUT(zip_Hook_Peer_Image);
  return status;
  }

long
zip::Hook_Inferior_Image( struct zip_image	      *image , struct zip_image	      *superior_image )
      {
  int			      status = zip_success;

  IN(zip_Hook_Inferior_Image);
  if ( image  &&  superior_image )
    {
    if ( image->zip_image_state.zip_image_state_unhooked )
      {
      image->zip_image_superior = superior_image;
      image->zip_image_peer = superior_image->zip_image_inferior;
      superior_image->zip_image_inferior = image;
      image->zip_image_stream = superior_image->zip_image_stream;
      Propagate_Image_Stream_Ptr( this, image );

/*=== should stream's greatest/least cells be modified ??? ===*/

      (this)->Set_Image_Name(  image, (this)->Image_Name(  image ) );
      image->zip_image_state.zip_image_state_unhooked = false;
      SetStreamModified;
      }
      else
      status = zip_failure; /*=== s/b "Already Hooked" ===*/
    }
    else
    status = zip_image_non_existent;
  ZIP_STATUS(this);
  OUT(zip_Hook_Inferior_Image);
  return status;
  }

static
int Propagate_Image_Stream_Ptr( class zip		      *self, zip_type_image	       image )
      {
  zip_type_image	      image_ptr;
  zip_type_image	      superior_image, peer_image;

  peer_image = image->zip_image_peer;
  image->zip_image_peer = NULL;
  superior_image = image->zip_image_superior;
  image->zip_image_superior = NULL;
  image_ptr = image;
  while ( ( image_ptr = (self)->Next_Image(  image_ptr ) ) )
    {
    image_ptr->zip_image_stream = image->zip_image_stream;
    }
  image->zip_image_peer = peer_image;
  image->zip_image_superior = superior_image;
  return zip_success;
  }

long
zip::Unhook_Image( struct zip_image	      *image )
      {
  struct zip_image	     *image_ptr;
  int			      status = zip_success;

  IN(zip_Unhook_Image);
  if ( image )
    {
    if ( ! image->zip_image_state.zip_image_state_unhooked )
      {
      SetStreamModified;
      if ( image->zip_image_superior )
	{
	if ( image->zip_image_superior->zip_image_inferior == image )
	  {
          image->zip_image_superior->zip_image_inferior = image->zip_image_peer;
	  }
          else
	  {
	  image_ptr = image->zip_image_superior->zip_image_inferior;
	  while ( image_ptr->zip_image_peer != image )
	    image_ptr = image_ptr->zip_image_peer;
	  image_ptr->zip_image_peer = image->zip_image_peer;
	  }
	image->zip_image_superior = NULL;
	}
	else
	{
	image->zip_image_stream->zip_stream_image_anchor = NULL;
	}
      image->zip_image_peer = NULL;
/*===
      image->zip_image_stream = NULL;
      Propagate_Image_Stream_Ptr( this, image );
===*/
/*=== should stream's greatest/least cells be modified ??? ===*/

      image->zip_image_state.zip_image_state_unhooked = true;
      }
      else  status = zip_failure; /*=== s/b "Already Un-Hooked" ===*/
    }
    else  status = zip_image_non_existent;
  ZIP_STATUS(this);
  OUT(zip_Unhook_Image);
  return status;
  }

long
zip::Set_Image_Name( struct zip_image	      *image, const char			      *name )
        {
  int			      status = zip_success;
  zip_type_image		      duplicate;

  IN(zip_Set_Image_Name);
  if ( image )
    {
    DEBUGst( Given Name,name);
    DEBUGst( Image Name,image->zip_image_name);
    SetStreamModified;
/*=== 8/5/87
    if ( image->zip_image_name )
      *image->zip_image_name = '\0';
===*/
    if ( name  &&  *name != '\0' )
      {
      if ( symtab_find( image->zip_image_stream->zip_stream_symbol_table,
		      (unsigned char *) name, (struct user_data  **) &duplicate ) == 0
/*===something wrong in symtab===*/ && strcmp(name,duplicate->zip_image_name)==0  )
        {
        status = zip_duplicate_image_name;
/*===
printf( "DUP IMAGE NAME '%s' (Stream '%s')\n",name,
image->zip_image_stream->zip_stream_name);
===*/
        }
        else
        {
        if ( (image->zip_image_name = strdup( name ) ) == NULL )
          status = zip_insufficient_image_space;
          else
          {
          symtab_add( image->zip_image_stream->zip_stream_symbol_table,
  		    (unsigned char *) image->zip_image_name, (struct user_data  *) image );
          }
        }
      }
      else
      {
      if ( image->zip_image_name )
        free( image->zip_image_name );
      image->zip_image_name = NULL;
      }
    }
    else  status = zip_image_non_existent;
  ZIP_STATUS(this);
  OUT(zip_Set_Image_Name);
  return status;
  }

long
zip::Set_Image_Text( struct zip_image *image, const char *text )
      {
  int			      status = zip_success;

  IN(zip_Set_Image_Text);
  if ( image )
    {
    DEBUGst( Given Text,text);
    DEBUGst( Image Name,image->zip_image_name);
    SetStreamModified;
    if ( text ) {
      if ( (image->zip_image_text = strdup( text ) ) == NULL )
        status = zip_insufficient_image_space;
    }
      else image->zip_image_text = NULL;
    }
    else  status = zip_image_non_existent;
  ZIP_STATUS(this);
  OUT(zip_Set_Image_Text);
  return status;
  }

long
zip::Set_Image_Pattern( struct zip_image *image, unsigned char pattern )
{
  int			      status = zip_success;

  IN(zip_Set_Image_Pattern);
  if ( image )
    {
    DEBUGct( Given Pattern,pattern);
    DEBUGst( Image Name,image->zip_image_name);
    SetStreamModified;
    image->zip_image_fill.zip_image_pattern = pattern;
    image->zip_image_mode.zip_image_mode_patterned = on;
    }
    else  status = zip_image_non_existent;
  ZIP_STATUS(this);
  OUT(zip_Set_Image_Pattern);
  return status;
  }

long
zip::Set_Image_Shade( struct zip_image *image, unsigned char shade )
{
  int			      status = zip_success;

  IN(zip_Set_Image_Shade);
  if ( image )
    {
    DEBUGct( Given Shade,shade);
    DEBUGst( Image Name,image->zip_image_name);
    SetStreamModified;
    image->zip_image_fill.zip_image_shade = shade;
    image->zip_image_mode.zip_image_mode_shaded = on;
    }
    else  status = zip_image_non_existent;
  ZIP_STATUS(this);
  OUT(zip_Set_Image_Shade);
  return status;
  }

long
zip::Set_Image_Font( struct zip_image	      *image, char			      *font_name )
        {
  int			      status = zip_success;

  IN(zip_Set_Image_Font);
  if ( image )
    {
    DEBUGst( Given Font,font_name);
    DEBUGst( Image Name,image->zip_image_name);
    SetStreamModified;
    if ( font_name  &&  *font_name )
      (this)->Define_Font(  font_name, &(image->zip_image_font) );
      else image->zip_image_font = 0;
    }
    else  status = zip_image_non_existent;
  ZIP_STATUS(this);
  OUT(zip_Set_Image_Font);
  return status;
  }

long
zip::Set_Image_Line_Width( struct zip_image	      *image, long			       width )
        {
  int			      status = zip_success;

  IN(zip_Set_Image_Line_Width);
  if ( image )
    {
    DEBUGct( Given Width,width);
    DEBUGst( Image Name,image->zip_image_name);
    SetStreamModified;
    image->zip_image_line_width = width;
    }
    else  status = zip_image_non_existent;
  ZIP_STATUS(this);
  OUT(zip_Set_Image_Line_Width);
  return status;
  }

long
zip::Set_Image_Line_Dash( zip_type_image		 image, unsigned char				 *pattern, int				 offset, graphic::LineDash			 type )
            {
  int			      status = zip_ok;

      IN(zip_Set_Image_Line_Dash);
      if ( image )
      {
	  if ( pattern )
            {
	      image->zip_image_line_dash_pattern = (unsigned char *)strdup( (char *)pattern );
	      image->zip_image_line_dash_offset = offset;
	      image->zip_image_line_dash_type = type;
	    }
	  else image->zip_image_line_dash_type = graphic::LineSolid;
	  SetStreamModified;
      }
      else
	  status = zip_image_non_existent;
      ZIP_STATUS(this);
      OUT(zip_Set_Image_Line_Dash);
      return status;
  }

long
zip::Set_Image_Line_Cap( zip_type_image		 image, graphic::LineCap			 cap )
        {
  int			      status = zip_ok;

      IN(zip_Set_Image_Line_Cap);
      if ( image )
      {
	  SetStreamModified;
	  image->zip_image_line_cap = cap;
      }
      else
	  status = zip_image_non_existent;
      ZIP_STATUS(this);
      OUT(zip_Set_Image_Line_Cap);
      return status;
  }

long
zip::Set_Image_Line_Join( zip_type_image		 image, graphic::LineJoin			 join )
        {
  int			      status = zip_ok;

      IN(zip_Set_Image_Line_Join);
      if ( image )
      {
	  SetStreamModified;
	  image->zip_image_line_join = join;
      }
      else
	  status = zip_image_non_existent;
      ZIP_STATUS(this);
      OUT(zip_Set_Image_Line_Join);
      return status;
  }

long
zip::Set_Image_Line_Color( zip_type_image		 image, double			 red , double			 green , double			 blue )
        {
  int			      status = zip_ok;

     IN(zip_Set_Image_Line_Color);
     if ( image )
      {
	  if ( image->zip_image_color_values == NULL )
	    image->zip_image_color_values = (this )->Allocate_Color_Values( );
          if ( image->zip_image_color_values &&
	     ( image->zip_image_color_values->line == NULL ))
	    image->zip_image_color_values->line = (this )->Allocate_Color( );
 	  if (  image->zip_image_color_values && image->zip_image_color_values->line )
            {
  	      SetStreamModified;
  	      image->zip_image_color_values->line->red = red;
  	      image->zip_image_color_values->line->green = green;
  	      image->zip_image_color_values->line->blue = blue;
	    }
	  else status = zip_failure;
      }
      else
	  status = zip_image_non_existent;
      ZIP_STATUS(this);
      OUT(zip_Set_Image_Line_Color);
      return status;
  }

long
zip::Set_Image_FillFG_Color( zip_type_image		 image, double			 red , double			 green , double			 blue )
        {
  int			      status = zip_ok;

     IN(zip_Set_Image_FillFG_Color);
     if ( image )
      {
	  if ( image->zip_image_color_values == NULL )
	    image->zip_image_color_values = (this )->Allocate_Color_Values( );
          if ( image->zip_image_color_values &&
	     ( image->zip_image_color_values->fillfg == NULL ))
	    image->zip_image_color_values->fillfg = (this )->Allocate_Color( );
 	  if (  image->zip_image_color_values && image->zip_image_color_values->fillfg )
            {
  	      SetStreamModified;
  	      image->zip_image_color_values->fillfg->red = red;
  	      image->zip_image_color_values->fillfg->green = green;
  	      image->zip_image_color_values->fillfg->blue = blue;
	    }
	  else status = zip_failure;
      }
      else
	  status = zip_image_non_existent;
      ZIP_STATUS(this);
      OUT(zip_Set_Image_FillFG_Color);
      return status;
  }

long
zip::Set_Image_FillBG_Color( zip_type_image		 image, double			 red , double			 green , double			 blue )
        {
  int			      status = zip_ok;

     IN(zip_Set_Image_FillBG_Color);
     if ( image )
      {
	  if ( image->zip_image_color_values == NULL )
	    image->zip_image_color_values = (this )->Allocate_Color_Values( );
          if ( image->zip_image_color_values &&
	     ( image->zip_image_color_values->fillbg == NULL ))
	    image->zip_image_color_values->fillbg = (this )->Allocate_Color( );
 	  if (  image->zip_image_color_values && image->zip_image_color_values->fillbg )
            {
  	      SetStreamModified;
  	      image->zip_image_color_values->fillbg->red = red;
  	      image->zip_image_color_values->fillbg->green = green;
  	      image->zip_image_color_values->fillbg->blue = blue;
	    }
	  else status = zip_failure;
      }
      else
	  status = zip_image_non_existent;
      ZIP_STATUS(this);
      OUT(zip_Set_Image_FillBG_Color);
      return status;
  }

long
zip::Adjust_Image_Point_Suite( struct zip_image *image, long x_delta, long y_delta )
{
  int			      status = zip_success;
  zip_type_figure	      figure_ptr;

  IN(zip::Adjust_Image_Point_Suite);
  if ( image )
    {
    SetStreamModified;
    figure_ptr = image->zip_image_figure_anchor;
    while ( figure_ptr  &&  status == zip_success )
      {
      status = (this)->Adjust_Figure_Point_Suite(  figure_ptr, x_delta, y_delta );
      figure_ptr = figure_ptr->zip_figure_next;
     }
    if ( image->zip_image_inferior  &&  status == zip_success )
      status = Adjust_Inferior_Image_Point_Suite( this, image->zip_image_inferior,
						  x_delta, y_delta );
    }
    else  status = zip_image_non_existent;
  ZIP_STATUS(this);
  OUT(zip::Adjust_Image_Point_Suite);
  return status;
  }

static int
Adjust_Inferior_Image_Point_Suite( class zip		      *self, zip_type_image	       image, int			       x_delta , int			       y_delta )
        {
  int			      status = zip_success;
  zip_type_figure	      figure_ptr;

  IN(Adjust_Inferior_Image_Point_Suite);
  figure_ptr = image->zip_image_figure_anchor;
  while ( figure_ptr  &&  status == zip_success )
    {
    status = (self)->Adjust_Figure_Point_Suite(  figure_ptr, x_delta, y_delta );
    figure_ptr = figure_ptr->zip_figure_next;
    }
  if ( image->zip_image_inferior  &&  status == zip_success )
    status = Adjust_Inferior_Image_Point_Suite( self, image->zip_image_inferior,
						      x_delta, y_delta );
  if ( image->zip_image_peer  &&  status == zip_success )
    status = Adjust_Inferior_Image_Point_Suite( self, image->zip_image_peer,
						      x_delta, y_delta );
  OUT(Adjust_Inferior_Image_Point_Suite);
  return status;
  }

struct zip_image *
zip::Image( const char			      *name )
      {
  zip_type_image		      image = NULL;
  zip_type_stream_chain      stream_link =
					StreamAnchor;
  int			      status = zip_success;

  IN(zip::Image);
  while ( stream_link )
    {
    if ( stream_link->zip_stream_chain_ptr->zip_stream_symbol_table )
       if ( symtab_find( stream_link->zip_stream_chain_ptr->zip_stream_symbol_table,
	     (unsigned char *) name, (struct user_data  **) &image ) == 0 )
	  break;
    stream_link = stream_link->zip_stream_chain_next;
    }
  ZIP_STATUS(this);
  OUT(zip::Image);
  return  image;
  }

struct zip_image *
zip::Stream_Image( struct zip_stream	      *stream, const char			      *name )
        {
  zip_type_image		      image = NULL;
  int			      status = zip_success;

  IN(zip::Stream_Image);
  if ( stream  &&  name )
    if ( stream->zip_stream_symbol_table )
      symtab_find( stream->zip_stream_symbol_table, (unsigned char *) name, (struct user_data  **) &image );
  ZIP_STATUS(this);
  OUT(zip::Stream_Image);
  return  image;
  }

struct zip_image *
zip::Next_Image( struct zip_image	      *image )
      {
  zip_type_image	      next_image = NULL;

  IN(zip::Next_Image);
  if ( !( next_image = image->zip_image_inferior ) )
     if ( !( next_image = image->zip_image_peer ) )
        {
        next_image = image->zip_image_superior;
        while ( next_image )
          if ( next_image->zip_image_peer )
            {
            next_image = next_image->zip_image_peer;
	    break;
	    }
            else
	    next_image = next_image->zip_image_superior;
        }
  OUT(zip::Next_Image);  
  return  next_image;
  }

 zip_type_image 
zip::Image_Left_Peer( zip_type_image	       image )
      {
  zip_type_image	      peer = NULL, candidate;

  IN(zip::Image_Left_Peer);
  if ( image  &&  image->zip_image_superior )
    {
    candidate = image->zip_image_superior->zip_image_inferior;
    while( candidate != image )
      {
      peer = candidate;
      candidate = candidate->zip_image_peer;
      }
    }
  OUT(zip::Image_Left_Peer);
  return  peer;
  }

int
zip::Set_Image_Extrema( zip_type_image	       image, zip_type_point	       x , zip_type_point	       y )
        {
  IN(zip::Set_Image_Extrema);
  if ( x < image->zip_image_least_x )
    {
    image->zip_image_least_x = x;
    DEBUGdt(Least    X,x );
    }
  if ( x > image->zip_image_greatest_x )
    { 
    image->zip_image_greatest_x = x;
    DEBUGdt(Greatest X,x );
    }
  if ( y < image->zip_image_least_y )
    {
    image->zip_image_least_y = y;
    DEBUGdt(Least    Y,y );
    }
  if ( y > image->zip_image_greatest_y )
    {
    image->zip_image_greatest_y = y;
    DEBUGdt(Greatest Y,y );
    }
  if ( image->zip_image_superior )
    (this)->Set_Image_Extrema(  image->zip_image_superior, x, y );
  OUT(zip::Set_Image_Extrema);
  return zip_success;
  }

/*===
static int
ZIP_Balance_Image_Extrema( self, image )
  struct zip		     *self;
  zip_type_image		  image;
  {
  IN(ZIP_Balance_Image_Extrema);
  if ( abs(image->zip_image_least_x) < abs(image->zip_image_greatest_x) )
      image->zip_image_least_x    = -abs(image->zip_image_greatest_x);
      else
      image->zip_image_greatest_x =  abs(image->zip_image_least_x);
  if ( abs(image->zip_image_least_y) < abs(image->zip_image_greatest_y) )
      image->zip_image_least_y    = -abs(image->zip_image_greatest_y);
      else
      image->zip_image_greatest_y =  abs(image->zip_image_least_y);
  OUT(ZIP_Balance_Image_Extrema);
  return zip_success;
  }
===*/
