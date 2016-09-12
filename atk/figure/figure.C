/* figure.c - drawing data object */
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
ATK_IMPL("figure.H")

#include <figobj.H>
#include <figogrp.H>
#include <figure.H>

#ifndef _SEPARATEFIGOBJECTS
#include <figorect.H>
#include <figoell.H>
#include <figorrec.H>
#include <figoplin.H>
#include <figospli.H>
#include <figotext.H>
#include <figoins.H>
#endif

#include <attribs.h>




ATKdefineRegistry(figure, dataobject, figure::InitializeClass);

static void FlattenRefList(class figure  *self, long  ix);
static class figobj *EOT_AllArea(class figure  *self, long  grp, figure_eofptr callfun, long  rock);
static class figobj *EOT_OverlapArea(class figure  *self, long  grp, struct rectangle  *area, figure_eofptr callfun, long  rock);
static class figobj *EOT_IncludeArea(class figure  *self, long  grp, struct rectangle  *area, figure_eofptr callfun, long  rock);
static long FRBPSplot(class figure  *self, long  gref, enum figobj_HitVal  howhit, long  x , long  y, long  delta, long  *ptref);
static void WriteObject(class figure  *self, long  oref, FILE  *fp, long  writeid, int  level);
static long ReadObject(class figure  *self, class figobj  *o , long  oref, FILE  *fp, long  oid);


boolean figure::InitializeClass()
{
#define _SEPARATEFIGOBJECTS
#ifndef _SEPARATEFIGOBJECTS
    figobj_StaticLoadOnlyThisClass();
    figogrp_StaticLoadOnlyThisClass();
    figorect_StaticLoadOnlyThisClass();
    figoell_StaticLoadOnlyThisClass();
    figorrec_StaticLoadOnlyThisClass();
    figoplin_StaticLoadOnlyThisClass();
    figospli_StaticLoadOnlyThisClass();
    figotext_StaticLoadOnlyThisClass();
    figoins_StaticLoadOnlyThisClass();
#endif

    return TRUE;
}

figure::figure()
{
	ATKinit;

    class figogrp *tmp;

    this->ReadOnly = FALSE;
    this->ocounter = 0;

    this->originx = 0;
    this->originy = 0;
    this->printscalex = 1.0;
    this->printscaley = 1.0;
    this->PrintLandscape = FALSE;

    this->objs_size = 8;
    this->objs = (struct figure_oref *)malloc(this->objs_size * sizeof(struct figure_oref));
    FlattenRefList(this, 0);
    this->root = figure_NULLREF;

    /* create a root group and stick it in */
    tmp = new figogrp;
    if (!tmp) THROWONFAILURE( FALSE);
    this->root = (this)->AlwaysInsertObject( tmp, figure_NULLREF, -1);

    THROWONFAILURE( TRUE);
}

figure::~figure()
{
	ATKinit;

    int ix;

    for (ix=0; ix<this->objs_size; ix++)
	if (this->objs[ix].o)
	    (this->objs[ix].o)->Destroy();

    free(this->objs);
}

const char *figure::ViewName()
{
    return "figview";
}

void figure::SetAttributes(struct attributes  *attributes)
{
    while (attributes) {
	if(!strcmp(attributes->key, "readonly")) {
	    this->ReadOnly = attributes->value.integer;
	}
	attributes = attributes->next;
    }
}

static void FlattenRefList(class figure  *self, long  ix)
{
    for (; ix<self->objs_size; ix++) {
	self->objs[ix].o = NULL;
	self->objs[ix].next = figure_NULLREF;
    }
}

long figure::InsertObject(class figobj  *o, long  parent , long  depth)
{
    if ((this)->GetReadOnly())
	return figure_NULLREF;
    return (this)->AlwaysInsertObject( o, parent, depth);
}

