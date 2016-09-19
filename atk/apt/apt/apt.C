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

/*
    $Log: apt.C,v $
 * Revision 1.6  1995/11/07  20:17:10  robr
 * OS/2 port
 *
 * Revision 1.5  1994/11/30  20:42:06  rr2b
 * Start of Imakefile cleanup and pragma implementation/interface hack for g++
 *
 * Revision 1.4  1994/03/21  16:50:19  rr2b
 * Fixed to use memset in place of bzero
 * BUILD
 *
 * Revision 1.3  1993/05/18  15:17:54  rr2b
 * Corrected for the proper type of the strings member of the apt_strings structure.
 * (It was a *(*)[], but it is really just a **.)
 *
 * Revision 1.2  1993/05/13  14:04:38  rr2b
 * Fixed char *((*)[]) types to be char ***.
 * Added class apt *self=this, for macros which depend on self.
 * Removed ::s from local variables.
 * Added use of apt_readfptr and apt_writefptr for ReadObject and WriteObject.
 * Removed bogus occurrence of &s.
 * Fixed conversion of reference to the object's UniqueID.
 * Fixed bogus occurences of *.
 *
 * Revision 1.12  1992/12/15  21:25:24  rr2b
 * more disclaimerization fixing
 *
 * Revision 1.11  1992/12/14  20:33:21  rr2b
 * disclaimerization
 *
 * Revision 1.10  1992/08/31  23:32:41  rr2b
 * renaming debug var to avoid name clashes.
 * .
 *
 * Revision 1.9  1991/09/12  15:56:20  bobg
 * Update copyright notice and rcsid
 *
 * Revision 1.8  1991/09/09  23:32:44  gk5g
 * Added forward declarations.
 *
 * Revision 1.7  1990/04/11  14:10:12  gk5g
 * Removed initialization of variable debug from apt.h and put it in apt.c:apt__InitializeClass.  Create apt__InitializeClass.
 *
 * Revision 1.6  89/12/12  14:57:47  ghoti
 * sync with MIT tape
 * 
 * Revision 1.2  89/11/28  16:05:37  xguest
 * Added Gary Keim's diffs.
 * 
 * Revision 1.1  89/11/28  15:51:35  xguest
 * Initial revision
 * 
 * Revision 1.5  89/08/30  12:24:30  gk5g
 * Removed include of andrewos.h because apt.h does that.
 * 
 * Revision 1.4  89/08/03  12:25:32  ghoti
 * added include of andrewos.h (HPUX)
 * changed #inlcude "" to #include <>
 * 
 * Revision 1.3  89/05/24  19:46:11  tom
 * Fix Free_Vector; use apt_LeftArea, etc.
 * 
 * Revision 1.2  89/05/18  20:32:44  tom
 * Utilize apts.c (apts_CompareStrings).
 * 
 * Revision 1.1  89/04/28  17:45:51  tom
 * Initial revision
 * 
*/

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Apt Data-object

MODULE	apt.c

VERSION	0.0

AUTHOR	TC Peters
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the suite of Methods that support the Apt Data-object.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  02/23/88	Created (TCP)
  05/18/89	Utilize apts facility apts_CompareStrings (TCP)
  05/22/89	Fix Free_Vector (TCP)
		Use apt_LeftArea, etc.
   08/30/89	Removed include of andrewos.h because that is in apt.h. (GW Keim)

END-SPECIFICATION  ************************************************************/
#include <andrewos.h>
ATK_IMPL("apt.H")
#include <dataobject.H>
#include <apt.H>
#include <apts.H>

static int apt_debug = 0;
#define debug apt_debug

#define	 Title(p)	 	    (self->titles[p].strings)[0]
#define	 TitlesAnchor(p) 	     (self->titles[p].strings)
#define	 TitlesCount(p) 	     (self->titles[p].strings_count)
#define	 Titles(p,i)	 	    (self->titles[p].strings)[i]
#define	 TitleMode(p)	 	     (self->titles[p].mode)
#define	 TitleFontName(p)	     (self->titles[p].font_name)

