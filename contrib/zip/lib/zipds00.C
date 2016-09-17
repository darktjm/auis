/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

/* zipeds.c	Zip Data-object	-- Streams			      */
/* Author	TC Peters					      */
/* Information Technology Center	   Carnegie-Mellon University */


/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Zip Data-object -- Streams

MODULE	zipeds.c

    NOTICE	IBM Internal Use Only

DESCRIPTION
	This is the suite of Methods that support the Stream facilities
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
  11/17/88	Auto creation of ZIP_ROOT_IMAGE for Create4_Stream (TCP/SG)
		Add Set_Stream_Line_Width.
  11/22/88	Eliminate automatic appending of .zip suffix to file-names (TCP)
   08/14/90	Add Set_Stream_<Attribute> for colors and line styles
                            Add proper defaults for line style attributes (SCG)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include "graphic.H"
#include "dataobject.H"
#include "environ.H"
#include "zip.H"
#include <errno.h>
#include <sys/stat.h>
#include <util.h>

#define  zip_default_path	      "/usr/andrew/lib/zip"
#define  zip_default_path_alternate   "/usr/local/lib/zip"

#define	 Data			      this

static void Extract_Stream_Name( class zip		      *self, const char			      *name, char			     **stream_name );
static void Extract_Stream_File_Name( class zip		      *self, const char			      *name, char			     **file_name );
static void Extract_Stream_File_Path( class zip		      *self, const char			      *name, char			     **path_name );
int zip_Open_Stream_File( class zip		      *self, zip_type_stream	       stream, long			       open_mode );
static int Open_Via_Alternate_Paths( class zip		      *self, zip_type_stream	       stream, const char  *			      mode );
static int Open_Alternate( class zip		      *self, zip_type_stream	       stream, const char  *			      open_mode_flags, const char			      *path );
int zip_Close_Stream_File( class zip		      *self, zip_type_stream	       stream );
static void Reset_Stream_File_Open_States( class zip		      *self, zip_type_stream	       stream );
static int Identify_Paths( class zip		      *self, zip_type_paths	      *paths_ptr );
static int Allocate_Stream_Object( class zip		      *self, zip_type_stream	      *stream, const char			      *name );
int zip_Set_Stream_File_Name( class zip		      *self, zip_type_stream	       stream, const char			      *name );
static int Deallocate_Stream_Object( class zip		      *self, zip_type_stream	       stream );
static int Deallocate_Stream_Resources( class zip		      *self, zip_type_stream	       stream );


long
zip::Open_Stream( zip_type_stream	      *stream, char			      *name, long			       mode )
          {
  int			      status = zip_ok;

  IN(zip::Open_Stream);
  DEBUGst(Name,name);
  DEBUGxt(Mode,mode);
  if (
	(status = Allocate_Stream_Object( this, stream, name )) == zip_ok
        &&
        (status = zip_Open_Stream_File( this, *stream, mode )) == zip_ok
     )
    {
    zip_Close_Stream_File( this, *stream );
    }
    else  *stream = NULL;
  ZIP_STATUS(this);
  OUT(zip::Open_Stream);
  return status;
  }

long
zip::Close_Stream( struct zip_stream	      *stream )
  /* ziP_Close_Stream is only invoked by the MACRO zip_Close_Stream
	-- to ensure the stream anchor cell in client space
	is nullified
  */

      {
  int			      status = zip_ok;
/*===
  zip_type_pane	      pane;
  zip_type_pane_chain	      pane_chain;
  zip_type_pane_auxiliary_stream  auxiliary, auxiliary_next;
===*/
  IN(zip::Close_Stream);
  if ( stream )
    {
/*===
    pane_chain = Env->zip_env_pane_anchor;
    while ( pane_chain )
      {
      pane = pane_chain->zip_pane_chain_ptr;
      auxiliary = pane->zip_pane_auxiliary_stream;
      while ( auxiliary )
	{
	auxiliary_next = auxiliary->zip_pane_auxiliary_stream_next; */
/*===
	if ( auxiliary->zip_pane_auxiliary_stream_ptr == stream )
	  zip_Reset_Pane_Auxiliary_Stream( this, pane, stream );
===!/
	auxiliary = auxiliary_next;
	}
      pane_chain = pane_chain->zip_pane_chain_next;
      }
===*/
    if ( (status = zip_Close_Stream_File( this, stream )) == zip_ok )
      status = Deallocate_Stream_Object( this, stream );
    }
  ZIP_STATUS(this);
  OUT(zip::Close_Stream);
  return status;
  }

