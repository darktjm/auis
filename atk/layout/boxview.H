#ifndef _boxview_H_
#define _boxview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup liblayout
 * @{ */
/* boxview.H - box view definition and interface */
#define boxview_VERSION 1


/* Interface definition */

#include <view.H>
class boxview: public  view {
public:
	virtual ATKregistryEntry *ATKregistry();



   void FullUpdate(enum view::UpdateType how, long left, long top, long width, long height);
   void Update();
   class view * Hit(enum view::MouseAction action, long x, long y, long numberOfClicks)  ;
   view::DSattributes DesiredSize(long width, long height, enum view::DSpass pass, long *dWidth, long *dheight)  ;
   void WantNewSize(class view *requestor);
   void LinkTree(class view *parent);
   void UnlinkNotification(class view *unlinkedview);
   void ObservedChanged(class observable *changed, long status);
   void Print(FILE *f, const char *processor, const char *finalFormat, boolean toplevel);


  virtual void ToggleDebug();
  virtual void Paste();
  virtual void DrawBox();
  virtual int BoxWidth()  ;


  
	static boolean InitializeClass();

  
	boxview();
  
	virtual ~boxview();


	boolean	updateNeeded;		    /* the box itself needs to be updated */
	long lastUpdate;		    /* modification timestamp of last update */
	class view *child;		    /* view of contents */
};
/** @} */
/* end of boxview.H */


#endif /* _boxview_H_ */
