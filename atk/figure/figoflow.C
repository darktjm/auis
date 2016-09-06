/* figoflow.c - fig element object: text flow unit */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
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
#include <andrewos.h>
ATK_IMPL("figoflow.H")
#include <figoflow.H>

#include <view.H>
#include <figview.H>
#include <figtoolview.H>
#include <figattr.H>
#include <print.H>
#include <fontdesc.H>

static class fontdesc *ClassFontDesc;

ATKdefineRegistry(figoflow, figorect, figoflow::InitializeClass);
boolean figoflow_OrderFlows(struct figobj *o, long ref, struct figure *self, long *rock);

boolean figoflow::InitializeClass()
{
    ClassFontDesc = fontdesc::Create("andysans", fontdesc_Bold, 12);
    return TRUE;
}

figoflow::figoflow()
{
    (this)->AttributesUsed() = 0;
    this->figmodified = 0;
    this->order = 0;
    THROWONFAILURE( TRUE);
}

class figoflow *figoflow::Create(long  left , long  top , long  width , long  height)
{
    class figoflow *res = new figoflow;
    if (!res) return NULL;

    (res)->PosX() = left;
    (res)->PosY() = top;
    (res)->PosW() = width;
    (res)->PosH() = height;
    (res)->RecomputeBounds();

    return res;
}

const char *figoflow::ToolName(class figtoolview  *v, long  rock)
{
    return "Text Flow";
}

enum figobj_HitVal figoflow::HitMe(long  x , long  y, long  delta, long  *ptref) 
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

static ATKregistryEntry *figoflow_reg = NULL;
boolean figoflow_OrderFlows(struct figobj *o, long ref, struct figure *self, long *rock)
{
    if (o->IsType(figoflow_reg)) {
	class figoflow *flo = (class figoflow *)o;
	long oldorder = flo->order;
	flo->order = (*rock);
	(*rock)++;
	flo->figmodified = self->modified;
	if (oldorder != flo->order) {
	    flo->SetModified();
	}
    }
    return FALSE;
}

void figoflow::ObservedChanged(class observable  *obs, long  status)
{
    if (obs == (class observable *)(this->GetAncestorFig())) {
	if (status==figure_DATACHANGED) {
	    if (!figoflow_reg) {
		figoflow_reg = ATK::LoadClass("figoflow");
	    }

	    if (this->figmodified != this->GetAncestorFig()->modified) {
		long num;
		num = 1;
		this->GetAncestorFig()->EnumerateObjectTree(figure_NULLREF, NULL, TRUE, (figure_eofptr)figoflow_OrderFlows, (long)(&num));
	    }

	}
    }
    else {
	fprintf(stderr, "figoflow: observed the wrong thing\n");
    }
}

void figoflow::Draw(class figview  *v) 
{
    long x, y, w, h;
    char buf[32];

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
    (v)->SetForegroundColor( "black", 0, 0, 0); 
    (v)->DrawRectSize(x, y, w, h);
    if (w > 16) {
	v->MoveTo(x+16, y);
	v->DrawLineTo(x+16, y+h);
	sprintf(buf, "%ld", this->order);
	v->SetFont( ClassFontDesc);
	v->MoveTo(x+4, y+4);
	v->DrawString(buf, graphic_ATLEFT|graphic_ATTOP);
    }
}

void figoflow::Sketch(class figview  *v) 
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

void figoflow::SetParent(long  pref, class figure  *ancfig)
{
    if (this->figo) {
	this->figo->RemoveObserver(this);
    }

    this->figorect::SetParent(pref, ancfig);

    if (this->figo) {
	this->figo->AddObserver(this);
    }
}

long figoflow::Read(FILE  *fp, long  id)
{
    long res = this->figorect::Read(fp, id);
    if (this->figo) {
	(this->figo)->NotifyObservers( figure_DATACHANGED); /* I don't know if this is the right thing to do */
    }
    return res;
}

void figoflow::PrintObject(class figview  *v, FILE  *file, const char  *prefix, boolean newstyle)
{
    fprintf(file, "%s  %% text flow\n", prefix);
}
