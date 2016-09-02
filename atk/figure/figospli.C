/* figospli.c - fig element object: spline */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
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
ATK_IMPL("figospli.H")
#include <math.h>
#include <figospli.H>

#include <figoplin.H>
#include <figattr.H>
#include <view.H>
#include <figview.H>
#include <figtoolview.H>
#include <message.H>
#include <print.H>

#define figospli_Segments (8)

struct figospli_temp {
    double t1, t2, t3, t4;
    double ss;
};

static struct figospli_temp *ctemp;
static int ctemp_size;
static struct point *ptemp;
static int ptemp_size;


ATKdefineRegistry(figospli, figoplin, figospli::InitializeClass);

static void spline(struct point  *pts, int  n, struct figospli_cubit  *cubit, boolean  closed);
static void SetNumCubits(class figospli  *self, long  inum);
static int FindLineHit(class figospli  *self, long  x , long  y, long  delta);


boolean figospli::InitializeClass()
{
    ctemp_size = 0;
    ctemp = NULL;
    ptemp_size = 0;
    ptemp = NULL;
    return TRUE;
}

figospli::figospli()
{
    this->cubit = NULL;
    this->cubit_size = 0;
    ((class figoplin *)this)->stickysketch = TRUE;
    this->tmppts = NULL;
    this->tmppts_size = 0;
    THROWONFAILURE(TRUE);
}

/* efficient? well, no. */
class figospli *figospli::Create(struct point  *pointlist, long  numpoints, boolean  isclosed)
{
    class figoplin *tmp = figoplin::Create(pointlist, numpoints, isclosed);
    class figospli *res;

    if (!tmp) return NULL;
    res = new figospli;
    if (!res) return NULL;

    (res)->CopyData(tmp);
    (tmp)->Destroy();

    return res;
}

figospli::~figospli()
{
    if (this->cubit)
	free(this->cubit);
    if (this->tmppts)
	free(this->tmppts);
}

const char *figospli::ToolName(class figtoolview  *v, long  rock)
{
    if (rock & 2)
	return "Closed Spline";
    else
	return "Spline";
}

