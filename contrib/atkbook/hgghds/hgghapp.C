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
#include <hgghapp.H>
#include <im.H>
#include <hggh.H>
#include <hgghview.H>
#include <frame.H>


ATKdefineRegistry(hgghapp, application, NULL);


hgghapp::hgghapp()
{
    (this)->SetMajorVersion( 1);
    (this)->SetMinorVersion( 0);
    THROWONFAILURE((TRUE));
}

hgghapp::~hgghapp()
{
}

boolean hgghapp::Start()
{
    class im *im = im::Create(NULL);
    class hggh *h = new hggh;
    class hgghview *hv = new hgghview;
    class frame *f = new frame;

    (hv)->SetDataObject( h);
    (f)->SetView( hv);
    (im)->SetView( f);
    (hv)->WantInputFocus(hv);
    return(TRUE);
}
