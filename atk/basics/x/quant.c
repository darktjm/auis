/* ppmquant.c - quantize the colors in a pixmap down to a specified number
**
** Copyright (C) 1989, 1991 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
  */
  
  /* XXX MODIFIED for ATK by Rob Ryan (robr@cmu.edu) January 1996 XXX */
  
  /*
** Here is the fun part, the median-cut colormap generator.  This is based
** on Paul Heckbert's paper "Color Image Quantization for Frame Buffer
** Display", SIGGRAPH '82 Proceedings, page 297.
*/
#include <andrewos.h>
#include <quant.h>

/* #define LARGE_NORM */
#define LARGE_LUM

/* #define REP_CENTER_BOX */
/* #define REP_AVERAGE_COLORS */
#define REP_AVERAGE_PIXELS

typedef struct box* box_vector;
struct box
{
    int index;
    int colors;
    int sum;
};

static int
redcompare(colorhist_vector ch1, colorhist_vector ch2)
{
    return (int) PPM_GETR( ch1->color ) - (int) PPM_GETR( ch2->color );
}

static int
greencompare(colorhist_vector ch1, colorhist_vector ch2)
{
    return (int) PPM_GETG( ch1->color ) - (int) PPM_GETG( ch2->color );
}

static int
bluecompare(colorhist_vector ch1,colorhist_vector ch2 )
{
    return (int) PPM_GETB( ch1->color ) - (int) PPM_GETB( ch2->color );
}

static int
sumcompare(box_vector b1,box_vector b2 )
{
    return b2->sum - b1->sum;
}