long figure::AlwaysInsertObject(class figobj  *o, long  parent /* if figure_NULLREF, put in top-level group, unless there is none. */, long  depth /* 0 = back, 1 = next to back, etc. -1 or a high value means in front. */)
{
    long ix, count, oldroot;
    long *pt;

    for (ix=0; ix<this->objs_size; ix++)
	if (!this->objs[ix].o) break;

    if (ix==this->objs_size) {
	this->objs_size *= 2;
	this->objs = (struct figure_oref *)realloc(this->objs, this->objs_size * sizeof(struct figure_oref));
	FlattenRefList(this, ix);
    }

    if (parent==figure_NULLREF)
	parent = this->root;

    if (parent==figure_NULLREF) {
	/* the very first object */
	this->objs[ix].next = figure_NULLREF;
	(o)->SetParent( figure_NULLREF, this);
    }
    else {
	class figogrp *tmp;
	if (!(this->objs[parent].o)->IsGroup()) {
	    fprintf(stderr, "fig: tried to insert an object into non-group\n");
	    return figure_NULLREF;
	}
	tmp = (class figogrp *)(this->objs[parent].o);

	for (pt = (tmp)->GetRootPtr(), count=0;
	     *pt != figure_NULLREF && count != depth; 
	     pt = &(this->objs[*pt].next), count++);
	oldroot = *pt;
	*pt = ix;
	this->objs[ix].next = oldroot;
	
	(o)->SetParent( parent, this);
	(o)->UpdateParentBounds();
    }
    this->objs[ix].o = o;
    this->objs[ix].counter = this->ocounter;
    this->ocounter++;
    (this)->SetModified();
    return ix;
}

boolean figure::DeleteObject(class figobj  *o)
{
    if ((this)->GetReadOnly())
	return FALSE;

    return (this)->AlwaysDeleteObject( o);
}

/* delete object o and all of its contents. If o is the root group, the fig is emptied and fig->root = NULLREF. */
boolean figure::AlwaysDeleteObject(class figobj  *o)
{
    long ix;
    long *pt;
    class figogrp *paro;

    if ((o)->IsGroup()) {
	for (ix=((class figogrp *)o)->GetRoot(); ix!=figure_NULLREF; ix=this->objs[ix].next) {
	    (this)->AlwaysDeleteObject( this->objs[ix].o);
	}
    }

    (o)->UpdateParentBounds();
    paro = (o)->GetParent();
    if (paro==NULL) {
	ix = this->root;
	this->root = figure_NULLREF;
    }
    else {
	for (pt = (paro)->GetRootPtr(); *pt != figure_NULLREF; pt = &(this->objs[*pt].next))
	    if (this->objs[*pt].o == o) break;

	if (*pt == figure_NULLREF) return FALSE;

	ix = *pt;
	*pt = this->objs[*pt].next;
    }

    this->objs[ix].o = NULL;
    (o)->SetParent( figure_NULLREF, NULL);

    (this)->SetModified();

    return TRUE;
}

void figure::LinkObjectByRef(long  ref, long  parent /* if figure_NULLREF, put in top-level group, unless there is none. */, long  depth /* 0 = back, 1 = next to back, etc. -1 or a high value means in front. */)
{
    if ((this)->GetReadOnly())
	return;

    (this)->AlwaysLinkObjectByRef( ref, parent, depth);
}

void figure::AlwaysLinkObjectByRef(long  ref, long  parent /* if figure_NULLREF, put in top-level group, unless there is none. */, long  depth /* 0 = back, 1 = next to back, etc. -1 or a high value means in front. */)
{
    long *pt;
    long count, oldroot;
    class figobj *o = this->objs[ref].o;

    if (parent==figure_NULLREF)
	parent = this->root;

    if (parent==figure_NULLREF) {
	/* the very first object */
	this->root = ref;
	this->objs[ref].next = figure_NULLREF;
	(o)->SetParent( figure_NULLREF, this);
    }
    else {
	class figogrp *tmp;
	if (!(this->objs[parent].o)->IsGroup()) {
	    fprintf(stderr, "fig: tried to insert an object into non-group\n");
	    return;
	}
	tmp = (class figogrp *)(this->objs[parent].o);

	for (pt = (tmp)->GetRootPtr(), count=0;
	     *pt != figure_NULLREF && count != depth; 
	     pt = &(this->objs[*pt].next), count++);
	oldroot = *pt;
	*pt = ref;
	this->objs[ref].next = oldroot;
	
	(o)->SetParent( parent, this);
	(o)->UpdateParentBounds();
    }
    (this)->SetModified();
}

