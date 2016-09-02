#ifndef _quant_H_
 #define _quant_H_
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
typedef unsigned short pixval;

typedef struct pixelstruct {
    pixval r, g, b;
} pixel;

typedef struct colorhist_item {
    pixel color;
    int value;
} *colorhist_vector;

#define PPM_GETR(p) ((p).r)
#define PPM_GETG(p) ((p).g)
#define PPM_GETB(p) ((p).b)
#define PPM_ASSIGN(p,red,grn,blu) do { (p).r = (red); (p).g = (grn); (p).b = (blu); } while ( 0 )
#define PPM_EQUAL(p,q) ( (p).r == (q).r && (p).g == (q).g && (p).b == (q).b )
/* Luminance macro. */

#define PPM_LUMIN(p) ( 0.299 * PPM_GETR(p) + 0.587 * PPM_GETG(p) + 0.114 * PPM_GETB(p) )

#ifdef __cplusplus
extern "C"
#endif
colorhist_vector mediancut(colorhist_vector chv, int colors, int sum, pixval maxval, int newcolors);
#endif /*_quant_H_ */