colorhist_vector
mediancut( colorhist_vector chv, int colors, int sum, pixval maxval, int newcolors )
{
    colorhist_vector colormap;
    box_vector bv;
    int bi, i;
    int boxes;

    colormap =
      (colorhist_vector) malloc( sizeof(struct colorhist_item) * newcolors );
    if (colormap == (colorhist_vector) 0 ) return NULL;
    for ( i = 0; i < newcolors; ++i )
	PPM_ASSIGN( colormap[i].color, 0, 0, 0 );
    if(colors<=newcolors) {
	for(i=0;i<colors;i++) {
	    PPM_ASSIGN(colormap[i].color, PPM_GETR(chv[i].color), PPM_GETG(chv[i].color), PPM_GETB(chv[i].color));
	}
	return colormap;
    }
    bv = (box_vector) malloc( sizeof(struct box) * newcolors );
    if(bv == (box_vector) 0) {
	free(colormap);
	return NULL;
    }
    /*
     ** Set up the initial box.
     */
    bv[0].index = 0;
    bv[0].colors = colors;
    bv[0].sum = sum;
    boxes = 1;

    /*
     ** Main loop: split boxes until we have enough.
     */
    while ( boxes < newcolors )
    {
	int indx, clrs;
	int sm;
	int minr, maxr, ming, maxg, minb, maxb, v;
	int halfsum, lowersum;

	/*
	 ** Find the first splittable box.
	 */
	for ( bi = 0; bv[bi].colors < 2 && bi < boxes; ++bi )
	    ;
	if ( bi == boxes )
	    break;	/* ran out of colors! */
	indx = bv[bi].index;
	clrs = bv[bi].colors;
	sm = bv[bi].sum;

	/*
	 ** Go through the box finding the minimum and maximum of each
	 ** component - the boundaries of the box.
	 */
	minr = maxr = PPM_GETR( chv[indx].color );
	ming = maxg = PPM_GETG( chv[indx].color );
	minb = maxb = PPM_GETB( chv[indx].color );
	for ( i = 1; i < clrs; ++i )
	{
	    v = PPM_GETR( chv[indx + i].color );
	    if ( v < minr ) minr = v;
	    if ( v > maxr ) maxr = v;
	    v = PPM_GETG( chv[indx + i].color );
	    if ( v < ming ) ming = v;
	    if ( v > maxg ) maxg = v;
	    v = PPM_GETB( chv[indx + i].color );
	    if ( v < minb ) minb = v;
	    if ( v > maxb ) maxb = v;
	}

	/*
	 ** Find the largest dimension, and sort by that component.  I have
	 ** included two methods for determining the "largest" dimension;
	 ** first by simply comparing the range in RGB space, and second
	 ** by transforming into luminosities before the comparison.  You
	 ** can switch which method is used by switching the commenting on
	 ** the LARGE_ defines at the beginning of this source file.
	 */
#ifdef LARGE_NORM
	if ( maxr - minr >= maxg - ming && maxr - minr >= maxb - minb )
	    qsort(
		  (char*) &(chv[indx]), clrs, sizeof(struct colorhist_item),
		  QSORTFUNC(redcompare) );
	else if ( maxg - ming >= maxb - minb )
	    qsort(
		  (char*) &(chv[indx]), clrs, sizeof(struct colorhist_item),
		  QSORTFUNC(greencompare) );
	else
	    qsort(
		  (char*) &(chv[indx]), clrs, sizeof(struct colorhist_item),
		  QSORTFUNC(bluecompare) );
#endif /*LARGE_NORM*/
#ifdef LARGE_LUM
	{
	    pixel p;
	    float rl, gl, bl;

	    PPM_ASSIGN(p, maxr - minr, 0, 0);
	    rl = PPM_LUMIN(p);
	    PPM_ASSIGN(p, 0, maxg - ming, 0);
	    gl = PPM_LUMIN(p);
	    PPM_ASSIGN(p, 0, 0, maxb - minb);
	    bl = PPM_LUMIN(p);

	    if ( rl >= gl && rl >= bl )
		qsort(
		      (char*) &(chv[indx]), clrs, sizeof(struct colorhist_item),
		      QSORTFUNC(redcompare) );
	    else if ( gl >= bl )
		qsort(
		      (char*) &(chv[indx]), clrs, sizeof(struct colorhist_item),
		      QSORTFUNC(greencompare) );
	    else
		qsort(
		      (char*) &(chv[indx]), clrs, sizeof(struct colorhist_item),
		      QSORTFUNC(bluecompare) );
	}
#endif /*LARGE_LUM*/

	/*
	 ** Now find the median based on the counts, so that about half the
	 ** pixels (not colors, pixels) are in each subdivision.
	 */
	lowersum = chv[indx].value;
	halfsum = sm / 2;
	for ( i = 1; i < clrs - 1; ++i )
	{
	    if ( lowersum >= halfsum )
		break;
	    lowersum += chv[indx + i].value;
	}

	/*
	 ** Split the box, and sort to bring the biggest boxes to the top.
	 */
	bv[bi].colors = i;
	bv[bi].sum = lowersum;
	bv[boxes].index = indx + i;
	bv[boxes].colors = clrs - i;
	bv[boxes].sum = sm - lowersum;
	++boxes;
	qsort( (char*) bv, boxes, sizeof(struct box), QSORTFUNC(sumcompare) );
    }

    /*
     ** Ok, we've got enough boxes.  Now choose a representative color for
     ** each box.  There are a number of possible ways to make this choice.
     ** One would be to choose the center of the box; this ignores any structure
     ** within the boxes.  Another method would be to average all the colors in
     ** the box - this is the method specified in Heckbert's paper.  A third
     ** method is to average all the pixels in the box.  You can switch which
     ** method is used by switching the commenting on the REP_ defines at
     ** the beginning of this source file.
     */
    for ( bi = 0; bi < boxes; ++bi )
    {
#ifdef REP_CENTER_BOX
	int indx = bv[bi].index;
	int clrs = bv[bi].colors;
	int minr, maxr, ming, maxg, minb, maxb, v;

	minr = maxr = PPM_GETR( chv[indx].color );
	ming = maxg = PPM_GETG( chv[indx].color );
	minb = maxb = PPM_GETB( chv[indx].color );
	for ( i = 1; i < clrs; ++i )
	{
	    v = PPM_GETR( chv[indx + i].color );
	    minr = min( minr, v );
	    maxr = max( maxr, v );
	    v = PPM_GETG( chv[indx + i].color );
	    ming = min( ming, v );
	    maxg = max( maxg, v );
	    v = PPM_GETB( chv[indx + i].color );
	    minb = min( minb, v );
	    maxb = max( maxb, v );
	}
	PPM_ASSIGN(
		   colormap[bi].color, ( minr + maxr ) / 2, ( ming + maxg ) / 2,
		   ( minb + maxb ) / 2 );
#endif /*REP_CENTER_BOX*/
#ifdef REP_AVERAGE_COLORS
	int indx = bv[bi].index;
	int clrs = bv[bi].colors;
	long r = 0, g = 0, b = 0;

	for ( i = 0; i < clrs; ++i )
	{
	    r += PPM_GETR( chv[indx + i].color );
	    g += PPM_GETG( chv[indx + i].color );
	    b += PPM_GETB( chv[indx + i].color );
	}
	r = r / clrs;
	g = g / clrs;
	b = b / clrs;
	PPM_ASSIGN( colormap[bi].color, r, g, b );
#endif /*REP_AVERAGE_COLORS*/
#ifdef REP_AVERAGE_PIXELS
	int indx = bv[bi].index;
	int clrs = bv[bi].colors;
	long r = 0, g = 0, b = 0, sum = 0;

	for ( i = 0; i < clrs; ++i )
	{
	    r += PPM_GETR( chv[indx + i].color ) * chv[indx + i].value;
	    g += PPM_GETG( chv[indx + i].color ) * chv[indx + i].value;
	    b += PPM_GETB( chv[indx + i].color ) * chv[indx + i].value;
	    sum += chv[indx + i].value;
	}
	r = r / sum;
	if ( r > maxval ) r = maxval;	/* avoid math errors */
	g = g / sum;
	if ( g > maxval ) g = maxval;
	b = b / sum;
	if ( b > maxval ) b = maxval;
	PPM_ASSIGN( colormap[bi].color, r, g, b );
#endif /*REP_AVERAGE_PIXELS*/
    }
    free(bv);
    /*
     ** All done.
     */
    return colormap;
}
