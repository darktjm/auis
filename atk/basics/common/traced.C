/* Copyright 1992 Andrew Toolkit Consortium, Carnegie Mellon University */

#include <andrewos.h>
ATK_IMPL("traced.H")
#include "traced.H"


ATKdefineRegistryNoInit(traced, ATK);


traced::traced()
{
    this->refcount=1;
    THROWONFAILURE( TRUE);
}

void traced::Destroy()
{
    if(--this->refcount<=0) {
	delete this;
    }
}

traced::~traced()
{
    if(this->refcount > 0)
	/* somebody called delete instead of Destroy() */
	kill(getpid(), 11);
    return;
}

