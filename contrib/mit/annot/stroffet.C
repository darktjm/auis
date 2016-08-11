/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
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
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/annot/RCS/stroffet.C,v 1.3 1994/08/11 03:03:00 rr2b Stab74 $";
#endif


static char *stroffet_rcsid = "$Header";

#include <andrewos.h>

#include <text.H>
#include <sys/types.h>
#include <pwd.h>
#include <stroffet.H>
#define WIDTH 438
#define HEIGHT 100

/****************************************************************/
/*		private functions				*/
/****************************************************************/



/****************************************************************/
/*		class procedures				*/
/****************************************************************/

ATKdefineRegistry(stroffet, icon, stroffet::InitializeClass);
#ifndef NORCSID
#endif


boolean
stroffet::InitializeClass()
    {
    return TRUE;
}

stroffet::stroffet()
{
	ATKinit;

    class text * to;

    to = new text;
    (to)->SetReadOnly(0);
    (this)->SetChild(to);
    (this)->SetTitle("FomatNote: Troff Commands");
    (this)->SetSize( WIDTH, HEIGHT);
    THROWONFAILURE( TRUE);
}

/****************************************************************/
/*		instance methods				*/
/****************************************************************/

void
stroffet::SetChild(class dataobject  * child)
        {
    (this)->icon::SetChild(child);
    if (child != (class dataobject *)0)
	((class text *)child)->SetReadOnly( 0);
}
