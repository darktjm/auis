#ifndef _matrix_h_
#define _matrix_h_
/***********************************************************************

matrix.h - matrix operations

Copyright (C) 1991 Dean Rubine

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License. See ../COPYING for
the full agreement.

**********************************************************************/

/** \addtogroup librecog
 * @{ */

/*
 
 Simple matrix operations
 Why I am writing this stuff over is beyond me

*/

/*

This package provides the Matrix and Vector data types

The difference between this matrix package and others is that:
	Vectors may be accessed as 1d arrays
	Matrices may still be accessed like two dimensional arrays
This is accomplished by putting a structure containing the bounds
of the matrix before the pointer to the (array of) doubles (in the
case of a Vector) or before the pointer to an (array of) pointers
to doubles (in the case of a Matrix).


Vectors and matrices are collectively called "arrays" herein.
*/

#include <andrewos.h>

#define HEADER(a)	( ((struct array_header *) a) - 1 )

#define	NDIMS(a)	(HEADER(a)->ndims)
#define NROWS(a)	(HEADER(a)->nrows)
#define NCOLS(a)	(HEADER(a)->ncols)
#define	ISVECTOR(a)	(NDIMS(a) == 1)
#define	ISMATRIX(a)	(NDIMS(a) == 2)

struct array_header {
	unsigned char	ndims;	/* 1 = vector, 2 = matrix */
	unsigned char	nrows;
	unsigned char	ncols;
	unsigned char	filler;
	char		pad[4];
};

typedef double **Matrix;
typedef double *Vector;

Vector	NewVector(int r);	/* (number of rows) */
Matrix	NewMatrix(int r, int c);	/* (number of rows, number of columns) */
void	FreeVector(Vector v);
void	FreeMatrix(Matrix m);
void PRINTF_LIKE(2,3) PrintVector(Vector v, const char *fmt, ...);
void PRINTF_LIKE(2,3) PrintMatrix(Matrix m, const char *fmt, ...);
double	InnerProduct(Vector v1, Vector v2);
void	MatrixMultiply(Matrix m1, Matrix m2, Matrix prod);
void	VectorTimesMatrix(Vector v, Matrix m, Vector prod);
void	ScalarTimesVector(double s, Vector v, Vector prod);
double	QuadraticForm(Vector v, Matrix m); /* (computes v'mv) */
double	InvertMatrix(Matrix input_matrix, Matrix result_matrix);	/* (returns det) */
#include "bitvector.h"
Vector	SliceVector(Vector v, BitVector rowmask);
Matrix	SliceMatrix(Matrix m, BitVector rowmask, BitVector colmask);
Vector	VectorCopy(Vector v);
Matrix	MatrixCopy(Matrix m);
#include <stdio.h>
Vector	InputVector(FILE *f);
Matrix	InputMatrix(FILE *f);

double	InvertSingularMatrix(Matrix input, Matrix result); /* (returns det) */
Matrix  DeSliceMatrix(Matrix m, double fill, BitVector rowmask, BitVector colmask, Matrix result);
void	ZeroMatrix(Matrix m);
void	FillMatrix(Matrix m, double fill);
void	ZeroVector(Vector v);

/** @} */

#endif
