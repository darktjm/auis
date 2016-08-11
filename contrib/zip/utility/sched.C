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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/zip/utility/RCS/sched.C,v 1.4 1994/08/14 18:44:54 rr2b Stab74 $";
#endif


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
sched::Create( register char *stream_name )
{
    register class sched		     *self;
    register long			      status;

    IN(sched_Create);
    if ( self = new sched )
    {
	if ( stream_name  &&  *stream_name )
	{
	    strcpy( self->stream_name, stream_name );
	    DEBUGst(Stream-name,self->stream_name);
	    if ( status = (self->zipp)->Open_Stream( &self->stream,
						   self->stream_name, 0 ) )
	    { DEBUG(Open Failure);
	    printf( "Schedule: Unable to Open %s\n", self->stream );
	    }
	    else  status = (self->zipp)->Read_Stream(  self->stream );
	}
    }
    OUT(sched_Create);
    return  self;
}


sched::sched( )
{
    register long			      status = false;

    IN(sched_InitializeObject);
    this->stream = NULL;
    *this->stream_name = 0;
    if ( this->zipp = new zip )
	status = true;
    OUT(sched_InitializeObject);
    THROWONFAILURE(status);
}
