#ifndef _compressv_H_
#define _compressv_H_
/* File compressv.ch created by R S Kemmetmueller
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $

   compressv: a view to display a box where the hidden text lies. */

#include <view.H>
class compressv: public view {
  public:
    virtual ATKregistryEntry *ATKregistry();
    static boolean InitializeClass();

    compressv();
    virtual ~compressv();

    view_DSattributes DesiredSize(long width,long height,enum view_DSpass pass,long *dWidth,long *dheight);
    void FullUpdate(enum view_UpdateType type,long left,long top,long width,long right);
    void GetOrigin(long width, long height, long *originX, long *originY);
    view *Hit(enum view_MouseAction action,long x,long y,long numberOfClicks);
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

#endif /* _compressv_H_ */
