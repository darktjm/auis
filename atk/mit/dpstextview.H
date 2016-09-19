#ifndef _dpstextview_H_
#define _dpstextview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <view.H>
class dpstextview : public  view
{
public:
	virtual ATKregistryEntry *ATKregistry();

  
    
	dpstextview();
    
	virtual ~dpstextview();
    
	static boolean InitializeClass();
	static boolean atk_inited;

  
     void Update();
     void FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);
     class view * Hit(enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
 
    virtual void SetDesired(long w, long h);
  
    inline void SetScaling(long x, long y, double w, double h) { ((this)->scale_width = (w), (this)->scale_height = (h), (this)->offset_y = (y), (this)->offset_x = (x));; }
  
    int drawn_at_least_once;
    long cached_width, cached_height;
    long desired_width, desired_height;
    double scale_width, scale_height;
    long offset_x, offset_y;
};

#endif /* _dpstextview_H_ */
