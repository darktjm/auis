/***********************************************************************

util.h - memory allocation, error reporting, and other mundane stuff

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


/*
 * General utility functionss
 *
 * Mostly for dealing with mundane issues such as:
 *	Memory allocation
 *	Error handling
 */

/*
 * General allocation macro
 *
 * Example:
 *	struct list *s; s = allocate(4, struct list)
 * returns space for an array of 4 list structures.
 * Allocate will die if there is no more memory
 */
#include <andrewos.h>

#define	allocate(n, type)	\
	((type *) recog_myalloc(n, sizeof(type), Stringize(type)))

/*
 * Functions
 */

#define	STREQ(a,b)	( ! strcmp(a,b) )

void	*recog_myalloc(int nitems, int itemsize, const char *typename);	/* Do not call this function directly */
char *recog_scopy(const char *s);
#ifdef __GNUC__
__attribute__((format(printf,1,2)))
#endif
void	recog_debug(const char *a, ...);	/* printf on stderr -
			   setting DebugFlag = 0 turns off debugging */
#ifdef __GNUC__
__attribute__((format(printf,1,2),noreturn))
#endif
void	recog_error(const char *a, ...);	/* printf on stderr, then dies */
#if 0
int	ucstrcmp();	/* strcmp, upper case = lower case */
#endif
char	*recog_tempstring();	/* returns a pointer to space that will reused soon */

/*
   this is the wrong place for all of this, but got chosen since
   every file includes this one
 */

#ifdef _IBMR2
/* IBM doesn't define unix? */
#define unix
#endif

#ifdef unix
#	define GRAPHICS		/* only GDEV on unix machines */
#endif

#ifndef unix

/* various BSD to lattice C name changes */

#define	bcopy	movmem
#define index	strchr
#define	rindex	strrchr

#endif
