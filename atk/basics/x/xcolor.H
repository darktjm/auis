ATK_INTER
#ifndef _xcolor_H_
#define _xcolor_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/

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
