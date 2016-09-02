ATK_INTER
#ifndef _fnotev_H_
#define _fnotev_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
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
*/




#include <view.H>

class fnotev: public  view {
  public:
    virtual ATKregistryEntry *ATKregistry();
    fnotev();
    virtual ~fnotev();

    void FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);
    void Update();
    class view * Hit (enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
    void ObservedChanged (class observable *changed, long value);
    view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass, long *dWidth, long *dheight)  ;
    void LinkTree(class view *parent);
    void GetOrigin(long width, long height, long *originX, long *originY);
    void Print(FILE *file, const char *processor, const char *finalFormat, boolean topLevel);
    void ReceiveInputFocus();
    boolean RecSearch(struct SearchPattern *pat, boolean toplevel);
    boolean RecSrchResume(struct SearchPattern *pat);
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


#endif /* _fnotev_H_ */
