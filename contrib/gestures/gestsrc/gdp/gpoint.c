/***********************************************************************

Copyright (C) 1991 Dean Rubine

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License. See ../COPYING for
the full agreement.

**********************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include "util.h"
#include <gestures/bool.h>
#include <gestures/gdev.h>
#include <gestures/util.h>
#include "proto.h"

#define	NLINES	50
#define	NPOINTS	500

/* extraneouse lines, not part of gesture */
static	int	nlines;
static	struct lines { int x1, y1, x2, y2; } line[NLINES];

/* endpoints of line segments which form gesture */
static	int	npoints;
static	struct point { int x, y; } point[NPOINTS];

static void Greset(void);
static void Gerase(void);

void Sreset(void) { Greset(); }
void Serase(void) { Gerase(); }

static void Greset(void)
{
	nlines = 0;
	npoints = 0;
}

static void Gerase(void)	/* just redraws, assumes XOR */
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

void Gpoint(int x, int y)
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

void Gline(int x1, int y1, int x2, int y2)
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
