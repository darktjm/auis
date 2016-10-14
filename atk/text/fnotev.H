#ifndef _fnotev_H_
#define _fnotev_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libtext
 * @{ */
#include <view.H>

class fnotev: public  view {
  public:
    TRACED_CLASS(fnotev);
    fnotev();

    void FullUpdate(enum view::UpdateType type, long left, long top, long width, long right);
    void Update();
    class view * Hit (enum view::MouseAction action, long x, long y, long numberOfClicks)  ;
    void ObservedChanged (class observable *changed, long value);
    view::DSattributes DesiredSize(long width, long height, enum view::DSpass pass, long *dWidth, long *dheight)  ;
    void LinkTree(class view *parent);
    void GetOrigin(long width, long height, long *originX, long *originY);
    void Print(FILE *file, const char *processor, const char *finalFormat, boolean topLevel);
    void ReceiveInputFocus();
    boolean RecSearch(class search *pat, boolean toplevel);
    boolean RecSrchResume(class search *pat);
    void *GetPSPrintInterface(const char *printtype);

    virtual void pushchild();
    virtual void popchild();

    inline void SetDisplayStr(const char *S) { ((this)->displaystr = (S)); }
    inline class textview * GetParentTextview() { return ((this)->parentview); }
    inline class text * GetParentText() { return ((this)->parenttext); }

    static void SetEndnote(boolean doendnotes);

    static boolean InitializeClass();

    struct impair *imp;
    class fontdesc *fd,*ofd;
    class cursor *cursor;
    int fnotetype;
    class text *parenttext;
    class textview *parentview;
    const char *displaystr;
    struct  fontdesc_charInfo *ci;
};
/** @} */
#endif /* _fnotev_H_ */
