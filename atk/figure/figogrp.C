/* figogrp.c - fig element object: group */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/
#include <andrewos.h>
ATK_IMPL("figogrp.H")
#include <figogrp.H>

#include <figview.H>
#include <figure.H>
#include <figobj.H>
#include <figattr.H>

#include <rect.h>

#include <message.H>
#include <proctable.H>
#include <bind.H>


ATKdefineRegistry(figogrp, figobj, figogrp::InitializeClass);
static void MoveHandleNocon(class figogrp  *self, long  x , long  y , long  ptref);
static void MoveHandleCon(class figogrp  *self, long  x , long  y , long  ptref);
static boolean ReconfigureChild(class figobj  *o, long  ref, class figure  *v, class figogrp  *self);


figogrp::figogrp()
{
	ATKinit;

    this->root = figure_NULLREF;
    this->bboxdirty = TRUE;
    (this)->IsGroup() = TRUE;

    (this)->SetNumHandles( 8);

    this->doconstraints=FALSE;

    THROWONFAILURE( TRUE);
}

figogrp::~figogrp()
{
	ATKinit;

    return;
}
/*  boolean callfun(struct figobj *o, long ref, struct figure *self, rock) */

boolean figogrp::InitializeClass()
{
    return TRUE;
}

const char *figogrp::ToolName(class figtoolview  *v, long  rock)
{
    return "<group>";
}

enum figobj_Status figogrp::Build(class figview  *v, enum view::MouseAction action, long  x , long  y /* in fig coords */, long  clicks)   
{
    return figobj_Failed;
}

void figogrp::Draw(class figview  *v) 
{
}

void figogrp::Sketch(class figview  *v) 
{
    long x, y, w, h;
    struct rectangle *rec = &this->handlebox;

    x = (v)->ToPixX( rec->left);
    y = (v)->ToPixY( rec->top);
    w = (v)->ToPixW( rec->width);
    h = (v)->ToPixH( rec->height);
    if (w<0) {
	w = (-w);
	x -= w;
    }
    if (h<0) {
	h = (-h);
	y -= h;
    }
    (v)->SetTransferMode( graphic::INVERT);
    (v)->DrawRectSize( x, y, w, h);
}

void figogrp::Select(class figview  *v)
{
    long ix;
    long x, y;
    /*struct figure *fig = figogrp_GetAncestorFig(self);*/

    (this)->GetBounds( v); /* make sure handles are up-to-date */

    if ((this)->GetHandles() && (this)->GetNumHandles()) {
	(v)->SetTransferMode( graphic::INVERT);

	for (ix=0; ix<(this)->GetNumHandles(); ix++) {
	    x = (v)->ToPixX( point_X(&((this)->GetHandles()[ix])));
	    y = (v)->ToPixY( point_Y(&((this)->GetHandles()[ix])));
	    (v)->DrawRectSize( x-figview_SpotRad, y-figview_SpotRad, 2*figview_SpotRad, 2*figview_SpotRad);
	}
    }

    /*if (self->doconstraints)
	for (ix=self->root; ix!=figure_NULLREF; ix=fig->objs[ix].next) 
	    if (v->objs[ix].selected) {
		struct figobj *o = figure_FindObjectByRef(fig, ix);
		figobj_DrawAttachments(o, v);
	    }*/
}

/* basic procedure to move a handle -- used by figogrp__MoveHandle(), figogrp__Reshape() */
/* use when self->doconstraints is FALSE */
static void MoveHandleNocon(class figogrp  *self, long  x , long  y , long  ptref)
{
    long dx, dy;

    (self)->GetBounds( NULL); /* make sure handles are right */
    dx = x - (self)->GetHandleX( ptref);
    dy = y - (self)->GetHandleY( ptref);

    (self)->Reposition( dx, dy);
}