/* if closed is TRUE, pts[n] doesn't exist; use pts[0] instead. */
static void spline(struct point  *pts, int  n, struct figospli_cubit  *cubit, boolean  closed)
{
    /* uses pts[0...n], ctemp[0...n], cubit[0...n-1] */
    int i;
    double dy1, dy2;
    int first, last;

    /* do y coords */
    dy1 = (pts[1].y) * 6.0;
    for (i=0; i<n-1; i++) {
	if (closed && i+2==n)
	    dy2 = (-pts[i+1].y) * 6.0;
	else
	    dy2 = (pts[i+2].y - pts[i+1].y) * 6.0;
	ctemp[i+1].t1 = 1.0;
	ctemp[i+1].t2 = 4.0;
	ctemp[i+1].t3 = 1.0;
	ctemp[i+1].t4 = dy2 - dy1;
	dy1 = dy2;
    }
    first = 1;
    last = n-2;


    for (i=first; i<=last; i++) {
	ctemp[i+1].t1 /= ctemp[i].t2;
	ctemp[i+1].t2 -= (ctemp[i+1].t1 * ctemp[i].t3);
	ctemp[i+1].t4 -= (ctemp[i+1].t1 * ctemp[i].t4);
    }

    if (last >= 0) {
	ctemp[last+1].t4 /= ctemp[last+1].t2;
    }
    for (i=last-1; i>=first-1; i--) {
	ctemp[i+1].t4 = (ctemp[i+1].t4 - ctemp[i+1].t3*ctemp[i+2].t4) / ctemp[i+1].t2;
    }

    for (i=first-1; i<=last; i++) {
	ctemp[i+1].ss = ctemp[i+1].t4;
    };

    if (!closed || n==1) {
	ctemp[0].ss = 0.0;
	ctemp[n].ss = 0.0;
    }
    else {
	double lhs = pts[1].y - (-pts[n-1].y) - (ctemp[1].ss + ctemp[n-1].ss)/6.0;
	ctemp[0].ss = 1.5 * lhs;
	ctemp[n].ss = 1.5 * lhs;
    }

    for (i=0; i<n; i++) {  
	cubit[i].ya = (ctemp[i+1].ss - ctemp[i].ss) / (6.0);
	cubit[i].yb = ctemp[i].ss / 2.0; 
	if (i==0) {
	    if (closed && i+1==n)
		cubit[i].yc = 0 - (2.0*ctemp[i].ss + ctemp[i+1].ss) / 6.0;
	    else
		cubit[i].yc = (pts[1].y) - (2.0*ctemp[i].ss + ctemp[i+1].ss) / 6.0;
	    cubit[i].yd = pts[0].y;
	}
	else {
	    if (closed && i+1==n)
		cubit[i].yc = (-pts[i].y) - (2.0*ctemp[i].ss + ctemp[i+1].ss) / 6.0;
	    else
		cubit[i].yc = (pts[i+1].y - pts[i].y) - (2.0*ctemp[i].ss + ctemp[i+1].ss) / 6.0;
	    cubit[i].yd = pts[0].y + pts[i].y;
	}
    }

    /* do x coords */
    dy1 = (pts[1].x) * 6.0;
    for (i=0; i<n-1; i++) {
	if (closed && i+2==n)
	    dy2 = (-pts[i+1].x) * 6.0;
	else
	    dy2 = (pts[i+2].x - pts[i+1].x) * 6.0;
	ctemp[i+1].t1 = 1.0;
	ctemp[i+1].t2 = 4.0;
	ctemp[i+1].t3 = 1.0;
	ctemp[i+1].t4 = dy2 - dy1;
	dy1 = dy2;
    }
    first = 1;
    last = n-2;

    for (i=first; i<=last; i++) {
	ctemp[i+1].t1 /= ctemp[i].t2;
	ctemp[i+1].t2 -= (ctemp[i+1].t1 * ctemp[i].t3);
	ctemp[i+1].t4 -= (ctemp[i+1].t1 * ctemp[i].t4);
    }

    if (last >= 0) {
	ctemp[last+1].t4 /= ctemp[last+1].t2;
    }
    for (i=last-1; i>=first-1; i--) {
	ctemp[i+1].t4 = (ctemp[i+1].t4 - ctemp[i+1].t3*ctemp[i+2].t4) / ctemp[i+1].t2;
    }

    for (i=first-1; i<=last; i++) {
	ctemp[i+1].ss = ctemp[i+1].t4;
    };

    if (!closed || n==1) {
	ctemp[0].ss = 0.0;
	ctemp[n].ss = 0.0;
    }
    else {
	double lhs = pts[1].x - (-pts[n-1].x) - (ctemp[1].ss + ctemp[n-1].ss)/6.0;
	ctemp[0].ss = 1.5 * lhs;
	ctemp[n].ss = 1.5 * lhs;
    }

    for (i=0; i<n; i++) {  
	cubit[i].xa = (ctemp[i+1].ss - ctemp[i].ss) / (6.0);
	cubit[i].xb = ctemp[i].ss / 2.0; 
	if (i==0) {
	    if (closed && i+1==n)
		cubit[i].xc = 0 - (2.0*ctemp[i].ss + ctemp[i+1].ss) / 6.0;
	    else
		cubit[i].xc = (pts[1].x) - (2.0*ctemp[i].ss + ctemp[i+1].ss) / 6.0;
	    cubit[i].xd = pts[0].x;
	}
	else {
	    if (closed && i+1==n)
		cubit[i].xc = (-pts[i].x) - (2.0*ctemp[i].ss + ctemp[i+1].ss) / 6.0;
	    else   
		cubit[i].xc = (pts[i+1].x - pts[i].x) - (2.0*ctemp[i].ss + ctemp[i+1].ss) / 6.0;
	    cubit[i].xd = pts[0].x + pts[i].x;
	}
    }
}

