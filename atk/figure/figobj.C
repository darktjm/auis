/* figobj.c - fig element object */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/
#include <andrewos.h>
ATK_IMPL("figobj.H")
#include <util.h>
#include <view.H>
#include <figobj.H>



#include <figview.H>
#include <figure.H>
#include <figogrp.H>
#include <figtoolview.H>
#include <figattr.H>
#include <message.H>
#include <fontsel.H>

#include <point.h>


ATKdefineRegistry(figobj, dataobject, figobj::InitializeClass);

static void UnionRectanglePt(struct rectangle  *rec, long  x , long  y);


boolean figobj::InitializeClass()
{
    return TRUE;
}

figobj::figobj()
{
	ATKinit;

    this->numpts = 0;
    this->pts = NULL;
    this->pt_size = 0;
    this->parent = NULL;
    this->parentref = figobj_NULLREF;
    this->figo = NULL;
    this->isgroup = FALSE;
    this->isinset = FALSE;
    this->attrused = figattr_MaskAll;
    this->attr = new figattr;
    this->iattr = new figattr;
    this->vas = NULL;
    this->anyattachmentsactive = FALSE;
    this->name = NULL;
    rectangle_EmptyRect(&(this->bounds));

    THROWONFAILURE( TRUE);
}

figobj::~figobj()
{
	ATKinit;

    if (this->pts)
	free(this->pts);
    if(this->vas)
	free(this->vas);

    (this->attr)->Destroy();
    (this->iattr)->Destroy();
}

const char *figobj::ToolName(class figtoolview  *v, long  rock)
{
    return "<no-name>";
}

/* called when the tool is reclicked in the toolset */
void figobj::ToolModify(class figtoolview  *v, long  rock, boolean  firsttime) 
{
}

class figobj *figobj::Instantiate(class figtoolview  *v, long  rock) 
{
    class figobj *res = (class figobj *)ATK::NewObject((this)->GetTypeName());
    return res;
}

void figobj::SetName(char *name)
{
    if (this->name)
	free(this->name);
    this->name = NULL;

    if (name) {
	this->name = strdup(name);
    }
}

void figobj::SetNumHandles(long  num)
{
    int ix;
    if (num > this->pt_size) {
	if (this->pt_size == 0) {
	    this->pt_size = num;
	    this->pts = (struct point *)malloc(this->pt_size * sizeof(struct point));
	    this->vas = (struct figobj_Attachment *)malloc(this->pt_size * sizeof(struct figobj_Attachment));
	    for (ix=0; ix<this->pt_size; ix++) 
		this->vas[ix].on = FALSE;
	}
	else {
	    while (num > this->pt_size)
		this->pt_size *= 2;
	    this->pts = (struct point *)realloc(this->pts, this->pt_size * sizeof(struct point));
	    this->vas = (struct figobj_Attachment *)realloc(this->vas, this->pt_size * sizeof(struct figobj_Attachment));
	    for (ix=this->numpts; ix<this->pt_size; ix++) 
		this->vas[ix].on = FALSE;
	}
    }
    this->numpts = num;
}

enum figobj_HandleType figobj::GetHandleType(long  num)
{
    return figobj_None;
}

static long canonical[] = {
    figobj_NULLREF
};

long *figobj::GetCanonicalHandles()
{
    return canonical;
}

struct rectangle *figobj::GetBounds(class figview  *vv)
{
    return &(this->bounds);
}

struct rectangle *figobj::GetSelectedBounds(class figview  *vv)
{
    return &(this->selbounds);
}

void figobj::UpdateParentBounds()
{
    class figogrp *tmp;

    /* it would be somewhat more logical to do this recursively. Or even make it an override on figobj_SetModified(). For now, this is good enough and fast. */
    for (tmp=this->parent; tmp; tmp=(tmp)->GetParent()) {
	(tmp)->SetChildBoundMod();
	(tmp)->SetModified();
    }
    if (this->figo) {
	(this->figo)->SetChildBoundMod();
	/* figure_SetModified(self->figo); lovely idea, but no good. The figotext objects update their bounds unpredictably. */
    }
}

/* Should only be called by figobj_ methods. 
  set bounding box in fig coordinates */
