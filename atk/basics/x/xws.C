/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("xws.H")
#include <xgraphic.H>
#include <X11/Xlib.h>
#include <xws.H>
#include <xcursor.H>
#include <xfontdesc.H>
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