extern long zip_Deparse_Stream( class zip *self, zip_type_stream stream_object );
extern long zip_Enparse_Stream( class zip *self, zip_type_stream stream_object );

long
zip::Read_Stream( struct zip_stream	      *stream )
      {
  int			      status = zip_ok;

  IN(zip::Read_Stream);
  if ( stream )
    {
    if ( (status = zip_Open_Stream_File( this, stream, zip_read )) == zip_ok )
      {
      status = zip_Deparse_Stream( this, stream );
      zip_Close_Stream_File( this, stream );
      }
    }
    else status = zip_stream_non_existent;
  ZIP_STATUS(this);
  OUT(zip::Read_Stream);
  return status;
  }

long
zip::Write_Stream( struct zip_stream	      *stream )
      {
  int			      status = zip_ok;

  IN(zip::Write_Stream);
  if ( stream )
    {
    if ( (status = zip_Open_Stream_File( this, stream, zip_write )) == zip_ok )
      if ( (status = zip_Enparse_Stream( this, stream ) ) == zip_ok )
	status = zip_Close_Stream_File( this, stream );
    }
    else status = zip_stream_non_existent;
  OUT(zip::Write_Stream);
  ZIP_STATUS(this);
  return status;
  }

long
zip::Create_Stream( zip_type_stream *stream, char *name, unsigned char mode )
{
  int			      status = zip_ok;
  zip_type_image		      image;

  IN(zip_Create_Stream);
  if ( (status = Allocate_Stream_Object( this, stream, name )) == zip_ok )
    {
    if ( (status = (this)->Create_Inferior_Image(  &image,
	  	    "ZIP_ROOT_IMAGE", *stream, NULL )) == zip_ok )
      {
      image->zip_image_state.zip_image_state_transient = true;
      ( *stream )->zip_stream_image_anchor = image;
      }
    else status = zip_failure; /*===*/
    }
  else status = zip_failure;
  ZIP_STATUS(this);
  OUT(zip_Create_Stream);
  return status;
  }

long
zip::Destroy_Stream( zip_type_stream stream )
      {
  int			      status = zip_ok;

  IN(zip_Destroy_Stream);
  if ( stream )
    status = Deallocate_Stream_Object( this, stream );
    else status = zip_stream_non_existent;
  ZIP_STATUS(this);
  OUT(zip_Destroy_Stream);
  return status;
  }

long
zip::Set_Stream_Name( zip_type_stream	       stream, char			      *name )
        {
  int			      status = zip_ok;

  IN(zip::Set_Stream_Name);
  if ( stream  &&  name )
    {
    Extract_Stream_Name( this, name, &stream->zip_stream_name );
    Extract_Stream_File_Name( this, name, &stream->zip_stream_file_name );
    Extract_Stream_File_Path( this, name, &stream->zip_stream_file_path );
    stream->zip_stream_file_full_name = (char *) malloc( 259 );
    stream->zip_stream_file_full_name[0] = '\0';
    if ( stream->zip_stream_file_path[0] )
      {
      strcpy( stream->zip_stream_file_full_name, stream->zip_stream_file_path );
      strcat( stream->zip_stream_file_full_name, "/" );
      }
    strcat( stream->zip_stream_file_full_name, stream->zip_stream_file_name );
    DEBUGst(Full-name, stream->zip_stream_file_full_name );
    }
    else
    {
/*===*/
    }
  ZIP_STATUS(this);
  OUT(zip::Set_Stream_Name);
  return status;
  }

long
zip::Set_Stream_Pattern( zip_type_stream stream, unsigned char pattern )
{
  int			      status = zip_ok;

  IN(zip_Set_Stream_Pattern);
  if ( stream  &&  pattern )
    {
    DEBUGct( Given Pattern,pattern);
    DEBUGst(Stream Name,stream->zip_stream_name);
    stream->zip_stream_fill.zip_stream_pattern = pattern;
    }
    else
    {
/*===*/
    }
  ZIP_STATUS(this);
  OUT(zip_Set_Stream_Pattern);
  return status;
  }

