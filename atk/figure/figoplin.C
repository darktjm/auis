/* figoplin.c - fig element object: polyline */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/
#include <andrewos.h>
ATK_IMPL("figoplin.H")
#include <math.h>

#include <figoplin.H>

#include <figattr.H>
#include <view.H>
#include <figview.H>
#include <figure.H>
#include <figtoolview.H>
#include <message.H>
#include <print.H>

#include <rect.h>

#define TWOPI (6.28318530718)
#define ClearOldPoints(self)  ((((self)->orpts) ? (free((self)->orpts), 1) : 0), ((self)->orpts) = NULL)



static struct point *ptemp;
static int ptemp_size;


ATKdefineRegistry(figoplin, figobj, figoplin::InitializeClass);
static void SetNumPts(class figoplin  *self, long  num);
static void PartialSketch(class figoplin  *self, class figview  *v, long  ptref);
static void RegularizePolygon(class figoplin  *self, long  endx , long  endy);
static int FindLineHit(class figoplin  *self, long  x , long  y, long  delta);
static void MoveHandle(class figoplin  *self, long  x , long  y , long  ptref);


boolean figoplin::InitializeClass()
{
    ptemp_size = 0;
    ptemp = NULL;

    return TRUE;
}

figoplin::figoplin()
{
    ATKinit;

    (this)->AttributesUsed() = (1<<figattr_LineWidth) | (1<<figattr_LineStyle) | (1<<figattr_Color); /* add (1<<figattr_Shade) if closed is true, and ((1<<figattr_ArrowPos) | (1<<figattr_Arrow) | (1<<figattr_ArrowSize)) if closed is false. The best way to do this is to call SetClosed() once you know which you want. */

    (this)->SetNumHandles(8);
    (this)->SetNumHandles(4);

    this->stickysketch = FALSE;
    this->shortmode = FALSE;
    /*this->closed = FALSE;*/
    this->SetClosed(FALSE);
    this->regular = 0;
    this->fliph = FALSE;
    this->flipv = FALSE;
    this->dummysides = 6;

    this->buildstate = 0;
    this->pts = NULL;
    this->orpts = NULL;
    this->numpts = 0;
    this->pts_size = 0;
    SetNumPts(this, 4);
    SetNumPts(this, 0);

    THROWONFAILURE(TRUE);
}

class figoplin *figoplin::Create(struct point  *pointlist, long  numpoints, boolean  isclosed)
{
    ATKinit;

    int ix;
    class figoplin *res = new figoplin;
    if (!res) return NULL;
    if (numpoints<2 || !pointlist) return NULL;

    SetNumPts(res, numpoints);
    for (ix=0; ix<numpoints; ix++) {
	(res)->Pts()[ix].x = pointlist[ix].x;
	(res)->Pts()[ix].y = pointlist[ix].y;
    }
    (res)->SetClosed(isclosed);

    (res)->RecomputeBounds();

    return res;
}

/* sets the closed flag, and the AttributesUsed value to match. */
void figoplin::SetClosed(boolean newval)
{
    if (newval) {
	this->closed = TRUE;
	this->AttributesUsed() |= (1<<figattr_Shade);
	this->AttributesUsed() &= ~((1<<figattr_Arrow) |(1<<figattr_ArrowPos) | (1<<figattr_ArrowSize));
    }
    else {
	this->closed = FALSE;
	this->AttributesUsed() |= ((1<<figattr_Arrow) |(1<<figattr_ArrowPos) | (1<<figattr_ArrowSize));
	this->AttributesUsed() &= ~(1<<figattr_Shade);
    }
}

figoplin::~figoplin()
{
	ATKinit;

    if (this->pts)
	free(this->pts);
    if (this->orpts)
	free(this->orpts);
}

const char *figoplin::ToolName(class figtoolview  *v, long  rock)
{
    if (rock & 2) {
	if (rock & 1)
	    return "Reg N-gon";
	else
	    return "Polygon";
    }

    if (rock & 1)
	return "Line";
    else
	return "Polyline";
}

/* ### ought to be inherited from figobj, probably. Or maybe not. */
void figoplin::CopyData(class figoplin  *src) 
{   
    int ix, num;
    class figattr *vtmp;

    num = (src)->NumPts();
    SetNumPts(this, num);

    for (ix=0; ix<num; ix++) {
	this->pts[ix].x = src->pts[ix].x;
	this->pts[ix].y = src->pts[ix].y;
    }
    this->closed = src->closed;
    (this)->AttributesUsed() = (src)->AttributesUsed();

    vtmp = (src)->GetVAttributes();
    (this)->UpdateVAttributes(vtmp, figattr_MaskAll);

    (this)->RecomputeBounds();
    (this)->SetModified();
}   

void figoplin::ToolModify(class figtoolview  *v, long  rock, boolean  firsttime) 
{
    char buffer[32];
    char obuffer[256];
    int ix;

    if (rock != 3) {
	return;
    }

    if (firsttime) {
	message::DisplayString(v, 10, "Click on this tool again to change the number of sides.");
	return;
    }
    /* else not first time */

    sprintf(obuffer, "How many sides? [%d]: ", this->dummysides);
    ix = message::AskForString(v, 40, obuffer, NULL, buffer, 30); 

    if (ix<0) {
	message::DisplayString(v, 10, "Cancelled.");
	return;
    }

    if (strlen(buffer)!=0) {
	ix = atol(buffer);
	if (ix<3)  {
	    message::DisplayString(v, 10, "The value must be 3 or greater.");
	    return;
	}
	if (ix>32)  {
	    message::DisplayString(v, 10, "The value must be 32 or less.");
	    return;
	}
	this->dummysides = ix;
    }

    sprintf(obuffer, "%d-sided polygons.", this->dummysides);
    message::DisplayString(v, 10, obuffer);
}

