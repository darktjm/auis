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

/*************************************************************

	mktrig.c

	Fills trig tables and creates a trig.h file
	that allows them to be compiled into draw.c

*/

#include <andrewos.h>
#include <math.h>
#ifndef PI
#define PI 3.14159265
#endif

int main();


int main() {
    int i;
    double f,g;

    f = 180.0/PI;
    printf("int SineMult[] = {\n");
    for (i=0; i<430; ++i) {
	g = (double) i / f;
	printf("\t%d,\n", (int) (sin(g) * 10000));
    }
    printf("};\n\n");
    printf("int CosineMult[] = {\n");
    for (i=0; i<430; ++i) {
	g = (double) i / f;
	printf("\t%d,\n", (int) (cos(g) * 10000));
    }
    printf("};\n\n");
    exit(0);
}


