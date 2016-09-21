ATK_INTER
#ifndef _faces_H_
#define _faces_H_
/** \addtogroup libimagev
 * @{ */
/* faces.H - class description for interface from Bennet Yee's face format to image */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/

#include <image.H>
class faces : public  image {
public:
	virtual ATKregistryEntry *ATKregistry();

  
     long Read( FILE *file, long id )  ;
     long Write( FILE *file, long writeID, int level )  ;
     long WriteNative( FILE *file, const char *filename )  ;
     int Load( const char *fullname, FILE *fp )  ;
  
    static int Ident( const char *fullname )  ;
};
/** @} */
#endif /* _faces_H_ */
