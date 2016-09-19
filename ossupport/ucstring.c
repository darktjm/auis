/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
	ucstring.c-- copies a source string to a dest buff, converting all lowercase to upper.
*/

#include <ctype.h>

#define UPCASE(x) (isascii(x) && isalpha(x) && isupper(x) ? (x) : (toupper(x)))

/* 
 * Just like strncpy but shift-case in transit and forces null termination.  
 * Copied 8/24/89 from afs/util/casestrcpy.c to allow omission of 
 * lib/afs/util.a; NOTE: on the sun4c version of AFS we need 
 * lib/afs/util.a so this function is going into its very own module.
 * (4/4/91)
*/

char *ucstring (d, s, n)
  char *d;				/* dest string */
  char *s;				/* source string */
  int   n;				/* max transfer size */
{   char *original_d = d;
    char  c;

    if ((s == 0) || (d == 0)) return 0;	/* just to be safe */
    while (n) {
	c = *s++;
	c = UPCASE(c);
	*d++ = c;
	if (c == 0) break;		/* quit after transferring null */
	if (--n == 0) *(d-1) = 0;	/* make sure null terminated */
    }
    return original_d;
}