class figobj *figoplin::Instantiate(class figtoolview  *v, long  rock) 
{
    class figoplin *res = (class figoplin *)(this)->figobj::Instantiate(v, rock);

    if (rock & 1)
	res->shortmode = TRUE;

    if (rock & 2) {
	res->SetClosed(TRUE);
	if (res->shortmode)
	    res->regular = this->dummysides;
    }
    else {
	res->SetClosed(FALSE);
    }

    return (class figobj *)res;
}

static void SetNumPts(class figoplin  *self, long  num)
{
    ClearOldPoints(self);

    if (num > self->pts_size) {
	if (self->pts_size == 0) {
	    self->pts_size = num;
	    self->pts = (struct point *)malloc(self->pts_size * sizeof(struct point));
	}
	else {
	    while (num > self->pts_size)
		self->pts_size *= 2;
	    self->pts = (struct point *)realloc(self->pts, self->pts_size * sizeof(struct point));
	}
    }
    self->numpts = num;

    num += 1;
    if (num > ptemp_size) {
	if (ptemp_size == 0) {
	    ptemp_size = num;
	    ptemp = (struct point *)malloc(ptemp_size * sizeof(struct point));
	}
	else {
	    while (num > ptemp_size)
		ptemp_size *= 2;
	    ptemp = (struct point *)realloc(ptemp, ptemp_size * sizeof(struct point));
	}
    }
}

/* set bounding box and handle list in fig coordinates */
void figoplin::RecomputeBounds()
{
    long basex, basey, left, right, top, bot, wid, hgt;
    long lwid, awid;
    int ix;
    short bleft, btop;

    (this)->SetNumHandles(this->numpts+4);
    basex = this->pts[0].x;
    basey = this->pts[0].y;
    (this)->SetHandle(4, basex, basey);

    left = right = top = bot = 0;
    for (ix=1; ix<this->numpts; ix++) {
	if (this->pts[ix].x < left)
	    left = this->pts[ix].x;
	if (this->pts[ix].x > right)
	    right = this->pts[ix].x;
	if (this->pts[ix].y < top)
	    top = this->pts[ix].y;
	if (this->pts[ix].y > bot)
	    bot = this->pts[ix].y;
	(this)->SetHandle(ix+4, basex + this->pts[ix].x, basey + this->pts[ix].y);
    }
    bleft = (this->fliph) ? 1 : 0;
    btop = (this->flipv) ? 2 : 0;
    (this)->SetHandle(bleft+btop, basex+left, basey+top);
    (this)->SetHandle((1-bleft)+btop, basex+right, basey+top);
    (this)->SetHandle(bleft+(2-btop), basex+left, basey+bot);
    (this)->SetHandle((1-bleft)+(2-btop), basex+right, basey+bot);
    wid = right - left;
    hgt = bot - top;

    lwid = ((this)->GetVAttributes())->GetLineWidth((this)->GetIVAttributes());
    lwid = lwid*figview_FigUPerPix;
    if (!this->closed && ((this)->GetVAttributes())->GetArrowPos((this)->GetIVAttributes())) {
	awid = ((this)->GetVAttributes())->GetArrowSize((this)->GetIVAttributes());
	awid = (awid*figview_FigUPerPix)*2;
	if (awid > lwid)
	    lwid = awid;
    }

    (this)->SetBoundsRect(left+basex-lwid/2, top+basey-lwid/2, wid+lwid+1, hgt+lwid+1);

    (this)->ComputeSelectedBounds();

    (this)->UpdateParentBounds();
}

static enum figobj_HandleType handletypes[4]={
    figobj_ULCorner,
    figobj_URCorner,
    figobj_LLCorner,
    figobj_LRCorner
};

enum figobj_HandleType figoplin::GetHandleType(long  num)
{
    if(num>=0 && num<=3) return handletypes[num];
    else return figobj_None;
}

static long canonical_poly[] = {
    0, 3, figobj_NULLREF
};
static long canonical_line[] = {
    4, 5, figobj_NULLREF
};

long *figoplin::GetCanonicalHandles()
{
    if ((this)->NumPts()==2)
	return canonical_line;
    else
	return canonical_poly;
}