void figure::UnlinkObjectByRef(long  ref)
{
    if ((this)->GetReadOnly())
	return;

    (this)->AlwaysUnlinkObjectByRef( ref);
}

void figure::AlwaysUnlinkObjectByRef(long  ref)
{
    long *pt;
    class figogrp *paro;
    class figobj *o = this->objs[ref].o;

    paro = (o)->GetParent();
    if (paro==NULL) {
	return; /* can't unlink root group */
    }

    (o)->UpdateParentBounds();

    for (pt = (paro)->GetRootPtr(); *pt != figure_NULLREF; pt = &(this->objs[*pt].next))
	if (*pt == ref) break;

    if (*pt == figure_NULLREF) return;

    *pt = this->objs[*pt].next;
    this->objs[ref].next = figure_NULLREF;

    /* we'll leave the parent pointer alone, but don't trust it. */

    (this)->SetModified();
}

long figure::FindDepthByRef(long  ref)
{
    long *pt;
    long count;
    class figogrp *paro;
    class figobj *o = this->objs[ref].o;

    paro = (o)->GetParent();
    if (paro==NULL) {
	return 0; /* root group */
    }

    for (pt = (paro)->GetRootPtr(), count=0; 
	  *pt != figure_NULLREF;
	  pt = &(this->objs[*pt].next), count++)
	if (*pt == ref) break;

    if (*pt == figure_NULLREF) return -1;
    return count;
}

static class figobj *EOT_AllArea(class figure  *self, long  grp, figure_eofptr callfun, long  rock)
{
    long ix, startval;
    class figobj *this_c, *ores;
    boolean res;

    if (grp==figure_NULLREF) {
	startval = self->root;
    }
    else {
	class figogrp *gr = (class figogrp *)self->objs[grp].o;
	startval = (gr)->GetRoot();
    }
    for (ix=startval; ix!=figure_NULLREF; ix=self->objs[ix].next) {
	this_c=self->objs[ix].o;
	res = (*callfun)(this_c, ix, self, rock);
	if (res)
	    return this_c;
	if ((this_c)->IsGroup()) {
	    ores = EOT_AllArea(self, ix, callfun, rock);
	    if (ores)
		return ores;
	}
    }
    return NULL;
}

static class figobj *EOT_OverlapArea(class figure  *self, long  grp, struct rectangle  *area, figure_eofptr callfun, long  rock)
{
    long ix, startval;
    class figobj *this_c, *ores;
    boolean res;
    struct rectangle tmp;

    if (grp==figure_NULLREF) {
	startval = self->root;
    }
    else {
	class figogrp *gr = (class figogrp *)self->objs[grp].o;
	startval = (gr)->GetRoot();
    }
    for (ix=startval; ix!=figure_NULLREF; ix=self->objs[ix].next) {
	this_c=self->objs[ix].o;
	rectangle_IntersectRect(&tmp, area, (this_c)->GetBounds( NULL));
	if (!rectangle_IsEmptyRect(&tmp)) {
	    res = (*callfun)(this_c, ix, self, rock);
	    if (res)
		return this_c;
	    if ((this_c)->IsGroup()) {
		ores = EOT_OverlapArea(self, ix, area, callfun, rock);
		if (ores)
		    return ores;
	    }
	}
    }
    return NULL;
}

