/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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
	ulsindex.c--find index of one string within another, ignoring alphabetic case.
*/

#include <util.h>
 

char *ULsindex(const char *big, const char *small)
/*  ULsindex  --  find index of one string within another, ignoring alphabetic case.
 *
 *  Usage:  p = ULsindex (big,small)
 *	char *p,*big,*small;
 *
 *  ULsindex searches for a substring of big which matches small,
 *  and returns a pointer to this substr.  If no matching
 *  substring is found, 0 is returned.
 *
 */
{
    const char *bp, *bp1, *sp;
    char bc, sc, c = *small;

    if (c==0) return(0);
    if (c <= 'Z') if (c >= 'A') c += ('a' - 'A');
    for (bp=big;  *bp;  bp++) {
	bc = *bp; if (bc <= 'Z') if (bc >= 'A') bc += ('a' - 'A');
	if (bc == c) {
	    sp = small;
	    bp1 = bp;
	    do {
		sc = *++sp; if (sc == '\0') {
		    return(char *)(bp);
		}
		if (sc <= 'Z') if (sc >= 'A') sc += ('a' - 'A');
		bc = *++bp1; if (bc <= 'Z') if (bc >= 'A') bc += ('a' - 'A');
	    } while (sc == bc);
	}
    }
    return 0;
}