void figoplin::Draw(class figview  *v) 
{
    long basex, basey;
    long ix, shad, dash, lw, asize, ashape, apos;
    const char *col;

    (v)->SetTransferMode(graphic::COPY);

    col = ((this)->GetVAttributes())->GetColor((this)->GetIVAttributes());
    (v)->SetForegroundColor(col, 0, 0, 0); 

    (v)->SetLineJoin(graphic::JoinBevel);

    lw = ((this)->GetVAttributes())->GetLineWidth((this)->GetIVAttributes());
    lw = (v)->ToPixW(lw*figview_FigUPerPix);
    if (lw <= 0) lw = 1;
    if (lw != 1)
	(v)->SetLineWidth(lw);

    dash = ((this)->GetVAttributes())->GetLineStyle( (this)->GetIVAttributes());
    if (dash != figattr_LineSolid) {
	char *patterns = figattr::LineStylePattern(dash, lw);
	(v)->SetLineDash((unsigned char *)patterns, 0, graphic::LineOnOffDash);
    }

    basex = this->pts[0].x;
    basey = this->pts[0].y;

    ptemp[0].x = (v)->ToPixX(basex);
    ptemp[0].y = (v)->ToPixY(basey);

    if (this->numpts < 2) {
	(v)->MoveTo(ptemp[0].x, ptemp[0].y);
	(v)->DrawLineTo(ptemp[0].x+1, ptemp[0].y+1);
    }
    else {

	for (ix=1; ix<this->numpts; ix++) {
	    ptemp[ix].x = (v)->ToPixX(basex+this->pts[ix].x);
	    ptemp[ix].y = (v)->ToPixY(basey+this->pts[ix].y);
	}
	if (this->closed) {
	    shad = ((this)->GetVAttributes())->GetShade((this)->GetIVAttributes());
	    if (shad != figattr_ShadeClear)
		(v)->FillPolygon(ptemp, this->numpts, (v)->GrayPattern(shad, figattr_ShadeDenom));
	}

	if (!(this)->Closed()) {
	    apos = ((this)->GetVAttributes())->GetArrowPos((this)->GetIVAttributes());
	    if (apos) {
		asize = ((this)->GetVAttributes())->GetArrowSize((this)->GetIVAttributes());
		asize = asize*figview_FigUPerPix;
		ashape = ((this)->GetVAttributes())->GetArrow((this)->GetIVAttributes());
	    }
	    else
		asize = 0;

	    if (asize > 0) {
		int workpt;
		long parx, pary;

		if (apos & figattr_ArrowTail) {
		    if (numpts>2) {
			workpt = this->numpts-1;
			parx = (this->pts[workpt].x - this->pts[workpt-1].x);
			pary = (this->pts[workpt].y - this->pts[workpt-1].y);
		    }
		    else {
			workpt = 1;
			parx = (this->pts[workpt].x);
			pary = (this->pts[workpt].y);
		    }

		    figattr::DrawArrowHead(v, ptemp[workpt].x, ptemp[workpt].y, &parx, &pary, ashape, asize);

		    ptemp[workpt].x = (v)->ToPixX(basex+this->pts[workpt].x-parx);
		    ptemp[workpt].y = (v)->ToPixY(basey+this->pts[workpt].y-pary);
		}
		if (apos & figattr_ArrowHead) {
		    parx = (-(this->pts[1].x));
		    pary = (-(this->pts[1].y));

		    figattr::DrawArrowHead(v, ptemp[0].x, ptemp[0].y, &parx, &pary, ashape, asize);

		    ptemp[0].x = (v)->ToPixX(basex-parx);
		    ptemp[0].y = (v)->ToPixY(basey-pary);
		}
	    }
	    (v)->DrawPath(ptemp, this->numpts);
	}
	else {
	    ptemp[this->numpts].x = ptemp[0].x;
	    ptemp[this->numpts].y = ptemp[0].y;
	    (v)->DrawPath(ptemp, this->numpts+1);
	}
    }

    if (lw != 1)
	(v)->SetLineWidth(1);
    if (dash!=figattr_LineSolid)
	(v)->SetLineDash(NULL, 0, graphic::LineSolid);
    (v)->SetLineJoin(graphic::JoinMiter);
}

void figoplin::Sketch(class figview  *v) 
{
    long x, y, basex, basey;
    long ix;

    if (this->numpts < 2) {
	return;
    }

    basex = this->pts[0].x;
    basey = this->pts[0].y;

    (v)->SetTransferMode(graphic::INVERT);

    (v)->MoveTo((v)->ToPixX(basex), (v)->ToPixY(basey));
    for (ix=1; ix<this->numpts; ix++) {
	x = basex + this->pts[ix].x;
	y = basey + this->pts[ix].y;
	(v)->DrawLineTo((v)->ToPixX(x), (v)->ToPixY(y));
    }
    if (this->closed)
	(v)->DrawLineTo((v)->ToPixX(basex), (v)->ToPixY(basey));
}

/* sketch all lines touching ptref */
static void PartialSketch(class figoplin  *self, class figview  *v, long  ptref)
{
    long x, y, basex, basey;

    basex = self->pts[0].x;
    basey = self->pts[0].y;

    (v)->SetTransferMode(graphic::INVERT);

    if (ptref==0 || ptref==1) {
	(v)->MoveTo((v)->ToPixX(basex), (v)->ToPixY(basey));
	x = basex + self->pts[1].x;
	y = basey + self->pts[1].y;
	(v)->DrawLineTo((v)->ToPixX(x), (v)->ToPixY(y));
    }

    if ((ptref==0 || ptref==self->numpts-1) && self->closed) {
	(v)->MoveTo((v)->ToPixX(basex), (v)->ToPixY(basey));
	x = basex + self->pts[self->numpts-1].x;
	y = basey + self->pts[self->numpts-1].y;
	(v)->DrawLineTo((v)->ToPixX(x), (v)->ToPixY(y));
    }

    if (ptref>0 && ptref<self->numpts-1) {
	x = basex + self->pts[ptref].x;
	y = basey + self->pts[ptref].y;
	(v)->MoveTo((v)->ToPixX(x), (v)->ToPixY(y));
	x = basex + self->pts[ptref+1].x;
	y = basey + self->pts[ptref+1].y;
	(v)->DrawLineTo((v)->ToPixX(x), (v)->ToPixY(y));
    }

    if (ptref>1 && ptref<self->numpts) {
	x = basex + self->pts[ptref].x;
	y = basey + self->pts[ptref].y;
	(v)->MoveTo((v)->ToPixX(x), (v)->ToPixY(y));
	x = basex + self->pts[ptref-1].x;
	y = basey + self->pts[ptref-1].y;
	(v)->DrawLineTo((v)->ToPixX(x), (v)->ToPixY(y));
    }
}

