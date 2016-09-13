#ifndef _gf_h_
#define _gf_h_
/***********************************************************************

gf.h - read and write gesture classifier files

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
