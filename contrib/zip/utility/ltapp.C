/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include <stdio.h>
#include <im.H>
#include <frame.H>
#include <message.H>
#include <view.H>
#include <environ.H>
#include <lpair.H>
#include <text.H>
#include <textview.H>
#include <lt.H>
#include <ltv.H>
#include <ltapp.H>
#include <sys/types.h>
#include <zip.H>

static boolean debug=FALSE;


ATKdefineRegistryNoInit(ltapp, application);

ltapp::ltapp( )
{
    IN(ltapp_InitializeObject);
    *(this->trace_stream_name) = 0;
    *(this->trace_raster_name) = 0;
    (this)->SetMajorVersion( 1 );
    (this)->SetMinorVersion( 0 );
    OUT(ltapp_InitializeObject);
    THROWONFAILURE( TRUE);
}

boolean
ltapp::ParseArgs( int  argc, const char **argv )
{
    IN(ltapp_ParseArgs);
    (this)->application::ParseArgs(argc, argv);
    while ( *++argv )
    {
	DEBUGst(ARGV,*argv);
	if ( **argv == '-' )
	{
	    if ( strcmp( *argv, "-d" ) == 0 )
		debug = 1;
	    else  printf( "LightTable: Unrecognized switch '%s'\n", *argv );
	}
	else
	{
	    if ( *(this->trace_stream_name) == 0 )
		strcpy( this->trace_stream_name, *argv );
	    else
		if ( *(this->trace_raster_name) == 0 )
		    strcpy( this->trace_raster_name, *argv );
		else  printf( "LightTable: Unrecognized argument '%s'\n", *argv );
	}
    }
    OUT(ltapp_ParseArgs);
    return TRUE;
}

boolean 
ltapp::Start( )
{
    IN(ltapp_Start);
    if( !(this)->application::Start() )
	return FALSE;
    this->imp = im::Create( NULL );
    if ( !this->imp )
    {
	fprintf(stderr,"lighttable: Failed to create new window; exiting.\n");
	return(FALSE);
    }
    if((this->ltp = new lt) == NULL) {
	fprintf(stderr,"lighttable: Could not allocate enough memory; exiting.\n");
	return(FALSE);
    }
    (this->ltp)->Set_Debug(  debug );
    if ( this->trace_stream_name[0]  && this->trace_raster_name[0]  )
	if ( (this->ltp)->Read_Visuals(  this->trace_stream_name, this->trace_raster_name ) )
	    return(FALSE);
    if((this->ltpview = new ltv) == NULL) {
	fprintf(stderr,"lighttable: Could not allocate enough memory; exiting.\n");
	return(FALSE);
    }
    (this->ltpview)->Set_Debug(  debug );
    (this->ltpview)->SetDataObject(  this->ltp );
    if((this->framep = new frame) == NULL) {
	fprintf(stderr,"lighttable: Could not allocate enough memory; exiting.\n");
	return(FALSE);
    }
    (this->framep)->SetView(  this->ltpview );
    (this->imp)->SetView(  this->framep );
    (this->ltpview)->WantInputFocus(  this->ltpview );
    OUT(ltapp_Start);
    return TRUE;
}
