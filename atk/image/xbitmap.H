ATK_INTER
#ifndef _xbitmap_H_
#define _xbitmap_H_
/** \addtogroup libimagev
 * @{ */
/* xbitmap.H - class description for interface from Xbitmap format to image */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/

#include <image.H>
class xbitmap : public  image {
public:
	virtual ATKregistryEntry *ATKregistry();

  
     long Read( FILE *file, long id )  ;
     long Write( FILE *file, long writeID, int level )  ;
     long WriteNative( FILE *file, const char *filename )  ;
     int Load( const char *fullname, FILE *fp )  ;
  
    static int Ident( const char *fullname )  ;
};
/** @} */
#endif /* _xbitmap_H_ */
