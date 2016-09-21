#ifndef _xddimage_H_
#define _xddimage_H_
ATK_INTER
/* Copyright 1995 Carnegie Mellon University All rights reserved. */


/** \addtogroup libbasics
 * @{ */
#include <ddimage.H>
class graphic;
class xgraphic;
class colormap;
class XCMapImageHist;
class event;
#include <image.H>
#include <xcolor.H>
#include <xim.H>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

class WorkProc;
DEFINE_MFLEX_CLASS(xcolors,xcolor *, 10);
class xddimage : public iddimage {
  public:
    virtual ATKregistryEntry *ATKregistry();

    xddimage();
    xddimage(image *src, colormap **map=NULL);
    virtual ~xddimage();
   
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

    virtual void ReleaseResources(boolean delayok=FALSE);
    
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

    XImage *ximage;
    xcolors allocated;
    boolean released;
    size_t maxcolors;
    image *tsrc;
    boolean waiting;
    XCMapImageHist *histogram;
    event *delayedrelease;
    event *delayedeval;
    WorkProc *draw;
    
    void ProcessTrueOrDirectColor(graphic *dest);

    void ProcessWithStaticColors(graphic *dest);

    void ProcessWithColorCube(graphic *dest);

    void ProcessWithExactColors(graphic *dest);

    void ProcessWithColorManager(graphic *dest);

    void ReProcess();
    
    xgraphic *xgd;
    Pixmap pixmap;
    boolean pixok;
};
/** @} */ 
#endif /* _xddimage_H_ */
