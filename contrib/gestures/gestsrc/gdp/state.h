/***********************************************************************

Copyright (C) 1991 Dean Rubine

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License. See ../COPYING for
the full agreement.

**********************************************************************/

extern void StInit(void);

extern void StRedraw(void);
extern Element StNewObj(ObjectType t);
extern void StDelete(Element e);
extern void StUpdatePoint(Element e, int point_number, int x, int y);
extern void StMove(Element e, int x, int y);
extern void StTransform(Element e, Transformation t);
extern Element StPick(int x, int y);
extern Element StEdit(Element e, Object *oldo);
extern Element StCopyElement(Element e);


extern Set CopySet(Set s);

extern Set StEmptySet(void);
extern void StBottomLine(const char *string);
extern void StAddSubObject(Element e, Element sube);
extern void StReplaceObject(Element e, Object s);
extern void StCheckpoint(void);
extern void StUndo(void);
extern void StHighlight(Element e, int highlight);

