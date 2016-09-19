/***********************************************************************

gf.c - read and write gesture classifier files

Copyright (C) 1991 Dean Rubine

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License. See ../COPYING for
the full agreement.

**********************************************************************/

#include "stdio.h"
#include "util.h"
#include "bitvector.h"
#include "matrix.h"
#include "gf.h"
#include "proto.h"

#define MAXCLASSES 200
char *classname[MAXCLASSES];
static int nclassnames = 0;

int
ClassEnter(char *name)
{
	int i;
	for(i = nclassnames - 1; i >= 0; i--)
		if(STREQ(classname[i], name)) return i;
	classname[nclassnames] = recog_scopy(name);
	return nclassnames++;
}

GestureFile
ReadGestureFile(char *infilename)
{
	GestureFile gf;
	struct gpoint p[MAXPOINTS];
	char classname[100];
	char examplename[100];
	FILE * infile = fopen(infilename, "r");
	struct gesture *g = NULL;
	char line[100];
	int path, x, y, t;
	int i;

	gf = allocate(1, struct gesturefile);
	gf->ngestures = 0;

	if(infile == NULL)
		fprintf(stderr, "Can't open %s\n", infilename), exit(2);

	for(;;) {
		line[0] = '\0';
		fgets(line, 100, infile);
		switch(line[0]) {
		case '#':	continue;
		case 'x':
		case '\0':	/* EOF */
			if(g) {
				g->point = allocate(g->npoints, struct gpoint);
				for(i = 0; i < g->npoints; i++)
					g->point[i] = p[i];
			}

			if(line[0] == '\0') {
				fclose(infile);
				return gf;
			}

			sscanf(line, "%*s %s %s", classname, examplename);
			g = &gf->gesture[gf->ngestures++];
			g->classindex = ClassEnter(classname);
			g->examplename = recog_scopy(examplename);
			g->npoints = 0;
			g->y = NULL;
			break;

		default:
			if(sscanf(line, "%d %d %d %d", &path, &x, &y, &t) != 4)
			    fprintf(stderr, "fscanf1 %s\n", line), exit(2);
			if(g == NULL)
			    fprintf(stderr, "%s no gesture\n", line), exit(2);
			p[g->npoints].path = path;
			p[g->npoints].x = x;
			p[g->npoints].y = y;
			p[g->npoints].t = t;
			g->npoints++;
		}
	}
}

void
WriteGesture(FILE *outfile, Gesture g, char *classname)
{
	Gpoint p;
	int i;

	fprintf(outfile, "x %s %s\n", classname, g->examplename);
	for(i = 0; i < g->npoints; i++) {
		p = &g->point[i];
		fprintf(outfile,
			"%1d %3d %3d %4d\n",
			   p->path, p->x, p->y, p->t);
	}
	fflush(outfile);
}
