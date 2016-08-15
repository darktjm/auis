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

#include <util.h>

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/util/lib/RCS/foldedeq.c,v 2.8 1994/10/26 15:57:14 Zarf Stab74 $";
#endif


 

const int FoldTRT[256] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, '!', '"', '#', '$', '%',
    '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?', '@', 'a', 'b', 'c',
    'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '[', '\\', ']', '^', '_', '`', 'a',
    'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
    'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', 127,
    128, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, '!', '"', '#', '$', '%',
    '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?', '@', 'a', 'b', 'c',
    'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '[', '\\', ']', '^', '_', '`', 'a',
    'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
    'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', 127,
};

/*
  if (s1 < s2) lexicographically, return a value less than 0;
  if (s1 == s2) return 0;
  if (s1 > s2) return a value greater than 0;

  High bits are disregarded EXCEPT for character 128.
  */

int lc_strcmp(const char *_s1, const char *_s2)
{
    const unsigned char *s1 = (const unsigned char *)_s1,
	                *s2 = (const unsigned char *)_s2;
    while (*s1 && (FoldTRT[*s1] == FoldTRT[*s2])) {
	++s1;
	++s2;
    }

    return FoldTRT[*s1] - FoldTRT[*s2];
}

int lc_strncmp(const char *_s1, const char *_s2, int n)
{
    const unsigned char *s1 = (const unsigned char *)_s1,
	                *s2 = (const unsigned char *)_s2;
    while (n && *s1 && (FoldTRT[*s1] == FoldTRT[*s2])) {
	--n;
	++s1;
	++s2;
    }

    return n == 0 ? 0 : FoldTRT[*s1] - FoldTRT[*s2];
}

int FoldedEQ(const char *s1, const char *s2)
{
    return (lc_strcmp(s1, s2) == 0);
}

int FoldedEQn(const char *s1, const char *s2, int n)
{
    return (lc_strncmp(s1, s2, n) == 0);
}

#define	ArbChar	'*'	/* Arbitrary string valid here */
#define	AnyChar	'?'	/* Any one character valid here */
#define	EscChar	'\\'	/* Next character taken as literal */

#define	False	0
#define	True	1

int FoldedWildEQ(const char *string, const char *template, int ignorecase)
{
    const char *oldPtr;   /* R4 Old template pointer */
    int arbActive, escActive, mustInc, matched;

    template--;		/* Move filter back one, so can inc */
    oldPtr = 0;		/* No old template pointer */
    arbActive =	False;	/* Arb match not active */
    escActive =	False;	/* Esc char not active */
    mustInc = True;	/* Should inc template */

    while (1) {			    /* Continue until something breaks out */

	if (mustInc) {		    /* If template ptr should be inc'd */
	    template++;
	    mustInc = False;
	    arbActive = False;
	    if (*template == ArbChar) {	    /* Handle start of arb char */
		arbActive = True;
		while (*(++template) == ArbChar);
	    }
	    if (*template == EscChar) {	    /* Handle escape char */
		template++;
		escActive = True;
	    }
	    else
		escActive = False;
	    if (!*template) {		/* End of template, cannot stay there */
		if (arbActive || !*string)  /* Arb active or end of string */
		    return True;		/* Return successful match */
		else if	(!oldPtr)	    /* If no old ptr available */
		    return False;		/* Return where match failed */
		else {			    /* Must back up in template */
		    arbActive =	True;		/* Back in arb mode */
		    template = oldPtr;		/* Reset template to old value */
		    oldPtr = 0;			/* No more old value */
		}
	    }
	}

	if (!*string) return False;	    /* At end of string, with no match */

	if (ignorecase)
	    matched = (FoldTRT[(unsigned char)*string] == FoldTRT[(unsigned char)*template] || (*template == AnyChar && !escActive));
	else
	    matched = (*string == *template || (*template == AnyChar && !escActive));

	if (matched && arbActive) {	    /* Match and Arb */
	    oldPtr = template;
	    arbActive = False;
	    mustInc = True;
	}
	else if	(matched)		    /* Match and Not Arb */
	    mustInc = True;
	else if	(arbActive);		    /* Not Match and Arb */
	else if	(oldPtr) {		    /* Not Match and old pntr avail */
	    arbActive =	True;			/* Back in arb mode */
	    template = oldPtr;			/* Reset template to old value */
	    oldPtr = 0;				/* No more old value */
	}
	else				    /* No valid match up, so give up */
	    return False;

	string++;			    /* Move to next position in string */
    }
}