static void SetNumCubits(class figospli  *self, long  inum)
{
    int num;

    num = inum;
    if (num > self->cubit_size) {
	if (self->cubit_size == 0) {
	    self->cubit_size = num;
	    self->cubit = (struct figospli_cubit *)malloc(self->cubit_size * sizeof(struct figospli_cubit));
	}
	else {
	    while (num > self->cubit_size)
		self->cubit_size *= 2;
	    self->cubit = (struct figospli_cubit *)realloc(self->cubit, self->cubit_size * sizeof(struct figospli_cubit));
	}
    }

    num = inum+2;
    if (num > ctemp_size) {
	if (ctemp_size == 0) {
	    ctemp_size = num;
	    ctemp = (struct figospli_temp *)malloc(ctemp_size * sizeof(struct figospli_temp));
	}
	else {
	    while (num > ctemp_size)
		ctemp_size *= 2;
	    ctemp = (struct figospli_temp *)realloc(ctemp, ctemp_size * sizeof(struct figospli_temp));
	}
    }

    num = 1 + inum * figospli_Segments;
    if (num > self->tmppts_size) {
	if (self->tmppts_size == 0) {
	    self->tmppts_size = num;
	    self->tmppts = (struct point *)malloc(self->tmppts_size * sizeof(struct point));
	}
	else {
	    while (num > self->tmppts_size)
		self->tmppts_size *= 2;
	    self->tmppts = (struct point *)realloc(self->tmppts, self->tmppts_size * sizeof(struct point));
	}
    }
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

void figospli::RecomputeBounds()
{
    int ix, jx, realnum, num, lwid, awid;
    long px, py;
    long xmin, xmax, ymin, ymax, wid, hgt;
    double t;
    boolean closed = (this)->Closed();

    (this)->figoplin::RecomputeBounds();

    realnum = (this)->NumPts();

    if (realnum==0 || !(this)->Pts()) {
	fprintf(stderr, "figospli: impossible RecomputeBounds()\n");
	return;
    }

    if (closed) 
	num = realnum+1;
    else
	num = realnum;

    SetNumCubits(this, num); 

    if (num>1) {
	spline((this)->Pts(), num-1, this->cubit, closed);

	if (closed) {
	    xmin = xmax = (this)->Pts()[0].x;
	    ymin = ymax = (this)->Pts()[0].y;
	}
	else {
	    xmin = xmax = (this)->Pts()[num-1].x + (this)->Pts()[0].x;
	    ymin = ymax = (this)->Pts()[num-1].y + (this)->Pts()[0].y;
	}
    }
    else {
	xmin = xmax = (this)->Pts()[0].x;
	ymin = ymax = (this)->Pts()[0].y;
    }

    this->tmppts[(num-1)*figospli_Segments].x = xmin;
    this->tmppts[(num-1)*figospli_Segments].y = ymin;

    for (ix=0; ix<num-1; ix++) {
	struct figospli_cubit *cb = &(this->cubit[ix]);
	for (t=0.0, jx=0; 
	     jx<figospli_Segments; 
	     jx++, t += (1.0 / (double)figospli_Segments)) {
	    px = (long)(cb->xa*t*t*t + cb->xb*t*t + cb->xc*t + cb->xd);
	    py = (long)(cb->ya*t*t*t + cb->yb*t*t + cb->yc*t + cb->yd);
	    this->tmppts[ix*figospli_Segments + jx].x = px;
	    this->tmppts[ix*figospli_Segments + jx].y = py;
	    if (px<xmin) xmin = px;
	    if (px>xmax) xmax = px;
	    if (py<ymin) ymin = py;
	    if (py>ymax) ymax = py;
	}
    }

    wid = xmax - xmin;
    hgt = ymax - ymin;

    lwid = ((this)->GetVAttributes())->GetLineWidth((this)->GetIVAttributes());
    lwid = lwid*figview_FigUPerPix;
    if (!closed && ((this)->GetVAttributes())->GetArrowPos((this)->GetIVAttributes())) {
	awid = ((this)->GetVAttributes())->GetArrowSize((this)->GetIVAttributes());
	awid = (awid*figview_FigUPerPix)*2;
	if (awid > lwid)
	    lwid = awid;
    }

    (this)->SetBoundsRect(xmin-lwid/2, ymin-lwid/2, wid+lwid+1, hgt+lwid+1);
    (this)->ComputeSelectedBounds(); /* ### inefficient -- this gets called twice, once in super_ and once here. */
}

void figospli::Draw(class figview  *v) 
{
    long ix, shad, dash, lw, apos, asize, ashape;
    const char *col;
    int num = (this)->NumPts();
    if ((this)->Closed()) {
	if (num<2) return;
	num++;
    }

    (v)->SetTransferMode(graphic_COPY);

    col = ((this)->GetVAttributes())->GetColor((this)->GetIVAttributes());
    (v)->SetForegroundColor(col, 0, 0, 0); 

    (v)->SetLineJoin(graphic_JoinBevel);

    lw = ((this)->GetVAttributes())->GetLineWidth((this)->GetIVAttributes());
    lw = (v)->ToPixW(lw*figview_FigUPerPix);
    if (lw <= 0) lw = 1;
    if (lw != 1)
	(v)->SetLineWidth(lw);

    dash = ((this)->GetVAttributes())->GetLineStyle( (this)->GetIVAttributes());
    if (dash != figattr_LineSolid) {
	char *patterns = figattr::LineStylePattern(dash, lw);
	(v)->SetLineDash(patterns, 0, graphic_LineOnOffDash);
    }

    for (ix=0; ix<figospli_Segments*(num-1)+1; ix++) {
	ptemp[ix].x = (v)->ToPixX(this->tmppts[ix].x);	
	ptemp[ix].y = (v)->ToPixY(this->tmppts[ix].y);
    }

    if ((this)->Closed()) {
	shad = ((this)->GetVAttributes())->GetShade((this)->GetIVAttributes());
	if (shad != figattr_ShadeClear) {
	    (v)->FillPolygon(ptemp, figospli_Segments*(num-1)+1, (v)->GrayPattern(shad, figattr_ShadeDenom));
	}
    }
    else { /* not closed */
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
		workpt = figospli_Segments*(num-1);
		parx = (this->tmppts[workpt].x - this->tmppts[workpt-1].x);
		pary = (this->tmppts[workpt].y - this->tmppts[workpt-1].y);

		figattr::DrawArrowHead(v, ptemp[workpt].x, ptemp[workpt].y, &parx, &pary, ashape, asize);

		ptemp[workpt].x = (v)->ToPixX(this->tmppts[workpt].x-parx);
		ptemp[workpt].y = (v)->ToPixY(this->tmppts[workpt].y-pary);
	    }
	    if (apos & figattr_ArrowHead) {
		parx = (this->tmppts[0].x - this->tmppts[1].x);
		pary = (this->tmppts[0].y - this->tmppts[1].y);

		figattr::DrawArrowHead(v, ptemp[0].x, ptemp[0].y, &parx, &pary, ashape, asize);

		ptemp[0].x = (v)->ToPixX(this->tmppts[0].x-parx);
		ptemp[0].y = (v)->ToPixY(this->tmppts[0].y-pary);
	    }
	}
    }

    (v)->DrawPath(ptemp, figospli_Segments*(num-1)+1);

    if (lw != 1)
	(v)->SetLineWidth(1);
    if (dash!=figattr_LineSolid)
	(v)->SetLineDash(NULL, 0, graphic_LineSolid);
    (v)->SetLineJoin(graphic_JoinMiter);
}

