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
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/basics/common/RCS/windowsystem.C,v 3.4 1996/01/31 19:54:06 robr Stab74 $";
#endif


 


ATK_IMPL("windowsystem.H")
#include <environ.H>
#include <cursor.H>
#include <fontdesc.H>
#include <graphic.H>
#include <im.H>
#include <windowsystem.H>


struct windowsysteminfo *windowSystem;


ATKdefineRegistry(windowsystem, ATK, NULL);
#ifndef NORCSID
#endif


class cursor *windowsystem::CreateCursor()
    {
    return NULL;
}

class fontdesc *windowsystem::CreateFontdesc()
    {
    return NULL;
}

class graphic *windowsystem::CreateGraphic()
    {
    return NULL;
}

class im *windowsystem::CreateIM()
    {
    return NULL;
}

struct offscrwin * windowsystem::CreateOffScreenWindow(char  * host,long  width ,long  height)
            {
    return NULL;
}

void windowsystem::FlushAllWindows()
    {
}

boolean windowsystem::HandleFiles(long  WaitTime, boolean  beCheap)
            {
    return FALSE;
}

class colormap *
windowsystem::CreateColormap( class im  *im )
{
    return(NULL);
}

class color *
windowsystem::CreateColor( char  *name, unsigned int r , unsigned int g , unsigned int b  )
    {
    return(NULL);
}

iddimage *windowsystem::CreateDDImage(image *i, colormap **map) {
    return NULL;
}

