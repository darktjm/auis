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


typedef Matrix Transformation;

Transformation	AllocTran();	/* Only call which allocs mem */
Transformation	IdentityTran();	/* Transformation t;  sets t to identity */
Transformation	TranslateTran();/* Transformation t; int x, y; */
Transformation	RotateTran();	/* Transformation t; int degrees; */
Transformation	RotateCosSinTran(); /* Transformation t; double a*cosine,a*sine;*/
Transformation	ScaleTran(); 	/* Transformation t; double dilation; */

void	ApplyTran(); 		/* int x, y; Transformation t; int *xp, *yp; */
#define	ComposeTran(r, a, b) 	/* Transformation r, a, b;  r = a o b */ \
			(MatrixMultiply((a), (b), (r)), (r))
double	TransScale();		/* Transformation t; */