void figobj::RecomputeBounds()
{
    (this)->UpdateParentBounds();
    rectangle_SetRectSize(&(this->bounds), this->x-50, this->y-50, 101, 101);
}

void figobj::Draw(class figview  *v) 
{
    (v)->SetTransferMode( graphic_COPY);
}

void figobj::Sketch(class figview  *v) 
{
    long x, y, w, h;
    struct rectangle *rec = (this)->GetBounds( v);

    x = (v)->ToPixX( rec->left);
    y = (v)->ToPixY( rec->top);
    w = (v)->ToPixW( rec->width);
    h = (v)->ToPixH( rec->height);
    (v)->SetTransferMode( graphic_INVERT);
    (v)->DrawRectSize( x, y, w, h);
}

/* draw handles or whatever. defaults to drawing all handles. */
void figobj::Select(class figview  *v)
{
    long ix;
    long x, y;
    class graphic *BlackPattern;

    if (this->pts && this->numpts) {
	(v)->SetTransferMode( graphic_INVERT);
	BlackPattern = (v)->BlackPattern();

	for (ix=0; ix<this->numpts; ix++) {
	    x = (v)->ToPixX( point_X(&this->pts[ix]));
	    y = (v)->ToPixY( point_Y(&this->pts[ix]));
	    (v)->FillRectSize( x-figview_SpotRad, y-figview_SpotRad, 1+2*figview_SpotRad, 1+2*figview_SpotRad, BlackPattern);
	}
    }
}

static void UnionRectanglePt(struct rectangle  *rec, long  x , long  y)
{
    if (rectangle_IsEmptyRect(rec)) {
	rectangle_SetRectSize(rec, x, y, 1, 1);
	return;
    }

    if (x < rec->left) {
	rec->width += rec->left-x;
	rec->left = x;
    }
    else if (x >= rec->left+rec->width)
	rec->width = (x + 1 - rec->left);

    if (y < rec->top) {
	rec->height += rec->top-y;
	rec->top = y;
    }
    else if (y >= rec->top+rec->height)
	rec->height = (y + 1 - rec->top);
}

/* this assumes that self->bounds has already been set to the basic bounding box value, and the object's handles have been properly defined. */
void figobj::ComputeSelectedBounds()
{
    long ix;
    long x, y;
    this->selbounds = this->bounds;

    /* self->anyattachmentsactive is only a hint; it may be TRUE even if no attachments are active. */
    if (this->anyattachmentsactive && this->vas && this->numpts) {
	for (ix=0; ix<this->numpts; ix++) 
	    if (this->vas[ix].on) {
		x = point_X(&this->pts[ix]) - this->vas[ix].offx;
		y = point_Y(&this->pts[ix]) - this->vas[ix].offy;
		UnionRectanglePt(&(this->selbounds), x, y);
	    }
    }
}

void figobj::ClearAttachments()
{
    int ix;

    if (this->vas)
	for (ix=0; ix<(this)->GetNumHandles(); ix++) {
	    (this)->SetAttachmentActive( ix, FALSE);
	}
    this->anyattachmentsactive = FALSE;
}

void figobj::StabilizeAttachments(boolean  keepproport)
{
    boolean didany = FALSE;
    class figogrp *vg = (this)->GetParent();
    struct rectangle *R = (&(vg)->handlebox);
    long x, y;
    int ix;

    if (this->anyattachmentsactive && this->vas) {
	for (ix=0; ix<(this)->GetNumHandles(); ix++) 
	    if ((this)->IsAttachmentActive( ix)) {
		didany = TRUE;

		if (!keepproport) {
		    x = (this)->GetHandleX( ix) - (this)->GetAttachmentOffX( ix);
		    y = (this)->GetHandleY( ix) - (this)->GetAttachmentOffY( ix);
		    (this)->SetAttachmentPosX( ix, ((x-R->left)*figogrp_Range)/R->width);
		    (this)->SetAttachmentPosY( ix, ((y-R->top)*figogrp_Range)/R->height);
		}
		else {
		    x = (this)->GetAttachmentPosX( ix)*R->width/figogrp_Range + R->left;
		    y = (this)->GetAttachmentPosY( ix)*R->height/figogrp_Range + R->top;
		    (this)->SetAttachmentOffX( ix, (this)->GetHandleX( ix) - x);
		    (this)->SetAttachmentOffY( ix, (this)->GetHandleY( ix) - y);
		} 
	    }

	if (didany) {
	    (this)->ComputeSelectedBounds();
	    (this)->SetModified(); 
	}
    }
}

