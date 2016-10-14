#ifndef _bpv_H_
#define _bpv_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libtext
 * @{ */
#include <view.H>

class bpv: public  view {
    public:
      TRACED_CLASS(bpv);
      static boolean InitializeClass();
      bpv();

      void SetDataObject(class dataobject *dataobject);
      void ObservedChanged (class observable *changed, long value);
      void FullUpdate(enum view::UpdateType type, long left, long top, long width, long right);
      void Update();
      view::DSattributes DesiredSize(long width, long height, enum view::DSpass pass, long *dWidth, long *dheight)  ;
      void PostMenus(class menulist  *ml);
      class view * Hit (enum view::MouseAction action, long x, long y, long numberOfClicks)  ;
      void ReceiveInputFocus();
      void LoseInputFocus();
      void Print(FILE *file, const char *processor, const char *finalFormat, boolean topLevel);
      void *GetPSPrintInterface(const char *printtype);

      boolean HasInputFocus;
      boolean NowBig;
      class menulist *Menus;
};
/** @} */
#endif /* _bpv_H_ */
