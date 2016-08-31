/* File hidden.C created by Ward Nelson
      (c) Copyright IBM Corp 1995.  All rights reserved. 

   hidden, an inset for stashing non-displayed text inside of */

#include <andrewos.h>
#include "hidden.H"

ATKdefineRegistry(hidden, htmltext, hidden::InitializeClass);

const char *hidden::ViewName()
{
    return "hiddenview";
}

boolean hidden::InitializeClass()
{
    return TRUE;
}

hidden::hidden()
{
    ATKinit;
    this->Visibility = hidden_EXPOSED;
    THROWONFAILURE( TRUE);
}


hidden::~hidden()
{
    ATKinit;
    return;
}
