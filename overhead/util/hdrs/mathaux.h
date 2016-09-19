#ifndef MATHAUX_H
#define MATHAUX_H

#include <math.h>
#include <atkproto.h>
BEGINCPLUSPLUSPROTOS

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

ENDCPLUSPLUSPROTOS
#endif
