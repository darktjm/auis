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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/mit/RCS/ez2ascii.C,v 1.3 1993/06/04 16:21:31 rr2b Stab74 $";
#endif


/* $Author: rr2b $ */
/*
 * Program to convert ATK datastream to plain ascii.
 *
 * Relies on overhead/mail/lib/unscribe code.
 * Uses same algorithm that sendmessages uses to strip
 * formatting and to break lines.
  *
  * Usage: ez2ascii [infile [outfile]]
  *
  * Outfile gets the stripped version of the file.
  * If outfile is not specified, output goes to stdout.
  * If no file is specified, stdin is read and output goes to stdout.
 */

#include <andrewos.h>
#include <stdio.h>
#include <sys/file.h>

#include <unscribe.h>

#ifndef NORCSID
#endif
static int ez2a_doit (FILE  *infile , FILE  *outfile);

static int ez2a_doit (FILE  *infile , FILE  *outfile)
{
    struct ScribeState *ussp;
    int version, err;
    char buf[1027];

    version = UnScribeInit(" 12", &ussp);

    while (fgets(buf,sizeof(buf)-1,infile)) {
	err = UnScribe(version, &ussp, buf, strlen(buf), outfile);
	if (err < 0) {
	    fprintf(stderr, "UnScribe error == %d.\n", err);
	    return -1;
	}
    }
    UnScribeFlush(version, &ussp, outfile);
    return 0;
}

int main(int  argc, char  *argv[])
{
    FILE *inptr, *outptr;
    int status = -1;

    if (argc > 3) {
	fprintf(stderr, "Too many arguments. First arg is input file; second is output file.\n");
    } else if (argc == 1) {
	status = ez2a_doit (stdin, stdout);
    } else if(argc == 3) {
	if ((outptr = fopen(argv[2], "w")) == NULL) {
	    fprintf(stderr, "Could not open output file '%s'.\n", argv[2]);
	    exit(-1);
	}
	if ((inptr = fopen((argv[1]), "r")) == NULL) {
	    fprintf(stderr,	"Could not open	input file '%s'.\n", argv[1]);
	    exit (-1);
	}
	status = ez2a_doit (inptr, outptr);
    } else {
	if ((inptr = fopen((argv[1]), "r")) == NULL) {
	    fprintf(stderr,	"Could not open	input file '%s'.\n", argv[1]);
	    exit (-1);
	}
	status = ez2a_doit (inptr, stdout);
    }
    exit (status);
}
