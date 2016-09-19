/* figorect.c - fig element object: rectangle */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/
#include <andrewos.h>
ATK_IMPL("figorect.H")
#include <figorect.H>

#include <figattr.H>
#include <view.H>
#include <figview.H>
#include <figure.H>
#include <figtoolview.H>
#include <message.H>
#include <print.H>

#include <rect.h>


ATKdefineRegistry(figorect, figobj, NULL);

static void MoveHandle(class figorect  *self, long  x , long  y , long  ptref);


figorect::figorect()
{
    (this)->PosW() = 0;
    (this)->PosH() = 0;

    (this)->AttributesUsed() = (1<<figattr_Shade) | (1<<figattr_LineWidth) | (1<<figattr_LineStyle) | (1<<figattr_Color);

    (this)->SetNumHandles( 9);
    this->dummysymmetry = FALSE;

    THROWONFAILURE( TRUE);
}

class figorect *figorect::Create(long  left , long  top , long  width , long  height)
{
    class figorect *res = new figorect;
    if (!res) return NULL;

    (res)->PosX() = left;
    (res)->PosY() = top;
    (res)->PosW() = width;
    (res)->PosH() = height;
    (res)->RecomputeBounds();

    return res;
}

const char *figorect::ToolName(class figtoolview  *v, long  rock)
{
    return "Rectangle";
}

void figorect::ToolModify(class figtoolview  *v, long  rock, boolean  firsttime) 
{
    if (firsttime) {
	message::DisplayString(v, 10, "Click on this tool again for symmetrical objects.");
	this->dummysymmetry = FALSE;
	return;
    }
    /* else not first time */

    this->dummysymmetry = (!this->dummysymmetry);

    if (this->dummysymmetry) 
	message::DisplayString(v, 10, "Objects will be constrained to be symmetrical.");
    else
	message::DisplayString(v, 10, "Objects will not be constrained to be symmetrical.");
}

class figobj *figorect::Instantiate(class figtoolview  *v, long  rock) 
{
    class figorect *res = (class figorect *)(this)->figobj::Instantiate( v, rock);

    res->dummysymmetry = this->dummysymmetry;

    return (class figobj *)res;
}

/* set bounding box and handle list in fig coordinates */
void figorect::RecomputeBounds()
{
    long left, width, top, height;
    long lwid;

    if ((this)->PosW() >= 0) {
	left = (this)->PosX();
	width = (this)->PosW();
    }
    else {
	left = (this)->PosX()+(this)->PosW();
	width = -(this)->PosW();
    }

    if ((this)->PosH() >= 0) {
	top = (this)->PosY();
	height = (this)->PosH();
    }
    else {
	top = (this)->PosY()+(this)->PosH();
	height = -(this)->PosH();
    }

    lwid = ((this)->GetVAttributes())->GetLineWidth( (this)->GetIVAttributes());
    lwid = lwid*figview_FigUPerPix;

    (this)->SetBoundsRect( left-lwid/2, top-lwid/2, width+lwid+1, height+lwid+1);

    (this)->SetHandle( 0, (this)->PosX()+(this)->PosW()/2, (this)->PosY()+(this)->PosH()/2);
    (this)->SetHandle( 1, (this)->PosX()+(this)->PosW(), (this)->PosY()+(this)->PosH()/2);
    (this)->SetHandle( 2, (this)->PosX()+(this)->PosW(), (this)->PosY());
    (this)->SetHandle( 3, (this)->PosX()+(this)->PosW()/2, (this)->PosY());
    (this)->SetHandle( 4, (this)->PosX(), (this)->PosY());
    (this)->SetHandle( 5, (this)->PosX(), (this)->PosY()+(this)->PosH()/2);
    (this)->SetHandle( 6, (this)->PosX(), (this)->PosY()+(this)->PosH());
    (this)->SetHandle( 7, (this)->PosX()+(this)->PosW()/2, (this)->PosY()+(this)->PosH());
    (this)->SetHandle( 8, (this)->PosX()+(this)->PosW(), (this)->PosY()+(this)->PosH());

    (this)->ComputeSelectedBounds();

    (this)->UpdateParentBounds();
}

static enum figobj_HandleType handletypes[9] = {
   figobj_Center,
   figobj_MiddleRight,
   figobj_URCorner,
   figobj_MiddleTop,
   figobj_ULCorner,
   figobj_MiddleLeft,
   figobj_LLCorner,
   figobj_MiddleBottom,
   figobj_LRCorner
};

