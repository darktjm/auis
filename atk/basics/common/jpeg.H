ATK_INTER
#ifndef _jpeg_H_
#define _jpeg_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <image.H>

class jpeg : public  image {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	jpeg();
      
	virtual ~jpeg();

    
       long Read( FILE *file, long id )  ;
       long Write( FILE *file, long writeID, int level )  ;
       long WriteNative( FILE *file, const char *filename )  ;
       int Load( const char *fullname, FILE *fp )  ;

    
      static int Ident( const char *fullname )  ;

    
      inline void SetSaveQuality(int q) { ((this)->saveQuality = (q)); }

    
      int saveQuality;
};


#endif /* _jpeg_H_ */
