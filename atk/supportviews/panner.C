/* panner.c - panner box view */

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
ATK_IMPL("panner.H")
#include <panner.H>
#include <view.H>
#include <cursor.H>
#include <im.H>
#include <updatelist.H>
#include <region.H>
#include <environ.H>
#include <sbuttonv.H>

#include <rect.h>


#undef MIN
#undef MAX
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define EQRECT(a, b)  ((a)->left == (b)->left && (a)->top == (b)->top && (a)->width == (b)->width && (a)->height == (b)->height)


ATKdefineRegistry(panner, scroll, panner::InitializeClass);
static void recompute_panrect(class panner  *self);
static void get_interface(class panner  *self, int  type /* scroll_VERT or scroll_HORIZ */);
static void getinfo(class panner  *self, int  type /* scroll_VERT or scroll_HORIZ */, struct range  *total , struct range  *seen , struct range  *dot);
static void calc_desired(class panner  *self);
static void update_everything(class panner  *self, boolean  wipeold);
static void draw_everything(class panner  *self, boolean  wipeback);
static void move_panner(class panner  *self);
static void HandleDragging(class panner  *self, enum view_MouseAction  action, long  x , long  y , long  num_clicks);
static void HandleThumbing(class panner  *self, enum view_MouseAction  action, long  x , long  y , long  num_clicks);
static void HandleDownHit(class panner  *self, enum view_MouseAction  action, long  x , long  y , long  num_clicks);


boolean panner::InitializeClass()
{
    return TRUE;
}

static int cursortypes[panner_NumberOfCursors] = {Cursor_SmallCross, Cursor_Octagon, Cursor_Gunsight, Cursor_CrossHairs};

panner::panner()
{
	ATKinit;

    int ix;

    this->MotifStyle = environ::GetProfileSwitch("MotifPanners", FALSE);
    this->PannerShade = environ::GetProfileInt("PannerShade", 8);

    this->visible = FALSE;
    rectangle_EmptyRect(&this->panrect);
    this->oldvisible = FALSE;
    rectangle_EmptyRect(&this->oldpanrect);

    this->idealwidth = 100;
    this->idealheight = 100;

    this->childclip = new region;
    this->pannerreg = new region;

    for (ix=0; ix<panner_NumberOfCursors; ix++) {
	this->cursors[ix] = cursor::Create((class view *)this);
	if (this->cursors[ix]) 
	    (this->cursors[ix])->SetStandard( cursortypes[ix]);
    }

    THROWONFAILURE( TRUE);
}

panner::~panner()
{
	ATKinit;

    int ix;

    delete this->childclip;
    delete this->pannerreg;

    for(ix=0; ix<scroll_TYPES; ix++) 
	if(this->cursors[ix]) 
	    delete this->cursors[ix];
}

class panner *panner::Create(class view  *scrollee)
{
	ATKinit;

    class panner *retval = new panner;

    (retval)->SetView( scrollee);
    (retval)->SetLocation( scroll_TOP|scroll_LEFT);

    return retval;
}


void panner::SetLocation(int  location)
{
    (this)->scroll::SetLocation( scroll_TOP|scroll_LEFT);
}

void panner::PostCursor(struct rectangle  *rec, class cursor  *cursor)
{
    (this)->scroll::PostCursor( rec, cursor);
    if (cursor != this->cursors[0] && cursor != this->cursors[3]) {
	(this)->RetractCursor( this->cursors[0]);
	(this)->RetractCursor( this->cursors[3]);
	(this)->PostCursor( &(this->panrect), this->cursors[0]);
	(this)->PostCursor( &(this->gseenrect), this->cursors[3]);
    }
}

