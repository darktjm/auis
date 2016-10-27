/* figoell.c - fig element object: ellipse */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/
#include <andrewos.h>
ATK_IMPL("figoell.H")
#include <figoell.H>

#include <view.H>
#include <figview.H>
#include <figtoolview.H>
#include <figattr.H>
#include <print.H>


ATKdefineRegistryNoInit(figoell, figorect);

figoell::figoell()
{
    THROWONFAILURE( TRUE);
}

class figoell *figoell::Create(long  left , long  top , long  width , long  height)
{
    class figoell *res = new figoell;
    if (!res) return NULL;

    (res)->PosX() = left;
    (res)->PosY() = top;
    (res)->PosW() = width;
    (res)->PosH() = height;
    (res)->RecomputeBounds();

    return res;
}

const char *figoell::ToolName(class figtoolview  *v, long  rock)
{
    return "Ellipse";
}

enum figobj_HitVal figoell::HitMe(long  x , long  y, long  delta, long  *ptref) 
{
    enum figobj_HitVal res = (this)->BasicHitMe( x, y, delta, ptref);
    long x0, y0, x1, y1;
    double dx, dy, sum, ddel;

    if (res!=figobj_HitInside)
	return res;

    x1 = (this)->PosW()/2;
    y1 = (this)->PosH()/2;

    x0 = x - ((this)->PosX() + x1);
    y0 = y - ((this)->PosY() + y1);

    if (x1<0) x1 = (-x1);
    if (y1<0) y1 = (-y1);

    dx = (double)(x0) / (double)(x1);
    dy = (double)(y0) / (double)(y1);
    sum = dx*dx + dy*dy;
    if (x1<y1)
	ddel = 2.0 * (double)delta / (double)x1;
    else
	ddel = 2.0 * (double)delta / (double)y1;
    if (sum >= 1.0-ddel && sum <= 1.0+ddel) {
	if (ptref)
	    *ptref = 0;
	return figobj_HitBody;
    }
    if (sum < 1.0) {
	return figobj_HitInside;
    }

    return figobj_Miss;
}

void figoell::Draw(class figview  *v) 
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

    (v)->SetTransferMode( graphic::COPY);

    col = ((this)->GetVAttributes())->GetColor( (this)->GetIVAttributes());
    (v)->SetForegroundColor( col, 0, 0, 0); 

    shad = ((this)->GetVAttributes())->GetShade( (this)->GetIVAttributes());
    if (shad != figattr_ShadeClear)
	(v)->FillOvalSize( x, y, w, h, (v)->GrayPattern( shad, figattr_ShadeDenom));

    lw = ((this)->GetVAttributes())->GetLineWidth( (this)->GetIVAttributes());
    lw = (v)->ToPixW( lw*figview_FigUPerPix);
    if (lw <= 0) lw = 1;
    if (lw != 1)
	(v)->SetLineWidth( lw);

    dash = ((this)->GetVAttributes())->GetLineStyle( (this)->GetIVAttributes());
    if (dash != figattr_LineSolid) {
	char *patterns = figattr::LineStylePattern(dash, lw);
	(v)->SetLineDash((unsigned char *)patterns, 0, graphic::LineOnOffDash);
    }

    (v)->DrawOvalSize(x, y, w, h); 

    if (lw != 1)
	(v)->SetLineWidth( 1);

    if (dash!=figattr_LineSolid)
	(v)->SetLineDash(NULL, 0, graphic::LineSolid);
}

void figoell::Sketch(class figview  *v) 
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

    (v)->SetTransferMode( graphic::INVERT);
    (v)->DrawOvalSize( x, y, w, h); 
    /*figview_DrawRectSize(v, x, y, w, h); */
}

#define FadeColor(col, shad)  (1.0 - (1.0-(shad)) * (1.0-(col)))

void figoell::PrintObject(class figview  *v, FILE  *file, const char  *prefix, boolean newstyle)
{
    long x, y, w, h;
    long shad, lw, dash;
    const char *col;
    double rcol, bcol, gcol, shadcol;

    fprintf(file, "%s  %% ellipse\n", prefix);

    w = (v)->ToPrintPixW( (this)->PosW());
    x = (v)->ToPrintPixX( (this)->PosX()) + w/2;
    if (w<0)
	w = (-w);
    else if (w==0)
	w = 1;

    h = (v)->ToPrintPixH( (this)->PosH());
    y = (v)->ToPrintPixY( (this)->PosY()) + h/2;
    if (h<0)
	h = (-h);
    else if (h==0)
	h = 1;

    fprintf(file, "%s  %ld %ld translate  %ld %ld scale  0 0 0.5 0 360 arc\n", prefix, x, y, w, h);
    fprintf(file, "%s  1.0 %ld div  1.0 %ld div  scale\n", prefix, w, h);

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
