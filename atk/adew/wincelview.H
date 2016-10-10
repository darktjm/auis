#ifndef _wincelview_H_
#define _wincelview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libarbiter
 * @{ */
#include <view.H>
class wincelview: public  view {
public:
	virtual ATKregistryEntry *ATKregistry();
	wincelview();


     void FullUpdate(enum view::UpdateType type, long left, long top, long width, long right);
     class view * Hit (enum view::MouseAction action, long x, long y, long numberOfClicks)  ;
     void ObservedChanged (class observable *changed, long value);

    virtual void pushchild();
    virtual void popchild();

    class celview *celviewp;
    class im *window;
};
/** @} */
#endif /* _wincelview_H_ */
