#ifndef _chompview_H_
#define _chompview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
#include <view.H>
class chompview: public  view {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	chompview();
      
	virtual ~chompview();
    
       void FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);
       view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass, long *dWidth, long *dheight)  ;
       void Print(FILE *file, const char *processor, const char *finalFormat, boolean topLevel);
       class view * Hit (enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
       void LinkTree(class view *parent);
    
      class lpair *toplpair, *lp1;
      class textview *tv;
      class month *m1, *m2;
      class monthview *mv1, *mv2;
      class scroll *s;
};

#endif /* _chompview_H_ */