static class figobj *EOT_IncludeArea(class figure  *self, long  grp, struct rectangle  *area, figure_eofptr callfun, long  rock)
{
    long ix, startval;
    class figobj *this_c, *ores;
    boolean res;

    if (grp==figure_NULLREF) {
	startval = self->root;
    }
    else {
	class figogrp *gr = (class figogrp *)self->objs[grp].o;
	startval = (gr)->GetRoot();
    }
    for (ix=startval; ix!=figure_NULLREF; ix=self->objs[ix].next) {
	this_c=self->objs[ix].o;
	if (rectangle_IsEnclosedBy((this_c)->GetBounds( NULL), area)) {
	    res = (*callfun)(this_c, ix, self, rock);
	    if (res)
		return this_c;
	}
	if ((this_c)->IsGroup()) {
	    ores = EOT_IncludeArea(self, ix, area, callfun, rock);
	    if (ores)
		return ores;
	}
    }
    return NULL;
}

/* call callfun on every object in a group in an area. If area is NULL, call callfun on every object in the group. If grp is figure_NULLREF, then every object in the fig is checked.
callfun should be of the form
  boolean callfun(struct figobj *o, long ref, struct figure *self, rock)
If an invocation of callfun returns TRUE, the enumeration halts and EnumerateObjects returns that figobj. Otherwise, EnumerateObjects returns NULL.
*/
class figobj *figure::EnumerateObjectTree(long  grp, struct rectangle  *area, boolean  allowoverlap, figure_eofptr callfun, long  rock)
{
    if (!area) {
	return EOT_AllArea(this, grp, callfun, rock);
    }
    else if (allowoverlap)  {
	return EOT_OverlapArea(this, grp, area, callfun, rock);
    }
    else  {
	return EOT_IncludeArea(this, grp, area, callfun, rock);
    }
}


/* call callfun on every object in an area. If area is NULL, call callfun on every object.
callfun should be of the form
  boolean callfun(struct figobj *o, long ref, struct figure *self, rock)
If an invocation of callfun returns TRUE, the enumeration halts and EnumerateObjects returns that figobj. Otherwise, EnumerateObjects returns NULL.
*/
class figobj *figure::EnumerateObjectGroup(long  grp, struct rectangle  *area, boolean  allowoverlap, figure_eofptr callfun, long  rock)
{
    int ix;
    struct rectangle tmp;
    class figobj *this_c;
    boolean res;
    long first;

    if (grp!=figure_NULLREF) {
	class figogrp *gr = (class figogrp *)this->objs[grp].o;
	first = (gr)->GetRoot();
    }
    else {
	first = (this)->RootObjRef();
    }

    if (!area) {
	for (ix=first; ix!=figure_NULLREF; ix=this->objs[ix].next) {
	    this_c=this->objs[ix].o;
	    res = (*callfun)(this_c, ix, this, rock);
	    if (res)
		return this_c;
	}
	return NULL;
    }
    else if (allowoverlap)  {
	for (ix=first; ix!=figure_NULLREF; ix=this->objs[ix].next) {
	    this_c=this->objs[ix].o;
	    rectangle_IntersectRect(&tmp, area, (this_c)->GetBounds( NULL));
	    if (!rectangle_IsEmptyRect(&tmp)) {
		res = (*callfun)(this_c, ix, this, rock);
		if (res)
		    return this_c;
	    }
	}
	return NULL;
    }
    else  {
	for (ix=first; ix!=figure_NULLREF; ix=this->objs[ix].next) {
	    this_c=this->objs[ix].o;
	    if (rectangle_IsEnclosedBy((this_c)->GetBounds( NULL), area)) {
		res = (*callfun)(this_c, ix, this, rock);
		if (res)
		    return this_c;
	    }
	}
	return NULL;
    }
}

