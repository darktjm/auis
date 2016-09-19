#ifndef _gf_h_
#define _gf_h_
/***********************************************************************

gf.h - read and write gesture classifier files

Copyright (C) 1991 Dean Rubine

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License. See ../COPYING for
the full agreement.

**********************************************************************/

#define	MAXGESTURES	10000
#define	MAXPOINTS	1000

typedef struct gesture *Gesture;
typedef struct gpoint *Gpoint;
typedef struct gesturefile *GestureFile;

GestureFile ReadGestureFile();

struct gesturefile  {
	int ngestures;
	struct gesture {
		int	classindex;
		char	*examplename;
		int	npoints;
		struct gpoint { short path, x, y, t; } *point;
		Vector	y;
	} gesture[MAXGESTURES];
};

#define ClassName(g)	(classname[(g)->classindex])

/* gf.c */
extern char *classname[];
extern int ClassEnter(char *name);
extern GestureFile ReadGestureFile(char *infilename);
extern void WriteGesture(FILE *outfile, Gesture g, char *classname);
#endif