static int FindLineHit(class figospli  *self, long  x , long  y, long  delta)
{
    int ix;
    long x0, y0, x1, y1;
    int num = (self)->NumPts();
    if ((self)->Closed())
	num++;

    for (ix=0; ix<figospli_Segments*(num-1); ix++) {
	if (self->tmppts[ix].x < self->tmppts[ix+1].x) {
	    x0 = self->tmppts[ix].x;
	    x1 = self->tmppts[ix+1].x;
	}
	else {
	    x1 = self->tmppts[ix].x;
	    x0 = self->tmppts[ix+1].x;
	}
	if (self->tmppts[ix].y < self->tmppts[ix+1].y) {
	    y0 = self->tmppts[ix].y;
	    y1 = self->tmppts[ix+1].y;
	}
	else {
	    y1 = self->tmppts[ix].y;
	    y0 = self->tmppts[ix+1].y;
	}
	if (x>=x0-delta && x<=x1+delta && y>=y0-delta && y<=y1+delta) {
	    return ix / figospli_Segments;
	}
    }
    return figobj_NULLREF;
}

enum figobj_HitVal figospli::HitMe(long  x , long  y, long  delta, long  *ptref) 
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

#define FadeColor(col, shad)  (1.0 - (1.0-(shad)) * (1.0-(col)))

