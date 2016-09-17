/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */

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


 

/* *********************************************** *\
*                                                   *
* Definition of the layout of a raster image file.  *
*                                                   *
\* *********************************************** */
 
struct RasterHeader {
        long Magic;             /* should be RasterMagic */
        long width;             /* Width in pixels */
        long height;            /* Height in pixels */
        short depth;            /* number of bits per pixel */
};                      
     /* This heading structure is followed by the bits of the image,
        one row after another.  Bits are packed into 8-bit bytes, and
        each row is padded to a multiple of 8 bits. Bits in each row
        are left to right high-order bit to low-order bit */
 
     /* In the packed bit array, 0 is a black bit; 1 is a white bit */
 
#define RasterMagic 0xF10040BB