#define	 Legend(p)	 	    (self->legends[p].strings)[0]
#define	 LegendsAnchor(p) 	     (self->legends[p].strings)
#define	 LegendsCount(p) 	     (self->legends[p].strings_count)
#define	 Legends(p,i)	 	    (self->legends[p].strings)[i]
#define	 LegendMode(p)	 	     (self->legends[p].mode)
#define	 LegendFontName(p)	     (self->legends[p].font_name)

#define  Id			      self->id

#define  Fields			      self->fields
#define  FieldsFile		      self->field_file
#define  FieldIndex		      self->field_index
#define  Field(i)		      self->fields->field[i]
#define  FieldCount		      self->fields->count
#define  FieldName(i)		      self->fields->field[i].name
#define  FieldContent(i)	      self->fields->field[i].content


static const char			     * const areas[] =
				     { "Left", "Top", "Right", "Bottom" };








ATKdefineRegistry(apt, dataobject, apt::InitializeClass);
static void Free_Vector( char **vector );
static long Assign_String( class apt		      *self, const char			      *prefix , const char			      *desire , const char			      *candidate , const char			      *source , char			      **target );
static long Assign_Strings( class apt *self, const char *prefix , const char *desire , const char *candidate , const char *source, char ***target, long *count );
static void Write_Strings( class apt		      *self, FILE			      *file, const char			      *name , const char			      *prefix, long			       count, const char			      * const anchor[] );
static void Parse_Field( class apt		      *self, const char			      *line, struct apt_field	      *field );


static
void Free_Vector( char **vector )
    {
  long			      i = 0;

  IN(Free_Vector);
  if ( vector )
    {
    while ( (vector)[i] )
      {
      DEBUGst(String,(vector)[i]);
      free( (vector)[i] );
      i++;
      }
    free( vector );
    }
  OUT(Free_Vector);
  }

boolean
apt::InitializeClass( )
    {
  IN(apt_InitializeClass);
  apt_debug = 0;
  OUT(apt_InitializeClass);
  return TRUE;
  }

apt::apt( )
      {
	ATKinit;
	class apt *self=this;
  IN(apt_InitializeObject);
  DEBUGst(RCSID,rcsidapt);
  memset( this->titles,  0, 4 * sizeof(struct apt_strings) );
  memset( this->legends, 0, 4 * sizeof(struct apt_strings) );
  Fields = NULL;
  OUT(apt_InitializeObject);
  THROWONFAILURE( TRUE);
  }

apt::~apt( )
      {
	ATKinit;
	class apt *self=this;

  long			       i,j;

  IN(apt_FinalizeObject);
  for(i = 0; i < 4; i++) {
      if(TitlesAnchor(i))
	  for(j = 0; j < TitlesCount(i) ; j++)
	      if(Titles(i,j)) free(Titles(i,j));
      if(LegendsAnchor(i))
	  for(j = 0; j < LegendsCount(i); j++)
	      if(Legends(i,j)) free(Legends(i,j));
      if(LegendFontName(i)) free(LegendFontName(i));
      if(TitleFontName(i)) free(TitleFontName(i));
  }
  if(Fields) {
      for(i = 0; i < FieldCount; i++) { /* allocated by apt, so safe to free */
	  if(FieldName(i)) free((char *)FieldName(i));
	  if(FieldContent(i)) free((char *)FieldContent(i));
      }
      free(Fields);
  }
  OUT(apt_FinalizeObject);
  }

void
apt::SetAreaTitle( const char			      *title, long			       area )
        {
  const char				     *titles[2];

  IN(apt_SetAreaTitle);
  titles[0] = title;
  titles[1] = NULL;
  (this)->SetAreaSpreadTitle(  titles, 1, area, apt_Center );
  OUT(apt_SetAreaTitle);
  }

