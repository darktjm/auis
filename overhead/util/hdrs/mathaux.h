/* C++ified by magic !@#%&@#$ */
#include <math.h>
#include <atkproto.h>
BEGINCPLUSPLUSPROTOS


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
#ifndef MATHAUX_H
#define MATHAUX_H


/* Useful constants */

#define mathaux_PI		M_PI
#define mathaux_E		M_E

#define mathaux_MAXFLOAT	MAXFLOAT
#define mathaux_MAXDOUBLE	
#define mathaux_FLOATEPSILON	(0.0)
#define mathaux_DOUBLEEPSILON	(0.0)
#define mathaux_MAXINT		0x7FFFFFFF
#define mathaux_MININT		0x80000000

#define mathaux_Distance(x1, y1, x2, y2) \
	hypot((x2) - (x1), (y2) - (y1))

#define mathaux_Floor(x) \
	floor(x)

#define mathaux_Ceiling(x) \
	ceil(x)

#define mathaux_Truncate(x) \
	(((x) < 0.0) ? ceil(x) : floor(x))

#define mathaux_Augment(x) \
	(((x) < 0.0) ? floor(x) : ceil(x))

#define mathaux_RoundUp(x) \
	floor((x) + 0.5)

#define mathaux_RoundTowardInfinity(x) \
	(((x) < 0.0) ? ceil((x) - 0.5) : mathaux_RoundUp(x))

#define mathaux_RoundTowardZero(x) \
	(((x) > 0.0) ? ceil((x) - 0.5) : mathaux_RoundUp(x))

#endif

ENDCPLUSPLUSPROTOS
 
