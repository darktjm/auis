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

#include <andrewos.h>

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/mail/cmd/RCS/arpadate.c,v 1.8 1992/12/15 21:04:17 rr2b Stab74 $";
#endif

/*
		arpadate.c -- Shell command to print current time in
			      ARPAnet format
*/

#include <stdio.h>
#include <ctype.h>

extern char *arpadate();

int main(argc, argv)
char *argv[];
{
    int AnyArgs, ac;
    unsigned long int Then;
    static char Usage[] = "[-n] [ctime1 ctime2 ...]";

    AnyArgs = 0;
    for (ac = 1; ac < argc; ++ac) {
	if (argv[ac][0] == '-') {
	    if (strcmp(argv[ac], "-n") == 0) {
		printf("%lu\n", time(0));
		AnyArgs = 1;
	    } else {
		fprintf(stderr, "Unknown option ``%s''\nusage: %s %s\n", argv[ac], argv[0], Usage);
		exit(1);
	    }
	} else if (isdigit(argv[ac][0])) {
	    Then = atol(argv[ac]);
	    printf("ctime(%lu)=%s", Then, ctime(&Then));
	    AnyArgs = 1;
	} else {
	    fprintf(stderr, "Unknown argument ``%s''\nusage: %s %s\n", argv[ac], argv[0], Usage);
	    exit(1);
	}
    }
    if (AnyArgs == 0) fputs(arpadate(), stdout);
    return 0;
}