void
apt::SetAreaSpreadTitle( const char			     **titles, long			       count , long			       area , long			       mode )
        {
  long			      i = 0;
  
	class apt *self=this;
  IN(apt_SetAreaSpreadTitle);
  DEBUGdt(Position,area);
  Free_Vector( TitlesAnchor(area) );
  TitlesAnchor(area) = NULL;
  TitlesCount(area) = count;
  if ( titles )
    {
    TitlesAnchor(area) = (char **)calloc( count + 1, sizeof(char *) );
    while ( i < count  &&  *titles )
      { DEBUGst(Title,*titles);
      Titles(area,i) = strdup( *titles );
      i++;
      titles++;
      }
    }
  OUT(apt_SetAreaSpreadTitle);
  }

void
apt::SetAreaTitleFontName( const char			      *font_name, long			       area )
        {
	class apt *self=this;
  IN(apt_SetAreaTitleFontName);
  if ( TitleFontName(area) )  free( TitleFontName(area) );
  TitleFontName(area) = NULL;
  if ( font_name  &&  *font_name )
    {
    DEBUGst(TitleFontName,font_name);
    TitleFontName(area) = strdup( font_name );
    }
  OUT(apt_SetAreaTitleFontName);
  }

void
apt::SetAreaLegend( const char			      *legend, long			       area )
        {
  const char				     *legends[2];

  IN(apt_SetAreaLegend);
  legends[0] = legend;
  legends[1] = NULL;
  (this)->SetAreaSpreadLegend(  legends, 1, area, apt_Center );
  OUT(apt_SetAreaLegend);
  }

void
apt::SetAreaSpreadLegend( const char			     **legends, long			       count , long			       area , long			       mode )
        {
  long			      i = 0;

	class apt *self=this;
  IN(apt_SetAreaSpreadLegend);
  Free_Vector( LegendsAnchor(area) );
  LegendsAnchor(area) = NULL;
  LegendsCount(area) = count;
  if ( legends )
    {
    LegendsAnchor(area) = (char **) calloc( count + 1, sizeof(char *) );
    while ( i < count  &&  *legends )
      {
      DEBUGst(Legend,*legends);
      Legends(area,i) = strdup( *legends );
      i++;
      legends++;
      }
    }
  OUT(apt_SetAreaSpreadLegend);
  }

void
apt::SetAreaLegendFontName( const char			      *font_name, long			       area )
        {
	class apt *self=this;
  IN(apt_SetAreaLegendFontName);
  if ( LegendFontName(area) )  free( LegendFontName(area) );
  LegendFontName(area) = NULL;
  if ( font_name  &&  *font_name )
    {
    DEBUGst(LegendFontName,font_name);
    LegendFontName(area) = strdup( font_name );
    }
  OUT(apt_SetAreaLegendFontName);
  }


struct apt_field *
apt::ReadObjectField( )
    {
  struct apt_field	     *field = NULL;

	class apt *self=this;
  IN(apt_ReadObjectField);
  if ( FieldIndex < FieldCount )
    {
    field = &Field(FieldIndex);
    FieldIndex++;
    }
  OUT(apt_ReadObjectField);
  return field;
  }

