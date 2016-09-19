/***********************************************************************

Copyright (C) 1991 Dean Rubine

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License. See ../COPYING for
the full agreement.

**********************************************************************/

#include <gestures/util.h>
#include <gestures/matrix.h>
#include "transform.h"
#include <math.h>
#include <stdio.h>

/*
 Transformation matrix:

 r00 r01  0
 r10 r11  0
 tx  ty   1

*/

Transformation
AllocTran(void)
{
	return NewMatrix(3, 3);
}

Transformation
IdentityTran(Transformation t)
{
	t[0][0] = 1;	t[0][1] = 0;	t[0][2] = 0;
	t[1][0] = 0;	t[1][1] = 1;	t[1][2] = 0;
	t[2][0] = 0;	t[2][1] = 0;	t[2][2] = 1;
	return t;
}

Transformation
TranslateTran(Transformation t, int x, int y)
{
	t[0][0] = 1;	t[0][1] = 0;	t[0][2] = 0;
	t[1][0] = 0;	t[1][1] = 1;	t[1][2] = 0;
	t[2][0] = x;	t[2][1] = y;	t[2][2] = 1;
	return t;
}

Transformation
RotateTran(Transformation t, int theta)
{
	double rad = (M_PI/180.0) * theta;
	return RotateCosSinTran(t, cos(rad), sin(rad));
}

Transformation
RotateCosSinTran(Transformation t, double c, double s)
{
	t[0][0] = c;	t[0][1] = s;	t[0][2] = 0;
	t[1][0] = -s;	t[1][1] = c;	t[1][2] = 0;
	t[2][0] = 0;	t[2][1] = 0;	t[2][2] = 1;
	return t;
}

Transformation
ScaleTran(Transformation t, double dilation)
{
	t[0][0] = dilation; 	t[0][1] = 0; 		t[0][2] = 0;
	t[1][0] = 0;		t[1][1] = dilation;	t[1][2] = 0;
	t[2][0] = 0;		t[2][1] = 0;		t[2][2] = 1;
	return t;
}

void
ApplyTran(int x, int y, Transformation t, int *xp, int *yp)
{
	/* .5 for rounding */
	if(xp)
		*xp = .5 + x * t[0][0] + y * t[1][0] + t[2][0];
	if(yp)
		*yp = .5 + x * t[0][1] + y * t[1][1] + t[2][1];
}

double
TransScale(Transformation t)
{
	static double last = -1, lastres = -1;
	double d;

	d = t[0][0] * t[1][1] - t[1][0] * t[0][1];
	if(d != last) {		/* cache to avoid repeated sqrt */
		last = d;
		lastres = sqrt(d);
	}
	return lastres;
}

