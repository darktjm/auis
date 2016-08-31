/***********************************************************************

util.c - memory allocation, error reporting, and other mundane stuff

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
 * Mundane utility routines
 *	see util.h
 */

/*LINTLIBRARY*/
#include <andrewos.h>
#include "util.h"
#include <stdio.h>
#include <ctype.h>

/*
 * Function used by allocation macro
 */

char *
recog_myalloc(nitems, itemsize, typename)
char *typename;
{
	register unsigned int bytes = nitems * itemsize;
	register char *p = malloc(bytes);
	if(p == NULL)
	     recog_error("Can't get mem for %d %s's (each %d bytes, %d total bytes)",
		nitems, typename, itemsize, bytes);
	return p;
}

/*
 * Return a copy of a string
 */

char *
recog_scopy(s)
char *s;
{
	register char *p = allocate(strlen(s) + 1, char);
	(void) strcpy(p, s);
	return p;
}

/*
 * Print an error message, a newline, and then exit
 */

/*VARARGS1*/
void
recog_error(a, b, c, d, e, f, g)
char *a;
{
	fprintf(stderr, a, b, c, d, e, f, g);
	fprintf(stderr, "\n");
	exit(1);
}

/*
 * print a message if DebugFlag is non-zero
 */

int	DebugFlag = 1;

void
recog_debug(a, b, c, d, e, f, g)
char *a;
{
	if(DebugFlag)
		fprintf(stderr, a, b, c, d, e, f, g);
}

#define	upper(c)	(islower(c) ? toupper(c) : (c))

#if 0
int
ucstrcmp(s1, s2)
register char *s1, *s2;
{
	register i;

	for(; *s1 && *s2; s1++, s2++)
		if( (i = (upper(*s1) - upper(*s2))) != 0)
			return i;
	return (upper(*s1) - upper(*s2));
}
#endif

#define NSTRINGS 3

char *
recog_tempstring()
{
	static char strings[NSTRINGS][100];
	static int index;
	if(index >= NSTRINGS) index = 0;
	return strings[index++];
}
