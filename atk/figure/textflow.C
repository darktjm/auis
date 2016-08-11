/*
	Copyright Carnegie Mellon University 1994 - All rights reserved
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

#ifndef NORCSID
char *textflow_c_rcsid = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/figure/RCS/textflow.C,v 1.1 1995/03/23 21:09:49 Zarf Stab74 $";
#endif

#include <andrewos.h>
ATK_IMPL("textflow.H")
#include <textflow.H>

ATKdefineRegistry(textflow, figure, NULL);

textflow::textflow()
{
    ATKinit;

    THROWONFAILURE( TRUE);
}

textflow::~textflow()
{
    ATKinit;
}

char *textflow::ViewName()
{
    return "textflowview";
}