void figoplin::Select(class figview  *v)
{
    long ix;
    long x, y;
    class graphic *BlackPattern;

    if ((this)->GetHandles() && (this)->GetNumHandles()) {
	(v)->SetTransferMode(graphic::INVERT);
	BlackPattern = (v)->BlackPattern(); /* ### should cache this! */

	for (ix=4; ix<(this)->GetNumHandles(); ix++) {
	    x = (v)->ToPixX(point_X(&((this)->GetHandles()[ix])));
	    y = (v)->ToPixY(point_Y(&((this)->GetHandles()[ix])));
	    (v)->FillRectSize(x-figview_SpotRad, y-figview_SpotRad, 1+2*figview_SpotRad, 1+2*figview_SpotRad, BlackPattern);
	}

	/* if there are three or more points, draw the box handles as well */
	if ((this)->GetNumHandles()>6)
	    for (ix=0; ix<4; ix++) {
		x = (v)->ToPixX(point_X(&((this)->GetHandles()[ix])));
		y = (v)->ToPixY(point_Y(&((this)->GetHandles()[ix])));
		(v)->DrawRectSize(x-figview_SpotRad, y-figview_SpotRad, 2*figview_SpotRad, 2*figview_SpotRad);
	    }
    }
}

/* create a regular polygon, using self->cen{x,y} as the center and (endx, endy) as the [offset] vector. */
static void RegularizePolygon(class figoplin  *self, long  endx , long  endy)
{
    int ix;
    double radius, divvy, offset;

    SetNumPts(self, self->regular);
    radius = sqrt((double)(endx*endx+endy*endy));
    divvy = TWOPI / (double)(self->regular);
    offset = atan2((double)(endy), (double)(endx));
    self->pts[0].x = self->cenx + (long)(radius * cos(offset));
    self->pts[0].y = self->ceny + (long)(radius * sin(offset));

    for (ix=1; ix<self->regular; ix++) {
	self->pts[ix].x = self->cenx + (long)(radius * cos(offset + divvy * (double)ix)) - self->pts[0].x;
	self->pts[ix].y = self->ceny + (long)(radius * sin(offset + divvy * (double)ix)) - self->pts[0].y;
    }
}

enum figobj_Status figoplin::Build(class figview  *v, enum view::MouseAction  action, long  x , long  y /* in fig coords */, long  clicks)   
{
    long px, py, apx, apy;
    int ix;

    if (clicks==0) {
	if (this->numpts<2 || (this->regular>=3 && this->buildstate!=3)) {
	    message::DisplayString(v, 10, "Only one point; object aborted.");
	    return figobj_Failed;
	}
	else {
	    (this)->SetModified();
	    return figobj_Done;
	}
    }