enum figobj_HandleType figorect::GetHandleType(long  num)
{
    if(num>=0 && num<=8) return handletypes[num];
    else return figobj_None;
}

static long canonical[] = {
    3, 5, 7, 1, figobj_NULLREF
};

long *figorect::GetCanonicalHandles()
{
    return canonical;
}

void figorect::Draw(class figview  *v) 
{
    long x, y, w, h;
    long shad, lw, dash;
    const char *col;

    if ((this)->PosW() >= 0) {
	x = (v)->ToPixX( (this)->PosX());
	w = (v)->ToPixW( (this)->PosW());
    }
    else {
	x = (v)->ToPixX( (this)->PosX()+(this)->PosW());
	w = (v)->ToPixW( -(this)->PosW());
    }
    
    if ((this)->PosH() >= 0) {
	y = (v)->ToPixY( (this)->PosY());
	h = (v)->ToPixH( (this)->PosH());
    }
    else {
	y = (v)->ToPixY( (this)->PosY()+(this)->PosH());
	h = (v)->ToPixH( -(this)->PosH());
    }

    (v)->SetTransferMode( graphic_COPY);

    col = ((this)->GetVAttributes())->GetColor( (this)->GetIVAttributes());
    (v)->SetForegroundColor( col, 0, 0, 0); 

    shad = ((this)->GetVAttributes())->GetShade( (this)->GetIVAttributes());
    if (shad != figattr_ShadeClear)
	(v)->FillRectSize( x, y, w, h, (v)->GrayPattern( shad, figattr_ShadeDenom));

    lw = ((this)->GetVAttributes())->GetLineWidth( (this)->GetIVAttributes());
    lw = (v)->ToPixW( lw*figview_FigUPerPix);
    if (lw <= 0) lw = 1;
    if (lw != 1)
	(v)->SetLineWidth( lw);

    dash = ((this)->GetVAttributes())->GetLineStyle( (this)->GetIVAttributes());
    if (dash != figattr_LineSolid) {
	char *patterns = figattr::LineStylePattern(dash, lw);
	(v)->SetLineDash(patterns, 0, graphic_LineOnOffDash);
    }

    (v)->DrawRectSize(x, y, w, h); 

    if (lw != 1)
	(v)->SetLineWidth( 1);

    if (dash!=figattr_LineSolid)
	(v)->SetLineDash(NULL, 0, graphic_LineSolid);
}

void figorect::Sketch(class figview  *v) 
{
    long x, y, w, h;

    if ((this)->PosW() >= 0) {
	x = (v)->ToPixX( (this)->PosX());
	w = (v)->ToPixW( (this)->PosW());
    }
    else {
	x = (v)->ToPixX( (this)->PosX()+(this)->PosW());
	w = (v)->ToPixW( -(this)->PosW());
    }
    
    if ((this)->PosH() >= 0) {
	y = (v)->ToPixY( (this)->PosY());
	h = (v)->ToPixH( (this)->PosH());
    }
    else {
	y = (v)->ToPixY( (this)->PosY()+(this)->PosH());
	h = (v)->ToPixH( -(this)->PosH());
    }

    (v)->SetTransferMode( graphic_INVERT);
    (v)->DrawRectSize( x, y, w, h); 
}

enum figobj_HitVal figorect::HitMe(long  x , long  y, long  delta, long  *ptref) 
{
    enum figobj_HitVal res = (this)->BasicHitMe( x, y, delta, ptref);
    long x0, y0, x1, y1;
    long xin, yin;

    if (res==figobj_HitHandle)
	return res;

    x0 = (this)->PosX();
    x1 = x0 + (this)->PosW();
    y0 = (this)->PosY();
    y1 = y0 + (this)->PosH();

    xin = !((x<x0 && x<x1) || (x>x0 && x>x1));
    yin = !((y<y0 && y<y1) || (y>y0 && y>y1));

    if (x>=x0-delta && x<=x0+delta && yin) {
	if (ptref)
	    *ptref = 2;
	return figobj_HitBody;
    }
    if (x>=x1-delta && x<=x1+delta && yin) {
	if (ptref)
	    *ptref = 0;
	return figobj_HitBody;
    }
    if (y>=y0-delta && y<=y0+delta && xin) {
	if (ptref)
	    *ptref = 1;
	return figobj_HitBody;
    }
    if (y>=y1-delta && y<=y1+delta && xin) {
	if (ptref)
	    *ptref = 3;
	return figobj_HitBody;
    }

    return res;
}

