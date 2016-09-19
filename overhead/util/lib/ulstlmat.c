/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
	ulstlmat.c--match leftmost part of string, ignoring alphabetic case.
*/

#include <util.h>

int ULstlmatch (const char *big,const char *small)
{
/*  stlmatch  --  match leftmost part of string
 *
 *  Usage:  i = stlmatch (big,small)
 *	int i;
 *	char *small, *big;
 *
 *  Returns 1 iff initial characters of big match small exactly, ignoring alphabetic case.
 *  else 0.
 */
    const char *s, *b;
    char sc, bc;
    s = small;
    b = big;
    do {
	if (*s == '\0')  return (1);
	sc = *s++; if (sc <= 'Z') if (sc >= 'A') sc += ('a' - 'A');
	bc = *b++; if (bc <= 'Z') if (bc >= 'A') bc += ('a' - 'A');
    } 
    while (sc == bc);
    return (0);
}
