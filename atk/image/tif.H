ATK_INTER
#ifndef _tif_H_
#define _tif_H_
/** \addtogroup libimagev
 * @{ */
/* tif.H - class description for interface from Tagged Image File Format to image */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/

#include <image.H>
class tif : public  image {
public:
	virtual ATKregistryEntry *ATKregistry();

  
     long Read( FILE *file, long id )  ;
     long Write( FILE *file, long writeID, int level )  ;
/*    WriteNative( FILE *file, const char *filename ) returns long;*/
     int Load( const char *fullname, FILE *fp )  ;
  
    static int Ident( const char *fullname )  ;
};
/** @} */
#endif /* _tif_H_ */