enum figobj_Status figorect::Build(class figview  *v, enum view_MouseAction  action, long  x , long  y /* in fig coords */, long  clicks)   
{
    long px, py;
    int signx, signy;

    switch (action) {
	case view_LeftDown:
	    (this)->PosX() = x;
	    (this)->PosY() = y;
	    (this)->RecomputeBounds();
	    (this)->Sketch( v);
	    return figobj_NotDone;
	case view_LeftMovement:
	    (this)->Sketch( v);
	    if (this->dummysymmetry) {
		px = x - (this)->PosX();
		py = y - (this)->PosY();
		signx = 1;
		signy = 1;
		if (px<0) {
		    px = (-px);
		    signx = (-1);
		}
		if (py<0) {
		    py = (-py);
		    signy = (-1);
		}
		if (px<py) px = py;
		else py = px;
		(this)->PosW() = px * signx;
		(this)->PosH() = py * signy;
	    }
	    else {
		(this)->PosW() = x - (this)->PosX();
		(this)->PosH() = y - (this)->PosY();
	    }
	    (this)->Sketch( v);
	    return figobj_NotDone;
	case view_LeftUp:
	    (this)->Sketch( v);
	    if (this->dummysymmetry) {
		px = x - (this)->PosX();
		py = y - (this)->PosY();
		signx = 1;
		signy = 1;
		if (px<0) {
		    px = (-px);
		    signx = (-1);
		}
		if (py<0) {
		    py = (-py);
		    signy = (-1);
		}
		if (px<py) px = py;
		else py = px;
		(this)->PosW() = px * signx;
		(this)->PosH() = py * signy;
	    }
	    else {
		(this)->PosW() = x - (this)->PosX();
		(this)->PosH() = y - (this)->PosY();
	    }
	    (this)->RecomputeBounds();
	    px = (v)->ToPixW( (this)->PosW());
	    py = (v)->ToPixH( (this)->PosH());
	    if (px < figview_MouseHysteresis && px > -figview_MouseHysteresis && py < figview_MouseHysteresis && py > -figview_MouseHysteresis) {
		/* point click */
		message::DisplayString(v, 10, "Zero size; object aborted.");
		return figobj_Failed;
	    }
	    else {
		(this)->SetModified();
		return figobj_Done;
	    }
	default:
	    return figobj_Failed;
    }
}

/* basic procedure to move a handle -- used by figorect__MoveHandle(), figorect__Reshape() */
static void MoveHandle(class figorect  *self, long  x , long  y , long  ptref)
{
    long ix;

    switch (ptref) {
	case 0:
	    (self)->PosX() = x - (self)->PosW()/2;
	    (self)->PosY() = y - (self)->PosH()/2;
	    break;
	case 1:
	    (self)->PosW() = x - (self)->PosX();
	    break;
	case 2:
	    (self)->PosW() = x - (self)->PosX();
	    ix = (self)->PosY();
	    (self)->PosY() = y;
	    (self)->PosH() += (ix-y);
	    break;
	case 3:
	    ix = (self)->PosY();
	    (self)->PosY() = y;
	    (self)->PosH() += (ix-y);
	    break;
	case 4:
	    ix = (self)->PosY();
	    (self)->PosY() = y;
	    (self)->PosH() += (ix-y);
	    ix = (self)->PosX();
	    (self)->PosX() = x;
	    (self)->PosW() += (ix-x);
	    break;
	case 5:
	    ix = (self)->PosX();
	    (self)->PosX() = x;
	    (self)->PosW() += (ix-x);
	    break;
	case 6:
	    (self)->PosH() = y - (self)->PosY();
	    ix = (self)->PosX();
	    (self)->PosX() = x;
	    (self)->PosW() += (ix-x);
	    break;
	case 7:
	    (self)->PosH() = y - (self)->PosY();
	    break;
	case 8:
	    (self)->PosW() = x - (self)->PosX();
	    (self)->PosH() = y - (self)->PosY();
	    break;
    }
}

boolean figorect::Reshape(enum view_MouseAction  action, class figview  *v, long  x , long  y , boolean  handle, long  ptref)
{
    if (!handle)
	return FALSE;

    switch (action) {
	case view_LeftDown:
	case view_RightDown:
	    if ((this)->GetReadOnly())
		return FALSE;
	    break;
	case view_LeftMovement:
	case view_RightMovement:
	    (this)->Sketch( v);
	    ::MoveHandle(this, x, y, ptref);
	    (this)->Sketch( v);
	    break;
	case view_LeftUp:
	case view_RightUp:
	    (this)->Sketch( v);
	    ::MoveHandle(this, x, y, ptref);
	    (this)->RecomputeBounds();
	    (this)->SetModified();
	    break;
	default:
	    break;
    }
    return TRUE;
}