long
zip::Set_Stream_Line_Width( zip_type_stream	       stream, long			       width )
        {
  int			      status = zip_ok;

  IN(zip_Set_Stream_Line_Width);
  if ( stream )
    {
    DEBUGct( Given Width,width);
    DEBUGst(Stream Name,stream->zip_stream_name);
    stream->zip_stream_line_width = width;
    }
    else
    {
/*===*/
    }
  ZIP_STATUS(this);
  OUT(zip_Set_Stream_Line_Width);
  return status;
  }

long
zip::Set_Stream_Line_Dash( zip_type_stream		 stream, char				 *pattern, int				 offset, short			 type )
            {
  int			      status = zip_ok;

      IN(zip_Set_Stream_Line_Dash);
      if ( stream )
      {
	  if ( pattern )
            {
	      stream->zip_stream_line_dash_pattern = strdup( pattern );
	      stream->zip_stream_line_dash_offset = offset;
	      stream->zip_stream_line_dash_type = type;
	    }
	  else stream->zip_stream_line_dash_type = graphic_LineSolid;
      }
      else
	  status = zip_stream_non_existent;
      ZIP_STATUS(this);
      OUT(zip_Set_Stream_Line_Dash);
      return status;
  }

long
zip::Set_Stream_Line_Cap( zip_type_stream		 stream, short			 cap )
        {
  int			      status = zip_ok;

      IN(zip_Set_Stream_Line_Cap);
      if ( stream )
	  stream->zip_stream_line_cap = cap;
      else
	  status = zip_stream_non_existent;
      ZIP_STATUS(this);
      OUT(zip_Set_stream_Line_Cap);
      return status;
  }

long
zip::Set_Stream_Line_Join( zip_type_stream		 stream, short			 join )
        {
  int			      status = zip_ok;

      IN(zip_Set_Stream_Line_Join);
      if ( stream )
	  stream->zip_stream_line_join = join;
      else
	  status = zip_stream_non_existent;
      ZIP_STATUS(this);
      OUT(zip_Set_Stream_Line_Join);
      return status;
  }

long
zip::Set_Stream_Line_Color( zip_type_stream		 stream, double			 red , double			 green , double			 blue )
        {
  int			      status = zip_ok;

     IN(zip_Set_Stream_Line_Color);
     if ( stream )
      {
	  if ( stream->zip_stream_color_values == NULL )
	    stream->zip_stream_color_values = (this )->Allocate_Color_Values( );
          if ( stream->zip_stream_color_values &&
	     ( stream->zip_stream_color_values->line == NULL ))
	    stream->zip_stream_color_values->line = (this )->Allocate_Color( );
 	  if (  stream->zip_stream_color_values && stream->zip_stream_color_values->line )
            {
  	      stream->zip_stream_color_values->line->red = red;
  	      stream->zip_stream_color_values->line->green = green;
  	      stream->zip_stream_color_values->line->blue = blue;
	    }
	  else status = zip_failure;
      }
      else
	  status = zip_stream_non_existent;
      ZIP_STATUS(this);
      OUT(zip_Set_Stream_Line_Color);
      return status;
  }

long
zip::Set_Stream_FillFG_Color( zip_type_stream		 stream, double			 red , double			 green , double			 blue )
        {
  int			      status = zip_ok;

     IN(zip_Set_Stream_FillFG_Color);
     if ( stream )
      {
	  if ( stream->zip_stream_color_values == NULL )
	    stream->zip_stream_color_values = (this )->Allocate_Color_Values( );
          if ( stream->zip_stream_color_values &&
	     ( stream->zip_stream_color_values->fillfg == NULL ))
	    stream->zip_stream_color_values->fillfg = (this )->Allocate_Color( );
 	  if (  stream->zip_stream_color_values && stream->zip_stream_color_values->fillfg )
            {
  	      stream->zip_stream_color_values->fillfg->red = red;
  	      stream->zip_stream_color_values->fillfg->green = green;
  	      stream->zip_stream_color_values->fillfg->blue = blue;
	    }
	  else status = zip_failure;
      }
      else
	  status = zip_stream_non_existent;
      ZIP_STATUS(this);
      OUT(zip_Set_Stream_FillFG_Color);
      return status;
  }

