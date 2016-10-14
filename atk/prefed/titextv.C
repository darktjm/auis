/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

#include <andrewos.h>
ATK_IMPL("titextv.H")


#include "titextv.H"

#include <text.H>
#include <textview.H>
#include "textintv.H"


ATKdefineRegistryNoInit(titextv, textview);

titextv::titextv()
{
    this->intv=NULL;
    this->lastpos=(-1);
    THROWONFAILURE( TRUE);
    
}

titextv::~titextv()
{
}


void titextv::Update()
{
    long pos;

    (this)->textview::Update();
    pos=(this)->GetDotPosition();
    if(this->lastpos!=pos) {
	this->lastpos=pos;
	if(this->intv) (this->intv)->SetDotPosition( pos);
    }
}