    switch (action) {
	case view::LeftDown:
	    (v)->BlockUpdates(TRUE);
	    if (this->regular >= 3) {
		if (this->buildstate==0) {
		    this->buildstate = 1;
		    SetNumPts(this, 2);
		    this->cenx = this->pts[0].x = x;
		    this->ceny = this->pts[0].y = y;
		    RegularizePolygon(this, 4, 4);
		    (this)->RecomputeBounds();
		    (this)->Sketch(v);
		}
		else {
		    (this)->Sketch(v);
		    RegularizePolygon(this, x-this->cenx, y-this->ceny);
		    (this)->RecomputeBounds();
		    (this)->Sketch(v);
		}
	    }
	    else {
		if (this->buildstate==0) {
		    this->buildstate = 1;
		    SetNumPts(this, 1);
		    this->pts[0].x = x;
		    this->pts[0].y = y;
		    this->rockx = (v)->ToPixX(x);
		    this->rocky = (v)->ToPixY(y);
		    this->lastx = this->rockx;
		    this->lasty = this->rocky;
		    (this)->RecomputeBounds();
		    (v)->SetTransferMode(graphic::INVERT);
		    (v)->MoveTo(this->rockx, this->rocky);
		    (v)->DrawLineTo(this->lastx, this->lasty);
		}
		else {
		    ix = this->numpts-1;
		    if (ix==0) {
			this->rockx = (v)->ToPixX(this->pts[0].x);
			this->rocky = (v)->ToPixY(this->pts[0].y);
		    }
		    else {
			this->rockx = (v)->ToPixX(this->pts[0].x+this->pts[ix].x);
			this->rocky = (v)->ToPixY(this->pts[0].y+this->pts[ix].y);
		    }
		    this->lastx = (v)->ToPixX(x);
		    this->lasty = (v)->ToPixY(y);
		    (v)->SetTransferMode(graphic::INVERT);
		    (v)->MoveTo(this->rockx, this->rocky);
		    (v)->DrawLineTo(this->lastx, this->lasty);
		}
	    }
	    return figobj_NotDone;
	case view::LeftMovement:
	    if (this->regular >= 3) {
		(this)->Sketch(v);
		px = x - this->cenx;
		py = y - this->ceny;
		RegularizePolygon(this, px, py);
		(this)->RecomputeBounds();
		(this)->Sketch(v);
	    }
	    else {
		(v)->SetTransferMode(graphic::INVERT);
		(v)->MoveTo(this->rockx, this->rocky);
		(v)->DrawLineTo(this->lastx, this->lasty);
		this->lastx = (v)->ToPixX(x);
		this->lasty = (v)->ToPixY(y);
		(v)->MoveTo(this->rockx, this->rocky);
		(v)->DrawLineTo(this->lastx, this->lasty);
	    }
	    return figobj_NotDone;
	case view::LeftUp:
	    (v)->BlockUpdates(FALSE);
	    if (this->regular >= 3) {
		px = x - this->cenx;
		py = y - this->ceny;
		apx = (v)->ToPixW(px);
		apy = (v)->ToPixH(py);
		if (this->buildstate == 1
		    && (apx < figview_MouseHysteresis)
		    && (apy < figview_MouseHysteresis) 
		    && (apx > -figview_MouseHysteresis)
		    && (apy > -figview_MouseHysteresis)) {
		    /* cursor didn't move far enough; we have a point click. */
		    message::DisplayString(v, 10, "Center point placed; use left button for radius end.");
		    this->buildstate=2;
		    return figobj_NotDone;
		}
		else {
		    this->buildstate=3;
		    (this)->Sketch(v);
		    RegularizePolygon(this, x-this->cenx, y-this->ceny);
		    (this)->RecomputeBounds();
		    (this)->SetModified();
		    return figobj_Done;
		}
	    }
	    else {
		px = x - this->pts[0].x;
		py = y - this->pts[0].y;
		apx = (v)->ToPixW(px);
		apy = (v)->ToPixH(py);
		if (this->buildstate==1 
		    && (apx < figview_MouseHysteresis)
		    && (apy < figview_MouseHysteresis) 
		    && (apx > -figview_MouseHysteresis)
		    && (apy > -figview_MouseHysteresis)) {
		    /* cursor didn't move far enough; we have a point click. */
		    (v)->SetTransferMode(graphic::INVERT);
		    (v)->MoveTo(this->rockx, this->rocky);
		    (v)->DrawLineTo(this->lastx, this->lasty);
		    (v)->MoveTo(this->rockx-1, this->rocky-1);
		    (v)->DrawLineTo(this->rockx+1, this->rocky+1);
		    (v)->MoveTo(this->rockx+1, this->rocky-1);
		    (v)->DrawLineTo(this->rockx-1, this->rocky+1);
		    if (!this->shortmode)
			message::DisplayString(v, 10, "First point placed; use left button for more points, right button when done.");
		    else
			message::DisplayString(v, 10, "First point placed; use left button for other end.");
		    this->buildstate=2;
		}
		else {
		    if (this->buildstate==1 && !this->shortmode) {
			message::DisplayString(v, 10, "2 points placed; use left button for more points, right button when done.");
			this->buildstate=2;
		    }
		    ix = this->numpts;
		    SetNumPts(this, ix+1);
		    this->pts[ix].x = px;
		    this->pts[ix].y = py;
		    (this)->RecomputeBounds();
		}
		if (this->shortmode && this->numpts==2) {
		    (this)->SetModified();
		    return figobj_Done;
		} else
		    return figobj_NotDone;
	    }
	case view::RightDown:
	case view::RightMovement:
	case view::RightUp:
	    if (this->numpts<2 || (this->regular>=3 && this->buildstate!=3)) {
		message::DisplayString(v, 10, "Only one point; object aborted.");
		return figobj_Failed;
	    }
	    (this)->SetModified();
	    return figobj_Done;
	default:
	    return figobj_Failed;
    }
}

/* return the ptref of the first point on the line segment within delta of (x, y). This will be in [0, numpts-1] if the polygon is closed, in [0, numpts-2] if it's open. If no segment is found, return figobj_NULLREF. */
static int FindLineHit(class figoplin  *self, long  x , long  y, long  delta)
{

#define IABS(v) (((v) < 0) ? (-(v)) : (v))
    int ix;
    long val, basex, basey, x0, y0, x1, y1;

    basex = self->pts[0].x;
    basey = self->pts[0].y;
    x0 = basex;
    y0 = basey;
    for (ix=1; ix<=self->numpts; ix++) {
	if (ix==self->numpts) {
	    if (!self->closed)
		continue;
	    x1 = basex;
	    y1 = basey;
	}
	else {
	    x1 = self->pts[ix].x + basex;
	    y1 = self->pts[ix].y + basey;
	}

	if (x1-x0 == 0 && y1-y0 == 0) {
	    /* forget it */
	}
	else if (IABS(x1-x0) > IABS(y1-y0)) {
	    val = y0 + ((x-x0) * (y1-y0)) / (x1-x0);
	    if (!((x<x0 && x<x1) || (x>x0 && x>x1)) && IABS(y-val) <= delta) {
		return ix-1;	
	    }
	}
	else {
	    val = x0 + ((y-y0) * (x1-x0)) / (y1-y0);
	    if (!((y<y0 && y<y1) || (y>y0 && y>y1)) && IABS(x-val) <= delta) {
		return ix-1;	
	    }
	}

	x0 = x1;
	y0 = y1;
    }

    return figobj_NULLREF;
}

enum figobj_HitVal figoplin::HitMe(long  x , long  y, long  delta, long  *ptref) 
{
    int ix;
    enum figobj_HitVal res = (this)->BasicHitMe(x, y, delta, ptref);

    if (res!=figobj_HitInside)
	return res;

    /* click hit bbox but no handles */
    ix = FindLineHit(this, x, y, delta);
    if (ix!=figobj_NULLREF) {
	if (ptref)
	    *ptref = ix;
	return figobj_HitBody;
    }

    return figobj_Miss;
}

