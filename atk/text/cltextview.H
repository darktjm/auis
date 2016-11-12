#ifndef _cltextview_H_
#define _cltextview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libtext
 * @{ */
#define cltextview_PREPROCESS 0
#define cltextview_POSTPROCESS 1


#include <textview.H>
 
	typedef void (*cltextview_hitfptr)(class ATK *observer, class cltextview *self, long *position, long *numClicks, enum view::MouseAction *act, long *startLeft, long *startRight, long *leftPos, long *rightPos, long rock, int prepost);

class cltextview : public  textview  {
public:
	TRACED_CLASS(cltextview);
       class view *Hit(enum view::MouseAction  action, long  x, long  y, long  numberOfClicks);
       void GetClickPosition(long position, long numberOfClicks, enum view::MouseAction action, long startLeft, long startRight, long *leftPos, long *rightPos);
       void FullUpdate(enum view::UpdateType type, long left, long top, long width, long height);

    
      virtual void AddClickObserver( ATK  * observer,
				    cltextview_hitfptr callBack, long rock );
      virtual void RemoveClickObserver( ATK  * observer );
      virtual void RemoveClick( ATK  * observer,
			       cltextview_hitfptr callBack );
    
      
      
	cltextview();
    
      short maxObservers;			/* number of entries in observers table */
      struct cltextview_observer *observers;	/* table of observers */
      class cursor *cursor;
};

struct cltextview_observer
{
  ATK  * observer;
  cltextview_hitfptr callBack;
  long rock;
};
/** @} */
#endif /* _cltextview_H_ */
