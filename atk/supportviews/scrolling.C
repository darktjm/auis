/* Copyright 1994 Carnegie Mellon University All rights reserved. */

#include <andrewos.h>
ATK_IMPL("scrolling.H")
#include "scroll.H"
#include <view.H>
#include <environ.H>
#include <sbutton.H>
#include <sbuttonv.H>



static const char * const InterfaceName[scroll_TYPES] = {"scroll,vertical", "scroll,horizontal"};

ScrollInterfaceClassic::ScrollInterfaceClassic(class view *v) {
    vw=v;
    if(vw) {
	fns[scroll_VERT]=(const struct scrollfns *)vw->GetInterface(InterfaceName[scroll_VERT]);
	fns[scroll_HORIZ]=(const struct scrollfns *)vw->GetInterface(InterfaceName[scroll_HORIZ]);
    }
}

ScrollInterfaceClassic::ScrollInterfaceClassic() {
    vw=NULL;
    fns[scroll_VERT]=fns[scroll_HORIZ]=NULL;
}

void ScrollInterfaceClassic::Absolute(long totalx, long x, long totaly, long y) {
    if(vw==NULL) return;
    if(totalx) {
	const struct scrollfns *horiz=fns[scroll_HORIZ];
	struct range total, seen, dot;
	if(horiz==NULL) return;
	horiz->GetInfo(vw, &total, &seen, &dot);

	long posn=(long)((((double)x)/totalx)*(total.end-total.beg))+total.beg;
	
	horiz->SetFrame(vw, posn, 0, vw->GetLogicalWidth());
	
    }

    if(totaly) {
	const struct scrollfns *vert=fns[scroll_VERT];
	struct range total, seen, dot;
	if(vert==NULL) return;
	vert->GetInfo(vw, &total, &seen, &dot);

	long posn=(long)((((double)y)/totaly)*(total.end-total.beg))+total.beg;
	
	vert->SetFrame(vw, posn, 0, vw->GetLogicalHeight());
    }
    
}

void ScrollInterfaceClassic::ScreenDelta(long dx, long dy) {
    if(vw==NULL) return;
    if(dx) {
	const struct scrollfns *horiz=fns[scroll_HORIZ];
	if(horiz==NULL) return;
	if(dx<0) {
	    long posn=horiz->WhatIsAt(vw, -dx, vw->GetLogicalWidth());
	    horiz->SetFrame(vw,posn,0,vw->GetLogicalWidth());
	} else {
	    struct range total, seen, dot;
	    horiz->GetInfo(vw, &total, &seen, &dot);
	    horiz->SetFrame(vw,seen.beg,dx,vw->GetLogicalWidth());
	}
    }
    if(dy) {
	const struct scrollfns *vert=fns[scroll_VERT];
	if(vert==NULL) return;
	if(dy<0) {
	    long posn=vert->WhatIsAt(vw, -dy, vw->GetLogicalHeight());
	    vert->SetFrame(vw,posn,0,vw->GetLogicalHeight());
	} else {
	    struct range total, seen, dot;
	    vert->GetInfo(vw, &total, &seen, &dot);
	    vert->SetFrame(vw,seen.beg,dy,vw->GetLogicalHeight());
	}
    }
      
}

static const struct scrollfns *fns=NULL;
static void DoHorizEndZone(class view *vw, int typeEnd, enum view_MouseAction action) {
    if(fns==NULL || fns->SetFrame==NULL || fns->GetInfo==NULL)  return;
    struct range total, seen, beg;
    fns->GetInfo(vw, &total, &seen, &beg);
    if(action==view_LeftDown || action == view_RightDown) {
	if (typeEnd == scroll_TOPENDZONE)
	    fns->SetFrame(vw, total.beg, 0, vw->GetLogicalWidth());
	else
	    fns->SetFrame(vw, total.end, vw->GetLogicalWidth() >> 2, vw->GetLogicalWidth());
    }
}

