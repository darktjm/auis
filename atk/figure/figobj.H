#ifndef _figobj_H_
#define _figobj_H_
/** \addtogroup libfigure
 * @{ */
/* figobj.H - fig element object */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/

#include <rect.h>
#include <point.h>

#define figobj_NULLREF (-1)

/* status values for NotifyObservers */
#define figobj_DATACHANGED 1

/* return values for Build() */
enum figobj_Status { 
    figobj_Done,
    figobj_NotDone,
    figobj_Failed
};

/* Hints for objects to tell the constraint manager what
    a particular handle does for the object. */
enum figobj_HandleType {
    figobj_None,
    figobj_ULCorner,
    figobj_LLCorner,
    figobj_LRCorner,
    figobj_URCorner,
    figobj_MiddleLeft,
    figobj_MiddleRight,
    figobj_MiddleTop,
    figobj_MiddleBottom,
    figobj_Center,
    figobj_LeftEndpoint,
    figobj_RightEndpoint,
    figobj_InteriorPoint,
    figobj_Point
};

struct figobj_Attachment {
    boolean on;
    long rposx, rposy;
    long offx, offy;
};
#include <figure.H>
#include <dataobject.H>
#include <view.H>
#include <figview.H>

class figobj : public  dataobject  {
    public:
      virtual ATKregistryEntry *ATKregistry();
      static boolean InitializeClass();

      figobj();
      virtual ~figobj();


      long Write(FILE *fp, long id, int level)  ;
      long Read(FILE *file, long id)  ;


      virtual void WriteBody(FILE *fp);
      virtual long ReadBody(FILE *file, boolean recompute)  ;
      virtual const char * ToolName(class figtoolview *v, long rock)  ;
      virtual void ToolModify(class figtoolview *v, long rock, boolean firsttime); 
      virtual class figobj * Instantiate(class figtoolview *v, long rock)  ;  

      virtual void Draw(class figview *v); 
      virtual void Sketch(class figview *v);
      virtual void DrawAttachments(class figview *v);
      virtual void ClearAttachments();
      virtual void PrintObject(class figview *v, FILE *file, const char *prefix, boolean newstyle);
      virtual void Select(class figview *v);
      virtual void SetNumHandles(long num);
      virtual enum figobj_Status Build(class figview *v, enum view::MouseAction action, long x, long y, long clicks)  ;
      virtual enum figobj_HitVal HitMe(long x, long y, long delta, long *ptref)  ;
      virtual  figobj_HitVal BasicHitMe(long x, long y, long delta, long *ptref)  ;
      virtual boolean Reshape(enum view::MouseAction action, class figview *v, long x, long y, boolean handle, long ptref)  ;
      virtual boolean AddParts(enum view::MouseAction action, class figview *v, long x, long y, boolean handle, long ptref)  ;
      virtual boolean DeleteParts(enum view::MouseAction action, class figview *v, long x, long y, boolean handle, long ptref)  ;
      virtual void MoveHandle(long  x, long y, long ptref);
      virtual void Reposition(long xd, long yd);
      virtual void InheritVAttributes(class figattr *attr, unsigned long mask);
      virtual unsigned long UpdateVAttributes(class figattr *attr, unsigned long mask)  ;
      virtual void RecomputeBounds();
      virtual struct rectangle * GetBounds(class figview *vv)  ;
      virtual struct rectangle * GetSelectedBounds(class figview *vv)  ;
      virtual void ComputeSelectedBounds();
      virtual void UpdateParentBounds();
      virtual void SetParent(long parentref, class figure *fig_ancestor);
      virtual enum figobj_HandleType GetHandleType(long ptref)  ;
      virtual long * GetCanonicalHandles()  ;
      virtual void StabilizeAttachments(boolean keepproport);
      virtual boolean ORecSearch(class search *pat);
      virtual boolean ORecSrchResume(class search *pat);
      virtual boolean ORecSrchReplace(class dataobject *text, long pos, long len);
      virtual void SetName(char *name);

    
      inline class figogrp  * &GetParent()  { return ((this)->parent); }
      inline long   &GetParentRef()  { return ((this)->parentref); }
      inline class figure  * &GetAncestorFig()  { return ((this)->figo); }
      inline boolean   &IsGroup()  { return ((this)->isgroup); }
      inline boolean   &IsInset()  { return ((this)->isinset); }
      inline long   &AttributesUsed()  { return ((this)->attrused); }
      inline struct point  * &GetHandles()  { return ((this)->pts); }
      inline long   &GetNumHandles()  { return ((this)->numpts); }
      inline void SetHandle(int num, long x, long y)  {(point_SetPt(&((this)->pts[num]), (x), (y))); }
      inline long   &PosX()  { return ((this)->x); }
      inline long   &PosY()  { return ((this)->y); }
      inline void  SetBoundsRect(long left, long top, long width, long height)  { (rectangle_SetRectSize(&((this)->bounds), (left), (top), (width), (height))); }
      inline class figattr  * &GetVAttributes()  { return ((this)->attr); }
      inline class figattr  * &GetIVAttributes()  { return ((this)->iattr); }
      inline boolean GetReadOnly()  { return ((this)->figo && ((this)->figo)->GetReadOnly()); }

      inline boolean IsAttachmentActive(int num)  { return ((this)->vas[num].on); }
      inline void SetAttachmentActive(int num, boolean val)  { ((this)->vas[num].on = (val), (this)->anyattachmentsactive = (this)->anyattachmentsactive || (val)); }
      inline void SetAttachmentPosX(int num, long pos) { ((this)->vas[num].rposx=(pos)); }
      inline void SetAttachmentOffX(int num, long offset) { ((this)->vas[num].offx=(offset)); }
      inline long GetAttachmentPosX(int num) { return ((this)->vas[num].rposx); }
      inline long GetAttachmentOffX(int num) { return ((this)->vas[num].offx); }
      inline void SetAttachmentPosY(int num, long pos) { ((this)->vas[num].rposy=(pos)); }
      inline void SetAttachmentOffY(int num, long offset) { ((this)->vas[num].offy=(offset)); }
      inline long GetAttachmentPosY(int num) { return ((this)->vas[num].rposy); }
      inline long GetAttachmentOffY(int num) { return ((this)->vas[num].offy); }

      inline long GetHandleX(int num) { return ((this)->pts[num].x); }
      inline long GetHandleY(int num) { return ((this)->pts[num].y); }
      inline char *GetName() { return (this->name); }
    
      long parentref;
      class figogrp *parent;
      class figure *figo;
      char *name;
      boolean isgroup, isinset;
      long attrused;
      long x, y;
      class figattr *attr;
      class figattr *iattr;
      struct rectangle bounds, selbounds;
      long pt_size;
      long numpts;
      struct point *pts;
      struct figobj_Attachment *vas;
      boolean anyattachmentsactive;
};
/** @} */
#endif /* _figobj_H_ */