/* call callfun on every object in an area. If area is NULL, call callfun on every object.
callfun should be of the form
  boolean callfun(struct figobj *o, long ref, struct figure *self, rock)
If an invocation of callfun returns TRUE, the enumeration halts and EnumerateObjects returns that figobj. Otherwise, EnumerateObjects returns NULL.
*/
class figobj *figure::EnumerateObjects(struct rectangle  *area, boolean  allowoverlap, figure_eofptr callfun, long  rock)
{
    int ix;
    struct rectangle tmp;
    class figobj *this_c;
    boolean res;

    if (!area) {
	for (ix=0; ix<this->objs_size; ix++) {
	    this_c=this->objs[ix].o;
	    if (this_c) {
		res = (*callfun)(this_c, ix, this, rock);
		if (res)
		    return this_c;
	    }
	}
	return NULL;
    }
    else if (allowoverlap)  {
	for (ix=0; ix<this->objs_size; ix++) {
	    this_c=this->objs[ix].o;
	    if (this_c) {
		rectangle_IntersectRect(&tmp, area, (this_c)->GetBounds( NULL));
		if (!rectangle_IsEmptyRect(&tmp)) {
		    res = (*callfun)(this_c, ix, this, rock);
		    if (res)
			return this_c;
		}
	    }
	}
	return NULL;
    }
    else  {
	for (ix=0; ix<this->objs_size; ix++) {
	    this_c=this->objs[ix].o;
	    if (this_c) {
		if (rectangle_IsEnclosedBy((this_c)->GetBounds( NULL), area)) {
		    res = (*callfun)(this_c, ix, this, rock);
		    if (res)
			return this_c;
		}
	    }
	}
	return NULL;
    }
}

long figure::FindRefByObject(class figobj  *o)
{
    int ix;

    for (ix=0; ix<this->objs_size; ix++)
	if (this->objs[ix].o == o)
	    return ix;

    return figure_NULLREF;
}
 
long figure::FindRefByName(char *nm)
{
    int ix;

    for (ix=0; ix<this->objs_size; ix++) {
	if(objs[ix].o) {
	    const char *n=this->objs[ix].o->GetName();
	    if (n && !strcmp(n, nm))
		return ix;
	}
    }

    return figure_NULLREF;
}
 
class figobj *figure::FindObjectByRef(long  ref)
{
    if (ref<0 || ref>=this->objs_size)
	return NULL;

    return this->objs[ref].o;
}

static long FRBPSplot(class figure  *self, long  gref, enum figobj_HitVal  howhit, long  x , long  y, long  delta, long  *ptref)
{
    long ix, jx;
    enum figobj_HitVal res;
    class figogrp *grp = (class figogrp *)self->objs[gref].o;

    for (ix=(grp)->GetRoot(); ix!=figure_NULLREF; ix=self->objs[ix].next) {
	res = (self->objs[ix].o)->HitMe( x, y, delta, ptref);
	if (((int)res) >= ((int)howhit)) {
	    /* got it */
	    return ix;
	}
	if ((self->objs[ix].o)->IsGroup()) {
	    jx = FRBPSplot(self, ix, howhit, x, y, delta, ptref);
	    if (jx != figure_NULLREF) {
		return jx;
	    }
	}
    }
    return figure_NULLREF;
}

/* search the tree under group gref for an object at x, y. If recursive is true, whole tree is searched; otherwise, just that group. howhit determines how good the hit has to be to be returned. *ptref will be set to a handle if HitMe returns one. delta is the allowable distance from a handle (in fig coords). */ 
long figure::FindRefByPos(long  gref, boolean  recursive, enum figobj_HitVal  howhit, long  delta, long  x , long  y, long  *ptref)
{
    int ix;
    long tmp = figobj_NULLREF;
    enum figobj_HitVal res;

    if (gref == figure_NULLREF) {
	for (ix=0; ix<this->objs_size; ix++)
	    if (this->objs[ix].o) {
		res = (this->objs[ix].o)->HitMe( x, y, delta, &tmp);
		if (((int)res) >= ((int)howhit)) {
		    /* got it */
		    if (ptref)
			*ptref = tmp;
		    return ix;
		}
	    }
	return figure_NULLREF;
    }
    else if (recursive) {
	ix = FRBPSplot(this, gref, howhit, x, y, delta, &tmp);
	if (ix != figure_NULLREF && ptref)
	    *ptref = tmp;
	return ix;
    }
    else {
	class figogrp *grp = (class figogrp *)this->objs[gref].o;
	for (ix=(grp)->GetRoot(); ix!=figure_NULLREF; ix=this->objs[ix].next) {
	    res = (this->objs[ix].o)->HitMe( x, y, delta, &tmp);
	    if (((int)res) >= ((int)howhit)) {
		/* got it */
		if (ptref)
		    *ptref = tmp;
		return ix;
	    }
	}
	return figure_NULLREF;
    }
}

