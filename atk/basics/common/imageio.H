#ifndef _imageio_H_
#define _imageio_H_
/* copied from jpeg.H.  Bite me, IBM. */

/** \addtogroup libbasics
 * @{ */
#include <image.H>

class imageio : public  image {
public:
    TRACED_CLASS_NODEST(imageio);

    imageio();

    long Read( FILE *file, long id )  ;
    /* FIXME: this needs a way to suggest write format if file name doesn't */
    long WriteNative( FILE *file, const char *filename )  ;
    int Load( const char *fullname, FILE *fp )  ;

    static int Ident( const char *fullname )  ;
    inline void SetSaveQuality(int q) { ((this)->saveQuality = (q)); }
private:
    int saveQuality;
};
/** @} */
#endif /* _imageio_H_ */
