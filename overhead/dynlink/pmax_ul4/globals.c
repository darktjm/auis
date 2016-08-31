/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

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
 * array of global entry points for dynamic loader
 *
 * Defines a table of entry points and their names
 *
 */

/* macros which actually define entry points */

/*   globalref - direct reference to global symbol x */

#define globalref(x) extern long x;
#define routine(x) extern void x();

/* common symbols referred to but not defined directly in libc.a */

	globalref(environ)		/* common symbol, defined nowhere */
	globalref(errno)		/* cerror */

/* do not delete the following line - make depends on it */

#include	<globalrefs._h>

#undef globalref
#undef routine

#if !__STDC__	  
#define globalref(x) (long) &x, "x",
#define routine(x) (long) x, "x",
#else
#define globalref(x) (long)&x,STRINGIZE(x)
#define routine(x) (long) x,STRINGIZE(x)
#endif
	  
struct globaltab {
    long entrypoint;	/* entry point value */
    char *entryname;	/* symbolic name */
} globals[] = {
/* common symbols referred to but not defined directly in libc.a */

	globalref(environ)		/* common symbol, defined nowhere */
	globalref(errno)		/* cerror */

/* do not delete the following line - make depends on it */

#include	<globalrefs._h>
};

long globalcount = sizeof(globals) / sizeof(struct globaltab);

