#ifndef _transform_h_
#define _transform_h_
/***********************************************************************

Copyright (C) 1991 Dean Rubine

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License. See ../COPYING for
the full agreement.

**********************************************************************/

#include <gestures/matrix.h>
typedef Matrix Transformation;

extern Transformation AllocTran(void);	/* Only call which allocs mem */
extern Transformation IdentityTran(Transformation t);	/* sets t to identity */
extern Transformation TranslateTran(Transformation t, int x, int y);
extern Transformation RotateTran(Transformation t, int theta);
extern Transformation RotateCosSinTran(Transformation t, double c, double s);
extern Transformation ScaleTran(Transformation t, double dilation);

extern void ApplyTran(int x, int y, Transformation t, int *xp, int *yp);
#define	ComposeTran(r, a, b) 	/* Transformation r, a, b;  r = a o b */ \
			(MatrixMultiply((a), (b), (r)), (r))
extern double TransScale(Transformation t);

#endif
