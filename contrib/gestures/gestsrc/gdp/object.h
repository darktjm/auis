/***********************************************************************

Copyright (C) 1991 Dean Rubine

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License. See ../COPYING for
the full agreement.

**********************************************************************/

/*
  Operations on objects
 */

typedef struct object *Object;

typedef	enum {
	Nothing,
	Line,
	Vline,
	Rect,
	Vrect,
	Text,
	Circle,
	SetOfObjects,
} ObjectType;


extern Object CreateObject(ObjectType type);
extern Bool UpdatePoint(Object o, int which, int x, int y);
extern Bool AddSubObject(Object o, Object subo);
extern Object CopyObject(Object old);
extern void FreeObject(Object o);
#include "transform.h"
extern void Transform(Object o, Transformation t);
extern int Distance(Object o, int x, int y);
extern void ObjHighlight(Object o, int highlight);
