
/***********************************************************************

mrecord - record example mouse gestures in files for subsuquent
	training or evaluation

Copyright (C) 1991 Dean Rubine

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (in ../COPYING); if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***********************************************************************/

/***********************************************************************
   (C) Copyright, 1990 by Dean Rubine, Carnegie Mellon University
    All Rights Reserved
 **********************************************************************/
#include <andrewos.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <gestures/gdev.h>
#include "region.h"

#define MAXPOINTS 10000

static FILE *outfile;
static const char *classname = "";

static int	Erase = 1;
static int	number = 1;
static int single = 0;

static int np;
static struct point {
	int	path, x, y, t;	
} p[MAXPOINTS];

static int tcmp(const void *p1, const void *p2) { return ((struct point *)p1)->t - ((struct point *)p2)->t; }

static void doit(void)
{
	int button;
	int i;
	int x, y, lx = -1, ly = -1;
	int path;
	int t, st;
	int cur, pnp;
	int done;

	printf("Click mouse (right button to end)\n");

top:
	np = 0;
	path = 0;
	st = -1;
	cur = 0; pnp = np;

	if(Erase)
		GDEVstart(), GDEVflush();

	for(;;) {
		done = 0;
		switch(GDEVgetchar()) {
		case MOUSE_CHAR:
			button = GDEVgetchar();
			GDEVgetXYT(&x, &y, &t);
			if(st == -1) {
				st = t;
				GDEVstart();
				GDEVtimeout(50, "T");
				cur = 0; pnp = np;
			}
			t -= st;
			p[np].path = path; p[np].t = t;
			p[np].x = x; p[np].y = y;
			np++;
			/*
			if(t == 0 && path != 0)
				np--; *//* ignore 1st point of all
						but 1st path */
			switch(button) {
			case RIGHT_DOWN:
				return;
			case LEFT_MOVE:
				if(lx != -1 && ly != -1)
					GDEVline(x, y, lx, ly);
				/* GDEVflush(); */
				lx = x; ly = y;
				break;
			case LEFT_DOWN:
				lx = x; ly = y;
				break;
			case LEFT_UP:
				path++;
				GDEVtimeout(0, NULL);
				st = -1;
				qsort(p, np, sizeof(p[0]), tcmp);
				/*
				printf("left up\n");
				for(i = 0; i < np; i++)
					printf("%4d %1d %3d,%3d\n",
					   p[i].t, p[i].path, p[i].x, p[i].y);
				*/
				done = 1;
				break;
			}
			/* fall into */
		case 'T':
			t = GDEVcurrenttime() - st;
			for(; cur < pnp && p[cur].t <= t; cur++) {
				for(i = cur-1; i >= 0; i--)
				    if(p[i].path == p[cur].path) {
					GDEVline(p[i].x, p[i].y,
						 p[cur].x, p[cur].y);
					break;
				}
			}
			GDEVflush();
			if(done && single) goto next;
			break;

		case 's':
			GDEVstart();
			GDEVtimeout(50, "T");
			cur = 0; pnp = np;
			st = GDEVcurrenttime();
			break;

		case 'n':
		next:
			printf("%d ", number), fflush(stdout);
			fprintf(outfile, "x %s %s%d\n", classname,
				classname, number++);
			for(i = 0; i < np; i++)
				fprintf(outfile,
					"%1d %3d %3d %4d\n",
					   p[i].path, p[i].x, p[i].y, p[i].t);
			fflush(outfile);
			/* fall into ... */

		case 'a':
			goto top;
		case 'q':
		case EOF:
			return;

		case 'p':
			GDEVstart();
			GDEVtimeout(50, "T");
			st = GDEVcurrenttime();
			for(cur = 0; cur < np; cur++) {
				while( (GDEVcurrenttime() - st) < p[cur].t)
					/* */;
				for(i = cur-1; i >= 0; i--)
				    if(p[i].path == p[cur].path) {
					GDEVline(p[i].x, p[i].y,
						 p[cur].x, p[cur].y);
					GDEVflush();
					break;
				}
			}
			st = -1;
			break;
			
		default:
			break;
		}
	}
}

static void init2(void)
{
	REGIONinit2();

	GDEVmenuitem("Record next file", "n");
	GDEVmenuitem("Record next path", "r");
	GDEVmenuitem("play back", "p");
	GDEVmenuitem("Record file again", "a");
	GDEVmenuitem("Quit", "q");
}

static void init1(void)
{
	REGIONinit();
}

/*-----------------------------------------------------------------*/


#include <gestures/args.h>

static int	verbose = 0;

static int	argc;
static char 	**argv;

static double	rho = 1.0;

static char *
fetcharg(int c)
{
	int i;
	char *r;

	for(i = 1; i < argc; i++) {
		if(strchr(argv[i], c)) {
			r = argv[i];
			--argc;
			for( ; i < argc; i++)
				argv[i] = argv[i+1];
			return r;
		}
	}
	return NULL;
}

int main(int ac, char **av)
{
	char *r;
	const char *outfilename = "gesture.in";

	argc = ac;
	argv = av;

	init1();
	GDEVsets("program", argv[0]);
	GDEVseti("thickness", 0);	/* make efficient lines */
	if((r = fetcharg('=')) != NULL)
		GDEVsets("Xgeometry", r);

	if((r = fetcharg('#')) != NULL) {
		char *p = strchr(r, '#');
		if(p != NULL) {
			*p++ = '\0';
			GDEVsets(r, p);
			printf("set %s=%s\n", r, p);
		}
	}


	ARGLOOP
		FLAGARG('E')	Erase = 1;		ENDFLAGARG
		FLAGARG('e')	Erase = 0;		ENDFLAGARG
		FLAGARG('G')	gdevdebug++;		ENDFLAGARG
		FLAGARG('v')	verbose++;		ENDFLAGARG
		FLAGARG('s')	single=1;		ENDFLAGARG
		STRINGARG('o')	outfilename = p;	ENDSTRINGARG
		STRINGARG('c')	classname = p;		ENDSTRINGARG
		STRINGARG('n')	number = atoi(p);	ENDSTRINGARG
		STRINGARG('i')	GDEVplay(p);		ENDSTRINGARG
		STRINGARG('p')	rho = atof(p);		ENDSTRINGARG
			
		BADARG
			fprintf(stderr, "unknown option %c\n", *p);
			fprintf(stderr, "Typical usage: mrecord -s -o file.train -c gesturename\n");
			exit(1);
		ENDBADARG
	ENDARGLOOP

	
	outfile = fopen(outfilename, "a");
	if(outfile == NULL)
		fprintf(stderr, "can't append to %s\n", outfilename), exit(2);

	init2();

	doit();
}


