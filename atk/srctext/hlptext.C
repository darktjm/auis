/* File hlptext.C created by R S Kemmetmueller
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

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

   hlptext, a text subclass with dogtags, for creating help files. */

#include <andrewos.h>

static UNUSED const char ibmid[] = "(c) Copyright IBM Corp.  1988-1995.  All rights reserved.";
static UNUSED const char rcsHeader[] = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/srctext/RCS/hlptext.C,v 2.0 1995/01/27 19:38:38 rr2b Stab74 $";

#include "dogtags.h"
#include "hlptext.H"

ATKdefineRegistry(hlptext, text, NULL);

hlptext::hlptext()
{
    SetCopyAsText(TRUE);
    THROWONFAILURE(TRUE);
}

long hlptext::Read(FILE *file, long id)
{
    long tmpRetValue;

    tmpRetValue = (this)->text::Read(file, id);
    if (tmpRetValue == dataobject_NOREADERROR) {
	dogtags_substitute(this);
    }
    return tmpRetValue;
}
