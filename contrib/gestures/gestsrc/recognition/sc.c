/***********************************************************************

sc.c - creates classifiers from feature vectors of examples, as well as
   classifying example feature vectors.

Copyright (C) 1991 Dean Rubine

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (in ../COPYING); if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***********************************************************************/

#include "bitvector.h"
#include "matrix.h"
#include "util.h"
#include "sc.h"
#include "stdio.h"
#include "math.h"
#include "zdebug.h"
#include "proto.h"

static sClassDope sAddClass(sClassifier sc, char *classname);
static void FixClassifier(sClassifier sc, Matrix avgcov);

#define	EPS	(1.0e-6)	/* for singular matrix check */
sClassifier
sNewClassifier(void)
{
	sClassifier sc = allocate(1, struct sclassifier);
	sc->nfeatures = -1;
	sc->nclasses = 0;
	sc->classdope = allocate(MAXSCLASSES, sClassDope);
	sc->w = NULL;
	return sc;
}

void
sFreeClassifier(sClassifier sc)
{
	int i;
	sClassDope scd;

	for(i = 0; i < sc->nclasses; i++) {
		scd = sc->classdope[i];
		if(scd->name) free(scd->name);
		free(scd);
		if(sc->w && sc->w[i]) FreeVector(sc->w[i]);
		if(scd->sumcov) FreeMatrix(scd->sumcov);
		if(scd->average) FreeVector(scd->average);
	}
	free(sc->classdope);
	if(sc->w) free(sc->w);
	if(sc->cnst) FreeVector(sc->cnst);
	if(sc->invavgcov) FreeMatrix(sc->invavgcov);
	free(sc);
}

sClassDope
sClassNameLookup(sClassifier sc, char *classname)
{
	int i;
	sClassDope scd;
	static sClassifier lastsc;
	static sClassDope lastscd;

	/* quick check for last class name */
	if(lastsc == sc && STREQ(lastscd->name, classname))
		return lastscd;

	/* linear search through all classes for name */
	for(i = 0; i < sc->nclasses; i++) {
		scd = sc->classdope[i];
		if(STREQ(scd->name, classname))
			return lastsc = sc, lastscd = scd;
	}
	return NULL;
}

static sClassDope
sAddClass(sClassifier sc, char *classname)
{
	sClassDope scd;

	sc->classdope[sc->nclasses] = scd = allocate(1, struct sclassdope);
	scd->name = recog_scopy(classname);
	scd->number = sc->nclasses;
	scd->nexamples = 0;
	scd->sumcov = NULL;
	++sc->nclasses;
	return scd;
}

void
sAddExample(sClassifier sc, char *classname, Vector y)
{
	sClassDope scd;
	int i, j;
	double nfv[50];
	double nm1on, recipn;

	scd = sClassNameLookup(sc, classname);
	if(scd == NULL)
		scd = sAddClass(sc, classname);

	if(sc->nfeatures == -1)
		sc->nfeatures = NROWS(y);

	if(scd->nexamples == 0) {
		scd->average = NewVector(sc->nfeatures);
		ZeroVector(scd->average);
		scd->sumcov = NewMatrix(sc->nfeatures, sc->nfeatures);
		ZeroMatrix(scd->sumcov);

	}

	if(sc->nfeatures != NROWS(y)) {
		PrintVector(y, "sAddExample: funny feature vector nrows!=%d", 
			sc->nfeatures);
		return;
	}

	scd->nexamples++;
	nm1on = ((double) scd->nexamples-1)/scd->nexamples;
	recipn = 1.0/scd->nexamples;

	/* incrementally update covariance matrix */
        for(i = 0; i < sc->nfeatures; i++)
		nfv[i] = y[i] - scd->average[i];

	/* only upper triangular part computed */
        for(i = 0; i < sc->nfeatures; i++)
	   for(j = i; j < sc->nfeatures; j++)
		scd->sumcov[i][j] += nm1on * nfv[i] * nfv[j];

	/* incrementally update mean vector */
	for(i = 0; i < sc->nfeatures; i++)
		scd->average[i] = nm1on * scd->average[i] + recipn * y[i];

}

