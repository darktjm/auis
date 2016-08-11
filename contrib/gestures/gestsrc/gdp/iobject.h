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


#define	MAXPOINTS	3
#define	NOT_A_POINT	(-9999)

typedef struct {
	int	x, y;
} Point;

struct object {
	ObjectType	type;
	int		state;
	Point		point[MAXPOINTS];
	char		*text;
	Set		subobjects;
	char		highlight;
};

void	Erase(), Draw();
void	EraseOn(), EraseOff();

void	LineDraw();
void	LineTransform();
int	LineDistance();

void	RectDraw();
void	RectTransform();
int	RectDistance();

void	TextDraw();
void	TextTransform();
int	TextDistance();

void	CircleDraw();
void	CircleTransform();
int	CircleDistance();

void	SetDraw();
void	SetTransform();
int	SetDistance();

Bool	AlwaysOK(), AlwaysBad(), VlinePoint();

extern struct dope {
	ObjectType	type;
	char		*name;
	int		npoints;
	Bool		(*point)();
	void		(*draw)();
	void		(*transform)();
	int		(*distance)();  /* actually distance squared */
} dope[];