static void recompute_panrect(class panner  *self)
{
    struct rectangle *r = &(((class scroll *)self)->childrect);

    if (rectangle_IsEmptyRect(&self->panrect)) {

	if (r->width < self->idealwidth+10 || r->height < self->idealheight+10) {
	    self->visible = FALSE;
	    rectangle_EmptyRect(&self->panrect);
	    return;
	}

	self->panrect.width = self->idealwidth;
	self->panrect.height = self->idealheight;

	self->panrect.left = r->left + (r->width - self->panrect.width);
	if (self->panrect.left > 50) self->panrect.left = 50;

	self->panrect.top = r->top + (r->height - self->panrect.height);
	if (self->panrect.top > 50) self->panrect.top = 50;

	self->visible = TRUE;
	return;
    }

    self->panrect.left = MAX(self->panrect.left, 0);
    self->panrect.left = MIN(self->panrect.left, r->width - self->panrect.width);
    if (self->panrect.left < 0) {
	self->visible = FALSE;
	return;
    }

    self->panrect.top = MAX(self->panrect.top, 0);
    self->panrect.top = MIN(self->panrect.top, r->height - self->panrect.height);

    if (self->panrect.top < 0) {
	self->visible = FALSE;
	return;
    }

    self->visible = TRUE;
}

static const char * const InterfaceName[scroll_TYPES] = {"scroll,vertical", "scroll,horizontal"};

/* right now, only called from getinfo(); may put in-line */
static void get_interface(class panner  *self, int  type /* scroll_VERT or scroll_HORIZ */)
{
    self->force_get_interface = FALSE;
    if (self->fns[type] == NULL)
        self->fns[type] = (const struct scrollfns *)(self->scrollee)->GetInterface( InterfaceName[type]);
}

static void getinfo(class panner  *self, int  type /* scroll_VERT or scroll_HORIZ */, struct range  *total , struct range  *seen , struct range  *dot)
{
    scroll_getinfofptr real_getinfo;

    get_interface(self, type);

    if (self->fns[type] != NULL && (real_getinfo = self->fns[type]->GetInfo) != NULL)
        real_getinfo(self->scrollee, total, seen, dot);

    if (total->beg == total->end) {
        total->end++;
        *seen = *total;
        dot->beg = dot->end = total->beg;
    }
}

static void calc_desired(class panner  *self)
{
    int i;

    for (i = 0; i < scroll_TYPES; i++) {
	struct scrollbar *bar = &(self->desired.bar[i]);
	getinfo(self, i, &bar->total, &bar->seen, &bar->dot);
    }
}

