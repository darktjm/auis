/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

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
