ATK_INTER
#ifndef _xpixmap_H_
#define _xpixmap_H_
/** \addtogroup libimagev
 * @{ */
/* xpixmap.H - class description for interface from Xpixmap format to image */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/

#include <image.H>
class xpixmap : public  image {
public:
	virtual ATKregistryEntry *ATKregistry();

  
     long Read( FILE *file, long id )  ;
     long Write( FILE *file, long writeID, int level )  ;
     long WriteNative( FILE *file, const char *filename )  ;
     int Load( const char *fullname, FILE *fp )  ;
};
/** @} */
#endif /* _xpixmap_H_ */
