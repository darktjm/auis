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
	lcstring.c-- copies a source string to a dest buff, converting all uppercase to lower.
*/


 

#include <ctype.h>

#define DOWNCASE(x) (isascii(x) && isalpha(x) && isupper(x) ? (tolower(x)) : (x))
/* 
 * Just like strncpy but shift-case in transit and forces null termination.  
 * Copied 8/24/89 from afs/util/casestrcpy.c to allow omission of 
 * lib/afs/util.a; NOTE: on the sun4c version of AFS we need 
 * lib/afs/util.a so this function is going into its very own module.
 * (4/4/91)
*/

char *lcstring (d, s, n)
  char *d;				/* dest string */
  char *s;				/* source string */
  int   n;				/* max transfer size */
{   char *original_d = d;
    char  c;

    if ((s == 0) || (d == 0)) return 0;	/* just to be safe */
    while (n) {
	c = *s++;
	c = DOWNCASE(c);
	*d++ = c;
	if (c == 0) break;		/* quit after transferring null */
	if (--n == 0) *(d-1) = 0;	/* make sure null terminated */
    }
    return original_d;
}

