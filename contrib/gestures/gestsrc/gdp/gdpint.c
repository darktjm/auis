
/***********************************************************************

gdp - a gesture-based drawing program

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

/* gesture interface to sdp */

#include <gestures/gdev.h>
#define	Pointer _Pointer
#include <gestures/util.h>
#include <gestures/bool.h>
#include "set.h"
#include "object.h"
#include "state.h"
#include <gestures/matrix.h>
#include "transform.h"
#include "gdp.h"
#include <math.h>
#include <stdio.h>
#include "proto.h"

#define	C_LINE		'l'
#define	C_RECT		'r'
#define	C_CIRCLE	'e'
#define	C_TEXT		't'
#define	C_ROTATE_AND_SCALE	'R'
#define	C_ROTATE	'Z'
#define	C_UNDO		'u'

#define	C_MOVE		'm'
#define	C_COPY		'c'
#define	C_SELECT	's'
#define	C_STATS		'C'
#define	C_DELETE	'd'
#define	C_PACK		'p'

#define	C_REFRESH	''
#define	C_QUIT		'q'

#define	C_DEBUG		'x'

/* flags */


#define MODE		00
#define	IMMED		01

#define	NO_MODIFY	00
#define	MODIFY		02

typedef struct command *Command;

static void LinePointHandler(Command c, int xdown, int ydown);
static void RectPointHandler(Command c, int xdown, int ydown);
static void CirclePointHandler(Command c, int xdown, int ydown);
static void TextHandler(Command c, int xdown, int ydown);
static void MoveHandler(Command c, int xdown, int ydown);
static void RotateHandler(Command c, int xdown, int ydown);
static void RotateHandler2(Command c, int xdown, int ydown);
static void SelectHandler(Command c, int xdown, int ydown);
static void DeleteHandler(Command c, int xdown, int ydown);
static void PackHandler(Command c, int xdown, int ydown);
static void PackHandler2(Command c, int xdown, int ydown);
static void DpQuit(void);
static void DpDebug(void);

static struct command {
	int	command;
	const char	*name;
	int	flags;
	void	(*handler)(/* Command c, int xdown, int ydown */); /* tjm - need to do this, but too many (void)s and mostly harmless */
	int	arg;
}  commands[] = {
	{ C_LINE, "line",	MODE | MODIFY,	LinePointHandler, (int)Line },
	{ C_RECT, "rect",	MODE | MODIFY,	RectPointHandler, (int)Rect},
	{ C_CIRCLE, "circle",	MODE | MODIFY,	CirclePointHandler, (int)Circle },
	{ C_TEXT, "ascii",	MODE | MODIFY,	TextHandler, (int) Text },
	{ C_MOVE, "move",	MODE | MODIFY,	MoveHandler, },
	{ C_COPY, "copy",	MODE | MODIFY,	MoveHandler, },
	{ -1, "pack",	MODE | MODIFY,	PackHandler, },
	{ C_PACK, "pack",	MODE | MODIFY,	PackHandler2, },
	{ -1, "rotate",	MODE | MODIFY,	RotateHandler, },
	{ C_ROTATE, "rotate2",	MODE | MODIFY,	RotateHandler2, },
	{ -1, "rotate",	MODE | MODIFY,	RotateHandler, },
	{ C_ROTATE_AND_SCALE, "rotate",	MODE | MODIFY,	RotateHandler2, },
	{ C_SELECT, "select",	MODE | MODIFY,	SelectHandler, },
	{ C_DELETE, "delete",	MODE | MODIFY,	DeleteHandler, },
	{ C_REFRESH, "refresh",	IMMED | NO_MODIFY,	StRedraw, },
	{ C_STATS, "stats",	IMMED | NO_MODIFY,	CacheStats, },
	{ C_UNDO, "undo",	IMMED | NO_MODIFY,	StUndo, },
	{ C_QUIT,  "quit",	IMMED | NO_MODIFY,	DpQuit, },
	{ C_DEBUG,  "debug",	IMMED | NO_MODIFY,	DpDebug, },
};

#define	GESTURE_CHAR	002

