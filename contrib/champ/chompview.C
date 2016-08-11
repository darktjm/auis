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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/champ/RCS/chompview.C,v 1.6 1994/08/11 03:02:28 rr2b Stab74 $";
#endif

#include <andrewos.h>	/* time.h */

#include <chompview.H>
#include <month.H>
#include <monthview.H>
#include <lpair.H>
#include <textview.H>
#include <im.H>
#include <event.H>
#include <scroll.H>



ATKdefineRegistry(chompview, view, NULL);
#ifndef NORCSID
#endif


chompview::chompview()
{
    int mon, yr;

    this->lp1 = new lpair;
    this->toplpair = new lpair;
    this->tv = new textview;
    this->m1 = new month;
    this->m2 = new month;
    this->mv1 = new monthview;
    this->mv2 = new monthview;
    this->s = new scroll;
    if (!this->lp1 || !this->lp1 || !this->tv || !this->m1 || !this->mv1 || !this->m2 || !this->mv2 || !this->s) THROWONFAILURE((FALSE));
    mon = (this->m2)->GetMonth();
    yr = (this->m2)->GetYear();
    if (++mon >= 12) {
	mon = 0;
	++yr;
    }
    (this->m2)->SetMonthAndYear( mon, yr);
    (this->mv1)->SetDataObject(this->m1);
    (this->mv2)->SetDataObject(this->m2);
    (this->mv1)->SetTextview( this->tv);
    (this->mv2)->SetTextview( this->tv);
    (this->mv2)->ResetMonth( FALSE);
    (this->mv1)->ResetMonth( FALSE); /* This goes second to "win" the textview */
    (this->s)->SetView( this->tv);
    (this->lp1)->VTFixed( this->mv1, this->mv2, 125, 1);
    (this->toplpair)->HTFixed( this->lp1, this->s, 175, 1);
    THROWONFAILURE((TRUE));
}

chompview::~chompview()
{
    (this->toplpair)->Destroy();
    (this->lp1)->Destroy();
    (this->s)->Destroy();
    (this->tv)->Destroy();
    (this->m1)->Destroy();
    (this->m2)->Destroy();
    (this->mv1)->Destroy();
    (this->mv2)->Destroy();
}

view_DSattributes chompview::DesiredSize(long  width, long  height, enum view_DSpass  pass, long  *dWidth, long  *dheight)
{
    *dWidth = 1000;
    *dheight = 250;
    return(view_WidthFlexible | view_HeightFlexible);
}

void chompview::FullUpdate(enum view_UpdateType  type, long  left, long  top, long  width, long  height)
{
    struct rectangle Rect;

    if((type == view_LastPartialRedraw) || (type == view_FullRedraw)) {
	(this)->GetLogicalBounds( &Rect);
	(this)->DrawRectSize( Rect.left, Rect.top, Rect.width-1, Rect.height-1);
	Rect.left += 2;
	Rect.top += 2;
	Rect.width -= 4;
	Rect.height -= 4;
	(this->toplpair)->InsertView( this, &Rect);
	(this->toplpair)->FullUpdate( type, left, top, width, height);
    }
}

class view *chompview::Hit(enum view_MouseAction  action, long  x , long  y , long  numberOfClicks)
{
    return((this->toplpair)->Hit( action, x, y, numberOfClicks));
}

void chompview::Print(FILE  *f, char  *process, char  *final, int  toplevel)
{
    (this->tv)->Print( f, process, final, toplevel);
}

void chompview::LinkTree(class view  *parent)
{
    (this)->view::LinkTree( parent);
    if(parent && this->GetIM() && this->toplpair)
	(this->toplpair)->LinkTree(this);
}
