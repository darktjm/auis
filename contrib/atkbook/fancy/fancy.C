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
#include <fancy.H>
#include <stylesheet.H>
#include <style.H>
#include <environment.H>

static char *InsertionText = "\n\n\n\nHello, world!\n\nYou are viewing a demonstration of how the Andrew Toolkit can be used to produce interesting and complex multi-font formatted text.\n\nThis should prove useful in understanding how to make fancy text.\n(At least, that's the intent.)\n";


ATKdefineRegistry(fancy, text, NULL);


fancy::fancy()
{
    class style *styletmp;
    class environment *envtmp;

    (this)->InsertCharacters( 0, InsertionText,
				strlen(InsertionText));
    (this)->ReadTemplate( "help", FALSE);
    styletmp = ((this)->GetStyleSheet())->Find(
	 "center");
    (this)->SetGlobalStyle( styletmp);
    styletmp = ((this)->GetStyleSheet())->Find(
	 "bold");
    envtmp = (this)->AddStyle( 4, 5, styletmp);
    (envtmp)->SetStyle( TRUE, TRUE);
    THROWONFAILURE((TRUE));
}

fancy::~fancy()
{
}

char *fancy::ViewName()
{
    return("textview");
}
