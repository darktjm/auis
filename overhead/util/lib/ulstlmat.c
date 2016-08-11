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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/util/lib/RCS/ulstlmat.c,v 2.7 1992/12/15 21:10:49 rr2b Stab74 $";
#endif

/*
	ulstlmat.c--match leftmost part of string, ignoring alphabetic case.
*/


 

int ULstlmatch (big,small)
char *small, *big;
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
    register char *s, *b, sc, bc;
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