static void update_everything(class panner  *self, boolean  wipeold)
{
    struct sbuttonv_view_info vi;
    long maxwid, maxhgt;
    struct scrollbar *tmp;
    static struct rectangle oldseenrect, olddotrect;

    sbuttonv::SaveViewState(self, &vi);

    oldseenrect = self->seenrect;
    olddotrect = self->dotrect;

    maxwid = self->panrect.width-5;
    maxhgt = self->panrect.height-5;

    if (self->fns[scroll_HORIZ]) {
	tmp = &(self->desired.bar[scroll_HORIZ]);
	self->seenrect.left =  (tmp->seen.beg - tmp->total.beg) * maxwid / (tmp->total.end - tmp->total.beg);
	self->seenrect.width =  (tmp->seen.end - tmp->seen.beg) * maxwid / (tmp->total.end - tmp->total.beg);

	self->dotrect.left =  (tmp->dot.beg - tmp->total.beg) * maxwid / (tmp->total.end - tmp->total.beg);
	self->dotrect.width =  (tmp->dot.end - tmp->dot.beg) * maxwid / (tmp->total.end - tmp->total.beg);

	if (self->seenrect.width < 2) {
	    self->seenrect.width = 2;
	    self->seenrect.left -= 1;
	}
	if (self->dotrect.width < 2) {
	    self->dotrect.width = 2;
	    self->dotrect.left -= 1;
	}
    }
    else {
	self->seenrect.left = 0;
	self->seenrect.width = maxwid;
	self->dotrect.left = 2;
	self->dotrect.width = maxwid-3;
    }

    if (self->fns[scroll_VERT]) {
	tmp = &(self->desired.bar[scroll_VERT]);
	self->seenrect.top =  (tmp->seen.beg - tmp->total.beg) * maxwid / (tmp->total.end - tmp->total.beg);
	self->seenrect.height =  (tmp->seen.end - tmp->seen.beg) * maxwid / (tmp->total.end - tmp->total.beg);

	self->dotrect.top =  (tmp->dot.beg - tmp->total.beg) * maxwid / (tmp->total.end - tmp->total.beg);
	self->dotrect.height =  (tmp->dot.end - tmp->dot.beg) * maxwid / (tmp->total.end - tmp->total.beg);

	if (self->seenrect.height < 2) {
	    self->seenrect.height = 2;
	    self->seenrect.top -= 1;
	}
	if (self->dotrect.height < 2) {
	    self->dotrect.height = 2;
	    self->dotrect.top -= 1;
	}
    }
    else {
	self->seenrect.top = 0;
	self->seenrect.height = maxhgt;
	self->dotrect.top = 2;
	self->dotrect.height = maxhgt-3;
    }

    self->gseenrect.width = self->seenrect.width;
    self->gseenrect.height = self->seenrect.height;
    self->gseenrect.left = self->panrect.left+2+self->seenrect.left;
    self->gseenrect.top = self->panrect.top+2+self->seenrect.top;

    if (!self->MotifStyle) {
	(self)->SetTransferMode( graphic_INVERT);

	if (wipeold) {
	    if (!EQRECT(&self->seenrect, &oldseenrect)) {
		(self)->DrawRectSize( self->panrect.left+2+oldseenrect.left, self->panrect.top+2+oldseenrect.top, oldseenrect.width, oldseenrect.height);
		(self)->DrawRectSize( self->gseenrect.left, self->gseenrect.top, self->gseenrect.width, self->gseenrect.height);
	    }
	    if (!EQRECT(&self->dotrect, &olddotrect)) {
		(self)->FillRectSize( self->panrect.left+2+olddotrect.left, self->panrect.top+2+olddotrect.top, olddotrect.width, olddotrect.height, (self)->BlackPattern());
		(self)->FillRectSize( self->panrect.left+2+self->dotrect.left, self->panrect.top+2+self->dotrect.top, self->dotrect.width, self->dotrect.height, (self)->BlackPattern());
	    }
	}
	else {
	    /* just draw them */
	    (self)->DrawRectSize( self->gseenrect.left, self->gseenrect.top, self->gseenrect.width, self->gseenrect.height);

	    (self)->FillRectSize( self->panrect.left+2+self->dotrect.left, self->panrect.top+2+self->dotrect.top, self->dotrect.width, self->dotrect.height, (self)->BlackPattern());
	}
    }
    else { /* Motif Style */
	(self)->SetTransferMode( graphic_COPY);

	if (wipeold) {
	    (self)->FillRectSize( self->panrect.left+1, self->panrect.top+1, self->panrect.width-2, self->panrect.height-2, (self)->GrayPattern( self->PannerShade, 16));
	}
	(self)->FillRectSize( self->panrect.left+2+self->dotrect.left, self->panrect.top+2+self->dotrect.top, self->dotrect.width, 1, (self)->BlackPattern());
	(self)->FillRectSize( self->panrect.left+2+self->dotrect.left, self->panrect.top+2+self->dotrect.top, 1, self->dotrect.height, (self)->BlackPattern());
	(self)->FillRectSize( self->panrect.left+2+self->dotrect.left, self->panrect.top+2+self->dotrect.top+self->dotrect.height, self->dotrect.width+1, 1, (self)->WhitePattern());
	(self)->FillRectSize( self->panrect.left+2+self->dotrect.left+self->dotrect.width, self->panrect.top+2+self->dotrect.top, 1, self->dotrect.height+1, (self)->WhitePattern());

	(self)->FillRectSize( self->gseenrect.left, self->gseenrect.top, self->gseenrect.width, 1, (self)->WhitePattern());
	(self)->FillRectSize( self->gseenrect.left, self->gseenrect.top, 1, self->gseenrect.height, (self)->WhitePattern());
	(self)->FillRectSize( self->gseenrect.left, self->gseenrect.top+self->gseenrect.height, self->gseenrect.width+1, 1, (self)->BlackPattern());
	(self)->FillRectSize( self->gseenrect.left+self->gseenrect.width, self->gseenrect.top, 1, self->gseenrect.height+1, (self)->BlackPattern());
    }

    sbuttonv::RestoreViewState(self, &vi);
}