/* notes: from the PostScript Red Book,
to draw a*t^3 + b*t^2 + c*t + d  (t from 0 to 1), we must do
(dx, dy) moveto
  and then let
P = (dx+cx/3, dy+cy/3) 
Q = (Px + (cx+bx)/3, Py + (cy+by)/3)
R = (dx+cx+bx+ax, dy+cy+by+ay)
  and then
P Q R curveto
*/
void figospli::PrintObject(class figview  *v, FILE  *file, const char  *prefix, boolean newstyle)
{
    long ix, x, y, xbase, ybase, nump;
    struct point *pts;
    long lw, shad, dash, apos, asize, ashape;
    const char *col;
    struct figospli_cubit *cb;
    long Px, Py, Qx, Qy, Rx, Ry;
    double rcol, bcol, gcol, shadcol;
    struct figattr_arrowhead arrowrockh, arrowrockt;

    fprintf(file, "%s  %% polyline\n", prefix);

    nump = (this)->NumPts();
    pts = (this)->Pts();

    if ((this)->Closed())
	nump++;
    if (nump < 2)
	return;

    xbase = pts[0].x;
    ybase = pts[0].y;
    
    fprintf(file, "%s  2 setlinejoin\n", prefix);
    
    fprintf(file, "%s  %ld %ld moveto\n", prefix, (v)->ToPrintPixX(xbase), (v)->ToPrintPixY(ybase));

    for (ix=0; ix<nump-1; ix++) {
	cb = &(this->cubit[ix]);
	Px = (long)(cb->xd + cb->xc/3);
	Py = (long)(cb->yd + cb->yc/3);
	Qx = (long)(Px + (cb->xc + cb->xb)/3);
	Qy = (long)(Py + (cb->yc + cb->yb)/3);
	Rx = (long)((cb->xa + cb->xb + cb->xc + cb->xd));
	Ry = (long)((cb->ya + cb->yb + cb->yc + cb->yd));
	fprintf(file, "%s  %ld %ld %ld %ld %ld %ld curveto\n", prefix, (v)->ToPrintPixX(Px), (v)->ToPrintPixY(Py), (v)->ToPrintPixX(Qx), (v)->ToPrintPixY(Qy), (v)->ToPrintPixX(Rx), (v)->ToPrintPixY(Ry));
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
		/*parx = (pts[workpt].x - pts[workpt-1].x);
		pary = (pts[workpt].y - pts[workpt-1].y);*/
	    }
	    else {
		workpt = 1;
		x = pts[workpt].x + xbase;
		y = pts[workpt].y + ybase;
		/*parx = (pts[workpt].x);
		pary = (pts[workpt].y);*/
	    }
	    cb = &(this->cubit[nump-2]);
	    parx = x - (long)(cb->xd + cb->xc/3 + (cb->xc + cb->xb)/3);
	    pary = y - (long)(cb->yd + cb->yc/3 + (cb->yc + cb->yb)/3);
	    figattr::SetupPrintArrowHead(&arrowrockt, v, x, y, &parx, &pary, ashape, asize);
	}
	if (apos & figattr_ArrowHead) {
	    x = xbase;
	    y = ybase;
	    cb = &(this->cubit[0]);
	    parx = x - (long)(cb->xd + cb->xc/3);
	    pary = y - (long)(cb->yd + cb->yc/3);
	    figattr::SetupPrintArrowHead(&arrowrockh, v, x, y, &parx, &pary, ashape, asize);
	}
    }

    /* turn off dashes? */

    if (apos & figattr_ArrowTail) {
	figattr::PrintArrowHead(file, prefix, &arrowrockt);
    }
    if (apos & figattr_ArrowHead) {
	figattr::PrintArrowHead(file, prefix, &arrowrockh);
    }

}
