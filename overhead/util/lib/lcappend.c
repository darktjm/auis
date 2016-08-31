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
  lcappend.c

  LCappend(s1, s2): append a lower-alpha-case copy of s2 to s1.
      */

#include <util.h>
#include <ctype.h>

void LCappend(char *s1, const char *s2)
{
    char *e1;

    for (e1 = &s1[strlen(s1)]; *s2 != '\0'; ++e1) {
	*e1 = *s2++;
	if (isupper(*e1)) *e1 = tolower(*e1);
    }
    *e1 = '\0';
}
#ifdef TESTINGONLYTESTING
#include <stdio.h>
main()
{
    char a[100];
    strcpy(a, "This is nice: ``");
    LCappend(a, "Andrew.CMU.EDU");
    strcat(a, "''.\n");
    fputs(a, stdout);
}
#endif /* TESTINGONLYTESTING */