/* basic procedure to move a handle -- used by figogrp__MoveHandle(), figogrp__Reshape() */
/* use when self->doconstraints is TRUE */
static void MoveHandleCon(class figogrp  *self, long  x , long  y , long  ptref)
{
    long ix;
    struct rectangle *R = &(self)->handlebox;

    switch (ptref) {
	case 0:
	    R->width = x - R->left;
	    break;
	case 1:
	    R->width = x - R->left;
	    ix = R->top;
	    R->top = y;
	    R->height += (ix-y);
	    break;
	case 2:
	    ix = R->top;
	    R->top = y;
	    R->height += (ix-y);
	    break;
	case 3:
	    ix = R->top;
	    R->top = y;
	    R->height += (ix-y);
	    ix = R->left;
	    R->left = x;
	    R->width += (ix-x);
	    break;
	case 4:
	    ix = R->left;
	    R->left = x;
	    R->width += (ix-x);
	    break;
	case 5:
	    R->height = y - R->top;
	    ix = R->left;
	    R->left = x;
	    R->width += (ix-x);
	    break;
	case 6:
	    R->height = y - R->top;
	    break;
	case 7:
	    R->width = x - R->left;
	    R->height = y - R->top;
	    break;
    }
}

boolean figogrp::Reshape(enum view::MouseAction  action, class figview  *v, long  x , long  y , boolean  handle, long  ptref)
{
    if (!handle)
	return FALSE;

    if (!this->doconstraints) {
	switch (action) {
	    case view::LeftDown:
	    case view::RightDown:
		if ((this)->GetReadOnly())
		    return FALSE;
		(this)->Sketch( v);
		break;
	    case view::LeftMovement:
	    case view::RightMovement:
		(this)->Sketch( v);
		MoveHandleNocon(this, x, y, ptref);
		(this)->Sketch( v);
		break;
	    case view::LeftUp:
	    case view::RightUp:
		(this)->Sketch( v);
		MoveHandleNocon(this, x, y, ptref);
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
		(this)->Sketch( v);
		break;
	    case view::LeftMovement:
	    case view::RightMovement:
		(this)->Sketch( v);
		MoveHandleCon(this, x, y, ptref);
		(this)->Sketch( v);
		break;
	    case view::LeftUp:
	    case view::RightUp:
		(this)->Sketch( v);
		MoveHandleCon(this, x, y, ptref);
		(this)->Reconfigure();
		(this)->RecomputeBounds();
		(this)->SetModified();
		break;
	    default:
		break;
	}
	return TRUE;
    }
    return FALSE;
}

void figogrp::MoveHandle(long  x , long  y , long  ptref)
{
    if ((this)->GetReadOnly())
	return;

    if (this->doconstraints) {
	MoveHandleCon(this, x, y, ptref);
	(this)->Reconfigure();
    }
    else 
	MoveHandleNocon(this, x, y, ptref);
    (this)->RecomputeBounds();
    (this)->SetModified();
}

void figogrp::Reposition(long  xd , long  yd)
{
    if (!this->doconstraints) {
	int ix;
	class figure *fig = (this)->GetAncestorFig();
	if (!fig) return;

	for (ix=this->root; ix!=figure_NULLREF; ix=fig->objs[ix].next) {
	    (fig->objs[ix].o)->Reposition( xd, yd);
	}
    } 
    else {
	struct rectangle *R = (&(this)->handlebox);
	R->left+=xd;
	R->top+=yd;
	(this)->Reconfigure();
	(this)->RecomputeBounds();
	(this)->SetModified();
    }
}

static boolean ReconfigureChild(class figobj  *o, long  ref, class figure  *v, class figogrp  *self)
{
    struct rectangle *R=(&(self)->handlebox);
    int i=0;
    for (i=0; i<(o)->GetNumHandles(); i++) {
	if ((o)->IsAttachmentActive( i)) {
	    long posx = (o)->GetAttachmentPosX( i);
	    long posy = (o)->GetAttachmentPosY( i);
	    long offx = (o)->GetAttachmentOffX( i);
	    long offy = (o)->GetAttachmentOffY( i);

	    double resx = ((double)posx*R->width) / (double)figogrp_Range;
	    double resy = ((double)posy*R->height) / (double)figogrp_Range;

	    (o)->MoveHandle( R->left+(long)resx+offx, R->top+(long)resy+offy, i);
	    (o)->SetModified();
	}
    }
    return FALSE;
}

				 
void figogrp::Reconfigure()
{
    class figure *fig = (this)->GetAncestorFig();
    if (!this->doconstraints || !fig) return;
    (fig)->EnumerateObjectGroup( (fig)->FindRefByObject( this), NULL, TRUE, (figure_eofptr)ReconfigureChild, (long)this);
}


