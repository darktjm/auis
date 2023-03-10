/***********************************************************************

gtest.c - simple test program for GDEV

Copyright (C) 1991 Dean Rubine

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License. See ../COPYING for
the full agreement.

**********************************************************************/

/*
 test out GDEV
 */
#include <andrewos.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "gdev.h"
#include "args.h"

static	int	NoWait = 0;
static	char  	*Tests = NULL;

static int	argc;
static char 	**argv;

static void input_test(void);
static void output_test(void);
static void menu_test(void);
static void draw_test(void);

static void DoTest(int c, void (*test)(void));
static char *fetcharg(int c);
static void init(void);
static void init_mouse(void);
static void Wait(void);

int main(int ac, char **av)
{
	argc = ac;
	argv = av;

	init();
	DoTest('m', menu_test);
	DoTest('o', output_test);
	DoTest('i', input_test);
	DoTest('d', draw_test);
	GDEVstop();
	return 0;
}

static void DoTest(int c, void (*test)(void))
{
	if(Tests == NULL || strchr(Tests, c)) {
		(*test)();
		GDEVflush();
		if(! NoWait)
			Wait();
	}
}

#define	MOUSE_CHAR	001

#define    LEFT_DOWN			001
#define    LEFT_MOVE		  	002
#define    LEFT_UP		  	003

#define    RIGHT_DOWN			011
#define    RIGHT_MOVE		  	012
#define    RIGHT_UP		  	013

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

static void init(void)
{
	char *r;

	GDEVinit(NULL);
	GDEVsets("program", argv[0]);
	if((r = fetcharg('=')) != NULL)
		GDEVsets("Xgeometry", r);

	while((r = fetcharg('#')) != NULL) {
		char *p = strchr(r, '#');
		if(p != NULL) {
			*p++ = '\0';
			if(isdigit(*p)) {
				GDEVseti(r, atoi(p));
				printf("seti %s=%d\n", r, atoi(p));
			}
			else {
				GDEVsets(r, p);
				printf("sets %s=%s\n", r, p);
			}
		}
	}


	ARGLOOP
		FLAGARG('G')	gdevdebug++;		ENDFLAGARG
		FLAGARG('w')	NoWait++;		ENDFLAGARG
		STRINGARG('t')	Tests = p;		ENDSTRINGARG
		STRINGARG('o')	GDEVrecord(p);		ENDSTRINGARG
		STRINGARG('i')	GDEVplay(p);		ENDSTRINGARG
			
		BADARG
			fprintf(stderr, "unknown option %c\n", *p);
			exit(1);
		ENDBADARG
	ENDARGLOOP

	init_mouse();
}

static void init_mouse(void)
{
	char rv[3];

	GDEVstart();

	rv[0] = MOUSE_CHAR; rv[1] = LEFT_DOWN; rv[2] = '\0';
	GDEVmouse(MOUSE_EVENT(LEFT_BUTTON, DOWN_TRANSITION), rv);

	rv[0] = MOUSE_CHAR; rv[1] = LEFT_MOVE; rv[2] = '\0';
	GDEVmouse(MOUSE_EVENT(LEFT_BUTTON, DOWN_MOVEMENT), rv);

	rv[0] = MOUSE_CHAR; rv[1] = LEFT_UP; rv[2] = '\0';
	GDEVmouse(MOUSE_EVENT(LEFT_BUTTON, UP_TRANSITION), rv);

	rv[0] = MOUSE_CHAR; rv[1] = RIGHT_DOWN; rv[2] = '\0';
	GDEVmouse(MOUSE_EVENT(RIGHT_BUTTON, DOWN_TRANSITION), rv);

	rv[0] = MOUSE_CHAR; rv[1] = RIGHT_MOVE; rv[2] = '\0';
	GDEVmouse(MOUSE_EVENT(RIGHT_BUTTON, DOWN_MOVEMENT), rv);

	rv[0] = MOUSE_CHAR; rv[1] = RIGHT_UP; rv[2] = '\0';
	GDEVmouse(MOUSE_EVENT(RIGHT_BUTTON, UP_TRANSITION), rv);

	GDEVflush();

}

static void Wait(void)
{
	int c;
	int button;
	int x, y;

	printf("click mouse or hit return to continue\n");
	for(;;) switch(c = GDEVgetchar()) {
	case MOUSE_CHAR:
		button = GDEVgetchar();
		GDEVgetXYT(&x, &y, NULL);
		switch(button) {
		case RIGHT_UP:
		case LEFT_UP:
			return;
		}
		break;

	case 'q':
	case EOF:
		exit(0);

	case '\n':
		return;


	default:
		printf("Got %c\n", c);
	}
}

/* ------------------- tests ------------------------ */


static void output_test(void)
{
	int width, height;
	int xincr, yincr;
	int i;
	char buf[100];
	int xshim, yshim;

	GDEVstart();

	GDEVseti("thickness", 1);
	GDEVgetdim(&width, &height);
	printf("-------- Output Test ---------\n");
	printf("width = %d  height = %d\n", width, height);

	xincr = width / 10; width = xincr * 10; xshim = xincr/10;
	yincr = height / 10; height = yincr * 10; yshim = yincr/10;

	for(i = 0; i <= 10; i++) {
		GDEVline(i*xincr, 0, i*xincr, height);
		GDEVline(0, i*yincr, width, i*yincr);
	}

	GDEVtext(0,0, "0,0");
	sprintf(buf, "%d,%d", 5*xincr, 5*yincr);
	GDEVtext(5*xincr,5*yincr, buf);

	/* thickness test */
	for(i = 1; i < 10; i++) {
		sprintf(buf, "%d", i);
		GDEVtext(i*xincr - xshim, yincr - yshim, buf);
		GDEVseti("thickness", i);
		GDEVline(i*xincr, yincr,
			(int) ((i+.75)*xincr), (int) (yincr*1.75) );
	}
	GDEVseti("thickness", 1);
}

static void input_test(void)
{
	printf("-------- Input Test ---------\n");
}


static void menu_test(void)
{
	printf("-------- Menu Test ---------\n");

	GDEVmenuitem("Item 1", "1");
	GDEVmenuitem("Item 2", "2");
	GDEVmenuitem("Quit", "q");
}

static void draw_test(void)
{
	int x, y;
	int lx = -1, ly = -1;
	int button;
	int c;

	printf("-------- Draw Test ---------\n");

	GDEVstart(); GDEVflush();
	for(;;) {
		switch(c = GDEVgetchar()) {
		case MOUSE_CHAR:
			button = GDEVgetchar();
			GDEVgetXYT(&x, &y, NULL);
			switch(button) {
			case RIGHT_DOWN:
				return;
			case LEFT_MOVE:
				if(lx != -1 && ly != -1)
					GDEVline(x, y, lx, ly);
				GDEVflush();
				/* fall into .. */
			case LEFT_DOWN:
				lx = x; ly = y;
				break;
			}
			break;

		case 'q':
		case EOF:
			GDEVstoprecording();
			return;
			
		default:
			printf("got %c\n", c);
			break;
		}

	}
}

/*-----------------------------------------------------------------*/