long
apt::ReadObject( FILE			      *file, long			       id, apt_readfptr reader )
          {
	class apt *self=this;
  long			      i, j, found, status = dataobject_NOREADERROR;
  char				      line[256 + 1];

  IN(apt_ReadObject);
  if ( ( Id = id ) )
    {
    Fields = (struct apt_fields *)
			malloc( sizeof(struct apt_fields) );
    /* Extract all fields */
    FieldCount = FieldIndex = i = 0;
    while ( fgets( line, 256, file ) )
      {
      DEBUGst(Line,line);
      if ( *line == '\\' )
	{
	DEBUG(BeginData | EndData Read);
	if ( strncmp( line, "\\begindata", 10 ) == 0 )
	  continue;
	break;
	}
        else
        {
	if ( *line  &&  *line != '\n' )
	  {
          Parse_Field( this, line, &Field(i) );
          FieldCount = ++i;
          Fields = (struct apt_fields *)
	    realloc( Fields, sizeof(struct apt_fields) +
		     FieldCount * sizeof(struct apt_field) );
	  }
        }
      }
    DEBUGdt(FieldCount,FieldCount);
    for ( i = 0; i < FieldCount; i++ )
      {
      found = 0;
      for ( j = 0; ! found && j < 4; j++ )
	if ( Assign_String( this, areas[j], "TitleFontName", FieldName(i),
			    FieldContent(i), &TitleFontName(j) ) )
	  found++;
      for ( j = 0; ! found && j < 4; j++ )
	if ( Assign_Strings( this, areas[j], "Title", FieldName(i),
			    FieldContent(i), &TitlesAnchor(j), &TitlesCount(j) ) )
	  found++;
      for ( j = 0; ! found && j < 4; j++ )
	if ( Assign_String( this, areas[j], "LegendFontName", FieldName(i),
			    FieldContent(i), &LegendFontName(j) ) )
	  found++;
      for ( j = 0; ! found && j < 4; j++ )
	if ( Assign_Strings( this, areas[j], "Legend", FieldName(i),
			    FieldContent(i), &LegendsAnchor(j), &LegendsCount(j) ) )
	  found++;
      }
    if ( reader )
      (*reader)( this );
    }
    else /* Id != id */
    {/*===*/}
  OUT(apt_Read_Object);
  return  status;
  }

static
long Assign_String( class apt		      *self, const char			      *prefix , const char			      *desire , const char			      *candidate , const char			      *source , char			      **target )
      {
  char				      name[257];
  long			      status = 0;

  IN(Assign_String);
  sprintf( name, "%s%s", prefix, desire );
  DEBUGst(Name,name);
  if ( apts::CompareStrings( name, candidate ) == 0 )
    {
    DEBUG(Satisfied);
    status = 1;
    if ( *target )  free( *target );
    *target = strdup( source );
    }
  OUT(Assign_String);
  return status;
  }

static
long Assign_Strings( class apt		      *self, const char			      *prefix , const char			      *desire , const char			      *candidate , const char			      *source, char			    ***target, long			      *count )
          {
  char				      name[257], **strings;
  long			      status = 0;

  IN(Assign_Strings);
  sprintf( name, "%s%s", prefix, desire );
  DEBUGst(Name,name);
  if ( apts::CompareStrings( name, candidate ) == 0 )
    {
    DEBUG(Satisfied);
    status = 1;
    *target = (self)->ParseFieldContent(  source );
    *count = 0;
    strings = *target;
    while ( *(strings++) ) (*count)++;
    DEBUGdt(Count,*count);
    }
  OUT(Assign_Strings);
  return status;
  }

void
apt::WriteObjectField( struct apt_field	      *field )
      {
	class apt *self=this;
  IN(apt_WriteObjectField);
  fprintf( FieldsFile, "%s %s\n", field->name, field->content );
  OUT(apt_WriteObjectField);
  }

void
apt::WriteObject( FILE			      *file, long			       id , long			       level, apt_writefptr writer)
          {
  char				      bracket[256 + 2];
  long			      i;
	class apt *self=this;

  IN(apt_WriteObject);
  if ( this->writeID != id ) /*avoid recursive writes */
    { DEBUG(Not Recursive);
    FieldsFile = file;
    this->writeID = id;
/*===    if ( level )===*/ /* not parent, use datastream */
      {
      DEBUG( Write To Datastream );
      sprintf( bracket, "data{%s, %ld}\n", (this )->GetTypeName( ),
			  this->UniqueID( ) );
      fprintf( file, "\\begin%s", bracket );
      for ( i = 0; i < 4; i++ )
	{
	Write_Strings( this, file, "Title",  areas[i], TitlesCount(i), TitlesAnchor(i) );
        if ( TitleFontName(i) )
	  fprintf( file, "%sTitleFontName %s\n",  areas[i], TitleFontName(i) );
	Write_Strings( this, file, "Legend", areas[i], LegendsCount(i), LegendsAnchor(i) );
        if ( LegendFontName(i) )
	  fprintf( file, "%sLegendFontName %s\n", areas[i], LegendFontName(i) );
	}
      if ( writer )
	(*writer)( this );
      fprintf( file, "\\end%s", bracket );
      }
    FieldsFile = NULL;
    }
  OUT(apt_WriteObject);
  }