static void DoVertEndZone(class view *vw, int typeEnd, enum view_MouseAction action) {
    if(fns==NULL || fns->SetFrame==NULL || fns->GetInfo==NULL)  return;
    struct range total, seen, beg;
    fns->GetInfo(vw, &total, &seen, &beg);
    if(action==view_LeftDown || action == view_RightDown) {
	if (typeEnd == scroll_TOPENDZONE)
	    fns->SetFrame(vw, total.beg, 0, vw->GetLogicalHeight());
	else
	    fns->SetFrame(vw, total.end, vw->GetLogicalHeight() >> 2, vw->GetLogicalHeight());
    }
}


void ScrollInterfaceClassic::Shift(scroll_Direction dir) {
    if(vw==NULL) return;
    const struct scrollfns *vert=fns[scroll_VERT];
    const struct scrollfns *horiz=fns[scroll_HORIZ];
    scroll_endzonefptr vzone=NULL,hzone=NULL;
    if(vert) {
	vzone=vert->EndZone;
	if(vzone==NULL) vzone=DoVertEndZone;
    }
    if(horiz) {
	hzone=horiz->EndZone;
	if(hzone==NULL) hzone=DoHorizEndZone;
    }
    switch(dir) {
	case scroll_Up:
	    ::fns=vert;
	    if(vzone) vzone(vw, scroll_MOTIFTOPENDZONE, view_RightDown);
	    break;
	case scroll_Down:
	    ::fns=vert;
	    if(vzone) vzone(vw, scroll_MOTIFBOTTOMENDZONE, view_RightDown);
	    break;
	case scroll_Left:
	    ::fns=horiz;
	    if(hzone) hzone(vw, scroll_MOTIFTOPENDZONE, view_RightDown);
	    break;
	case scroll_Right:
	    ::fns=horiz;
	    if(hzone) hzone(vw, scroll_MOTIFBOTTOMENDZONE, view_RightDown);
	    break;
	case scroll_None:
	    break;
    }
}

void ScrollInterfaceClassic::Extreme(scroll_Direction dir) {
    if(vw==NULL) return;
    const struct scrollfns *vert=fns[scroll_VERT];
    const struct scrollfns *horiz=fns[scroll_HORIZ];
    scroll_endzonefptr vzone=NULL,hzone=NULL;
    if(vert) {
	vzone=vert->EndZone;
	if(vzone==NULL) vzone=DoVertEndZone;
    }
    if(horiz) {
	hzone=horiz->EndZone;
	if(hzone==NULL) hzone=DoHorizEndZone;
    }
    switch(dir) {
	case scroll_Up:
	    ::fns=vert;
	    if(vzone) vzone(vw, scroll_TOPENDZONE, view_LeftDown);
	    break;
	case scroll_Down:
	    ::fns=vert;
	    if(vzone) vzone(vw, scroll_BOTTOMENDZONE, view_LeftDown);
	    break;
	case scroll_Left:
	    ::fns=horiz;
	    if(hzone) hzone(vw, scroll_TOPENDZONE, view_LeftDown);
	    break;
	case scroll_Right:
	    ::fns=horiz;
	    if(hzone) hzone(vw, scroll_BOTTOMENDZONE, view_LeftDown);
	    break;
	case scroll_None:
	    break;
    }
}

void ScrollInterfaceClassic::UpdateRegions(class scroll &sc) {
    struct range htotal, hseen, hdot;
    struct range vtotal, vseen, vdot;
    const struct scrollfns *vert=fns[scroll_VERT];
    const struct scrollfns *horiz=fns[scroll_HORIZ];
    long vrange=0;
    long vsb = 0, vse = 0;
    long vdb = 0, vde = 0;
    long hrange=0;
    long hsb = 0, hse = 0;
    long hdb = 0, hde = 0;
    if(vert!=NULL) {
	vert->GetInfo(vw, &vtotal,&vseen,&vdot);

	vrange=vtotal.end-vtotal.beg;
	vsb=vseen.beg-vtotal.beg;
	vse=vseen.end-vtotal.beg;
	vdb=vdot.beg-vtotal.beg;
	vde=vdot.end-vtotal.beg;
    }
    if(horiz!=NULL) {
	horiz->GetInfo(vw, &htotal,&hseen,&hdot);

	hrange=htotal.end-htotal.beg;
	hsb=hseen.beg-htotal.beg;
	hse=hseen.end-htotal.beg;
	hdb=hdot.beg-htotal.beg;
	hde=hdot.end-htotal.beg;
    }
    if(sc.Region(scroll_Elevator)) sc.Region(scroll_Elevator)->SetRanges(hrange, hsb, hse, vrange, vsb, vse);
    if(sc.Region(scroll_Dot)) sc.Region(scroll_Dot)->SetRanges(hrange, hdb, hde, vrange, vdb, vde);
}

