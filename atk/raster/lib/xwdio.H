ATK_INTER
#ifndef _xwdio_H_
#define _xwdio_H_
 /* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*  xwdio.ch

        xwdio package

        Routines for reading and writing rasters in X Window Dump (V7)
        as defined by XWDFile.h

 */

#include <pixelimage.H>

#define  xwdio_PROGRAMMERVERSION    1

#define BUFBITS (1<<17)

#include <ATK.H>
class xwdio : public ATK   {
public:
	virtual ATKregistryEntry *ATKregistry();


    
      static void WriteRow(FILE *file,  unsigned char *byteaddr,  long nbytes);
    /* Write 'nbytes' bytes starting at 'byteaddr'
      into 'file' in X Window Dump image format,
     after the header is written. */

    static long ReadRow(FILE *file,  unsigned char *row,  long nbytes)  ;
    /* Read 'nbytes' bytes from 'file' into
      positions starting at 'row', after the
      header is read; there is usually some
	padding in the last byte of each row,
      but the image is resized after all rows
      are read to remove it. */
    static long ReadImage(FILE *file, class pixelimage *pix)  ;
    /* Read an X Window Dump  from 'file' and
      put it in 'pix,' return error code */
    static void WriteImage(FILE *file, class pixelimage *pix, struct rectangle *sub);
    /* Write an X Window Dump  to 'file' from  subrectangle 'sub' of pixelimage 'pix'  */

};


#endif /* _xwdio_H_ */
