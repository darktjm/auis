/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
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

#include <andrewos.h>

#ifndef NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/basics/x/RCS/xcolor.C,v 3.10 1996/03/21 20:30:34 robr Stab74 $";
#endif
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
