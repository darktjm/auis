/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
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

/* zip.c	Zip Data-object					      */
/* Author	TC Peters					      */
/* Information Technology Center	   Carnegie-Mellon University */


/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Zip Data-object

MODULE	zip.c

NOTICE	IBM Internal Use Only

DESCRIPTION
	This is the suite of Methods that support the Zip Data-object.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...

HISTORY
  06/16/87	Created (TCP)
  08/06/87	Once more to the breach, dear friends ...
  10/30/87	return long instead of void for zip::Read (TCP)
  11/09/87	Support "Absolute" as well as "Relative" sizing (TCP)
  12/09/87	Fixed reading code to be able to recognize EOF when reading.
		Also changed fgetc to getc. (ajp)
  03/31/88	Revised to ATK (TCP)
  03/16/89	Accomodate short file-names for objects (TCP)
  08/07/89	Override GetModified to check for changes to Imbedded objects (TCP)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include <environ.H>
#include <dataobject.H>
#include <zip.H>
#include <zipobject.H>
#include <sys/stat.h>

static boolean debug;

#define	 Objects(s,i)		      (((s)->objects)[i])


static const char   * const object_names[] = {
/* NULL	       */ "zipobject",
/* A Caption   */ "zipocapt",
/* B FlexCapt  */ "zipofcapt",
/* C Line      */ "zipoline",
/* D PolyLine  */ "zipoplin",
/* E Polygon   */ "zipopolygon",
/* F Trapezoid */ "zipotrap",
/* G Rectangle */ "ziporect",
/* H Path      */ "zipopath",
/* I Imbed     */ "zipoimbed",
/* J Circle    */ "zipocirc",
/* K Photo     */ "zipobject",
/* L Ellipse   */ "zipoelli",
/* M Arc       */ "zipoarc",
/* N RoundAngle*/ "ziporang",
/* O Arrow     */ "zipoarrow",
/* P Symbol    */ "ziposymbol",
NULL,
/* Q	       */ "zipobject",
/* R           */ "zipobject",
/* S           */ "zipobject",
/* T           */ "zipobject",
/* U           */ "zipobject",
/* V           */ "zipobject",
/* W           */ "zipobject",
/* X           */ "zipobject",
/* Y           */ "zipobject",
/* Z           */ "zipobject",
NULL
};


ATKdefineRegistry(zip, dataobject, zip::InitializeClass);
static long Check_Image( class zip		      *self, zip_type_image	       image, long			       modified );
static void Write_View_Info( class zip		      *self, FILE			      *file );
static void Write_Object_Info( class zip		      *self, FILE			      *file );
void Write_PrintSize_Info( class zip		      *self, FILE			      *file );
static long Generate_Temp_File( class zip		      *self, FILE			      *file, char			     **generated_file_name );
static long Init_Message_Writer( class zip		      *self, char			      *msg );
static long Init_Message_Clearer( class zip		      *self );
static long Init_Message_Acknowledger( class zip		      *self, char			      *msg );

boolean
zip::InitializeClass( )
    {
  IN(zip_InitializeClass);
  debug = 1;
  OUT(zip_InitializeClass);
  return TRUE;
  }










zip::zip( )
      {
	ATKinit;

  long			      status = zip_ok;
  const char			     *font_name = NULL;
  int			      i;

  IN(zip_InitializeObject);
  STREAM = NULL;
  StreamFileName = NULL;
  StreamAnchor = NULL;
  ImageAnchor = NULL;
  FigureAnchor = NULL;
  Paths = NULL;
  Fonts = NULL;
  STATUS = StatusAddenda = 0;
  Facility = NULL;
  generalExceptionHandler = 0;
  StreamExceptionHandler = 0;
  ImageExceptionHandler = 0;
  FigureExceptionHandler = 0;
  Id = 0;
  PageCount = 0;
  DesiredWidth = DesiredHeight = ObjectWidth = ObjectHeight = 0;
  MessageWriter  = (long (*)(class zip *, char *)) Init_Message_Writer;
  MessageClearer = (long (*)(class zip *)) Init_Message_Clearer;
  MessageAcknowledger = (long (*)(class zip *, char *)) Init_Message_Acknowledger;
  if ( (font_name = (char *) environ::GetProfile( "zip.bodyfont" )) == NULL )
      font_name = "andy12";
  (this)->Define_Font(  font_name, NULL );
  this->objects = (class zipobject **) calloc( 28, sizeof(class zipobject *) );
  for ( i = 0; object_names[i]  &&  status == zip_ok; i++ )
    {
    DEBUGst(Loading,object_names[i]);
    Objects(this,i) = (class zipobject *) ATK::NewObject( object_names[i] );
    if ( Objects(this,i) )
      {
      (Objects(this,i))->Set_Data_Object( this );
      }
      else
      {
      DEBUG(NewObject FAILED);
/*===*/status = zip_failure; /*===s/b Missing Object===*/
      }
    }

  /* SET DEFAULT PRINT SIZE */

  this->def_inch_width = 7.5;
  this->def_inch_height = 5.0;
  this->size_from_file = 0;	/* not set from file */
  DEBUGgt(Default Inch Width, this->def_inch_width);
  DEBUGgt(Default Inch Height, this->def_inch_height);

  OUT(zip_InitializeObject);
  THROWONFAILURE( (status == zip_ok));
  }

