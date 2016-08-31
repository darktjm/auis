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
 * This package contains more C library routines either 1.missing from those
 * supplied with the MSC/IBM compilers, or 2. included in libmail or libitc.
 *
 *  Andrew Version
 *
 */

#include <andrewos.h>
#include <ctype.h>
#include <util.h>
ULsubstr(s1, s2)
char *s1, *s2;
{
    while (*s2 && *s1 &&
	   ((isupper(*s1) ? tolower(*s1) : *s1) ==
	    (isupper(*s2) ? tolower(*s2) : *s2))) {
	++s1; ++s2;
    }
    if (!*s2) return(0);
    return((isupper(*s1) ? tolower(*s1) : *s1) - (isupper(*s2) ? tolower(*s2) : *s2));
}

char *memcpy_preserve_overlap(dest, source, len)
char *dest, *source;
int len;
{
#ifndef sun
    bcopy(source, dest, len);
#else /* sun */
    memcpy(dest, source, len);
#endif /* sun */

    return dest;
}