/* basic procedure to move a handle -- used by figoplin__MoveHandle(), figoplin__Reshape() */
static void MoveHandle(class figoplin  *self, long  x , long  y , long  ptref)
{
    long ix;
    long noffx, noffy, offx, offy;

    if (ptref<0 || ptref>=self->numpts+4)
	return;

    if (ptref<4) {
	long oldwidth, oldheight, newwidth, newheight;
	long nbasex, nbasey;

	if (!self->orpts) {
	    self->orpts = (struct point *)malloc(sizeof(struct point) * self->numpts);
	    for (ix=0; ix<self->numpts; ix++)
		self->orpts[ix] = self->pts[ix];
	    for (ix=0; ix<4; ix++)
		self->orhandles[ix] = (self)->GetHandles()[ix];
	}

	offx = self->orhandles[3-ptref].x;
	offy = self->orhandles[3-ptref].y;
	noffx = (self)->GetHandles()[3-ptref].x;
	noffy = (self)->GetHandles()[3-ptref].y;
	oldwidth = self->orhandles[ptref].x - offx;
	oldheight = self->orhandles[ptref].y - offy;
	newwidth = x - noffx;
	newheight = y - noffy;
	switch (ptref) {
	    case 0:
		self->fliph = !(newwidth<0);
		self->flipv = !(newheight<0);
		break;
	    case 1:
		self->fliph = (newwidth<0);
		self->flipv = !(newheight<0);
		break;
	    case 2:
		self->fliph = !(newwidth<0);
		self->flipv = (newheight<0);
		break;
	    case 3:
		self->fliph = (newwidth<0);
		self->flipv = (newheight<0);
		break;
	}
	if (oldwidth==0) {
	    oldwidth=1; 
	    newwidth=0;
	}
	if (oldheight==0) {
	    oldheight=1; 
	    newheight=0;
	}

	nbasex = ((self->orpts[0].x-offx) * newwidth) / oldwidth + noffx;
	nbasey = ((self->orpts[0].y-offy) * newheight) / oldheight + noffy;
	self->pts[0].x = nbasex;
	self->pts[0].y = nbasey;
	for (ix=1; ix<self->numpts; ix++) {
	    self->pts[ix].x = ((self->orpts[0].x+self->orpts[ix].x-offx) * newwidth) / oldwidth + noffx - nbasex;
	    self->pts[ix].y = ((self->orpts[0].y+self->orpts[ix].y-offy) * newheight) / oldheight + noffy - nbasey;
	}
    }
    else {
	ClearOldPoints(self);
	ptref -= 4;
	if (ptref==0) {
	    offx = x - self->pts[0].x;
	    offy = y - self->pts[0].y;
	    self->pts[0].x += offx;
	    self->pts[0].y += offy;
	    for (ix=1; ix<self->numpts; ix++) {
		self->pts[ix].x -= offx;
		self->pts[ix].y -= offy;
	    }
	}
	else {
	    self->pts[ptref].x = x - self->pts[0].x;
	    self->pts[ptref].y = y - self->pts[0].y;
	}
    }
}

boolean figoplin::Reshape(enum view::MouseAction  action, class figview  *v, long  x , long  y , boolean  handle , long  ptref)
{
    if (!handle)
	return FALSE;

    if (ptref>=0 && ptref<4) {
	switch (action) {
	    case view::LeftDown:
	    case view::RightDown:
		if ((this)->GetReadOnly())
		    return FALSE;
		if (this->stickysketch)
		    (this)->Sketch(v);
		break;
	    case view::LeftMovement:
	    case view::RightMovement:
		(this)->Sketch(v);
		::MoveHandle(this, x, y, ptref);
		(this)->RecomputeBounds();
		(this)->Sketch(v);
		break;
	    case view::LeftUp:
	    case view::RightUp:
		(this)->Sketch(v);
		::MoveHandle(this, x, y, ptref);
		(this)->RecomputeBounds();
		(this)->SetModified();
		break;
	    default:
		break;
	}
	return TRUE;
    }
    else {
	switch (action) {
	    case view::LeftDown:
	    case view::RightDown:
		if ((this)->GetReadOnly())
		    return FALSE;
		if (this->stickysketch)
		    PartialSketch(this, v, ptref-4);
		break;
	    case view::LeftMovement:
	    case view::RightMovement:
		PartialSketch(this, v, ptref-4);
		::MoveHandle(this, x, y, ptref);
		PartialSketch(this, v, ptref-4);
		break;
	    case view::LeftUp:
	    case view::RightUp:
		PartialSketch(this, v, ptref-4);
		::MoveHandle(this, x, y, ptref);
		(this)->RecomputeBounds();
		(this)->SetModified();
		break;
	    default:
		break;
	}
	return TRUE;
    }
}

void figoplin::MoveHandle(long  x , long  y , long  ptref)
{
    if ((this)->GetReadOnly())
	return;
    ::MoveHandle(this, x, y, ptref);
    (this)->SetModified();
    (this)->RecomputeBounds();
}