zip::~zip( )
      {
	ATKinit;

  IN(zip_FinalizeObject);
/*===*/
  OUT(zip_FinalizeObject);
  }

void 
zip::Set_Debug( boolean state )
      {
  long i;

  IN(zip_Set_Debug);
  debug = state;
  if ( this->objects )
    for ( i = 0; (i < 26); i++ )
      if ( Objects(this,i) )
        (Objects(this,i))->Set_Debug( state );
  OUT(zip_Set_Debug);
  }

static long
Check_Image( class zip *self, zip_type_image image, long modified )
        {
  zip_type_figure figure;

  IN(Check_Image);
  if ( image )
    {
    figure = image->zip_image_figure_anchor;
    while ( figure )
      {
      if ( (Objects(self,figure->zip_figure_type))->Object_Modified( figure ) > modified )
	modified = (Objects(self,figure->zip_figure_type))->Object_Modified( figure );
      figure = figure->zip_figure_next;
      }
    if ( image->zip_image_inferior )
      modified = Check_Image( self, image->zip_image_inferior, modified );
    if ( image->zip_image_peer )
      modified = Check_Image( self, image->zip_image_peer, modified );
    }
  OUT(Check_Image);
  return  modified;
  }

long
zip::GetModified( )
    {
  long			      modified = (this )->dataobject::GetModified( );

  IN(zip_GetModified);
  if ( STREAM )
    modified = Check_Image( this, STREAM->zip_stream_image_anchor, modified );
  OUT(zip_GetModified);
  return  modified;
  }

long
zip::Read( FILE			      *file, long			       id )
        {
  long			      status;
  char				     *generated_file_name;

  IN(zip_Read);
  if ( (status = Generate_Temp_File( this, file, &generated_file_name )) == zip_ok )
    {
    if ( ( status = (this)->Open_Stream(  &STREAM, generated_file_name, zip_default ) ) )
      { DEBUGdt(Open Status,status);
      status = dataobject_BADFORMAT;
      }
    if ( ( status = (this)->Read_Stream(  STREAM ) ) )
      { DEBUGdt(Read Status,status);
      status = dataobject_BADFORMAT;
      }
    unlink( generated_file_name );
    }
  OUT(zip_Read);
  return status;
  }

extern long zip_Enparse_Stream(class zip *self, struct zip_stream *stream);

long
zip::Write( FILE			      *file, long			       id, int 			       level )
          {
  long UNUSED		      status; /* used only if DB=1 */

  IN(zip_Write);
  DEBUGdt( Headerwriteid, this->writeID );
  DEBUGdt( Given Id, id );
  DEBUGdt( Given Level, level );
  WriteStreamFile = file;
  WriteStreamId = id;
  WriteStreamLevel = level;
  if ( this->writeID != id ) /*===???avoid recursive writes */
    {
    this->writeID = id;
    if ( level )
      { DEBUG(Not Parent -- Write To Datastream);
      fprintf( file, "\\begindata{%s,%ld}\n",
		(this)->GetTypeName( ),
		(this)->UniqueID( ) );
      Write_View_Info( this, file );
      Write_Object_Info( this, file );
      Write_PrintSize_Info( this, file );
      if ( STREAM->zip_stream_attributes.zip_stream_attribute_reference )
	{ DEBUG(Write File-reference);
	fprintf( file, "!%s", STREAM->zip_stream_file_full_name );
	if ( STREAM->zip_stream_states.zip_stream_state_modified )
          status = (this)->Write_Stream(  STREAM );
	}
	else
	{ DEBUG(Write File-substance);
	STREAM->zip_stream_file = file;
        status = zip_Enparse_Stream( this, STREAM );
	}
      DEBUGdt(Status,status);
      fprintf( file, "\n\\enddata{%s,%ld}\n",
		(this )->GetTypeName( ),
		(this)->UniqueID( ) );
      }
      else
      { DEBUG(Parent -- Write To Raw File);
      Write_Object_Info( this, file );
      Write_PrintSize_Info( this, file );
      STREAM->zip_stream_file = file;
      status = zip_Enparse_Stream( this, STREAM );
      DEBUGdt(Enparse_Stream Status,status);
      }
    }
  OUT(zip_Write);
  return (long) this;
  }

