/* File hr.C created by Ward Nelson
      (c) Copyright IBM Corp 1995.  All rights reserved. 

   hr, an inset that draws and prints a window-width horizontal line */

#include <andrewos.h>
#include "hr.H"

ATKdefineRegistry(hr, dataobject, NULL);

char *hr::ViewName()
{
    return "hrv";
}
