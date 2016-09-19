/***********************************************************************

Copyright (C) 1991 Dean Rubine

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License. See ../COPYING for
the full agreement.

**********************************************************************/

#include "set.h"
#include <gestures/bool.h>
#include <gestures/util.h>
#include "object.h"
#include "iobject.h"
#include <gestures/matrix.h>
#include "transform.h"
#include <stdio.h>

struct dope dope[] = {
	{ Nothing, "nothing" },
	{ Line, "line",  2, AlwaysOK, LineDraw, LineTransform, LineDistance},
	{ Vline, "vline", 2, VlinePoint, LineDraw, LineTransform, LineDistance},
	{ Rect, "rect", 3, AlwaysOK, RectDraw, RectTransform, RectDistance},
	{ Vrect, "vrect", 3, AlwaysOK, RectDraw, RectTransform, RectDistance},
	{ Text, "text", 1, AlwaysOK, TextDraw, TextTransform, TextDistance },
	{ Circle, "circle", 2, AlwaysOK, CircleDraw, CircleTransform, CircleDistance },
	{ SetOfObjects, "set", 0, AlwaysOK, SetDraw, SetTransform, SetDistance },
};

#define NDOPE	((int)(sizeof(dope)/sizeof(dope[0])))

Object
CreateObject(ObjectType type)
{
	Object o = allocate(1, struct object);
	int i;

	if( (int) type < 0 || (int) type >= NDOPE)
		recog_error("CreateObject: bad type %d\n", (int) type);
	if(dope[ (int) type ].type != type )
		recog_error("CreateObject: internal inconsistency, dope[%d]\n",
				(int) type);

	o->type = type;
	for(i = 0; i < MAXPOINTS; i++)
		o->point[i].x = o->point[i].y = NOT_A_POINT;
	o->subobjects = NULL;
	if(o->type == SetOfObjects) {
		extern Set StEmptySet();
		o->subobjects = StEmptySet();
	}
	return o;
}

Bool
UpdatePoint(Object o, int which, int x, int y)
{
	struct dope *d = &dope[(int) o->type];
	struct object save;
	
	if(which < 0 || which > d->npoints)
		return FALSE;
	save = *o;
	o->point[which].x = x;
	o->point[which].y = y;
	if( (*d->point)(o, which) ) {	/* OK */
		Erase(&save);
		Draw(o);
		return TRUE;
	}
	*o = save;	/* restore */
	return FALSE;
}

Bool
AddSubObject(Object o, Object subo)
{
	if(o->type != SetOfObjects)
		return FALSE;
	Draw(subo);
	(void) AddElement(o->subobjects, (Pointer)subo);
	return TRUE;
}

Object
CopyObject(Object old)
{
	Object o = allocate(1, struct object);
	Set CopySet();
	*o = *old;
	if(o->type == SetOfObjects)
		o->subobjects = CopySet(o->subobjects);
	return o;
}

void
FreeObject(Object o)
{
	free( (char *) o);
}

void
Transform(Object o, Transformation t)
{
	struct dope *d = &dope[(int) o->type];
	if(o->type == SetOfObjects)
		(d->transform)(o, t);
	else {
		Erase(o);
		(d->transform)(o, t);
		Draw(o);
	}
}

/* --------------------- dope functions ---------------- */

Bool
AlwaysOK(void)
{
	return TRUE;
}

Bool
AlwaysBad(void)
{
	return FALSE;
}


Bool
VlinePoint(Object o, int which)
{
	/* make vertical or horizontal or 45 degrees */
	return TRUE;
}

int
Distance(Object o, int x, int y)
{
	return (*dope[(int)o->type].distance)(o, x, y);	
}

/* ---------------- object manipulation ----------- */

static void mObjHighlight(DllElement e, Pointer p)
{
    ObjHighlight((Object)e, (int)(long)p);
}

void ObjHighlight(Object o, int highlight)
{
	static int depth = 0;

/*
	printf("%d ", depth);
	for(i = 0; i < depth; i++)
		printf("  ");
	printf("ObjHighlight(type=%d, highlight=%d)\n",
		o->type, highlight);
*/
	

	if(o->type == SetOfObjects) {
		depth++;
		Map(o->subobjects, mObjHighlight, (Pointer)(long)highlight);
		depth--;
	} else {
		o->highlight = highlight;
	}

}
