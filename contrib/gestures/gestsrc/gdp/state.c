/***********************************************************************

Copyright (C) 1991 Dean Rubine

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License. See ../COPYING for
the full agreement.

**********************************************************************/

/*
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

#include <gestures/bool.h>
#include <gestures/util.h>
#include "object.h"
#include "set.h"
#include "iobject.h"
#include <gestures/matrix.h>
#include "transform.h"
#include "state.h"
#include <stdio.h>
#include <gestures/gdev.h>
#include "proto.h"

#define	EtoO(e)	( (Object) ElementPointer(e))

/* The state of the picture is entirely described by these variables: */

static	Set	Objs;	/* set of all visible objects */

static	VersionNumber	lastckp;

Set
StEmptySet(void)
{
	return EmptySet(NULL, NULL, Objs);
}

static void object_added(Set s, Element e, Pointer p);
static void object_deleted(Set s, Element e, Pointer p);

void
StInit(void)
{
	Objs = EmptySet(object_added, object_deleted, NULL);
	lastckp = CheckpointSetGroup(Objs);
}

#define LINESIZE 100
static char bottom_line[LINESIZE];

static void
DrawBottomLine(void)
{
	char line[102];
	sprintf(line, "%-100.100s", bottom_line);
	GDEVtext(0, 0, line);
}

void
StBottomLine(const char *string)
{
	DrawBottomLine();  /* Erases old bottom line */
	strncpy(bottom_line, string, LINESIZE);
	DrawBottomLine();
}


static void mDraw(DllElement e, Pointer p)
{
    Draw((Object)e);
}

void
StRedraw(void)
{
	DrawClear();
	Map(Objs, mDraw, NULL);
	DrawBottomLine();
	WmFlush();
}

Element
StNewObj(ObjectType t)
{
	Object o = CreateObject(t);
	return AddElement(Objs, (Pointer)o);
}

void
StAddSubObject(Element e, Element sube)
{
	AddSubObject(EtoO(e),EtoO(sube));
}

Element
StCopyElement(Element e)
{
	Object o = CopyObject(EtoO(e));
	return AddElement(Objs, (Pointer)o);
}


void
StDelete(Element e)
{
	DeleteElement(Objs, e);
}

static
void
object_added(Set s, Element e, Pointer p)
{
	Object o = (Object)p;
	Draw(o);
}

static
void
object_deleted(Set s, Element e, Pointer p)
{
	Object o = (Object)p;
	Erase(o);
}

void
StUpdatePoint(Element e, int point_number, int x, int y)
{
	UpdatePoint(EtoO(e), point_number, x, y);
}

void
StTransform(Element e, Transformation t)
{
	Transform(EtoO(e), t);
}

void
StMove(Element e, int x, int y)
{
	static Transformation t;

	if(t == NULL) t = AllocTran();
	StTransform(e, TranslateTran(t, x, y));
}

static int _x, _y, _min_d;
static Element _min_e;

static void
sizem(Element e, Pointer p)
{
	int d = Distance(EtoO(e), _x, _y);
	if(d < _min_d) {
		_min_d = d;
		_min_e = e;
	}
}

Element
StPick(int x, int y)	/* pick the item pointed to by x, y */
{
	_x = x; _y = y;
	_min_d = 5;	/* must be within five pixels */
	_min_d = 20;	/* must be within 20 pixels */
	_min_e = NULL;
	MapE(Objs, sizem, NULL);
	return _min_e;
}

void
StReplaceObject(Element e, Object s)
{
	Object d = EtoO(e);

/*
	printf("StReplaceObject: \nreplacing ("); DumpObject(d);
	printf("with ("); DumpObject(s);
*/

	if(d->type != s->type)
		recog_error("ReplaceObject: type change");
	if(d->type == SetOfObjects) {
		Element se, de;
		IterateSet(s->subobjects);
		IterateSet(d->subobjects);
		for(;;) {
			se = NextElement(s->subobjects);
			de = NextElement(d->subobjects);
			if(se == NULL && de == NULL)
				break;
			if(se == NULL || de == NULL)
				recog_error("ReplaceObject: set sizes differ");
			StReplaceObject(de, EtoO(se));
		}
	}
	else {
		Erase(d);
		*d = *s;
		Draw(d);
	}
}

Element
StEdit(Element e, Object *oldo)
{
	Object o = CopyObject(EtoO(e));
	if(oldo)
		*oldo = EtoO(e);
	/* printf("StEdit old=%x, copy=%x\n", EtoO(e), o); */

	DeleteElement(Objs, e);
	return AddElement(Objs, (Pointer)o);
/*
	return e;
*/
}

void StCheckpoint(void)
{
	lastckp = CheckpointSetGroup(Objs);
}

void StUndo(void)
{
	UndoSetGroup(Objs, lastckp);
}

#if 0
static void
CopySetHelp(Set s)
{
	Element e = AnElement(s);
	Object o;

	if(e == NULL)
		return;
	o = CopyObject(EtoO(e));
	printf("."); fflush(stdout);
	DeleteElement(s, e);
	CopySetHelp(s);
	AddElement(s, (Pointer)o);
}
#endif

static void
copysethelp(DllElement e, Pointer p)
{
	AddElement((Set)p, (Pointer)CopyObject((Object)e));
}

Set
CopySet(Set s)
{
	Set new = EmptySet(NULL, NULL, Objs);
	Map(s, copysethelp, (Pointer)new);
	return new;
}


void StHighlight(Element e, int highlight)
{
	ObjHighlight(EtoO(e), highlight);
}

#if 0
static void StErase(Element e)
{
	Erase(EtoO(e));
}

static void StDraw(Element e)
{
	Draw(EtoO(e));
}
#endif
