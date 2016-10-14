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
#include <hwapp.H> /* Definition of this object (hwapp) */
#include <im.H> /* Import interaction manager definition */
#include <text.H> /* Import text object definition */
#include <textview.H> /* Import textview object definition */


ATKdefineRegistryNoInit(hwapp, application);


boolean hwapp::Start()
{
    class im *im = im::Create(NULL);
    class text *t = new text;
    class textview *tv = new textview;

    if (im == NULL || t == NULL || tv == NULL) {
	/* Object creation failed, return error code */
	return(FALSE);
    }
    (tv)->SetDataObject( t);
    (t)->InsertCharacters( 0, "Hello, world!", 13); 
    (im)->SetView( tv);
    (tv)->WantInputFocus( tv);
    return(TRUE); /* Success */
}
