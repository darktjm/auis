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

TITLE	The Chart Application-Class

MODULE	chartapp.c

VERSION	0.0

AUTHOR	TC Peters
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the suite of Methods that support the Chart Application-Class.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  01/19/89	Created (TCP)
  05/04/89	Add Major/Minor Version settings (TCP)
  05/10/89	Move im_Create outside im_SetView (macro problem) (TCP)
  05/31/89	Add classID parameter in FinalizeObject (TCP)
  09/01/89	Use super_ParseArgs (TCP)
  09/07/89	Remove Open-msg for non-existent new file (TCP)

END-SPECIFICATION  ************************************************************/


#include  <andrewos.h>
ATK_IMPL("chartapp.H")
#include  <im.H>
#include  <filetype.H>
#include  <frame.H>
#include  <chart.H>
#include  <chartv.H>
#include  <chartapp.H>

static int chartapp_debug = 0;

#define debug chartapp_debug

#define  Chart		    (self->chart_data_object)
#define  ChartView	    (self->chart_view_object)
#define  Frame		    (self->framep)
#define  Im		    (self->imp)
#define  Source		    (self->source)

static chart_Specification	    data_specification[] =
  {
  0
  };

static chartv_Specification	    view_specification[] =
  {
  0
  };


ATKdefineRegistry(chartapp, application, NULL);

chartapp::chartapp( )
      {
    class chartapp *self=this;
    IN(chartapp::chartapp);
  *Source = 0;
  (this)->SetMajorVersion(  1 );
  (this)->SetMinorVersion(  0 );
  OUT(chartapp::chartapp);
  THROWONFAILURE( TRUE);
  }

chartapp::~chartapp( )
      {}

boolean
chartapp::ParseArgs( int			   argc, const char			 **argv )
        {
  class chartapp *self=this;
  IN(chartapp_ParseArgs);
  (this)->application::ParseArgs(  argc, argv );
  while ( *++argv )
    { DEBUGst(ARGV,*argv);
    if ( **argv == '-' )
      {
      if ( strcmp( *argv, "-D" ) == 0 )
        debug = 1;
      else  printf( "Chart: Unrecognized switch '%s'\n", *argv );
      }
      else
      {
      if ( *(Source) == 0 )
        strcpy( Source, *argv );
	else
	printf( "Chart: Excessive Argument (Ignored) '%s'\n", *argv );
      }
    }
  OUT(chartapp_ParseArgs);
  return TRUE;
  }

boolean
chartapp::Start( )
    {
  class chartapp *self=this;
  FILE			 *file;
  long				  id, status = true;

  (this )->application::Start( );
  if ( ( Chart = chart::Create( data_specification, (char *) this ) ) )
    (Chart)->SetDebug(  debug );
  if ( ( ChartView = chartv::Create( view_specification, (char *) this ) ) )
    (ChartView)->SetDebug(  debug );
  if ( Chart  &&  ChartView )
    {
    if((Frame = new frame) == NULL) {
	fprintf(stderr,"Could not allocate enough memory.\nexiting.\n");
	exit(-1);
    }
    (Frame)->SetView(  ChartView );
    if((Im = im::Create(NULL)) == NULL) {
	fprintf(stderr,"Could not create new window.\nexiting.\n");
	exit(-1);
    }
    (Im)->SetView(  Frame );
    if ( *Source )
      {
      (Chart)->SetChartAttribute(  chart_FileName(Source) );
      (Frame)->SetTitle(  Source );
      if ( ( file = fopen( Source, "r" ) ) )
        {
        filetype::Lookup( file, Source, &id, 0 );
        (Chart)->Read(  file, id );
        fclose( file );
        }
      }
    (ChartView)->SetDataObject(  Chart );
    (ChartView)->WantInputFocus(  ChartView );
    return  status;
    }
    else 
    {
    printf( "Chart: Failed to Create objects\n" );
    return FALSE;
    }
  }