static short offsets[9][2] = {
    {0, 0},   /*figobj_None*/
    {1, -1},  /*figobj_ULCorner*/
    {-1, -1}, /*figobj_LLCorner*/
    {-1, 1},  /*figobj_LRCorner*/
    {1, 1},   /*figobj_URCorner*/
    {0, -1},  /*figobj_MiddleLeft*/
    {0, 1},   /*figobj_MiddleRight*/
    {1, 0},   /*figobj_MiddleTop*/
    {-1, 0}   /*figobj_MiddleBottom*/
};

void figobj::DrawAttachments(class figview  *v) 
{
    long ix;
    long x, y, dx, dy;
    short offx, offy; /* offsets to make things more visible. */
    int typ;

    if (this->anyattachmentsactive && this->vas && this->numpts) {
	(v)->SetTransferMode( graphic_INVERT);

	for (ix=0; ix<this->numpts; ix++) 
	    if (this->vas[ix].on) {
		typ = (int)((this)->GetHandleType( ix)) - (int)(figobj_None);
		if (typ >= 9) {
		    offx = 0;
		    offy = 0;
		}
		else {
		    offx = offsets[typ][0];
		    offy = offsets[typ][1];
		}
		x = offx + (v)->ToPixX( point_X(&this->pts[ix]));
		y = offy + (v)->ToPixY( point_Y(&this->pts[ix]));
		dx = -(v)->ToPixW( this->vas[ix].offx);
		dy = -(v)->ToPixH( this->vas[ix].offy);

		(v)->DrawRectSize( x+dx-(figview_AnchRad), y+dy-(figview_AnchRad), 2*(figview_AnchRad), 2*(figview_AnchRad));

		(v)->MoveTo( x+dx, y+dy);
		(v)->DrawLineTo( x, y);
	    }
    }
}

/* zero clicks means user hit ctrl-G or switched modes. Build may draw further bits on the screen. */
enum figobj_Status figobj::Build(class figview  *v, enum view_MouseAction action, long  x , long  y /* in fig coords */, long  clicks)   
{
    if (clicks==0) {
	message::DisplayString(v, 10, "Object aborted.");
	return figobj_Failed;
    }

    this->x = x;
    this->y = y;
    (this)->RecomputeBounds();
    (this)->SetModified();
    return figobj_Done;
}

enum figobj_HitVal figobj::HitMe(long  x , long  y, long  delta, long  *ptref) 
{
    return (this)->BasicHitMe( x, y, delta, ptref);
}

/* check if point is in bounds rectangle, with a little leeway */
/* the idea is to return figobj_HitHandle if it hits a handle, figobj_HitInside if it's in the bounding rectangle, figobj_Miss otherwise. */
enum figobj_HitVal figobj::BasicHitMe(long  x , long  y, long  delta, long  *ptref) 
{
    struct point pt;
    struct rectangle tmp, *bbox;
    long xd, yd;
    long ix, tmpt;

    point_SetPt(&pt, x, y);
    bbox = (this)->GetBounds( NULL);

    if (!this->pts || !this->numpts) {
	if (rectangle_IsPtInRect(&pt, bbox)) {
	    return figobj_HitInside;
	}
	else
	    return figobj_Miss;
    }

    tmp = *bbox;
    rectangle_InsetRect(&tmp, -delta, -delta);
    if (!rectangle_IsPtInRect(&pt, &tmp)) {
	return figobj_Miss;
    }

    /* see if handles match */
    tmpt = figobj_NULLREF;
    for (ix=(this)->GetNumHandles()-1; ix>=0; ix--) {
	xd = x - point_X(&((this)->GetHandles()[ix]));
	yd = y - point_Y(&((this)->GetHandles()[ix]));
	if (xd<=delta && xd>=(-delta) && yd<=delta && yd>=(-delta)) {
	    tmpt = ix;
	    break;
	}
    }

