/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*************************************************************

	mktrig.c

	Fills trig tables and creates a trig.h file
	that allows them to be compiled into draw.c

*/

#include <andrewos.h>
#include <math.h>

int main();


int main() {
    int i;
    double f,g;

    f = 180.0/M_PI;
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


