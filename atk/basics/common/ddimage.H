#ifndef _ddimage_H_
#define _ddimage_H_
ATK_INTER
/* Copyright 1995 Carnegie Mellon University All rights reserved. */


/** \addtogroup libbasics
 * @{ */
#include <observable.H>
#include <graphic.H>
#include <image.H>

#define ddimage_MAXQUOTA (2<<24)

class iddimage : public observable {
  public:
    virtual ATKregistryEntry *ATKregistry();

    iddimage();
    iddimage(image *src, colormap **map=NULL);
    virtual ~iddimage();
   
    virtual void ObservedChanged(observable *changed, long change);
    // When the source image changes outdated will be set to TRUE.
    // If a ddimage with outdated set to TRUE is used (by calling Draw) the image will
    // be re-processed by calling Process.
    
    virtual void Release(boolean delayok=FALSE);
    // When an image is no longer onscreen Release will free any
    // resources used in displaying the image.  (such as colors.)
    // outdated will be set to TRUE indicating that the image
    // needs to be re-processed if it is displayed again. Any portion
    // of the image which remains on screen may be corrupted.
    // If appropriate Release will also call ReleaseData.
    
    virtual void ReleaseData(boolean delayok=FALSE);
    // If display update speed is not important, but memory is then
    // ReleaseData can be used to release any memory used to
    // cache the results of processing.  No resource will be freed
    // if doing so would corrupt the on-screen image.
    
    virtual boolean Process(graphic *dest);
    // Process will process the image in whatever manner is needed
    // for display on the particular device the image will be displayed
    // upon.  The processing may include allocating display device
    // resources such as colors.  oudated will be set to FALSE.
    // Process will be called automatically if needed, but may be called
    // manually to perform the potentially lengthly processing
    // at a more convenient time.  If the processing fails FALSE
    // will be returned, otherwise TRUE will be returned.

    virtual void Draw(graphic *dest, long left, long top, long width, long height, long destLeft, long destTop);
    //  Draws the image area enclosed by the rectangle width x height @ (left,top) with
    // the upper left at (destLeft, destTop) in the dest graphic.
    
    virtual void Draw(graphic *dest, const struct rectangle *src, long destLeft, long destTop);
    //  Draws the image area enclosed by the rectangle src with
    // the upper left at (destLeft, destTop) in the dest graphic.
     
    void SetSource(image *src);
    inline image *GetSource() {
	return source;
    }

    inline boolean Outdated() 
      // If the image needs to be reprocessed for any reason Outdated will return TRUE.
    {
	return outdated;
    }

    inline colormap **GetColormap() {
	return cmap;
    }

    inline void SetColormap(colormap **map) {
	if((map!=cmap) || (map && cmap && *map!=*cmap)) {
	    Release(FALSE);
	    outdated=TRUE;
	    if(cmap && *cmap) (*cmap)->RemoveObserver(this);
	    cmap=map;
	    if(map && *map) (*map)->AddObserver(this);
	}
    }

    // if it is useful for some device dependent layer to let the client
    // suggest a quota they may use the member 'quota' for that purpose.
    // the X layer ignores this.
    inline void SetQuota(int q) {
	if(quota!=q) {
	    quota=q;
	    outdated=TRUE;
	}
    }

    inline int GetQuota() {
	return quota;
    }
    
    colormap **cmap;
    boolean outdated;
    image *source;
    int quota;
};

/* ddimage is designed to be a holder class, so that a ddimage can be a member object
 in a view or other object, and it can free the underlying iddimage automatically when
 the object is destroyed.  */
class ddimage {
    iddimage *di;
  public:
    ddimage(image *i=NULL, colormap **map=NULL);
    ddimage(iddimage *);
    ddimage(const ddimage &);
    ~ddimage();
    ddimage &operator=(iddimage *);
    ddimage &operator=(const ddimage &);
    ddimage &operator=(image *i);
    inline void Release(boolean delayok=TRUE) {
	di->Release(delayok);
    }
    // When an image is no longer onscreen Release will free any
    // resources used in displaying the image.  (such as colors.)
    // outdated will be set to TRUE indicating that the image
    // needs to be re-processed if it is displayed again. Any portion
    // of the image which remains on screen may be corrupted.
    // If appropriate Release will also call ReleaseData.
    
    inline void ReleaseData(boolean delayok) {
	di->ReleaseData(delayok);
    }
    // If display update speed is not important, but memory is then
    // ReleaseData can be used to release any memory used to
    // cache the results of processing.  No resource will be freed
    // if doing so would corrupt the on-screen image.
    
    inline boolean Process(graphic *dest) {
	return di->Process(dest);
    }
    // Process will process the image in whatever manner is needed
    // for display on the particular device the image will be displayed
    // upon.  The processing may include allocating display device
    // resources such as colors.  oudated will be set to FALSE.
    // Process will be called automatically if needed, but may be called
    // manually to perform the potentially lengthly processing
    // at a more convenient time. Process will return TRUE if processing
    // succeeded, FALSE otherwise.

    inline void Draw(graphic *dest, long left, long top, long width, long height, long destLeft, long destTop) {
	di->Draw(dest, left, top, width, height, destLeft, destTop);
    }
    //  Draws the image area enclosed by the rectangle width x height @ (left,top) with
    // the upper left at (destLeft, destTop) in the dest graphic.
    
    inline void Draw(graphic *dest, const struct rectangle *src, long destLeft, long destTop) {
	di->Draw(dest, src, destLeft, destTop);
    }
    //  Draws the image area enclosed by the rectangle src with
    // the upper left at (destLeft, destTop) in the dest graphic.
     
    inline void SetSource(image *src) {
	di->SetSource(src);
    }
    inline image *GetSource() {
	return di->GetSource();
    }

    // If the image needs to be reprocessed for any reason Outdated will return TRUE.
    inline boolean Outdated() {
	return di->Outdated();
    }

    inline colormap **Colormap() {
	return di->GetColormap();
    }

    inline void SetColormap(colormap **map) {
	di->SetColormap(map);
    }

    inline iddimage *IDDImage() {
	return di;
    }

    inline void SetQuota(int q) {
	di->SetQuota(q);
    }

    inline int Quota() {
	return di->GetQuota();
    }
    inline void AddObserver(observable *observed) {
	di->AddObserver(observed);
    }
    inline void RemoveObserver(observable *observed) {
	if(di) di->RemoveObserver(observed);
    }
};
/** @} */ 
#endif /* _ddimage_H_ */
