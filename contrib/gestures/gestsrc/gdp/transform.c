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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/gestures/gestsrc/gdp/RCS/transform.c,v 1.3 1992/12/15 21:50:19 rr2b Stab74 $";
#endif

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
AllocTran()
{
	return NewMatrix(3, 3);
}

Transformation
IdentityTran(t)
register Transformation t;
{
	t[0][0] = 1;	t[0][1] = 0;	t[0][2] = 0;
	t[1][0] = 0;	t[1][1] = 1;	t[1][2] = 0;
	t[2][0] = 0;	t[2][1] = 0;	t[2][2] = 1;
	return t;
}

Transformation
TranslateTran(t, x, y)
register Transformation t;
int x, y;
{
	t[0][0] = 1;	t[0][1] = 0;	t[0][2] = 0;
	t[1][0] = 0;	t[1][1] = 1;	t[1][2] = 0;
	t[2][0] = x;	t[2][1] = y;	t[2][2] = 1;
	return t;
}

/* man 3 sin | grep 'pi =' */

#define 	PI 	3.1415926535897932384626433

Transformation
RotateTran(t, theta)
register Transformation t;
int theta;
{
	double rad = (PI/180.0) * theta;
	return RotateCosSinTran(t, cos(rad), sin(rad));
}

Transformation
RotateCosSinTran(t, c, s)
register Transformation t;
double c, s;
{
	t[0][0] = c;	t[0][1] = s;	t[0][2] = 0;
	t[1][0] = -s;	t[1][1] = c;	t[1][2] = 0;
	t[2][0] = 0;	t[2][1] = 0;	t[2][2] = 1;
	return t;
}

Transformation
ScaleTran(t, dilation)
register Transformation t;
double dilation;
{
	t[0][0] = dilation; 	t[0][1] = 0; 		t[0][2] = 0;
	t[1][0] = 0;		t[1][1] = dilation;	t[1][2] = 0;
	t[2][0] = 0;		t[2][1] = 0;		t[2][2] = 1;
	return t;
}

void
ApplyTran(x, y, t, xp, yp)
int x, y, *xp, *yp;
register Transformation t;
{
	/* .5 for rounding */
	if(xp)
		*xp = .5 + x * t[0][0] + y * t[1][0] + t[2][0];
	if(yp)
		*yp = .5 + x * t[0][1] + y * t[1][1] + t[2][1];
}

double
TransScale(t)
register Transformation t;
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