ScrollRegionElevator::ScrollRegionElevator()  : ScrollRegion(scroll_Range) {
}

ScrollRegionElevator::~ScrollRegionElevator() {
}

void ScrollRegion::Draw(class scroll &scroll, int stype,  view_UpdateType type, const struct rectangle &damaged) {
    struct sbutton_info b;
    b.prefs=baseprefs;
    b.label="";
    currentHighlight=newHighlight;
    b.lit=currentHighlight;
    b.sensitive=TRUE;
    sbuttonv::SafeDrawButton(&scroll,&b,&rect[stype]);
    lastrect[stype]=rect[stype];
}

void ScrollRegion::Update(class scroll &scroll, int stype) {
    struct sbutton_info b;
    b.prefs=baseprefs;
    b.label="";
    currentHighlight=newHighlight;
    b.lit=currentHighlight;
    b.sensitive=TRUE;
    sbuttonv::SafeDrawButton(&scroll,&b, &rect[stype]);
    lastrect[stype]=rect[stype];
}

ScrollRegionDot::ScrollRegionDot()  : ScrollRegion(scroll_Range, FALSE) {
    currentHighlight=newHighlight=TRUE;
}

ScrollRegionDot::~ScrollRegionDot() {
}

ScrollRegionEndzone::ScrollRegionEndzone()  : ScrollRegion(scroll_Endzone, FALSE) {
}

ScrollRegionEndzone::~ScrollRegionEndzone() {
}

void ScrollRegionEndzone::Draw(class scroll &scroll, int stype, view_UpdateType type, const struct rectangle &damaged) {
    Update(scroll, stype);
}

void ScrollRegionEndzone::Update(class scroll &scroll, int stype) {
    struct rectangle t;
    struct sbutton_info b;
    b.prefs=baseprefs;
    currentHighlight=newHighlight;
    b.lit=currentHighlight;
    b.label="";
    b.sensitive=TRUE;
    sbuttonv::SafeDrawButton(&scroll, &b, &rect[stype]);
    t.left = rect[stype].left + rect[stype].width/2 - 3;
    t.width = 6 + (rect[stype].width % 2);
    t.top = rect[stype].top + rect[stype].height/2 - 2;
    t.height = 4 + (rect[stype].height % 2);
    b.lit = !b.lit;
    sbuttonv::SafeDrawButton(&scroll, &b, &t);
    lastrect[stype]=rect[stype];
}

ScrollInterface::ScrollInterface() {
}

ScrollInterface::~ScrollInterface() {
}


ScrollRegion::ScrollRegion(scroll_RegionType rt, boolean elevator, int pri, scroll_Zone z, boolean highlight) {
    type=rt;
    if(rt==scroll_Endzone) {
	data.zone=z;
    } else if(rt==scroll_Range) {
	data.range.xrange=data.range.yrange=1;
	data.range.xstart=data.range.xend=data.range.ystart=data.range.yend=0;
    }
    newHighlight=currentHighlight=highlight;
    isElevator=elevator;
    changed=TRUE;
    for(int i=scroll_VERT;i<=scroll_HORIZ;i++) {
	rect[i].top=rect[i].left=rect[i].width=rect[i].height=0;
	lastrect[i].top=lastrect[i].left=lastrect[i].width=lastrect[i].height=0;
    }
    priority=pri;
    baseprefs=NULL;
    minheight=minwidth=1;
}

