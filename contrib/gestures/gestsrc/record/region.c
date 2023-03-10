/***********************************************************************

region.c - kaka for mrecord to work

Copyright (C) 1991 Dean Rubine

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License. See ../COPYING for
the full agreement.

**********************************************************************/

/*
 region.c - manipulates a GDEV window
 */
#include <andrewos.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <gestures/gdev.h>
#include "region.h"

void
REGIONinit(void)
{

	GDEVinit(NULL);
	GDEVsets("program", "gsink");
}

#if 0
/* char	*template = "out.%d.rec"; */
static char	*template = NULL;
static int	current = 1;

static void
setrecordfile(char *filename)
{
	char *p, *endp;
	int i;
	static char fname[100];

	strcpy(fname, filename);

	current = 1;
	for(p = &filename[strlen(filename) - 1]; p >= filename; p--)
		if(isdigit(*p)) {
			for(endp = p ; isdigit(*endp); endp--)
				/* */ ;
			endp++;
			current = atoi(endp);
			for(i = 0; &filename[i] < endp; i++)
				fname[i] = filename[i];
			fname[i++] = '%';
			fname[i++] = 'd';
			for(++p; *p != '\0'; p++, i++)
				fname[i] = *p;
			fname[i] = '\0';
			break;
		}

 	if(strchr(fname, '%') == NULL)
		printf("Warning: File name \"%s\" does not contain a number or %%d\n",
			filename);

	template = fname;
}
#endif

void
REGIONinit2(void)
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