/* draw everything, assuming (or forcing) a blank background. sself->desired should already have been set, with calc_desired(). */
static void draw_everything(class panner  *self, boolean  wipeback)
{
    struct sbuttonv_view_info vi;

    if (self->visible) {

	sbuttonv::SaveViewState(self, &vi);

	if (!self->MotifStyle) {
	    /*if (wipeback) */ /* doesn't seem to do the right thing, so we'll always erase the background. */
	    (self)->EraseRect( &self->panrect);
	    (self)->DrawRectSize( self->panrect.left, self->panrect.top, self->panrect.width-1, self->panrect.height-1);
	}
	else {
	    (self)->FillRect( &self->panrect, (self)->GrayPattern( self->PannerShade, 16));
	    (self)->FillRectSize( self->panrect.left, self->panrect.top, self->panrect.width-1, 1, (self)->WhitePattern());
	    (self)->FillRectSize( self->panrect.left, self->panrect.top, 1, self->panrect.height-1, (self)->WhitePattern());
	    (self)->FillRectSize( self->panrect.left, self->panrect.top+self->panrect.height-1, self->panrect.width, 1, (self)->BlackPattern());
	    (self)->FillRectSize( self->panrect.left+self->panrect.width-1, self->panrect.top, 1, self->panrect.height, (self)->BlackPattern());
	}

	sbuttonv::RestoreViewState(self, &vi);

	update_everything(self, FALSE);
    }
}

/* assumes panrect, visible have already been set */
static void move_panner(class panner  *self)
{
    (self->childclip)->RectRegion( &self->childrect); 
    if (self->visible) {
	(self->pannerreg)->RectRegion( &self->panrect); 
	(self->childclip)->SubtractRegion( self->pannerreg, self->childclip);
    }
    else
	(self->pannerreg)->ClearRegion();
    (self->child)->InsertViewRegion( self, self->childclip);

    /* ask for update in old region */
    if (self->oldvisible) {
	(self)->EraseRect( &self->oldpanrect);
	(self->child)->FullUpdate( view_LastPartialRedraw, self->oldpanrect.left, self->oldpanrect.top, self->oldpanrect.width, self->oldpanrect.height);
    }
    self->oldpanrect = self->panrect;
    self->oldvisible = self->visible;

    self->panrectchanged = FALSE;
    draw_everything(self, TRUE);
    (self)->PostCursor( &(self->panrect), self->cursors[0]);
    (self)->PostCursor( &(self->gseenrect), self->cursors[3]);
}

