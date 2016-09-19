/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>

#ifdef SGI_4D_ENV
#include <sys/stat.h>
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <cmenu.h>

#define class_StaticEntriesOnly

#include <xfontdesc.H>
#include <xgraphic.H>
#include <xim.H>
#include <xcursor.H>
#include <xws.H>

int xbasicsinit ();


int xbasicsinit ()
{
    xfontdesc_StaticEntry;
    xgraphic_StaticEntry;
    xim_StaticEntry;
    xcursor_StaticEntry;
    xws_StaticEntry;
    return 1;
}
