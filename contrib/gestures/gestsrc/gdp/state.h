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