long
zip::Set_Stream_FillBG_Color( zip_type_stream		 stream, double			 red , double			 green , double			 blue )
        {
  int			      status = zip_ok;

     IN(zip_Set_stream_FillBG_Color);
     if ( stream )
      {
	  if ( stream->zip_stream_color_values == NULL )
	    stream->zip_stream_color_values = (this )->Allocate_Color_Values( );
          if ( stream->zip_stream_color_values &&
	     ( stream->zip_stream_color_values->fillbg == NULL ))
	    stream->zip_stream_color_values->fillbg = (this )->Allocate_Color( );
 	  if (  stream->zip_stream_color_values && stream->zip_stream_color_values->fillbg )
            {
  	      stream->zip_stream_color_values->fillbg->red = red;
  	      stream->zip_stream_color_values->fillbg->green = green;
  	      stream->zip_stream_color_values->fillbg->blue = blue;
	    }
	  else status = zip_failure;
      }
      else
	  status = zip_stream_non_existent;
      ZIP_STATUS(this);
      OUT(zip_Set_Stream_FillBG_Color);
      return status;
  }

long
zip::Set_Stream_Text( zip_type_stream	       stream, char			      *text )
        {
  int			      status = zip_ok;

  IN(zip_Set_Stream_Text);
  if ( stream  &&  text )
    {
    DEBUGst(Given Text,text);
    DEBUGst(Stream Name,stream->zip_stream_name);
    stream->zip_stream_text = text;
    }
    else
    {
/*===*/
    }
  ZIP_STATUS(this);
  OUT(zip_Set_Stream_Text);
  return status;
  }

long
zip::Set_Stream_Font( zip_type_stream	       stream, const char			      *font_name )
        {
  int			      status = zip_ok;
  short				      font;

  IN(zip_Set_Stream_Font);
  if ( stream  &&  font_name  &&  (this)->Define_Font(  font_name, &font ) )
    {
    DEBUGst(Given Font,font);
    DEBUGst(Stream Name,stream->zip_stream_name);
    stream->zip_stream_font = font;
    }
    else
    {
/*===*/
    }
  ZIP_STATUS(this);
  OUT(zip_Set_Stream_Font);
  return status;
  }

long
zip::Set_Stream_Source( zip_type_stream	       stream, const char			      *source )
        {
  int			      status = zip_ok;
  char				      temp_name[257];
  FILE			     *temp_file;

  IN(zip_Set_Stream_Source);
  if ( stream  &&  source )
    {
    DEBUGst(Stream Name,stream->zip_stream_name);
/*=== deallocate any prior objects ===*/
    stream->zip_stream_image_anchor->zip_image_inferior = NULL;
    strcpy(temp_name, "/tmp/ZIPXXXXXX");
    strcat(mktemp(temp_name), ".zip");
    if ( ( temp_file = fopen( temp_name, "w+" ) ) )
      { DEBUG(Open OK);
      if ( fwrite( source, 1, strlen( source ), temp_file ) )
	{ DEBUG(Write OK);
	rewind( temp_file );
	stream->zip_stream_file = temp_file;
	status = zip_Deparse_Stream( this, stream );
	}
	else
	{ DEBUG(Write ERROR);
	status = zip_failure;
	}
      fclose( temp_file );
      }
      else
      { DEBUG(Open ERROR);
      status = zip_failure;
      }
    unlink( temp_name );
    }
    else
    {
/*===*/
    }
  ZIP_STATUS(this);
  OUT(zip_Set_Stream_Source);
  return status;
  }

zip_type_stream
zip::Stream( char			      *stream_name )
      {
  zip_type_stream	      stream = NULL;
  zip_type_stream_chain      stream_link = StreamAnchor;
  int			      status = zip_ok;

  IN(zip_Stream);
  while ( stream_link )
    {
    if ( stream_link->zip_stream_chain_ptr->zip_stream_name )
      { DEBUGst(Looking at Stream-named,stream_link->zip_stream_chain_ptr->zip_stream_name); }
    if ( stream_link->zip_stream_chain_ptr->zip_stream_name  &&
	 apt_MM_Compare( stream_link->zip_stream_chain_ptr->zip_stream_name, stream_name ) == 0 )
      {
      DEBUGst(Found Named-Stream,stream_link->zip_stream_chain_ptr->zip_stream_name);
      stream = stream_link->zip_stream_chain_ptr;
      goto exit_point;
      }
    stream_link = stream_link->zip_stream_chain_next;
    }
  exit_point:
  ZIP_STATUS(this);
  OUT(zip_Stream);
  return stream;
  }