void figogrp::RecomputeBounds()
{
    this->bboxdirty = TRUE;
}

void figogrp::SetConstraintsActive(boolean  newval)
{
    class figure *fig = (this)->GetAncestorFig();
    if (!fig) return;

    if (newval && !this->doconstraints) {
	/* turn on */
	(this)->GetBounds( NULL); /* make sure handlebox is set to the bounding box */
	this->doconstraints = TRUE;
	(this)->RecomputeBounds();
	(this)->SetModified();
	(fig)->SetModified();
	(fig)->NotifyObservers( figure_DATACHANGED);
    }
    else if (!newval && this->doconstraints) {
	/* turn off */
	this->doconstraints = FALSE;
	(this)->SetModified();
	(this)->RecomputeBounds();
	/* figogrp_GetBounds(self, NULL); */ /* ### probably not necessary */
	(fig)->SetModified();
	(fig)->NotifyObservers( figure_DATACHANGED);
    }
}

struct rectangle *figogrp::GetBounds(class figview  *vv)
{
    struct rectangle *R = &((class figobj *)this)->bounds;
    struct rectangle *HR = &this->handlebox;
    class figure *fig;
    int ix;

    if (!this->bboxdirty)
	return R; /* cached bbox */

    this->bboxdirty = FALSE;

    rectangle_EmptyRect(R);
    fig = (this)->GetAncestorFig();
    if (!fig) return NULL; /* a group has no contents unless it's within a fig */

    for (ix=this->root; ix!=figure_NULLREF; ix=fig->objs[ix].next) {
	rectangle_UnionRect(R, R, (fig->objs[ix].o)->GetBounds( vv));
    }

    if (!this->doconstraints) {
	*HR = *R;
    }
    else {
	struct rectangle tmp;
	tmp = *HR;
	if (tmp.width<0) {
	    tmp.width = (-tmp.width);
	    tmp.left -= tmp.width;
	}
	if (tmp.height<0) {
	    tmp.height = (-tmp.height);
	    tmp.top -= tmp.height;
	}

	rectangle_UnionRect(R, R, &tmp);
    };
    /*if (vv) printf("doconstr = %s; handlebox = (%d..%d) (%d..%d)\n", (self->doconstraints ? "true" : "false"), figview_ToPixX(vv, HR->left), figview_ToPixX(vv, HR->left+HR->width), figview_ToPixX(vv, HR->top), figview_ToPixX(vv, HR->top+HR->height));*/
    (this)->SetHandle( 0, HR->left+HR->width, HR->top+HR->height/2);
    (this)->SetHandle( 1, HR->left+HR->width, HR->top);
    (this)->SetHandle( 2, HR->left+HR->width/2, HR->top);
    (this)->SetHandle( 3, HR->left, HR->top);
    (this)->SetHandle( 4, HR->left, HR->top+HR->height/2);
    (this)->SetHandle( 5, HR->left, HR->top+HR->height);
    (this)->SetHandle( 6, HR->left+HR->width/2, HR->top+HR->height);
    (this)->SetHandle( 7, HR->left+HR->width, HR->top+HR->height);

    (this)->ComputeSelectedBounds(); /* this winds up calling figogrp_GetBounds() again, but self->bboxdirty will be FALSE, so no loop will result. */
    return R;
}

struct rectangle *figogrp::GetSelectedBounds(class figview  *vv)
{
    (this)->GetBounds( vv); /* make sure bbox is cleaned up */
    return &(((class figobj *)this)->selbounds);
}

void figogrp::StabilizeAttachments(boolean  keepproport)
{
    if (((class figobj *)this)->anyattachmentsactive 
	 && ((class figobj *)this)->vas) {
	(this)->RecomputeBounds();
	(this)->GetBounds( NULL);
	(this)->figobj::StabilizeAttachments( keepproport);
    }
}

static enum figobj_HandleType handletypes[8]={
    figobj_MiddleRight,
    figobj_URCorner,
    figobj_MiddleTop,
    figobj_ULCorner,
    figobj_MiddleLeft,
    figobj_LLCorner,
    figobj_MiddleBottom,
    figobj_LRCorner
};

