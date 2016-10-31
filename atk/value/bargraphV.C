/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
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

ATKdefineRegistryNoInit(bargraphV, sliderV);
static void DrawKnurl(class bargraphV  * self,boolean  fullupdate,struct rectangle  *rr);


static void DrawKnurl(class bargraphV  * self,boolean  fullupdate,struct rectangle  *rr)

{
    long start,height;
    struct rectangle r,clipper;
    class sliderV *ss;
    int offset;
    ss = (class sliderV *) self;
    offset = FUDGE2;
    (self)->SetTransferMode( graphic::COPY);
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

// from sbuttonv.C
#define BUTTONDEPTH 4
#define MOTIFBUTTONDEPTH 2
#define TEXTPAD 2

view::DSattributes bargraphV::DesiredSize(long  width , long  height, enum view::DSpass  pass, long  *desiredwidth , long  *desiredheight)
{
    if(!this->fontname) // params haven't been read yet; no idea what to return
	return valueview::DesiredSize(width, height, pass, desiredwidth, desiredheight);
    int ret = view::WidthLarger|view::HeightLarger;
    struct FontSummary *fs;
    char buf[30];
    long labelwidth, valheight,valwidth,valwidth1,junk;
    fs = boldfont->FontSummary(GetDrawable());
    // technically, this is wrong:  neither minval nor maxval is necessarily the widest
    // close enough for me, though.
    sprintf(buf,"%ld",maxval);
    boldfont->StringSize(GetDrawable(), buf,&(valwidth),&(junk));
    sprintf(buf,"%ld",minval);
    boldfont->StringSize(GetDrawable(), buf,&(valwidth1),&(junk));
    if(valwidth1 > valwidth) valwidth = valwidth1;
    valheight = ( fs->newlineHeight == 0) ? fs->maxHeight : fs->newlineHeight;
    if(label){
	(boldfont)->StringSize(GetDrawable(), label,&(labelwidth),&(junk));
	if(labelwidth > valwidth)
	    valwidth = labelwidth;
	valheight *= 2;
    }
    valwidth += (x + FUDGE2) * 2;
    // the minimum depends on style & sbutton parameters
    // should probably just call sbuttonv::DesiredSize on a properly
    // configured button, but for now just copy some sbuttonv logic
    int sliderw = 5, sliderh = 5; // an arbitrary min size
    if((maxval - minval)/increment + 1 > sliderh)
	sliderh = (maxval - minval) / increment + 1; // at least 1 pixel per step
    int pad;
    if(prefs->style == sbutton_PLAIN)
	pad = 0;
    else if(prefs->style == sbutton_BOXEDRECT)
	pad = TEXTPAD;
    else {
	pad = prefs->style == sbutton_MOTIF ? MOTIFBUTTONDEPTH : BUTTONDEPTH;
	if(prefs->bdepth > 0)
	    pad = prefs->bdepth;
    }
    sliderw += 2 * pad;
    sliderh += 2 * pad;
    if(valwidth < sliderw)
	valwidth = sliderw;
    if(pass != view::WidthSet) {
	if(valwidth > 75)
	    *desiredwidth = valwidth;
	else {
	    ret |= view::WidthSmaller;
	    *desiredwidth = 75;
	}
    } else
	*desiredwidth = MAX(valwidth, sliderw);
    *desiredheight = valheight + sliderh;
    
    return (view::DSattributes)ret ;
}