struct rectangle *figure::GetOverallBounds()
{
    class figobj *o = NULL;

    if (!this->bboxdirty) {
	return &(this->bbox);
    } 

    this->bboxdirty = FALSE;
    if ((this)->RootObjRef() != figure_NULLREF) {
	o = this->objs[(this)->RootObjRef()].o;
    }
    if (o)
	this->bbox = *((o)->GetBounds( NULL));
    else
	rectangle_EmptyRect(&this->bbox);

    return &(this->bbox);
}

#define LINELENGTH (250)
static char buf[LINELENGTH+1];

static void WriteObject(class figure  *self, long  oref, FILE  *fp, long  writeid, int  level)
{
    class figobj *o = self->objs[oref].o;
    /* printf("fig: WriteObject(%d): %s\n", oref, class_GetTypeName(o)); */

    if (!(o)->IsGroup()) {
	(o)->Write( fp, writeid, level+1);
    }
    else {
	long root = ((class figogrp *)o)->GetRoot();
	long ix;

	if (root == figure_NULLREF && (o)->GetParentRef() != figure_NULLREF) {
	    return; /* it's an empty group; don't bother writing it out. */
	}

	(o)->Write( fp, writeid, level+1);

	for (ix=root; ix!=figure_NULLREF; ix=self->objs[ix].next)
	    WriteObject(self, ix, fp, writeid, level+1);
	fprintf(fp, "#end\n");
    }
}

static long ReadObject(class figure  *self, class figobj  *o , long  oref, FILE  *fp, long  oid)
{
    long ix, subref, tid;
    char namebuf[100];
    class figobj *subo;

    ix = (o)->Read( fp, oid);
    if (ix!=dataobject_NOREADERROR) return ix;

    /* printf("fig: ReadObject(%d): %s\n", oref, class_GetTypeName(o)); */
    if (!(o)->IsGroup())
	return dataobject_NOREADERROR;

    if (fgets(buf, LINELENGTH, fp) == NULL)
	return dataobject_PREMATUREEOF;

    while (1) {
	if (!strncmp(buf, "#end", 4)) 
	    return dataobject_NOREADERROR;

	ix = sscanf(buf, "\\begindata{%[^,],%ld}", namebuf, &tid);
	if (ix!=2) return dataobject_BADFORMAT;

	if (!ATK::IsTypeByName(namebuf, "figobj"))
	    return dataobject_BADFORMAT;
	subo = (class figobj *)ATK::NewObject(namebuf);
	if (!subo) return dataobject_OBJECTCREATIONFAILED;

	subref = (self)->AlwaysInsertObject( subo, oref, -1);
	ix = ReadObject(self, subo, subref, fp, tid);
	if (ix!=dataobject_NOREADERROR) return ix;

	if (fgets(buf, LINELENGTH, fp) == NULL)
	    return dataobject_PREMATUREEOF;
    }
}

