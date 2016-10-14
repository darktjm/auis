#ifndef _iconview_H_
#define _iconview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libnote
 * @{ */
#include <view.H>
class iconview : public  view
{
    public:
      TRACED_CLASS(iconview);

  
    void GetOrigin(long width, long height, long *originX, long *originY);
    class view * Hit (enum view::MouseAction action, long x, long y, long numberOfClicks)  ;
    void ReceiveInputFocus();
    void Update();
    void FullUpdate(enum view::UpdateType type, long left, long top,
		    long width, long right);
    view::DSattributes DesiredSize(long width, long height, enum view::DSpass pass,	long *dWidth, long *dheight) ;
    void SetDataObject(class dataobject * d);
    void ObservedChanged(class observable *data, long value);
    void LinkTree(class view *parent);
    void ExposeChild(class view *v);
    boolean RecSearch(class search *pat, boolean toplevel);
    boolean RecSrchResume(class search *pat);
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
/** @} */
#endif /* _iconview_H_ */