    if (tmpt != figobj_NULLREF) {
	if (ptref)
	    *ptref = tmpt;
	return figobj_HitHandle;
    }

    return figobj_HitInside; 
}

void figobj::MoveHandle(long  x , long  y , long  ptref)
{
    
}

boolean figobj::AddParts(enum view_MouseAction  action, class figview  *v, long  x , long  y , boolean  handle, long  ptref)
{
    return FALSE;
}

boolean figobj::DeleteParts(enum view_MouseAction  action, class figview  *v, long  x , long  y , boolean  handle, long  ptref)
{
    return FALSE;
}

boolean figobj::Reshape(enum view_MouseAction  action, class figview  *v, long  x , long  y , boolean  handle, long  ptref)
{
    if (!handle)
	return FALSE;

    switch (action) {
	case view_LeftDown:
	case view_RightDown:
	    if ((this)->GetReadOnly())
		return FALSE;
	    /*figobj_Sketch(self, v);*/
	    break;
	case view_LeftMovement:
	case view_RightMovement:
	    (this)->Sketch( v);
	    (this)->MoveHandle( x, y, ptref);
	    (this)->Sketch( v);
	    break;
	case view_LeftUp:
	case view_RightUp:
	    (this)->Sketch( v);
	    (this)->MoveHandle( x, y, ptref);
	    (this)->SetModified();
	    break;
	default:
	    break;
    }
    return TRUE;
}

void figobj::Reposition(long  xd , long  yd)
{
    if ((this)->GetReadOnly())
	return;

    this->x += xd;
    this->y += yd;
    (this)->RecomputeBounds();
    (this)->SetModified();
}

void figobj::SetParent(long  pref, class figure  *ancfig)
{
    class figattr *tmp;

    this->figo = ancfig;
    this->parentref = pref;
    if (ancfig==NULL) {
	this->parent = NULL;
    }
    else {
	this->parent = (class figogrp *)(ancfig)->FindObjectByRef( pref);
	if (this->parent==NULL) {
	    (this->iattr)->SetShade( 0);
	    (this->iattr)->SetTextPos( figattr_PosCenter);
	    (this->iattr)->SetLineWidth( 0);
	    (this->iattr)->SetRRectCorner( 10);
	    (this->iattr)->SetColor( "black");
	    (this->iattr)->SetFontFamily( fontsel_default_Family);
	    (this->iattr)->SetFontSize( fontsel_default_Size);
	    (this->iattr)->SetFontStyle( fontsel_default_Style);
	    (this->iattr)->SetArrowSize( 8);
	    (this->iattr)->SetArrowPos(figattr_ArrowNone);
	}
	else {
	    (this->iattr)->CopyData( (this->parent)->GetIVAttributes(), figattr_MaskAll);
	    tmp = (this->parent)->GetVAttributes();
	    (this->iattr)->CopyData( tmp, tmp->active);
	}
    }
    (this)->SetModified();
}

/* notify self that an ancestor group's attributes have changed. mask indicates which ones; all mask-active parts will be active. */
void figobj::InheritVAttributes(class figattr  *attr, unsigned long  mask)
{
    (this->iattr)->CopyData( attr, mask);
    if (mask & (~(this->attr->active)) & (this)->AttributesUsed())
	(this)->SetModified();
}

/* update self's personal attributes. mask indicates which parts of attr to use; ignore all others. Returns a mask of which attributes were actually set. */
unsigned long figobj::UpdateVAttributes(class figattr  *attr, unsigned long  mask)
{
    if ((this)->GetReadOnly())
	return 0;
    (this->attr)->CopyData( attr, mask);
    if (mask & (this)->AttributesUsed()) {
	(this)->SetModified();
	if (this->figo) {
	    (this->figo)->SetModified();
	}
    }
    return mask;
}

#define LINELENGTH (250)
static char buf[LINELENGTH+1];

void figobj::WriteBody(FILE  *fp)
{
    fprintf(fp, "$ %ld %ld\n", (this)->PosX(), (this)->PosY());
}

