ATK_INTER
#ifndef _mac_H_
#define _mac_H_
/** \addtogroup libimagev
 * @{ */
/* mac.H - class description for interface from MACpaint format to image */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/

#include <image.H>
class mac : public  image {
public:
	virtual ATKregistryEntry *ATKregistry();

  
     long Read( FILE *file, long id )  ;
     long Write( FILE *file, long writeID, int level )  ;
     long WriteNative( FILE *file, const char *filename )  ;
     int Load( const char *fullname, FILE *fp )  ;
  
    static int Ident( const char *fullname )  ;
};
/** @} */
#endif /* _mac_H_ */