void
sDoneAdding(sClassifier sc)
{
	int i, j;
	int c;
	int ne, denom;
	double oneoverdenom;
	Matrix s;
	Matrix avgcov;
	double det;
	sClassDope scd;

	if(sc->nclasses == 0) 
		recog_error("sDoneAdding: No classes\n");

	/* Given covariance matrices for each class (* number of examples - 1)
	    compute the average (common) covariance matrix */

	avgcov = NewMatrix(sc->nfeatures, sc->nfeatures);
	ZeroMatrix(avgcov);
	ne = 0;
	for(c = 0; c < sc->nclasses; c++) {
		scd = sc->classdope[c];
		ne += scd->nexamples;
		s = scd->sumcov;
		for(i = 0; i < sc->nfeatures; i++)
			for(j = i; j < sc->nfeatures; j++)
				avgcov[i][j] += s[i][j]; 
	}

	denom = ne - sc->nclasses;
	if(denom <= 0) {
		printf("no examples, denom=%d\n", denom);
		return;
	}

	oneoverdenom = 1.0 / denom;
	for(i = 0; i < sc->nfeatures; i++)
		for(j = i; j < sc->nfeatures; j++)
			avgcov[j][i] = avgcov[i][j] *= oneoverdenom;

	Z('a') PrintMatrix(avgcov, "Average Covariance Matrix\n");
	/* invert the avg covariance matrix */

	sc->invavgcov = NewMatrix(sc->nfeatures, sc->nfeatures);
	det = InvertMatrix(avgcov, sc->invavgcov);
	if(fabs(det) <= EPS)
		FixClassifier(sc, avgcov);
	
	/* now compute discrimination functions */
	sc->w = allocate(sc->nclasses, Vector);
	sc->cnst = NewVector(sc->nclasses);
	for(c = 0; c < sc->nclasses; c++) {
		scd = sc->classdope[c];
		sc->w[c] = NewVector(sc->nfeatures);
		VectorTimesMatrix(scd->average, sc->invavgcov, sc->w[c]);
		sc->cnst[c] = -0.5 * InnerProduct(sc->w[c], scd->average);
		/* could add log(priorprob class c) to cnst[c] */
	}

	FreeMatrix(avgcov);
	return;
}

sClassDope
sClassify(sClassifier sc, Vector fv) {
	return sClassifyAD(sc, fv, NULL, NULL);
}

sClassDope
sClassifyAD(sClassifier sc, Vector fv, double *ap, double *dp)
{
	double disc[MAXSCLASSES];
	int i, maxclass;
	double denom;
	sClassDope scd;
	double d;

	if(sc->w == NULL)
		recog_error("sClassifyAD: %p no trained classifier", sc);

	for(i = 0; i < sc->nclasses; i++)
		disc[i] = InnerProduct(sc->w[i], fv) + sc->cnst[i];

	maxclass = 0;
	for(i = 1; i < sc->nclasses; i++)
		if(disc[i] > disc[maxclass])
			maxclass = i;

	ZZ('C') {
		scd = sc->classdope[maxclass];
		PrintVector(fv, "%10.10s  ", scd->name);
		ZZZ('C') {
			for(i = 0; i < sc->nclasses; i++) {
				scd = sc->classdope[i];
				PrintVector(scd->average, "%5.5s %5g ",
					scd->name, disc[i]);
			}
		}
	}

	scd = sc->classdope[maxclass];

	if(ap) {	/* calculate probability of non-ambiguity */
		for(denom = 0, i = 0; i < sc->nclasses; i++)
			/* quick check to avoid computing negligible term */
			if((d = disc[i] - disc[maxclass]) > -7.0)
				denom += exp(d);
		*ap = 1.0 / denom;
	}

	if(dp) 	/* calculate distance to mean of chosen class */
		*dp = MahalanobisDistance(fv, scd->average, sc->invavgcov);

	return scd;
}

/*
 Compute (v-u)' sigma (v-u)
 */

double
MahalanobisDistance(Vector v, Vector u, Matrix sigma)
{
	int i;
	static Vector space;
	double result;

	if(space == NULL || NROWS(space) != NROWS(v)) {
		if(space) FreeVector(space);
		space = NewVector(NROWS(v));
	}
	for(i = 0; i < NROWS(v); i++)
		space[i] = v[i] - u[i];
	result =  QuadraticForm(space, sigma);
	return result;
}

