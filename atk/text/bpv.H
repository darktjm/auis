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
      virtual ATKregistryEntry *ATKregistry();
      static boolean InitializeClass();
      bpv();
      virtual ~bpv();

      void SetDataObject(class dataobject *dataobject);
      void ObservedChanged (class observable *changed, long value);
      void FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);
      void Update();
      view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass, long *dWidth, long *dheight)  ;
      void PostMenus(class menulist  *ml);
      class view * Hit (enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
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
