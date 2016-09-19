/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("xcolor.H")
#include <X11/X.h>
#include <X11/Xlib.h>
#include <xcolormap.H>
#include <color.H>
#include <xcolor.H>
#include <atom.H>


xcolor::xcolor(colormap *map, const atom *name, unsigned short r, unsigned short g, unsigned short b) : ddcolor(map, name, r, g, b) {
    static atom_def trans("transparent");
    if(name!=trans) valid=Set(r, g, b);
    else {
	tcolor.pixel=0;
	tcolor.flags=DoRed|DoGreen|DoBlue;
	tcolor.red=0;
	tcolor.green=0;
	tcolor.blue=0;
	valid=TRUE;
    }
}


xcolor::~xcolor( )
{
    if(IsValid()) {
	xcolormap *m=(xcolormap *)GetColormap();
	XFreeColors(m->XDisplay(), m->XColormap(), &PixelRef(), 1, 0);
    }
}

boolean xcolor::Set(unsigned short r, unsigned short g, unsigned short b) {
    xcolormap *m=(xcolormap *)GetColormap();
    tcolor.flags=DoRed|DoGreen|DoBlue;
    tcolor.red=r;
    tcolor.green=g;
    tcolor.blue=b;
    if(XAllocColor(m->XDisplay(), m->XColormap(), &tcolor)) {
	valid=TRUE;
    } else valid=FALSE;
    ir=r;
    ig=g;
    ib=b;
    return valid;
}
