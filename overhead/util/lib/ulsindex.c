/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

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
