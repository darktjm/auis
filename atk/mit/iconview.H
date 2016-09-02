#ifndef _iconview_H_
#define _iconview_H_
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
class iconview : public  view
{
    public:
      virtual ATKregistryEntry *ATKregistry();

  
    void GetOrigin(long width, long height, long *originX, long *originY);
    class view * Hit (enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
    void ReceiveInputFocus();
    void Update();
    void FullUpdate(enum view_UpdateType type, long left, long top,
		    long width, long right);
    view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass,	long *dWidth, long *dheight) ;
    void SetDataObject(class dataobject * d);
    void ObservedChanged(class observable *data, long value);
    void LinkTree(class view *parent);
    void ExposeChild(class view *v);
    boolean RecSearch(struct SearchPattern *pat, boolean toplevel);
    boolean RecSrchResume(struct SearchPattern *pat);
    boolean RecSrchReplace(class dataobject *text, long pos, long len);
    void RecSrchExpose(const struct rectangle &logical, struct rectangle &hit);


    virtual void DecidedSize(long w,long h);
    virtual void RecommendSize(long w,long h);
    virtual void Open();
    virtual void Close();
    virtual void SetIconFont(const char *iconfont, int iconstyle, int iconpts);
    virtual void SetIconChar(char iconchar);
    virtual void SetTitleFont(const char *titlefont, int titlestyle, int titlepts);
    virtual void SetChild(char *viewclass);
    virtual class view * GetChild()  ;

    
    iconview();

    static boolean InitializeClass();
    static boolean atk_inited;


    virtual ~iconview();
    static void CloseRelated(class view *v);
    static void OpenRelated(class view *v);

    class fontdesc * titlefont;
    char iconchar;
    class fontdesc * iconfont;
    class view * child;
    class view * bottomview;
    int	isopen;
    long cx, cy, cw, ch;   /* childs extents in my logical space */
    long dw, dh;    /* desired hight and width */
    short neednewsize;
    class iconview *next;
};

#endif /* _iconview_H_ */
