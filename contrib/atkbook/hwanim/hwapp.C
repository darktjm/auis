static char *hwapp_rcsid = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/atkbook/hwanim/RCS/hwapp.C,v 1.2 1994/08/11 03:01:51 rr2b Stab74 $";

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
#include <hwapp.H>
#include <im.H>
#include <text.H>
#include <textview.H>


ATKdefineRegistry(hwapp, application, NULL);


hwapp::hwapp()
{
    this->t = NULL;
    this->tv = NULL;
    (this)->SetMajorVersion( 1);
    (this)->SetMinorVersion( 0);
    THROWONFAILURE((TRUE));
}

hwapp::~hwapp()
{
    if (this->t != NULL) (this->t)->Destroy();
    if (this->tv != NULL) (this->tv)->Destroy();
}

boolean hwapp::Start()
{
    class im *im = im::Create(NULL);

    this->t = new text;
    this->tv = new textview;
    if (this->t == NULL || this->tv == NULL) return(FALSE);
    (this->tv)->SetDataObject( this->t);
    (this->t)->InsertCharacters( 0, "Hello world.", 12); 
    (im)->SetView( this->tv);
    (this->tv)->WantInputFocus( this->tv);
    return(TRUE);
}

class text * hwapp::GetText()
{
    return(this->t);
}
