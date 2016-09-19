/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

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