void panner::FullUpdate(enum view_UpdateType  type, long  left , long  top , long  width , long  height)
{
    class panner *self=this;
    struct rectangle r;
    int i;

    /*if(panner_GetIM(self) && !self->prefsready) {
	InitPrefs(self);
    }*/
    
    (this)->GetLogicalBounds( &r);
    
    this->pending_update = 0;
    
    if (this->force_get_interface) {
	for (i = 0; i < scroll_TYPES; i++) {
            this->fns[i] = NULL;
        }
    }

    if (type!=view_Remove 
	 && (this->lastwidth!=r.width ||
	     this->lastheight!=r.height)
	 && this->child) {
	this->lastwidth=r.width;
	this->lastheight=r.height;
	this->childrect=r;
	recompute_panrect(this);
	/* view_InsertView(this->child, self, &this->childrect); */
	move_panner(this);
    }

    this->force_full_update = FALSE;
    switch(type) {
	case view_Remove:
	    (this)->RetractCursor( this->cursors[0]);
	    if(this->child) (this->child)->FullUpdate( type, left, top, width, height);
	    return;
	case view_MoveNoRedraw:
	    if(this->child) (this->child)->FullUpdate( view_MoveNoRedraw, left, top, width, height);
	    break;
	case view_PartialRedraw:
	    if(this->child) (this->child)->FullUpdate( type, (this->child)->EnclosedXToLocalX( left),  (this->child)->EnclosedYToLocalY(top), width, height);
	    break;
	case view_LastPartialRedraw:
	    if(this->child) (this->child)->FullUpdate( type, (this->child)->EnclosedXToLocalX( left),  (this->child)->EnclosedYToLocalY(top), width, height);
	    calc_desired(this);
	    draw_everything(this, FALSE);
	    break;
	case view_FullRedraw:
	    if(this->child) (this->child)->FullUpdate( view_FullRedraw, r.left, r.top, r.width, r.height);
	    calc_desired(this);
	    draw_everything(this, FALSE);
	    break;
	default: 
	    break;
    }

    if(type!=view_Remove && type!=view_PartialRedraw) {
	(this)->RetractCursor( this->cursors[0]);
	(this)->RetractCursor( this->cursors[3]);
	(this)->PostCursor( &(this->panrect), this->cursors[0]);
	(this)->PostCursor( &(this->gseenrect), this->cursors[3]);
    }

    this->current = this->desired;
    (this)->FlushGraphics();
}

void panner::Update()
{
    int i;
    struct rectangle r;
    (this)->GetLogicalBounds( &r);

    if (r.width <= 0 || r.height <= 0) return;

    this->pending_update = 0;
    /* Let the children modify their state however they want. */
    (this->updatelistp)->Clear();
    calc_desired(this);

    if (this->current.location != this->desired.location || this->force_full_update) {
	(this)->EraseVisualRect();
	(this)->FullUpdate( view_FullRedraw, r.left, r.top, r.width, r.height);
    } 
    else {
	if (this->panrectchanged) 
	    move_panner(this);
	else {
	    int type;
	    boolean refiddle = FALSE;

	    for (type=0; type<scroll_TYPES; type++) {
		struct scrollbar *des = &this->desired.bar[type], *cur = &this->current.bar[type];
		if (des->total.beg != cur->total.beg || des->total.end != cur->total.end || des->seen.beg != cur->seen.beg || des->dot.beg != cur->dot.beg || des->seen.end != cur->seen.end || des->dot.end != cur->dot.end) {
		    refiddle = TRUE;
		}
	    }
	    if (refiddle)
		update_everything(this, TRUE);
	}
	this->current = this->desired;
    }
}

static void HandleDragging(class panner  *self, enum view_MouseAction  action, long  x , long  y , long  num_clicks)
{
    if (action==view_RightUp || action==view_LeftUp) {
	self->panrect.left = x - self->rockx;
	self->panrect.top = y - self->rocky;
	self->panrect.width = self->idealwidth;
	self->panrect.height = self->idealheight;
	if (self->panrect.left < 0) self->panrect.left = 0;
	if (self->panrect.top < 0) self->panrect.top = 0;
	if (self->panrect.left >= self->childrect.width - self->panrect.width)
	    self->panrect.left = self->childrect.width - self->panrect.width;
	if (self->panrect.top >= self->childrect.height - self->panrect.height)
	    self->panrect.top = self->childrect.height - self->panrect.height;
	self->panrectchanged = TRUE;
	self->visible = TRUE;
	(self)->WantUpdate( self);
    }
}

