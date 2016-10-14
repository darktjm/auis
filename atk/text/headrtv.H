#ifndef _headrtv_H_
#define _headrtv_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libtext
 * @{ */
#include <header.H>
#include <view.H>
class headrtv: public  view {
    public:
      TRACED_CLASS_NODEST(headrtv);
      headrtv();
      static boolean InitializeClass();
      static boolean atk_inited;

      void PostKeyState(class keystate *ks);
      void Print(FILE *file,const char *processor, const char *finalFormat, boolean topLevel);
      void WantInputFocus(class view *requestor);
      void LinkTree(class view *parent);
      class view * Hit (enum view::MouseAction action, long x, long y, long numberOfClicks)  ;
      void SetDataObject(class dataobject *object);
      void Update();
      void ReceiveInputFocus();
      void LoseInputFocus();
      void FullUpdate(enum view::UpdateType type, long left, long top, long width, long right);
      view::DSattributes DesiredSize(long width, long height, enum view::DSpass pass, long *dWidth, long *dheight)  ;
      void GetOrigin(long width, long height, long *originX, long *originY);
      void *GetPSPrintInterface(const char *printtype);

      virtual char *GetExpandedString(int pos, boolean *haspage);

      class fontdesc *font,*bfont,*bifont,*ifont;
      class keystate *keystate;
      boolean open;
      long top;
      long closebox, wherebox;
      int my_focus;
      class lpair *sections;
      class textview *textvs[header_TEXTS];
};
/** @} */
#endif /* _headrtv_H_ */
