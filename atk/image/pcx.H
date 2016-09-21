ATK_INTER
#ifndef _pcx_H_
#define _pcx_H_
/** \addtogroup libimagev
 * @{ */
/* pcx.H - class description for interface from PCX format to image */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/

#include <image.H>
class pcx : public  image {
public:
	virtual ATKregistryEntry *ATKregistry();

  
     long Read( FILE *file, long id )  ;
     long Write( FILE *file, long writeID, int level )  ;
     long WriteNative( FILE *file, const char *filename )  ;
     int Load( const char *fullname, FILE *fp )  ;
  
    static int Ident( const char *fullname )  ;
};
/** @} */
#endif /* _pcx_H_ */