static void HandleThumbing(class panner  *self, enum view_MouseAction  action, long  x , long  y , long  num_clicks)
{
    scroll_setframefptr SetFrame;
    struct scrollbar *tmp;
    long valx, valy;

    x -= self->rockx;
    y -= self->rocky;

    /* turn panner coords to view coords */

    if (self->fns[scroll_HORIZ]) {
	tmp = &(self->desired.bar[scroll_HORIZ]);
	valx = x * (tmp->total.end - tmp->total.beg) / (self->panrect.width-5);
	valx = MIN(valx, tmp->total.end);
	valx = MAX(valx, tmp->total.beg);
	SetFrame = self->fns[scroll_HORIZ]->SetFrame;
	if (SetFrame) {
	    SetFrame(self->scrollee, valx, self->isatx, self->childrect.width);
	}
    }

    if (self->fns[scroll_VERT]) {
	tmp = &(self->desired.bar[scroll_VERT]);
	valy = y * (tmp->total.end - tmp->total.beg) / (self->panrect.height-5);
	valy = MIN(valy, tmp->total.end);
	valy = MAX(valy, tmp->total.beg);
	SetFrame = self->fns[scroll_VERT]->SetFrame;
	if (SetFrame) {
	    SetFrame(self->scrollee, valy, self->isaty, self->childrect.height);
	}
    }
}

static void HandleDownHit(class panner  *self, enum view_MouseAction  action, long  x , long  y , long  num_clicks)
{
    scroll_whatfptr WhatIsAt;
    long valx, valy;

    if (action==view_RightDown) {
	self->rockx = x - self->panrect.left;
	self->rocky = y - self->panrect.top;
	self->visible = FALSE;
	self->panrectchanged = TRUE;
	self->mousestate=scroll_DRAGGING;
	(self)->RetractCursor( self->cursors[0]);
	((self)->GetIM())->SetWindowCursor( self->cursors[2]);
	(self)->WantUpdate( self);
	return;
    }
    
    if (action==view_LeftDown) {
	x -= self->panrect.left+2+self->seenrect.left;
	y -= self->panrect.top+2+self->seenrect.top;
	/* x and y are now coordinates in the seen rectangle */

	if (x>=0 && y>=0 && x<self->seenrect.width && y<self->seenrect.height) {
	    /* turn panner coords to pixel coords */
	    self->isatx = x * self->childrect.width / (self->seenrect.width);
	    self->isaty = y * self->childrect.height / (self->seenrect.height);
	    self->rockx = self->panrect.left+2;
	    self->rocky = self->panrect.top+2;

	    self->mousestate=scroll_THUMBING;
	    ((self)->GetIM())->SetWindowCursor( self->cursors[1]);
	}
	else
	    self->mousestate=scroll_NOTHING;
	return;
    }

    self->mousestate=scroll_NOTHING;
}

class view *panner::Hit(enum view_MouseAction  action, long  x , long  y , long  num_clicks)
{
    switch (this->mousestate) {
	case scroll_DRAGGING:
	    HandleDragging(this, action, x, y, num_clicks);
	    break;
	case scroll_THUMBING:
	    HandleThumbing(this, action, x, y, num_clicks);
	    break;
	case scroll_NOTHING:
	    if (action==view_LeftUp || action==view_RightUp || action==view_LeftMovement || action==view_RightMovement) return (class view *)this;
	    if (x >= this->panrect.left 
		&& x < this->panrect.left+this->panrect.width
		&& y >= this->panrect.top 
		&& y < this->panrect.top+this->panrect.height) {
		HandleDownHit(this, action, x, y, num_clicks);
		return (class view *)this;
	    }
	    else if (this->child) 
		return (this->child)->Hit( action, (this->child)->EnclosedXToLocalX( x), (this->child)->EnclosedYToLocalY( y), num_clicks);
	    break;
	default: 
	    break;
    }

    if (action==view_RightUp || action==view_LeftUp) {
	switch (this->mousestate) {
	    case scroll_THUMBING:
		((this)->GetIM())->SetWindowCursor( NULL);
		(this)->RetractCursor( this->cursors[3]);
		(this)->PostCursor( &(this->gseenrect), this->cursors[3]);
		break;
	    case scroll_DRAGGING:
		((this)->GetIM())->SetWindowCursor( NULL);
		break;
	    default:
		break;
	}
	this->mousestate=scroll_NOTHING;
    }

    return (class view *)this;
}
