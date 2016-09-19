ATK_INTER
#ifndef _xcolor_H_
#define _xcolor_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <color.H>
#include <X11/X.h>
#include <X11/Xlib.h>
class xcolor : public  ddcolor {
  public:
    xcolor();

    virtual ~xcolor();
    xcolor(colormap *map, const atom *name, unsigned short r, unsigned short g, unsigned short b); 
    inline unsigned long &PixelRef() { return ((this)->tcolor.pixel); }
    inline XColor &GetXColor() { return ((this)->tcolor); }
    inline void GetRGB(unsigned short &r, unsigned short &g, unsigned short &b) {
	r=tcolor.red;
	g=tcolor.green;
	b=tcolor.blue;
    }
    inline boolean IsValid() {
	return valid;
    }
    boolean Set(unsigned short r, unsigned short g, unsigned short b);
    XColor tcolor;
    boolean valid;
};

#endif /* _xcolor_H_ */