long
zip::Set_Stream_Extrema( zip_type_stream	       stream, zip_type_image	       image )
        {
  IN(zip::Set_Stream_Extrema);
/*===7/18/86  zip_Balance_Image_Extrema( image );===*/
  if ( image->zip_image_least_x < stream->zip_stream_least_x )
    {
    stream->zip_stream_least_x = image->zip_image_least_x;
    DEBUGdt(Least    X,image->zip_image_least_x );
    }
  if ( image->zip_image_greatest_x > stream->zip_stream_greatest_x )
    { 
    stream->zip_stream_greatest_x = image->zip_image_greatest_x;
    DEBUGdt(Greatest X,image->zip_image_greatest_x );
    }
  if ( image->zip_image_least_y < stream->zip_stream_least_y )
    {
    stream->zip_stream_least_y = image->zip_image_least_y;
    DEBUGdt(Least    Y,image->zip_image_least_y );
    }
  if ( image->zip_image_greatest_y > stream->zip_stream_greatest_y )
    {
    stream->zip_stream_greatest_y = image->zip_image_greatest_y;
    DEBUGdt(Greatest Y,image->zip_image_greatest_y );
    }

  /* Balance Stream X/Y Extremes around X/Y Origin */
  if ( abs(stream->zip_stream_least_x) < abs(stream->zip_stream_greatest_x) )
      stream->zip_stream_least_x    = -abs(stream->zip_stream_greatest_x);
      else
      stream->zip_stream_greatest_x =  abs(stream->zip_stream_least_x);
  if ( abs(stream->zip_stream_least_y) < abs(stream->zip_stream_greatest_y) )
      stream->zip_stream_least_y    = -abs(stream->zip_stream_greatest_y);
      else
      stream->zip_stream_greatest_y =  abs(stream->zip_stream_least_y);
  OUT(zip::Set_Stream_Extrema);
  return zip_ok;
  }

static
void Extract_Stream_Name( class zip		      *self, const char			      *name, char			     **stream_name )
        {
  const char			     *start_ptr, *end_ptr;

  IN(Extract_Stream_Name);
  DEBUGst(Name,name);
  if ( (long)(start_ptr = (char *)(rindex( name, '/' ) + 1)) == 1 )
    start_ptr = name;
  DEBUGst(Start-ptr,start_ptr);
  end_ptr = name + strlen( name );
  DEBUGst(End-ptr,end_ptr);
  if ( apt_MM_Compare( (end_ptr - (sizeof ".zip" - 1)), ".zip" ) == 0 )
    {
    end_ptr -= (sizeof ".zip" - 1);
    DEBUGst(End-ptr of .zip,end_ptr);
    }
  *stream_name = (char *) malloc( (end_ptr - start_ptr) + 1 );
  strncpy( *stream_name, start_ptr, end_ptr - start_ptr );
  *((*stream_name) + (end_ptr - start_ptr) ) = '\0';
  DEBUGst(Stream_name, *stream_name );
  OUT(Extract_Stream_Name);
  }

static
void Extract_Stream_File_Name( class zip		      *self, const char			      *name, char			     **file_name )
        {
  const char			     *start_ptr, *end_ptr;

  IN(Extract_Stream_File_Name);
  DEBUGst(Name,name);
  if ( (long)(start_ptr = (char *)(rindex( name, '/' ) + 1)) == 1 )
    start_ptr = name;
  end_ptr = name + strlen( name );
  *file_name = (char *) malloc( (end_ptr - start_ptr) + 5 );
  strcpy( *file_name, start_ptr );
/*===
  if ( apt_MM_Compare( (end_ptr - (sizeof ".zip" - 1)), ".zip" ) != 0 )
    {
    strcat( *file_name, ".zip" );
    DEBUG( Appended .zip);
    }
===*/
  DEBUGst(File_name, *file_name );
  OUT(Extract_Stream_File_Name);
  }

