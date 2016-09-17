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

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Org Data-Class

MODULE	org.c

VERSION	1.0

AUTHOR	TC Peters
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the suite of Methods that support the Org Data-Class.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  01/19/89	Created (TCP)
  08/23/89	Remove Create method (TCP)
  08/24/89	Upgrade to Version 1.0 (TCP)
  08/31/89	Change OfData to OfDatum (TCP)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
ATK_IMPL("org.H")
#include <ctype.h>
#include <sys/param.h>
#include <dataobject.H>
#include <text.H>
#include <tree.H>
#include <filetype.H>
#include <org.H>

#define Tree (self->tree_data_object)

static tree_Specification specification[] = {
  tree_Order( tree_PreOrder ),
  0
};

boolean Org_Debug = 0;
#define debug Org_Debug

ATKdefineRegistry(org, apt, NULL);
static long Read_Body( class org		      *self, FILE			      *file );
static long Write_Body( class org  *self, FILE  *file, long writeID );
static void Strip( char  *string );


const char *
org::ViewName( )
  {
    return ( "orgv" );
}

static
void FreeDatum( tree_type_node node, void *user)
{
  class org *self = (class org *)user;
  if ( Tree->NodeDatum(  node ) )
    delete (class text *)(Tree)->NodeDatum(  node );
}

org::org( )
    {
  class org *self=this;
  boolean status = true;

  IN(org_InitializeObject);
  DEBUGst(RCSID,rcsid);
  if ( (Tree = tree::Create( specification, this )) == NULL ) {
    printf( "ORGVIEW: Unable to Create AptTree Object\n" );
    status = false;
  }
  Tree->DestroyDatum(FreeDatum, this);
  OUT(org_InitializeObject);
  THROWONFAILURE((status));
}

org::~org( )
    {
  class org *self=this;
  IN(org_FinalizeObject );
  if ( Tree )
      (Tree )->Destroy();
  OUT(org_FinalizeObject );
}

long
org::Read( FILE  *file, long  id )
      {
  long status;

  IN(org_Read);
  status = Read_Body( this, file );
  OUT(org_Read);
  return(status);
}

static
long Read_Body( class org		      *self, FILE			      *file )
      {
  boolean		      done = false;
  long			      c, braces = 0, brackets = 0, status = ok;
  char				      string[4096], *ptr, *end_ptr;
  tree_type_node	      parent = NULL, child = NULL, node = NULL;
  class text			     *textp = NULL;

  IN(Read_Body);
  ptr = string;
  end_ptr = ptr + sizeof(string) - 2;
  while ( !done  &&  (c = getc( file )) ) {
    switch ( c ) {
      case '\n':
	if ( ptr > string ) {
	  Strip( ptr = string );
	  if ( parent ) {
	    if ( (child = node = (Tree)->CreateChildNode(  string, 0, parent )) == NULL )
	      { DEBUG(ERROR Creating Node);/*===*/      }
	  }
	  else {
	    if ( (parent = node = (Tree)->CreateRootNode(  string, 0 )) == NULL )
	      { DEBUG(ERROR Creating RootNode);/*===*/  }
	  }
	}
        continue;
      case '{':
	if(ptr > string)
	    break;
	braces++;
	if ( child )  parent = child;
        continue;
      case '}':
	if(ptr > string)
	    break;
	braces--;
	child = parent;
	parent = (Tree)->ParentNode(  parent );
        continue;
      case '[':
	if(ptr > string)
	    break;
	brackets++;
	/* I'd like to ignore length, since inset reader will stop at end */
	/* However, some files (e.g. example3.org) have broken insets */
	/* As a compromise, if length=0, ignore length */
	/* This will of course be broken on unseekables, but who cares? */
	while((c = getc(file)) != '\n' && c != EOF) *ptr++ = c;
	*ptr = 0;
	ptr = string;
	        {
		    int len = atoi(string);
		    unsigned long epos = 0;
		    if(len > 0)
			epos = ftell(file) + len;
		    long objID;
		    filetype::Lookup(file, NULL, &objID, NULL);
		    textp = new text;
		    (textp)->Read( file, objID);
		    if(epos)
			fseek(file, epos, SEEK_SET);
		}
	(Tree)->SetNodeDatum(  node, (long) textp );
	continue;
      case ']':
	if(ptr > string)
	    break;
	brackets--;
	continue;
      case '\\':
	/* the old code pretty much aborted no matter what, here */
	/* I guess that meant it always assumed it was \enddata */
	/* I prefer allowing backslashes within the node name */
	if(ptr > string)
	    break;
	/* and at the beginning, to escape \ { [ */
	if ( (c = getc( file )) == '\\' || c == '[' || c == '{')
	    break;
	/* now it's either enddata or an error.  In any case, just
	 * suck the line in and exit */
	/* not that org is embeddable in other docs, anyway, but also
	 * stop on }, and if not @ eol, preserve char after } */
	while ( c != EOF && c != '}' && (c = getc( file )) != '\n' );
	if(c == '}' && (c = getc( file )) != '\n')
	    ungetc(c, file);
	done = true;
	continue;
      case EOF:
	done = true;
        continue;
    }
      if ( ptr < end_ptr ) {
	  if ( (ptr > string)  ||  (c != ' '  &&  c != '\t') )
	    {  *ptr++ = c;  *ptr = 0;  }
	}
	else
	  { DEBUG(ERROR: exceeded string);/*===*/ }
  }
  if ( braces ) {
    status = failure;
/*===*/printf("ORG: ERROR  %ld Unbalanced Braces\n", braces);
  }
  if ( brackets ) {
      status = failure;
/*===*/printf("ORG: ERROR  %ld Unbalanced Brackets\n", brackets);
  }
/*===*/
  OUT(Read_Body);
  return(status);
  }