long figure::Write(FILE  *fp, long  writeid, int  level)
{
    if ((this)->GetWriteID() != writeid) {
	(this)->SetWriteID( writeid);

	fprintf(fp, "\\begindata{%s,%ld}\n", (this)->GetTypeName(), (this)->GetID());
	fprintf(fp, "$origin %ld %ld\n", this->originx, this->originy);
	fprintf(fp, "$printscale %f %f\n", this->printscalex, this->printscaley);
	/* this is to make future expansion easier. */
	fprintf(fp, "#none %d\n", this->GetPrintLandscape()); 
	if (this->root == figure_NULLREF)
	    fprintf(fp, "#empty\n");
	else {
	    WriteObject(this, this->root, fp, writeid, level+1);
	}

	fprintf(fp, "\\enddata{%s,%ld}\n", (this)->GetTypeName(), (this)->GetID());
    }

    return (this)->GetID();
}

void figure::WritePartial(FILE  *fp, long  writeid, int  level, long  *list, long  listnum, struct point  *origin)
{
    class figogrp *tmpgrp;
    int ix;
    struct rectangle *tmp, bbox;

    rectangle_EmptyRect(&bbox);
    for (ix=0; ix<listnum; ix++) {
	tmp = (this->objs[list[ix]].o)->GetBounds( NULL);
	rectangle_UnionRect(&bbox, &bbox, tmp);
    }

    fprintf(fp, "\\begindata{%s,%ld}\n", (this)->GetTypeName(), (this)->GetID());
    fprintf(fp, "$origin %ld %ld\n", bbox.left-64, bbox.top-64);
    if (origin) {
	origin->x = bbox.left-64;
	origin->y = bbox.top-64;
    }
    fprintf(fp, "$printscale %f %f\n", this->printscalex, this->printscaley);
    fprintf(fp, "#none %d\n", this->GetPrintLandscape()); 

    /* create a temporary group and set its handlebox to (-1,-1,-1,-1) so that we know that those values are meaningless. */
    tmpgrp = new figogrp;
    tmpgrp->handlebox.left = (-1);
    tmpgrp->handlebox.top = (-1);
    tmpgrp->handlebox.width = (-1);
    tmpgrp->handlebox.height = (-1);

    (tmpgrp)->Write( fp, writeid, level+1);
    for (ix=0; ix<listnum; ix++) {
	WriteObject(this, list[ix], fp, writeid, level+1);
    }
    fprintf(fp, "#end\n");
    (tmpgrp)->Destroy();

    fprintf(fp, "\\enddata{%s,%ld}\n", (this)->GetTypeName(), (this)->GetID());
}

long figure::Read(FILE  *fp, long  id)
{
    long tid, ix, ref, val1, val2;
    double fal1, fal2;
    char namebuf[100];
    class figobj *o;

    (this)->SetID( (this)->UniqueID()); 

    if (fgets(buf, LINELENGTH, fp) == NULL)
	return dataobject_PREMATUREEOF;
    ix = sscanf(buf, "$origin %ld %ld", &val1, &val2);
    if (ix!=2) return dataobject_BADFORMAT;
    this->originx = val1;
    this->originy = val2;

    if (fgets(buf, LINELENGTH, fp) == NULL)
	return dataobject_PREMATUREEOF;
    ix = sscanf(buf, "$printscale %lf %lf", &fal1, &fal2);
    if (ix!=2) return dataobject_BADFORMAT;
    this->printscalex = fal1;
    this->printscaley = fal2;

    if (fgets(buf, LINELENGTH, fp) == NULL)
	return dataobject_PREMATUREEOF;
    if (strncmp(buf, "#none", 5)) return dataobject_BADFORMAT;
    ix = sscanf(buf, "#none %ld", &val1); /* optional parameter */
    if (ix==1) {
	val1 = (val1) ? 1 : 0;
	this->SetPrintLandscape(val1);
    }

    if (fgets(buf, LINELENGTH, fp) == NULL)
	return dataobject_PREMATUREEOF;

    if ((this)->RootObjRef() != figure_NULLREF)
	(this)->AlwaysDeleteObject( this->objs[(this)->RootObjRef()].o);

    if (!strncmp(buf, "#empty", 6)) {

    }
    else {
	ix = sscanf(buf, "\\begindata{%[^,],%ld}", namebuf, &tid);
	if (ix!=2) return dataobject_BADFORMAT;

	if (!ATK::IsTypeByName(namebuf, "figobj"))
	    return dataobject_BADFORMAT;
	o = (class figobj *)ATK::NewObject(namebuf);
	if (!o) return dataobject_OBJECTCREATIONFAILED;
	
	ref = (this)->AlwaysInsertObject( o, figure_NULLREF, -1);
	this->root = ref;
	ix = ReadObject(this, o, ref, fp, tid);
	if (ix!=dataobject_NOREADERROR) return ix;
    }

    if (fgets(buf, LINELENGTH, fp) == NULL)
	return dataobject_PREMATUREEOF;
    ix = sscanf(buf, "\\enddata{%[^,],%ld}", namebuf, &tid);
    if (ix!=2) return dataobject_MISSINGENDDATAMARKER;

    if ((id && tid!=id) || strcmp(namebuf, (this)->GetTypeName()))
	return dataobject_MISSINGENDDATAMARKER;

    (this)->GetOverallBounds();

    return dataobject_NOREADERROR;     
}

