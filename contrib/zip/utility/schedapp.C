/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

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

ATKdefineRegistryNoInit(schedapp, application);

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
    if (*this->stream_name == 0 ) strcpy( this->stream_name,environ::AndrewDir("/lib/zip/itcCR.scd"));
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