static
void
DpInit(void)
{
	char rv[3];

	rv[0] = C_REFRESH; rv[1] = '\0';
	GDEVrefresh(rv);

	rv[0] = MOUSE_CHAR; rv[1] = LEFT_UP; rv[2] = '\0';
	GDEVmouse(MOUSE_EVENT(LEFT_BUTTON, UP_TRANSITION), rv);

	rv[0] = MOUSE_CHAR; rv[1] = RIGHT_UP; rv[2] = '\0';
	GDEVmouse(MOUSE_EVENT(RIGHT_BUTTON, UP_TRANSITION), rv);

	rv[0] = MOUSE_CHAR; rv[1] = LEFT_DOWN; rv[2] = '\0';
	GDEVmouse(MOUSE_EVENT(LEFT_BUTTON, DOWN_TRANSITION), rv);

	rv[0] = MOUSE_CHAR; rv[1] = RIGHT_DOWN; rv[2] = '\0';
	GDEVmouse(MOUSE_EVENT(RIGHT_BUTTON, DOWN_TRANSITION), rv);

	rv[0] = MOUSE_CHAR; rv[1] = LEFT_MOVE; rv[2] = '\0';
	GDEVmouse(MOUSE_EVENT(LEFT_BUTTON, DOWN_MOVEMENT), rv);

	rv[0] = MOUSE_CHAR; rv[1] = RIGHT_MOVE; rv[2] = '\0';
	GDEVmouse(MOUSE_EVENT(RIGHT_BUTTON, DOWN_MOVEMENT), rv);
}

static void
DpDebug(void)
{
	extern int DrawDebug;

	DrawDebug = !DrawDebug;
	printf("DrawDebug = %d\n", DrawDebug);
}

static void
DpQuit(void)
{
	(void) system("stty echo -cbreak");	/* too lazy to ioctl */
	exit(0);
}

static
Command
Lookup(int chr)
{
	Command c;
	for(c = commands;
	    c < &commands[sizeof(commands)/sizeof(commands[0])]; c++)
		if(c->command == chr)
			return c;
	return NULL;
}

static void
GetNextMouseEvent(int *whichp, int *xp, int *yp)
{
	int chr;

	DrawSync(); 
	while((chr = Ggetchar()) != MOUSE_CHAR)
		printf("%c ignored (typed while mouse down)\n", chr);
	
	*whichp = Ggetchar();
	GgetXYT(xp, yp, NULL);
}

static int
IsUpEvent(int which)
{
	return which == LEFT_UP || which == RIGHT_UP;
}

static void mainloop(void);

int main(int argc, char **argv)
{
	WmInit(argv[0]);
	StInit();
	DpInit();
	StBottomLine("GDP: a gesture based drawing program.  Copyright (C) 1991 Dean Rubine");
	WmFlush();
	GESTUREinit(argc, argv);
	GESTUREcharacter(GESTURE_CHAR);
	(void) system("stty -echo cbreak");	/* too lazy to ioctl */

	mainloop();
}

static void mainloop(void)
{
	Command command = &commands[0], c;
	int chr;
	int x, y;

	StBottomLine(command->name);
	for(;;) {
		DrawSync();
		if((chr = GESTUREgetchar()) == GESTURE_CHAR) {
			chr = GESTUREgetchar();
		}

		c = (chr == '.' ? command : Lookup(chr));
		if(c == NULL) {
			if(chr != '\n')
				printf("%c: Unknown command\n", chr);
			continue;
		}
		if(c->flags & IMMED) {
			if(c->flags & MODIFY)
				StCheckpoint();
			StBottomLine(c->name);
			(*c->handler)(c);
			StBottomLine(command->name);
		}
		else {
			command = c; 
			StBottomLine(command->name);
			GESTUREgetXYT(&x, &y, NULL);
			if(command->flags & MODIFY)
				StCheckpoint();
			(*command->handler)(command, x, y);
		}
	}
}

static void
LinePointHandler(Command c, int xdown, int ydown)
{
	int which, x, y;
	Element e;
	
	e = StNewObj((ObjectType) c->arg);

	StUpdatePoint(e, 0, xdown, ydown);
	do {
		GetNextMouseEvent(&which, &x, &y);
		StUpdatePoint(e, 1, x, y);
	} while( ! IsUpEvent(which));
}


static void
RectPointHandler(Command c, int xdown, int ydown)
{
	int which, x, y;
	Element e;
	
	e = StNewObj((ObjectType) c->arg);

	StUpdatePoint(e, 0, xdown, ydown);
	do {
		GetNextMouseEvent(&which, &x, &y);
		StUpdatePoint(e, 1, x, ydown);
		StUpdatePoint(e, 2, x, y);
	} while( ! IsUpEvent(which));
}

