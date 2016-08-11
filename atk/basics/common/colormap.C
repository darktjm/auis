/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/basics/common/RCS/colormap.C,v 1.1 1996/05/06 12:35:16 robr Exp $";
#endif

#include <andrewos.h>
ATK_IMPL("colormap.H")
#include <color.H>
#include <colormap.H>

#define INIT_CMAP_SIZE 8

ATKdefineRegistry(colormap, dataobject, colormap::InitializeClass);

boolean
colormap::InitializeClass( )
{
    return(TRUE);
}

colormap::colormap( )
{
	ATKinit;

    this->size = INIT_CMAP_SIZE;
    this->used = 0;
    this->colors = (class color **) calloc(this->size, sizeof(class color *));
    THROWONFAILURE((TRUE));
}

colormap::~colormap( )
{
    ATKinit;
    if(this->colors!=NULL) free(this->colors);
}

int
colormap::SetColor( class color  *color, boolean  needpixel )
{
    if(this->used == this->size) {
	this->size *= 2;
	this->colors = (class color **) realloc(this->colors, this->size * sizeof(class color *));
    }
    this->colors[this->used++] = color;
    return(0);
}

int
colormap::Copy( class colormap  *source )
{
    return 0;
}

int
colormap::Merge( class colormap  *other )
{
    return 0;
}

class color *
colormap::AllocColor( char  *name, unsigned int red , unsigned int green , unsigned int blue, boolean  needpixel )
{
    return NULL;
}

int
colormap::ChangeColor( class color  *c )
{
    return 0;
}

class color *
colormap::LookupColor( char  *name, unsigned int r , unsigned int g , unsigned int b, boolean  needpixel )
{
    return NULL;
}

char *
colormap::ViewName( )
{
    return("colormapv");
}

void
colormap::DestroyColor( class color  *c )
{
    register int i;
    for(i = 0; i < this->used; i++)
	if(c == *(this->colors + i))
	    break;
    if(i != this->used) {
	this->used--;
	for(; i < this->used; i++)
	    *(this->colors + i) = *(this->colors + i + 1);
    }
}

int
colormap::LookupColorCell( class color  *c )
{
    register int i;
    int returnIndex = -1;

    for(i = 0; i < (this)->Used(); i++)
	if(c == (class color *) (this)->NthColor( i)) {
	    returnIndex = i;
	    break;
	}
    return(returnIndex);
}
