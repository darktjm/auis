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
#include <im.H>
#include <frame.H>
#include <message.H>
#include <view.H>
#include <environ.H>
#include <lpair.H>
#include <text.H>
#include <textview.H>
#include <sched.H>
#include <schedv.H>
#include <schedapp.H>
#include <zip.H>

static boolean debug=FALSE;

ATKdefineRegistry(schedapp, application, NULL);

schedapp::schedapp( )
{
    IN(schedapp_InitializeObject);
    *(this->stream_name) = 0;
    OUT(schedapp_InitializeObject);
    THROWONFAILURE( TRUE);
}

boolean
schedapp::ParseArgs( int  argc, const char **argv )
{
    IN(schedapp_ParseArgs);
    while ( *++argv )
    {
	DEBUGst(ARGV,*argv);
	if ( **argv == '-' )
	{
	    if ( strcmp( *argv, "-d" ) == 0 )
		debug = 1;
	    else  printf( "Sched: Unrecognized switch '%s'\n", *argv );
	}
	else
	{
	    if ( *(this->stream_name) == 0 )
		strcpy( this->stream_name, *argv );
	}
    }
    OUT(schedapp_ParseArgs);
    return TRUE;
}

boolean 
schedapp::Start( )
{
    IN(schedapp_Start);
    if( !(this)->application::Start() )
	return FALSE;
    if((this->imp  = im::Create(NULL)) == NULL) {
	fprintf(stderr,"sched: Failed to create new window; exiting.\n");
	return(FALSE);
    }
    if (*this->stream_name == 0 ) strcpy( this->stream_name,"itcCR.scd");
    if((this->schedp = sched::Create( this->stream_name )) == NULL) {
	fprintf(stderr,"sched: Could not allocate enough memory; exiting.\n");
	return(FALSE);
    }
    (this->schedp)->Set_Debug(  debug );
    if((this->schedpview = new schedv) == NULL) {
	fprintf(stderr,"sched: Could not allocate enough memory; exiting.\n");
	return(FALSE);
    }
    (this->schedpview)->Set_Debug(  debug );
    (this->schedpview)->SetDataObject(  this->schedp );
    if((this->framep = new frame) == NULL) {
	fprintf(stderr,"sched: Could not allocate enough memory; exiting.\n");
	return(FALSE);
    }
    (this->framep)->SetView(  this->schedpview );
    (this->imp)->SetView(  this->framep );
    OUT(schedapp_Start);
    return TRUE;
}

int
schedapp::Run()
{
    IN(schedapp_Run);
    (this )->Fork( );
    (this->schedpview)->WantInputFocus(  this->schedpview );
    DEBUG(...Interacting...);
    while ( im::Interact( 0 ) ) ;
    im::KeyboardProcessor();
    OUT(schedapp_Run);
    return(0);
}