enum figobj_HandleType figogrp::GetHandleType(long  num)
{
    if (num>=0 && num<=7) return handletypes[num];
    else return figobj_None;
}

static long canonical_con[] = {
    2, 4, 6, 0, figobj_NULLREF
};
static long canonical_nocon[] = {
    3, figobj_NULLREF
};

long *figogrp::GetCanonicalHandles()
{
    if (this->doconstraints)
	return canonical_con;
    else
	return canonical_nocon;
}

void figogrp::SetParent(long  pref, class figure  *ancfig)
{
    (this)->figobj::SetParent( pref, ancfig);
}

void figogrp::InheritVAttributes(class figattr  *attr, unsigned long  mask)
{
    int ix;
    class figure *fig;
    unsigned long tmpmask, cmask;
    class figattr *selfattr = (this)->GetVAttributes();
    class figattr *selfiattr = (this)->GetIVAttributes();

    (selfiattr)->CopyData( attr, mask);
    cmask = mask & (~(selfattr->active)) & figattr_MaskAll;
    /* cmask is now the list of things that have changed in self's working attributes, and thus the list of things that have changed in all children's inherited attributes. */
    if (cmask)
	(this)->SetModified();

    fig = (this)->GetAncestorFig();
    if (!fig) return;

    if (cmask) {
	/* gross hack approaching -- see comment in UpdateVAttributes() */
	tmpmask = selfattr->active;

	(selfattr)->CopyData( selfiattr, mask & (~tmpmask));

	for (ix=this->root; ix!=figure_NULLREF; ix=fig->objs[ix].next) {
	    (fig->objs[ix].o)->InheritVAttributes( selfattr, cmask);
	}

	/* put attr back the way we found it. */
	selfattr->active = tmpmask;
    }
}

unsigned long figogrp::UpdateVAttributes(class figattr  *attr, unsigned long  mask)
{
    int ix;
    class figure *fig;
    unsigned long tmpmask;
    class figattr *selfattr = (this)->GetVAttributes();
    class figattr *selfiattr = (this)->GetIVAttributes();

    if ((this)->GetReadOnly())
	return 0;

    (selfattr)->CopyData( attr, mask);
    (this)->SetModified();

    fig = (this)->GetAncestorFig();
    if (!fig) return 0;

    /* we now need to create a new environment attr, which has all parts active. We do this in selfattr, replacing all inactive parts with parts from iattr. This is, of course, horrible -- but very easy. (However, don't even *think* of calling this reentrantly (with the same self).) */
    tmpmask = selfattr->active;
    
    (selfattr)->CopyData( selfiattr, ~tmpmask);

    for (ix=this->root; ix!=figure_NULLREF; ix=fig->objs[ix].next) {
	(fig->objs[ix].o)->InheritVAttributes( selfattr, mask);
    }

    /* put attr back the way we found it. */
    selfattr->active = tmpmask;
    return selfattr->active;
}

void figogrp::WriteBody(FILE  *fp)
{
    fprintf(fp, "$ %d %ld %ld %ld %ld\n", this->doconstraints, this->handlebox.left, this->handlebox.top, this->handlebox.width, this->handlebox.height);
}

long figogrp::ReadBody(FILE  *fp, boolean  recompute)
{
#define LINELENGTH (250)
    char buf[LINELENGTH+1];
    int ix;
    long val1, val2, val3, val4, val5;

    if (fgets(buf, LINELENGTH, fp) == NULL)
	return dataobject::PREMATUREEOF;
    ix = sscanf(buf, "$ %ld %ld %ld %ld %ld", &val1, &val2, &val3, &val4, &val5);

    if (ix != 5) return dataobject::BADFORMAT;

    if (val2 == (-1) && val3 == (-1) && val4 == (-1) && val5 == (-1)) {
	/* the group was written out as the root of a figure_WritePartial() operation. Thus, we should ignore these values, rather than overwrite the values of this (the focus group.) */
    }
    else {
	this->doconstraints = (val1) ? TRUE : FALSE;
	rectangle_SetRectSize(&this->handlebox, val2, val3, val4, val5);
    }

    if (recompute) {
	(this)->RecomputeBounds();
	(this)->SetModified();
    }

    return dataobject::NOREADERROR;
}
