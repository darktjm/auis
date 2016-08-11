/* figorrec.c - fig element object: rounded rectangle */
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
#ifndef NORCSID
char *figorrec_c_rcsid = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/figure/RCS/figorrec.C,v 3.5 1994/11/30 20:42:06 rr2b Stab74 $";
#endif

#include <andrewos.h>
ATK_IMPL("figorrec.H")
#include <figorrec.H>

#include <view.H>
#include <figview.H>
#include <figtoolview.H>
#include <figattr.H>
#include <print.H>


ATKdefineRegistry(figorrec, figorect, NULL);
#ifndef NORCSID
#endif


figorrec::figorrec()
{
    (this)->AttributesUsed() = (1<<figattr_Shade) | (1<<figattr_LineWidth)  | (1<<figattr_LineStyle) | (1<<figattr_Color) | (1<<figattr_RRectCorner);

    THROWONFAILURE( TRUE);
}

class figorrec *figorrec::Create(long  left , long  top , long  width , long  height)
{
    class figorrec *res = new figorrec;
    if (!res) return NULL;

    (res)->PosX() = left;
    (res)->PosY() = top;
    (res)->PosW() = width;
    (res)->PosH() = height;
    (res)->RecomputeBounds();

    return res;
}

char *figorrec::ToolName(class figtoolview  *v, long  rock)
{
    return "Round Rect";
}

void figorrec::Draw(class figview  *v) 
{
    long x, y, w, h;
    long shad, lw, corn, dash;
    char *col;

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
    corn = ((this)->GetVAttributes())->GetRRectCorner( (this)->GetIVAttributes());
    corn = (v)->ToPixW( corn*figview_FigUPerPix);

    if (shad != figattr_ShadeClear)
	(v)->FillRRectSize( x, y, w, h, corn, corn, (v)->GrayPattern( shad, figattr_ShadeDenom));

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

    (v)->DrawRRectSize( x, y, w, h, corn, corn); 

    if (lw != 1)
	(v)->SetLineWidth( 1);

    if (dash!=figattr_LineSolid)
	(v)->SetLineDash(NULL, 0, graphic_LineSolid);
}

void figorrec::Sketch(class figview  *v) 
{
    long x, y, w, h, corn;

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

    corn = ((this)->GetVAttributes())->GetRRectCorner( (this)->GetIVAttributes());
    corn = (v)->ToPixW( corn*figview_FigUPerPix);

    (v)->SetTransferMode( graphic_INVERT);
    (v)->DrawRRectSize( x, y, w, h, corn, corn); 
    /*figview_DrawRectSize(v, x, y, w, h);  */
}

#define FadeColor(col, shad)  (1.0 - (1.0-(shad)) * (1.0-(col)))

void figorrec::PrintObject(class figview  *v, FILE  *file, char  *prefix, boolean newstyle)
{
    long x, y, w, h;
    long shad, lw, corn, dash;
    char *col;
    double rcol, bcol, gcol, shadcol;

    fprintf(file, "%s  %% rounded rectangle\n", prefix);

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
    if (w==0)
	w=1;
    if (h==0)
	h=1;

    corn = ((this)->GetVAttributes())->GetRRectCorner( (this)->GetIVAttributes());
    corn = (v)->ToPrintPixW( corn*figview_FigUPerPix);

    if (corn==0) {
	fprintf(file, "%s  %d %d moveto  %d %d lineto  %d %d lineto  %d %d lineto closepath\n", prefix, x, y,  x, y+h,  x+w, y+h,  x+w, y);
    }
    else if ((2*corn >= h) || (2*corn >= w)) {
	/* Bizarre -- corners are bigger than rectangle, so 
	 make an appropriate looking oval */
	if ((2*corn >= h) && (2*corn >= w)) {
	    fprintf(file, "%s  %d %d translate  %d %d scale  0 0 0.5 0 360 arc\n", prefix, x+w/2, y+h/2, w, h);
	    fprintf(file, "%s  1.0 %d div  1.0 %d div  scale\n", prefix, w, h);
	}
	else if (2*corn >= h) {
	    /* Draw left semi-oval */
	    fprintf(file, "%s  %d %d translate  %d %d scale  0 0 0.5 90 270 arc\n", prefix, x+corn, y+h/2, 2*corn, h);
	    fprintf(file, "%s  1.0 %d div  1.0 %d div  scale\n", prefix, 2*corn, h);
	    /* Draw right semi-oval */
	    fprintf(file, "%s  %d %d translate  %d %d scale  0 0 0.5 270 450 arc\n", prefix, w-2*corn, 0, 2*corn, h);
	    fprintf(file, "%s  1.0 %d div  1.0 %d div  scale\n", prefix, 2*corn, h);
	    fprintf(file, "%s  closepath\n", prefix);
	}
	else { /* assuming (2*corn >= w) */
	    /* Draw top semi-oval */
	    fprintf(file, "%s  %d %d translate  %d %d scale  0 0 0.5 -180 0 arc\n", prefix, x+w/2, y+corn, w, 2*corn);
	    fprintf(file, "%s  1.0 %d div  1.0 %d div  scale\n", prefix, w, 2*corn);
	    /* Draw bottom semi-oval */
	    fprintf(file, "%s  %d %d translate  %d %d scale  0 0 0.5 0 180 arc\n", prefix, 0, h-2*corn, w, 2*corn);
	    fprintf(file, "%s  1.0 %d div  1.0 %d div  scale\n", prefix, w, 2*corn);
	    fprintf(file, "%s  closepath\n", prefix);
	}
	
    }
    else {
	fprintf(file, "%s  %d %d translate  %d %d scale  0 0 0.5 180 270 arc\n", prefix, x+corn, y+corn, 2*corn, 2*corn);
	fprintf(file, "%s  1.0 %d div  1.0 %d div  scale\n", prefix, 2*corn, 2*corn);
	fprintf(file, "%s  %d %d translate  %d %d scale  0 0 0.5 270 360 arc\n", prefix, w-2*corn, 0, 2*corn, 2*corn);
	fprintf(file, "%s  1.0 %d div  1.0 %d div  scale\n", prefix, 2*corn, 2*corn);
	fprintf(file, "%s  %d %d translate  %d %d scale  0 0 0.5 360 450 arc\n", prefix, 0, h-2*corn, 2*corn, 2*corn);
	fprintf(file, "%s  1.0 %d div  1.0 %d div  scale\n", prefix, 2*corn, 2*corn);
	fprintf(file, "%s  %d %d translate  %d %d scale  0 0 0.5 450 540 arc\n", prefix, -(w-2*corn), 0, 2*corn, 2*corn);
	fprintf(file, "%s  1.0 %d div  1.0 %d div  scale\n", prefix, 2*corn, 2*corn);
	fprintf(file, "%s  closepath\n", prefix);
    }

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
    fprintf(file, "%s  %d setlinewidth\n", prefix, lw);
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
