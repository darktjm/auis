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

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include "util.h"
#include <gestures/bool.h>

#define	NLINES	50
#define	NPOINTS	500

/* extraneouse lines, not part of gesture */
static	int	nlines;
static	struct lines { int x1, y1, x2, y2; } line[NLINES];

/* endpoints of line segments which form gesture */
static	int	npoints;
static	struct point { int x, y; } point[NPOINTS];

Sreset() { Greset(); }
Serase() { Gerase(); }

Greset()
{
	nlines = 0;
	npoints = 0;
}

Gerase()	/* just redraws, assumes XOR */
{
	int i;

	GDEVsets("currentlinetype", "solid");
	for(i = 0; i < nlines; i++)
		GDEVline(	  line[i].x1, line[i].y1,
				  line[i].x2, line[i].y2);
	for(i = 1; i < npoints; i++)
		GDEVline(	  point[i-1].x, point[i-1].y,
				  point[i].x, point[i].y);
	GDEVflush();
}

Gpoint(x, y)
int x, y;
{
	if(npoints >= NPOINTS)
		recog_error("Gpoint");
	point[npoints].x = x;
	point[npoints].y = y;
	if(npoints > 0) {
		GDEVsets("currentlinetype", "solid");
		GDEVline( point[npoints-1].x, point[npoints-1].y, x, y);
	}
	npoints++;
}

Gline(x1, y1, x2, y2)
int x1, y1, x2, y2;
{
	if(nlines >= NLINES)
		recog_error("Gline");
	line[nlines].x1 = x1;
	line[nlines].y1 = y1;
	line[nlines].x2 = x2;
	line[nlines].y2 = y2;
	nlines++;
	GDEVsets("currentlinetype", "solid");
	GDEVline(x1, y1, x2, y2);
}
