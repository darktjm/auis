/* scpanner.c - scrollbar / panner box view */

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
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/supportviews/RCS/scpanner.C,v 3.5 1994/11/30 20:42:06 rr2b Stab74 $";
#endif

/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
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


ATK_IMPL("scpanner.H")
#include <scpanner.H>
#include <panner.H>


ATKdefineRegistry(scrollandpanner, scroll, NULL);
#ifndef NORCSID
#endif


scrollandpanner::scrollandpanner()
{
    this->pannerp = new panner;
    THROWONFAILURE( TRUE);
}

scrollandpanner::~scrollandpanner()
{
    (this->pannerp)->Destroy();
}

class scrollandpanner *scrollandpanner::Create(class view  *scrollee, int  location)
{
    class scrollandpanner *retval=NULL;

    retval = new scrollandpanner;
    if (retval==NULL) return NULL;

    (retval)->SetView( scrollee);
    (retval)->SetLocation( location);

    return retval;
}

void scrollandpanner::SetScrollee(class view  *scrollee)
{
    (this)->scroll::SetScrollee( scrollee);
    (this->pannerp)->SetScrollee( scrollee);
}

void scrollandpanner::SetChild(class view  *child)
{
    (this->pannerp)->SetChild( child);
    (this)->scroll::SetChild( this->pannerp);
}

class view *scrollandpanner::GetChild()
{
    return (this->pannerp)->GetChild();
}