static void
CirclePointHandler(Command c, int xdown, int ydown)
{
	int which, x, y;
	Element e;
	
	e = StNewObj((ObjectType) c->arg);

	StUpdatePoint(e, 0, xdown, ydown);
	do {
		GetNextMouseEvent(&which, &x, &y);
		StUpdatePoint(e, 1, (int)sqrt((double)
			DistanceToPoint(x, y, xdown, ydown)), 0);
	} while( ! IsUpEvent(which));
}

static void
TextHandler(Command c, int xdown, int ydown)
{
}

static
Element
WaitForPick(/* in out */ int *xdownp, /* in out */ int *ydownp)
{
	int x, y, which;
	Element e;

	if((e = StPick(*xdownp, *ydownp)) != NULL)
		return e;

	/* printf("Pick something\n"); */
	for(;;) {
		GetNextMouseEvent(&which, &x, &y);
		if(IsUpEvent(which)) {
			/* printf("Didn't pick anything\n"); */
			return NULL;
		}
		if((e = StPick(x, y)) != NULL) {
			*xdownp = x;
			*ydownp = y;
			return e;
		}
	}
}

static void
MoveHandler(Command c, int xdown, int ydown)
{
	int x, y, which;
	Element e;
	
	if( (e = WaitForPick(&xdown, &ydown)) == NULL)
		return;

	if(c->command == C_MOVE) {
		printf("Move\n");
		e = StEdit(e, NULL);
	}
	else {
		printf("Copy\n");
		e = StCopyElement(e);
	}

	do {
		GetNextMouseEvent(&which, &x, &y);
		StMove(e, x - xdown, y - ydown);
		xdown = x; ydown = y;
	} while( ! IsUpEvent(which));
}

#define	SHRINK	1
#define	UNITY	2
#define	MAGNIFY	3

static int
GenRotate(Transformation t, int x1, int y1, int x2, int y2, int scalekind)
{
	int dx, dy;
	double ihyp = 1.0;

	dx = x2 - x1;
	dy = y2 - y1;
	if(dx == 0 && dy == 0)
		return 0;

	switch(scalekind) {
	case SHRINK:	ihyp = 1. / (dx*dx + dy*dy); 			break;
	case UNITY: 	ihyp = 1./sqrt((double) (dx*dx + dy*dy));	break;
	case MAGNIFY:  	ihyp = 1.0; 					break;
	}

	RotateCosSinTran(t, dx * ihyp, dy * ihyp);
	return 1;
}


static void
RotateHandler(Command c, int xdown, int ydown)
{
	int x, y, which;
	Element e;
	Object save;
	static Transformation t_to_origin, t_rotate, t_from_origin;
	static Transformation t_to_origin_and_undo_angle;
	static Transformation t_temp1, t_temp2;
	int xcenter = xdown, ycenter = ydown;
	int xstart, ystart;

	if(t_to_origin == NULL) {
		t_to_origin = AllocTran();
		t_rotate = AllocTran();
		t_from_origin = AllocTran();
		t_to_origin_and_undo_angle = AllocTran();
		t_temp1 = AllocTran();
		t_temp2 = AllocTran();
	}

	if( (e = WaitForPick(&xdown, &ydown)) == NULL) 
		return;

	printf("Rotate, release button, pick starting angle point\n");
	e = StEdit(e, &save);

	TranslateTran(t_to_origin, -xcenter, -ycenter);
	TranslateTran(t_from_origin, xcenter, ycenter);

	do
		GetNextMouseEvent(&which, &x, &y); 
	while( ! IsUpEvent(which));

	GetNextMouseEvent(&which, &xstart, &ystart);

	if( ! GenRotate(t_temp1, xcenter, ystart, xstart, ycenter,
				c->command == C_ROTATE ? UNITY : SHRINK) ) {
			printf("Can't pick center as rotate point\n");
			return;
	}
	(void)ComposeTran(t_to_origin_and_undo_angle, t_to_origin, t_temp1);

	do {
		GetNextMouseEvent(&which, &x, &y);
		if(GenRotate(t_rotate, xcenter, ycenter, x, y,
			c->command == C_ROTATE ? UNITY : MAGNIFY) ) {

			(void)ComposeTran(t_temp2, t_to_origin_and_undo_angle,
						t_rotate);
			(void)ComposeTran(t_temp1, t_temp2, t_from_origin);

			StReplaceObject(e, save);
			StTransform(e, t_temp1);
		}
	} while( ! IsUpEvent(which));
	FreeObject(save);
}



