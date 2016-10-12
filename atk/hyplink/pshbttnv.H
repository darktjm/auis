#ifndef _pushbuttonview_H_
#define _pushbuttonview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup liblink
 * @{ */
/*
  Trigger:  "buttonpushed"
     called when the user lets up on the pushbutton over the pushbutton.

  Call pushbuttonview_HitButton(self) as shorthand for pulling the trigger.

*/

#include <view.H>
#include <atom.H>
#include <pushbutton.H>
class pushbuttonview: public  view {
    public:
      virtual ATKregistryEntry *ATKregistry();



      static boolean InitializeClass();


      pushbuttonview();

      virtual ~pushbuttonview();

      void ReceiveInputFocus();
      void LoseInputFocus();
      void ObservedChanged (class observable *changed, long value);
      view::DSattributes DesiredSize(long width, long height, enum view::DSpass pass, long * desired_width, long * desired_height)  ;
      void GetOrigin(long width, long height, long * originX, long * originY);
      void FullUpdate(enum view::UpdateType type, long left, long top, long width, long height);
      void Update();
      class view * Hit (enum view::MouseAction action, long x, long y, long numberOfClicks)  ;
      void LinkTree(class view *parent);
      void PostMenus(class menulist *ml);
      void WantUpdate(class view *requestor);
      void Print(FILE *file, const char *processor, const char *finalFormat, boolean topLevel);
      boolean RecSearch(class search *pat, boolean toplevel);
      boolean RecSrchResume(class search *pat);
      boolean RecSrchReplace(class dataobject *text, long pos, long len);
      void RecSrchExpose(const struct rectangle &logical, struct rectangle &hit);
      void *GetPSPrintInterface(const char *printtype);
      void PrintPSRect(FILE *outfile, long logwidth, long logheight, struct rectangle *visrect);


      inline void HitButton() { (((class pushbutton *) (this)->GetDataObject())->PullTrigger( atom::Intern("buttonpushed"))); }

      short lit;
      class cursor *cursor;
      double foreground_color[3], background_color[3];
      const char *cached_label;
      int cached_style;
      class fontdesc *cached_fontdesc;
      class menulist *ml;
      int awaitingUpdate;
      short recsearchsubstart, recsearchsublen; /* segment of label which matched RecSearch. Only defined if recsearchvalid */
      boolean recsearchvalid;		/* was last RecSearch successful? */
      struct rectangle crect;
};
/** @} */
#endif /* _pushbuttonview_H_ */
