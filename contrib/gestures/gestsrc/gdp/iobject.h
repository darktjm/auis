/***********************************************************************

Copyright (C) 1991 Dean Rubine

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License. See ../COPYING for
the full agreement.

**********************************************************************/

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

extern void Erase(Object o);
extern void Draw(Object o);
extern void LineDraw(Object o);
extern void LineTransform(Object o, Transformation t);
extern void RectDraw(Object o);
extern void RectTransform(Object o, Transformation t);
extern void TextDraw(Object o);
extern void TextTransform(Object o, Transformation t);
extern void CircleDraw(Object o);
extern void CircleTransform(Object o, Transformation t);
extern int LineDistance(Object o, int x, int y);
extern int RectDistance(Object o, int x, int y);
extern int TextDistance(Object o, int x, int y);
extern int CircleDistance(Object o, int x, int y);
extern void SetDraw(Object o);
extern void SetTransform(Object o, Transformation t);
extern int SetDistance(Object o, int x, int y);

Bool	AlwaysOK(void), AlwaysBad(void), VlinePoint(Object o, int which);

extern struct dope {
	ObjectType	type;
	const char	*name;
	int		npoints;
	Bool		(*point)();
	void		(*draw)();
	void		(*transform)();
	int		(*distance)();  /* actually distance squared */
} dope[];
