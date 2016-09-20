/***********************************************************************

util.h - memory allocation, error reporting, and other mundane stuff

Copyright (C) 1991 Dean Rubine

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License. See ../COPYING for
the full agreement.

**********************************************************************/

/** \addtogroup librecog
 * @{ */

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
void PRINTF_LIKE(1,2) recog_debug(const char *a, ...);	/* printf on stderr -
			   setting DebugFlag = 0 turns off debugging */
void PRINTF_LIKE(1,2) EXIT_LIKE recog_error(const char *a, ...);	/* printf on stderr, then dies */
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

/** @} */

#endif
