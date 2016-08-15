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
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/org/RCS/orgapp.C,v 1.6 1995/11/07 20:17:10 robr Stab74 $";
#endif

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Org Application-Class

MODULE	orga.c

VERSION	1.0

AUTHOR	TC Peters
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the suite of Methods that support the Org Application-Class.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  01/19/89	Created (TCP)
  05/10/89	Move im_Create outside im_SetView (macro problem) (TCP)
  05/19/89	Added Ancestry testing in Org_Hit (TCP)
  05/24/89	Permit argument of non-existent org file (TCP)
  05/31/89	Add CLissID to FinalizeObject parameters (TCP)
  08/22/89	Use super_ParseArgs (TCP)
		Remove Testing via Org_Hit
		Remove parochial Specifications
  09/07/89	Set V1.0 msg (TCP)

END-SPECIFICATION  ************************************************************/

ATK_IMPL("orgapp.H")
#include  <im.H>
#include  <filetype.H>
#include  <frame.H>
#include  <tree.H>
#include  <org.H>
#include  <orgv.H>
#include  <orgapp.H>

boolean orgapp_debug = FALSE;
#define debug orgapp_debug

#define  Org		    (self->org_data_object)
#define  OrgView	    (self->org_view_object)
#define  Frame		    (self->framep)
#define  Im		    (self->imp)
#define  Source		    (self->source)
#define  Tree		    (Org->tree_data_object)


ATKdefineRegistry(orgapp, application, NULL);

orgapp::orgapp(  )
      {
    class orgapp *self=this;
    IN(orgapp::InitializeObject);
  *Source = 0;
  (this)->SetMajorVersion( 1 );
  (this)->SetMinorVersion( 0 );
  OUT(orgapp::InitializeObject);
  }

orgapp::~orgapp(  )
      {}

boolean
orgapp::ParseArgs( register int			   argc, register const char			 **argv )
        {
  class orgapp *self=this;  
  IN(orgapp_ParseArgs);
  (this)->application::ParseArgs( argc, argv );
  while ( *++argv )
    { DEBUGst(ARGV,*argv);
    if ( **argv == '-' )
      {
      if ( strcmp( *argv, "-D" ) == 0 )
        debug = TRUE;
      else  printf( "Org: Unrecognized switch '%s' (Ignored)\n", *argv );
      }
      else
      {
      if ( *(Source) == 0 )
        strcpy( (char*) Source, *argv );
	else
	printf( "Org: Excessive Argument (Ignored) '%s'\n", *argv );
      }
    }
  OUT(orgapp_ParseArgs);
  return TRUE;
  }

boolean
orgapp::Start(  )
    {
  class orgapp *self=this;
  register FILE			 *file;
  long				  id, status = true;

  (this)->application::Start(  );
  if ( Org = new org )
    (Org)->SetDebug(  debug );
  if ( OrgView = new orgv )
    (OrgView)->SetDebug(  debug );
  if ( Org  &&  OrgView )
    {
    if((Frame = new frame) == NULL) {
	fprintf(stderr,"org: Could not allocate enough information; exiting.\n");
	exit(-1);
    }
    (Frame)->SetView(  OrgView );
    if((Im = im::Create(NULL)) == NULL) {
	fprintf(stderr,"org: Could not create new window; exiting.\n");
	exit(-1);
    }
    (Im)->SetView(  Frame );
    if ( *Source )
      {
      (Frame)->SetTitle(  Source );
      if ( file = fopen( Source, "r" ) )
        {
        filetype::Lookup( file, Source, &id, 0);
        (Org)->Read(  file, 0 );
        fclose( file );
        }
      }
    if ( status == true )
      {
      (OrgView)->SetDataObject(  Org );
      (OrgView)->WantInputFocus(  OrgView );
      }
    return  status;
    }
    else 
    {
    printf("Org: Failed to Create objects\n");
    return FALSE;
    }
  }
