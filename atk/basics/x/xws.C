/*LIBS: -lcmenu -lcwm -lX11 -lerrors -lutil
*/

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
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/basics/x/RCS/xws.C,v 3.6 1996/01/31 18:41:59 robr Stab74 $";
#endif

/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

 

ATK_IMPL("xws.H")
#include <X11/Xlib.h>
#include <xws.H>
#include <xcursor.H>
#include <xfontdesc.H>
#include <xgraphic.H>
#include <xim.H>
#include <xddimage.H>
	
ATKdefineRegistryNoInit(xws, windowsystem);

class cursor *
xws::CreateCursor()
{
	return new xcursor;
}

class fontdesc *
xws::CreateFontdesc()
{
	return new xfontdesc;
}

class graphic *
xws::CreateGraphic()
{
	return new xgraphic;
}

class im *
xws::CreateIM()
{
	return new xim;
}

struct offscrwin *
xws::CreateOffScreenWindow(char  * host,long  width ,long  height)
{
/*
	return xoffscrwin_Create(host,width,height);
*/
	return NULL;
}

boolean 
xws::HandleFiles(long  WaitTime, boolean  beCheap)
{
	return xim::HandleFiles(WaitTime, beCheap);
}

void 
xws::FlushAllWindows()
{
	xim::FlushAllWindows();
}

iddimage *xws::CreateDDImage(image *i, colormap **map) {
    return new xddimage(i, map);
}