boolean figoplin::AddParts(enum view::MouseAction  action, class figview  *v, long  x , long  y , boolean  handle , long  ptref)
{
    int ix;
    long offx, offy;

    if (ptref>=0 && handle) {
	if (ptref<4)
	    return FALSE;
	else
	    ptref -= 4;
    }

    switch (action) {
	case view::LeftDown:
	case view::RightDown:
	    if ((this)->GetReadOnly())
		return FALSE;
	    if (ptref==0) {
		x = this->pts[0].x;
		y = this->pts[0].y;
	    }
	    else {
		x = this->pts[ptref].x + this->pts[0].x;
		y = this->pts[ptref].y + this->pts[0].y;
	    }
	    if (ptref == 0 && handle == TRUE) {
		SetNumPts(this, this->numpts+1);
		offx = this->pts[0].x - x;
		offy = this->pts[0].y - y;
		for (ix=this->numpts-2; ix>=1; ix--) {
		    this->pts[ix+1].x = this->pts[ix].x + offx;
		    this->pts[ix+1].y = this->pts[ix].y + offy;
		}
		this->pts[1].x = offx;
		this->pts[1].y = offy;
		this->pts[0].x -= offx;
		this->pts[0].y -= offy;
		(this)->RecomputeBounds();
		this->rock = 4;
		return (this)->Reshape(action, v, x, y, TRUE, this->rock);
	    }
	    else {
		SetNumPts(this, this->numpts+1);
		for (ix=this->numpts-2; ix>ptref; ix--) {
		    this->pts[ix+1].x = this->pts[ix].x;
		    this->pts[ix+1].y = this->pts[ix].y;
		}
		this->pts[ptref+1].x = x - this->pts[0].x;
		this->pts[ptref+1].y = y - this->pts[0].y;
		(this)->RecomputeBounds();
		this->rock = 4+ptref+1;
		return (this)->Reshape(action, v, x, y, TRUE, this->rock);
	    }
	case view::LeftMovement:
	case view::RightMovement:
	case view::LeftUp:
	case view::RightUp:
	    return (this)->Reshape(action, v, x, y, TRUE, this->rock);
	default:
	    break;
    }
    return FALSE;
}

boolean figoplin::DeleteParts(enum view::MouseAction  action, class figview  *v, long  x , long  y , boolean  handle , long  ptref)
{
    int ix;
    long offx, offy;

    if (!handle || (this)->GetReadOnly())
	return FALSE;

    if (ptref>=0) {
	if (ptref<4)
	    return FALSE;
	else
	    ptref -= 4;
    }

    if (ptref == figobj_NULLREF)
	return FALSE;
    if (this->numpts <= 2) {
	message::DisplayString(v, 10, "There are only two points left.");
	return FALSE;
    };

    switch (action) {
	case view::LeftDown:
	case view::RightDown:
	case view::LeftMovement:
	case view::RightMovement:
	    break;
	case view::LeftUp:
	case view::RightUp:
	    if (ptref==0) {
		offx = this->pts[1].x;
		offy = this->pts[1].y;
		this->pts[0].x += offx;
		this->pts[0].y += offy;
		for (ix=2; ix<this->numpts; ix++) {
		    this->pts[ix-1].x = this->pts[ix].x - offx;
		    this->pts[ix-1].y = this->pts[ix].y - offy;
		}
		SetNumPts(this, this->numpts-1);	
	    }
	    else {
		for (ix=ptref+1; ix<this->numpts; ix++) {
		    this->pts[ix-1].x = this->pts[ix].x;
		    this->pts[ix-1].y = this->pts[ix].y;
		}
		SetNumPts(this, this->numpts-1);
	    }
	    (this)->RecomputeBounds();
	    (this)->SetModified();
	    break;
	default:
	    break;
    }
    return TRUE;
}

void figoplin::Reposition(long  xd , long  yd)
{
    if ((this)->GetReadOnly())
	return;
    this->pts[0].x += xd;
    this->pts[0].y += yd;
    (this)->RecomputeBounds();
    (this)->SetModified();
}

void figoplin::InheritVAttributes(class figattr  *attr, unsigned long  mask)
{
    (this)->figobj::InheritVAttributes(attr, mask);

    if (mask & (~((this)->GetVAttributes()->active))
	& ((1<<figattr_LineWidth) | (1<<figattr_ArrowSize) | (1<<figattr_ArrowPos))) {
	(this)->RecomputeBounds();
    }
}

unsigned long figoplin::UpdateVAttributes(class figattr  *attr, unsigned long  mask)
{
    mask = (this)->figobj::UpdateVAttributes(attr, mask);
    
    if (mask
	& ((1<<figattr_LineWidth) | (1<<figattr_ArrowSize) | (1<<figattr_ArrowPos))) {
	(this)->RecomputeBounds();
    }

    return mask;
}

void figoplin::WriteBody(FILE  *fp)
{
   /* we don't call super_WriteBody() because that just outputs PosX and PosY, which aren't being used */
    int ix;

    fprintf(fp, "$$ %d %d %d %d\n", this->closed, this->numpts, this->fliph, this->flipv);
    for (ix=0; ix<this->numpts; ix++)
	fprintf(fp, "$ %ld %ld\n", this->pts[ix].x, this->pts[ix].y);
}

long figoplin::ReadBody(FILE  *fp, boolean  recompute)
{
    int	ix, jx; 
    long num, num2, xp, yp;
    long numh, numv;

#define LINELENGTH (250)
    char buf[LINELENGTH+1];

    if (fgets(buf, LINELENGTH, fp) == NULL)
	return dataobject::PREMATUREEOF;
    ix = sscanf(buf, "$$ %ld %ld %ld %ld", &num2, &num, &numh, &numv);
    if (ix!=4) return dataobject::BADFORMAT;

    this->SetClosed(num2);

    this->fliph = numh;
    this->flipv = numv;
    this->numpts = num;
    SetNumPts(this, num);

    for (jx=0; jx<this->numpts; jx++) {
	if (fgets(buf, LINELENGTH, fp) == NULL)
	    return dataobject::PREMATUREEOF;
	ix = sscanf(buf, "$ %ld %ld", &xp, &yp);
	if (ix!=2) return dataobject::BADFORMAT;
	this->pts[jx].x = xp;
	this->pts[jx].y = yp;
    }

    if (recompute) {
	(this)->RecomputeBounds();
	(this)->SetModified();
    }

    return dataobject::NOREADERROR;
}

