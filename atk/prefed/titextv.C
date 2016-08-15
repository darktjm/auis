/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/

#include <andrewos.h>

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/titextv.C,v 1.4 1994/11/30 20:42:06 rr2b Stab74 $";
#endif



 

ATK_IMPL("titextv.H")


#include "titextv.H"

#include <text.H>
#include <textview.H>
#include "textintv.H"


ATKdefineRegistry(titextv, textview, NULL);
#ifndef NORCSID
#endif


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
