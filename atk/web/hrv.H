#ifndef _hrv_H_
#define _hrv_H_
/* File hrv.H created by Ward Nelson
    (c) Copyright IBM Corp 1995.  All rights reserved. 

   hrv, an inset that draws and prints a window-width horizontal line */

#include <view.H>
class hrv: public view {
  public:
    virtual ATKregistryEntry *ATKregistry();

    void FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);
    view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass, long *dWidth, long *dheight);
    void LinkTree(class view *parent);
    void ReceiveInputFocus();
    void Print(FILE *file, char *processor, char *finalFormat, boolean topLevel);

    hrv();

    class textview *parentview;
};

#endif /* _hrv_H_ */