static void FixClassifier(sClassifier sc, Matrix avgcov)
{
	int i;
	double det;
	BitVector bv;
	Matrix m, r;

	/* just add the features one by one, discarding any that cause
	   the matrix to be non-invertible */

	CLEAR_BIT_VECTOR(bv);
	for(i = 0; i < sc->nfeatures; i++) {
		BIT_SET(i, bv);
		m = SliceMatrix(avgcov, bv, bv);
		r = NewMatrix(NROWS(m), NCOLS(m));
		det = InvertMatrix(m, r);
		if(fabs(det) <= EPS)
			BIT_CLEAR(i, bv);
		FreeMatrix(m);
		FreeMatrix(r);
	}

	m = SliceMatrix(avgcov, bv, bv);
	r = NewMatrix(NROWS(m), NCOLS(m));
	det = InvertMatrix(m, r);
	if(fabs(det) <= EPS)
		recog_error("Can't fix classifier!");
	DeSliceMatrix(r, 0.0, bv, bv, sc->invavgcov);

	FreeMatrix(m);
	FreeMatrix(r);

}

void sDumpClassifier(sClassifier sc)
{
	sClassIndex c;

	printf("\n----Classifier %p, %d features:-----\n", sc, sc->nfeatures);
	printf("%d classes: ", sc->nclasses);
	for(c = 0; c < sc->nclasses; c++)
		printf("%s  ", sc->classdope[c]->name);
	printf("Discrimination functions:\n");
	for(c = 0; c < sc->nclasses; c++)
		PrintVector(sc->w[c], "%s: %g + ", sc->classdope[c]->name, sc->cnst[c]);
	printf("Mean vectors:\n");
		PrintVector(sc->classdope[c]->average, "%s: ", sc->classdope[c]->name);
	PrintMatrix(sc->invavgcov, "Inverse average covariance matrix:\n");
	printf("\n---------\n\n");
}

void
sWrite(FILE *outfile, sClassifier sc)
{
	int i;
	sClassDope scd;

	fprintf(outfile, "%d classes\n", sc->nclasses);
	for(i = 0; i < sc->nclasses; i++) {
		scd = sc->classdope[i];
		fprintf(outfile, "%s\n", scd->name);
	}
	for(i = 0; i < sc->nclasses; i++) {
		scd = sc->classdope[i];
		OutputVector(outfile, scd->average);
		OutputVector(outfile, sc->w[i]);

	}
	OutputVector(outfile, sc->cnst);
	OutputMatrix(outfile, sc->invavgcov);
}

sClassifier
sRead(FILE *infile)
{
	int i, n;
	sClassifier sc;
	sClassDope scd;
	char buf[100];

	
	printf("Reading classifier "), fflush(stdout);
	sc = sNewClassifier();
	fgets(buf, 100, infile);
	if(sscanf(buf, "%d", &n) != 1) recog_error("sRead 1");
	printf("%d classes ", n), fflush(stdout);
	for(i = 0; i < n; i++) {
		fscanf(infile, "%s", buf);
		scd = sAddClass(sc, buf);
		printf("%s ", scd->name), fflush(stdout);
	}
	sc->w = allocate(sc->nclasses, Vector);
	for(i = 0; i < sc->nclasses; i++) {
		scd = sc->classdope[i];
		scd->average = InputVector(infile);
		sc->w[i] = InputVector(infile);
	}
	sc->cnst = InputVector(infile);
	sc->invavgcov = InputMatrix(infile);
	printf("\n");
	return sc;
}


void sDistances(sClassifier sc, int nclosest)
{
	Matrix d = NewMatrix(sc->nclasses, sc->nclasses);
	int i, j;
	double min, max = 0;
	int n, mi, mj;

	printf("-----------\n");
	printf("Computing %d closest pairs of classes\n", nclosest);
	for(i = 0; i < NROWS(d); i++) {
		for(j = i+1; j < NCOLS(d); j++) {
			d[i][j] = MahalanobisDistance(
						sc->classdope[i]->average,
						sc->classdope[j]->average,
						sc->invavgcov);
			if(d[i][j] > max) max = d[i][j];
		}
	}

	for(n = 1; n <= nclosest; n++) {
		min = max;
		mi = mj = -1;
		for(i = 0; i < NROWS(d); i++) {
			for(j = i+1; j < NCOLS(d); j++) {
				if(d[i][j] < min)
					min = d[mi=i][mj=j];
			}
		}
		if(mi == -1)
			break;

		printf("%2d) %10.10s to %10.10s d=%g nstd=%g\n",
			n,
			sc->classdope[mi]->name,
			sc->classdope[mj]->name,
			d[mi][mj],
			sqrt(d[mi][mj]));

		d[mi][mj] = max+1;
	}
	printf("-----------\n");
	FreeMatrix(d);
}
