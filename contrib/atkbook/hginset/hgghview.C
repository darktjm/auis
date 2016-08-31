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
#include <hgghview.H>
#include <im.H>
#include <view.H>
#include <text.H>
#include <textview.H>
#include <lpair.H>
#include <butt.H>
#include <buttview.H>




ATKdefineRegistry(hgghview, lpair, NULL);
static void ToggleLpairViews(class hgghview  *self, class lpair  *lp, class butt  *b, enum view_MouseAction  action);


hgghview::hgghview()
{
    class text *t1 = new text;
    class text *t2 = new text;
    class textview *tv1 = new textview;
    class textview *tv2 = new textview;
    class lpair *lp = new lpair;
    class butt *b = new butt;
    class buttview *bv = new buttview;

    (tv1)->SetDataObject( t1);
    (tv2)->SetDataObject( t2);
    (bv)->SetDataObject( b);
    (t1)->InsertCharacters( 0, "Hello, world!", 13);
    (t2)->InsertCharacters( 0, "Goodbye, world!", 15);
    (b)->SetText( "Toggle");
    (bv)->SetHitFunction( (buttview_hit_fptr)ToggleLpairViews);
    (bv)->SetRocks( this, lp);
    (lp)->SetUp( tv1, tv2, 50, lpair_PERCENTAGE,
		 lpair_HORIZONTAL, TRUE);
    (this)->SetUp( bv, lp, 25, lpair_TOPFIXED,
		    lpair_HORIZONTAL, TRUE);
    (tv1)->WantInputFocus( tv1);
    THROWONFAILURE((TRUE));
}

hgghview::~hgghview()
{
}

static void ToggleLpairViews(class hgghview  *self, class lpair  *lp, class butt  *b, enum view_MouseAction  action)
{
    class view *v1, *v2;

    if (action == view_LeftDown || action == view_RightDown) {
	v1 = (lp)->GetNth( 0);
	v2 = (lp)->GetNth( 1);
	(lp)->SetNth( 0, v2);
	(lp)->SetNth( 1, v1);
	(lp)->WantUpdate( lp);
	(v2)->WantInputFocus( v2);
    }
}
