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
#include <fancier.H>
#include <style.H>
#include <stylesheet.H>
#include <fontdesc.H>


ATKdefineRegistryNoInit(fancier, fancy);


fancier::fancier()
{
    class style *newstyle;
    
    newstyle = new style;
    (newstyle)->AddNewFontFace( fontdesc_Bold);
    (newstyle)->AddNewFontFace( fontdesc_Italic);
    (newstyle)->SetFontSize( style_ConstantFontSize, 20);
    (newstyle)->SetJustification( style_RightJustified);
    (newstyle)->SetNewLeftMargin( style_PreviousIndentation,
			    2, style_Inches);
    (this)->AddStyle( 152, 65, newstyle);
    /* Wrapped this style around 65 specific characters */

    (newstyle)->SetName( "stupidstyle");
    (newstyle)->SetMenuName( "Stupidity,That's My Style");
    ((this)->GetStyleSheet())->Add( newstyle);

    THROWONFAILURE((TRUE));
}

fancier::~fancier()
{
}