static
void Extract_Stream_File_Path( class zip		      *self, const char			      *name, char			     **path_name )
        {
  const char			     *start_ptr, *end_ptr;

  IN(Extract_Stream_File_Path);
  DEBUGst(Name,name);
  *path_name = (char *) malloc( 257 );
  *(*path_name) = '\0';
  start_ptr = name;
  if ( (long)(end_ptr = (char *)(rindex( name, '/' ))) != 0 )
    {
    strncpy( *path_name, start_ptr, end_ptr - start_ptr );
    *((*path_name) + (end_ptr - start_ptr) ) = '\0';
    }
  DEBUGst(Path_name, *path_name );
  OUT(Extract_Stream_File_Path);
  }

int
zip_Open_Stream_File( class zip		      *self, zip_type_stream	       stream, long			       open_mode )
        {
  int			      status = zip_ok;
  const char		     *open_mode_flags = 0;

  IN(zip_Open_Stream_File);
  DEBUGxt(Mode,open_mode);
  if ( open_mode == zip_write )	
    {
    open_mode_flags = "w";
    stream->zip_stream_file_states.zip_stream_file_state_open_write = true;
    }
  else if ( open_mode == zip_update )
    {	
    open_mode_flags = "r+w";
    stream->zip_stream_file_states.zip_stream_file_state_open_update = true;
    }
  else if ( (open_mode == zip_read)  ||  (open_mode_flags == 0) )
    {
    open_mode_flags = "r";
    stream->zip_stream_file_states.zip_stream_file_state_open_read = true;
    }
  else  open_mode_flags = "r";
  DEBUGst(Open-mode-flags,open_mode_flags);
  DEBUGst(A -- stream-name           ,stream->zip_stream_name);
  DEBUGst(A -- stream-file-name      ,stream->zip_stream_file_name);
  DEBUGst(A -- stream-file-path      ,stream->zip_stream_file_path);
  DEBUGst(A -- stream-file-full-name ,stream->zip_stream_file_full_name);

  errno = 0;
  if ( (stream->zip_stream_file =
          fopen( stream->zip_stream_file_full_name, open_mode_flags )) == NULL  )
    {
    DEBUGdt(ERRNO,errno);
    if ( errno == ENOENT  &&  stream->zip_stream_file_path[0] == '\0' )
      status = Open_Via_Alternate_Paths( self, stream, open_mode_flags );
      else  status = zip_system_status_value_boundary + errno;
    }
  DEBUGdt(ERRNO,errno);
  if ( status != zip_ok )
    Reset_Stream_File_Open_States( self, stream );
  DEBUGdt(Status,status);
  OUT(zip_Open_Stream_File);
  return status;
  }

static int
Open_Via_Alternate_Paths( class zip *self, zip_type_stream stream, const char  *mode )
{
  int			      status = zip_ok;
  int			      i;

  IN(Open_Via_Alternate_Paths);
  if ( self->paths == NULL )
    status = Identify_Paths( self, &self->paths );
  if ( status == zip_ok )
    {
    status = -1;
    for ( i = 0; status == -1  &&  i < self->paths->zip_paths_count; i++ )
      {
      status = Open_Alternate( self, stream, mode, self->paths->zip_paths_vector[i] );
      }
    if ( status == -1 )
      {
      strcpy( stream->zip_stream_file_full_name, stream->zip_stream_file_name );
      status = zip_system_status_value_boundary + errno;
      }
    }
  OUT(Open_Via_Alternate_Paths);
  return status;
  }

