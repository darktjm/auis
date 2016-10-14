/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("windowsystem.H")
#include <environ.H>
#include <cursor.H>
#include <fontdesc.H>
#include <graphic.H>
#include <im.H>
#include <windowsystem.H>


ATKdefineRegistryNoInit(windowsystem, ATK);

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