#define ROTATETHRESH	(8*8)

static void
RotateHandler2(Command c, int xdown, int ydown)
{
	int x, y, which;
	Element e;
	Object save;
	static Transformation t_to_origin, t_rotate, t_from_origin;
	static Transformation t_to_origin_and_undo_angle;
	static Transformation t_temp1, t_temp2;
	int xcenter = xdown, ycenter = ydown;
	int xstart, ystart;

	if(t_to_origin == NULL) {
		t_to_origin = AllocTran();
		t_rotate = AllocTran();
		t_from_origin = AllocTran();
		t_to_origin_and_undo_angle = AllocTran();
		t_temp1 = AllocTran();
		t_temp2 = AllocTran();
	}

	if( (e = WaitForPick(&xdown, &ydown)) == NULL) 
		return;

	printf("Rotate "), fflush(stdout);
/*
	printf("Rotate, release button, pick starting angle point\n");
	do
		GetNextMouseEvent(&which, &x, &y); 
	while( ! IsUpEvent(which));

	GetNextMouseEvent(&which, &xstart, &ystart);
*/

	do {
		GetNextMouseEvent(&which, &xstart, &ystart);
		if( IsUpEvent(which)) {
			printf("aborted\n");
			return;
		}
		x=xstart-xcenter; y = ystart - ycenter;
		printf("."), fflush(stdout);
	} while(x*x + y*y < ROTATETHRESH);

	printf("started\n");

	e = StEdit(e, &save);

	TranslateTran(t_to_origin, -xcenter, -ycenter);
	TranslateTran(t_from_origin, xcenter, ycenter);

	if( ! GenRotate(t_temp1, xcenter, ystart, xstart, ycenter,
				c->command == C_ROTATE ? UNITY : SHRINK) ) {
			printf("Can't pick center as rotate point\n");
			return;
	}
	(void)ComposeTran(t_to_origin_and_undo_angle, t_to_origin, t_temp1);

	do {
		GetNextMouseEvent(&which, &x, &y);
		if(GenRotate(t_rotate, xcenter, ycenter, x, y,
			c->command == C_ROTATE ? UNITY : MAGNIFY) ) {

			(void)ComposeTran(t_temp2, t_to_origin_and_undo_angle,
						t_rotate);
			(void)ComposeTran(t_temp1, t_temp2, t_from_origin);

			StReplaceObject(e, save);
			StTransform(e, t_temp1);
		}
	} while( ! IsUpEvent(which));
	FreeObject(save);
}


static void
SelectHandler(Command c, int xdown, int ydown)
{
}

static void
DeleteHandler(Command c, int xdown, int ydown)
{
	Element e;
	
	for(;;) {
		if( (e = WaitForPick(&xdown, &ydown)) == NULL)
			return;
		printf("Delete\n");
		StDelete(e);
	}
}

static void
PackHandler(Command c, int xdown, int ydown)
{
	Element newe, e;
	
	newe = StNewObj(SetOfObjects);
	for(;;) {
		if( (e = WaitForPick(&xdown, &ydown)) == NULL)
			break;
		if(e != newe) {
			printf("Added\n");
			StDelete(e); /* fishy since we use e after this */
			StAddSubObject(newe, e);
		}
		xdown = ydown = -1;
	}
}

static void
PackHandler2(Command c, int xdown, int ydown)
{
	Element newe, e;
	
	newe = StNewObj(SetOfObjects);
	for(;;) {
		if( (e = WaitForPick(&xdown, &ydown)) == NULL)
			break;
		if(e != newe) {
			printf("Added\n");
			StDelete(e); /* fishy since we use e after this */
			StHighlight(e, 1);
			StAddSubObject(newe, e);
			DrawSync();
		}
		xdown = ydown = -1;
	}
#undef FOUND_MY_NASTY_REDRAW_BUG
#ifdef FOUND_MY_NASTY_REDRAW_BUG
	StErase(newe);
	StHighlight(newe, 0);
	StDraw(newe);
#else
	StHighlight(newe, 0);
	StRedraw();
#endif
}