static int
Open_Alternate( class zip		      *self, zip_type_stream	       stream, const char  *			      open_mode_flags, const char			      *path )
          {
  int			      status = -1;

  IN(Open_Alternate);

  strcpy( stream->zip_stream_file_full_name, path );
  strcat( stream->zip_stream_file_full_name, "/" );
  strcat( stream->zip_stream_file_full_name, stream->zip_stream_file_name );
  DEBUGst(B -- stream-name           ,stream->zip_stream_name);
  DEBUGst(B -- stream-file-name      ,stream->zip_stream_file_name);
  DEBUGst(B -- stream-file-path      ,stream->zip_stream_file_path);
  DEBUGst(B -- stream-file-full-name ,stream->zip_stream_file_full_name);
  if ( ( stream->zip_stream_file =
              fopen( stream->zip_stream_file_full_name, open_mode_flags ) ) )
    {
    DEBUGst(Successful open of, stream->zip_stream_file_full_name );
    strcpy( stream->zip_stream_file_path, path );
    DEBUGst(Path is, stream->zip_stream_file_path );
    status = zip_ok;
    }
    else
    if ( errno != ENOENT )
      status = zip_system_status_value_boundary + errno;
  OUT(Open_Alternate);
  return status;
  }

int
zip_Close_Stream_File( class zip		      *self, zip_type_stream	       stream )
      {
  IN(zip_Close_Stream_File);
  if ( stream->zip_stream_file )
    fclose( stream->zip_stream_file );
  stream->zip_stream_file = NULL;
  OUT(zip_Close_Stream_File);
  return zip_ok;
  }

static
void Reset_Stream_File_Open_States( class zip		      *self, zip_type_stream	       stream )
      {
/*===*/
  }
/*===
static int
Stream_File_Exists( self, stream_name )
  struct zip		     *self;
  char			     *stream_name;
  {
  struct stat			      st;
  char				      full_name[257];
  int			      status = true;

  IN(Stream_File_Exists);
  strcpy( full_name, stream_name );
  strcat( full_name, ".zip" );
  if ( stat( full_name, &st ) != 0 )
    status = false;
  OUT(Stream_File_Exists);
  return status;
  }
===*/
static int
Identify_Paths( class zip		      *self, zip_type_paths	      *paths_ptr )
      {
  int			      new_path = 1;
  char				     *zippath_profile, *zippath_string;
  int			      status = zip_ok;

  IN(Identify_Paths);
  if ( (*paths_ptr = (zip_type_paths)
        calloc( 1, sizeof(struct zip_paths) + (100 * sizeof(zip_type_path)) )) != NULL )
    {
    (*paths_ptr)->zip_paths_vector[0] = ".";
    (*paths_ptr)->zip_paths_count = 1;
    if ( (zippath_profile = (char *) environ::GetProfile( "zippaths" ))  ||
         (zippath_profile = (char *) environ::GetProfile( "zippath" )) )
      {
      if ( (zippath_string = strdup( zippath_profile ) ) != NULL )
        {
        zippath_string--;
        while ( *++zippath_string != '\0'  &&  (*paths_ptr)->zip_paths_count < 100 )
          {
          if ( new_path  &&  *zippath_string != ':' )
            {
            (*paths_ptr)->zip_paths_vector[(*paths_ptr)->zip_paths_count++] =
              zippath_string;
            new_path = false;
	    }
          if ( *zippath_string  == ':' )
            {
	    *zippath_string = '\0';
            new_path = true;
            }
          }
	}
	else status = zip_insufficient_stream_space;
      }
    if ( ( (*paths_ptr)->zip_paths_vector[(*paths_ptr)->zip_paths_count] =
         strdup(zip_default_path ) ) )
      (*paths_ptr)->zip_paths_count++;
      else status = zip_insufficient_stream_space;
    if ( ( (*paths_ptr)->zip_paths_vector[(*paths_ptr)->zip_paths_count] =
         strdup( zip_default_path_alternate ) ) )
      (*paths_ptr)->zip_paths_count++;
      else status = zip_insufficient_stream_space;
    }
    else status = zip_insufficient_stream_space;
  OUT(Identify_Paths);
  return status;
  }
/*===
static int
Set_Paths( self, paths )
  struct zip		     *self;
  zip_type_paths	      paths;
  {
  IN(Set_Paths);
  Paths = paths;
  OUT(Set_Paths);
  return zip_ok;
  }

static int
Reset_Paths( self )
  struct zip		     *self;
  {
  IN(Reset_Paths);
  Paths = NULL;
  OUT(Reset_Paths);
  return zip_ok;
  }
===*/