#define FadeColor(col, shad)  (1.0 - (1.0-(shad)) * (1.0-(col)))

void figoplin::PrintObject(class figview  *v, FILE  *file, const char  *prefix, boolean newstyle)
{
    long ix, x, y, xbase, ybase, nump;
    struct point *pts;
    long lw, shad, dash, asize, apos, ashape;
    const char *col;
    double rcol, bcol, gcol, shadcol;
    struct point arrowparh, arrowpart;
    struct figattr_arrowhead arrowrockh, arrowrockt;

    fprintf(file, "%s  %% polyline\n", prefix);

    nump = (this)->NumPts();
    pts = (this)->Pts();

    if (nump < 2)
	return;

    xbase = pts[0].x;
    ybase = pts[0].y;
    
    apos = ((this)->GetVAttributes())-> GetArrowPos((this)->GetIVAttributes());
    if (this->Closed())
	apos = 0;
    if (apos) {
	int workpt;
	long parx, pary;

	asize = ((this)->GetVAttributes())-> GetArrowSize((this)->GetIVAttributes());
	asize = asize*figview_FigUPerPix;
	ashape = ((this)->GetVAttributes())-> GetArrow((this)->GetIVAttributes());
	
	if (apos & figattr_ArrowTail) {
	    if (numpts>2) {
		workpt = this->numpts-1;
		x = pts[workpt].x + xbase;
		y = pts[workpt].y + ybase;
		parx = (pts[workpt].x - pts[workpt-1].x);
		pary = (pts[workpt].y - pts[workpt-1].y);
	    }
	    else {
		workpt = 1;
		x = pts[workpt].x + xbase;
		y = pts[workpt].y + ybase;
		parx = (pts[workpt].x);
		pary = (pts[workpt].y);
	    }
	    figattr::SetupPrintArrowHead(&arrowrockt, v, x, y, &parx, &pary, ashape, asize);
	    arrowpart.x = (v)->ToPrintPixW(parx);
	    arrowpart.y = (v)->ToPrintPixW(pary);
	}
	if (apos & figattr_ArrowHead) {
	    x = xbase;
	    y = ybase;
	    parx = (-(pts[1].x));
	    pary = (-(pts[1].y));
	    figattr::SetupPrintArrowHead(&arrowrockh, v, x, y, &parx, &pary, ashape, asize);
	    arrowparh.x = (v)->ToPrintPixW(parx);
	    arrowparh.y = (v)->ToPrintPixW(pary);
	}
    }

    fprintf(file, "%s  2 setlinejoin\n", prefix);
    x = (v)->ToPrintPixX(xbase);
    y = (v)->ToPrintPixY(ybase);
    if (apos & figattr_ArrowHead) {
	x -= arrowparh.x;
	y -= arrowparh.y;
    }
    fprintf(file, "%s  %ld %ld moveto\n", prefix, x, y);
    for (ix=1; ix<nump; ix++) {
	x = (v)->ToPrintPixX(pts[ix].x + xbase);
	y = (v)->ToPrintPixY(pts[ix].y + ybase);
	if (ix==nump-1 && (apos & figattr_ArrowTail)) {
	    x -= arrowpart.x;
	    y -= arrowpart.y;
	}
	fprintf(file, "%s  %ld %ld lineto\n", prefix, x, y);
    }

    col = ((this)->GetVAttributes())->GetColor((this)->GetIVAttributes());
    print::LookUpColor(col, &rcol, &gcol, &bcol);

    if ((this)->Closed()) {
	fprintf(file, "%s  closepath\n", prefix);

	shad = ((this)->GetVAttributes())->GetShade((this)->GetIVAttributes());
	if (shad != figattr_ShadeClear) {
	    fprintf(file, "%s  gsave\n", prefix);
	    shadcol = (double)(figattr_ShadeDenom-shad) / (double)figattr_ShadeDenom;
	    fprintf(file, "%s  %f %f %f setrgbcolor\n", prefix, FadeColor(rcol, shadcol), FadeColor(gcol, shadcol), FadeColor(bcol, shadcol));
	    /*fprintf(file, "%s  %f setgray\n", prefix, shadcol);*/
	    fprintf(file, "%s  eofill grestore\n", prefix);
	}
    }

    lw = ((this)->GetVAttributes())->GetLineWidth((this)->GetIVAttributes());
    lw = (v)->ToPrintPixW(lw*figview_FigUPerPix);
    if (lw <= 0) lw = 0;
    fprintf(file, "%s  %ld setlinewidth\n", prefix, lw);
    fprintf(file, "%s  %f %f %f setrgbcolor\n", prefix, rcol, gcol, bcol);
    /*fprintf(file, "%s  0 setgray\n", prefix);*/
    /* print dashes */
    dash = ((this)->GetVAttributes())->GetLineStyle( (this)->GetIVAttributes());
    if (dash != figattr_LineSolid)  {
	char *patterns = figattr::LineStylePattern(dash, lw);
	int ic, dval;
	fprintf(file, "%s [ ", prefix);
	for (ic=0; (dval=patterns[ic]); ic++)  
	    fprintf(file, "%d ", dval);
	fprintf(file, "] 0 setdash\n");
    }
    fprintf(file, "%s  stroke\n", prefix);
    /* turn off dashes? */

    if (apos & figattr_ArrowTail) {
	figattr::PrintArrowHead(file, prefix, &arrowrockt);
    }
    if (apos & figattr_ArrowHead) {
	figattr::PrintArrowHead(file, prefix, &arrowrockh);
    }
}
