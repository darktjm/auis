#ifndef _transform_h_
#define _transform_h_
/***********************************************************************

Copyright (C) 1991 Dean Rubine

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License. See ../COPYING for
the full agreement.

**********************************************************************/

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
