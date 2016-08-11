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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/util/lib/RCS/encode.c,v 1.6 1995/03/18 17:30:55 rr2b Stab74 $";
#endif


 

#include <andrewos.h>
#include <stdio.h>
#include <ctype.h>

static char basis_64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

to64(infile, outfile) 
FILE *infile, *outfile;
{
    int c1, c2, c3, ct=0;
    while ((c1 = getc(infile)) != EOF) {
        c2 = getc(infile);
        if (c2 == EOF) {
            output64chunk(c1, 0, 0, 2, outfile);
        } else {
            c3 = getc(infile);
            if (c3 == EOF) {
                output64chunk(c1, c2, 0, 1, outfile);
            } else {
                output64chunk(c1, c2, c3, 0, outfile);
            }
        }
        ct += 4;
        if (ct > 71) {
            putc('\n', outfile);
            ct = 0;
        }
    }
    if (ct) putc('\n', outfile);
    fflush(outfile);
}

output64chunk(c1, c2, c3, pads, outfile)
FILE *outfile;
{
    putc(basis_64[c1>>2], outfile);
    putc(basis_64[((c1 & 0x3)<< 4) | ((c2 & 0xF0) >> 4)], outfile);
    if (pads == 2) {
        putc('=', outfile);
        putc('=', outfile);
    } else if (pads) {
        putc(basis_64[((c2 & 0xF) << 2) | ((c3 & 0xC0) >>6)], outfile);
        putc('=', outfile);
    } else {
        putc(basis_64[((c2 & 0xF) << 2) | ((c3 & 0xC0) >>6)], outfile);
        putc(basis_64[c3 & 0x3F], outfile);
    }
}
static char basis_hex[] = "0123456789ABCDEF";

hexchar(c)
char c;
{
    char *s;
    if (islower(c)) c = toupper(c);
    s = (char *) strchr(basis_hex, c);
    if (s) return(s-basis_hex);
    return(-1);
}


from64(infile, outfile)
FILE *infile, *outfile;
{
    unsigned int c1, c2, c3, c4, total = 0;

    while ((c1 = getc(infile)) != EOF) {
	if (isspace(c1)) continue;
        do {
            c2 = getc(infile);
        } while (c2 != EOF && isspace(c2));
        do {
            c3 = getc(infile);
        } while (c3 != EOF && isspace(c3));
        do {
            c4 = getc(infile);
        } while (c4 != EOF && isspace(c4));
        if (c2 == EOF || c3 == EOF || c4 == EOF) {
            fprintf(stderr, "Premature EOF!\n");
            return(total);
        }
        c1 = char64(c1);
        c2 = char64(c2);
        putc(((c1<<2) | ((c2&0x30)>>4)), outfile);
	++total;
        if (c3 != '=') {
            c3 = char64(c3);
            putc((((c2&0XF) << 4) | ((c3&0x3C) >> 2)), outfile);
	    ++total;
            if (c4 != '=') {
                c4 = char64(c4);
                putc((((c3&0x03) <<6) | c4), outfile);
		++total;
            }
	}
    }
}

char64(c)
char c;
{
    char *s = (char *) strchr(basis_64, c);
    if (s) return(s-basis_64);
    return(-1);
}

fromqp(infile, outfile)
FILE *infile, *outfile;
{
    int c1, c2, total = 0;

    while ((c1 = getc(infile)) != EOF) {
	if (c1 == '=') {
	    c1 = getc(infile);
	    if (c1 != '\n') {
		c2 = getc(infile);
		c1 = hexchar(c1);
		c2 = hexchar(c2);
		putc(c1<<4 | c2, outfile);
		++total;
	    }
	} else {
	    putc(c1, outfile);
	    ++total;
	}
    }
    return(total);
}

