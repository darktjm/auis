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

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The LightTable Data-object Program

MODULE	lt.c

NOTICE	IBM Internal Use Only

DESCRIPTION
	This is the suite of Methods that support the LightTable Data-object.

HISTORY
  10/10/88	Created (TCP)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include "lt.H"
#include "zip.H"
#include "raster.H"
#include <errno.h>

static boolean debug=FALSE;
#define  BackgroundWidth	    (self->background_width)
#define  BackgroundHeight	    (self->background_height)


ATKdefineRegistry(lt, dataobject, NULL);



void
lt::Set_Debug( boolean mode )
{
    debug = mode;
    (this->zipp)->Set_Debug(  debug );
}

lt::lt( )
{
    IN(lt_InitializeObject);
    this->zipp = new zip;
    (this->zipp)->Set_Debug(  debug );
    this->foreground_stream = NULL;
    *this->foreground_stream_name = 0;
    this->rasterp = new raster;
    OUT(lt_InitializeObject);
    THROWONFAILURE( TRUE);
}

long
lt::Read_Visuals( register char *foreground , register char *background )
{
    register long			      status = 0;
    register FILE			     *file;
    static const char			     source[] =
      "*G;-1000,1000\nLW0\n>1000,-1000";

    IN(lt_Read_Visuals);
    strcpy( this->foreground_stream_name, foreground );
    strcpy( this->background_raster_name, background );
    DEBUGst(Stream-name,this->foreground_stream_name);
    if ( status = (this->zipp)->Open_Stream( &this->foreground_stream,
					   this->foreground_stream_name, 0 ) )
    { DEBUG(Open Failure);
    if ( status ==  zip_system_status_value_boundary + ENOENT )
    { DEBUG(Non-existent);
    if ( (status = (this->zipp)->Create_Stream( 
					      &this->foreground_stream, this->foreground_stream_name, 0 )) == zip_ok )
	status = (this->zipp)->Set_Stream_Source(  this->foreground_stream, source );
    }
    }
    else
	status = (this->zipp)->Read_Stream(  this->foreground_stream );
    if ( status == zip_ok )
    {
	if ( file = fopen( this->background_raster_name, "r" ) )
	{
	    if ( (this->rasterp)->Read(  file, 12345 ) )
	    { DEBUG(ERROR Reading Raster);
	    printf( "LT: Error Reading Raster-file '%s'\n", this->background_raster_name );
	    status = zip_failure;
	    }
	    else
	    { DEBUG(Success Reading Raster);
	    this->background_width = (this->rasterp )->GetWidth( );
	    DEBUGdt(BackgroundWidth,this->background_width);
	    this->background_height = (this->rasterp )->GetHeight( );
	    DEBUGdt(BackgroundHeight,this->background_height);
	    }
	    fclose( file );
	}
	else
	{
	    status = zip_failure;
	    printf( "LT: Error Opening Raster-file '%s'\n", this->background_raster_name );
	}
    }
    else printf( "LT: Error Reading Zip-file '%s'\n", this->foreground_stream_name );
    OUT(lt_Read_Visuals);
    return  status;
}
