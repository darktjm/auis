#ifndef _compressv_H_
#define _compressv_H_
/** \addtogroup libsrctext
 * @{ */
/* File compressv.ch created by R S Kemmetmueller
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   compressv: a view to display a box where the hidden text lies. */

#include <view.H>
class compressv: public view {
  public:
    virtual ATKregistryEntry *ATKregistry();
    static boolean InitializeClass();

    compressv();
    virtual ~compressv();

    view::DSattributes DesiredSize(long width,long height,enum view::DSpass pass,long *dWidth,long *dheight);
    void FullUpdate(enum view::UpdateType type,long left,long top,long width,long right);
    void GetOrigin(long width, long height, long *originX, long *originY);
    view *Hit(enum view::MouseAction action,long x,long y,long numberOfClicks);
    void LinkTree(view *parent);
    void ObservedChanged(observable *changed,long value);
    void Print(FILE *file,const char *processor,const char *finalFormat,boolean topLevel);
    void ReceiveInputFocus();
    void Update();

    virtual char *BoxText();

    class cursor *cursor;
    class text *parenttext;
    class textview *parentview;
};
/** @} */
#endif /* _compressv_H_ */
