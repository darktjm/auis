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
#include <hwview.H>
#include <graphic.H>
#include <fontdesc.H>


ATKdefineRegistry(hwview, view, NULL);


void hwview::FullUpdate(enum view_UpdateType  type, long  left , long  top , long  width , long  height)
{
    struct rectangle Rect;
    class fontdesc *font;

    (this)->SetTransferMode( graphic::COPY); /* set transfer mode to default */

    (this)->GetVisualBounds( &Rect);
    (this)->MoveTo( Rect.left + Rect.width/2,
		   Rect.top + Rect.height/2);
    (this)->DrawString( "Hello, world!",
	graphic::BETWEENTOPANDBASELINE
	| graphic::BETWEENLEFTANDRIGHT);

    font = fontdesc::Create("andy", fontdesc_Bold, 20);
    /* Note that creating this font on every 
      FullUpdate leaks core.  We should create
      it in InitializeObject and re-use it. */
    (this)->SetFont( font);

    (this)->MoveTo( Rect.left + Rect.width/2,
		   Rect.top + Rect.height/2 + 20);
    (this)->DrawText( "(big stuff)garbage", 11,
	graphic::BETWEENTOPANDBASELINE
	| graphic::BETWEENLEFTANDRIGHT);

    Rect.left = Rect.left + Rect.width/4;
    Rect.width = Rect.width/2;
    Rect.top = Rect.top + Rect.height/4;
    Rect.height = Rect.height/2;
    (this)->DrawRect( &Rect);

    (this)->SetLineWidth( 4);
    (this)->DrawRectSize( 10, 20, 25, 40);
    (this)->SetLineWidth( 1);

    (this)->DrawOvalSize( 30, 30, 25, 25);

    (this)->DrawRRectSize( 50, 50, 100, 50, 8, 8);

    (this)->DrawArc( &Rect, 45, 90);

    (this)->MoveTo( 5, 5);
    (this)->DrawLineTo( 25, 25);

    (this)->FillRectSize( Rect.left, 0, Rect.width,
		Rect.top, (this)->GrayPattern( 3, 10));
    (this)->FillRectSize( Rect.left + Rect.width/4,
		Rect.top/4, Rect.width/2, Rect.top/2,
		(this)->BlackPattern());

    (this)->MoveTo( Rect.left+Rect.width/4, Rect.top/4);
    (this)->SetTransferMode( graphic::WHITE);
    (this)->DrawString( "White Left",
		       graphic::ATLEFT | graphic::ATTOP);

    (this)->SetTransferMode( graphic::INVERT);
    (this)->MoveTo( Rect.left+(3*Rect.width)/4 - 30,
		   Rect.top/4);
    (this)->DrawString( "Inverted text example",
		       graphic::ATLEFT | graphic::ATTOP);
    
}