/* put stuff from file into focus. The root group of the file is *not* inserted; everything in the root group is copied into the focus group of self. The origin field of the file is returned in origin.
### If the root group of the file has attributes active, they will overwrite the focus group's original attributes. This is not a problem for pasting, since a copied fig has nothing active.
*/
long figure::ReadPartial(FILE  *fp, long  id, long  focus, struct point  *origin)
{
    long tid, ix, val1, val2;
    double fal1, fal2;
    char namebuf[100];
    class figobj *o;
    /* long unid;
    if (id==0) 
	unid = figure_UniqueID(self);
    else
	unid = id;
    figure_SetID(self, unid);*/ /* ### ??? */

    if (fgets(buf, LINELENGTH, fp) == NULL)
	return dataobject_PREMATUREEOF;
    ix = sscanf(buf, "$origin %ld %ld", &val1, &val2);
    if (ix!=2) return dataobject_BADFORMAT;
    if (origin) {
	origin->x = val1;
	origin->y = val2;
    }

    if (fgets(buf, LINELENGTH, fp) == NULL)
	return dataobject_PREMATUREEOF;
    ix = sscanf(buf, "$printscale %lf %lf", &fal1, &fal2);
    if (ix!=2) return dataobject_BADFORMAT;

    if (fgets(buf, LINELENGTH, fp) == NULL)
	return dataobject_PREMATUREEOF;
    if (strncmp(buf, "#none", 5)) return dataobject_BADFORMAT;
    /* ignore PrintLandscale value for partial read */

    if (fgets(buf, LINELENGTH, fp) == NULL)
	return dataobject_PREMATUREEOF;

    if (!strncmp(buf, "#empty", 6)) {

    }
    else {
	ix = sscanf(buf, "\\begindata{%[^,],%ld}", namebuf, &tid);
	if (ix!=2) return dataobject_BADFORMAT;

	if (!ATK::IsTypeByName(namebuf, "figobj"))
	    return dataobject_BADFORMAT;

	o = (this)->FindObjectByRef( focus);

	ix = ReadObject(this, o, focus, fp, tid); 

	if (ix!=dataobject_NOREADERROR) return ix;
    }

    if (fgets(buf, LINELENGTH, fp) == NULL)
	return dataobject_PREMATUREEOF;
    ix = sscanf(buf, "\\enddata{%[^,],%ld}", namebuf, &tid);
    if (ix!=2) return dataobject_MISSINGENDDATAMARKER;

    if ((id && tid!=id) || !ATK::IsTypeByName(namebuf, "figure"))
	return dataobject_MISSINGENDDATAMARKER;

    /*printf("figure_ReadPartial: done OK\n");*/
    return dataobject_NOREADERROR; 
}
