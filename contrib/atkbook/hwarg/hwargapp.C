static char *hwargapp_rcsid = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/atkbook/hwarg/RCS/hwargapp.C,v 1.1 1994/05/19 20:43:09 Zarf Stab74 $";

/* **************************************************** *\
Copyright 1989 Nathaniel S. Borenstein
Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear in
supporting documentation, and that the name of 
Nathaniel S. Borenstein not be used in
advertising or publicity pertaining to distribution of the software
without specific, written prior permission. 

Nathaniel S. Borenstein DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
Nathaniel S. Borenstein BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY
DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
\* ***************************************************** */
#include <andrewos.h>
#include <hwargapp.H>
#include <im.H>
#include <text.H>


ATKdefineRegistry(hwargapp, hwapp, NULL);


hwargapp::hwargapp()
{
    this->whattosay = NULL;
    THROWONFAILURE((TRUE));
}

hwargapp::~hwargapp()
{
}

boolean hwargapp::ParseArgs(int  argc, const char  **argv)
{
    if (argc > 1) this->whattosay = argv[1];
    return(TRUE);
}

boolean hwargapp::Start()
{
    if (!(this)->hwapp::Start()) return(FALSE);
    if (this->whattosay) {
	class text *t;

	t = (this)->GetText();
	(t)->Clear();
	(t)->InsertCharacters( 0, this->whattosay, strlen(this->whattosay)); 
    }
    return(TRUE);
}
