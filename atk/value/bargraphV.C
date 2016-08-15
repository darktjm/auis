/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/

#include <andrewos.h>

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/value/RCS/bargraphV.C,v 1.3 1994/11/30 20:42:06 rr2b Stab74 $";
#endif


 


ATK_IMPL("bargraphV.H")
#include <bargraphV.H>
#include <sbuttonv.H>
#include <sliderV.H>
#include <sbutton.H>

#define FUDGE 2
#define FUDGE2 4
#define FUDGE4 8
#define MAXWID 36

#ifndef MIN
#define MIN(A,B) ((A > B)? B:A)
#endif

ATKdefineRegistry(bargraphV, sliderV, NULL);
#ifndef NORCSID
#endif
#ifndef MIN
#endif
static void DrawKnurl(class bargraphV  * self,boolean  fullupdate,struct rectangle  *rr);


static void DrawKnurl(class bargraphV  * self,boolean  fullupdate,struct rectangle  *rr)

{
    long start,height;
    struct rectangle r,clipper;
    class sliderV *ss;
    int offset;
    ss = (class sliderV *) self;
    offset = FUDGE2;
    (self)->SetTransferMode( graphic_COPY);
    height =  ((ss->tmpval - ss->minval)* (rectangle_Height(rr) - FUDGE2)) /(ss->maxval - ss->minval) ;
    start = rectangle_Height(rr) - height - FUDGE2; 
    rectangle_SetRectSize(&r,rectangle_Left(rr) + FUDGE,
			  rectangle_Top(rr) +  start, 
			  rectangle_Width(rr) - FUDGE2,height);
    if(!fullupdate){
	clipper.top = MIN(r.top,self->lasttop)- FUDGE2 ;
	clipper.left = rr->left;
	clipper.height = abs(r.top - self->lasttop)+ FUDGE4;
	clipper.width = rr->width + FUDGE4;
	(self)->SetClippingRect(&(clipper));
    }
    (self)->EraseRectSize(rr->left,rr->top,rr->width + FUDGE4,rr->height);
    (self)->FillTrapezoid(r.left,r.top + r.height, r.width,
			 r.left	+ offset, r.top	+ r.height + offset, r.width,
			 (self)->GrayPattern(8,16));
    (self)->FillTrapezoid(r.left + r.width ,r.top , 0,
			 r.left + r.width,  r.top + offset,offset,
			 (self)->GrayPattern(12,16));
    (self)->FillTrapezoid(r.left + r.width ,r.top + r.height , offset,
			 r.left + r.width +offset , r.top + r.height + offset,0,
			 (self)->GrayPattern(12,16));
    (self)->FillRectSize(r.left + r.width ,r.top + offset,
			offset,r.height - offset,(self)->GrayPattern(12,16));
    (self)->MoveTo(r.left + r.width,r.top );
    (self)->DrawLine(offset,offset);(self)->DrawLine(0,r.height);
    (self)->DrawLine(-r.width,0);(self)->DrawLine(-offset,-offset);
    (self)->MoveTo(r.left + r.width,r.top + r.height);
    (self)->DrawLine(offset,offset);
    (self)->DrawRect( &r);
    if(!fullupdate)
	(self)->ClearClippingRect();
    self->lasttop = r.top;
}

void bargraphV::Drawslider(boolean  fullupdate,struct rectangle  *rr)
{
    int start,height;
    struct rectangle interior,exterior,cl,clside,clipper;
    class sliderV *ss;
    struct sbuttonv_view_info vi;
    boolean pushed = TRUE; /* should be TRUE, change when sbutton is fixed */
    ss = (class sliderV *) this;
    if( ss->prefs->style == 2) ss->prefs->style = 4;
    if( ss->prefs->style != 4){
	DrawKnurl(this,fullupdate,rr);
	return;
    }
    exterior = *rr;
    clside = *rr;
    cl = *rr;
    height =  ((ss->tmpval - ss->minval )* (rectangle_Height(&exterior))) / (ss->maxval - ss->minval);
    height += (4 *  ( rr->height - height)) / rr->height ;
    start = rectangle_Top(&exterior) + rectangle_Height(&exterior) - height;
    exterior.height = height;
    exterior.top = start;
    cl.height = cl.height + 1 -  exterior.height ;
    sbuttonv::DrawBorder(this, exterior.left,exterior.top,
			 exterior.width, exterior.height,
			 ss->prefs,pushed,FALSE,&interior);
    clside = exterior;
    clside.width = interior.left - exterior.left;
    if(!fullupdate){
	clipper.top = MIN(exterior.top,this->lasttop)- FUDGE2 ;
	clipper.left = exterior.left;
	clipper.height = abs(exterior.top - this->lasttop)+ FUDGE4;
	clipper.width = exterior.width;
	(this)->SetClippingRect(&(clipper));
    }
    if(fullupdate || (this->lasttop > exterior.top)){
	sbuttonv::SaveViewState( this, &vi);
	sbuttonv::DrawBorder(this, exterior.left,exterior.top,
			    exterior.width, exterior.height,
			    ss->prefs,!pushed,TRUE,&interior);
	sbuttonv::RestoreViewState( this, &vi);
    }
    if(cl.height > 0) (this)->EraseRect(&cl);
    if(clside.width > 0)  (this)->EraseRect(&clside);
    (this)->FillTrapezoid(interior.left + interior.width,interior.top - 1,exterior.left + exterior.width - (interior.left + interior.width),exterior.left + exterior.width,interior.top + (interior.top  - exterior.top),0,(this)->WhitePattern());
    if(!fullupdate)
	(this)->ClearClippingRect();

    this->lasttop = exterior.top;

}
bargraphV::bargraphV()
{
    class sliderV *ss;
    ss = (class sliderV *) this;
    ss->readonlydefault = TRUE;
    THROWONFAILURE( TRUE);
}
void bargraphV::DrawFromScratch(long  x,long  y,long  width,long  height)
{
    class sliderV *ss;
    ss = (class sliderV *) this;

    ss->sliderwidth = width - FUDGE4 - x - x;
    if( ss->sliderwidth > MAXWID){
	ss->sliderwidth = MAXWID;
    }
    (this)->sliderV::DrawFromScratch(x,y,width,height);
}
