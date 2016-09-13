#ifndef _sc_h_
#define _sc_h_
/***********************************************************************

sc.h - creates classifiers from feature vectors of examples, as well as
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



/*
 single path classifier
*/

#define	MAXSCLASSES	100

typedef struct sclassifier *sClassifier; /* single feature-vector classifier */
typedef int		    sClassIndex; /* single feature-vector class index */
typedef struct sclassdope  *sClassDope;	 /* single feature-vector class dope */

struct sclassdope {
	char 		*name;
	sClassIndex	number;
	Vector		average;
	Matrix		sumcov;
	int		nexamples;
};

struct sclassifier {
	int		nfeatures;
	int		nclasses;
	sClassDope	*classdope;

	Vector		cnst;	/* constant term of discrimination function */
	Vector		*w;	/* array of coefficient weights */
	Matrix		invavgcov; /* inverse covariance matrix */
};

/* sc.c */
extern sClassifier sNewClassifier(void);
extern void sFreeClassifier(sClassifier sc);
extern sClassDope sClassNameLookup(sClassifier sc, char *classname);
extern void sAddExample(sClassifier sc, char *classname, Vector y);
extern void sDoneAdding(sClassifier sc);
extern sClassDope sClassify(sClassifier sc, Vector fv);
extern sClassDope sClassifyAD(sClassifier sc, Vector fv, double *ap, double *dp);
extern double MahalanobisDistance(Vector v, Vector u, Matrix sigma);
extern void sDumpClassifier(sClassifier sc);
extern void sWrite(FILE *outfile, sClassifier sc);
extern sClassifier sRead(FILE *infile);
extern void sDistances(sClassifier sc, int nclosest);
#endif
