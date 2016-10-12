/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

#include <andrewos.h>
ATK_IMPL("prefsbv.H")


#include "prefsbv.H"

#include <view.H>


ATKdefineRegistry(prefsbv, sbuttonv, NULL);

prefsbv::prefsbv()
{
    THROWONFAILURE( TRUE);
    
}

prefsbv::~prefsbv()
{
}

boolean prefsbv::Touch(int  ind, enum view::MouseAction  act)
{
    if(!(this)->sbuttonv::Touch( ind, act)) return FALSE;
    if(act!=view::LeftUp) return TRUE;
    else return FALSE;
}