static
void Write_Strings( class apt		      *self, FILE			      *file, const char			      *name , const char			      *prefix, long			       count, const char			      * const *anchor )
            {
  long			      i;

  IN(Write_Strings);
  if ( anchor )
    {
    fprintf( file, "%s%s ", prefix, name );
    for ( i = 0; i < count; i++ )
      {
      if ( i )  fprintf( file, ";" );
      fprintf( file, "%s", anchor[i] );
      DEBUGst(String,anchor[i]);
      }
    fprintf( file, "\n" );
    }
  OUT(Write_Strings);
  }

static
void Parse_Field( class apt		      *self, const char			      *line, struct apt_field	      *field )
        {
  char				      work[257];
  char			     *t = work;

  IN(Parse_Field);
  DEBUGst(Given-Line,line);
  while ( *line  &&  *line != ' ' )  *t++ = *line++;
  *t = 0;
  field->name = strdup( work );
  while ( *line  &&  *line == ' ' )  line++;
  t = work;
  while ( *line  &&  *line != '\n' )  *t++ = *line++;
  *t = 0;
  field->content = strdup( work );
  OUT(Parse_Field);
  }

#define  max_field_content_items    50

char **
apt::ParseFieldContent( const char			      *string )
      {
  char			    **fields;
  const char		     *s = string, *s2;
  char *t;
  long			      i, length;

  IN(apt_ParseFieldContent);
  DEBUGst(String,string);
  fields = (char **) calloc( max_field_content_items, sizeof (char *) );
  i = 0;
  while ( *s )
    {
    s2 = s;
    length = 0;
    while ( *s2  &&  *s2 != ';' )
      {
      s2++;
      length++;
      }
    fields[i] = t = (char *) malloc( length + 1 );
    while ( *s  &&  *s != ';' )
      *t++ = *s++;
    *t = 0;
    if ( *s )
      s++;
    DEBUGst(Field,fields[i]);
    i++;
    }
  OUT(apt_ParseFieldContent);
  return  fields;
  }

struct apt_field_contents *
apt::ParseFieldContents( const char			      *string )
      {
  char			    **content, **field;
  struct apt_field_contents *contents;
  char			     *s, *s2, *t;
  long			      i = 0, length = 0;

  IN(apt_ParseFieldContents);
  DEBUGst(String,string);
  contents = (struct apt_field_contents *)
    malloc( sizeof(struct apt_field_contents) +
	    max_field_content_items * sizeof(struct apt_field) );
  contents->count = 0;
  field = content = (this)->ParseFieldContent(  string );
  while ( *field )
    {
    s = s2 = *field;
    while ( *s2  &&  *s2 != '(' )
      { length++; s2++; }
    contents->field[i].name = t = (char *) malloc( length + 1 );
    while ( *s  &&  *s != '(' )
      *t++ = *s++;
    *t = 0;
    DEBUGst(Name,contents->field[i].name);
    if ( *s )  s++;
    s2 = s;
    length = 0;
    while( *s2  &&  *s2 != ')' )
      { length++; s2++; }
    contents->field[i].content = t = (char *) malloc( length + 1 );
    while ( *s  &&  *s != ')' )
      *t++ = *s++;
    *t = 0;
    DEBUGst(Content,contents->field[i].content);
    free( *field );
    field++;
    contents->count = ++i;
    }
  free( content );
  OUT(apt_ParseFieldContents);
  return  contents;
  }
