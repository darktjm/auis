/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Sched Data-object Program

MODULE	sched.c

NOTICE	IBM Internal Use Only

DESCRIPTION
	This is the suite of Methods that support the LightTable Data-object.

HISTORY
  10/10/88	Created (TCP)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include "sched.H"
#include "zip.H"
#include <errno.h>
static boolean debug=FALSE;

ATKdefineRegistry(sched, dataobject, NULL);

void
sched::Set_Debug( boolean mode )
{
    debug = mode;
}

class sched *
sched::Create( char *stream_name )
{
    class sched		     *self;
    long			      status;

    IN(sched_Create);
    if ( ( self = new sched ) )
    {
	if ( stream_name  &&  *stream_name )
	{
	    strcpy( self->stream_name, stream_name );
	    DEBUGst(Stream-name,self->stream_name);
	    if ( ( status = (self->zipp)->Open_Stream( &self->stream,
						   self->stream_name, 0 ) ) )
	    { DEBUG(Open Failure);
	    printf( "Schedule: Unable to Open %s\n", self->stream_name );
	    }
	    else  status = (self->zipp)->Read_Stream(  self->stream );
	}
    }
    OUT(sched_Create);
    return  self;
}


sched::sched( )
{
    long			      status = false;

    IN(sched_InitializeObject);
    this->stream = NULL;
    *this->stream_name = 0;
    if ( ( this->zipp = new zip ) )
	status = true;
    OUT(sched_InitializeObject);
    THROWONFAILURE(status);
}