long figobj::ReadBody(FILE  *fp, boolean  recompute)
{
    int	ix; 
    long x, y;

    if (fgets(buf, LINELENGTH, fp) == NULL)
	return dataobject_PREMATUREEOF;
    ix = sscanf(buf, "$ %ld %ld", &x, &y);
    if (ix!=2) return dataobject_BADFORMAT;

    this->x = x;
    this->y = y;

    if (recompute) {
	(this)->RecomputeBounds();
	(this)->SetModified();
    }

    return dataobject_NOREADERROR;
}

long figobj::Write(FILE  *fp, long  writeid, int  level)
{
    int ix;
    if ((this)->GetWriteID() != writeid) {
	(this)->SetWriteID( writeid);

	fprintf(fp, "\\begindata{%s,%ld}\n", (this)->GetTypeName(), (this)->GetID());
	((this)->GetVAttributes())->Write( fp, writeid, level+1);
	(this)->WriteBody( fp);
	if (this->numpts && this->vas) {
	    for (ix=0; ix<this->numpts; ix++) 
		if (this->vas[ix].on) {
		    fprintf(fp, "$# %d %ld %ld %ld %ld\n", ix, this->vas[ix].rposx, this->vas[ix].rposy, this->vas[ix].offx, this->vas[ix].offy);
		}
	}
	fprintf(fp, "$endatt\n");
	if (this->GetName()) {
	    fprintf(fp, "$name:%s\n", this->GetName());
	}
	fprintf(fp, "\\enddata{%s,%ld}\n", (this)->GetTypeName(), (this)->GetID());
    }

    return (this)->GetID();
}

long figobj::Read(FILE  *fp, long  id)
{
    long unid, tid, ix;
    long val1, val2, val3, val4, val5;
    char *cx, *cx2;
    char namebuf[100];

    if (id==0) 
	unid = (this)->UniqueID();
    else
	unid = id;
    (this)->SetID( unid); 

    ix = ((this)->GetVAttributes())->Read( fp, 0);
    if (ix!=dataobject_NOREADERROR) return ix;
    ix = (this)->ReadBody( fp, TRUE);
    if (ix!=dataobject_NOREADERROR) return ix;

    if (this->numpts && this->vas) {
	for (ix=0; ix<this->numpts; ix++) 
	    this->vas[ix].on = FALSE;
    }
    
    if (fgets(buf, LINELENGTH, fp) == NULL)
	return dataobject_PREMATUREEOF;

    while (strncmp(buf, "$endatt", 7)) {
	ix = sscanf(buf, "$# %ld %ld %ld %ld %ld", &val1, &val2, &val3, &val4, &val5);
	if (ix!=5) return dataobject_BADFORMAT;
	(this)->SetAttachmentActive( val1, TRUE);
	this->vas[val1].rposx = val2;
	this->vas[val1].rposy = val3;
	this->vas[val1].offx = val4;
	this->vas[val1].offy = val5;
	if (fgets(buf, LINELENGTH, fp) == NULL)
	    return dataobject_PREMATUREEOF;
    }

    if (fgets(buf, LINELENGTH, fp) == NULL)
	return dataobject_PREMATUREEOF;
    if (!strncmp(buf, "$name:", 6)) {
	cx = buf+6;
	cx2 = strchr(cx, '\n');
	if (cx2)
	    (*cx2) = '\0';
	this->SetName(cx);
	if (fgets(buf, LINELENGTH, fp) == NULL)
	    return dataobject_PREMATUREEOF;
    }

    ix = sscanf(buf, "\\enddata{%[^,],%ld}", namebuf, &tid);
    if (ix!=2) return dataobject_MISSINGENDDATAMARKER;

    if (tid!=id || strcmp(namebuf, (this)->GetTypeName()))
	return dataobject_MISSINGENDDATAMARKER;

    return dataobject_NOREADERROR;     
}

void figobj::PrintObject(class figview  *v, FILE  *file, const char  *prefix, boolean newstyle)
{
}

boolean figobj::ORecSearch(struct SearchPattern *pat)
{
    return FALSE;
}

boolean figobj::ORecSrchResume(struct SearchPattern *pat)
{
    return FALSE;
}

boolean figobj::ORecSrchReplace(class dataobject *text, long pos, long len)
{
    return FALSE;
}

