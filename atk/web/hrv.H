#ifndef _hrv_H_
#define _hrv_H_
/** \addtogroup libhtmltext
 * @{ */
/* File hrv.H created by Ward Nelson
    (c) Copyright IBM Corp 1995.  All rights reserved. 

   hrv, an inset that draws and prints a window-width horizontal line */

#include <view.H>
class hrv: public view {
  public:
    TRACED_CLASS_NODEST(hrv);

    void FullUpdate(enum view::UpdateType type, long left, long top, long width, long right);
    view::DSattributes DesiredSize(long width, long height, enum view::DSpass pass, long *dWidth, long *dheight);
    void LinkTree(class view *parent);
    void ReceiveInputFocus();
    void Print(FILE *file, const char *processor, const char *finalFormat, boolean topLevel);

    hrv();

    class textview *parentview;
};
/** @} */
#endif /* _hrv_H_ */