ScrollRegion::~ScrollRegion() {
    if(baseprefs) sbutton::FreePrefs(baseprefs);
}


void ScrollRegion::ComputeRect(class scroll &scroll, int stype, const struct rectangle &bar, struct rectangle &area) {
    if(type==scroll_Range) {
	long xrange=data.range.xrange;
	long yrange=data.range.yrange;
	long xend=data.range.xend;
	long yend=data.range.yend;
	if(xrange==0) {
	    xrange++;
	    if(xend==0) xend++;
	}
	if(yrange==0) {
	    yrange++;
	    if(yend==0) yend++;
	}
	if(stype!=scroll_VERT) {
	    area.left=(long)(((double)data.range.xstart) * (bar.width-1)/xrange) + bar.left;
	    if(xend>=0 && data.range.xstart>=0) {
		area.width=(long)(((double)(xend-data.range.xstart))*(bar.width-1)/xrange);
		if(area.width<minwidth) {
		    long x1=area.left-bar.left;
		    long x2=x1+area.width;
		    if (x1 == x2) {
			x1 -= 1;
			x2 += 2;
		    }
		    if (x2 - x1 < minwidth) {
			x1 = (x1 + x2 - minwidth) / 2;
			x2 = (x1 + x2 + minwidth) / 2;
		    }
		    if(x1<0) {
			x2+=(-x1);
			x1=0;
		    }
		    area.left=x1+bar.left;
		    area.width=(x2-x1)+1;
		    if(area.width+area.left>=bar.left+bar.width) {
			long diff=(area.width + area.left) - (bar.left + bar.width);
			area.left-=diff;
			area.width-=diff;
			if(area.left<bar.left) area.left=bar.left;
		    }
		}
	    } else area.width=(-1);
	}
	if(stype!=scroll_HORIZ) {
	    area.top=(long)(((double)data.range.ystart)*(bar.height-1)/yrange)+bar.top;
	    if(yend>=0 && data.range.ystart>=0) {
		area.height=(long)(((double)(yend-data.range.ystart)) * (bar.height-1)/yrange);
		if(area.height<minheight) {
		    long y1=area.top-bar.top;
		    long y2=y1+area.height;
		    if (y1 == y2) {
			y1 -= 1;
			y2 += 2;
		    }
		    if (y2 - y1 < minheight) {
			y1 = (y1 + y2 - minheight) / 2;
			y2 = (y1 + y2 + minheight) / 2;
		    }
		    if(y1<0) {
			y2+=(-y1);
			y1=0;
		    }
		    area.top=y1+bar.top;
		    area.height=(y2-y1)+1;
		    if(area.height+area.top>=bar.top+bar.height) {
			long diff=(area.height + area.top) - (bar.top + bar.height);
			area.top-=diff;
			area.height-=diff;
			if(area.top<bar.top) area.top=bar.top;
		    }
		}
	    } else area.height=(-1);
	}
    }
    if(rect[stype].left!=area.left || rect[stype].width!=area.width || rect[stype].top!=area.top || rect[stype].height!=area.height) {
	changed=TRUE;
	rect[stype]=area;
    }
}

void ScrollRegion::SetHighlight(boolean highlight) {
    newHighlight=highlight;
    if(newHighlight!=currentHighlight) changed=TRUE;
}

class view *ScrollRegion::GetView() {
    return NULL;
}

void ScrollRegion::SetRanges(long xrange, long xstart, long xend, long yrange, long ystart, long yend) {
    if(type!=scroll_Range) return;
    if(xrange) {
	data.range.xrange=xrange;
	data.range.xstart=xstart;
	data.range.xend=xend;
    }
    if(yrange) {
	data.range.yrange=yrange;
	data.range.ystart=ystart;
	data.range.yend=yend;
    }
}

void ScrollRegion::SetBaseSButtonPrefs(struct sbutton_prefs *sbp)
{
    delete baseprefs;
    if(sbp) baseprefs=sbutton::DuplicatePrefs(sbp, NULL);
    else baseprefs=NULL;
}
