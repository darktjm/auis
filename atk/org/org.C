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

#include <andrewos.h>

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/org/RCS/org.C,v 1.8 1995/11/07 20:17:10 robr Stab74 $";
#endif

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
static long Read_Body( register class org		      *self, register FILE			      *file );
static long Write_Body( register class org  *self, register FILE  *file, long writeID );
static void Strip( register char  *string );


char *
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
  register boolean status = true;

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
org::Read( register FILE  *file, register long  id )
      {
  register long status;

  IN(org_Read);
  status = Read_Body( this, file );
  OUT(org_Read);
  return(status);
}

static
long Read_Body( register class org		      *self, register FILE			      *file )
      {
  register boolean		      done = false;
  register long			      c, count, braces = 0, brackets = 0, status = ok,
				      description_size, description_length,
				      description_increment = 32;
  char				      string[4096], *ptr, *end_ptr,
				     *count_ptr, counter[32];
  register char			     *description_ptr = NULL, *description;
  register tree_type_node	      parent = NULL, child = NULL, node;
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
	if ( ptr > string ) {
	  Strip( ptr = string );
	  if ( parent ) {
	    if ( (child = node = (Tree)->CreateChildNode(  string, 0, parent )) == NULL )
	      { DEBUG(ERROR Creating Node);/*===*/      }
	  }
	  else {/*===*/}
	}
	else {
	  child = parent;
	  parent = (Tree)->ParentNode(  parent );
	}
        continue;
      case '[':
	if(ptr > string)
	    break;
	brackets++;
	count_ptr = counter;
	while ( (c = getc( file ))  &&  c != EOF  && c != '\n' )
	  *count_ptr++ = c;
	*count_ptr = 0;
	count = atoi( counter );
	DEBUGdt(Count,count);
	description_ptr = description = (char *) malloc( count );
	fread(description, count, 1, file);
	{
	  FILE *f = tmpfile();
	  fwrite(description, count, 1, f);
	  fputc('\n', f);
	  rewind(f);
	        {
		    long objID;
		    filetype::Lookup(f, NULL, &objID, NULL);
		    textp = new text;
		    (textp)->Read( f, objID);
		}
	  fclose(f);
	}
	(Tree)->SetNodeDatum(  node, (long) textp );
	DEBUGst(Description, description);
	continue;
      case ']':
	if(ptr > string)
	    break;
	brackets--;
	continue;
      case '\\': /* no idea what old behavior was supposed to do */
	if(ptr > string)
	    break;
	if ( (c = getc( file )) != '\n'  &&  c != EOF ) break;
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
org::Write( register FILE			      *file, register long			       writeID, register int			       level )
        {
  register long			      status, id;

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
long Write_Body( register class org  *self, register FILE  *file, long writeID )
    {
  register long status = ok;
  register tree_type_node node = (Tree )->RootNode( );
  register long level, current_level = 1;
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
	long realSize = 0;
	char *description;
	FILE *f;
	if(f = tmpfile()) {
	    (textp)->Write( f, writeID, 1);
	    fseek(f, 0, 2);
	    realSize = ftell(f);
	    rewind(f);
	    {
		if(description = (char*) malloc(realSize + 1)) {
		    if(fread(description, realSize, 1, f) != 1) {
			fprintf(stderr, "org: incomplete read on temp file\n");
			realSize = 0;
		    }
		    description[realSize] = (char)0;
		}
	    }
	    fclose(f);
	}
	fprintf( file, "%*s[%ld\n%s]\n", (int)(2 * level), "", realSize, description);
	free(description);
    }
    node = (Tree)->NextNode(  node );
  }
  for ( ; current_level > 1; current_level-- )
    fprintf( file, "%*s}\n", (int)(2 * current_level), "" );
  OUT(Write_Body);
  return(status);
}

char *
org::NodeName( register struct tree_node  *node )
    {
  class org *self=this;
  register char *name = NULL;

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
void Strip( register char  *string )
  {
  register char *ptr = string;
  int len;

  while ( *ptr == ' '  ||  *ptr == '\t' )  ptr++;
  len = strlen(ptr);
  memmove( string, ptr , len);
  ptr = string + len - 1;
  while ( *ptr == ' '  ||  *ptr == '\t' && ptr >= string ) *ptr-- = 0;
}
