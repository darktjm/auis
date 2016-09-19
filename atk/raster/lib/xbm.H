ATK_INTER
#ifndef _xbm_H_
#define _xbm_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*  xbm.H

        xbm package

        Routines for reading and writing rasters in X Bitmap format.


 */



#include <pixelimage.H>

#define  xbm_PROGRAMMERVERSION    1

#define BUFBITS (1<<17)

#include <ATK.H>
class xbm : public ATK   {
public:
	virtual ATKregistryEntry *ATKregistry();


    

    static long ReadImage(FILE *file, class pixelimage *pix)  ;
    /* Read an X Window Dump  from 'file' and put it in 'pix,'
      return error code */
    static void WriteImage(FILE *file, class pixelimage *pix, struct rectangle *sub);
    /* Write an X Window Dump  to 'file' from  subrectangle 'sub' of pixelimage 'pix'  */

};


#endif /* _xbm_H_ */
