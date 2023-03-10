#ifndef _cmuwm_H_
#define _cmuwm_H_
/** \addtogroup libraster
 * @{ */
/* cmuwm.H - class description for ATK raster interface to image */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/

#include <image.H>
class cmuwm : public  image {
public:
	TRACED_CLASS_NODEST(cmuwm);

  
     long Read( FILE *file, long id )  ;
     long Write( FILE *file, long writeID, int level )  ;
     long WriteNative( FILE *file, const char *filename )  ;
     int Load( const char *fullname, FILE *fp )  ;
  
    static int Ident( const char *fullname )  ;
};
/** @} */
#endif /* _cmuwm_H_ */