static int
Allocate_Stream_Object( class zip		      *self, zip_type_stream	      *stream, const char			      *name )
        {
  int			      status = zip_ok;
  zip_type_stream_chain      stream_link;

  IN(Allocate_Stream_Object);
  DEBUGst(Name,name);
  if (
	(*stream = (zip_type_stream) calloc( 1, sizeof(struct zip_stream))) == NULL
	||
	(stream_link = (zip_type_stream_chain) calloc( 1, sizeof(struct zip_stream_chain))) == NULL
     )
    status = zip_insufficient_stream_space;
    else
    {
    stream_link->zip_stream_chain_next = self->stream_anchor;
    stream_link->zip_stream_chain_ptr  = *stream;
    self->stream_anchor = stream_link;
    zip_Set_Stream_File_Name( self, *stream, name );
    (self)->Set_Stream_Line_Width(  *stream, 1 );
    (self)->Set_Stream_Line_Cap(  *stream, graphic_CapButt );
    (self)->Set_Stream_Line_Join(  *stream, graphic_JoinMiter );
    symtab_create( &(*stream)->zip_stream_symbol_table, 100 );
    }
  OUT(Allocate_Stream_Object);
  return status;
  }

int
zip_Set_Stream_File_Name( class zip		      *self, zip_type_stream	       stream, const char			      *name )
        {
  long			      status = zip_ok;

  IN(zip_Set_Stream_File_Name);
  DEBUGst(Name,name);
  if ( name  &&  *name )
    {
    Extract_Stream_Name( self, name, &stream->zip_stream_name );
    Extract_Stream_File_Name( self, name, &stream->zip_stream_file_name );
    Extract_Stream_File_Path( self, name, &stream->zip_stream_file_path );
    stream->zip_stream_file_full_name = (char *) malloc( 259 );
    stream->zip_stream_file_full_name[0] = 0;
    if ( stream->zip_stream_file_path[0] )
      {
      strcpy( stream->zip_stream_file_full_name, stream->zip_stream_file_path );
      strcat( stream->zip_stream_file_full_name, "/" );
      }
    strcat( stream->zip_stream_file_full_name, stream->zip_stream_file_name );
    DEBUGst(Full-name, stream->zip_stream_file_full_name );
    Reset_Stream_File_Open_States( self, stream );
    }
  OUT(zip_Set_Stream_File_Name);
  return  status;
  }


static int
Deallocate_Stream_Object( class zip *self, zip_type_stream stream )
      {
  int			      status = zip_ok;
  zip_type_stream_chain      stream_link, prior_link;

  IN(Deallocate_Stream_Object);
  if ( stream->zip_stream_name )
    free( stream->zip_stream_name );
  stream->zip_stream_name = NULL;

  if ( stream->zip_stream_file_name )
    free( stream->zip_stream_file_name );
  stream->zip_stream_file_name = NULL;

  if ( stream->zip_stream_file_path )
    free( stream->zip_stream_file_path );
  stream->zip_stream_file_path = NULL;

  if ( stream->zip_stream_file_full_name )
    free( stream->zip_stream_file_full_name );
  stream->zip_stream_file_full_name = NULL;

  if ( stream->zip_stream_text )
    free( stream->zip_stream_text );
  stream->zip_stream_text = NULL;

  Deallocate_Stream_Resources( self, stream );

  stream_link = self->stream_anchor;
  prior_link = (zip_type_stream_chain) &(self->stream_anchor);
  while ( stream_link )
    {
    if ( stream_link->zip_stream_chain_ptr == stream )
      {
      prior_link->zip_stream_chain_next = stream_link->zip_stream_chain_next;
      break;
      }
    prior_link = stream_link;
    stream_link = stream_link->zip_stream_chain_next;
    }
  free( stream_link );
  free( stream );
  OUT(Deallocate_Stream_Object);
  return status;
  }

static int
Deallocate_Stream_Resources( class zip		      *self, zip_type_stream	       stream )
      {
  int			      status = zip_ok;

  IN(Deallocate_Stream_Resources);
  if ( stream->zip_stream_image_anchor )
    (self)->Destroy_Image(  stream->zip_stream_image_anchor );
  symtab_destroy( stream->zip_stream_symbol_table );
  stream->zip_stream_symbol_table = NULL;
  stream->zip_stream_object_pool = NULL;
  OUT(Deallocate_Stream_Resources);
  return status;
  }

