/***********************************************************************

gdev.h - interface to a window system independent graphics library

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

/** \addtogroup libgdev gestures/libgdev
 * Device-independent grpahics interface for use with gesture recognizer.
 * @{ */
/*
 * gdev.h - header for device independent graphics interface.
 *
 * Applications that want device independent input/output should use the
 * include the file, and use the functions declared herein.  For an
 * application to be independent of window managers, it must never make
 * a call to a specific window manager function, but instead use the
 * corresponding call described here.
 *
 * gdev reads the GDEV environment variable to get the kind of device desired.
 * Devices currently supported are:
 *	X11
 */

#ifndef _GDEV_
#define _GDEV_

/* generic address of variables and functions */

typedef void *Pointer;

/* Functions */

/* initialization, flushing, and exiting */

int GDEVinit(char *device);	/* Opens a new window, making
				   it the current output window.
				   Call this function before
				   any of the others.  Uses GDEV env var,
				   or passed device, if GDEV is null 
				   returns -1 on error, >=0 if all went OK
				   The return value refers to the newly created
				   window, and is useful as an argument to
				   GDEVcurrentoutput();
				   */

void GDEVcurrentoutput(int w);	/* make window the current output
				   window - subsequent calls to other GDEV
				   functions refer to this window */

void GDEVstart(void);		/* Clears the current window, starts a new page */
void GDEVflush(void);		/* outputs current page of current window */
void GDEVstop(void);		/* Cleanup before exiting program */

void GDEV00topleft(void);  	/* normally GDEV believes that the window
				   is a cartesian plane with 0,0 at the bottom
				   left.  Calling this function makes 0,0
				   the top left of the screen */


/* displaying graphics and text */


void GDEVpoint(int x, int y);
void GDEVline(int x1, int y1, int x2, int y2);
void GDEVrect(int x1, int y1, int x2, int y2);
void GDEVtext(int x1, int y1, const char *str);

void GDEVsetdim(int x, int y);	/* - sets dimentions, but it may
				be ignored */
void GDEVgetdim(int *x, int *y);	/* returns screen size */

/* input */

int GDEVgetchar(void);	/* returns the next input character.  Normally
			   this is the next character typed at the keyboard,
			   but it may also be part of a menu string,
			   mouse string, or redisplay request.  Returns EOF
			   on EOF */

char *GDEVgets(char *line);	/*
			   returns the next line of input.  Menu strings,
			   mouse strings, and redisplay requests will not
			   be returned until the next GDEVgetchar call */

/* menus */

/*
   Example:
	GDEVmenuitem("print", "\001\001");
	GDEVmenuitem("format", "\001\002");

   When the user asks to see a menu, he sees "print" and "format".  If
he chooses "format", the next call to GDEVgetchar() will return ^A, and the
next call ^B (i.e. \001 then \002).

   To delete an item from the menu, call, e.g.
	GDEVmenuitem("print", NULL);
*/

void GDEVmenuitem(const char *label, const char *retval);


/* mouse */

/*
   mouse events occur when a button is pushed, released, or the mouse is
   moved while a button is held down.  An application will only get informed
   of events it cares about.  If an application calls
	GDEVmouse(MOUSE_EVENT(LEFT_BUTTON, DOWN_MOVEMENT), "\004\001")
   it will receive (via GDEVgetchar()) the characters ^D (004) then ^A (001).
   It should then call GDEVgetXYT() to return the mouse
   position.

	Call GDEVmouse(MOUSE_EVENT(...), NULL) if you're no longer interested
	in the event.
 */


#define	BUTTON_MASK	0xf

#define	LEFT_BUTTON	0x1
#define	MIDDLE_BUTTON	0x2
#define RIGHT_BUTTON	0x4

#define ACTION_MASK	0xf0

#define	DOWN_TRANSITION	0x10
#define	UP_TRANSITION	0x20
#define	DOWN_MOVEMENT	0x40

#define	MOUSE_EVENT(button, action)	((button) | (action))

void GDEVmouse(int event, char *retval);
void GDEVgetXYT(int *xp, int *yp, int *timep); /*
				   Any of these can be NULL if the application
				   is not interested in them */

void GDEVwindowchanged(char *retval); /*
			    Whenever input comes in from a window it did not
			    previously come in from, this string is returned,
			    followed by a single character indicated which
			    window  */

void GDEVrefresh(char *retval); /*
			   When, for whatever reason, the screen needs to be
			   redrawn, GDEVgetchar() will return the passed 
			   string.  Call GDEVrefresh(NULL) if you're not
			   interested in refresh events, though this is
			   probably a bad idea. 
			   The first GDEVstart is always guarenteed to generate
			   a refresh, so the GDEVrefresh should occur before
			   the first GDEVstart.
			   */

void GDEVrefresh_function(int (*fcn)(void)); /*
			   Arranges for function to be called whenever
			   the screen needs to be refreshed.  This is
			   independent of the characters returned by
			   GDEVrefresh */

void GDEVtimeout(int msec, const char *retval); /*
			    Arranges for "retval" to be returned (by IOgetchar)
			    if no input (from select time) arrives for more
			    than msec milliseconds.  Use NULL for retval
			    to restore IOgetchar to its blocking behavior
				*/

int GDEVseti(const char *variable, int value);
int GDEVsetd(const char *variable, double value);
int GDEVsets(const char *variable, const char *value);
			/* This routine is for device specific functions.
			   Each device will recognize some variables used to
			   control various things - other devices will ignore
			   the GDEVset? call */


/*
 * GDEV has facilities for recording input onto a file (similar in spirit to a
 * keystroke file, but includes mouse and menu events) and for playing back a
 * recording.
 */


int GDEVrecord(char *file); /*
				starts recording, returning 0 iff file couldnt
				be opened */
void GDEVstoprecording(void);	/* stops the recording, closing and flusing
				   the output file.  The output file will
				   be automatically closed and flushed upon
				   exit */


int GDEVplay(char *file); /*
				plays back a file, return 0 iff it couldn't
				be opened.  GDEVgetchar (and GDEVgets) will
				return the same string of characters as they
				did before */

int GDEVcurrenttime(void);

extern int gdevdebug;

/** @} */
#endif /* _GDEV_ */