void figorect::MoveHandle(long  x , long  y , long  ptref)
{
    if ((this)->GetReadOnly())
	return;
    ::MoveHandle(this, x, y, ptref);
    (this)->RecomputeBounds();
    (this)->SetModified();
}

void figorect::Reposition(long  xd , long  yd)
{
    if ((this)->GetReadOnly())
	return;
    (this)->PosX() += xd;
    (this)->PosY() += yd;
    (this)->RecomputeBounds();
    (this)->SetModified();
}

void figorect::InheritVAttributes(class figattr  *attr, unsigned long  mask)
{
    (this)->figobj::InheritVAttributes( attr, mask);

    if (mask & (~((this)->GetVAttributes()->active)) & (1<<figattr_LineWidth)) {
	(this)->RecomputeBounds();
    }
}

unsigned long figorect::UpdateVAttributes(class figattr  *attr, unsigned long  mask)
{
    mask = (this)->figobj::UpdateVAttributes( attr, mask);
    
    if (mask & (1<<figattr_LineWidth))
	(this)->RecomputeBounds();

    return mask;
}

void figorect::WriteBody(FILE  *fp)
{
    (this)->figobj::WriteBody( fp);

    fprintf(fp, "$ %ld %ld\n", this->w, this->h);
}

long figorect::ReadBody(FILE  *fp, boolean  recompute)
{
    int	ix; 
    long w, h;

#define LINELENGTH (250)
    char buf[LINELENGTH+1];

    ix = (this)->figobj::ReadBody( fp, FALSE);
    if (ix!=dataobject_NOREADERROR) return ix;

    if (fgets(buf, LINELENGTH, fp) == NULL)
	return dataobject_PREMATUREEOF;
    ix = sscanf(buf, "$ %ld %ld", &w, &h);
    if (ix!=2) return dataobject_BADFORMAT;

    this->w = w;
    this->h = h;
    if (recompute) {
	(this)->RecomputeBounds();
	(this)->SetModified();
    }

    return dataobject_NOREADERROR;
}

#define FadeColor(col, shad)  (1.0 - (1.0-(shad)) * (1.0-(col)))

void figorect::PrintObject(class figview  *v, FILE  *file, const char  *prefix, boolean newstyle)
{
    long x, y, w, h;
    long shad, lw, dash;
    const char *col;
    double rcol, bcol, gcol, shadcol;

    fprintf(file, "%s  %% rectangle\n", prefix);

    if ((this)->PosW() >= 0) {
	x = (v)->ToPrintPixX( (this)->PosX());
	w = (v)->ToPrintPixW( (this)->PosW());
    }
    else {
	x = (v)->ToPrintPixX( (this)->PosX()+(this)->PosW());
	w = (v)->ToPrintPixW( -(this)->PosW());
    }
    
    if ((this)->PosH() >= 0) {
	y = (v)->ToPrintPixY( (this)->PosY());
	h = (v)->ToPrintPixH( (this)->PosH());
    }
    else {
	y = (v)->ToPrintPixY( (this)->PosY()+(this)->PosH());
	h = (v)->ToPrintPixH( -(this)->PosH());
    }

    fprintf(file, "%s  %ld %ld moveto  %ld %ld lineto  %ld %ld lineto  %ld %ld lineto closepath\n", prefix, x, y,  x, y+h,  x+w, y+h,  x+w, y);

    col = ((this)->GetVAttributes())->GetColor( (this)->GetIVAttributes());
    print::LookUpColor(col, &rcol, &gcol, &bcol);

    shad = ((this)->GetVAttributes())->GetShade( (this)->GetIVAttributes());
    if (shad != figattr_ShadeClear) {
	fprintf(file, "%s  gsave\n", prefix);
	shadcol = (double)(figattr_ShadeDenom-shad) / (double)figattr_ShadeDenom;
	fprintf(file, "%s  %f %f %f setrgbcolor\n", prefix, FadeColor(rcol, shadcol), FadeColor(gcol, shadcol), FadeColor(bcol, shadcol));
	/*fprintf(file, "%s  %f setgray\n", prefix, shadcol);*/
	fprintf(file, "%s  fill grestore\n", prefix);
    }
    lw = ((this)->GetVAttributes())->GetLineWidth( (this)->GetIVAttributes());
    lw = (v)->ToPrintPixW( lw*figview_FigUPerPix);
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
}
