ATK_INTER
#ifndef _lpair_H_
#define _lpair_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libsupviews
 * @{ */
/* lpair.H
 * class header file for lpair view.
 *
 */

#define lpair_VERSION 2

#define lpair_VERTICAL			0 /* The split line runs top to bottom of the screen */
#define lpair_HORIZONTAL		1 /* The split line runs left to right on the screen */

#define lpair_NOCHANGE		(-1)	/* Used when it is wished not to change the current value of a state */

/* values for lpair.sizeform */
#define lpair_PERCENTAGE		0
#define lpair_FIXED			1 /* Compatibility is the mother of hacks... */
#define lpair_BOTTOMFIXED		1
#define lpair_TOPFIXED			2

#include <view.H>

class lpair: public view {
    public:
      virtual ATKregistryEntry *ATKregistry();


       void FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);
       void Update();
       class view * Hit (enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
       void WantUpdate(class view *requestor);
       void LinkTree(class view *parent);
       view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass, long *dWidth, long *dheight)  ;
       void InitChildren();
       boolean RecSearch(struct SearchPattern *pat, boolean toplevel);
       boolean RecSrchResume(struct SearchPattern *pat);
       boolean RecSrchReplace(class dataobject *text, long pos, long len);
       void RecSrchExpose(const struct rectangle &logical,struct rectangle &hit);


       virtual void Init(class view *l1,class view *l2,long x) ;
       virtual class lpair * VFixed(class view *l1,class view *l2,int bsize, int moveable)  ;
       virtual class lpair * VTFixed(class view *l1,class view *l2,int bsize, int moveable)  ;
       virtual class lpair * VSplit(class view *l1,class view *l2,int pct, int moveable)  ;
       virtual class lpair * HFixed(class view *l1,class view *l2,int bsize, int moveable)  ;
       virtual class lpair * HTFixed(class view *l1,class view *l2,int bsize, int moveable)  ;
       virtual class lpair * HSplit(class view *l1,class view *l2,int pct, int moveable)  ;
       virtual void SetMovable(int i) ;
       virtual void SetLPState(int porf, int vorh, int movable);
       /* Warning -- if you try to use setlpstate to change top vs bottom fixed, you will
	run into problems that need to be corrected by setting the appropriate objsize.
	This interface needs to be entirely rethought. */
       virtual void GetLPState(int *porf, int *vorh, int *movable) ;
       virtual class view * GetNth(int ai)  ;
       virtual void SetNth(int ai, class view *x) ;
       virtual class lpair * SetUp(class view *l1, class view *l2, int bsize, int porf, int vorh, boolean moveable)  ;

       inline int &GetObjSize(int i) { return (this->objsize[i]); }

       static class lpair * Create(class view *l1,class view *l2,long x)  ;

       lpair();

       virtual ~lpair();


       class cursor *cursor; /* The "correct" cursor for this lpair. */
       class view *obj[2];	/* object */
       int objsize[2];	/* obj size */
       int objcvt[2];	/* converted size */
       int typex;	/* vertical or horizontal */
       int lasthit;        /* Coordinate (perpendicular to bar) of last mouse down. */
       char sizeform;	/* form of the size specs */
       char movable; 	/* May we drag the boundary? */
       char ismoving;	/* In the middle of a move */
       char needsfull;	/* Full update needed 'cause either size or children changed */
       boolean maybeZero;		/* true if one half can shrink to 0 */
       boolean recsearchvalid;
       int recsearchchild;
};
/** @} */
#endif /* _lpair_H_ */