static
void Write_View_Info( class zip *self, FILE *file )
{
  if ( self->desired_view_width )
    fprintf( file, "%%ViewWidth %ld\n", self->desired_view_width );
  if ( self->desired_view_height )
    fprintf( file, "%%ViewHeight %ld\n", self->desired_view_height );
}

static
void Write_Object_Info( class zip *self, FILE *file )
      {
  if ( self->object_width )
    fprintf( file, "%%ObjectWidth %ld\n", self->object_width );
  if ( self->object_height )
    fprintf( file, "%%ObjectHeight %ld\n", self->object_height );
  }

void Write_PrintSize_Info( class zip *self, FILE *file )
{
    /* Encode print size as a presentation parameter */

    fprintf( file, "%%PrintSize %.2fx%.2f\n",
	     self->def_inch_width,
	     self->def_inch_height);
}


static
long Generate_Temp_File( class zip		      *self, FILE			      *file, char			     **generated_file_name )
        {
  long			      status = dataobject_NOREADERROR;
  static char			      temp_name[512];
  FILE			     *temp_file;
  long			      level = 0;
  char				      line[32000];


  IN(Generate_Temp_File);
  strcpy(temp_name, "/tmp/ZIPXXXXXX");
  strcat(mktemp(temp_name), ".zip");
  DEBUGst(Temp-Name,temp_name);
  *generated_file_name = temp_name;
  if ( (temp_file = fopen( temp_name, "w+" )) == NULL )
    { DEBUG(Open Temp-File ERROR);
    status = zip_failure;
    }
    else
    { DEBUG(Open Temp-File OK);
    while ( level >= 0 )
      {
      if ( fgets( line, sizeof( line ) - 1, file ) )
	{ DEBUGst(Line,line);
        if ( strncmp( line, "\\begindata", 10 ) == 0 )
  	  level++;
        else
        if ( strncmp( line, "\\enddata", 8 ) == 0 )
	  level--;
        if ( level >= 0 )
          fputs( line, temp_file );
	}
	else
	{ DEBUG(EOF);
	if ( level > 0 )
	  status = dataobject_PREMATUREEOF;
	break;
	}
      }
    }
  fclose( temp_file );
  OUT(Generate_Temp_File);
  return  status;
  }

void zip::Show_Statistics( int options )
      {
  zip_type_stream_chain	  stream_chain;
  zip_type_stream		  stream;
  zip_type_image		  image;
  zip_type_figure		  figure;
  int				  bytes,
					  figure_count = 0,
					  image_count = 0,
					  stream_count = 0,
					  stream_byte_count = 0,
					  image_byte_count = 0,
					  figure_byte_count = 0;
/*===  if ( options ... ) ===*/
    {

      {
      stream_chain = StreamAnchor;
      while ( stream_chain )
	{
	stream_count++;
	stream = stream_chain->zip_stream_chain_ptr;
	stream_byte_count += sizeof(struct zip_stream);
	if ( (this)->Stream_Name(  stream ) )
	  stream_byte_count += strlen( (this)->Stream_Name(  stream ) );
	image = stream->zip_stream_image_anchor;
	while ( image )
	  {
	  image_count++;
	  bytes = sizeof(struct zip_image);
	  if ( (this)->Image_Name(  image ) )
	    bytes += strlen( (this)->Image_Name(  image ) );
	  image_byte_count += bytes;
	  stream_byte_count += bytes;
	  figure = image->zip_image_figure_anchor;
	  while ( figure )
	    {
	    figure_count++;
	    bytes = sizeof(struct zip_figure) +
		((figure->zip_figure_points) ?
		  (figure->zip_figure_points->zip_points_count *
			 sizeof(struct zip_point_pair))
		    :
		   0);
	    if ( (this)->Figure_Name(  figure ) )
	      bytes += strlen( (this)->Figure_Name(  figure ) );
	    figure_byte_count += bytes;
	    stream_byte_count += bytes;
	    figure = figure->zip_figure_next;
	    }
	  image = (this)->Next_Image( image );
	  }
	printf( "Stream '%s' (%d Bytes)\n\t%d Images ( %d Bytes)\n\t%d Figures  (%d Bytes)\n",
		 (this)->Stream_Name(  stream ), stream_byte_count,
		 image_count, image_byte_count,
		 figure_count, figure_byte_count );
        stream_chain = stream_chain->zip_stream_chain_next;
	}
      }
    }
  }

static long
Init_Message_Writer( class zip *self, char  *msg )
      {
/*===  apt_Announce( msg );===*/
  return zip_success;
  }

static long
Init_Message_Clearer( class zip		      *self )
    {
/*===  apt_Unannounce();===*/
  return zip_success;
  }

static long
Init_Message_Acknowledger( class zip		      *self, char			      *msg )
      {
/*===
  while ( apt_Acknowledge( msg ) == -1 )
	;
===*/
  return zip_success;
  }
/*=== === ===*/
