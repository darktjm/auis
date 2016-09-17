/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
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

#include <andrewos.h>
#include <stdio.h>
#include <math.h>
#include <circlepi.H>
#include <observable.H>

/* Defined constants and macros */

/* External declarations */

/* Forward Declarations */

/* Global variables */



ATKdefineRegistry(circlepi, dataobject, circlepi::InitializeClass);

boolean
circlepi::InitializeClass()
{
/* 
  Initialize all the class data.
*/
  return(TRUE);
}


circlepi::circlepi()
{
	ATKinit;

/*
  Inititialize the object instance data.
*/
    this->depth_limit = 4;
    THROWONFAILURE((TRUE));
}


circlepi::~circlepi()
{
	ATKinit;

/*
  Finalize the object instance data.
*/
  return;
}


void
circlepi::SetDepth(int  limit)
          {
    if (this->depth_limit != limit) {
	this->depth_limit = limit;
	(this)->NotifyObservers( observable_OBJECTCHANGED);
    }
}
