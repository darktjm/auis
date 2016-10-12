#ifndef _pagev_H_
#define _pagev_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libarbiter
 * @{ */
struct pagev_switcheroo {
    struct page_switchee *switchee;
    class view *v;
    struct pagev_switcheroo *next;
};

#include <view.H>
class pagev: public view {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	static boolean InitializeClass();

      
	pagev();
      
	virtual ~pagev();
    
       void FullUpdate(enum view::UpdateType type, long left,
		  long top, long width, long height);
       void Update();
       class view * Hit (enum view::MouseAction action, long x, long y,
	    long numberOfClicks)  ;
       void PostKeyState(class keystate *keystate);
       void PostMenus(class menulist *ml);
       void LinkTree(class view *parent);
       void WantInputFocus(class view *v);
       void InitChildren() ;
     view::DSattributes DesiredSize(long width, long height, enum view::DSpass pass, long *dWidth, long *dheight)  ;
     void Print(FILE *file, const char *processor, const char *finalFormat, boolean topLevel);
     void ObservedChanged (class observable *changed, long value);

    
      class keystate *ks;
      class menulist *ml;
      struct pagev_switcheroo *Firstswitcheroo, *NowPlaying;
};
/** @} */
#endif /* _pagev_H_ */