long
org::Write( FILE			      *file, long			       writeID, int			       level )
        {
  long			      id;
  UNUSED long status; /* only used when debugging */

  IN(org_Write);
  DEBUGdt(Headerwriteid,this->writeID);
  DEBUGdt(Given Id,writeID);
  DEBUGdt(Given Level,level);
  id = (this )->UniqueID( );
  DEBUGdt(Local-ID,id);
  if ( this->writeID != writeID ) {
    this->writeID = writeID;
    fprintf( file, "\\begindata{%s,%ld}\n", (this )->GetTypeName( ), id );
    status = Write_Body( this, file, writeID );
    fprintf( file, "\n\\enddata{%s,%ld}\n", (this )->GetTypeName( ), id );
  }
  DEBUGdt(Status,status);
  OUT(org_Write);
  return  this->dataobject::id;
}

static
long Write_Body( class org  *self, FILE  *file, long writeID )
    {
  long status = ok;
  tree_type_node node = (Tree )->RootNode( );
  long level, current_level = 1;
  class text *textp;
  int size;

  IN(Write_Body);
  while ( node ) {
    if ( (level = (Tree)->NodeLevel(  node )) > current_level )
	fprintf( file, "%*s{\n", (int)(2 * level), "" );
    else 
	if ( level < current_level )
	    for ( ; current_level > level; current_level-- )
		fprintf( file, "%*s}\n", (int)(2 * current_level), "" );
    current_level = level;
    const char *name = (Tree)->NodeName( node );
    const char *bs = *name == '\\' || *name == '{' || *name == '[' ? "\\" : "";
    fprintf( file, "%*s%s%s\n", (int)(2 * level), "", bs, name );
    if ( (textp = (class text *) (Tree)->NodeDatum( node)) && 
	 (size = (textp)->GetLength()) > 0 ) {
	/* don't write to tmp file, and don't output size of text */
	/* it's unnecessary and just makes this code uglier than needed */
	/* however, it does make this incompatible with official */
	/* (and mostly broken anyway) org.  Oh well. */
	/* I'll still write a length of 0 just for giggles.  The old */
	/* org will abort reading shortly thereafter */
	/* I'm tempted to just drpo the whole square bracket crap and use */
	/* \begindata to signify descriptions */
	fprintf( file, "%*s[0\n", (int)(2 * level), "" );
	(textp)->Write( file, writeID, 1);
        fputs( "]\n", file);
    }
    node = (Tree)->NextNode(  node );
  }
  for ( ; current_level > 1; current_level-- )
    fprintf( file, "%*s}\n", (int)(2 * current_level), "" );
  OUT(Write_Body);
  return(status);
}

const char *
org::NodeName( struct tree_node  *node )
    {
  class org *self=this;
  const char *name = NULL;

  IN(org_NodeName);
  if ( node )
    name = (Tree)->NodeName(  node );
  OUT(org_NodeName);
  return  name;
}

void
org::SetDebug( boolean  state )
    {
  class org *self=this;
  IN(org_SetDebug);
  debug = state;
  (Tree)->SetDebug(  debug );
  OUT(org_SetDebug);
}

static
void Strip( char  *string )
  {
  char *ptr = string;
  int len;

  while ( *ptr == ' '  ||  *ptr == '\t' )  ptr++;
  len = strlen(ptr);
  memmove( string, ptr , len);
  ptr = string + len - 1;
  while ( ( *ptr == ' '  ||  *ptr == '\t' ) && ptr >= string ) *ptr-- = 0;
}
